#include <Energia.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

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

static void empty_serial_buffer(void);
static Conversion_Errno str2int(int *out, char *s, int base);
static int parse_int(char *s, int size, int base);
static char parse_char(char* s);
static char parse_double(char *s);
static void error_handler(void);


void gps_init(void) {
    Serial2.begin(GPS_BAUD_RATE);
    empty_serial_buffer();
}

GPS_Data_t gps_read(void) {
    uint8_t sentenceCount = 0;
    char c;
    int charIndex = 0;

    memset(serialBuffer, 0, sizeof(serialBuffer));

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
    char *word = strtok(sentenceBuffer, delimiter);
    if (word != NULL) strncpy(messageId, word, MESSAGE_ID_LENGTH);

    uint8_t positionInSentence = MESSAGE_ID;
    while (word != NULL) {
        if (strcmp(messageId, GPS_MESSAGE_ID_GPRMC) == 0) {
            switch (positionInSentence) {
                case MESSAGE_ID:
                    break;
                case TIME:
                    // Parsing is done on the word pointer with an offset, size, and base
                    data.GPRMC.Time.Hours = parse_int(word, 2, 10);
                    data.GPRMC.Time.Minutes = parse_int(word + 2, 2, 10);
                    data.GPRMC.Time.Seconds = parse_int(word + 4, 2, 10);
                    data.GPRMC.Time.Hundredths = parse_int(word + 7, 2, 10);
                    break;
                case STATUS:
                    data.GPRMC.Status = parse_char(word);
                    break;
                case LATITUDE:
                    data.GPRMC.Latitude = parse_double(word);
                    break;
                case NORTH_SOUTH_INDICATOR:
                    data.GPRMC.NorthSouthIndicator = parse_char(word);
                    break;
                case LONGITUDE:
                    data.GPRMC.Longitude = parse_double(word);
                    break;
                case EAST_WEST_INDICATOR:
                    data.GPRMC.EastWestIndicator = parse_char(word);
                    break;
                case SPEED:
                    data.GPRMC.Speed = parse_double(word);
                    break;
                case COURSE:
                    data.GPRMC.Course = parse_double(word);
                    break;
                case DATE:
                    data.GPRMC.Date.Day = parse_int(word, 2, 10);
                    data.GPRMC.Date.Month = parse_int(word + 2, 2, 10);
                    data.GPRMC.Date.Year = parse_int(word + 4, 2, 10);
                    break;
                case VARIATION:
                    data.GPRMC.Variation = parse_char(word);
                    break;
                case CHECKSUM:
                    data.GPRMC.Checksum = parse_int(word + 2, 2, 16);
                    break;
            }
        } else {
            // break;
        }

        positionInSentence++;
        word = strtok(NULL, delimiter);
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

static int parse_int(char *s, int size, int base) {
    char stringBuffer[30];
    char formatString[5];
    int result;

    sprintf(formatString, "%%.%ds", size);
    sprintf(stringBuffer, formatString, s);
    
    if (str2int(&result, stringBuffer, base) != CONVERSION_SUCCESS) {
        error_handler();
        return 0;
    }

    return result;
}

static char parse_char(char* s) {
    return s[0];
}

static char parse_double(char *s) {
    double result;

    sscanf(s, "%lf", &result);

    return result;
}

static void error_handler(void) {
    Serial.println("Uh oh spaghettio");
}
