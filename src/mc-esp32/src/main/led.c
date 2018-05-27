/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

typedef enum
{
    ACTIVE_MODE,
    LIGHT_SLEEP_MODE,
    DEEP_SLEEP_MODE
} Device_Mode;

/**
 * Brief:
 * This test code shows how to configure gpio and how to use gpio interrupt.
 *
 * GPIO status:
 * GPIO18: output
 * GPIO19: output
 * GPIO4:  input, pulled up, interrupt from rising edge and falling edge
 * GPIO5:  input, pulled up, interrupt from rising edge.
 *
 * Test:
 * Connect GPIO18 with GPIO4
 * Connect GPIO19 with GPIO5
 * Generate pulses on GPIO18/19, that triggers interrupt on GPIO4/5
 *
 */

#define GPIO_OUTPUT_RED_LED 12
#define GPIO_OUTPUT_GREEN_LED 13
#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_RED_LED) | (1ULL << GPIO_OUTPUT_GREEN_LED))

#define NUMBER_OF_DEVICE_MODES 3

SemaphoreHandle_t GreenActiveOn;
SemaphoreHandle_t GreenActiveOff;
SemaphoreHandle_t RedDeepSleepOn;
SemaphoreHandle_t RedDeepSleepOff;

static void green_led_blink_task(void *arg)
{
    int cnt = 0;
    int active_mode = 0;

    while (1)
    {
        if (GreenActiveOn != NULL)
        {
            if (xSemaphoreTake(GreenActiveOn, 0) == pdTRUE)
            {
                active_mode = 1;
            }
        }
        if (GreenActiveOff != NULL)
        {
            if (xSemaphoreTake(GreenActiveOff, 0) == pdTRUE)
            {
                active_mode = 0;
            }
        }
        if (active_mode)
        {
            printf("GREEN cnt: %d\n", cnt++);

            gpio_set_level(GPIO_OUTPUT_GREEN_LED, cnt % 2);
        }
        else
        {
            gpio_set_level(GPIO_OUTPUT_GREEN_LED, 0);
        }
        vTaskDelay(125 / portTICK_RATE_MS);
    }
}

static void red_led_blink_task(void *arg)
{
    int deep_sleep = 0;

    while (1)
    {
        if (RedDeepSleepOn != NULL)
        {
            if (xSemaphoreTake(RedDeepSleepOn, 0) == pdTRUE)
            {
                deep_sleep = 1;
            }
        }
        if (RedDeepSleepOff != NULL)
        {
            if (xSemaphoreTake(RedDeepSleepOff, 0) == pdTRUE)
            {
                deep_sleep = 0;
            }
        }
        if (deep_sleep)
        {
            gpio_set_level(GPIO_OUTPUT_RED_LED, 1);
            vTaskDelay(100 / portTICK_RATE_MS);
            gpio_set_level(GPIO_OUTPUT_RED_LED, 0);
            vTaskDelay(300 / portTICK_RATE_MS);
        }
        else
        {
            gpio_set_level(GPIO_OUTPUT_RED_LED, 1);
            vTaskDelay(100 / portTICK_RATE_MS);
            gpio_set_level(GPIO_OUTPUT_RED_LED, 0);
            vTaskDelay(100 / portTICK_RATE_MS);
            gpio_set_level(GPIO_OUTPUT_RED_LED, 1);
            vTaskDelay(100 / portTICK_RATE_MS);
            gpio_set_level(GPIO_OUTPUT_RED_LED, 0);
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        vTaskDelay(9600 / portTICK_RATE_MS);
    }
}

// TODO maybe rename this file to gpio.c instead of creating a new one?
// void app_main()
// {
//     gpio_config_t io_conf;
//     //disable interrupt
//     io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
//     //set as output mode
//     io_conf.mode = GPIO_MODE_OUTPUT;
//     //bit mask of the pins that you want to set,e.g.GPIO18/19
//     io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
//     //disable pull-down mode
//     io_conf.pull_down_en = 0;
//     //disable pull-up mode
//     io_conf.pull_up_en = 0;
//     //configure GPIO with the given settings
//     gpio_config(&io_conf);

//     //start gpio task
//     xTaskCreate(red_led_blink, "red_led_blink", 2048, NULL, 10, NULL);
//     xTaskCreate(green_led_blink, "green_led_blink", 2048, NULL, 10, NULL);

//     GreenActiveOn = xSemaphoreCreateBinary();
//     GreenActiveOff = xSemaphoreCreateBinary();
//     RedDeepSleepOn = xSemaphoreCreateBinary();
//     RedDeepSleepOff = xSemaphoreCreateBinary();

//     int mode = ACTIVE_MODE;
//     while (1)
//     {
//         switch (mode)
//         {
//         case ACTIVE_MODE:
//             xSemaphoreGive(GreenActiveOn);
//             xSemaphoreGive(RedDeepSleepOff);
//             break;
//         case LIGHT_SLEEP_MODE:
//             xSemaphoreGive(GreenActiveOff);
//             xSemaphoreGive(RedDeepSleepOff);
//             break;
//         case DEEP_SLEEP_MODE:
//             xSemaphoreGive(GreenActiveOff);
//             xSemaphoreGive(RedDeepSleepOn);
//             break;
//         }

//         mode = (mode + 1) % NUMBER_OF_DEVICE_MODES;
//         vTaskDelay(12000 / portTICK_RATE_MS);
//     }
// }
