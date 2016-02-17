#include "HIH61XX.h"



HIH61XX::HIH61XX(uint8_t address, uint8_t powerPin)
  : a(address), p(powerPin), f(0), h(0), t(0)
{
  if(p < 255) {
    digitalWrite(p, LOW);
    pinMode(p, OUTPUT);
  }
}



uint8_t HIH61XX::start()
{
  if(p < 255) {
    digitalWrite(p, HIGH);
  }
  f |= RunningFlag;
  return setError(0);
}

uint8_t HIH61XX::stop()
{
  if(p < 255) {
    digitalWrite(p, LOW);
  }
  f &= ~RunningFlag;
  return setError(0);
}



uint8_t HIH61XX::update()
{
  if(!isRunning()) {
    return setError(NotRunningError);
  }
  
  uint8_t x, y, s;
  
  Wire.beginTransmission(a);
  int azer = Wire.endTransmission();
  if(azer == 0) {    
    while(true) {
      delay(10);
      
      Wire.requestFrom(a, (uint8_t) 4);
      if(Wire.available()) {
        x = Wire.read();
        y = Wire.read();
        s = x >> 6;
        
        switch(s) { 
          case 0:
            h = (((uint16_t) (x & 0x3f)) << 8) | y;
            x = Wire.read();
            y = Wire.read();
            t = ((((uint16_t) x) << 8) | y) >> 2;
            Wire.endTransmission();
            return setError(0);
            
          case 1:
            Wire.endTransmission();
            break;
            
          case 2:
            Wire.endTransmission();
            return setError(CommandModeError);
        }
      }
      else {
        return setError(CommunicationError);
      }
    }
  }
  else {
    Serial.print("...");
    Serial.println(azer);
    return setError(ConnectionError);
  }
}



uint8_t HIH61XX::commandRequest(Stream& stream)
{
  if(stream.available()) {
    return commandProcess(stream, stream.read());
  }
  return commandReply(stream, 255);
}

uint8_t HIH61XX::commandProcess(Stream& stream, uint8_t command)
{
  switch(command) {
  //  get humidity
  case 'h':
    return commandReply(stream, 0, humidity());
    
  //  get temperature
  case 't':
    return commandReply(stream, 0, temperature());
    
  //  get i2c address
  case 'a':
    return commandReply(stream, 0, address());
    
  //  get power pin
  case 'p':
    return commandReply(stream, 0, powerPin());
    
  //  update
  case 'u':
    return commandReply(stream, update());
    
  //  start
  case '1':
    return commandReply(stream, start());
    
  //  stop
  case '0':
    return commandReply(stream, stop());
  }
  
  return commandReply(stream, 254);
}
