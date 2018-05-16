/*
  Si7006 Temperature and humidity sensor library for Arduino
  Lovelesh, thingTronics
  
The MIT License (MIT)

Copyright (c) 2015 thingTronics Limited

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

version 0.1
*/

#ifndef Si7006_h
#define Si7006_h

#include "Arduino.h"

#define Si7006_ADDR   0x40 // default address

// Si7006 register addresses
#define Si7006_MEAS_REL_HUMIDITY_MASTER_MODE    0xE5
#define Si7006_MEAS_REL_HUMIDITY_NO_MASTER_MODE 0xF5
#define Si7006_MEAS_TEMP_MASTER_MODE            0xE3
#define Si7006_MEAS_TEMP_NO_MASTER_MODE         0xF3
#define Si7006_READ_OLD_TEMP                    0xE0
#define Si7006_RESET              0xFE
#define Si7006_WRITE_HUMIDITY_TEMP_CONTR    0xE6            
#define Si7006_READ_HUMIDITY_TEMP_CONTR     0xE7
#define Si7006_WRITE_HEATER_CONTR       0x51
#define Si7006_READ_HEATER_CONTR        0x11
#define Si7006_READ_ID_LOW_0          0xFA
#define Si7006_READ_ID_LOW_1          0x0F
#define Si7006_READ_ID_HIGH_0         0xFC
#define Si7006_READ_ID_HIGH_1         0xC9
#define Si7006_FIRMWARE_0           0x84
#define Si7006_FIRMWARE_1           0xB8


class Si7006 {
  public:
    Si7006(void);
      // Si7006 object
      
    boolean begin();
      // Initialize Si7006 library with default address (0x40)
      // Always returns true
      
    boolean reset(void);
      // SW Reset the sensor
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() below)
      
    boolean getTempControl(byte &res, boolean voltage, boolean heater);
      // Gets the contents RH/Temp User Register of the sensor
      // res uses D7 and D0 bit
      // If res = 0, RH is set to 12 bit & temp 14 bit resolution (default)
      // If res = 1, RH is set to 8 bit & temp 12 bit resolution
      // If res = 2, RH is set to 10 bit & temp 13 bit resolution
      // If res = 4, RH is set to 11 bit & temp 11 bit resolution
      //----------------------------------------------------------
      // If voltage = false(0), VDD OK (default)
      // If voltage = true(1), VDD LOW
      //----------------------------------------------------------
      // If heater = false(0), On-chip Heater is disabled (default)
      // If heater = true(1), On-chip Heater is disabled
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() below)
      
    boolean setTempControl(byte res, boolean heater);
      // Sets the contents RH/Temp User Register of the sensor
      // Gets the contents RH/Temp User Register of the sensor
      // res uses D7 and D0 bit
      // If res = 0, RH is set to 12 bit & temp 14 bit resolution (default)
      // If res = 1, RH is set to 8 bit & temp 12 bit resolution
      // If res = 2, RH is set to 10 bit & temp 13 bit resolution
      // If res = 4, RH is set to 11 bit & temp 11 bit resolution
      //----------------------------------------------------------
      // If heater = false(0), On-chip Heater is disabled (default)
      // If heater = true(1), On-chip Heater is disabled
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() below)
      
    boolean getHeaterControl(byte &heaterCurrent);
      // Gets the Heater current of the On-chip Heater
      // If heaterCurrent = 0, Heater current is 3.09mA (default)
      // If heaterCurrent = 15, Heater current is 94.20mA
      // heaterCurrent is in multiples of 3.09mA
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() below)
      
    boolean setHeaterControl(byte heaterCurrent);
      // Sets the Heater current of the On-chip Heater
      // If heaterCurrent = 0, Heater current is 3.09mA (default)
      // If heaterCurrent = 15, Heater current is 94.20mA
      // heaterCurrent is in multiples of 3.09mA
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() below)
    

    boolean getDeviceID(char (&deviceID)[8]);
      // Gets the Device ID of the chip
      // Default value of MSB 0x06
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() below)

    boolean getFirmwareVer(byte &firmware);
      // Gets the Firmware Version of the chip
      // Default value is 0xFF for version 1.0
      // or 0x20 for version 2.0
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() below)
      
    boolean getTemperature(float &temperature, boolean mode);
      // Gets the Temperature data from the sensor
      // If mode = true(1), Hold Master Mode is used
      // If mode = false(0), No Hold Master Mode is used
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() below)
      
    boolean getHumidity(float &humidity, boolean mode);
      // Gets the Humidity data from the sensor
      // If mode = true(1), Hold Master Mode is used
      // If mode = false(0), No Hold Master Mode is used
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() below)
      
    boolean getOldTemperature(float &temperature);
      // Gets the Old Temperature data from the sensor
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() below)
      
    uint8_t crc8(const uint8_t *data, int len);
      // Returns the CRC byte generated from the data
      
    byte getError(void);
      // If any library command fails, you can retrieve an extended
      // error code using this command. Errors are from the wire library: 
      // 0 = Success
      // 1 = Data too long to fit in transmit buffer
      // 2 = Received NACK on transmit of address
      // 3 = Received NACK on transmit of data
      // 4 = Other error
    
  private:
    
    boolean readByte(byte address, byte &value);
      // Reads a byte from a LTR303 address
      // Address: LTR303 address (0 to 15)
      // Value will be set to stored byte
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() above)
  
    boolean writeByte(byte address, byte value);
      // Write a byte to a LTR303 address
      // Address: LTR303 address (0 to 15)
      // Value: byte to write to address
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() above)

    boolean readUInt(byte address, unsigned int &value);
      // Reads an unsigned integer (16 bits) from a LTR303 address (low byte first)
      // Address: LTR303 address (0 to 15), low byte first
      // Value will be set to stored unsigned integer
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() above)

    boolean read1ByteData(byte address1, byte address2, byte &value);
      // Reads a byte from a Si7006 sensor when provided with 2 addresses
      // Address: Si7006 address (0 to 15)
      // Value will be set to stored byte
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() above)
      
    boolean read4ByteData(byte address1, byte address2, char (&value)[4]);
      // Reads an unsigned long (32 bits) from a Si7006 address (high byte first)
      // Address: Si7006 register address (0 to 15), high byte first
      // Value will be set to stored unsigned long
      // Returns true (1) if successful, false (0) if there was an I2C error
      // (Also see getError() above)
    
    byte _i2c_address;
    byte _error;
};

