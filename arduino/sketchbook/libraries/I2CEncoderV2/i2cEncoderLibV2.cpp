//
//    FILE: i2cEncoderLibV2.h
// VERSION: 0.1..
// PURPOSE: Library for I2C Encoder V2 board with Arduino
// LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)
//
// DATASHEET:
//
//     URL:
//
// AUTHOR:
// Simone Caron
//

#include "i2cEncoderLibV2.h"
#include <Wire.h>

/*********************************** Public functions *************************************/
/** Class costructor **/
i2cEncoderLibV2::i2cEncoderLibV2(uint8_t add) {
  _add = add;
}

/** Used for initialize the encoder **/
uint8_t i2cEncoderLibV2::begin(uint8_t conf) {

  //  Wire.begin();
  _gconf = conf;
  return writeEncoder(REG_GCONF, conf);
}

/*********************************** Read functions *************************************/

/** Return the GP1 Configuration**/
uint8_t i2cEncoderLibV2::readGP1conf(void) {
  return (readEncoderByte(REG_GP1CONF));
}

/** Return the GP1 Configuration**/
uint8_t i2cEncoderLibV2::readGP2conf(void) {
  return (readEncoderByte(REG_GP2CONF));
}

/** Return the GP1 Configuration**/
uint8_t i2cEncoderLibV2::readGP3conf(void) {
  return (readEncoderByte(REG_GP3CONF));
}

/** Return the INT pin configuration**/
uint8_t i2cEncoderLibV2::readInterruptConfig(void) {
  return (readEncoderByte(REG_INTCONF));
}


/** Return true if the status of the econder changed, otherwise return false **/
bool i2cEncoderLibV2::updateStatus(void) {
  _stat = readEncoderByte(REG_ESTATUS);

  if (_stat == 0) {
    _stat2 = 0;
    return false;
  }
  if ((_stat & INT2) != 0) {
    _stat2 = readEncoderByte(REG_I2STATUS);
  } else {
    _stat2 = 0;
  }
  return true;
}

/** Check if a particular status match, return true is match otherwise false. Before require updateStatus() **/
bool i2cEncoderLibV2::readStatus(uint8_t s) {
  if ((_stat & s) != 0) {
    return true;
  }
  return false;
}

/** Return the status of the encoder **/
uint8_t i2cEncoderLibV2::readStatus(void) {
  return _stat;
}


/** Check if a particular status of the Int2 match, return true is match otherwise false. Before require updateStatus() **/
bool i2cEncoderLibV2::readInt2(uint8_t s) {
  if ((_stat2 & s)  != 0) {
    return true;
  }
  return false;
}

/** Return the Int2 status of the encoder. Before require updateStatus()  **/
uint8_t i2cEncoderLibV2::readInt2(void) {
  return _stat2;
}

/** Return Fade process status  **/
uint8_t i2cEncoderLibV2::readFadeStatus(void) {
  return readEncoderByte(REG_FSTATUS);
}

/** Check if a particular status of the Fade process match, return true is match otherwise false. **/
bool i2cEncoderLibV2::readFadeStatus(uint8_t s) {
  if ((readEncoderByte(REG_FSTATUS) & s) == 1)
    return true;

  return false;
}

/** Return the PWM LED R value  **/
uint8_t i2cEncoderLibV2::readLEDR(void) {
  return ((uint8_t) readEncoderByte(REG_RLED));
}

/** Return the PWM LED G value  **/
uint8_t i2cEncoderLibV2::readLEDG(void) {
  return ((uint8_t) readEncoderByte(REG_GLED));
}

/** Return the PWM LED B value  **/
uint8_t i2cEncoderLibV2::readLEDB(void) {
  return ((uint8_t) readEncoderByte(REG_BLED));
}

/** Return the 32 bit value of the encoder counter  **/
float i2cEncoderLibV2::readCounterFloat(void) {
  return (readEncoderFloat(REG_CVALB4));
}

/** Return the 32 bit value of the encoder counter  **/
int32_t i2cEncoderLibV2::readCounterLong(void) {
  return ((int32_t) readEncoderLong(REG_CVALB4));
}

/** Return the 16 bit value of the encoder counter  **/
int16_t i2cEncoderLibV2::readCounterInt(void) {
  return ((int16_t) readEncoderInt(REG_CVALB2));
}

