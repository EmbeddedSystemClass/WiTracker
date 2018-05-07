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

    wifi_scan_config_t scanConf = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
    };

    // char packet[sizeof(struct AP_Scan) * 50];
    // char strings[60][20];
    char sendpacket[12][sizeof(struct AP_Scan)];

    while (1)
    {
        char *scanString = mc_wifi_scan();

        //mc_mqtt_publish("Hello, how's it going?");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void hardware_init(void)
{
    nvs_flash_init();

    mc_wifi_init();
    scan_init();
    mc_mqtt_init();
}
