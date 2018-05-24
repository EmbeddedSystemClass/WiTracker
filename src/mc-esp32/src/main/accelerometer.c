/* LIS2DH12 code rewritten to a C ESP-IDF implementation from https://github.com/DFRobot/DFRobot_LIS2DH12
   @file DFRobot_LIS2DH12.h
   @brief DFRobot's Read LIS2DH12 data
   @n This example achieve receiving LIS2DH12  data via serial

   @copyright  [DFRobot](http://www.dfrobot.com), 2016
   @copyright GNU Lesser General Public License
   @author [Wuxiao](xiao.wu@dfrobot.com)
   @version  V1.0
   @date  2016-10-13
   @https://github.com/DFRobot/DFRobot_LIS2DH12
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "driver/i2c.h"

#include "i2c.h"
#include "accelerometer.h"

static uint8_t sensorAddress;

static uint8_t mgScaleVel;

static bool check_sensor_exists(void);
static bool write_reg_single(uint8_t addr, uint8_t value);
static bool write_reg_multiple(uint8_t addr, uint8_t *value, uint8_t size);
static bool read_reg_single(uint8_t addr, uint8_t *value);
static bool read_reg_multiple(uint8_t addr, uint8_t *value, uint8_t size);
static void set_range(uint8_t range);
static bool read_xyz(int16_t *x, int16_t *y, int16_t *z);
static void mg_scale(int16_t *x, int16_t *y, int16_t *z);

bool mc_accelerometer_init(void)
{
    bool ret = false;
    sensorAddress = LIS2DH12_ADDR;

    set_range(ACCEL_DEFAULT_RANGE);

    // TODO replace this with a get device id? Similar to uv.c/begin()
    if (!check_sensor_exists())
        return false;

    // TODO sort out these magic numbers and put these in their own functions, look at temphumid
    uint8_t ctrlRegValues[] = {0x2F, 0x01, 0x40, ACCEL_DEFAULT_RANGE, 0x08, 0x00};
    ret = write_reg_multiple(0xA0, ctrlRegValues, sizeof(ctrlRegValues));
    if (ret == false)
        return false;

    uint8_t int1CfgReg = 0x30;
    uint8_t int1CfgRegValues[] = {0x0A};
    ret = write_reg_multiple(int1CfgReg, int1CfgRegValues, sizeof(int1CfgRegValues));
    if (ret == false)
        return false;

    uint8_t int1ThsReg = 0x32;
    uint8_t int1ThsRegValues[] = {0x02};
    ret = write_reg_multiple(int1ThsReg, int1ThsRegValues, sizeof(int1ThsRegValues));
    if (ret == false)
        return false;

    uint8_t int1DurReg = 0x33;
    uint8_t int1DurRegValues[] = {0x02};
    ret = write_reg_multiple(int1DurReg, int1DurRegValues, sizeof(int1DurRegValues));
    if (ret == false)
        return false;

    return true;
}

bool mc_accelerometer_get_data(Accelerometer_Data *data)
{
    int16_t x, y, z;
    if (!read_xyz(&x, &y, &z))
        return false;

    mg_scale(&x, &y, &z);

    data->x = x;
    data->y = y;
    data->z = z;
    return true;
}

bool check_sensor_exists(void)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (sensorAddress << 1) | WRITE_BIT, ACK_CHECK_EN));
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

        printf("accel_sensorexists_ret= %d\n", ret);
        return false;
    }

    return true;
}

bool write_reg_single(uint8_t addr, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (sensorAddress << 1) | WRITE_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, addr, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, value, ACK_CHECK_EN));
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

        printf("accel_writesingle_ret= %d\n", ret);
        return false;
    }

    return true;
}

bool write_reg_multiple(uint8_t addr, uint8_t *value, uint8_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (sensorAddress << 1) | WRITE_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, addr, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write(cmd, value, size, ACK_CHECK_EN));
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

        printf("accel_writemultiple_ret= %d\n", ret);
        return false;
    }

    return true;
}

bool read_reg_single(uint8_t addr, uint8_t *value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (sensorAddress << 1) | WRITE_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, addr, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (sensorAddress << 1) | READ_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, value, NACK_VAL));
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

        printf("accel_readsingle_ret= %d\n", ret);
        return false;
    }

    return true;
}

bool read_reg_multiple(uint8_t addr, uint8_t *value, uint8_t size)
{
    addr |= 0x80;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (sensorAddress << 1) | WRITE_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, addr, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (sensorAddress << 1) | READ_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_read(cmd, value, size - 1, ACK_VAL));
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, value + (size - 1), NACK_VAL));
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

        printf("accel_readmultiple_ret= %d\n", ret);
        return false;
    }

    return true;
}

void set_range(uint8_t range)
{
    switch (range)
    {
    case LIS2DH12_RANGE_16GA:
        mgScaleVel = 2;
        break;
    case LIS2DH12_RANGE_8GA:
        mgScaleVel = 4;
        break;
    case LIS2DH12_RANGE_4GA:
        mgScaleVel = 8;
        break;
    default:
    case LIS2DH12_RANGE_2GA:
        mgScaleVel = 16;
        break;
    }
}

bool read_xyz(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t data[6] = {0};
    if (!read_reg_multiple(0xA8, data, 6))
        return false;

    *x = ((int8_t)data[1]) * 256 + data[0];
    *y = ((int8_t)data[3]) * 256 + data[2];
    *z = ((int8_t)data[5]) * 256 + data[4];

    return true;
}

void mg_scale(int16_t *x, int16_t *y, int16_t *z)
{
    *x = (int32_t)*x * 1000 / (1024 * mgScaleVel); // transform data to millig, for 2g scale axis*1000/(1024*16),
    *y = (int32_t)*y * 1000 / (1024 * mgScaleVel); // for 4g scale axis*1000/(1024*8),
    *z = (int32_t)*z * 1000 / (1024 * mgScaleVel); // for 8g scale axis*1000/(1024*4)
}

void app_main()
{
#include "temphumid.h"
#include "uv.h"
#include <inttypes.h>

    mc_i2c_init();
    mc_accelerometer_init();
    mc_temphumid_init();
    mc_uv_init();

    /* TEMP *****************************************************/
    uint8_t firmvers;
    float temp;
    float humidity;

    mc_temphumid_get_firmware_vers(&firmvers);
    mc_temphumid_get_temperature(&temp, true);
    mc_temphumid_get_humidity(&humidity, true);

    printf("firmware revision: %d\n", firmvers);
    printf("temperature: %f\n", temp);
    printf("humidity: %f\n", humidity);
    /* *******************************************************/

    /* UV *****************************************************/
    mc_uv_poll();
    uint16_t deviceId = mc_uv_get_device_id();
    float uva = mc_uv_get_uva();
    float uvb = mc_uv_get_uvb();
    float index = mc_uv_get_uv_index();

    printf("device id: %" PRIu16 "\n", deviceId);
    printf("uva: %f\n", uva);
    printf("uvb: %f\n", uvb);
    printf("index: %f\n", index);
    /* *******************************************************/

    /* ACCEL ************************************************/
    Accelerometer_Data data;
    if (!mc_accelerometer_get_data(&data))
    {
        printf("OH NO!\n");
    }
    else
    {
        printf("x: %" PRIi16 "\n", data.x);
        printf("y: %" PRIi16 "\n", data.y);
        printf("z: %" PRIi16 "\n", data.z);
    }
    /* ********************************************************/
}
