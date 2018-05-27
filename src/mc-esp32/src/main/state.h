#ifndef STATE_H
#define STATE_H

typedef enum
{
    ACTIVE_MODE,
    LIGHT_SLEEP_MODE,
    DEEP_SLEEP_MODE
} Device_Mode;

extern bool mc_state_init(void);
extern void mc_state_set_state(uint8_t newMode);
extern uint8_t mc_state_get_state(void);
extern void mc_state_apply(void);
extern void mc_state_toggle_deep(void);

#endif /* STATE_H */
