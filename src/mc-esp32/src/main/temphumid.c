/* Si7006 code rewritten to a C ESP-IDF implementation from https://github.com/automote/Si7006/blob/master/Si7006.cpp
Si7006 Temperature and humidity sensor library for Arduino
Lovelesh, thingTronics

The MIT License (MIT)

Copyright (c) 2015 thingTronics Limited

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

version 0.1
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ESP-IDF I2C DRIVER
// Note: The internally defined macro I2C_MASTER_TOUT_CNUM_DEFAULT, located in driver/i2c.c,
// must be reassigned to the value of "1200" (without quotes) in order for proper functionality.
#include "driver/i2c.h"

#include "i2c.h"
#include "temphumid.h"

// Si7006 register addresses
#define Si7006_MEAS_REL_HUMIDITY_MASTER_MODE 0xE5
#define Si7006_MEAS_REL_HUMIDITY_NO_MASTER_MODE 0xF5
#define Si7006_MEAS_TEMP_MASTER_MODE 0xE3
#define Si7006_MEAS_TEMP_NO_MASTER_MODE 0xF3
#define Si7006_READ_OLD_TEMP 0xE0
#define Si7006_RESET 0xFE
#define Si7006_WRITE_HUMIDITY_TEMP_CONTR 0xE6
#define Si7006_READ_HUMIDITY_TEMP_CONTR 0xE7
#define Si7006_WRITE_HEATER_CONTR 0x51
#define Si7006_READ_HEATER_CONTR 0x11
#define Si7006_READ_ID_LOW_0 0xFA
#define Si7006_READ_ID_LOW_1 0x0F
#define Si7006_READ_ID_HIGH_0 0xFC
#define Si7006_READ_ID_HIGH_1 0xC9
#define Si7006_FIRMWARE_0 0x84
#define Si7006_FIRMWARE_1 0xB8

static uint8_t sensorAddress;
static uint8_t error;

static bool get_temperature(float *temperature, bool mode);
static bool get_humidity(float *humidity, bool mode);
static bool read_byte(uint8_t address, uint8_t *value);
static bool write_byte(uint8_t address, uint8_t value);
static bool read_uint(uint8_t address, unsigned int *value);
static bool read_1_byte_data(uint8_t address1, uint8_t address2, uint8_t *value);
static bool read_4_byte_data(uint8_t address1, uint8_t address2, uint8_t value[4]);

bool mc_temphumid_init(void)
{
    sensorAddress = Si7006_ADDR;
    mc_temphumid_reset();
    return true;
}

bool mc_temphumid_reset(void)
{
    uint8_t res = 0x01;
    bool heater = false;

    if (!mc_temphumid_set_temp_control(res, heater))
        return false;

    return true;
}

bool mc_temphumid_get_temp_control(uint8_t *res, bool voltage, bool heater)
{
    uint8_t control, humidity;

    if (!read_byte(Si7006_READ_HUMIDITY_TEMP_CONTR, &control))
        return false;

    *res = ((control & 0x80) >> 6) | (control & 0x01);
    voltage = (control & 0x40) ? true : false;
    heater = (control & 0x04) ? true : false;

    if (heater || voltage || humidity)
    {
        // Note: This is reserved for future implementation
    }

    return true;
}

bool mc_temphumid_set_temp_control(uint8_t res, bool heater)
{
    uint8_t control = 0;

    res = (res > 4) ? 0 : res;

    control |= (res & 0x02) << 6 | (res & 0x01);

    if (heater)
    {
        control |= 0x04;
    }

    return write_byte(Si7006_WRITE_HUMIDITY_TEMP_CONTR, control);
}

bool mc_temphumid_get_heater_control(uint8_t *heaterCurrent)
{
    if (!read_byte(Si7006_READ_HEATER_CONTR, heaterCurrent))
        return false;

    *heaterCurrent |= *heaterCurrent & 0x0F;
    return true;
}

bool mc_temphumid_set_heater_control(uint8_t heaterCurrent)
{
    heaterCurrent = (heaterCurrent > 15) ? 15 : heaterCurrent;
    return write_byte(Si7006_WRITE_HEATER_CONTR, heaterCurrent);
}

bool mc_temphumid_get_device_id(char deviceId[8])
{
    uint8_t tempDeviceId[4];

    if (!read_4_byte_data(Si7006_READ_ID_LOW_0, Si7006_READ_ID_LOW_1, tempDeviceId))
        return false;

    strcpy(deviceId, (char *)tempDeviceId);

    if (!read_4_byte_data(Si7006_READ_ID_HIGH_0, Si7006_READ_ID_HIGH_1, tempDeviceId))
        return false;

    strcat(deviceId, (char *)tempDeviceId);
    return true;
}

bool mc_temphumid_get_firmware_vers(uint8_t *firmware)
{
    return read_1_byte_data(Si7006_FIRMWARE_0, Si7006_FIRMWARE_1, firmware);
}

bool mc_temphumid_get_temperature(float *temperature)
{
    return get_temperature(temperature, true);
}

bool get_temperature(float *temperature, bool mode)
{
    unsigned int tempTemperature;

    // Only Hold master mode is implemented
    if (mode)
    {
        if (!read_uint(Si7006_MEAS_TEMP_MASTER_MODE, &tempTemperature))
            return false;
    }
    else
    {
        // This will not work!
        if (!read_uint(Si7006_MEAS_TEMP_NO_MASTER_MODE, &tempTemperature))
            return false;
    }

    if (!(tempTemperature & 0xFFFC))
        return false;

    *temperature = (175.72 * (float)tempTemperature) / 65536 - 46.85;
    return true;
}

bool mc_temphumid_get_humidity(float *humidity)
{
    return get_humidity(humidity, true);
}

bool get_humidity(float *humidity, bool mode)
{
    unsigned int tempHumidity;

    // Only Hold master mode is implemented
    if (mode)
    {
        if (!read_uint(Si7006_MEAS_REL_HUMIDITY_MASTER_MODE, &tempHumidity))
            return false;
    }
    else
    {
        // This will not work!
        if (!read_uint(Si7006_MEAS_REL_HUMIDITY_NO_MASTER_MODE, &tempHumidity))
            return false;
    }

    if (!(tempHumidity & 0xFFFE))
        return false;

    *humidity = (125 * (float)tempHumidity) / 65536 - 6;
    return true;
}

uint8_t mc_temphumid_get_error(void)
{
    return error;
}

bool read_byte(uint8_t address, uint8_t *value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (sensorAddress << 1) | READ_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address << 1), ACK_CHECK_EN));

    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, value, NACK_VAL));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    if (ret)
    {
        if (ret == ESP_ERR_INVALID_ARG)
        {
            printf("5ESP_ERR_INVALID_ARG  ");
        }
        if (ret == ESP_FAIL)
        {
            printf("5ESP_FAIL  ");
        }
        if (ret == ESP_ERR_INVALID_STATE)
        {
            printf("5ESP_ERR_INVALID_STATE  ");
        }
        if (ret == ESP_ERR_TIMEOUT)
        {
            printf("5ESP_ERR_TIMEOUT  ");
        }
        printf("readbRET= %d\n", ret);

        i2c_cmd_link_delete(cmd);
        return false;
    }

    i2c_cmd_link_delete(cmd);
    return true;
}

bool write_byte(uint8_t address, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (sensorAddress << 1) | WRITE_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, address, ACK_CHECK_EN));

    ESP_ERROR_CHECK(i2c_master_write(cmd, &value, 1, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    if (ret)
    {
        if (ret == ESP_ERR_INVALID_ARG)
        {
            printf("3ESP_ERR_INVALID_ARG  ");
        }
        if (ret == ESP_FAIL)
        {
            printf("3ESP_FAIL  ");
        }
        if (ret == ESP_ERR_INVALID_STATE)
        {
            printf("3ESP_ERR_INVALID_STATE  ");
        }
        if (ret == ESP_ERR_TIMEOUT)
        {
            printf("3ESP_ERR_TIMEOUT  ");
        }
        printf("writebRET= %d\n", ret);

        i2c_cmd_link_delete(cmd);
        return false;
    }

    i2c_cmd_link_delete(cmd);
    return true;
}

bool read_uint(uint8_t address, unsigned int *value)
{
    uint8_t high, low;

    // Hold master mode implementation
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (sensorAddress << 1) | WRITE_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address << 0), ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (sensorAddress << 1) | READ_BIT, ACK_CHECK_EN));

    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &high, ACK_VAL));
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &low, NACK_VAL));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    if (ret)
    {
        if (ret == ESP_ERR_INVALID_ARG)
        {
            printf("2ESP_ERR_INVALID_ARG  ");
        }
        if (ret == ESP_FAIL)
        {
            printf("2ESP_FAIL  ");
        }
        if (ret == ESP_ERR_INVALID_STATE)
        {
            printf("2ESP_ERR_INVALID_STATE  ");
        }
        if (ret == ESP_ERR_TIMEOUT)
        {
            printf("2ESP_ERR_TIMEOUT  ");
        }
        printf("uintRET= %d\n", ret);

        i2c_cmd_link_delete(cmd);
        return false;
    }

    i2c_cmd_link_delete(cmd);
    *value = (high << 8) | low;
    return true;
}

bool read_1_byte_data(uint8_t address1, uint8_t address2, uint8_t *value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (sensorAddress << 1) | WRITE_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address1 << 0), ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address2 << 0), ACK_CHECK_EN));

    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (sensorAddress << 1) | READ_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, value, NACK_VAL));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    if (ret)
    {
        if (ret == ESP_ERR_INVALID_ARG)
        {
            printf("1ESP_ERR_INVALID_ARG  ");
        }
        if (ret == ESP_FAIL)
        {
            printf("1ESP_FAIL  ");
        }
        if (ret == ESP_ERR_INVALID_STATE)
        {
            printf("1ESP_ERR_INVALID_STATE  ");
        }
        if (ret == ESP_ERR_TIMEOUT)
        {
            printf("1ESP_ERR_TIMEOUT  ");
        }
        printf("1byteRET= %d\n", ret);

        i2c_cmd_link_delete(cmd);
        return false;
    }

    i2c_cmd_link_delete(cmd);
    return true;
}

/* Unimplemented! */
bool read_4_byte_data(uint8_t address1, uint8_t address2, uint8_t value[4])
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (sensorAddress << 1) | READ_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address1 << 1), ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address2 << 1), ACK_CHECK_EN));

    ESP_ERROR_CHECK(i2c_master_read(cmd, value, 3, ACK_VAL));
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, value + 3, NACK_VAL));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    if (ret)
    {
        if (ret == ESP_ERR_INVALID_ARG)
        {
            printf("4ESP_ERR_INVALID_ARG  ");
        }
        if (ret == ESP_FAIL)
        {
            printf("4ESP_FAIL  ");
        }
        if (ret == ESP_ERR_INVALID_STATE)
        {
            printf("4ESP_ERR_INVALID_STATE  ");
        }
        if (ret == ESP_ERR_TIMEOUT)
        {
            printf("4ESP_ERR_TIMEOUT  ");
        }
        printf("4byteRET= %d\n", ret);

        i2c_cmd_link_delete(cmd);
        return false;
    }

    i2c_cmd_link_delete(cmd);
    return true;
}

// void app_main()
// {
//     mc_i2c_init();
//     mc_temphumid_init();

//     uint8_t firmvers;
//     float temp;
//     float humidity;

//     mc_temphumid_get_firmware_vers(&firmvers);
//     mc_temphumid_get_temperature(&temp, true);
//     mc_temphumid_get_humidity(&humidity, true);

//     printf("firmware revision: %d\n", firmvers);
//     printf("temperature: %f\n", temp);
//     printf("humidity: %f\n", humidity);
// }
