#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

#include "gps.h"
#include "spi.h"

static char serialBuffer[GPS_MAX_SENTENCE_LENGTH * GPS_SENTENCES_PER_READ];
static char sentenceBuffer[GPS_MAX_SENTENCE_LENGTH];

static const char delimiter = ',';

static void empty_serial_buffer(void);
static Conversion_Errno str2int(int *out, const char *s, int base);
static int parse_int(const char *s, int size, int base);
static char parse_char(const char* s);
static double parse_double(const char *s);
static void error_handler(void);
static uint8_t tokenise(const char *s, char delim, char out[MAX_WORDS_IN_SENTENCE][MAX_CHARS_IN_WORD]);


GPS_Errno gps_init(void) {
    spi_begin(GPS_SERIAL_CHANNEL);
    empty_serial_buffer();
    
    return GPS_SUCCESS;
}

GPS_Errno gps_read(GPS_Data_t *data) {
    memset(serialBuffer, 0, sizeof(serialBuffer));

        printf("got here!\n");


    for (uint16_t i = 0; spi_available(GPS_SERIAL_CHANNEL) && i < sizeof(serialBuffer); i++) {
        serialBuffer[i] = spi_read(GPS_SERIAL_CHANNEL);
    }
    empty_serial_buffer(); // in case we filled the serial buffer array and there's more data to read

    char *gprmcStartPointer;
    if ((gprmcStartPointer = strstr(serialBuffer, GPS_MESSAGE_ID_GPRMC)) != NULL) {
        uint16_t gprmcStartIndex = gprmcStartPointer - serialBuffer;

        // Iterate up to the <CR> character
        uint16_t i;
        for (i = 0; serialBuffer[gprmcStartIndex + i] != 13; i++) {
            sentenceBuffer[i] = serialBuffer[gprmcStartIndex + i];
        }

        // Null-terminate the array
        sentenceBuffer[i] = '\0';
    }

    char messageId[GPS_MESSAGE_ID_LENGTH];
    char tokens[MAX_WORDS_IN_SENTENCE][MAX_CHARS_IN_WORD];
    int numTokens = tokenise(sentenceBuffer, delimiter, tokens);

    if (numTokens == 0) {
        error_handler();
        return GPS_INVALID_MESSAGE_FORMAT;
    }
    
    strncpy(messageId, tokens[0], sizeof(messageId));
    
    char word[MAX_CHARS_IN_WORD];
    for (uint8_t position = MESSAGE_ID; position <= CHECKSUM; position++) {
        strncpy(word, tokens[position], sizeof(word));

        if (strcmp(messageId, GPS_MESSAGE_ID_GPRMC) == 0) {
            switch (position) {
                case MESSAGE_ID:
                    break;
                case TIME:
                    // Parsing is done on the word pointer with an offset, size, and base
                    data->GPRMC.Time.Hours = parse_int(word, 2, 10);
                    data->GPRMC.Time.Minutes = parse_int(word + 2, 2, 10);
                    data->GPRMC.Time.Seconds = parse_int(word + 4, 2, 10);
                    data->GPRMC.Time.Hundredths = parse_int(word + 7, 2, 10);
                    break;
                case STATUS:
                    data->GPRMC.Status = parse_char(word);
                    break;
                case LATITUDE:
                    data->GPRMC.Latitude = parse_double(word);
                    break;
                case NORTH_SOUTH_INDICATOR:
                    data->GPRMC.NorthSouthIndicator = parse_char(word);
                    break;
                case LONGITUDE:
                    data->GPRMC.Longitude = parse_double(word);
                    break;
                case EAST_WEST_INDICATOR:
                    data->GPRMC.EastWestIndicator = parse_char(word);
                    break;
                case SPEED:
                    data->GPRMC.Speed = parse_double(word);
                    break;
                case COURSE:
                    data->GPRMC.Course = parse_double(word);
                    break;
                case DATE:
                    data->GPRMC.Date.Day = parse_int(word, 2, 10);
                    data->GPRMC.Date.Month = parse_int(word + 2, 2, 10);
                    data->GPRMC.Date.Year = parse_int(word + 4, 2, 10);
                    break;
                case VARIATION:
                    data->GPRMC.Variation = parse_char(word);
                    break;
                case CHECKSUM:
                    data->GPRMC.Checksum = parse_int(word + 2, 2, 16);
                    break;
            }
        }
    }

    // Calculate checksum. XOR all values successfully between $ and * non-inclusive
    uint16_t checksum = 0;
    for (uint16_t i = 1; sentenceBuffer[i] != '*'; i++) {
        checksum ^= sentenceBuffer[i];
    }

    return GPS_SUCCESS;
}

