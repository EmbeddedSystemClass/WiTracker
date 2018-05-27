#ifndef UV_H
#define UV_H

#include <stdbool.h>
#include <stdint.h>

#define VEML6075_ADDR 0x10 // default address
#define VEML6075_DEVID 0x26

// Initialises the UV sensor
extern bool mc_uv_init(void);
// Updates the UV sensor's read registers
extern void mc_uv_poll(void);
// Gets the UVA reading from the UV sensor
extern float mc_uv_get_uva(void);
// Gets the UVB reading from the UV sensor
extern float mc_uv_get_uvb(void);
// Gets the UV Index reading from the UV sensor
extern float mc_uv_get_uv_index(void);
// Sets the powermode of the UV sensor
extern bool mc_uv_set_powermode(uint8_t powerMode);
// Returns whether the device is outside
extern bool mc_uv_check_outside(void);
// Gets the device id of the UV sensor
extern uint16_t mc_uv_get_device_id(void);

#endif /* UV_H */
