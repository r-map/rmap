/*
    bq769x0.cpp - Battery management system based on bq769x0 for Arduino
    Copyright (C) 2015  Martin Jäger (m.jaeger@posteo.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as 
    published by the Free Software Foundation, either version 3 of the 
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this program. If not, see 
    <http://www.gnu.org/licenses/>.
*/

/*
  TODO:
  - Balancing algorithm
  - SOC calculation + coulomb counting
  - Autobalancing (what is this?)
*/

#include <Arduino.h>
#include <Wire.h>     // I2C/TWI (for Battery Management IC)
#include <math.h>     // log for thermistor calculation

#include "bq769x0.h"
#include "registers.h"

// for the ISR to know the bq769x0 instance
bq769x0* bq769x0::instancePointer = 0;


#if BQ769X0_DEBUG
  const char *byte2char(int x)
  {
    static char b[9];
    b[0] = '\0';
    int z;
    for (z = 128; z > 0; z >>= 1) strcat(b, ((x & z) == z) ? "1" : "0");
    return b;
  }
#endif

// CRC calculation taken from LibreSolar mbed firmware
uint8_t _crc8_ccitt_update (uint8_t inCrc, uint8_t inData)
{
  uint8_t i;
  uint8_t data;
  data = inCrc ^ inData;

  for ( i = 0; i < 8; i++ )
  {
    if (( data & 0x80 ) != 0 )
    {
      data <<= 1;
      data ^= 0x07;
    }
    else data <<= 1;
  }

  return data;
}

//----------------------------------------------------------------------------

bq769x0::bq769x0(byte bqType, int bqI2CAddress)
{
  type = bqType;
  I2CAddress = bqI2CAddress;
  
  if (type == bq76920) {
    numberOfCells = 5;
  }
  else if (type == bq76930) {
    numberOfCells = 10;  
  }
  else {
    numberOfCells = 15;
  }
  
  // prevent errors if someone reduced MAX_NUMBER_OF_CELLS accidentally
  if (numberOfCells > MAX_NUMBER_OF_CELLS) {
    numberOfCells = MAX_NUMBER_OF_CELLS;
  }
}


//-----------------------------------------------------------------------------

int bq769x0::begin(byte alertPin, byte bootPin)
{
  //Wire.begin();        // join I2C bus

  // initialize variables
  for (byte i = 0; i < numberOfCells; i++) {
    cellVoltages[i] = 0;
  }
  
  // Boot IC if pin is defined (else: manual boot via push button has to be done before calling this method)
  if (bootPin >= 0)
  {
    pinMode(bootPin, OUTPUT);
    digitalWrite(bootPin, HIGH);
    delay(5);   // wait 5 ms for device to receive boot signal (datasheet: max. 2 ms)
    pinMode(bootPin, INPUT);     // don't disturb temperature measurement
    delay(10);  // wait for device to boot up completely (datasheet: max. 10 ms)
  }
 
  if (determineAddressAndCrc())
  {
    LOG_PRINTLN("Address and CRC detection successful");
    LOG_PRINT("Address: ");
    LOG_PRINTLN(I2CAddress);
    LOG_PRINT("CRC Enabled: ");
    LOG_PRINTLN(crcEnabled);

    // initial settings for bq769x0
    writeRegister(SYS_CTRL1, B00011000);  // switch external thermistor (TEMP_SEL) and ADC on (ADC_EN)
    writeRegister(SYS_CTRL2, B01000000);  // switch CC_EN on

    // attach ALERT interrupt to this instance
    instancePointer = this;
    attachInterrupt(digitalPinToInterrupt(alertPin), bq769x0::alertISR, RISING);

    // get ADC offset and gain
    adcOffset = (signed int) readRegister(ADCOFFSET);  // convert from 2's complement
    adcGain = 365 + (((readRegister(ADCGAIN1) & B00001100) << 1) | ((readRegister(ADCGAIN2) & B11100000) >> 5)); // uV/LSB
    
    return 0;
  }

  else
  {
    LOG_PRINTLN("BMS communication error");
    return 1;
  }
}

//----------------------------------------------------------------------------
// automatically find out address and CRC setting

