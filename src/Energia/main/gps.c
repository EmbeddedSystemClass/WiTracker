#include <Energia.h>
#include <stdint.h>

#include "gps.h"

#define MESSAGE_ID_LENGTH   6

static GPS_Data_t data;
static char sentenceBuffer[SENTENCES][MAX_SENTENCE_SIZE];
static char tokenisedBuffer[MAX_WORDS_IN_SENTENCE][MAX_SENTENCE_SIZE];
static char currentSentence[MESSAGE_ID_LENGTH];

void gps_init(void) {
    Serial2.begin(GPS_BAUD_RATE);

    // Clear current sentence buffer array
    memset(&currentSentence, 0, sizeof(currentSentence));
}

GPS_Data_t gps_read(void) {
    char *token;
    const char *delimiter = ",";
    uint8_t sentenceCount = 0;
    char c;
    int bufferIndex;
    
    while (Serial2.available()) {
        // Read until LF character (last in line)
        while ((c = Serial2.read()) != 10) {
            // Skip CR character
            if (c == 13) continue;
            
            sentenceBuffer[sentenceCount][i] = c;
            i++;
        }
        sentenceCount++;
    }

    for (int i = 0; i < SENTENCES; i++) {
        // Calculate checksum
        data.Checksum = 0;
        for (int j = 1; sentenceBuffer[j] != '*'; j++) {
            data.Checksum ^= sentenceBuffer[j];
        }

        for (token = strtok(sentenceBuffer, delimiter), bufferIndex = 0; token != NULL;
                token = strtok(NULL, delimiter), bufferIndex++) {
            strncpy(tokenisedBuffer[bufferIndex], token, READ_BUFFER_SIZE);
        }
    }

    // switch (currentSentence) {
    //     case GPRMC:
            
    //         break;
    //     default:
    //         if (Serial2.readBytes(currentSentence, MESSAGE_ID_LENGTH) == 0) {
    //             // error
    //         }
    // }
}

// void serial_parse_data_value(uint8_t *dataValue, size) {
//     // Parse the data
//     Serial2.readBytes(dataValue, size);

//     // Ignore the next, separation byte
//     Serial2.readBytes(NULL, 1);
// }


// // UTC Position
//                 Serial2.readBytes(data.Time.Hours, sizeof(data.Time.Hours));
//                 Serial2.readBytes(NULL, 1);
//                 Serial2.readBytes(data.Time.Minutes, sizeof(data.Time.Minutes));
//                 Serial2.readBytes(NULL, 1);
//                 Serial2.readBytes(data.Time.Seconds, sizeof(data.Time.Seconds));
//                 Serial2.readBytes(NULL, 1);
//                 Serial2.readBytes(data.Time.Hundredths, sizeof(data.Time.Hundredths));
//                 Serial2.readBytes(NULL, 1);
                
//                 // Status
//                 Serial2.readBytes(data.Status, sizeof(data.Status));
//                 Serial2.readBytes(NULL, 1);

//                 // Latitude

//                 // N/S Indicator

//                 // Longitude

//                 // E/W Indicator

//                 // Speed Over Ground

//                 // Course Over Ground

//                 // Date

//                 // Magnetic Variation

//                 // Checksum
