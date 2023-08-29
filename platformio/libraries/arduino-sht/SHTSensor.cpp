/*
 *  Copyright (c) 2018, Sensirion AG <andreas.brauchli@sensirion.com>
 *  Copyright (c) 2015-2016, Johannes Winkelmann <jw@smts.ch>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of the Sensirion AG nor the names of its
 *        contributors may be used to endorse or promote products derived
 *        from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <inttypes.h>
#include <Wire.h>
#include <Arduino.h>

#include "SHTSensor.h"

//
// class SHTI2cSensor
//

const uint8_t SHTI2cSensor::CMD_SIZE             = 2;
const uint8_t SHTI2cSensor::EXPECTED_DATA_SIZE   = 6;


SHTI2cSensor::SHTI2cSensor(uint8_t i2cAddress, SHTI2cSensor::SHTRepeatibility repeatability, SHTI2cSensor::SHTmps mps)
{
  mI2cAddress=i2cAddress;  
  setRepeatability(repeatability,mps);
}

uint8_t SHTI2cSensor::crc8(const uint8_t *data, uint8_t len)
{
  // adapted from SHT21 sample code from
  // http://www.sensirion.com/en/products/humidity-temperature/download-center/
  
  uint8_t crc = 0xff;
  uint8_t byteCtr;
  for (byteCtr = 0; byteCtr < len; ++byteCtr) {
    crc ^= data[byteCtr];
    for (uint8_t bit = 8; bit > 0; --bit) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0x31;
      } else {
        crc = (crc << 1);
      }
    }
  }
  return crc;
}


bool SHTI2cSensor::setRepeatability(SHTI2cSensor::SHTRepeatibility newRepeatability, SHTI2cSensor::SHTmps newmps)
{

  // Single shot
  switch (newRepeatability) {
  case SHTI2cSensor::SHT_REPEATABILITY_HIGH:
    mI2cSSCommand = SHTI2cSensor::SHT_SINGLESHOT_REPEATABILITY_HIGH_COMMAND;
    mDuration = SHTI2cSensor::SHT_REPEATABILITY_HIGH_DURATION;
    break;
  case SHTI2cSensor::SHT_REPEATABILITY_MEDIUM:
    mI2cSSCommand = SHTI2cSensor::SHT_SINGLESHOT_REPEATABILITY_MEDIUM_COMMAND;
    mDuration = SHTI2cSensor::SHT_REPEATABILITY_MEDIUM_DURATION;
    break;
  case SHTI2cSensor::SHT_REPEATABILITY_LOW:
    mI2cSSCommand = SHTI2cSensor::SHT_SINGLESHOT_REPEATABILITY_LOW_COMMAND;
    mDuration = SHTI2cSensor::SHT_REPEATABILITY_LOW_DURATION;
    break;
  default:
    return false;
  }
  
  //  Periodic Data Acquisition Mode
  switch (newmps) {
  case SHTI2cSensor::SHT_MPS_05:
    mI2cPCommand = 0X2000;
    switch (newRepeatability) {
    case SHTI2cSensor::SHT_REPEATABILITY_HIGH:
      mI2cPCommand |=  0X32;
      break;
    case SHTI2cSensor::SHT_REPEATABILITY_MEDIUM:
      mI2cPCommand |=  0X24;
      break;
    case SHTI2cSensor::SHT_REPEATABILITY_LOW:
      mI2cPCommand |=  0X2F;
      break;
    default:
      return false;
    }
    break;
  case SHTI2cSensor::SHT_MPS_1:
    mI2cPCommand = 0X2100;
    switch (newRepeatability) {
    case SHTI2cSensor::SHT_REPEATABILITY_HIGH:
      mI2cPCommand |=  0X30;
      break;
    case SHTI2cSensor::SHT_REPEATABILITY_MEDIUM:
      mI2cPCommand |=  0X26;
      break;
    case SHTI2cSensor::SHT_REPEATABILITY_LOW:
      mI2cPCommand |=  0X2D;
      break;
    default:
      return false;
    }
    break;
  case SHTI2cSensor::SHT_MPS_2:
    mI2cPCommand = 0X2200;
    switch (newRepeatability) {
    case SHTI2cSensor::SHT_REPEATABILITY_HIGH:
      mI2cPCommand |=  0X36;
      break;
    case SHTI2cSensor::SHT_REPEATABILITY_MEDIUM:
      mI2cPCommand |=  0X20;
      break;
    case SHTI2cSensor::SHT_REPEATABILITY_LOW:
      mI2cPCommand |=  0X2D;
      break;
    default:
      return false;
    }
    break;
  case SHTI2cSensor::SHT_MPS_4:
    mI2cPCommand = 0X2300;
    switch (newRepeatability) {
    case SHTI2cSensor::SHT_REPEATABILITY_HIGH:
      mI2cPCommand |=  0X34;
      break;
    case SHTI2cSensor::SHT_REPEATABILITY_MEDIUM:
      mI2cPCommand |=  0X22;
      break;
    case SHTI2cSensor::SHT_REPEATABILITY_LOW:
      mI2cPCommand |=  0X29;
      break;
    default:
      return false;
    }
    break;
  case SHTI2cSensor::SHT_MPS_10:
    mI2cPCommand = 0X2700;
    switch (newRepeatability) {
    case SHTI2cSensor::SHT_REPEATABILITY_HIGH:
      mI2cPCommand |=  0X37;
      break;
    case SHTI2cSensor::SHT_REPEATABILITY_MEDIUM:
      mI2cPCommand |=  0X21;
      break;
    case SHTI2cSensor::SHT_REPEATABILITY_LOW:
      mI2cPCommand |=  0X2A;
      break;
    default:
      return false;
    }
    break;

  default:
    return false;
  }
    
  return true;

}


bool SHTI2cSensor::sendCommand(const uint8_t *i2cCommand,
                               uint8_t commandLength)
{
  
  Wire.beginTransmission(mI2cAddress);
  for (int i = 0; i < commandLength; ++i) {
    if (Wire.write(i2cCommand[i]) != 1) {
      //Serial.println("error write");
      return false;
    }
  }

  if (Wire.endTransmission(true) != 0) {
    //Serial.println("error end");
    return false;
  }
  return true;
}


bool SHTI2cSensor::checkStatus()
{
  uint8_t cmd[CMD_SIZE];
  
  uint8_t data[EXPECTED_DATA_SIZE];
  const uint8_t C_EXPECTED_DATA_SIZE   = 3;
  
  cmd[0]=0xF3;
  cmd[1]=0x2D;
  if (  !sendCommand(cmd,2)) {
    //Serial.println("error send verification");
    return false;
  }
  
  Wire.requestFrom(mI2cAddress, C_EXPECTED_DATA_SIZE);
  
  // check if the same number of bytes are received that are requested.
  if (Wire.available() != C_EXPECTED_DATA_SIZE) {
    //Serial.println("error request");
    return false;
  }
  
  for (int i = 0; i < C_EXPECTED_DATA_SIZE; ++i) {
    data[i] = Wire.read();
  }

  // check CRC
  if (crc8(&data[0], 2) != data[2]) {
    //Serial.println("error crc");
    return false;
  }

  uint16_t val;
  val = ((data[0] << 8) + data[1]) & 0xEC13;
  //Serial.print(" byte  error status: ");
  //Serial.println(val,BIN);

  if (val != 0){
    return false;
  }
  
  //Serial.println("OK");
  
  return true;
  
}


bool SHTI2cSensor::fetchData()
{
  uint8_t cmd[CMD_SIZE];
  
  cmd[0]=0xE0;
  cmd[1]=0x00;
  return sendCommand(cmd,CMD_SIZE);
}



bool SHTI2cSensor::softReset()
{
  uint8_t cmd[CMD_SIZE];
  
  cmd[0]=0x30;
  cmd[1]=0xA2;
  return sendCommand(cmd,CMD_SIZE);

}

bool SHTI2cSensor::heaterEnable()
{
  uint8_t cmd[CMD_SIZE];
  
  cmd[0]=0x30;
  cmd[1]=0x6D;
  return sendCommand(cmd,CMD_SIZE);

}

bool SHTI2cSensor::heaterDisable()
{
  uint8_t cmd[CMD_SIZE];
  
  cmd[0]=0x30;
  cmd[1]=0x66;
  return sendCommand(cmd,CMD_SIZE);

}



bool SHTI2cSensor::clearStatusRegister()
{
  uint8_t cmd[CMD_SIZE];
  
  cmd[0]=0x30;
  cmd[1]=0x41;
  return sendCommand(cmd,CMD_SIZE);
}


bool SHTI2cSensor::ART()
{
  uint8_t cmd[CMD_SIZE];
  
  cmd[0]=0x2B;
  cmd[1]=0x32;
  return sendCommand(cmd,CMD_SIZE);
}

bool SHTI2cSensor::stopPeriodicDataAcquisition()
{
  uint8_t cmd[CMD_SIZE];
  
  cmd[0]=0x30;
  cmd[1]=0x93;
  return sendCommand(cmd,CMD_SIZE);
}

bool SHTI2cSensor::getValues()
{
  uint8_t data[EXPECTED_DATA_SIZE];
  
  Wire.requestFrom(mI2cAddress, EXPECTED_DATA_SIZE);
  
  // check if the same number of bytes are received that are requested.
  if (Wire.available() != EXPECTED_DATA_SIZE) {
    //Serial.println("error request");
    return false;
  }
  
  for (int i = 0; i < EXPECTED_DATA_SIZE; ++i) {
    data[i] = Wire.read();
  }
  //Serial.println("OK");
  
  // -- Important: assuming each 2 byte of data is followed by 1 byte of CRC
  
  // check CRC for both RH and T
  if (crc8(&data[0], 2) != data[2] || crc8(&data[3], 2) != data[5]) {
    //Serial.println("error crc");
    return false;
  }
  
  // convert to Temperature/Humidity
  uint16_t val;
  val = (data[0] << 8) + data[1];
  mTemperature = -45. + 175. * (val / 65535.);
  
  val = (data[3] << 8) + data[4];
  mHumidity = 100. * (val / 65535.);
  
  return true;
}


bool SHTI2cSensor::periodicDataAcquisition(){

  uint8_t cmd[CMD_SIZE];
  
  cmd[0] = mI2cPCommand >> 8;
  cmd[1] = mI2cPCommand & 0xff;
  return sendCommand(cmd,CMD_SIZE);
}


bool SHTI2cSensor::singleShotDataAcquisition(){
  uint8_t cmd[CMD_SIZE];
  
  cmd[0] = mI2cSSCommand >> 8;
  cmd[1] = mI2cSSCommand & 0xff;
  return sendCommand(cmd,CMD_SIZE);
}


// utility blocking function
// query a Single Shot Data Acquisition, wait for response and acquire measurements

bool SHTI2cSensor::readSample()
{
  //Serial.println("sendCommand");
  if (singleShotDataAcquisition()){
    //Serial.println(mDuration);
    delay(mDuration);
    //Serial.println("getValues");  
    if (getValues()){
      //Serial.println("checkStatus");
      if (checkStatus()){
	return true;
      }
    }
  }

  return false;
}


float SHTI2cSensor::getHumidity() {
  return mHumidity;
}

float SHTI2cSensor::getTemperature() {
  return mTemperature;
}