bool bq769x0::determineAddressAndCrc(void)
{
  LOG_PRINTLN("Determining i2c address and whether CRC is enabled");

  // check for each address and CRC combination while also set CC_CFG to 0x19 as per datasheet
  I2CAddress = 0x08;
  crcEnabled = false;
  writeRegister(CC_CFG, 0x19);
  if (readRegister(CC_CFG) == 0x19) return true;

  I2CAddress = 0x18;
  crcEnabled = false;
  writeRegister(CC_CFG, 0x19);
  if (readRegister(CC_CFG) == 0x19) return true;

  I2CAddress = 0x08;
  crcEnabled = true;
  writeRegister(CC_CFG, 0x19);
  if (readRegister(CC_CFG) == 0x19) return true;

  I2CAddress = 0x18;
  crcEnabled = true;
  writeRegister(CC_CFG, 0x19);
  if (readRegister(CC_CFG) == 0x19) return true;

  return false;
}

//----------------------------------------------------------------------------
// Fast function to check whether BMS has an error
// (returns 0 if everything is OK)

int bq769x0::checkStatus()
{
   LOG_PRINT("checkStatus: ");
   LOG_PRINTLN(errorStatus);
  if (alertInterruptFlag == false && errorStatus == 0) {
    return 0;
  }
  else {
    regSYS_STAT_t sys_stat;
    sys_stat.regByte = readRegister(SYS_STAT);

    if (sys_stat.bits.CC_READY == 1) {
      //LOG_PRINTLN("Interrupt: CC ready");
      updateCurrent(true);  // automatically clears CC ready flag	
    }
    
    // Serious error occured
    if (sys_stat.regByte & B00111111)
    {
      if (alertInterruptFlag == true) {
        secSinceErrorCounter = 0;
      }
      errorStatus = sys_stat.regByte;
      
      int secSinceInterrupt = (millis() - interruptTimestamp) / 1000;
      
      // check for overrun of millis() or very slow running program
      if (abs(static_cast<long>(secSinceInterrupt - secSinceErrorCounter)) > 2) {
        secSinceErrorCounter = secSinceInterrupt;
      }
      
      // called only once per second
      if (secSinceInterrupt >= secSinceErrorCounter)
      {
        if (sys_stat.regByte & B00100000) { // XR error
          // datasheet recommendation: try to clear after waiting a few seconds
          if (secSinceErrorCounter % 3 == 0) {
            LOG_PRINTLN(F("Clearing XR error"));
            writeRegister(SYS_STAT, B00100000);
          }
        }
        if (sys_stat.regByte & B00010000) { // Alert error
          if (secSinceErrorCounter % 10 == 0) {
            LOG_PRINTLN(F("Clearing Alert error"));
            writeRegister(SYS_STAT, B00010000);
          }
        }
        if (sys_stat.regByte & B00001000) { // UV error
          updateVoltages();
          if (cellVoltages[idCellMinVoltage] > minCellVoltage) {
            LOG_PRINTLN(F("Clearing UV error"));
            writeRegister(SYS_STAT, B00001000);
          }
        }
        if (sys_stat.regByte & B00000100) { // OV error
          updateVoltages();
          if (cellVoltages[idCellMaxVoltage] < maxCellVoltage) {
            LOG_PRINTLN(F("Clearing OV error"));
            writeRegister(SYS_STAT, B00000100);
          }
        }
        if (sys_stat.regByte & B00000010) { // SCD
          if (secSinceErrorCounter % 60 == 0) {
            LOG_PRINTLN(F("Clearing SCD error"));
            writeRegister(SYS_STAT, B00000010);
          }
        }
        if (sys_stat.regByte & B00000001) { // OCD
          if (secSinceErrorCounter % 60 == 0) {
            LOG_PRINTLN(F("Clearing OCD error"));
            writeRegister(SYS_STAT, B00000001);
          }
        }
        
        secSinceErrorCounter++;
      }
    }
    else {
      errorStatus = 0;
    }
    
    return errorStatus;

  }

}

//----------------------------------------------------------------------------
// should be called at least once every 250 ms to get correct coulomb counting

void bq769x0::update()
{
  LOG_PRINTLN("update");
  updateCurrent();  // will only read new current value if alert was triggered
  updateVoltages();
  updateTemperatures();
  updateBalancingSwitches();
}

