#include <Energia.h>
#include <stdint.h>

#include "gps.h"

typedef enum {
    CONVERSION_SUCCESS,
    CONVERSION_OVERFLOW,
    CONVERSION_UNDERFLOW,
    CONVERSION_INCONVERTIBLE
} Conversion_Errno;

#define MESSAGE_ID_LENGTH   6

#define MAX_SENTENCE_LENGTH 82
#define SENTENCES_PER_READ  12

static GPS_Data_t data;
static char serialBuffer[MAX_SENTENCE_LENGTH * SENTENCES_PER_READ];
static char sentenceBuffer[MAX_SENTENCE_LENGTH];

static const char *delimiter = ",";

void gps_init(void) {
    Serial2.begin(GPS_BAUD_RATE);
    empty_serial_buffer();
}

GPS_Data_t gps_read(void) {
    uint8_t sentenceCount = 0;
    char c;
    int charIndex = 0;

    memset(serialBuffer, 0, sizeof(buffer));

    for (uint16_t i = 0; Serial2.available() && i < sizeof(serialBuffer); i++) {
        serialBuffer[i] = Serial2.read();
    }
    empty_serial_buffer(); // in case we filled the serial buffer array and there's more data to read

    char *gprmcStartPointer;
    if ((gprmcStartPointer = strstr(serialBuffer, GPS_MESSAGE_ID_GPRMC)) != NULL) {
        uint16_t gprmcStartIndex = gprmcStartPointer - serialBuffer;

        // Iterate up to the <CR> character
        for (uint16_t i = 0; serialBuffer[gprmcStartIndex + i] != 13; i++) {
            sentenceBuffer[i] = serialBuffer[gprmcStartIndex + i];
        }
    }

    char messageId[MESSAGE_ID_LENGTH];
    char *token = strtok(sentenceBuffer, delimiter);
    if (token != NULL) strncpy(messageId, token, MESSAGE_ID_LENGTH);

    uint8_t currentWord = TIME;
    while (token != NULL) {
        if (strcmp(messageId, GPS_MESSAGE_ID_GPRMC) == 0) {
            switch (currentWord) {
                case TIME:

                    break;
                case STATUS:
                    break;
                case LATITUDE:
                    break;
                case NORTH_SOUTH_INDICATOR:
                    break;
                case LONGITUDE:
                    break;
                case EAST_WEST_INDICATOR:
                    break;
                case SPEED:
                    break;
                case COURSE:
                    break;
                case DATE:
                    break;
                case VARIATION:
                    break;
                case CHECKSUM:
                    break;
            }
        } else {
            // break;
        }

        currentWord++;
        token = strtok(NULL, delimiter);
    }


    // Calculate checksum. XOR all values successfully between $ and * non-inclusive
    uint16_t checksum = 0;
    for (uint16_t i = 1; sentenceBuffer[i] != '*'; i++) {
        checksum ^= sentenceBuffer[i];
    }
}

static void empty_serial_buffer(void) {
    while (Serial2.available()) Serial2.read();
}

/**
 * Shamelessly taken from https://stackoverflow.com/a/12923949/4912373
 */
static Conversion_Errno str2int(int *out, char *s, int base) {
    char *end;
    
    if (s[0] == '\0' || isspace((unsigned char) s[0]))
        return CONVERSION_INCONVERTIBLE;
    
    long l = strtol(s, &end, base);
    if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
        return CONVERSION_OVERFLOW;
    if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
        return CONVERSION_UNDERFLOW;
    
    if (*end != '\0')
        return CONVERSION_INCONVERTIBLE;
    
    *out = l;
    return CONVERSION_SUCCESS;
}
