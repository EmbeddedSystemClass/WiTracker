#include <Energia.h>
#include <stdint.h>

#include "gps.h"

#define MESSAGE_ID_LENGTH   6

#define MAX_SENTENCE_LENGTH 82
#define SENTENCES_PER_READ  12

static GPS_Data_t data;
static char sentencesBuffer[SENTENCES][MAX_SENTENCE_SIZE];
static char tokenisedBuffer[MAX_WORDS_IN_SENTENCE][MAX_SENTENCE_SIZE];
static char currentSentence[MESSAGE_ID_LENGTH];

static char serialBuffer[MAX_SENTENCE_LENGTH * SENTENCES_PER_READ];
static char sentenceBuffer[MAX_SENTENCE_LENGTH];

void gps_init(void) {
    Serial2.begin(GPS_BAUD_RATE);
    empty_serial_buffer();

    // Clear current sentence buffer array
    memset(&currentSentence, 0, sizeof(currentSentence));
}

GPS_Data_t gps_read(void) {
    char *token;
    const char *delimiter = ",";
    uint8_t sentenceCount = 0;
    char c;
    int charIndex = 0;
    uint8_t checksum = 0;
    char messageId[MESSAGE_ID_LENGTH];

    memset(serialBuffer, 0, sizeof(buffer));
    memset(sentenceBuffer, 0, sizeof(sentencesBuffer));

    for (uint16_t i = 0; Serial2.available() && i < sizeof(serialBuffer); i++) {
        serialBuffer[i] = Serial2.read();
    }
    empty_serial_buffer(); // in case we filled the serial buffer array and there's more data to read

    char *gprmcStartPointer;
    if ((gprmcStartPointer = strstr(serialBuffer, GPS_MESSAGE_ID_GPRMC)) != NULL) {
        uint16_t gprmcStartIndex = gprmcStartPointer - serialBuffer;
        for (uint16_t i = 0; serialBuffer[gprmcStartIndex + i] != 13; i++) {
            sentenceBuffer[i] = serialBuffer[gprmcStartIndex + i];
        }
    }
    // while (Serial2.available()) {
    //     switch (c = Serial2.read()) {
    //         case 10:
    //             sentenceCount++;
    //             charIndex = 0;
    //         case 13: break;
    //         default:
    //             sentencesBuffer[sentenceCount][charIndex++] = c;
    //     }
    // }

    // for (int i = 0; i < SENTENCES; i++) {
    //     // Calculate checksum. XOR all values successfully between $ and * non-inclusive
    //     for (int j = 1; sentencesBuffer[i][j] != '*'; j++) {
    //         checksum ^= sentencesBuffer[i][j];
    //     }

    //     for (token = strtok(sentencesBuffer[i], delimiter), charIndex = 0; token != NULL;
    //             token = strtok(NULL, delimiter), charIndex++) {
    //         // Get the message id
    //         strncpy(messageId, token, MESSAGE_ID_LENGTH);

    //         if (strcmp(messageId, GPS_MESSAGE_ID_GPRMC)) {
    //             Serial.println("YEEEEEEEEEEEEE");
    //         } else if (strcmp(messageId, GPS_MESSAGE_ID_GPGGA)) {

    //         } else {

    //         }
    //     }
    // }
}

static void empty_serial_buffer(void) {
    while (Serial2.available()) Serial2.read();
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
