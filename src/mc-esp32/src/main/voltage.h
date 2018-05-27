#ifndef VOLTAGE_H
#define VOLTAGE_H

#include <stdint.h>

// Initialises the voltage ADC config
extern void mc_voltage_init(void);
// Returns a voltage ADC sample reading
extern uint32_t mc_voltage_sample(void);

#endif /* VOLTAGE_H */
