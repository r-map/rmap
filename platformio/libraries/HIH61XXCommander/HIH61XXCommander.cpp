#include "HIH61XXCommander.h"



HIH61XXCommander::HIH61XXCommander(uint8_t address, uint8_t powerPin)
  : HIH61XX(address, powerPin), g(0)
{
  for(int i = 0; i < 7; ++i) {
    e[i] = 0;
  }
}



uint8_t HIH61XXCommander::start()
{
  if(!isRunning()) {
    setError(0);
    digitalWrite(p, HIGH);
    f |= RunningFlag;
    uint8_t result = enterCommandMode();
    if(result == 0) {
      readEEPROM();
      leaveCommandMode();
    }
    return result;
  }
  return 0;
}

uint8_t HIH61XXCommander::stop()
{
  if(isRunning()) {
    uint8_t result = 0;
    if(isEEPROMUpdateNeeded()) {
      if(!isCommandMode()) {
        digitalWrite(p, LOW);
        delay(1);
        digitalWrite(p, HIGH);
        result = enterCommandMode();
      }

      if(result == 0) {
        result = writeEEPROM();
        if(result == 0) {
          a = e[4] & B01111111;
        }
      }
    }
    digitalWrite(p, LOW);
    f &= ~RunningFlag;
    return 0;
  }
  else return 1;
}

uint8_t HIH61XXCommander::restart()
{
  stop();
  delay(1);
  return start();
}



uint8_t HIH61XXCommander::enterCommandMode()
{
  setError(0);
  
  if(!isRunning()) {
    return setError(NotRunningError);
  }
  
  if(isCommandMode()) {
    return setError(CommandModeError);
  }
  
  commandWrite(0xA0);
  
  uint8_t s;
  while(true) {
    delayMicroseconds(10);
    commandRead(&s);
    switch(s & B00000011) {
      case 0:
        break;
        
      case 1:
        f |= CommandModeFlag;
        return 0;
        
      default:
        return setError(CommandNotRecognizedOrEEPROMLockedError);
    }
  }
}

uint8_t HIH61XXCommander::leaveCommandMode()
{
  setError(0);
  
  if(!isRunning()) {
    return setError(NotRunningError);
  }
  
  if(!isCommandMode()) {
    return setError(NotInCommandModeError);
  }
  
  commandWrite(0x80);
  delay(100);
  
  f &= ~CommandModeFlag;
  return 0;
}



const uint16_t* HIH61XXCommander::eeprom() const
{
  return e;
}

void HIH61XXCommander::setEEPROM(uint16_t* eeprom)
{
  for(int i = 0; i < 8; ++i) {
    e[i] = eeprom[i];
  }
  g = B01111111;
}



uint8_t HIH61XXCommander::readEEPROM()
{
  setError(0);
  
  if(!isRunning()) {
    return setError(NotRunningError);
  }
  
  if(!isCommandMode()) {
    return setError(NotInCommandModeError);
  }
  
  uint8_t s;
  const uint8_t reg[7] = {0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1E, 0x1F};
  
  for(int i = 0; i < 8; ++i) {
    commandWrite(reg[i]);
    delayMicroseconds(150);
    commandRead(&s, &e[i]);
  }
  
  return 0;
}

uint8_t HIH61XXCommander::writeEEPROM()
{
  setError(0);
  
  if(!isRunning()) {
    return setError(NotRunningError);
  }
  
  if(!isCommandMode()) {
    return setError(NotInCommandModeError);
  }
   
  const uint8_t reg[7] = {0x18 | 64, 0x19 | 64, 0x1A | 64, 0x1B | 64, 0x1C | 64, 0x1E | 64, 0x1F | 64};
   
  for(int i = 0; i < 8; ++i) {
    if(bitRead(g, i)) {
      commandWrite(reg[i], e[i]);
      delay(15);
    }
  }
  
  g = 0;
  
  return 0;
}

uint8_t HIH61XXCommander::resetEEPROM()
{
  setError(0);
  
  if(!isRunning()) {
    return setError(NotRunningError);
  }
  
  if(!isCommandMode()) {
    return setError(NotInCommandModeError);
  }
  
  uint8_t s;
  const uint8_t reg[7] = {0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1E, 0x1F};
  
  for(int i = 0; i < 8; ++i) {
    if(bitRead(g, i)) {
      commandWrite(reg[i]);
      delayMicroseconds(150);
      commandRead(&s, &e[i]);
    }
  }
  
  return g = 0;
}



