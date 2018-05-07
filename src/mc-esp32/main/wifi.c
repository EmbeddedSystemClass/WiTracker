#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
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

#define AP_SCAN_DELIMETER   "%"

struct AP_Scan scanArray[20];
int nScans;     // extern

wifi_scan_config_t scanConf = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
};

static const char *TAG = "WIFI_SAMPLE";     // logging tag
static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event);

esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
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
                // char *authmode;
                // switch (list[i].authmode)
                // {
                // case WIFI_AUTH_OPEN:
                //     authmode = "WIFI_AUTH_OPEN";
                //     break;
                // case WIFI_AUTH_WEP:
                //     authmode = "WIFI_AUTH_WEP";
                //     break;
                // case WIFI_AUTH_WPA_PSK:
                //     authmode = "WIFI_AUTH_WPA_PSK";
                //     break;
                // case WIFI_AUTH_WPA2_PSK:
                //     authmode = "WIFI_AUTH_WPA2_PSK";
                //     break;
                // case WIFI_AUTH_WPA_WPA2_PSK:
                //     authmode = "WIFI_AUTH_WPA_WPA2_PSK";
                //     break;
                // default:
                //     authmode = "Unknown";
                //     break;
                // }
                
                // TODO: make this better obviously
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

void mc_wifi_init(void) {
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    
    ESP_LOGI(TAG, "start the WIFI SSID:[%s] password:[%s]", CONFIG_WIFI_SSID, "******");
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "Waiting for wifi");
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
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
            printf("~~~~FULL STRING IS:~~~~\n%s\n", packet);
            printf("apScanLength = %d\n", apScanLength);
            strcpy(sendpacket[i], packet);
        }

        // char *packet = join(strings, sizeof(strings)/sizeof(strings[0]));
        printf("length=%d\n", apScanLength);
        uint16_t fullLength = apScanLength + nScans-1;
        result = (char *) malloc(fullLength + 1); // should check for null maybe
        strncpy(result, sendpacket[0], apScanLength);
        for (uint8_t i = 1; i < nScans; i++) {
            strncat(result, AP_SCAN_DELIMETER, fullLength - strlen(result));
            strncat(result, sendpacket[i], fullLength - strlen(result));
        }
        printf("%s\nlength of result=%d\n", result, strlen(result));
        for (uint16_t i = 0; i <= strlen(result); i++) {
            printf("%d ", result[i]);
        }
    }

    return result;
}