#endif



/*
  Si7006 Temperature and humidity sensor library for Arduino
  Lovelesh, thingTronics
  
The MIT License (MIT)

Copyright (c) 2015 thingTronics Limited

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

version 0.1
*/


#include <Wire.h>

Si7006::Si7006(void) {
  // Si7006 object
}
      
boolean Si7006::begin(void) {
  // Initialize Si7006 library with default address (0x40)
  // Always returns true
  
  _i2c_address = Si7006_ADDR;
  //Wire.begin();
  return(true);
}
      
    
boolean Si7006::reset(void) {
  // SW Reset the sensor
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
  
  // In reset we write default value to USER Register
  byte res = 0x00;
  boolean heater = false;
  if(setTempControl(res, heater)) {
    delay(15);
    return(true);
  }
  
  return(false);
}
      
boolean Si7006::getTempControl(byte &res, boolean voltage, boolean heater) {
  // Gets the contents RH/Temp User Register of the sensor
  // res uses D7 and D0 bit
  // If res = 0, RH is set to 12 bit & temp 14 bit resolution (default)
  // If res = 1, RH is set to 8 bit & temp 12 bit resolution
  // If res = 2, RH is set to 10 bit & temp 13 bit resolution
  // If res = 4, RH is set to 11 bit & temp 11 bit resolution
  //----------------------------------------------------------
  // If voltage = false(0), VDD OK (default)
  // If voltage = true(1), VDD LOW
  //----------------------------------------------------------
  // If heater = false(0), On-chip Heater is disabled (default)
  // If heater = true(1), On-chip Heater is disabled
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
      
  byte control, humidity;
  
  // Reading the control byte
  if(readByte(Si7006_READ_HUMIDITY_TEMP_CONTR,control)) {
    // Extract resolution
    // extracting D7 control bit into D1 res; D0 control bit into D0 res 
    res = ((control & 0x80) >> 6) | (control & 0x01);
    
    // Extract voltage
    voltage = (control & 0x40) ? true : false; 
    
    // Extract heater
    heater = (control & 0x04) ? true : false;
    
    // return if successful
    return(true);
  }
  return(false);    
}

