#ifndef STATE_H
#define STATE_H

typedef enum
{
    ACTIVE_MODE,
    LIGHT_SLEEP_MODE,
    DEEP_SLEEP_MODE
} Device_Mode;

// Initialises the state manager
extern bool mc_state_init(void);
// Sets the device;s current state
extern void mc_state_set_state(uint8_t newMode);
// Get's the device's current state
extern uint8_t mc_state_get_state(void);
// Applies the device's current state to the leds
extern void mc_state_apply(void);
// Togglees the device into and out of deep sleep mode
extern void mc_state_toggle_deep(void);

#endif /* STATE_H */
