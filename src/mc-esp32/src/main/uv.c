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

#include "driver/i2c.h"

#include "i2c.h"
#include "uv.h"

// Reading the application note on calculation of UV index, the "dummy" channel
// value is actually not a dummy value at all, but the dark current count.
// NAMES ARE IMPORTANT PEOPLE.

#define VEML6075_REG_CONF (0x00)    // Configuration register (options below)
#define VEML6075_REG_UVA (0x07)     // UVA register
#define VEML6075_REG_DUMMY (0x08)   // Dark current register (NOT DUMMY)
#define VEML6075_REG_UVB (0x09)     // UVB register
#define VEML6075_REG_UVCOMP1 (0x0A) // Visible compensation register
#define VEML6075_REG_UVCOMP2 (0x0B) // IR compensation register
#define VEML6075_REG_DEVID (0x0C)   // Device ID register

#define VEML6075_CONF_IT_50MS (0x00)  // Integration time = 50ms (default)
#define VEML6075_CONF_IT_100MS (0x10) // Integration time = 100ms
#define VEML6075_CONF_IT_200MS (0x20) // Integration time = 200ms
#define VEML6075_CONF_IT_400MS (0x30) // Integration time = 400ms
#define VEML6075_CONF_IT_800MS (0x40) // Integration time = 800ms
#define VEML6075_CONF_IT_MASK (0x8F)  // Mask off other config bits

#define VEML6075_CONF_HD_NORM (0x00) // Normal dynamic seetting (default)
#define VEML6075_CONF_HD_HIGH (0x08) // High dynamic seetting

#define VEML6075_CONF_TRIG (0x04) // Trigger measurement, clears by itself

#define VEML6075_CONF_AF_OFF (0x00) // Active force mode disabled (default)
#define VEML6075_CONF_AF_ON (0x02)  // Active force mode enabled (?)

#define VEML6075_CONF_SD_OFF (0x00) // Power up
#define VEML6075_CONF_SD_ON (0x01)  // Power down

// To calculate the UV Index, a bunch of empirical/magical coefficients need to
// be applied to UVA and UVB readings to get a proper composite index value.
// Seems pretty hand wavey, though not nearly as annoying as the dark current
// not being subtracted out by default.

#define VEML6075_UVI_UVA_VIS_COEFF (3.33)
#define VEML6075_UVI_UVA_IR_COEFF (2.5)
#define VEML6075_UVI_UVB_VIS_COEFF (3.66)
#define VEML6075_UVI_UVB_IR_COEFF (2.75)

// Once the above offsets and crunching is done, there's a last weighting
// function to convert the ADC counts into the UV index values. This handles
// both the conversion into irradiance (W/m^2) and the skin erythema weighting
// by wavelength--UVB is way more dangerous than UVA, due to shorter
// wavelengths and thus more energy per photon. These values convert the compensated values

#define VEML6075_UVI_UVA_RESPONSE (1.0 / 909.0)
#define VEML6075_UVI_UVB_RESPONSE (1.0 / 800.0)

#define OUTSIDE_MIN_UVAB_THRESHOLD 150

enum veml6075_int_time
{
    VEML6075_IT_50MS,
    VEML6075_IT_100MS,
    VEML6075_IT_200MS,
    VEML6075_IT_400MS,
    VEML6075_IT_800MS
};
typedef enum veml6075_int_time veml6075_int_time_t;

static uint8_t config;
static uint16_t raw_uva;
static uint16_t raw_uvb;
static uint16_t raw_dark;
static uint16_t raw_vis;
static uint16_t raw_ir;

static void init(void);
static bool begin(void);
static bool set_config(void);
static void create_config(uint8_t powerMode);
static bool write_16(uint8_t reg, uint16_t data);
static uint16_t read_16(uint8_t reg);

void mc_uv_poll(void)
{
    raw_uva = read_16(VEML6075_REG_UVA);
    raw_uvb = read_16(VEML6075_REG_UVB);
    raw_dark = read_16(VEML6075_REG_DUMMY);
    raw_vis = read_16(VEML6075_REG_UVCOMP1);
    raw_ir = read_16(VEML6075_REG_UVCOMP2);
}

float mc_uv_get_uva(void)
{
    float comp_vis = raw_vis - raw_dark;
    float comp_ir = raw_ir - raw_dark;
    float comp_uva = raw_uva - raw_dark;

    comp_uva -= (VEML6075_UVI_UVA_VIS_COEFF * comp_vis) - (VEML6075_UVI_UVA_IR_COEFF * comp_ir);

    return comp_uva;
}

