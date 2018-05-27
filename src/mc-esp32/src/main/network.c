#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "mqtt_client.h"

#include "network.h"
#include "mqtt.h"
#include "adt.h"
#include "queue.h"

typedef enum
{
    MAX_PACKETNUMBER_STRING_LENGTH = 5, // unsigned 16 bit limit of 65535 has 5 chars
    MAX_NEWDATA_STRING_LENGTH = 1,
    MAX_GPSSTATE_STRING_LENGTH = 1,
    MAX_LATITUDE_STRING_LENGTH = 11,   // sign + 3 digits + decimal + 6 digits
    MAX_LONGITUDE_STRING_LENGTH = 11,  // sign + 3 digits + decimal + 6 digits
    MAX_TEMPERATURE_STRING_LENGTH = 4, // sign + 3 digits
    MAX_HUMIDITY_STRING_LENGTH = 3,    // 3 digits (0-100)
    MAX_OUTSIDE_STRING_LENGTH = 1,
    MAX_TONE_STRING_LENGTH = 1,
    MAX_VOLTAGE_STRING_LENGTH = 6, // 6 digits
    MAX_CHECKSUM_STRING_LENGTH = 4 // unsigned 16 bit limit of 65535 in HEX has 4 chars (FFFF)
} Max_Data_Length;

#define PACKET_STRING_BUFFER 0

static uint16_t packetNumber;

// static char *serialise_packet(const char *s);
static char *concat_packet_array(char **arr, uint8_t size);
static char *construct_packet(Device_Data *data);
static char *construct_dataless_packet(void);

void mc_network_init(void)
{
    mc_mqtt_init();
    mc_queue_init();

    packetNumber = 0;
}

void mc_network_transmit(Device_Data data)
{
    char *packet = (data.newData) ? construct_packet(&data) : construct_dataless_packet();
    mc_mqtt_publish(packet);
    return;

#ifdef NETWORK_DEBUG
    printf("ENQUEUEING PACKET: %s\n", packet);
    fflush(0);
    mc_queue_enqueue(packet);

    // Try to upload the packet immediately
    mc_network_upload();
#endif
}

void mc_network_upload(void)
{
    char *packets[MAX_PACKETS_IN_SINGLE_UPLOAD];

    uint8_t totalPackets;
    for (totalPackets = 0; totalPackets < MAX_PACKETS_IN_SINGLE_UPLOAD; totalPackets++)
    {
        if ((packets[totalPackets] = mc_queue_dequeue()) == NULL)
            break;
    }
    if (totalPackets == MAX_PACKETS_IN_SINGLE_UPLOAD) // no null packets were dequeued - we reached the upper bound
        totalPackets--;

    if (totalPackets == 0)
        return;

    char *bigPacket = concat_packet_array(packets, totalPackets);
    if (mc_mqtt_publish(bigPacket) == -1)
    {
        printf("NETWORK ERROR: COULD NOT UPLOAD DATA!\n");
        // Enqueue the items back into the queue (head-first), in reverse order of how they were taken out
        for (int8_t i = totalPackets; i >= 0; i--)
            mc_queue_enqueue_at_head(packets[i]);
    }

    for (uint8_t i = 0; i <= totalPackets; i++)
        free(packets[i]);
}

char *construct_dataless_packet(void)
{
    char *result = malloc(5);
    result[0] = 'H';
    result[1] = 'E';
    result[2] = 'L';
    result[3] = 'L';
    result[4] = 0;
    return result;
}

