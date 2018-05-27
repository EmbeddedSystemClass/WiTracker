/* VEML6075 code rewritten to a C ESP-IDF implementation from https://github.com/schizobovine/VEML6075
 * VEML6075.h
 *
 * Arduino library for the Vishay VEML6075 UVA/UVB i2c sensor.
 *
 * Author: Sean Caulfield <sean@yak.net>
 * License: GPLv2.0
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "state.h"
#include "led.h"

static Device_Mode mode;

bool mc_state_init(void)
{
    mode = ACTIVE_MODE;
    mc_state_apply();

    return true;
}

void mc_state_set_state(uint8_t newMode)
{
    mode = newMode;
    mc_state_apply();
}

uint8_t mc_state_get_state(void)
{
    return mode;
}

void mc_state_toggle_deep(void)
{
    mode = (mode != DEEP_SLEEP_MODE) ? DEEP_SLEEP_MODE : ACTIVE_MODE;
    mc_state_apply();
}

void mc_state_apply(void)
{
    mc_led_set_state(mode);
}
