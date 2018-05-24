#ifndef UV_H
#define UV_H

#include <stdbool.h>
#include <stdint.h>

#define VEML6075_ADDR 0x10 // default address
#define VEML6075_DEVID 0x26

extern float mc_uv_get_uva(void);
extern float mc_uv_get_uvb(void);

extern uint16_t mc_uv_get_device_id(void);

#endif /* UV_H */
