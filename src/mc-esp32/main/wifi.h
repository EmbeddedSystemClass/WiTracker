#ifndef WIFI_H
#define WIFI_H


struct AP_Scan
{
    char ssid[33];
    char bssid[25];
    char rssi[5];
};

extern struct AP_Scan scanArray[20];
extern int nScans;

extern void mc_wifi_init(void);
char *mc_wifi_scan(void);

#endif /* WIFI_H */
