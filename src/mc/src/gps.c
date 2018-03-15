#include <Energia.h>
#include <stdint.h>

#include "gps.h"

static GPS_Data_t data[DATA_RECORDS];
static char byte;

uint8_t currSeg = 0;
uint8_t currSent = 0;
uint8_t currChar = 0;

void gps_init(void) {
    Serial2.begin(9600);

    // Initialise 
    for (uint8_t i = 0; i < DATA_SEGMENTS; i++) {
        for (uint8_t j = 0; j < SENTENCES; j++) {
            for (uint8_t k = 0; k < MAX_SENTENCE_SIZE; k++) {
                data[i][j][k] = 0;
            }
        }
    }
}

GPS_Data_t gps_read(void) {
    while (Serial2.available() && (byte = Serial2.read())) {
        switch (currSent) {
            case GPRMC:
                byte 
        }
    }
}

void 


void loop() {
    while (Serial2.available()) {
        gpsReadChar = Serial2.read();
        if 
    }
}

