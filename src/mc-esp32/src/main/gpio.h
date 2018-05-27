#ifndef GPIO_H
#define GPIO_H

// Initialises GPIO functionality
extern void mc_gpio_init();
// Returns whether the test switch is toggled on or off
extern int mc_gpio_test_enabled();

#endif /* GPIO_H */
