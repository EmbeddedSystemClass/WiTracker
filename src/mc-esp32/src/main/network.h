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
    int voltage;
    char *wifiScanResult;
} Device_Data;

typedef struct
{
    Device_Data data;
} Device_Packet;

typedef ADT_Node Packet;

#define PACKET_OBJECT_SEPARATOR ","
#define PACKET_START_CHAR "$"
#define PACKET_ID_CHARS "GP03"
#define PACKET_CHECKSUM_CHAR "*"
#define PACKET_END_CHARS "\r\n"
#define MAX_PACKETS_IN_SINGLE_UPLOAD 10

// Initialises the network functionality
extern void mc_network_init(void);
// Stores a device data packet and prepares it for transmission over MQTT
extern void mc_network_transmit(Device_Data data);
// Uploads the current state of the transmission queue to the MQTT server
extern void mc_network_upload(void);

#endif /* NETWORK_H */
