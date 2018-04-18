/*

Arduino Library for Texas Instruments HDC1010 Digital Humidity and Temperature Sensor
Written by AA for ClosedCube
---

The MIT License (MIT)

Copyright (c) 2016-2017 ClosedCube Limited

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef _CLOSEDCUBE_HDC1010_h

#define _CLOSEDCUBE_HDC1010_h
#include <Arduino.h>

typedef enum {
  TEMPERATURE = 0x00,
  HUMIDITY = 0x01,
  CONFIGURATION = 0x02,
  MANUFACTURER_ID = 0xFE,
  DEVICE_ID = 0xFF,
  SERIAL_ID_FIRST = 0xFB,
  SERIAL_ID_MID = 0xFC,
  SERIAL_ID_LAST = 0xFD,
} HDC1010_Pointers;

typedef union {
  uint8_t rawData;
  struct {
    uint8_t HumidityMeasurementResolution : 2;
    uint8_t TemperatureMeasurementResolution : 1;
    uint8_t BatteryStatus : 1;
    uint8_t ModeOfAcquisition : 1;
    uint8_t Heater : 1;
    uint8_t ReservedAgain : 1;
    uint8_t SoftwareReset : 1;
  };
} HDC1010_Registers;


class ClosedCube_HDC1010 {
public:
  ClosedCube_HDC1010();

  void begin(uint8_t address);
  uint16_t readManufacturerId(); // 0x5449 ID of Texas Instruments
  uint16_t readDeviceId(); // 0x1000 ID of the device

  HDC1010_Registers readRegister(); 
  void writeRegister(HDC1010_Registers reg);

  void heatUp(uint8_t seconds);

  float readTemperature();
  float readHumidity();

  float readT(); // short-cut for readTemperature
  float readH(); // short-cut for readHumidity

private:
  uint8_t _address;
  uint16_t readData(uint8_t pointer);

  const double pow16 = pow(2, 16);
};

#endif


/*

Arduino Library for Texas Instruments HDC1010 Digital Humidity and Temperature Sensor
Written by AA for ClosedCube
---

The MIT License (MIT)

Copyright (c) 2016-2017 ClosedCube Limited

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include <Wire.h>



ClosedCube_HDC1010::ClosedCube_HDC1010()
{
}

void ClosedCube_HDC1010::begin(uint8_t address) {
  _address = address;
  Wire.begin();

  // heater:off, mode: T or RH, T 14 bit, RH 14 bit
  Wire.beginTransmission(_address);
  Wire.write(CONFIGURATION);
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.endTransmission();
}

float ClosedCube_HDC1010::readT() {
  return readTemperature();
}

float ClosedCube_HDC1010::readTemperature() {
  uint16_t rawT = readData(TEMPERATURE);
  return (rawT / pow16) * 165 - 40;
}

float ClosedCube_HDC1010::readH() {
  return readHumidity();
}

float ClosedCube_HDC1010::readHumidity() {
  uint16_t rawH = readData(HUMIDITY);
  return (rawH / pow16) * 100;
}

uint16_t ClosedCube_HDC1010::readManufacturerId() {
  return readData(MANUFACTURER_ID);
}

HDC1010_Registers ClosedCube_HDC1010::readRegister() {
  HDC1010_Registers reg;
  reg.rawData = (readData(CONFIGURATION) >> 8);
  return reg;
}

void ClosedCube_HDC1010::writeRegister(HDC1010_Registers reg) {
  Wire.beginTransmission(_address);
  Wire.write(CONFIGURATION);
  Wire.write(reg.rawData);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(10);
}

void ClosedCube_HDC1010::heatUp(uint8_t seconds) {
  HDC1010_Registers reg = readRegister();
  reg.Heater = 1;
  reg.ModeOfAcquisition = 1;
  writeRegister(reg);

  uint8_t buf[4];
  for (int i = 1; i < (seconds*66); i++) {
    Wire.beginTransmission(_address);
    Wire.write(0x00);
    Wire.endTransmission();
    delay(20);
    Wire.requestFrom(_address, (uint8_t)4);
    Wire.readBytes(buf, (size_t)4);
  }
  reg.Heater = 0;
  reg.ModeOfAcquisition = 0;
  writeRegister(reg);
}

uint16_t ClosedCube_HDC1010::readDeviceId() {
  return readData(DEVICE_ID);
}


uint16_t ClosedCube_HDC1010::readData(uint8_t pointer) {
  Wire.beginTransmission(_address);
  Wire.write(pointer);
  Wire.endTransmission();

  delay(8); // 6.35/6.50 typ conversion time for T/RH sensors
  Wire.requestFrom(_address, (uint8_t)2);

  byte msb = Wire.read();
  byte lsb = Wire.read();

  return msb << 8 | lsb;
}




/**************************************************************************************

This is example for ClosedCube HDC1010 Humidity and Temperature Sensor breakout booard

Initial Date: 20-Oct-2016

Hardware connections for Arduino Uno:
VDD to 3.3V DC
SCL to A5
SDA to A4
GND to common ground

Written by AA for ClosedCube

MIT License

**************************************************************************************/

#include <Wire.h>

#define HDC1010_I2C_ADDRESS 0x48

ClosedCube_HDC1010 hdc1010;

void setup()
{
  Serial.begin(9600);
  Serial.println("ClosedCube HDC1010 Arduino Test");

  hdc1010.begin(HDC1010_I2C_ADDRESS);

  Serial.print("Manufacturer ID=0x");
  Serial.println(hdc1010.readManufacturerId(), HEX); // 0x5449 ID of Texas Instruments
  Serial.print("Device ID=0x");
  Serial.println(hdc1010.readDeviceId(), HEX); // 0x1000 ID of the device
  Serial.println();

  printRegister(hdc1010.readRegister());

}

void loop()
{
  Serial.print("T=");
  Serial.print(hdc1010.readTemperature());
  Serial.print("C, RH=");
  Serial.print(hdc1010.readHumidity());
  Serial.println("%");
  delay(300);
}

void printRegister(HDC1010_Registers reg) {
  Serial.println("HDC1010 Configuration Register");
  Serial.println("------------------------------");
  
  Serial.print("Software reset bit: ");
  Serial.print(reg.SoftwareReset, BIN);
  Serial.println(" (0=Normal Operation, 1=Software Reset)");

  Serial.print("Heater: ");
  Serial.print(reg.Heater, BIN);
  Serial.println(" (0=Disabled, 1=Enabled)");

  Serial.print("Mode of Acquisition: ");
  Serial.print(reg.ModeOfAcquisition, BIN);
  Serial.println(" (0=T or RH is acquired, 1=T and RH are acquired in sequence, T first)");

  Serial.print("Battery Status: ");
  Serial.print(reg.BatteryStatus, BIN);
  Serial.println(" (0=Battery voltage > 2.8V, 1=Battery voltage < 2.8V)");

  Serial.print("T Measurement Resolution: ");
  Serial.print(reg.TemperatureMeasurementResolution, BIN);
  Serial.println(" (0=14 bit, 1=11 bit)");

  Serial.print("RH Measurement Resolution: ");
  Serial.print(reg.HumidityMeasurementResolution, BIN);
  Serial.println(" (00=14 bit, 01=11 bit, 10=8 bit)");
}
