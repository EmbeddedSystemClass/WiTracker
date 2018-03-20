#ifndef GPS_H_
#define GPS_H_

#include <stdint.h>

// #define GPS_DISABLE_GPRMC
#define GPS_DISABLE_GPGGA
#define GPS_DISABLE_GPGSA
#define GPS_DISABLE_GPGSV

#define GPS_BAUD_RATE 9600

#define GPS_MESSAGE_ID_GPRMC   "$GPRMC"
#define GPS_MESSAGE_ID_GPVTG   "$GPVTG"
#define GPS_MESSAGE_ID_GPGGA   "$GPGGA"
#define GPS_MESSAGE_ID_GPGSA   "$GPGSA"
#define GPS_MESSAGE_ID_GPGSV1  "$GPGSV"
#define GPS_MESSAGE_ID_GPGSV2  "$GPGSV"
#define GPS_MESSAGE_ID_GPGSV3  "$GPGSV"
#define GPS_MESSAGE_ID_GPGSV4  "$GPGSV"
#define GPS_MESSAGE_ID_GPGLL   "$GPGLL"

#define GPS_GPRMC_OK        'A'
#define GPS_GPRMC_INVALID   'V'

typedef enum {
    DATA_RECORDS = 1,
    SENTENCES = 6,
    MAX_SENTENCE_SIZE = 82,
    MAX_WORDS_IN_SENTENCE = 12,
    MAX_CHARS_IN_WORD = MAX_SENTENCE_SIZE
} Gps_Buffer_Size;

typedef enum {
    MESSAGE_ID,
    TIME,
    STATUS,
    LATITUDE,
    NORTH_SOUTH_INDICATOR,
    LONGITUDE,
    EAST_WEST_INDICATOR,
    SPEED,
    COURSE,
    DATE,
    VARIATION,
    NONE,
    CHECKSUM
} GPS_GPRMC_ORDER;

typedef struct {
    uint8_t Day;  /*!< Day in month from GPS. */
    uint8_t Month; /*!< Month from GPS. */
    uint8_t Year;  /*!< Year from GPS. */
} TM_GPS_Date_t;

typedef struct {
    uint8_t Hours;       /*!< Hours from GPS time. */
    uint8_t Minutes;     /*!< Minutes from GPS time. */      
    uint8_t Seconds;     /*!< Seconds from GPS time. */
    uint16_t Hundredths; /*!< Hundredths from GPS time. */
} TM_GPS_Time_t;

typedef struct {
    uint8_t ID;        /*!< SV PRN number */
    uint8_t Elevation; /*!< Elevation in degrees, 90 maximum */
    uint16_t Azimuth;  /*!< Azimuth, degrees from true north, 000 to 359 */
    uint8_t SNR;       /*!< SNR, 00-99 dB (0 when not tracking) */
} TM_GPS_Satellite_t;

typedef struct {
    TM_GPS_Time_t Time;
    char Status;
    float Latitude;
    char NorthSouthIndicator;
    float Longitude;
    char EastWestIndicator;
    float Speed;                                          /*!< Speed in knots from GPS. */
    float Course;                                      /*!< Course on the ground in relation to North. */
    TM_GPS_Date_t Date;                                   /*!< Current data from GPS. @ref TM_GPS_Date_t. */
    char Variation;                                     /* Magnetic Variation */
    uint8_t Checksum;
} GPS_GPRMC_t;

typedef struct {
    float Latitude;                                       /*!< Latitude position from GPS, -90 to 90 degrees response. */
    float Longitude;                                      /*!< Longitude position from GPS, -180 to 180 degrees response. */
    uint8_t Satellites;                                   /*!< Number of satellites in use for GPS position. */
    uint8_t Fix;                                          /*!< GPS fix; 0: Invalid; 1: GPS Fix; 2: DGPS Fix. */
    float Altitude;                                       /*!< Altitude above the sea. */
    TM_GPS_Time_t Time;                                   /*!< Current time from GPS. @ref TM_GPS_Time_t. */
} GPS_GPGGA_t;

typedef struct {
    float HDOP;                                           /*!< Horizontal dilution of precision. */
    float PDOP;                                           /*!< Position dilution od precision. */
    float VDOP;                                           /*!< Vertical dilution of precision. */
    uint8_t FixMode;                                      /*!< Current fix mode in use:; 1: Fix not available; 2: 2D; 3: 3D. */
    uint8_t SatelliteIDs[12];                             /*!< Array with IDs of satellites in use. 
                                                               Only first data are valid, so if you have 5 satellites in use, only SatelliteIDs[4:0] are valid */
} GPS_GPGSA_t;

typedef struct {
    uint8_t SatellitesInView;                             /*!< Number of satellites in view */
    TM_GPS_Satellite_t SatDesc[30];                       /*!< Description of each satellite in view */ 
} GPS_GPGSV_t;

typedef struct {
#ifndef GPS_DISABLE_GPRMC
    GPS_GPRMC_t GPRMC;
#endif
#ifndef GPS_DISABLE_GPGGA
    GPS_GPGGA_t GPGGA;
#endif
#ifndef GPS_DISABLE_GPGSA
    GPS_GPGSA_t GPGSA;
#endif
#ifndef GPS_DISABLE_GPGSV
    GPS_GPGSV_t GPGSV;
#endif
} GPS_Data_t;

void gps_init(void);
GPS_Data_t gps_read(void);

#endif /* GPS_H_ */




#include <Energia.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

typedef enum {
    CONVERSION_SUCCESS,
    CONVERSION_OVERFLOW,
    CONVERSION_UNDERFLOW,
    CONVERSION_INCONVERTIBLE
} Conversion_Errno;

