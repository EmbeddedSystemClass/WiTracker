#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_pm.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
// #include "mqtt_client.h"

#include "wifi.h"
#include "i2c.h"
#include "temphumid.h"
#include "uv.h"
#include "accelerometer.h"
// #include "mqtt.h"
#include "network.h"
#include "time.h"

#define SLEEP_MODE_LIGHT_LENGTH 5000

static const char *TAG = "MAIN_APP"; // logging tag

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */
RTC_DATA_ATTR static int bootCount = 0;

static Device_Data data;
static const Device_Data EmptyDataStruct;

static void program_init(void);
static void collection_preprocessor(void);
static void set_low_power_mode(void);
static void set_high_power_mode(void);
static void refresh_data(void);
static bool collect_data(void);
static void handle_data(void);
static void enter_sleep(void);

void app_main(void)
{
    program_init();

    while (1)
    {
        collection_preprocessor();

        // Active scan mode
        if (!collect_data())
        {
            ESP_LOGI(TAG, "ERROR COLLECTING DATA");
        }
        handle_data();

        // Enter light/deep sleep mode
        enter_sleep();
    }
}

// TODO: add function comments
void collection_preprocessor(void)
{
    // check if test switch is set
    // check if deep sleep switch is set
    printf("It's loop time!\n");
}

// Reads and stores results in data variables
bool collect_data(void)
{
    set_high_power_mode();
    refresh_data(); // some sensors need to be explicitly re-polled

    // The data struct is cleared by assignment at the beginning of each read to wipe previous data readings
    data = EmptyDataStruct;

    data.newData = mc_accelerometer_check_interrupt(); // we only get a new measurement when the device moves
    // if (!data.newData)
    //     return true;

    data.gpsState = 0;
    data.latitude = 0;  // mc_gps_get_latitude();
    data.longitude = 0; //mc_gps_get_longitude();

    // update_time(); // TODO should be done as part of the gps data?
    data.timestamp = 0;

    if (!mc_temphumid_get_temperature(&data.temperature))
    {
        ESP_LOGI(TAG, "ERROR GETTING TEMPERATURE");
        return false;
    }

    if (!mc_temphumid_get_humidity(&data.humidity))
    {
        ESP_LOGI(TAG, "ERROR GETTING HUMIDITY");
        return false;
    }

    data.outside = mc_uv_check_outside();
    data.tone = 0;    //mc_microphone_check_tone();
    data.voltage = 0; //mc_adc_get_voltage();

    // esp_wifi_start();
    data.wifiScanResult = mc_wifi_scan();
    // data.wifiScanResult = 0;

    return true;
}

// Processes and uploads data to the MQTT datastore
void handle_data(void)
{
    printf("temp: %f\n", data.temperature);
    printf("humidity: %f\n", data.humidity);
    printf("uva: %f\n", mc_uv_get_uva());
    // mc_network_transmit(data);
    vTaskDelay(7000 / portTICK_PERIOD_MS);
}

// Enters an appropriate sleep mode
void enter_sleep(void)
{
    set_low_power_mode();

    // TODO check for deep sleep variable
    printf("Entering light sleep for 10 seconds...\n");
    fflush(NULL);

    vTaskDelay(SLEEP_MODE_LIGHT_LENGTH / portTICK_PERIOD_MS);
}

void set_low_power_mode(void)
{
    mc_uv_set_powermode(0);
    mc_accelerometer_set_powermode(0);
    mc_wifi_disconnect();
}

void set_high_power_mode(void)
{
    mc_uv_set_powermode(1);
    mc_accelerometer_set_powermode(1);
    mc_wifi_connect();

    vTaskDelay(30 / portTICK_PERIOD_MS);
}

void refresh_data(void)
{
    mc_uv_poll();
}

// Initialises the hardware and program variables
void program_init(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    bootCount++;
    ESP_LOGI(TAG, "Boot count: %d", bootCount);

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

#if CONFIG_PM_ENABLE
    // Configure dynamic frequency scaling: maximum frequency is set in sdkconfig,
    // minimum frequency is XTAL.
    rtc_cpu_freq_t max_freq;
    rtc_clk_cpu_freq_from_mhz(CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ, &max_freq);
    esp_pm_config_esp32_t pm_config = {
        .max_cpu_freq = max_freq,
        .min_cpu_freq = RTC_CPU_FREQ_XTAL};
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
#endif // CONFIG_PM_ENABLE

    mc_wifi_init();

    mc_i2c_init();
    mc_temphumid_init();
    mc_uv_init();
    mc_accelerometer_init();

    // mc_mqtt_init(); // must be done after wifi init
    // mc_network_init(); // TODO do this to initialise network queue, etc?
    // mc_time_init();
}
