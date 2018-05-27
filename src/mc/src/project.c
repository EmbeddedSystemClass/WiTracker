#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "adt.h"
#include "gps.h"
#include "queue.h"
#include "network.h"

int main(void) {
    char s1[] = "This is.";
    char *s2 = "A test.";
    char *s3 = "String!";

    char *packet1 = serialise_packet(s1);
    char *packet2 = serialise_packet(s2);
    char *packet3 = serialise_packet(s3);

    queue_init();
    queue_enqueue(packet1);
    queue_enqueue(packet2);
    queue_enqueue(packet3);

    char *data = queue_dequeue();

    GPS_Data_t gps_data;
    gps_read(&gps_data);

    printf("%d\n", gps_data.GPRMC.Checksum);

    printf("%s\n", data);

    free(data);
    
    return 0;
}