char *construct_packet(Device_Data *data)
{
    /* The packet structure is as follows:
        $id,packet_number,new_data,data,*checksum<CR><LF>
    where data is empty when new_data = 0, otherwise is as follows:
        state,latitude,longitude,datetime,temperature,humidity,outside,tone,voltage,\
        ssid1;bssid1;rssi1%ssid2;bssid2;rssi2
    
    Note: the \ character above does not exist in the packet, it's there because there isn't
    enough room to wright the entire structure on one line. It does not indicate a newline character.

    For more information take a look at this doc:
    https://bit.ly/2sgRjW3
    */
    // Calculate length of the packet
    uint16_t length = PACKET_STRING_BUFFER;

    length += sizeof(PACKET_START_CHAR) - 1; // subtract 1 because the string literal always includes an
                                             // implicit null-termination character
    length += sizeof(PACKET_ID_CHARS) - 1;
    length += sizeof(PACKET_OBJECT_SEPARATOR) - 1;
    length += MAX_PACKETNUMBER_STRING_LENGTH;
    length += sizeof(PACKET_OBJECT_SEPARATOR) - 1;

    length += MAX_NEWDATA_STRING_LENGTH;
    length += sizeof(PACKET_OBJECT_SEPARATOR) - 1;

    length += MAX_GPSSTATE_STRING_LENGTH;
    length += sizeof(PACKET_OBJECT_SEPARATOR) - 1;

    length += MAX_LATITUDE_STRING_LENGTH;
    length += sizeof(PACKET_OBJECT_SEPARATOR) - 1;

    length += MAX_LONGITUDE_STRING_LENGTH;
    length += sizeof(PACKET_OBJECT_SEPARATOR) - 1;

    length += strlen(data->timestamp);
    length += sizeof(PACKET_OBJECT_SEPARATOR) - 1;

    length += MAX_TEMPERATURE_STRING_LENGTH;
    length += sizeof(PACKET_OBJECT_SEPARATOR) - 1;

    length += MAX_HUMIDITY_STRING_LENGTH;
    length += sizeof(PACKET_OBJECT_SEPARATOR) - 1;

    length += MAX_OUTSIDE_STRING_LENGTH;
    length += sizeof(PACKET_OBJECT_SEPARATOR) - 1;

    length += MAX_TONE_STRING_LENGTH;
    length += sizeof(PACKET_OBJECT_SEPARATOR) - 1;

    length += MAX_VOLTAGE_STRING_LENGTH;
    length += sizeof(PACKET_OBJECT_SEPARATOR) - 1;

    length += strlen(data->wifiScanResult);
    length += sizeof(PACKET_OBJECT_SEPARATOR) - 1;

    length += sizeof(PACKET_CHECKSUM_CHAR) - 1;
    length += MAX_CHECKSUM_STRING_LENGTH;
    length += sizeof(PACKET_END_CHARS) - 1;

    char *result = malloc(length + 1);

    snprintf(result, length, PACKET_START_CHAR PACKET_ID_CHARS PACKET_OBJECT_SEPARATOR "%d,1,%d,%f,%f,%s,%f,%f,%d,%d,%d,%s,*,0000\r\n",
             packetNumber++, data->gpsState, data->latitude, data->longitude, data->timestamp, data->temperature, data->humidity, data->outside, data->tone, data->voltage, data->wifiScanResult);

    return result;
}

char *concat_packet_array(char **arr, uint8_t size)
{
    printf("got here");
    fflush(NULL);
    uint16_t length = 0;
    for (uint8_t i = 0; i < size; i++)
        length += strlen(arr[i]);
    printf("2 got here");
    fflush(0);
    char *result = (char *)malloc(length + 1);

    if (size == 0)
    {
        result[0] = 0;
        return result;
    }
    printf("3 got here");
    fflush(0);

    strncpy(result, arr[0], length);
    for (uint8_t i = 1; i < size; i++)
        strncat(result, arr[i], length - strlen(result));

    printf("4got here");
    fflush(0);
    return result;
}

void free_data_struct(Device_Data data)
{
    if (data.timestamp != NULL)
        free(data.timestamp);

    if (data.wifiScanResult != NULL)
        free(data.wifiScanResult);
}

// char *serialise_packet(const char *s)
// {
//     char *result = malloc(strlen(s) + 1);
//     if (result == NULL)
//     {
//         error_handler();
//     }
//     strcpy(result, s);

//     return result;
// }