//----------------------------------------------------------------------------
// puts BMS IC into SHIP mode (i.e. switched off)

void bq769x0::shutdown()
{
  writeRegister(SYS_CTRL1, 0x0);
  writeRegister(SYS_CTRL1, 0x1);
  writeRegister(SYS_CTRL1, 0x2);
}

//----------------------------------------------------------------------------

bool bq769x0::enableCharging()
{
  LOG_PRINTLN("enableCharging");
  if (checkStatus() == 0 &&
    cellVoltages[idCellMaxVoltage] < maxCellVoltage &&
    temperatures[0] < maxCellTempCharge &&
    temperatures[0] > minCellTempCharge)
  {
    byte sys_ctrl2;
    sys_ctrl2 = readRegister(SYS_CTRL2);
    writeRegister(SYS_CTRL2, sys_ctrl2 | B00000001);  // switch CHG on
    LOG_PRINTLN("enableCharging: enabled");
    return true;
  }
  else {
    LOG_PRINTLN("enableCharging: failed");
    return false;
  }
}

//----------------------------------------------------------------------------

bool bq769x0::enableDischarging()
{
  LOG_PRINTLN("enableDischarging");
  if (checkStatus() == 0 &&
    cellVoltages[idCellMinVoltage] > minCellVoltage &&
    temperatures[0] < maxCellTempDischarge &&
    temperatures[0] > minCellTempDischarge)
  {
    byte sys_ctrl2;
    sys_ctrl2 = readRegister(SYS_CTRL2);
    writeRegister(SYS_CTRL2, sys_ctrl2 | B00000010);  // switch DSG on
    LOG_PRINTLN("enableDischarging: enabled");
    return true;
  }
  else {
    LOG_PRINTLN("enableDischarging: failed");
    return false;
  }
}

//----------------------------------------------------------------------------

void bq769x0::enableAutoBalancing(void)
{
  autoBalancingEnabled = true;
}


//----------------------------------------------------------------------------

void bq769x0::setBalancingThresholds(int idleTime_min, int absVoltage_mV, byte voltageDifference_mV)
{
  balancingMinIdleTime_s = idleTime_min * 60;
  balancingMinCellVoltage_mV = absVoltage_mV;
  balancingMaxVoltageDifference_mV = voltageDifference_mV;
}

//----------------------------------------------------------------------------
// sets balancing registers if balancing is allowed 
// (sufficient idle time + voltage)

void bq769x0::updateBalancingSwitches(void)
{
  LOG_PRINTLN("updateBalancingSwitches");
  long idleSeconds = (millis() - idleTimestamp) / 1000;
  byte numberOfSections = numberOfCells/5;
  
  // check for millis() overflow
  if (idleSeconds < 0) {
    idleTimestamp = 0;
    idleSeconds = millis() / 1000;
  }
    
  // check if balancing allowed
  if (checkStatus() == 0 &&
    idleSeconds >= balancingMinIdleTime_s && 
    cellVoltages[idCellMaxVoltage] > balancingMinCellVoltage_mV &&
    (cellVoltages[idCellMaxVoltage] - cellVoltages[idCellMinVoltage]) > balancingMaxVoltageDifference_mV)
  {
    balancingActive = true;
    //LOG_PRINTLN("Balancing enabled!");
    
    //regCELLBAL_t cellbal;
    byte balancingFlags;
    byte balancingFlagsTarget;
    
    for (int section = 0; section < numberOfSections; section++)
    {
      balancingFlags = 0;
      for (int i = 0; i < 5; i++)
      {
        if ((cellVoltages[section*5 + i] - cellVoltages[idCellMinVoltage]) > balancingMaxVoltageDifference_mV) {
          
          // try to enable balancing of current cell
          balancingFlagsTarget = balancingFlags | (1 << i);

          // check if attempting to balance adjacent cells
          bool adjacentCellCollision = 
            ((balancingFlagsTarget << 1) & balancingFlags) ||
            ((balancingFlags << 1) & balancingFlagsTarget);
            
          if (adjacentCellCollision == false) {
            balancingFlags = balancingFlagsTarget;
          }          
        }
      }
      LOG_PRINT("Setting CELLBAL");
      LOG_PRINT(section+1);
      LOG_PRINT(" register to: ");
      LOG_PRINTLN(byte2char(balancingFlags));
      
      // set balancing register for this section
      writeRegister(CELLBAL1+section, balancingFlags);
    }
  }
  else if (balancingActive == true)
  {  
    // clear all CELLBAL registers
    for (int section = 0; section < numberOfSections; section++)
    {
      LOG_PRINT("Clearing Register CELLBAL");
      LOG_PRINTLN(section+1);
      writeRegister(CELLBAL1+section, 0x0);
    }
    
    balancingActive = false;
  }
}