uint8_t HIH61XXCommander::setAddress(uint8_t address)
{
  setError(0);
  
  if(address == 0 || address > 120) {
    return setError(InvalidArgumentError);
  }
  
  e[4] = (e[4] & 0xFF80) | address;
  bitWrite(g, 4, 1);
  
  return 0;
}

// uint8_t HIH61XXCommander::setPowerPin(uint8_t power)
// {
//   setError(0);
//   p = power;
//   
//   return 0;
// }



void HIH61XXCommander::setStartupMode(StartupMode startupMode)
{
  setError(0);
  if(this->startupMode() != startupMode) {
    bitWrite(e[4], 13, startupMode);
    bitWrite(g, 4, 1);
  }
}

void HIH61XXCommander::setHighAlarmOn_Raw(uint16_t raw)
{
  setError(0);
  if(highAlarmOn_Raw() != raw) {
    e[0] = raw;
    bitWrite(g, 0, 1);
  }
}

void HIH61XXCommander::setHighAlarmOff_Raw(uint16_t raw)
{
  setError(0);
  if(highAlarmOff_Raw() != raw) {
    e[1] = raw;
    bitWrite(g, 1, 1);
  }
}

void HIH61XXCommander::setHighAlarmPolarity(AlarmPolarity polarity)
{
  setError(0);
  if(highAlarmPolarity() != polarity) {
    bitWrite(e[4], 9, polarity);
    bitWrite(g, 4, 1);
  }
}

void HIH61XXCommander::setHighAlarmOutputConfig(AlarmOuputConfig config)
{
  setError(0);
  if(highAlarmOutputConfig() != config) {
    bitWrite(e[4], 10, config);
    bitWrite(g, 4, 1);
  }
}

void HIH61XXCommander::setLowAlarmOn_Raw(uint16_t raw)
{
  setError(0);
  if(lowAlarmOn_Raw() != raw) {
    e[2] = raw;
    bitWrite(g, 2, 1);
  }
}

void HIH61XXCommander::setLowAlarmOff_Raw(uint16_t raw)
{
  setError(0);
  if(lowAlarmOff_Raw() != raw) {
    e[3] = raw;
    bitWrite(g, 3, 1);
  }
}

void HIH61XXCommander::setLowAlarmPolarity(AlarmPolarity polarity)
{
  setError(0);
  if(lowAlarmPolarity() != polarity) {
    bitWrite(e[4], 7, polarity);
    bitWrite(g, 4, 1);
  }
}

void HIH61XXCommander::setLowAlarmOutputConfig(AlarmOuputConfig config)
{
  setError(0);
  if(lowAlarmOutputConfig() != config) {
    bitWrite(e[4], 8, config);
    bitWrite(g, 4, 1);
  }
}

void HIH61XXCommander::setCustomerId(uint32_t id)
{
  setError(0);
  if(customerId() != id) {
    e[5] = id >> 16;
    e[6] = id & 0x0000FFFF;
    bitWrite(g, 5, 1);
    bitWrite(g, 6, 1);
  }
}



uint8_t HIH61XXCommander::commandWrite(uint8_t command, uint8_t data1, uint8_t data2)
{
  Wire.beginTransmission(a);
  Wire.write(command);
  Wire.write(data1);
  Wire.write(data2);
  return Wire.endTransmission();
}

uint8_t HIH61XXCommander::commandWrite(uint8_t command, uint16_t data)
{
  return commandWrite(command, data >> 8, data & 0x00FF);
}

uint8_t HIH61XXCommander::commandRead(uint8_t* status, uint8_t* data1, uint8_t* data2)
{
  Wire.requestFrom(a, (uint8_t) (data1 && data2 ? 3 : 1));
  if(Wire.available()) {
    *status = Wire.read();
    if(data1 && data2 && Wire.available()) {
      *data1 = Wire.read();
      *data2 = Wire.read();
    }
  }
  return Wire.endTransmission();
}

uint8_t HIH61XXCommander::commandRead(uint8_t* status, uint16_t* data)
{
  return commandRead(status, &reinterpret_cast<uint8_t*>(data)[1], &reinterpret_cast<uint8_t*>(data)[0]);
}