float mc_uv_get_uvb(void)
{
    float comp_vis = raw_vis - raw_dark;
    float comp_ir = raw_ir - raw_dark;
    float comp_uvb = raw_uvb - raw_dark;

    comp_uvb -= (VEML6075_UVI_UVB_VIS_COEFF * comp_vis) - (VEML6075_UVI_UVB_IR_COEFF * comp_ir);

    return comp_uvb;
}

float mc_uv_get_uv_index(void)
{
    float uva_weighted = mc_uv_get_uva() * VEML6075_UVI_UVA_RESPONSE;
    float uvb_weighted = mc_uv_get_uvb() * VEML6075_UVI_UVB_RESPONSE;

    return (uva_weighted + uvb_weighted) / 2.0;
}

uint16_t mc_uv_get_device_id(void)
{
    return read_16(VEML6075_REG_DEVID);
}

bool mc_uv_set_powermode(uint8_t powerMode)
{
    create_config(powerMode);
    return set_config();
}

bool mc_uv_check_outside(void)
{
    return (mc_uv_get_uva() > OUTSIDE_MIN_UVAB_THRESHOLD) && (mc_uv_get_uvb() > OUTSIDE_MIN_UVAB_THRESHOLD);
}

bool set_config(void)
{
    return write_16(VEML6075_REG_CONF, config);
}

void create_config(uint8_t powerMode)
{
    config = 0;

    if (powerMode)
        config |= VEML6075_CONF_SD_OFF;
    else
        config |= VEML6075_CONF_SD_ON;

    config |= VEML6075_CONF_IT_100MS;
}

void init(void)
{
    create_config(1);
}

bool begin(void)
{
    if (mc_uv_get_device_id() != VEML6075_DEVID)
        return false;

    return set_config();
}

bool write_16(uint8_t reg, uint16_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, ((uint8_t)VEML6075_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xFF & data, ACK_CHECK_EN));        // LSB
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0xFF & (data >> 8), ACK_CHECK_EN)); // MSB
    ESP_ERROR_CHECK(i2c_master_stop(cmd));

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    if (ret)
    {
        if (ret == ESP_ERR_INVALID_ARG)
        {
            printf("ESP_ERR_INVALID_ARG  ");
        }
        if (ret == ESP_FAIL)
        {
            printf("ESP_FAIL  ");
        }
        if (ret == ESP_ERR_INVALID_STATE)
        {
            printf("ESP_ERR_INVALID_STATE  ");
        }
        if (ret == ESP_ERR_TIMEOUT)
        {
            printf("ESP_ERR_TIMEOUT  ");
        }
        printf("uv_write16_ret= %d\n", ret);

        i2c_cmd_link_delete(cmd);
        return false;
    }

    i2c_cmd_link_delete(cmd);
    return true;
}

uint16_t read_16(uint8_t reg)
{
    uint8_t msb = 0;
    uint8_t lsb = 0;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, ((uint8_t)VEML6075_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, ((uint8_t)VEML6075_ADDR << 1) | READ_BIT, ACK_CHECK_EN));

    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &lsb, ACK_VAL));
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &msb, ACK_VAL));

    ESP_ERROR_CHECK(i2c_master_stop(cmd));

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    if (ret)
    {
        if (ret == ESP_ERR_INVALID_ARG)
        {
            printf("ESP_ERR_INVALID_ARG  ");
        }
        if (ret == ESP_FAIL)
        {
            printf("ESP_FAIL  ");
        }
        if (ret == ESP_ERR_INVALID_STATE)
        {
            printf("ESP_ERR_INVALID_STATE  ");
        }
        if (ret == ESP_ERR_TIMEOUT)
        {
            printf("ESP_ERR_TIMEOUT  ");
        }
        printf("uv_read16_ret= %d\n", ret);

        i2c_cmd_link_delete(cmd);
        return 0;
    }

    i2c_cmd_link_delete(cmd);
    return (msb << 8) | lsb;
}

bool mc_uv_init(void)
{
    init();
    begin();
    return true;
}

// void app_main()
// {
//     mc_i2c_init();
//     mc_uv_init();

//     mc_uv_poll();
//     uint16_t deviceId = mc_uv_get_device_id();
//     float uva = mc_uv_get_uva();
//     float uvb = mc_uv_get_uvb();
//     float index = mc_uv_get_uv_index();

// #include <inttypes.h>
//     printf("device id: %" PRIu16 "\n", deviceId);
//     printf("uva: %f\n", uva);
//     printf("uvb: %f\n", uvb);
//     printf("index: %f\n", index);
// }