boolean Si7006::setTempControl(byte res, boolean heater) {
  // Sets the contents RH/Temp User Register of the sensor
  // Gets the contents RH/Temp User Register of the sensor
  // res uses D7 and D0 bit
  // If res = 0, RH is set to 12 bit & temp 14 bit resolution (default)
  // If res = 1, RH is set to 8 bit & temp 12 bit resolution
  // If res = 2, RH is set to 10 bit & temp 13 bit resolution
  // If res = 4, RH is set to 11 bit & temp 11 bit resolution
  //----------------------------------------------------------
  // If heater = false(0), On-chip Heater is disabled (default)
  // If heater = true(1), On-chip Heater is disabled
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
  
  byte control = 0x00;
  
  // sanity check for gain
  if (res > 4) {
    res = 0x00;
  }
  
  // control byte logic
  control |= (res & 0x02) << 6 | (res & 0x01);
  
  if(heater) {
    control |= 0x04;
  }
  
  return(writeByte(Si7006_WRITE_HUMIDITY_TEMP_CONTR,control));
}     
      
boolean Si7006::getHeaterControl(byte &heaterCurrent) {
  // Gets the Heater current of the On-chip Heater
  // If heaterCurrent = 0, Heater current is 3.09mA (default)
  // If heaterCurrent = 15, Heater current is 94.20mA
  // heaterCurrent is in multiples of 3.09mA
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
  
  // Reading the status byte
  if(readByte(Si7006_READ_HEATER_CONTR,heaterCurrent)) {
    // Extract heater current
    heaterCurrent |= heaterCurrent & 0x0F; 
    
    // return if successful
    return(true);
  }
  return(false);
}
      
boolean Si7006::setHeaterControl(byte heaterCurrent){
  // Sets the Heater current of the On-chip Heater
  // If heaterCurrent = 0, Heater current is 3.09mA (default)
  // If heaterCurrent = 15, Heater current is 94.20mA
  // heaterCurrent is in multiples of 3.09mA
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
  
  // sanity check
  if(heaterCurrent >= 15) {
    heaterCurrent = 0x15;
  }
  return(writeByte(Si7006_WRITE_HEATER_CONTR,heaterCurrent));
}

boolean Si7006::getDeviceID(char (&deviceID)[8]) {
  // Gets the Device ID of the chip
  // Default value of MSB 0x06
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
  
  char tempDeviceID[4];
  // read first 4 bytes
  if(read4ByteData(Si7006_READ_ID_LOW_0,Si7006_READ_ID_LOW_1,tempDeviceID)) {
    strcpy(deviceID, tempDeviceID);
    //deviceID <<= 32;
    
    // read the next 4 bytes
    if(read4ByteData(Si7006_READ_ID_HIGH_0,Si7006_READ_ID_HIGH_1,tempDeviceID)) {
      strcat(deviceID, tempDeviceID);
      //deviceID += double(tempDeviceID) * pow(2,32);
      // return if successful
      return(true);
    }
  }
  return(false);
}

boolean Si7006::getFirmwareVer(byte &firmware) {
  // Gets the Firmware Version of the chip
  // Default value is 0xFF for version 1.0
  // or 0x20 for version 2.0
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
  
  return(read1ByteData(Si7006_FIRMWARE_0, Si7006_FIRMWARE_1, firmware));
}
      
boolean Si7006::getTemperature(float &temperature, boolean mode) {
  // Gets the Temperature data from the sensor
  // If mode = true(1), Hold Master Mode is used
  // If mode = false(0), No Hold Master Mode is used
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
  
  boolean success = false;
  unsigned int tempTemperature;
  
  if(mode) {
    if(!readUInt(Si7006_MEAS_TEMP_MASTER_MODE,tempTemperature)) {
      return(false);
    }
  }
  else {
    if(!readUInt(Si7006_MEAS_TEMP_NO_MASTER_MODE,tempTemperature)) {
       Serial.println("HEYEYYYE");

      return(false);
    }
  }
  
  // Check if temperature is correct by ANDing with 0xFFFC
  if(tempTemperature & 0xFFFC) {
    temperature = (175.72 * (float)tempTemperature)/65536 - 46.85;
    return(true);
  }
  
  return(false);
}   
    
