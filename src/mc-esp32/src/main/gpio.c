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
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "gpio.h"
#include "state.h"

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

#define GPIO_INPUT_IO_0 15
#define GPIO_INPUT_IO_1 17
#define GPIO_INPUT_PIN_SEL (1ULL << GPIO_INPUT_IO_0 | (1ULL << GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0
#define DEBOUNCE_THRESHOLD 70

static xQueueHandle gpio_evt_queue = NULL;
static TickType_t xLastTrigger;

static void gpio_deep_sleep_task(void *arg);

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    if (xTaskGetTickCount() - xLastTrigger > DEBOUNCE_THRESHOLD)
    {
        xLastTrigger = xTaskGetTickCount();

        uint32_t gpio_num = (uint32_t)arg;
        xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    }
}

static void gpio_deep_sleep_task(void *arg)
{
    uint32_t io_num;
    while (1)
    {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            mc_state_toggle_deep();
        }
    }
}

int mc_gpio_test_enabled(void)
{
    return gpio_get_level(GPIO_INPUT_IO_1);
}

void mc_gpio_init()
{
    gpio_config_t io_conf;

    //interrupt of falling edge
    io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    // TEST SWITCH
    gpio_set_intr_type(GPIO_INPUT_IO_1, GPIO_INTR_DISABLE);

    //start gpio task
    xTaskCreate(gpio_deep_sleep_task, "gpio_deep_sleep_task", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void *)GPIO_INPUT_IO_0);
}