void bq769x0::setShuntResistorValue(int res_mOhm)
{
  shuntResistorValue_mOhm = res_mOhm;
}

void bq769x0::setThermistorBetaValue(int beta_K)
{
  thermistorBetaValue = beta_K;
}

void bq769x0::setTemperatureLimits(int minDischarge_degC, int maxDischarge_degC, 
  int minCharge_degC, int maxCharge_degC)
{
  // Temperature limits (°C/10)
  minCellTempDischarge = minDischarge_degC * 10;
  maxCellTempDischarge = maxDischarge_degC * 10;
  minCellTempCharge = minCharge_degC * 10;
  maxCellTempCharge = maxCharge_degC * 10;  
}

void bq769x0::setIdleCurrentThreshold(int current_mA)
{
  idleCurrentThreshold = current_mA;
}


//----------------------------------------------------------------------------

long bq769x0::setShortCircuitProtection(long current_mA, int delay_us)
{
  LOG_PRINTLN("setSCD");
  regPROTECT1_t protect1;
  
  // only RSNS = 1 considered
  protect1.bits.RSNS = 1;

  protect1.bits.SCD_THRESH = 0;
  for (int i = sizeof(SCD_threshold_setting) / sizeof(SCD_threshold_setting[0]) - 1; i > 0; i--) {
    if (current_mA * shuntResistorValue_mOhm / 1000 >= SCD_threshold_setting[i]) {
      protect1.bits.SCD_THRESH = i;
      LOG_PRINT("SCD threshold: ");
      LOG_PRINTLN(i);
      break;
    }
  }
  
  protect1.bits.SCD_DELAY = 0;
  for (int i = sizeof(SCD_delay_setting) / sizeof(SCD_delay_setting[0]) - 1; i > 0; i--) {
    if (delay_us >= SCD_delay_setting[i]) {
      protect1.bits.SCD_DELAY = i;
      LOG_PRINT("SCD delay: ");
      LOG_PRINTLN(i);
      break;
    }
  }
  
  writeRegister(PROTECT1, protect1.regByte);
  
  // returns the actual current threshold value
  return (long)SCD_threshold_setting[protect1.bits.SCD_THRESH] * 1000 / 
    shuntResistorValue_mOhm;
}

//----------------------------------------------------------------------------

long bq769x0::setOvercurrentChargeProtection(long current_mA, int delay_ms)
{
  // ToDo: Software protection for charge overcurrent
  return 0;
}

//----------------------------------------------------------------------------

long bq769x0::setOvercurrentDischargeProtection(long current_mA, int delay_ms)
{
  LOG_PRINTLN("setOCD");
  regPROTECT2_t protect2;
  
  // Remark: RSNS must be set to 1 in PROTECT1 register

  protect2.bits.OCD_THRESH = 0;
  for (int i = sizeof(OCD_threshold_setting) / sizeof(OCD_threshold_setting[0]) - 1; i > 0; i--) {
    if (current_mA * shuntResistorValue_mOhm / 1000 >= OCD_threshold_setting[i]) {
      protect2.bits.OCD_THRESH = i;
      LOG_PRINT("OCD threshold: ");
      LOG_PRINTLN(i);
      break;
    }
  }
  
  protect2.bits.OCD_DELAY = 0;
  for (int i = sizeof(OCD_delay_setting) / sizeof(OCD_delay_setting[0]) - 1; i > 0; i--) {
    if (delay_ms >= OCD_delay_setting[i]) {
      protect2.bits.OCD_DELAY = i;
      LOG_PRINT("OCD delay: ");
      LOG_PRINTLN(i);
      break;
    }
  }
  
  writeRegister(PROTECT2, protect2.regByte);
 
  // returns the actual current threshold value
  return (long)OCD_threshold_setting[protect2.bits.OCD_THRESH] * 1000 / 
    shuntResistorValue_mOhm;
}


