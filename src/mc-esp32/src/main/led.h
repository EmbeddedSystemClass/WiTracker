#ifndef LED_H
#define LED_H

#include "stdint.h"

#define GPIO_OUTPUT_RED_LED 12
#define GPIO_OUTPUT_GREEN_LED 13
#define GPIO_OUTPUT_BLUE_LED 5

// Initialises the device's leds
void mc_led_init();
// Sets the state of the device's leds
void mc_led_set_state(uint8_t mode);

#endif /* LED_H */