boolean Si7006::getOldTemperature(float &temperature) {
  // Gets the Old Temperature data from the sensor
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
  
  unsigned int tempTemperature;
  if(readUInt(Si7006_READ_OLD_TEMP,tempTemperature)) {
    // Check if temperature is correct by ANDing with 0xFFFC
    if(tempTemperature & 0xFFFC) {
      temperature = (175.72 * (float)tempTemperature)/65536 - 46.85;
      return(true);
    }
  }
  
  return(false);
}
    
boolean Si7006::getHumidity(float &humidity, boolean mode) {
  // Gets the Humidity data from the sensor
  // If mode = true(1), Hold Master Mode is used
  // If mode = false(0), No Hold Master Mode is used
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() below)
  
  boolean success = false;
  unsigned int tempHumidity;
  
  if(mode) {
    if(!readUInt(Si7006_MEAS_REL_HUMIDITY_MASTER_MODE,tempHumidity))
      return(false);
  }
  else {
    if(!readUInt(Si7006_MEAS_REL_HUMIDITY_NO_MASTER_MODE,tempHumidity))
      return(false);
  }
  
  // Check if humidity is correct by ANDing with 0xFFFE
  if(tempHumidity & 0xFFFE) {
    humidity = (125 * (float)tempHumidity)/65536 - 6;
    return(true);
  }
  
  return(false);
}
      
byte Si7006::crc8(const uint8_t *data, int len) {
  // Returns the CRC byte generated from the data
  /*
   *
   * CRC-8 formula from page 14 of SHT spec pdf
   *
   * Test data 0xBE, 0xEF should yield 0x92
   *
   * Initialization data 0xFF
   * Polynomial 0x31 (x8 + x5 +x4 +1)
   * Final XOR 0x00
  */

  const uint8_t POLYNOMIAL(0x31);
  uint8_t crc(0xFF);

  for ( int j = len; j; --j ) {
    crc ^= *data++;

    for ( int i = 8; i; --i ) {
  crc = ( crc & 0x80 )
    ? (crc << 1) ^ POLYNOMIAL
    : (crc << 1);
    }
  }
  return crc;
}
      
byte Si7006::getError(void) {
  // If any library command fails, you can retrieve an extended
  // error code using this command. Errors are from the wire library: 
  // 0 = Success
  // 1 = Data too long to fit in transmit buffer
  // 2 = Received NACK on transmit of address
  // 3 = Received NACK on transmit of data
  // 4 = Other error
  
  return(_error);
}
    
// Private Functions:
    
boolean Si7006::readByte(byte address, byte &value) {
  // Reads a byte from a Si7006 address
  // Address: LTR303 address (0 to 15)
  // Value will be set to stored byte
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() above)
  
  // Check if sensor present for read
  Wire.beginTransmission(_i2c_address);
  Wire.write(address);
  _error = Wire.endTransmission();

  // Read requested byte
  if (_error == 0)
  {
    Wire.requestFrom(_i2c_address,1);
    if (Wire.available() == 1)
    {
      value = Wire.read();
      return(true);
    }
  }
  return(false);
}

boolean Si7006::writeByte(byte address, byte value) {
  // Write a byte to a Si7006 address
  // Address: LTR303 address (0 to 15)
  // Value: byte to write to address
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() above)
  
  Wire.beginTransmission(_i2c_address);
  // Write byte
  Wire.write(address);
  Wire.write(value);
  _error = Wire.endTransmission();
  if (_error == 0)
    return(true);

  return(false);
}

boolean Si7006::readUInt(byte address, unsigned int &value) {
  // Reads an unsigned integer (16 bits) from a Si7006 address (high byte first)
  // Address: Si7006 address (0 to 15), high byte first
  // Value will be set to stored unsigned integer
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() above)
  
  byte high, low;
  
  // Check if sensor present for read
  Wire.beginTransmission(_i2c_address);
  Wire.write(address);
  _error = Wire.endTransmission();

  // Read two bytes (low and high)
  if (_error == 0)
  {
    Wire.requestFrom(_i2c_address,2);
    if (Wire.available() == 2)
    {
      high = Wire.read();
      low = Wire.read();
      // Combine bytes into unsigned int
      value = word(high,low);
      return(true);
    }
  } 
  return(false);
}

