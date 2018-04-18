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
void scan_init(void);
char* join(char** in, size_t inlen);
char* join_helper(char** in, size_t inlen, size_t inpos, size_t accum);

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
    char strings[60][20];

    while (1)
    {
        nScans = 0;
        ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, true)); //The true parameter cause the function to block until

        if (nScans) {
            for (uint8_t i = 0; i < nScans; i++) {
                char packet[50];
                sprintf(packet, "%s;%s;%s ", (char*)scanArray[i].ssid, (char*)scanArray[i].bssid, (char*)scanArray[i].rssi);
                printf("~~~~FULL STRING IS:~~~~\n%s\n", packet);

                mc_mqtt_publish(packet);
                
                
                // sprintf(strings[i], "%s;%s;%s ", (char*)scanArray[i].ssid, (char*)scanArray[i].bssid, (char*)scanArray[i].rssi);
                
                // strcpy(ssids[i], scanArray[i].ssid);
                // strcpy(bssids[i], scanArray[i].bssid);
                // strcpy(rssis[i], scanArray[i].rssi);
                
                // printf("SSID: %s\nBSSID: %s\nRSSI: %d\n\n", scanArray[i].ssid, scanArray[i].bssid, scanArray[i].rssi);
                // sprintf(packet + offset, "%s;%s;%d", (char*)scanArray[i].ssid, (char*)scanArray[i].bssid, scanArray[i].rssi);
                // offset += strlen(scanArray[i].ssid) + strlen(scanArray[i].bssid) + (floor(log10(abs(scanArray[i].rssi))) + 2);
            }

            // char *packet = join(strings, sizeof(strings)/sizeof(strings[0]));


        }

        //mc_mqtt_publish("Hello, how's it going?");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void hardware_init(void)
{
    nvs_flash_init();

    mc_wifi_init();
    scan_init();
    mc_mqtt_init();
}

void scan_init(void)
{
    // system_init();
    // ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
}



char* join_helper(char** in, size_t inlen, size_t inpos, size_t accum) {
  if (inpos == inlen)
    return strcpy(malloc(accum + 1) + accum, "");
  else {
    size_t mylen = strlen(in[inpos]);
    return memcpy(
      join_helper(in, inlen, inpos + 1, accum + mylen) - mylen,
      in[inpos], mylen);
  }
}

// BORROWED FROM https://stackoverflow.com/a/29149454/4912373

/* The string returned by this function is malloc'd and needs
 * to be free'd by the caller
 */
char* join(char** in, size_t inlen) {
  return join_helper(in, inlen, 0, 0);
}