//----------------------------------------------------------------------------

int bq769x0::setCellUndervoltageProtection(int voltage_mV, int delay_s)
{
  LOG_PRINTLN("setUVP");
  regPROTECT3_t protect3;
  byte uv_trip = 0;
  
  minCellVoltage = voltage_mV;
  
  protect3.regByte = readRegister(PROTECT3);
  
  uv_trip = ((((long)voltage_mV - adcOffset) * 1000 / adcGain) >> 4) & 0x00FF;
  uv_trip += 1;   // always round up for lower cell voltage
  writeRegister(UV_TRIP, uv_trip);
  
  protect3.bits.UV_DELAY = 0;
  for (int i = sizeof(UV_delay_setting)-1; i > 0; i--) {
    if (delay_s >= UV_delay_setting[i]) {
      protect3.bits.UV_DELAY = i;
      LOG_PRINT("UV_DELAY: ");
      LOG_PRINTLN(i);
      break;
    }
  }
  
  writeRegister(PROTECT3, protect3.regByte);
  
  // returns the actual current threshold value
  return ((long)1 << 12 | uv_trip << 4) * adcGain / 1000 + adcOffset;
}

//----------------------------------------------------------------------------

int bq769x0::setCellOvervoltageProtection(int voltage_mV, int delay_s)
{
  LOG_PRINTLN("setOVP");
  regPROTECT3_t protect3;
  byte ov_trip = 0;

  maxCellVoltage = voltage_mV;
  
  protect3.regByte = readRegister(PROTECT3);
  
  ov_trip = ((((long)voltage_mV - adcOffset) * 1000 / adcGain) >> 4) & 0x00FF;
  writeRegister(OV_TRIP, ov_trip);
    
  protect3.bits.OV_DELAY = 0;
  for (int i = sizeof(OV_delay_setting)-1; i > 0; i--) {
    if (delay_s >= OV_delay_setting[i]) {
      protect3.bits.OV_DELAY = i;
      LOG_PRINT("OV_DELAY: ");
      LOG_PRINTLN(i);
      break;
    }
  }
  
  writeRegister(PROTECT3, protect3.regByte);
 
  // returns the actual current threshold value
  return ((long)1 << 13 | ov_trip << 4) * adcGain / 1000 + adcOffset;
}


//----------------------------------------------------------------------------

long bq769x0::getBatteryCurrent()
{
  return batCurrent;
}

//----------------------------------------------------------------------------

long bq769x0::getBatteryVoltage()
{
  return batVoltage;
}

//----------------------------------------------------------------------------

int bq769x0::getMaxCellVoltage()
{
  return cellVoltages[idCellMaxVoltage];
}

//----------------------------------------------------------------------------

int bq769x0::getCellVoltage(byte idCell)
{
  return cellVoltages[idCell-1];
}


//----------------------------------------------------------------------------

float bq769x0::getTemperatureDegC(byte channel)
{
  if (channel >= 1 && channel <= 3) {
    return (float)temperatures[channel-1] / 10.0;
  }
  else
    return -273.15;   // Error: Return absolute minimum temperature
}

//----------------------------------------------------------------------------

float bq769x0::getTemperatureDegF(byte channel)
{
  return getTemperatureDegC(channel) * 1.8 + 32;
}


//----------------------------------------------------------------------------

void bq769x0::updateTemperatures()
{
  float tmp = 0;
  int adcVal = 0;
  int vtsx = 0;
  unsigned long rts = 0;
  
  Wire.beginTransmission(I2CAddress);
  Wire.write(0x2C);
  Wire.endTransmission();
  
  if (Wire.requestFrom(I2CAddress, 2) == 2)
  {
    // calculate R_thermistor according to bq769x0 datasheet
    adcVal = ((Wire.read() & B00111111) << 8) | Wire.read();
    vtsx = adcVal * 0.382; // mV
    rts = 10000.0 * vtsx / (3300.0 - vtsx); // Ohm
        
    // Temperature calculation using Beta equation
    // - According to bq769x0 datasheet, only 10k thermistors should be used
    // - 25°C reference temperature for Beta equation assumed
    tmp = 1.0/(1.0/(273.15+25) + 1.0/thermistorBetaValue*log(rts/10000.0)); // K
    
    temperatures[0] = (tmp - 273.15) * 10.0;
  }
}


