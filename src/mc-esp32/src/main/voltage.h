#ifndef VOLTAGE_H
#define VOLTAGE_H

#include <stdint.h>

extern void mc_voltage_init(void);
extern uint32_t mc_voltage_sample(void);

#endif /* VOLTAGE_H */