boolean Si7006::read1ByteData(byte address1, byte address2, byte &value) {
  // Reads a byte from a Si7006 sensor when provided with 2 addresses
  // Address: Si7006 address (0 to 15)
  // Value will be set to stored byte
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() above)
  
  // Check if sensor present for read
  Wire.beginTransmission(_i2c_address);
  Wire.write(address1);
  Wire.write(address2);
  _error = Wire.endTransmission();

  // Read requested byte
  if (_error == 0)
  {
    Wire.requestFrom(_i2c_address,1);
    if (Wire.available() == 1)
    {
      value = Wire.read();
      return(true);
    }
  }
  return(false);
}

boolean Si7006::read4ByteData(byte address1, byte address2, char (&value)[4]) {
  // Reads an unsigned long (32 bits) from a Si7006 address (high byte first)
  // Address: Si7006 register addresses (0 to 15), high byte first
  // Value will be set to stored 4 byte character array
  // Returns true (1) if successful, false (0) if there was an I2C error
  // (Also see getError() above)
  
  // Check if sensor present for read
  Wire.beginTransmission(_i2c_address);
  Wire.write(address1);
  Wire.write(address2);
  _error = Wire.endTransmission();

  // Read requested byte
  if (_error == 0)
  {
    Wire.requestFrom(_i2c_address,4);
    if (Wire.available() == 4)
    {
      value[3] |= Wire.read();
      value[2] |= Wire.read();
      value[1] |= Wire.read();
      value[0] |= Wire.read();
      return(true);
    }
  }
  return(false);
}



/* 
  Si7006-A20 library example sketch
  Lovelesh, thingTronics

This sketch shows how to use the Si7006 library to read the Silicon Labs Si7006-A20 temperature and humidity sensor.

Hardware connections:

3V3 to 3.3V
GND to GND

(WARNING: do not connect 3V3 to 5V or the sensor will be damaged!)

You will also need to connect the I2C pins (SCL and SDA) to your Arduino.
The pins are different on different Arduinos:

                    SDA    SCL
Any Arduino        "SDA"  "SCL"
Uno, Pro            A4     A5
Mega2560, Due       20     21
Leonardo            2      3
ESP8266       5      4

*/
 
#include <Wire.h>

Si7006 tempNHum; 

void temphumid_setup() {
  
  // Initialize the Serial port:  
  Serial.println("Si7006-A20 temphumidity setup");

  // Initialize the Si7006 library
  // You can pass nothing to light.begin() for the default I2C address (0x40)
  //tempNHum.begin();
  
  if (!tempNHum.begin()) {
    Serial.println("Couldn't find Si7006");
  }
  
  // Get factory ID from sensor:
  // (Just for fun, you don't need to do this to operate the sensor)
  
  char ID[8];

  if (tempNHum.getDeviceID(ID)) {
    Serial.print("Got Sensor Part ID: 0X");
    // Default value of MSB 0x06
    for(int i = 0; i < sizeof(ID); i++) {
      Serial.print(ID[i],HEX);
    }
    Serial.println();
    
  }
  // Most library commands will return true if communications was successful,
  // and false if there was a problem. You can ignore this returned value,
  // or check whether a command worked correctly and retrieve an error code:
  else {
    byte error = tempNHum.getError();
    printError(error);
  }
  
  // Gets the Firmware Version of the chip
  // Default value is 0xFF for version 1.0 or 0x20 for version 2.0
  byte firmwareVer;
  
  if(tempNHum.getFirmwareVer(firmwareVer)) {
    Serial.print("Got Sensor Firmware Version: 0X");
    Serial.println(firmwareVer,HEX);
  }
  else {
    byte error = tempNHum.getError();
    printError(error);
  }
  
  // Gets the contents RH/Temp User Register of the sensor
  byte resolution;
  boolean voltage, heaterStatus;
  tempNHum.getTempControl(resolution, voltage, heaterStatus);
  Serial.print("Resolution is: ");
  Serial.println(resolution);
  
  // Setting the resolution and heater disable
  resolution = 0x00;
  heaterStatus = false;
  tempNHum.setTempControl(resolution, heaterStatus);
  
  // Getting heater current
  byte heaterCurrent;
  tempNHum.getHeaterControl(heaterCurrent);
  Serial.print("Heater Current is ");
  Serial.println(heaterCurrent);
  
  // Setting heater current
  tempNHum.setHeaterControl(heaterCurrent);
}


