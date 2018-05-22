#ifndef TEMPHUMID_H
#define TEMPHUMID_H

#include <stdbool.h>
#include <stdint.h>

#define Si7006_ADDR 0x40 // default address

extern bool mc_temphumid_init(void);
extern bool mc_temphumid_reset(void);
extern bool mc_temphumid_get_temp_control(uint8_t *res, bool voltage, bool heater);
extern bool mc_temphumid_set_temp_control(uint8_t res, bool heater);
extern bool mc_temphumid_get_heater_control(uint8_t *heaterCurrent);
extern bool mc_temphumid_set_heater_control(uint8_t heaterCurrent);
extern bool mc_temphumid_get_device_id(char deviceId[8]);
extern bool mc_temphumid_get_firmware_vers(uint8_t *firmware);
extern bool mc_temphumid_get_temperature(float *temperature, bool mode);
extern bool mc_temphumid_get_humidity(float *humidity, bool mode);
// extern bool mc_temphumid_get_old_temperature(float *temperature);

// extern uint8_t mc_temphumid_crc8(const uint8_t *data, int len);
extern uint8_t mc_temphumid_getError(void);

#endif /* TEMPHUMID_H */
