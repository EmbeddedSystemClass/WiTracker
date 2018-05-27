#ifndef TEMPHUMID_H
#define TEMPHUMID_H

#include <stdbool.h>
#include <stdint.h>

#define Si7006_ADDR 0x40 // default address

// Initialises the temphumid sensor
extern bool mc_temphumid_init(void);
// Resets the temphumid sensor
extern bool mc_temphumid_reset(void);
// Returns the temp control values for the temphumid sensor
extern bool mc_temphumid_get_temp_control(uint8_t *res, bool voltage, bool heater);
// Sets the temp control values for the temphumid sensor
extern bool mc_temphumid_set_temp_control(uint8_t res, bool heater);
// Gets the heater control values for the temphumid sensor
extern bool mc_temphumid_get_heater_control(uint8_t *heaterCurrent);
// Sets the heater control values for the temphumid sensor
extern bool mc_temphumid_set_heater_control(uint8_t heaterCurrent);
// Gets the device id of the temphumid sensor
extern bool mc_temphumid_get_device_id(char deviceId[8]);
// Gets the running firmware version of the temphumid sensor
extern bool mc_temphumid_get_firmware_vers(uint8_t *firmware);
// Gets the temperature
extern bool mc_temphumid_get_temperature(float *temperature);
// Gets the humidity
extern bool mc_temphumid_get_humidity(float *humidity);
// Gets the last-written error value
extern uint8_t mc_temphumid_get_error(void);

#endif /* TEMPHUMID_H */
