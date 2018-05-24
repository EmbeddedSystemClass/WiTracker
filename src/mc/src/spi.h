#ifndef SPI_H
#define SPI_H

#include <stdint.h>

extern void spi_begin(uint8_t channel);
extern uint8_t spi_available(uint8_t channel);
extern char spi_read(uint8_t channel);

#endif /* SPI_H */
