#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "network.h"
#include "mqtt.h"
#include "adt.h"

static void error_handler(void);
static char *serialise_packet(const char *s);

void mc_network_transmit(Device_Data data)
{
    // TODO move the data into some sort of queue
    if (mc_mqtt_publish(data.wifiScanResult) == -1)
    {
        printf("ERROR: MQTT COULDNT CONNECT!\n");
    }

    free(data.wifiScanResult);
}

char *serialise_packet(const char *s)
{
    char *result = malloc(strlen(s) + 1);
    if (result == NULL)
    {
        error_handler();
    }
    strcpy(result, s);

    return result;
}

void error_handler(void)
{
    // do stuff
}