//----------------------------------------------------------------------------
// If ignoreCCReadFlag == true, the current is read independent of an interrupt
// indicating the availability of a new CC reading

void bq769x0::updateCurrent(bool ignoreCCReadyFlag)
{
  LOG_PRINTLN("updateCurrent");
  int16_t adcVal = 0;
  regSYS_STAT_t sys_stat;
  sys_stat.regByte = readRegister(SYS_STAT);
  
  if (ignoreCCReadyFlag == true || sys_stat.bits.CC_READY == 1)
  {
    adcVal = (readRegister(0x32) << 8) | readRegister(0x33);
    batCurrent = adcVal * 8.44 / shuntResistorValue_mOhm;  // mA

    if (batCurrent > -10 && batCurrent < 10)
    {
      batCurrent = 0;
    }
    
    // reset idleTimestamp
    if (batCurrent > idleCurrentThreshold) {
      idleTimestamp = millis();
    }

    // no error occured which caused alert
    if (!(sys_stat.regByte & B00111111)) {
      alertInterruptFlag = false;
    }

    writeRegister(SYS_STAT, B10000000);  // Clear CC ready flag	
    LOG_PRINTLN("updateCurrent: updated, CC flag cleared");
  }
}

//----------------------------------------------------------------------------
// reads all cell voltages and updates batVoltage
// now supports CRC, taken from mbed version of LibreSolar firmware (thanks to mikethezipper)

void bq769x0::updateVoltages()
{
  LOG_PRINTLN("updateVoltages");
  long adcVal = 0;
  char buf[4];
  int connectedCells = 0;
  idCellMaxVoltage = 0; //resets to zero before writing values to these vars
  idCellMinVoltage = 0;

  uint8_t crc;
  crc = 0;
  buf[0] = (char) VC1_HI_BYTE; // start with the first cell
  
  Wire.beginTransmission(I2CAddress);
  Wire.write(buf[0]);     // tell slave that this is the address it is interested in
  Wire.endTransmission(); // end transmission so that read can begin

  /****************************************************\
    Note that each cell voltage is 14 bits stored across two 8 bit register locations in the BQ769x0 chip.
    This means that first we need to read register HI (in first instance this is VC1_HI_BYTE), 
    however this 8 bit piece of data has two worthless first digits - garbage.
    To remove the first two bits, the bitwise & is used. By saying A & 00111111, only the last 6 bits of A are used. 
    Meanwhile all of the 8 bits on the low side are used. So the overall reading is the last 6 bits of high in front of the 8 bits from low.
    To add the hi and lo readings together, the << is used to shift the hi value over by 8 bits, then adding it to the 8 bits.
    This is done by using the OR operator |. So the total thing looks like: adcVal = (VC_HI_BYTE & 0b00111111) << 8 | VC_LO_BYTE;
  \****************************************************/

  // will run once for each cell up to the total num of cells
  
  for (int i = 0; i < numberOfCells; i++) {
    if (crcEnabled == true) {
      // refer to datasheet at 10.3.1.4 "Communications Subsystem"
      Wire.requestFrom(I2CAddress, 4);  // request 4 bytes: 1) VCx_HI - 2) VCx_HI CRC - 3) VCx_LO - 4) VCx_LO CRC
      buf[0] = Wire.read();             // VCx_HI - note that only bottom 6 bits are good
      buf[1] = Wire.read();             // VCx_HI CRC - done on address and data
      buf[2] = Wire.read();             // VCx_LO - all 8 bits are used
      buf[3] = Wire.read();             // VCx_LO CRC - done on just the data byte
      
      // check if CRC matches data bytes
      // CRC of first byte includes slave address (including R/W bit) and data
      crc = _crc8_ccitt_update(0, (I2CAddress << 1) | 1);
      crc = _crc8_ccitt_update(crc, buf[0]);
      // if (crc != buf[1]){
      //   LOG_PRINTLN("VCxHI CRC doesn't match");
      //   return; // to exit without saving corrupted value
      // }
        
      // CRC of subsequent bytes contain only data
      crc = _crc8_ccitt_update(0, buf[2]);
      // if (crc != buf[3]) {
      //   LOG_PRINTLN("VCxLO CRC doesn't match");
      //   return; // to exit without saving corrupted value
      // }
    } 

    // if CRC is disabled only read 2 bytes and call it a day :)
    else { 
      Wire.requestFrom(I2CAddress, 2);
      buf[0] = Wire.read(); // VCx_HI - note that only bottom 6 bits are good
      buf[2] = Wire.read(); // VCx_LO - all 8 bits are used
    }

    // combine VCx_HI and VCx_LO bits and calculate cell voltage
    adcVal = (buf[0] & 0b00111111) << 8 | buf[2];           // read VCx_HI bits and drop the first two bits, shift left then append VCx_LO bits
    cellVoltages[i] = adcVal * adcGain / 1000 + adcOffset;  // calculate real voltage in mV
  
    // filter out voltage readings from unconnected cell(s)
    if (cellVoltages[i] > 500) {  
      connectedCells++; // add one to the temporary cell counter var - only readings above 500mV are counted towards real cell count
    }

    if (cellVoltages[i] > cellVoltages[idCellMaxVoltage]) {
      idCellMaxVoltage = i;
    }

    if (cellVoltages[i] < cellVoltages[idCellMinVoltage] && cellVoltages[i] > 500) {
      idCellMinVoltage = i;
    }
  }
  
  long adcValPack = ((readRegister(BAT_HI_BYTE) << 8) | readRegister(BAT_LO_BYTE)) & 0b1111111111111111;
  batVoltage = 4 * adcGain * adcValPack / 1000 + (connectedCells * adcOffset); // in original LibreSolar, connectedCells is converted to byte, maybe to reduce bit size
}

