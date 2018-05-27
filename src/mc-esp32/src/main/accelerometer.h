#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    LIS2DH12_RANGE_2GA = 0x00,
    LIS2DH12_RANGE_4GA = 0x10,
    LIS2DH12_RANGE_8GA = 0x20,
    LIS2DH12_RANGE_16GA = 0x30
} LIS2DH12_Range;

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} Accelerometer_Data;

#define LIS2DH12_ADDR 0x19

#define ACCEL_DEFAULT_RANGE LIS2DH12_RANGE_2GA

// Initialises the accelerometer
extern bool mc_accelerometer_init(void);
// Gets the last set data from the accelerometer
extern bool mc_accelerometer_get_data(Accelerometer_Data *data);
// Returns whether the accelerometer has moved since it was last checked
extern bool mc_accelerometer_check_interrupt(void);
// Sets the powermode of the accelerometer
extern bool mc_accelerometer_set_powermode(uint8_t powerMode);

#endif /* ACCELEROMETER_H */
