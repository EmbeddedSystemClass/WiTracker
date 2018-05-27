#ifndef MQTT_H
#define MQTT_H

// #define PROD_ENV_ENABLED

extern void mc_mqtt_init(void);
extern int mc_mqtt_publish(const char *data);

#endif /* MQTT_H */