void gps_print(GPS_Data_t *data) {
    // // Time
    // debug_printf("Hours: ", data->GPRMC.Time.Hours);
    // debug_printf("Minutes: ", data->GPRMC.Time.Minutes);
    // debug_printf("Seconds: ", data->GPRMC.Time.Seconds);
    // debug_printf("Hundredths: ", data->GPRMC.Time.Hundredths);

    // // Status
    // debug_printf("Status: ", data->GPRMC.Status);

    // // Latitude
    // debug_printf("Latitude: ", data->GPRMC.Latitude, 5);

    // // North-South Indicator
    // debug_printf("NorthSouthIndicator: ", data->GPRMC.NorthSouthIndicator);

    // // Longitude
    // debug_printf("Longitude: ", data->GPRMC.Longitude, 5);

    // // East-West Indicator
    // debug_printf("EastWestIndicator: ", data->GPRMC.EastWestIndicator);

    // // Speed
    // debug_printf("Speed: ", data->GPRMC.Speed, 5);

    // // Course
    // debug_printf("Course: ", data->GPRMC.Course, 5);

    // // Date
    // debug_printf("Day: ", data->GPRMC.Date.Day);
    // debug_printf("Month: ", data->GPRMC.Date.Month);
    // debug_printf("Year: ", data->GPRMC.Date.Year);

    // // Variation
    // debug_printf("Variation: ", data->GPRMC.Variation););

    // // Checksum
    // debug_printf("Checksum: ", data->GPRMC.Checksum); // TODO: print this in HEX);
}

void empty_serial_buffer(void) {
    while (spi_available(GPS_SERIAL_CHANNEL)) spi_read(GPS_SERIAL_CHANNEL);
}

/**
 * Shamelessly taken from https://stackoverflow.com/a/12923949/4912373
 */
Conversion_Errno str2int(int *out, const char *s, int base) {
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

int parse_int(const char *s, int length, int base) {
    char stringBuffer[10];  // 10 character-long int limit
    char formatString[5];
    int result;

    sprintf(formatString, "%%.%ds", length);
    sprintf(stringBuffer, formatString, s);
    
    if (str2int(&result, stringBuffer, base) != CONVERSION_SUCCESS) {
        error_handler();
        return 0;
    }

    return result;
}

char parse_char(const char* s) {
    return s[0];
}

double parse_double(const char *s) {
    return strtod(s, NULL);
}

void error_handler(void) {
    // debug_printf("Uh oh spaghettio");
}

/**
 * Inspired by https://gist.github.com/afiedler/662786
 */
uint8_t tokenise(const char *s, char delim, char out[MAX_WORDS_IN_SENTENCE][MAX_CHARS_IN_WORD]) {
    uint8_t tokenCount = 0;
    uint8_t prevToken = 0;
    uint8_t i;

    // Loop through and extract each token
    for (i = 0; s[i] != '\0'; i++) {
        // Reached the end of a token?
        if(s[i] == delim) {
            // If the token is not empty, copy over the token
            if(i - prevToken > 0)
                memcpy(out[tokenCount], &s[prevToken], (i - prevToken));

            // Null-terminate the string
            out[tokenCount][(i - prevToken)] = '\0';

            // The next token starts at i + 1
            prevToken = i + 1;
            tokenCount++;
        }
    }

    // If the token is not empty, copy over the token
    if(i - prevToken > 0)
        memcpy(out[tokenCount], &s[prevToken], (i - prevToken));

    // Null-terminate the string
    out[tokenCount][(i - prevToken)] = '\0';

    tokenCount++;

    return tokenCount;
}
