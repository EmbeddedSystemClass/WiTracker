#include <Energia.h>
#include <stdint.h>

typedef enum {
    DATA_SEGMENTS = 1,
    SENTENCES = 6,
    MAX_SENTENCE_SIZE = 82
} Gps_Buffer_Format;

typedef struct {
	uint8_t Date;  /*!< Date in month from GPS. */
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
#ifndef GPS_DISABLE_GPGGA
	float Latitude;                                       /*!< Latitude position from GPS, -90 to 90 degrees response. */
	float Longitude;                                      /*!< Longitude position from GPS, -180 to 180 degrees response. */
	uint8_t Satellites;                                   /*!< Number of satellites in use for GPS position. */
	uint8_t Fix;                                          /*!< GPS fix; 0: Invalid; 1: GPS Fix; 2: DGPS Fix. */
	float Altitude;                                       /*!< Altitude above the sea. */
	TM_GPS_Time_t Time;                                   /*!< Current time from GPS. @ref TM_GPS_Time_t. */
#endif
#ifndef GPS_DISABLE_GPRMC
	TM_GPS_Date_t Date;                                   /*!< Current data from GPS. @ref TM_GPS_Date_t. */
	float Speed;                                          /*!< Speed in knots from GPS. */
	uint8_t Validity;                                     /*!< GPS validation; 1: valid; 0: invalid. */
	float Direction;                                      /*!< Course on the ground in relation to North. */
#endif
#ifndef GPS_DISABLE_GPGSA
	float HDOP;                                           /*!< Horizontal dilution of precision. */
	float PDOP;                                           /*!< Position dilution od precision. */
	float VDOP;                                           /*!< Vertical dilution of precision. */
	uint8_t FixMode;                                      /*!< Current fix mode in use:; 1: Fix not available; 2: 2D; 3: 3D. */
	uint8_t SatelliteIDs[12];                             /*!< Array with IDs of satellites in use. 
	                                                           Only first data are valid, so if you have 5 satellites in use, only SatelliteIDs[4:0] are valid */
#endif
#ifndef GPS_DISABLE_GPGSV	
	uint8_t SatellitesInView;                             /*!< Number of satellites in view */
	TM_GPS_Satellite_t SatDesc[30];                       /*!< Description of each satellite in view */ 
#endif
} GPS_Data_t;

char gpsData[DATA_SEGMENTS][SENTENCES][82];
char gpsReadChar;

uint8_t currSeg = 0;
uint8_t currSent = 0;
uint8_t currChar = 0;

void setup() {
    Serial.begin(9600);
    Serial2.begin(9600);

    for (uint8_t i = 0; i < DATA_SEGMENTS; i++) {
        for (uint8_t j = 0; j < SENTENCES; j++) {
            for (uint8_t k = 0; k < MAX_SENTENCE_SIZE; k++) {
                gpsData[i][j][k] = 0;
            }
        }
    }

    
}

void loop() {
    while (Serial2.available()) {
        gpsReadChar = Serial2.read();
        if 
    }
}

