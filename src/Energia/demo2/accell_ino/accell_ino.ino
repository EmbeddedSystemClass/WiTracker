/*!
   @file DFRobot_LIS2DH12.h
   @brief DFRobot's Read LIS2DH12 data
   @n This example achieve receiving LIS2DH12  data via serial

   @copyright	[DFRobot](http://www.dfrobot.com), 2016
   @copyright	GNU Lesser General Public License
   @author [Wuxiao](xiao.wu@dfrobot.com)
   @version  V1.0
   @date  2016-10-13
   @https://github.com/DFRobot/DFRobot_LIS2DH12
*/


#ifndef DFRobot_LIS2DH12_h
#define DFRobot_LIS2DH12_h

#include <stdlib.h>
#include <stdint.h>

#define LIS2DH12_RANGE_2GA	0x00
#define LIS2DH12_RANGE_4GA	0x10
#define LIS2DH12_RANGE_8GA	0x20
#define LIS2DH12_RANGE_16GA	0x30

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

   @copyright	[DFRobot](http://www.dfrobot.com), 2016
   @copyright	GNU Lesser General Public License
   @author [Wuxiao](xiao.wu@dfrobot.com)
   @version  V1.0
   @date  2016-10-13
   @https://github.com/DFRobot/DFRobot_LIS2DH12
*/

#include <Wire.h>

uint8_t DFRobot_LIS2DH12::sensorAddress = 0x19; //	0x18

int8_t DFRobot_LIS2DH12::init(uint8_t range)
{
  int8_t ret = 0;

  setRange(range);
  Wire.beginTransmission(sensorAddress);
  ret = Wire.endTransmission();
  if (ret != 0) {
    ret = -1;
  } else {
    uint8_t ctrl_reg_values[] = {0x2F, 0x00, 0x40, range, 0x08, 0x00};
    ret = (int8_t)writeReg(0xA0, ctrl_reg_values, sizeof(ctrl_reg_values));

    // set interrupt
    uint8_t int1_cfg_reg = 0x30;
    uint8_t int1_cfg_reg_values[] = {0x02};
    writeReg(int1_cfg_reg, int1_cfg_reg_values, sizeof(int1_cfg_reg_values));

    uint8_t int1_ths_reg = 0x32;
    uint8_t int1_ths_reg_values[] = {0x02};
    writeReg(int1_ths_reg, int1_ths_reg_values, sizeof(int1_ths_reg_values));

    uint8_t int1_dur_reg = 0x33;
    uint8_t int1_dur_reg_values[] = {0x05};
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

   @copyright	[DFRobot](http://www.dfrobot.com), 2016
   @copyright	GNU Lesser General Public License
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

void setup() {
  Wire.begin();
  Serial.begin(115200);
  delay(100);

  // Set measurement range
  // Ga: LIS2DH12_RANGE_2GA
  // Ga: LIS2DH12_RANGE_4GA
  // Ga: LIS2DH12_RANGE_8GA
  // Ga: LIS2DH12_RANGE_16GA
  while (LIS.init(LIS2DH12_RANGE_16GA) == -1) { //Equipment connection exception or I2C address error
    Serial.println("No I2C devices found");
    delay(1000);
  }
}

void loop() {
  acceleration();
}

/*!
    @brief Print the position result.
*/
void acceleration(void)
{
  int16_t x, y, z;

  delay(1000);
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
