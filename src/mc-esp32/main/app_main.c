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

static void hardware_init(void);

void app_main(void)
{
    hardware_init();

    while (1) {
        char *scanString = mc_wifi_scan();
        if (scanString == NULL) {
            vTaskDelay(10000 / portTICK_PERIOD_MS);
            continue;
        }

        printf("%s\n", scanString);

        if (mc_mqtt_publish(scanString) == -1) {
            printf("ERROR: MQTT COULDNT CONNECT!\n");
        }

        free(scanString);
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void hardware_init(void)
{
    nvs_flash_init();

    mc_wifi_init();
    mc_mqtt_init();
}