#define MESSAGE_ID_LENGTH   7

#define MAX_SENTENCE_LENGTH 82
#define SENTENCES_PER_READ  12

static GPS_Data_t data;
static char serialBuffer[MAX_SENTENCE_LENGTH * SENTENCES_PER_READ];
static char sentenceBuffer[MAX_SENTENCE_LENGTH];

static char delimiter = ',';

static void empty_serial_buffer(void);
static Conversion_Errno str2int(int *out, const char *s, int base);
static int parse_int(const char *s, int size, int base);
static char parse_char(const char* s);
static double parse_double(const char *s);
static void error_handler(void);
static uint8_t tokenise(const char *s, char delim, char out[MAX_WORDS_IN_SENTENCE][MAX_CHARS_IN_WORD]);


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
        uint16_t i;
        for (i = 0; serialBuffer[gprmcStartIndex + i] != 13; i++) {
            sentenceBuffer[i] = serialBuffer[gprmcStartIndex + i];
        }

        // Null-terminate the array
        sentenceBuffer[i] = '\0';
    }

    Serial.println(sentenceBuffer);

    char messageId[MESSAGE_ID_LENGTH];
    char tokens[MAX_WORDS_IN_SENTENCE][MAX_CHARS_IN_WORD];
    int numTokens = tokenise(sentenceBuffer, delimiter, tokens);

    if (numTokens == 0) {
        error_handler();
    }
    
    char word[MAX_CHARS_IN_WORD];
    strncpy(messageId, tokens[0], sizeof(messageId));
    for (uint8_t position = MESSAGE_ID; position <= CHECKSUM; position++) {
        strncpy(word, tokens[position], sizeof(word));

        Serial.println(word);

        if (strcmp(messageId, GPS_MESSAGE_ID_GPRMC) == 0) {
            switch (position) {
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
    }

    // Calculate checksum. XOR all values successfully between $ and * non-inclusive
    uint16_t checksum = 0;
    for (uint16_t i = 1; sentenceBuffer[i] != '*'; i++) {
        checksum ^= sentenceBuffer[i];
    }
    Serial.print("Calculated Checksum: ");
    Serial.println(checksum, HEX);
}

void gps_print(void) {
    // Time
    Serial.print("Hours: ");
    Serial.println(data.GPRMC.Time.Hours);
    Serial.print("Minutes: ");
    Serial.println(data.GPRMC.Time.Minutes);
    Serial.print("Seconds: ");
    Serial.println(data.GPRMC.Time.Seconds);
    Serial.print("Hundredths: ");
    Serial.println(data.GPRMC.Time.Hundredths);

    // Status
    Serial.print("Status: ");
    Serial.println(data.GPRMC.Status);

    // Latitude
    Serial.print("Latitude: ");
    Serial.println(data.GPRMC.Latitude, 5);

    // North-South Indicator
    Serial.print("NorthSouthIndicator: ");
    Serial.println(data.GPRMC.NorthSouthIndicator);

    // Longitude
    Serial.print("Longitude: ");
    Serial.println(data.GPRMC.Longitude, 5);

    // East-West Indicator
    Serial.print("EastWestIndicator: ");
    Serial.println(data.GPRMC.EastWestIndicator);

    // Speed
    Serial.print("Speed: ");
    Serial.println(data.GPRMC.Speed, 5);

    // Course
    Serial.print("Course: ");
    Serial.println(data.GPRMC.Course, 5);

    // Date
    Serial.print("Day: ");
    Serial.println(data.GPRMC.Date.Day);
    Serial.print("Month: ");
    Serial.println(data.GPRMC.Date.Month);
    Serial.print("Year: ");
    Serial.println(data.GPRMC.Date.Year);

    // Variation
    Serial.print("Variation: ");
    Serial.println(data.GPRMC.Variation);

    // Checksum
    Serial.print("Checksum: ");
    Serial.println(data.GPRMC.Checksum, HEX);

    Serial.println();
}

static void empty_serial_buffer(void) {
    while (Serial2.available()) Serial2.read();
}

/**
 * Shamelessly taken from https://stackoverflow.com/a/12923949/4912373
 */
static Conversion_Errno str2int(int *out, const char *s, int base) {
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

static int parse_int(const char *s, int size, int base) {
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

static char parse_char(const char* s) {
    return s[0];
}

static double parse_double(const char *s) {
//  Serial.print("Parsing double: ");
//  Serial.println(s);
  
//  double d1 = strtod("123.0",NULL);
//  Serial.println(d1);
    return strtod(s, NULL);
//    
//    if (sscanf(s, "%lf", &result) <= 0) {}
//      //error_handler();
//
//    return result;
}

static void error_handler(void) {
    //Serial.println("Uh oh spaghettio");
}

/**
 * Inspired by https://gist.github.com/afiedler/662786
 */
static uint8_t tokenise(const char *s, char delim, char out[MAX_WORDS_IN_SENTENCE][MAX_CHARS_IN_WORD]) {
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






#include <Energia.h>

char buff[4096];
char on = 0;

int led = 0;
void setup() {
  pinMode(RED_LED, OUTPUT);
  Serial.begin(9600);
  delay(100);
  Serial2.begin(9600);
  delay(100);
  gps_init();
}

//void loop(){
//  while (Serial2.available()) {
//    char c = Serial2.read();
//    Serial.print(c);
//  }
//}

void loop() {
  digitalWrite(RED_LED, led);
  led ^= 0x01;
  gps_read();
  gps_print();
  Serial.flush();
  //Serial.println(loopCount);
  delay(1000);
//  if (on) {
//    on = 0;
//    Serial.println();
//    Serial.println("##################################");
//    Serial.println();
//  }
}