void temphumid_loop() {
  float temp;
  float humidity;
  boolean mode = true;    // if it doesnt work try changing this

  // Read temperature
  if(tempNHum.getTemperature(temp, mode)) {
    Serial.print("Temp *C = ");
    Serial.println(temp);
  }
  else{
    Serial.println("Failed to read temperature");
    byte error = tempNHum.getError();
    printError(error);
  }

  // Read humidity
  if(tempNHum.getHumidity(humidity, mode)) {
    Serial.print("Humidity = ");
    Serial.print(humidity);
    Serial.println("%");
  }
  else {
    Serial.println("Failed to read humidity");
    byte error = tempNHum.getError();
    printError(error);
  }
}

void printError(byte error) {
  // If there's an I2C error, this function will
  // print out an explanation.

  Serial.print("I2C error: ");
  Serial.print(error,DEC);
  Serial.print(", ");
  
  switch(error) {
    case 0:
      Serial.println("success");
      break;
    case 1:
      Serial.println("data too long for transmit buffer");
      break;
    case 2:
      Serial.println("received NACK on address (disconnected?)");
      break;
    case 3:
      Serial.println("received NACK on data");
      break;
    case 4:
      Serial.println("other error");
      break;
    default:
      Serial.println("unknown error");
  }
}











/*!
   @file DFRobot_LIS2DH12.h
   @brief DFRobot's Read LIS2DH12 data
   @n This example achieve receiving LIS2DH12  data via serial

   @copyright  [DFRobot](http://www.dfrobot.com), 2016
   @copyright GNU Lesser General Public License
   @author [Wuxiao](xiao.wu@dfrobot.com)
   @version  V1.0
   @date  2016-10-13
   @https://github.com/DFRobot/DFRobot_LIS2DH12
*/


#ifndef DFRobot_LIS2DH12_h
#define DFRobot_LIS2DH12_h

#include <stdlib.h>
#include <stdint.h>

#define LIS2DH12_RANGE_2GA  0x00
#define LIS2DH12_RANGE_4GA  0x10
#define LIS2DH12_RANGE_8GA  0x20
#define LIS2DH12_RANGE_16GA 0x30

class DFRobot_LIS2DH12 {
  public:
    static uint8_t sensorAddress; ///< IIC address of the sensor
    int8_t init(uint8_t range); ///< Initialization function
    void readXYZ(int16_t&, int16_t&, int16_t&); ///< read x, y, z data
    void mgScale(int16_t&, int16_t&, int16_t&); ///< transform data to millig
    /*!
        Through the I2C to specify register read a single data
    */
    uint8_t readReg(uint8_t);
    /*!
        Through the I2C to specify register read more data
    */
    void readReg(uint8_t, uint8_t *, uint8_t, bool autoIncrement = true);
    /*!
        Write a single data through the I2C to specify register
    */
    uint8_t writeReg(uint8_t, uint8_t);
    /*!
        Through the I2C write multiple data on the specified register
    */
    uint8_t writeReg(uint8_t, uint8_t *, size_t, bool autoIncrement = true);

  private:
    /*!
        Set measurement range
    */
    void setRange(uint8_t range);

    uint8_t mgPerDigit;
    uint8_t  mgScaleVel;
};
#endif


/*!
   @file DFRobot_LIS2DH12.cpp
   @brief DFRobot's Read LIS2DH12 data
   @n This example is in order to achieve the serial port to receive LIS2DH12 back to the data

   @copyright [DFRobot](http://www.dfrobot.com), 2016
   @copyright GNU Lesser General Public License
   @author [Wuxiao](xiao.wu@dfrobot.com)
   @version  V1.0
   @date  2016-10-13
   @https://github.com/DFRobot/DFRobot_LIS2DH12
*/

#include <Wire.h>

uint8_t DFRobot_LIS2DH12::sensorAddress = 0x19; //  0x18

