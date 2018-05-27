#ifndef MQTT_H
#define MQTT_H

extern void mc_mqtt_init(void);
extern int mc_mqtt_publish(const char *data);
extern uint8_t mc_mqtt_check_connected(void);

#endif /* MQTT_H */
