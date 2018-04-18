/*
 * VEML6075.h
 *
 * Arduino library for the Vishay VEML6075 UVA/UVB i2c sensor.
 *
 * Author: Sean Caulfield <sean@yak.net>
 * License: GPLv2.0
 *
 */

#ifndef _VEML6075_H
#define _VEML6075_H

#include <Arduino.h>
#include <Wire.h>

#define VEML6075_ADDR  0x10
#define VEML6075_DEVID 0x26

// Reading the application note on calculation of UV index, the "dummy" channel
// value is actually not a dummy value at all, but the dark current count.
// NAMES ARE IMPORTANT PEOPLE.

#define VEML6075_REG_CONF        (0x00) // Configuration register (options below)
#define VEML6075_REG_UVA         (0x07) // UVA register
#define VEML6075_REG_DUMMY       (0x08) // Dark current register (NOT DUMMY)
#define VEML6075_REG_UVB         (0x09) // UVB register
#define VEML6075_REG_UVCOMP1     (0x0A) // Visible compensation register
#define VEML6075_REG_UVCOMP2     (0x0B) // IR compensation register
#define VEML6075_REG_DEVID       (0x0C) // Device ID register

#define VEML6075_CONF_IT_50MS    (0x00) // Integration time = 50ms (default)
#define VEML6075_CONF_IT_100MS   (0x10) // Integration time = 100ms
#define VEML6075_CONF_IT_200MS   (0x20) // Integration time = 200ms
#define VEML6075_CONF_IT_400MS   (0x30) // Integration time = 400ms
#define VEML6075_CONF_IT_800MS   (0x40) // Integration time = 800ms
#define VEML6075_CONF_IT_MASK    (0x8F) // Mask off other config bits

#define VEML6075_CONF_HD_NORM    (0x00) // Normal dynamic seetting (default)
#define VEML6075_CONF_HD_HIGH    (0x08) // High dynamic seetting

#define VEML6075_CONF_TRIG       (0x04) // Trigger measurement, clears by itself

#define VEML6075_CONF_AF_OFF     (0x00) // Active force mode disabled (default)
#define VEML6075_CONF_AF_ON      (0x02) // Active force mode enabled (?)

#define VEML6075_CONF_SD_OFF     (0x00) // Power up
#define VEML6075_CONF_SD_ON      (0x01) // Power down

// To calculate the UV Index, a bunch of empirical/magical coefficients need to
// be applied to UVA and UVB readings to get a proper composite index value.
// Seems pretty hand wavey, though not nearly as annoying as the dark current
// not being subtracted out by default.

#define VEML6075_UVI_UVA_VIS_COEFF (3.33)
#define VEML6075_UVI_UVA_IR_COEFF  (2.5)
#define VEML6075_UVI_UVB_VIS_COEFF (3.66)
#define VEML6075_UVI_UVB_IR_COEFF  (2.75)

// Once the above offsets and crunching is done, there's a last weighting
// function to convert the ADC counts into the UV index values. This handles
// both the conversion into irradiance (W/m^2) and the skin erythema weighting
// by wavelength--UVB is way more dangerous than UVA, due to shorter
// wavelengths and thus more energy per photon. These values convert the compensated values 

#define VEML6075_UVI_UVA_RESPONSE (1.0 / 909.0)
#define VEML6075_UVI_UVB_RESPONSE (1.0 / 800.0)

enum veml6075_int_time {
  VEML6075_IT_50MS,
  VEML6075_IT_100MS,
  VEML6075_IT_200MS,
  VEML6075_IT_400MS,
  VEML6075_IT_800MS
};
typedef enum veml6075_int_time veml6075_int_time_t;

class VEML6075 {

  public:

    VEML6075();
    bool begin();

    void poll();
    float getUVA();
    float getUVB();
    float getUVIndex();
    uint16_t getDevID();

    uint16_t getRawUVA();
    uint16_t getRawUVB();
    uint16_t getRawDark();
    uint16_t getRawVisComp();
    uint16_t getRawIRComp();

    void setIntegrationTime(veml6075_int_time_t it);

  private:

    uint8_t config;

    uint16_t raw_uva;
    uint16_t raw_uvb;
    uint16_t raw_dark;
    uint16_t raw_vis;
    uint16_t raw_ir;

    uint16_t read16(uint8_t reg);
    void write16(uint8_t reg, uint16_t data);

};

#endif

/*
 * VEML6075.cpp
 *
 * Arduino library for the Vishay VEML6075 UVA/UVB i2c sensor.
 *
 * Author: Sean Caulfield <sean@yak.net>
 * 
 * License: GPLv2.0
 *
 */