/** Return the 8 bit value of the encoder counter  **/
int8_t i2cEncoderLibV2::readCounterByte(void) {
  return ((int8_t) readEncoderByte(REG_CVALB1));
}

/** Return the Maximum threshold of the counter **/
int32_t i2cEncoderLibV2::readMax(void) {
  return ((int32_t) readEncoderLong(REG_CMAXB4));
}

/** Return the Minimum threshold of the counter **/
int32_t i2cEncoderLibV2::readMin(void) {
  return ((int32_t) readEncoderLong(REG_CMINB4));
}

/** Return the Maximum threshold of the counter **/
float i2cEncoderLibV2::readMaxFloat(void) {
  return (readEncoderFloat(REG_CMAXB4));
}

/** Return the Minimum threshold of the counter **/
float i2cEncoderLibV2::readMinFloat(void) {
  return (readEncoderFloat(REG_CMINB4));

}

/** Return the Steps increment **/
int32_t i2cEncoderLibV2::readStep(void) {
  return (readEncoderInt(REG_ISTEPB4));
}

/** Return the Steps increment, in float variable **/
float i2cEncoderLibV2::readStepFloat(void) {
  return (readEncoderFloat(REG_ISTEPB4));

}

/** Read GP1 register value **/
uint8_t i2cEncoderLibV2::readGP1(void) {
  return (readEncoderByte(REG_GP1REG));
}

/** Read GP2 register value **/
uint8_t i2cEncoderLibV2::readGP2(void) {
  return (readEncoderByte(REG_GP2REG));
}

/** Read GP3 register value **/
uint8_t i2cEncoderLibV2::readGP3(void) {
  return (readEncoderByte(REG_GP3REG));
}

/** Read Anti-bouncing period register **/
uint8_t i2cEncoderLibV2::readAntibouncingPeriod(void) {
  return (readEncoderByte(REG_ANTBOUNC));
}

/** Read Double push period register **/
uint8_t i2cEncoderLibV2::readDoublePushPeriod(void) {
  return (readEncoderByte(REG_DPPERIOD));
}

/** Read the fade period of the RGB LED**/
uint8_t i2cEncoderLibV2::readFadeRGB(void) {
  return (readEncoderByte(REG_FADERGB));
}

/** Read the fade period of the GP LED**/
uint8_t i2cEncoderLibV2::readFadeGP(void) {
  return (readEncoderByte(REG_FADEGP));
}

/** Read the EEPROM memory**/
uint8_t i2cEncoderLibV2::readEEPROM(uint8_t add) {
  if (add <= 0x7f) {
    if ((_gconf & EEPROM_BANK1) != 0) {
      _gconf = _gconf & 0xBF;
      writeEncoder(REG_GCONF, _gconf);
    }
    return (readEncoderByte((REG_EEPROMS + add)));
  } else {
    if ((_gconf & EEPROM_BANK1) == 0) {
      _gconf = _gconf | 0x40;
      writeEncoder(REG_GCONF, _gconf);
    }
    readEncoderByte(add);
  }
  return EC_SUCCESS;
}

/*********************************** Write functions *************************************/
/** Write the GP1 configuration**/
uint8_t i2cEncoderLibV2::writeGP1conf(uint8_t gp1) {
  return writeEncoder(REG_GP1CONF, gp1);
}

/** Write the GP2 configuration**/
uint8_t i2cEncoderLibV2::writeGP2conf(uint8_t gp2) {
  return writeEncoder(REG_GP2CONF, gp2);
}

/** Write the GP3 configuration**/
uint8_t i2cEncoderLibV2::writeGP3conf(uint8_t gp3) {
  return writeEncoder(REG_GP3CONF, gp3);
}

/** Write the interrupt configuration **/
uint8_t i2cEncoderLibV2::writeInterruptConfig(uint8_t interrupt) {
  return writeEncoder(REG_INTCONF, interrupt);
}

/** Write the counter value **/
uint8_t i2cEncoderLibV2::writeCounter(int32_t value) {
  return writeEncoder(REG_CVALB4, value);
}

/** Write the counter value **/
uint8_t i2cEncoderLibV2::writeCounter(float value) {
  return writeEncoder(REG_CVALB4, value);
}

/** Write the maximum threshold value **/
uint8_t i2cEncoderLibV2::writeMax(int32_t max) {
  return writeEncoder(REG_CMAXB4, max);
}