int8_t DFRobot_LIS2DH12::init(uint8_t range)
{
  int8_t ret = 0;

  setRange(range);
  Wire.beginTransmission(sensorAddress);
  ret = Wire.endTransmission();
  if (ret != 0) {
    ret = -1;
  } else {
    uint8_t ctrl_reg_values[] = {0x2F, 0x01, 0x40, range, 0x08, 0x00};
    ret = (int8_t)writeReg(0xA0, ctrl_reg_values, sizeof(ctrl_reg_values));

    // set interrupt
    uint8_t int1_cfg_reg = 0x30;
    uint8_t int1_cfg_reg_values[] = {0x0A};
    writeReg(int1_cfg_reg, int1_cfg_reg_values, sizeof(int1_cfg_reg_values));

    uint8_t int1_ths_reg = 0x32;
    uint8_t int1_ths_reg_values[] = {0x02};
    writeReg(int1_ths_reg, int1_ths_reg_values, sizeof(int1_ths_reg_values));

    uint8_t int1_dur_reg = 0x33;
    uint8_t int1_dur_reg_values[] = {0x02};
    writeReg(int1_dur_reg, int1_dur_reg_values, sizeof(int1_dur_reg_values));

    
  }
  return ret;
}

void DFRobot_LIS2DH12::readXYZ(int16_t &x, int16_t &y, int16_t &z) //read x, y, z data
{
  uint8_t sensorData[6];
  readReg(0xA8, sensorData, 6);
  x = ((int8_t)sensorData[1]) * 256 + sensorData[0]; //return values
  y = ((int8_t)sensorData[3]) * 256 + sensorData[2];
  z = ((int8_t)sensorData[5]) * 256 + sensorData[4];
}

void DFRobot_LIS2DH12::mgScale(int16_t &x, int16_t &y, int16_t &z)
{
  x = (int32_t)x * 1000 / (1024 * mgScaleVel); //transform data to millig, for 2g scale axis*1000/(1024*16),
  y = (int32_t)y * 1000 / (1024 * mgScaleVel); //for 4g scale axis*1000/(1024*8),
  z = (int32_t)z * 1000 / (1024 * mgScaleVel); //for 8g scale axis*1000/(1024*4)
}

