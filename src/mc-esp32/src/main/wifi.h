#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>

struct AP_Scan
{
    char ssid[33];
    char bssid[25];
    char rssi[5];
};

#define SCAN_ARRAY_SIZE 10

extern int nScans;

// Initialises the wifi task
extern void mc_wifi_init(void);
// Starts the wifi task
extern void mc_wifi_start(void);
// Stops the wifi task
extern void mc_wifi_stop(void);
// Attempts to connect the wifi if it isn't already
extern void mc_wifi_connect(void);
// Attempts to disconnect the wifi if it hasn't already
extern void mc_wifi_disconnect(void);
// Performs a wifi scan and returns the result
extern char *mc_wifi_scan(void);
// Returns whether the device is connected to wifi
extern uint8_t mc_wifi_check_connected(void);

#endif /* WIFI_H */
