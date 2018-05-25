#ifndef NETWORK_H
#define NETWORK_H

#include "adt.h"

typedef struct
{
    uint8_t newData;
    uint8_t gpsState;
    float latitude;
    float longitude;
    char *timestamp;
    float temperature;
    float humidity;
    uint8_t outside;
    uint8_t tone;
    float voltage;
    char *wifiScanResult;
} Device_Data;

typedef struct
{
    Device_Data data;
} Device_Packet;

typedef ADT_Node Packet;

extern void mc_network_transmit(Device_Data data);

#endif /* NETWORK_H */