//----------------------------------------------------------------------------
// now supports CRC, taken from mbed version of LibreSolar firmware (thanks to mikethezipper)

void bq769x0::writeRegister(byte address, int data)
{
  LOG_PRINT("write: ");
  LOG_PRINT(byte2char(address));
  LOG_PRINT(" --> ");
  LOG_PRINT(byte2char(data));
  uint8_t crc = 0;
  char buf[3];
  buf[0] = (char) address;
  buf[1] = data;

  // note that writes to the bq769x0 IC are: 1) start - 2) address - 3) address - 4) data - 5) CRC8 - 6) stop bit
  Wire.beginTransmission(I2CAddress); // writes start bit - the first step
  Wire.write(buf[0]);                 // writes register address
  Wire.write(buf[1]);                 // writes data - the fourth step
 
  if (crcEnabled == true) {
    // CRC is calculated over the slave address (including R/W bit), register address, and data.
    crc = _crc8_ccitt_update(crc, (I2CAddress << 1) | 0);
    crc = _crc8_ccitt_update(crc, buf[0]);
    crc = _crc8_ccitt_update(crc, buf[1]);
    buf[2] = crc;

    Wire.write(buf[2]); // writes CRC
    LOG_PRINT(" CRC:");
    LOG_PRINT(byte2char(buf[2]));
  }

  Wire.endTransmission();
  LOG_PRINTLN();
}

//----------------------------------------------------------------------------

int bq769x0::readRegister(byte address)
{  
  Wire.beginTransmission(I2CAddress);
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(I2CAddress, 1);
  return Wire.read();
}

//----------------------------------------------------------------------------
// supports CRC, taken from mbed version of LibreSolar firmware
// however often freeze momentarily at "while (crc != buf[1]);"

// int bq769x0::readRegister(byte address)
// {
//   LOG_PRINT("read: ");
//   LOG_PRINTLN(byte2char(address));

//   uint8_t crc = 0;
//   char buf[2];
//   buf[0] = (char) address;

//   Wire.beginTransmission(I2CAddress);
//   Wire.write(buf[0]);
//   Wire.endTransmission();