/** Write the maximum threshold value **/
uint8_t i2cEncoderLibV2::writeMax(float max) {
  return writeEncoder(REG_CMAXB4, max);
}

/** Write the minimum threshold value **/
uint8_t i2cEncoderLibV2::writeMin(int32_t min) {
  return writeEncoder(REG_CMINB4, min);
}

/** Write the minimum threshold value **/
uint8_t i2cEncoderLibV2::writeMin(float min) {
  return writeEncoder(REG_CMINB4, min);
}

/** Write the Step increment value **/
uint8_t i2cEncoderLibV2::writeStep(int32_t step) {
  return writeEncoder(REG_ISTEPB4, step);
}

/** Write the Step increment value **/
uint8_t i2cEncoderLibV2::writeStep(float step) {
  return writeEncoder(REG_ISTEPB4, step);
}

/** Write the PWM value of the RGB LED red **/
uint8_t i2cEncoderLibV2::writeLEDR(uint8_t rled) {
  return writeEncoder(REG_RLED, rled);
}

/** Write the PWM value of the RGB LED green **/
uint8_t i2cEncoderLibV2::writeLEDG(uint8_t gled) {
  return writeEncoder(REG_GLED, gled);
}

/** Write the PWM value of the RGB LED blue **/
uint8_t i2cEncoderLibV2::writeLEDB(uint8_t bled) {
  return writeEncoder(REG_BLED, bled);
}

/** Write 24bit color code **/
uint8_t i2cEncoderLibV2::writeRGBCode(uint32_t rgb) {
  return writeEncoder24bit(REG_RLED, rgb);
}

/** Write GP1 register, used when GP1 is set to output or PWM **/
uint8_t i2cEncoderLibV2::writeGP1(uint8_t gp1) {
  return writeEncoder(REG_GP1REG, gp1);
}

/** Write GP2 register, used when GP2 is set to output or PWM **/
uint8_t i2cEncoderLibV2::writeGP2(uint8_t gp2) {
  return writeEncoder(REG_GP2REG, gp2);
}

/** Write GP3 register, used when GP3 is set to output or PWM **/
uint8_t i2cEncoderLibV2::writeGP3(uint8_t gp3) {
  return writeEncoder(REG_GP3REG, gp3);
}

/** Write Anti-bouncing period register **/
uint8_t i2cEncoderLibV2::writeAntibouncingPeriod(uint8_t bounc) {
  return writeEncoder(REG_ANTBOUNC, bounc);
}

/** Write Anti-bouncing period register **/
uint8_t i2cEncoderLibV2::writeDoublePushPeriod(uint8_t dperiod) {
  return writeEncoder(REG_DPPERIOD, dperiod);
}

/** Write Fade timing in ms **/
uint8_t i2cEncoderLibV2::writeFadeRGB(uint8_t fade) {
  return writeEncoder(REG_FADERGB, fade);
}

/** Write Fade timing in ms **/
uint8_t i2cEncoderLibV2::writeFadeGP(uint8_t fade) {
  return writeEncoder(REG_FADEGP, fade);
}

/** Write the EEPROM memory**/
uint8_t i2cEncoderLibV2::writeEEPROM(uint8_t add, uint8_t data) {
  if (add <= 0x7f) {
    if ((_gconf & EEPROM_BANK1) != 0) {
      _gconf = _gconf & 0xBF;
      if (writeEncoder(REG_GCONF, _gconf)  != 0) return EC_INTERNAL_ERROR;
    }
    writeEncoder((REG_EEPROMS + add), data);
  } else {
    if ((_gconf & EEPROM_BANK1) == 0) {
      _gconf = _gconf | 0x40;
      if (writeEncoder(REG_GCONF, _gconf) != 0) return EC_INTERNAL_ERROR;
    }
    if (writeEncoder(add, data) != 0) return EC_INTERNAL_ERROR;
  }

  return EC_SUCCESS;

}

/*********************************** Private functions *************************************/
/***************************** Read function to the encoder ********************************/

/** Read 1 byte from the encoder **/
uint8_t i2cEncoderLibV2::readEncoderByte(uint8_t reg) {
  byte rdata = 0xEF;

  Wire.beginTransmission(_add);
  Wire.write(reg);
  if (Wire.endTransmission() != 0) return rdata;
  Wire.requestFrom(_add, 1);
  if (Wire.available()==1) {
    rdata = Wire.read();
  }
  //	delay(5);
  return rdata;
}