VEML6075::VEML6075() {

  // Despite the datasheet saying this isn't the default on startup, it appears
  // like it is. So tell the thing to actually start gathering data.
  this->config = 0;
  this->config |= VEML6075_CONF_SD_OFF;

  // App note only provided math for this one...
  this->config |= VEML6075_CONF_IT_100MS;
}

bool VEML6075::begin() {

  Wire.begin();
  if (this->getDevID() != VEML6075_DEVID) {
    return false;
  }

  // Write config to make sure device is enabled
  this->write16(VEML6075_REG_CONF, this->config);

  return true;
}

// Poll sensor for latest values and cache them
void VEML6075::poll() {
  this->raw_uva = this->read16(VEML6075_REG_UVA);
  this->raw_uvb = this->read16(VEML6075_REG_UVB);
  this->raw_dark = this->read16(VEML6075_REG_DUMMY);
  this->raw_vis = this->read16(VEML6075_REG_UVCOMP1);
  this->raw_ir = this->read16(VEML6075_REG_UVCOMP2);
}

uint16_t VEML6075::getRawUVA() {
  return this->raw_uva;
}

uint16_t VEML6075::getRawUVB() {
  return this->raw_uvb;
}

uint16_t VEML6075::getRawDark() {
  return this->raw_dark;
}

uint16_t VEML6075::getRawVisComp() {
  return this->raw_vis;
}

uint16_t VEML6075::getRawIRComp() {
  return this->raw_ir;
}


uint16_t VEML6075::getDevID() {
  return this->read16(VEML6075_REG_DEVID);
}

float VEML6075::getUVA() {
  float comp_vis = this->raw_vis - this->raw_dark;
  float comp_ir = this->raw_ir - this->raw_dark;
  float comp_uva = this->raw_uva - this->raw_dark;

  comp_uva -= (VEML6075_UVI_UVA_VIS_COEFF * comp_vis) - (VEML6075_UVI_UVA_IR_COEFF * comp_ir);

  return comp_uva;
}

float VEML6075::getUVB() {
  float comp_vis = this->raw_vis - this->raw_dark;
  float comp_ir = this->raw_ir - this->raw_dark;
  float comp_uvb = this->raw_uvb - this->raw_dark;

  comp_uvb -= (VEML6075_UVI_UVB_VIS_COEFF * comp_vis) - (VEML6075_UVI_UVB_IR_COEFF * comp_ir);

  return comp_uvb;
}

float VEML6075::getUVIndex() {
  float uva_weighted = this->getUVA() * VEML6075_UVI_UVA_RESPONSE;
  float uvb_weighted = this->getUVB() * VEML6075_UVI_UVB_RESPONSE;
  return (uva_weighted + uvb_weighted) / 2.0;
}

uint16_t VEML6075::read16(uint8_t reg) {
  uint8_t msb = 0;
  uint8_t lsb = 0;

  Wire.beginTransmission(VEML6075_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);

  Wire.requestFrom(VEML6075_ADDR, 2, true);
  lsb = Wire.read();
  msb = Wire.read();

  return (msb << 8) | lsb;
}

void VEML6075::write16(uint8_t reg, uint16_t data) {
  Wire.beginTransmission(VEML6075_ADDR);
  Wire.write(reg);
  Wire.write((uint8_t)(0xFF & (data >> 0))); // LSB
  Wire.write((uint8_t)(0xFF & (data >> 8))); // MSB
  Wire.endTransmission();
}





/*
 * test-veml6075.ino
 *
 * Simpel sketch to test if a given board words.
 *
 * Author: Sean Caulfield <sean@yak.net>
 * License: GPLv2.0
 *
 */

#include <Arduino.h>
#include <Wire.h>

VEML6075 veml6075 = VEML6075();
bool found = false;

void setup() {
  delay(1000);
  Serial.begin(57600);
  Wire.begin();
  if (!veml6075.begin()) {
    Serial.println(F("VEML6075 not found!"));
  } else {
    found = true;
  }
}

void loop() {

  if (found) {
    float value;

    // Poll sensor
    veml6075.poll();

    Serial.print(F("t = "));
    Serial.println(millis());

    value = veml6075.getUVA();
    Serial.print(F("UVA = "));
    Serial.println(value, 2);

    value = veml6075.getUVB();
    Serial.print(F("UVB = "));
    Serial.println(value, 2);

    value = veml6075.getUVIndex();
    Serial.print(F("UV Index = "));
    Serial.println(value, 1);

    uint16_t devid = veml6075.getDevID();
    Serial.print(F("Device ID = "));
    Serial.println(devid, HEX);

    Serial.println(F("----------------"));
  }

    delay(1000);
}
