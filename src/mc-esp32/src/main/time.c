#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "apps/sntp/sntp.h"

#include "time.h"

#define TIME_UPDATE_DELAY 2000

static const char *TAG = "TIME";

static time_t now;
static struct tm timeinfo = {0};

static void init_sntp(void);
static void obtain_time(void);

// TODO: TIME SHOULD BE UPDATED VIA GPS DATA
// TODO make a function called get_timestamp()

void obtain_time(void)
{
    // wait for time to be set
    time_t newNow = 0;
    struct tm newTimeinfo = {0};
    int retry = 0;
    const int retry_count = 10;
    while (newTimeinfo.tm_year < (2018 - 1900) && retry++ < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(TIME_UPDATE_DELAY / portTICK_PERIOD_MS);
        time(&newNow);
        localtime_r(&newNow, &newTimeinfo);
    }

    // update local time
    if (newTimeinfo.tm_year >= (2018 - 1900))
    {
        now = newNow;
        timeinfo = newTimeinfo;
    }
}

void init_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

void update_time(void)
{
    obtain_time();

    char strftime_buf[64];

    // Set timezone to Eastern Standard Time and print local time
    setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in New York is: %s", strftime_buf);

    // Set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);
}

void mc_time_init(void)
{
    now = 0;
    init_sntp();
}