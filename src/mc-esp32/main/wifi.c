#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_pm.h"
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

#include "wifi.h"

/*set the ssid and password via "make menuconfig"*/
#define DEFAULT_SSID CONFIG_WIFI_SSID
#define DEFAULT_PWD CONFIG_WIFI_PASSWORD

#if CONFIG_POWER_SAVE_MIN_MODEM
#define DEFAULT_PS_MODE WIFI_PS_MIN_MODEM
#elif CONFIG_POWER_SAVE_MAX_MODEM
#define DEFAULT_PS_MODE WIFI_PS_MAX_MODEM
#elif CONFIG_POWER_SAVE_NONE
#define DEFAULT_PS_MODE WIFI_PS_NONE
#else
#define DEFAULT_PS_MODE WIFI_PS_NONE
#endif /*CONFIG_POWER_SAVE_MODEM*/

#define AP_SCAN_DELIMETER   "%"

static const char *TAG = "WIFI"; // logging tag

struct AP_Scan scanArray[20];
int nScans;     // extern

wifi_scan_config_t scanConf = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
};

static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event);

esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            ESP_LOGI(TAG, "got ip:%s\n", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            // Change this to prevent auto-reassociate
            ESP_ERROR_CHECK(esp_wifi_connect());
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_SCAN_DONE: ;
            uint16_t apCount = 0;
            esp_wifi_scan_get_ap_num(&apCount);
            printf("Number of access points found: %d\n", event->event_info.scan_done.number);
            nScans = apCount;
            if (apCount == 0)
            {
                return ESP_OK;
            }
            wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
            ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, list));
            int i;
            printf("======================================================================\n");
            printf("             SSID             |    RSSI    |           BSSID          \n");
            printf("======================================================================\n");
            for (i = 0; i < apCount; i++)
            {
                // TODO: make this better obviously, remove hard limit of 40 etc
                if (i < 40) {
                    strcpy(scanArray[i].ssid, (const char*)list[i].ssid);
                    sprintf(scanArray[i].bssid, "%02X %02X %02X %02X %02X %02X", 
                        (uint8_t)list[i].bssid[0],
                        (uint8_t)list[i].bssid[1],
                        (uint8_t)list[i].bssid[2],
                        (uint8_t)list[i].bssid[3],
                        (uint8_t)list[i].bssid[4],
                        (uint8_t)list[i].bssid[5]);
                    sprintf(scanArray[i].rssi, "%d", list[i].rssi);
                }

                // printf("%26.26s    |    % 4d    |    %22.22s\n", list[i].ssid, list[i].rssi, authmode);
                printf("%26.26s    |    % 4d    |    %s\n", list[i].ssid, list[i].rssi, scanArray[i].bssid);
            }

            free(list);
            printf("\n\n");

            break;
        default:
            break;
    }

    return ESP_OK;
}

/*init wifi as sta and set power save mode*/
void mc_wifi_init(void) {
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = DEFAULT_SSID,
            .password = DEFAULT_PWD
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    ESP_LOGI(TAG, "esp_wifi_set_ps().");
    esp_wifi_set_ps(DEFAULT_PS_MODE);

    // ESP_LOGI(TAG, "start the WIFI SSID:[%s] password:[%s]", CONFIG_WIFI_SSID, "******");
    // ESP_ERROR_CHECK(esp_wifi_start());
    
    // ESP_LOGI(TAG, "Waiting for wifi");
    // xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, 10000 / portMAX_DELAY);
}

char *mc_wifi_scan(void) {
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, true)); //The true parameter cause the function to block until
    char *result = NULL;
    char sendpacket[12][sizeof(struct AP_Scan)];
    
    if (nScans) {
        uint16_t apScanLength = 0;
        
        for (uint8_t i = 0; i < nScans; i++) {
            char packet[sizeof(struct AP_Scan)];
            sprintf(packet, "%s;%s;%s", (char*)scanArray[i].ssid, (char*)scanArray[i].bssid, (char*)scanArray[i].rssi);
            apScanLength += strlen(packet);
            strcpy(sendpacket[i], packet);
        }

        uint16_t fullLength = apScanLength + nScans-1;
        result = (char *) malloc(fullLength + 1); // should check for null maybe
        strncpy(result, sendpacket[0], apScanLength);
        for (uint8_t i = 1; i < nScans; i++) {
            strncat(result, AP_SCAN_DELIMETER, fullLength - strlen(result));
            strncat(result, sendpacket[i], fullLength - strlen(result));
        }
    }

    return result;
}

uint8_t mc_wifi_status(void) {
    return xEventGroupGetBits(wifi_event_group) & CONNECTED_BIT;
}