uint8_t DFRobot_LIS2DH12::readReg(uint8_t regAddress)
{
  uint8_t regValue;
  Wire.beginTransmission(sensorAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(sensorAddress, (uint8_t)1);
  regValue = Wire.read();
  return regValue;
}

void DFRobot_LIS2DH12::readReg(uint8_t regAddress, uint8_t *regValue, uint8_t quanity, bool autoIncrement)
{
  regAddress = 0x80 | regAddress;
  if (autoIncrement) {
    Wire.beginTransmission(sensorAddress);
    Wire.write(regAddress);
    Wire.endTransmission();
    Wire.requestFrom(sensorAddress, quanity);
    for (uint8_t i = 0; i < quanity; i++)
      regValue[i] = Wire.read();
  } else {
    for (uint8_t i = 0; i < quanity; i++) {
      Wire.beginTransmission(sensorAddress);
      Wire.write(regAddress + i);
      Wire.endTransmission();
      Wire.requestFrom(sensorAddress, (uint8_t)1);
      regValue[i] = Wire.read();
    }
  }

}

uint8_t DFRobot_LIS2DH12::writeReg(uint8_t regAddress, uint8_t regValue)
{
  Wire.beginTransmission(sensorAddress);
  Wire.write(regAddress);
  Wire.write(regValue);
  return Wire.endTransmission(true);
}

uint8_t DFRobot_LIS2DH12::writeReg(uint8_t regAddress, uint8_t *regValue, size_t quanity, bool autoIncrement)
{
  Wire.beginTransmission(sensorAddress);
  if (autoIncrement) {
    Wire.write(regAddress);
    Wire.write(regValue, quanity);
  }
  else {
    for (size_t i = 0; i < quanity; i++) {
      Wire.write(regAddress + i);
      Wire.write(regValue[i]);
      if ( i < (quanity - 1) ) {
        Wire.endTransmission(false);
        Wire.beginTransmission(sensorAddress);
      }
    }
  }
  return Wire.endTransmission(true);
}

void DFRobot_LIS2DH12::setRange(uint8_t range)
{
  switch (range)
  {
    case LIS2DH12_RANGE_2GA:
      mgScaleVel = 16;
      break;

    case LIS2DH12_RANGE_4GA:
      mgScaleVel = 8;
      break;

    case LIS2DH12_RANGE_8GA:
      mgScaleVel = 4;
      break;

    case LIS2DH12_RANGE_16GA:
      mgScaleVel = 2;
      break;

    default:
      mgScaleVel = 16;
      break;
  }
}


/*!
   @file testLIS2DH12.ino
   @brief DFRobot's Read LIS2DH12 data
   @n This example is in order to achieve the serial port to receive LIS2DH12 back to the data

   @copyright [DFRobot](http://www.dfrobot.com), 2016
   @copyright GNU Lesser General Public License
   @author [Wuxiao](xiao.wu@dfrobot.com)

   @version  V1.0
   @date  2016-10-13

   @version  V1.1
   @date  2016-12-23
   @change add the setting range function

   @https://github.com/DFRobot/DFRobot_LIS2DH12
*/

#include <Wire.h>


DFRobot_LIS2DH12 LIS; //Accelerometer

void accel_setup() {
//  Wire.begin();
//  Serial.begin(115200);

  // Set measurement range
  // Ga: LIS2DH12_RANGE_2GA
  // Ga: LIS2DH12_RANGE_4GA
  // Ga: LIS2DH12_RANGE_8GA
  // Ga: LIS2DH12_RANGE_16GA
  while (LIS.init(LIS2DH12_RANGE_2GA) == -1) { //Equipment connection exception or I2C address error
    Serial.println("No I2C  Accelerometer devices found");
    delay(1000);
  }
}

void accel_loop() {
  Serial.println("Start Accel Loop");
  acceleration();
}

/*!
    @brief Print the position result.
*/
void acceleration(void)
{
  int16_t x, y, z;

//  delay(1000);
  LIS.readXYZ(x, y, z);
  LIS.mgScale(x, y, z);
  Serial.print("Acceleration x: "); //print acceleration
  Serial.print(x);
  Serial.print(" mg \ty: ");
  Serial.print(y);
  Serial.print(" mg \tz: ");
  Serial.print(z);
  Serial.println(" mg");
}



/*!
    @brief Check for interupt.
*/
int interupt_check(void) {

  uint8_t int1_src;

  int1_src = LIS.readReg(0x31);

  if (int1_src & 0x40) {
    Serial.println("Interupt Active!"); 
    return 1; 
  }

  return 0;
  
}

void accel_power(int on) {
  
    uint8_t ctrl_reg1 = 0x20;
    uint8_t ctrl_reg1_values[] = {0x2F};

    if (!on) {
      
        ctrl_reg1_values[0] &= 0x0F;
      
    } 

    LIS.writeReg(ctrl_reg1, ctrl_reg1_values, sizeof(ctrl_reg1_values));
  
}




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

    void write16(uint8_t reg, uint16_t data);

  private:

    uint8_t config;

    uint16_t raw_uva;
    uint16_t raw_uvb;
    uint16_t raw_dark;
    uint16_t raw_vis;
    uint16_t raw_ir;

    uint16_t read16(uint8_t reg);

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

//  Wire.begin();
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

void uv_setup() {
//  delay(1000);
//  Serial.begin(57600);
//  Wire.begin();
  if (!veml6075.begin()) {
    Serial.println(F("VEML6075 not found!"));
  } else {
    Serial.println("uv started!");
    found = true;
  }
}

void uv_loop() {

  Serial.println("Start UV Loop");
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

//    delay(1000);
}

void uv_power(int on) {

  uint16_t config_reg = VEML6075_CONF_IT_100MS;

  if (on) {
    
    config_reg |= VEML6075_CONF_SD_OFF;
    
  } else {

    config_reg |= VEML6075_CONF_SD_ON;
    
  }

  veml6075.write16(VEML6075_REG_CONF, config_reg);

}





void setup() {  
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Here1");
  temphumid_setup();
  Serial.println("whataboutnow");
  accel_setup();
  uv_setup();
  Serial.println("Here2");
  delay(100);
  temphumid_loop();
  uv_loop();
  delay(100);
  uv_power(0);
}


void loop() {
  Serial.println("Start Loop");
  accel_loop();
  if (interupt_check()) {
    Serial.println("Waking Up");
    uv_power(1);
    delay(100);
    temphumid_loop();
    uv_loop();
    delay(100);
    uv_power(0);
    interupt_check();  
  }
  Serial.println("Deep Sleep");
  accel_power(0);
  delay(3000);
  accel_power(1);
  Serial.println("Sleep Mode");
  Serial.println("End of Loop");
  delay(3000);
}



