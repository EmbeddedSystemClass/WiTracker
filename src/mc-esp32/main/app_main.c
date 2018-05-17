#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
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
#include "mqtt_client.h"

#include "mqtt.h"
#include "wifi.h"
#include "time.h"

typedef struct {
    char* wifiScanResult;
} Device_Data;

#define SLEEP_MODE_LIGHT_LENGTH 30000

static const char *TAG = "MAIN_APP"; // logging tag

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */
RTC_DATA_ATTR static int bootCount = 0;

static Device_Data data;  // This should be a custom struct type

static void program_init(void);
void collection_preprocessor(void);
void collect_data(void);
void handle_data(void);
void enter_sleep(void);

void app_main(void)
{
    program_init();

    while (1) {
        collection_preprocessor();
        
        // Active scan mode
        collect_data();
        update_time();
        handle_data();

        // Enter light/deep sleep mode
        enter_sleep();
    }
}

// TODO: add function comments
void collection_preprocessor(void) {
    // check if test switch is set
    // check if deep sleep switch is set
    printf("It's loop time!\n");
}

// Reads and stores results in data variables
void collect_data(void) {
    esp_wifi_start();
    data.wifiScanResult = mc_wifi_scan();
    // data.wifiScanResult = NULL;
}

// Processes and uploads data to the MQTT datastore
void handle_data(void) {
    if (data.wifiScanResult == NULL)
    {
        return;
    }

    if (mc_mqtt_publish(data.wifiScanResult) == -1)
    {
        printf("ERROR: MQTT COULDNT CONNECT!\n");
    }

    free(data.wifiScanResult);
}

// Enters an appropriate sleep mode
void enter_sleep(void) {
    printf("Entering light sleep for 10 seconds...\n");
    fflush(NULL);
    vTaskDelay(SLEEP_MODE_LIGHT_LENGTH / portTICK_PERIOD_MS);
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

    mc_mqtt_init();     // must be done after wifi init
    mc_time_init();
}