//   if (crcEnabled == true) {
//     do {
//       Wire.requestFrom(I2CAddress, 2);
//       buf[0] = Wire.read();
//       buf[1] = Wire.read();
//       // CRC is calculated over the slave address (including R/W bit) and data.
//       crc = _crc8_ccitt_update(crc, (I2CAddress << 1) | 1);
//       crc = _crc8_ccitt_update(crc, buf[0]);
//     } while (crc != buf[1]);
//     return buf[0];
//   }
//   else {
//     Wire.requestFrom(I2CAddress, 1);
//     return Wire.read();
//   }
// }

//----------------------------------------------------------------------------
// the actual ISR, called by static function alertISR()

void bq769x0::setAlertInterruptFlag()
{
  interruptTimestamp = millis();
  alertInterruptFlag = true;
}

//----------------------------------------------------------------------------
// The bq769x0 drives the ALERT pin high if the SYS_STAT register contains
// a new value (either new CC reading or an error)

void bq769x0::alertISR()
{
  if (instancePointer != 0)
  {
    instancePointer->setAlertInterruptFlag();
  }
}

//----------------------------------------------------------------------------
// for debug purposes
#if BQ769X0_DEBUG
  void bq769x0::printRegisters()
  {
    LOG_PRINT(F("0x00 SYS_STAT:  "));
    LOG_PRINTLN(byte2char(readRegister(SYS_STAT)));

    LOG_PRINT(F("0x01 CELLBAL1:  "));
    LOG_PRINTLN(byte2char(readRegister(CELLBAL1)));

    LOG_PRINT(F("0x02 CELLBAL2:  "));
    LOG_PRINTLN(byte2char(readRegister(CELLBAL2)));

    LOG_PRINT(F("0x03 CELLBAL3:  "));
    LOG_PRINTLN(byte2char(readRegister(CELLBAL3)));

    LOG_PRINT(F("0x04 SYS_CTRL1: "));
    LOG_PRINTLN(byte2char(readRegister(SYS_CTRL1)));
    
    LOG_PRINT(F("0x05 SYS_CTRL2: "));
    LOG_PRINTLN(byte2char(readRegister(SYS_CTRL2)));
    
    LOG_PRINT(F("0x06 PROTECT1:  "));
    LOG_PRINTLN(byte2char(readRegister(PROTECT1)));
    
    LOG_PRINT(F("0x07 PROTECT2:  "));
    LOG_PRINTLN(byte2char(readRegister(PROTECT2)));
    
    LOG_PRINT(F("0x08 PROTECT3   "));
    LOG_PRINTLN(byte2char(readRegister(PROTECT3)));
    
    LOG_PRINT(F("0x09 OV_TRIP:   "));
    LOG_PRINTLN(byte2char(readRegister(OV_TRIP)));
    
    LOG_PRINT(F("0x0A UV_TRIP:   "));
    LOG_PRINTLN(byte2char(readRegister(UV_TRIP)));
    
    LOG_PRINT(F("0x0B CC_CFG:    "));
    LOG_PRINTLN(byte2char(readRegister(CC_CFG)));

    LOG_PRINT(F("0x2A BAT_HI:    "));
    LOG_PRINTLN(byte2char(readRegister(BAT_HI_BYTE)));

    LOG_PRINT(F("0x2B BAT_LO:    "));
    LOG_PRINTLN(byte2char(readRegister(BAT_LO_BYTE)));

    LOG_PRINT(F("0x32 CC_HI:     "));
    LOG_PRINTLN(byte2char(readRegister(CC_HI_BYTE)));

    LOG_PRINT(F("0x33 CC_LO:     "));
    LOG_PRINTLN(byte2char(readRegister(CC_LO_BYTE)));

    // LOG_PRINT(F("0x50 ADCGAIN1:  "));
    // LOG_PRINTLN(byte2char(readRegister(ADCGAIN1)));

    // LOG_PRINT(F("0x51 ADCOFFSET: "));
    // LOG_PRINTLN(byte2char(readRegister(ADCOFFSET)));

    // LOG_PRINT(F("0x59 ADCGAIN2:  "));
    // LOG_PRINTLN(byte2char(readRegister(ADCGAIN2)));
  }
#endif
