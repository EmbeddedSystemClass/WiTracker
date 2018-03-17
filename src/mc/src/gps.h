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
