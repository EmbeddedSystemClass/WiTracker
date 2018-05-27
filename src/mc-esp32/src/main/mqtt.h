#ifndef MQTT_H
#define MQTT_H

// Initialises the MQTT task
extern void mc_mqtt_init(void);
// Publishes a message over MQTT to the config's defined endpoint
extern int mc_mqtt_publish(const char *data);
// Returns whether the device is connected to the MQTT server
extern uint8_t mc_mqtt_check_connected(void);

#endif /* MQTT_H */