uint8_t HIH61XXCommander::commandProcess(Stream& stream, uint8_t command)
{
  switch(command) {    
  // get high alarm ...
  case 'H':
    switch(stream.read()) {
      // get high alarm on trippoint
      case '1':
        return commandReply(stream, 0, highAlarmOn());
        
      // get high alarm off trippoint
      case '0':
        return commandReply(stream, 0, highAlarmOff());
        
      // get high alarm polarity
      case 'p':
        return commandReply(stream, 0, highAlarmPolarity());
        
      // get high alarm output config
      case 'o':
        return commandReply(stream, 0, highAlarmOutputConfig());
    }
    break;
    
  // get low alarm ...
  case 'L':
    switch(stream.read()) {
      // get low alarm on trippoint
      case '1':
        return commandReply(stream, 0, lowAlarmOn());
        
      // get low alarm off trippoint
      case '0':
        return commandReply(stream, 0, lowAlarmOff());
        
      // get low alarm polarity
      case 'p':
        return commandReply(stream, 0, lowAlarmPolarity());
        
      // get low alarm output config
      case 'o':
        return commandReply(stream, 0, lowAlarmOutputConfig());
    }
    break;
    
  // get startup mode
  case 's':
    return commandReply(stream, 0, startupMode());
    
  // get customer id
  case 'c':
    return commandReply(stream, 0, customerId());
    
  // get eeprom
  case 'e':
    {
      String s;
      for(int i = 0; i < 7; ++i) {
        if(i) {
          s.concat(' ');
        }
        s.concat(e[i]);
      }
      return commandReply(stream, 0, s);
    }
    
  //  set i2c address
  case 'A':
    {
      int a = stream.parseInt();
      if(a > 0 && a < 120) {
        if(setAddress(a) == 0) {
          restart();
          return commandReply(stream, 0);
        }
      }
    }
    return commandReply(stream, 1);
    
  //  set power pin
//   case 'P':
//     return commandReply(stream, 1);
    
  // set high alarm ...
  case 'I':
    switch(stream.read()) {
      // set high alarm on trippoint
      case '1':
        setHighAlarmOn(stream.parseFloat());
        restart();
        return commandReply(stream, 0);
        
      // set high alarm off trippoint
      case '0':
        setHighAlarmOff(stream.parseFloat());
        restart();
        return commandReply(stream, 0);
        
      // set high alarm polarity
      case 'p':
        setHighAlarmPolarity((AlarmPolarity) stream.parseInt());
        restart();
        return commandReply(stream, 0);
        
      // set high alarm output config
      case 'o':
        setHighAlarmOutputConfig((AlarmOuputConfig) stream.parseInt());
        restart();
        return commandReply(stream, 0);
    }
    break;
  
  // set low alarm ...
  case 'M':
    switch(stream.read()) {
      // set low alarm on trippoint
      case '1':
        setLowAlarmOn(stream.parseFloat());
        restart();
        return commandReply(stream, 0);
        
      // set low alarm off trippoint
      case '0':
        setLowAlarmOff(stream.parseFloat());
        restart();
        return commandReply(stream, 0);
        
      // set low alarm polarity
      case 'p':
        setLowAlarmPolarity((AlarmPolarity) stream.parseInt());
        restart();
        return commandReply(stream, 0);
        
      // set low alarm output config
      case 'o':
        setLowAlarmOutputConfig((AlarmOuputConfig) stream.parseInt());
        restart();
        return commandReply(stream, 0);
    }
    break;
    
  // set startup mode
  case 'S':
    setStartupMode((StartupMode) stream.parseInt());
    restart();
    return commandReply(stream, 0);
    
  // set customer id
  case 'C':
    setCustomerId(stream.parseInt());
    restart();
    return commandReply(stream, 0);
    
  // set eeprom
  case 'E':
    {
      uint16_t e[7];
      for(int i = 0; i < 7; ++i) {
        e[i] = stream.parseInt();
      }
      setEEPROM(e);
    }
    restart();
    return commandReply(stream, 0);
    
  // reset eeprom
  case 'R':
    return commandReply(stream, resetEEPROM());
  }
    
  return HIH61XX::commandProcess(stream, command);
}
