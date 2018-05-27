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

#include "mqtt.h"

static const char *MQTT_TOPIC = "/engg4810_2018/G03";

static const char *TAG = "MQTT"; // logging tag

static EventGroupHandle_t mqtt_event_group;
const static int CONNECTED_BIT = BIT0;

esp_mqtt_client_handle_t mqtt_client;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);
static void mqtt_app_start(void);

esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(mqtt_event_group, CONNECTED_BIT);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        xEventGroupClearBits(mqtt_event_group, CONNECTED_BIT);
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    }

    return ESP_OK;
}

void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://tp-mqtt.zones.eait.uq.edu.au",
        .username = "engg4810_2018",
        .password = "blpc7n2DYExpBGY5BP7",
        .event_handle = mqtt_event_handler};

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(mqtt_client);
}

int mc_mqtt_publish(const char *data)
{
    if (!mc_mqtt_check_connected())
        return -1;
    int msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC, data, 0, 0, 0);
    ESP_LOGI(TAG, "sent publish attempted, msg_id=%d", msg_id);
    return msg_id;
}

uint8_t mc_mqtt_check_connected(void)
{
    return xEventGroupGetBits(mqtt_event_group) & CONNECTED_BIT;
}

void mc_mqtt_init(void)
{
    mqtt_event_group = xEventGroupCreate();

    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    mqtt_app_start();
}