/** Read 2 bytes from the encoder **/
int16_t i2cEncoderLibV2::readEncoderInt(uint8_t reg) {

  union Data_v _tem_data;  
  _tem_data.val=0xEFFF;
  Wire.beginTransmission(_add);
  Wire.write(reg);
  if (Wire.endTransmission() != 0) return _tem_data.val;
  Wire.requestFrom(_add, 2);
  if (Wire.available()==2) {
    _tem_data.bval[1] = Wire.read();
    _tem_data.bval[0] = Wire.read();
  }
  return _tem_data.val;
}

/** Read 4 bytes from the encoder **/
int32_t i2cEncoderLibV2::readEncoderLong(uint8_t reg) {

  union Data_v _tem_data;
  _tem_data.lval=0xEFFFFFFF;
  Wire.beginTransmission(_add);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(_add, 4);
  if (Wire.available()==4) {
    _tem_data.bval[3] = Wire.read();
    _tem_data.bval[2] = Wire.read();
    _tem_data.bval[1] = Wire.read();
    _tem_data.bval[0] = Wire.read();
  }
  return _tem_data.lval;
}

/** Read 4 bytes from the encoder **/
float i2cEncoderLibV2::readEncoderFloat(uint8_t reg) {

  union Data_v _tem_data;
  _tem_data.fval=FLT_MAX;  
  Wire.beginTransmission(_add);
  Wire.write(reg);
  if (Wire.endTransmission() != 0) return _tem_data.fval;
  Wire.requestFrom(_add, 4);
  if (Wire.available()==4) {
    _tem_data.bval[3] = Wire.read();
    _tem_data.bval[2] = Wire.read();
    _tem_data.bval[1] = Wire.read();
    _tem_data.bval[0] = Wire.read();
  }
  return _tem_data.fval;
}

/***************************** Write function to the encoder ********************************/
/** Send to the encoder 1 byte **/
uint8_t i2cEncoderLibV2::writeEncoder(uint8_t reg, uint8_t data) {

  Wire.beginTransmission(_add);
  Wire.write(reg);
  Wire.write(data);
  if (Wire.endTransmission() != 0) return EC_INTERNAL_ERROR;
  //  delay(1);
  return EC_SUCCESS;

}




/** Send to the encoder 4 byte **/
uint8_t i2cEncoderLibV2::writeEncoder(uint8_t reg, int32_t data) {
  uint8_t temp[4];
  union Data_v _tem_data;
  _tem_data.lval = data;
  temp[0] = _tem_data.bval[3];
  temp[1] = _tem_data.bval[2];
  temp[2] = _tem_data.bval[1];
  temp[3] = _tem_data.bval[0];
  Wire.beginTransmission(_add);
  Wire.write(reg);
  Wire.write(temp, 4);
  if (Wire.endTransmission() != 0) return EC_INTERNAL_ERROR;
  // delay(1);
  return EC_SUCCESS;

}

/** Send to the encoder 4 byte for floating number **/
uint8_t i2cEncoderLibV2::writeEncoder(uint8_t reg, float data) {

  uint8_t temp[4];
  union Data_v _tem_data;
  _tem_data.fval = data;
  temp[0] = _tem_data.bval[3];
  temp[1] = _tem_data.bval[2];
  temp[2] = _tem_data.bval[1];
  temp[3] = _tem_data.bval[0];
  Wire.beginTransmission(_add);
  Wire.write(reg);
  Wire.write(temp, 4);
  if (Wire.endTransmission() != 0) return EC_INTERNAL_ERROR;
  //  delay(1);
  return EC_SUCCESS;

}


/** Send to the encoder 3 byte **/
uint8_t i2cEncoderLibV2::writeEncoder24bit(uint8_t reg, uint32_t data) {
  uint8_t temp[3];
  union Data_v _tem_data;
  _tem_data.lval = data;
  temp[0] = _tem_data.bval[2];
  temp[1] = _tem_data.bval[1];
  temp[2] = _tem_data.bval[0];
  Wire.beginTransmission(_add);
  Wire.write(reg);
  Wire.write(temp, 3);
  if (Wire.endTransmission() != 0) return EC_INTERNAL_ERROR;
  // delay(1);

  return EC_SUCCESS;
  
}
