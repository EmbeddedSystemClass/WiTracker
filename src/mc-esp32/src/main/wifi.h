#ifndef WIFI_H
#define WIFI_H

struct AP_Scan
{
    char ssid[33];
    char bssid[25];
    char rssi[5];
};

#define SCAN_ARRAY_SIZE 40

extern int nScans;

extern void mc_wifi_init(void);
extern void mc_wifi_start(void);
extern void mc_wifi_stop(void);
extern void mc_wifi_connect(void);
extern void mc_wifi_disconnect(void);
extern char *mc_wifi_scan(void);
extern uint8_t mc_wifi_check_connected(void);

#endif /* WIFI_H */
