#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
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
#define TIME_STRING_FORMAT "%Y-%m-%dT%H-%M-%SZ" // E.g. 2018-05-22T13-02-02Z
#define TIME_STRING_MAX_SIZE 21                 // E.g. 2018-05-22T13-02-02Z = 20 chars + null

static const char *TAG = "TIME";

static void update_system_time(struct tm newTime);

// Returns a malloc'd timestamp of a form conforming to ISO 8601
// E.g. 20180319T105701Z
char *mc_time_get_timestamp(void)
{
    struct timeval currentSysTime = {0};
    struct tm currentTime = {0};

    gettimeofday(&currentSysTime, NULL);
    localtime_r(&currentSysTime.tv_sec, &currentTime);

    char *result = malloc(TIME_STRING_MAX_SIZE);
    strftime(result, TIME_STRING_MAX_SIZE, TIME_STRING_FORMAT, &currentTime);

    return result;
}

bool mc_time_set_time_str(char *newTimeString)
{
    struct tm newTime = {0};
    strptime(newTimeString, TIME_STRING_FORMAT, &newTime);

    update_system_time(newTime);
    return true;
}

void update_system_time(struct tm newTime)
{
    struct timeval newSysTime = {0};
    newSysTime.tv_sec = mktime(&newTime);
    settimeofday(&newSysTime, NULL);
    ESP_LOGI(TAG, "Updated current system time.");
}

void mc_time_init(void)
{
    char *newTime = "2018-05-28T10-00-00Z";
    mc_time_set_time_str(newTime);
}