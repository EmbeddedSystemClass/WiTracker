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

#include "wifi.h"
#include "i2c.h"
#include "temphumid.h"
#include "uv.h"
#include "accelerometer.h"
#include "network.h"
#include "time.h"
#include "state.h"
#include "gpio.h"
#include "led.h"
#include "tone.h"
#include "voltage.h"

#define DEFAULT_SLEEP_LENGTH 10000
#define TEST_SLEEP_LENGTH 1000

#define TONE_1K_THRESHOLD 10
#define TONE_4K_THRESHOLD 10

static const char *TAG = "MAIN_APP"; // logging tag

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */
RTC_DATA_ATTR static int bootCount = 0;

static int sleepLength = DEFAULT_SLEEP_LENGTH;

static Device_Data data;
static const Device_Data EmptyDataStruct;

static void program_init(void);
static void collection_preprocessor(void);
static void collection_postprocessor(void);
static void set_low_power_mode(void);
static void set_high_power_mode(void);
static void refresh_data(void);
static bool collect_data(void);
static void handle_data(void);
static void enter_sleep(void);
static void enter_deep_sleep(void);
static void enter_light_sleep(void);

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

        collection_postprocessor();

        // Enter light/deep sleep mode
        enter_sleep();
    }
}

void collection_preprocessor(void)
{
    ESP_LOGI(TAG, "[APP] Active Mode Entered");

    // If we haven't already pressed the button
    if (mc_state_get_state() != DEEP_SLEEP_MODE)
        mc_state_set_state(ACTIVE_MODE);
}

// TODO: add function comments
void collection_postprocessor(void)
{
    // check if test switch is set
    if (mc_gpio_test_enabled())
        sleepLength = TEST_SLEEP_LENGTH;
    else
        sleepLength = DEFAULT_SLEEP_LENGTH;
}

// Reads and stores results in data variables
bool collect_data(void)
{
    bool result = true;

    set_high_power_mode();
    refresh_data(); // some sensors need to be explicitly re-polled

    // The data struct is cleared by assignment at the beginning of each read to wipe previous data readings
    data = EmptyDataStruct;

    data.newData = mc_accelerometer_check_interrupt(); // we only get a new measurement when the device moves
    if (!data.newData)
        return true;

    data.gpsState = 0;
    data.latitude = 0;  // mc_gps_get_latitude();
    data.longitude = 0; //mc_gps_get_longitude();

    data.timestamp = mc_time_get_timestamp();

    if (!mc_temphumid_get_temperature(&data.temperature))
    {
        ESP_LOGI(TAG, "ERROR GETTING TEMPERATURE");
        data.temperature = 0;
        result = false;
    }

    if (!mc_temphumid_get_humidity(&data.humidity))
    {
        ESP_LOGI(TAG, "ERROR GETTING HUMIDITY");
        data.humidity = 0;
        result = false;
    }

    data.outside = mc_uv_check_outside();

    Tone_Reading toneResult = mc_tone_sample();
    printf("TONERESULT %f | %f\n", toneResult.value1K, toneResult.value4K);
    if (toneResult.value1K > TONE_1K_THRESHOLD && toneResult.value4K <= TONE_4K_THRESHOLD)
        data.tone = 1;
    else if (toneResult.value1K <= TONE_1K_THRESHOLD && toneResult.value4K > TONE_4K_THRESHOLD)
        data.tone = 2;
    else if (toneResult.value1K > TONE_1K_THRESHOLD && toneResult.value4K > TONE_4K_THRESHOLD)
        data.tone = 3;
    else
        data.tone = 0;

    data.voltage = mc_voltage_sample();

    data.wifiScanResult = mc_wifi_scan();

    return result;
}

// Processes and uploads data to the MQTT datastore
void handle_data(void)
{
    // Print useful info
    printf("interrupt: %d\n", data.newData);
    if (data.newData)
    {
        printf("time: %s\n", data.timestamp);
        printf("temp: %f\n", data.temperature);
        printf("humidity: %f\n", data.humidity);
        printf("tone: %d\n", data.tone);
        printf("uva: %f\n", mc_uv_get_uva());
        printf("voltage: %d\n", data.voltage);
        printf("outside: %d\n", data.outside);
    }

    mc_network_transmit(data);
}

// Enters an appropriate sleep mode
void enter_sleep(void)
{
    if (mc_state_get_state() == DEEP_SLEEP_MODE)
        enter_deep_sleep();
    else
        enter_light_sleep();
}

void enter_deep_sleep(void)
{
    ESP_LOGI(TAG, "[APP] Deep Sleep Mode Entered");

    mc_state_set_state(DEEP_SLEEP_MODE);
    set_low_power_mode();

    while (mc_state_get_state() == DEEP_SLEEP_MODE)
        vTaskDelay(200 / portTICK_PERIOD_MS);
}

void enter_light_sleep(void)
{
    ESP_LOGI(TAG, "[APP] Light Sleep Mode Entered for %d seconds", sleepLength / 1000);
    mc_state_set_state(LIGHT_SLEEP_MODE);
    vTaskDelay(sleepLength / portTICK_PERIOD_MS);
}

void set_low_power_mode(void)
{
    mc_uv_set_powermode(0);
    mc_accelerometer_set_powermode(0);
    // mc_wifi_disconnect();
}

void set_high_power_mode(void)
{
    mc_uv_set_powermode(1);
    mc_accelerometer_set_powermode(1);
    mc_wifi_connect();

    vTaskDelay(30 / portTICK_PERIOD_MS); // delay to let the devices wake up
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

    mc_network_init();
    mc_time_init();
    mc_led_init(); // must be called before mc_state_init()
    mc_gpio_init();
    mc_tone_init();
    mc_voltage_init();
    mc_state_init();

    // TODO gpio set pin 5 (blue led) permanently LOW
}
