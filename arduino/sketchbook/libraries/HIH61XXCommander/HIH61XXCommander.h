#ifndef HONEYWELL_HIH61XXCommander_H
#define HONEYWELL_HIH61XXCommander_H

//  author: Tomas Van Verrewegen <tomasvanverrewegen@telenet.be>
//  version: 0.2

#include <HIH61XX.h>



class HIH61XXCommander
  : public HIH61XX
{
  public:

  enum Error
  {
    NotInCommandModeError                   = 5,
    CommandNotRecognizedOrEEPROMLockedError = 6,
    InvalidArgumentError                    = 7
  };
  
  enum StartupMode { SlowStartup = 0, FastStartup = 1 };
  enum AlarmPolarity { ActiveHighPolarity = 0, ActiveLowPolarity = 1 };
  enum AlarmOuputConfig { PushPullOuputConfig = 0, OpenDrainOuputConfig = 1 };
  
  HIH61XXCommander(uint8_t address, uint8_t powerPin);

  uint8_t setAddress(uint8_t address);
  
//   uint8_t setPowerPin(uint8_t power);
  
  bool isCommandMode() const { return f & CommandModeFlag; }
  bool isEEPROMUpdateNeeded() const { return g; }

  //  start / stop the device
  uint8_t start();
  uint8_t stop();
  uint8_t restart();
  
  //  conversion
  static uint16_t humidityToRaw(float rh) { return rh * 16382; }
  
  //  enter / leave command mode
  uint8_t enterCommandMode();
  uint8_t leaveCommandMode();
  
  //  access to raw eeprom data
  const uint16_t* eeprom() const;
  void setEEPROM(uint16_t* eeprom);
  
  //  read / write / reset eeprom data
  uint8_t readEEPROM();
  uint8_t writeEEPROM();
  uint8_t resetEEPROM();

  //  eeprom, split in pieces
  
  StartupMode startupMode() const { return (StartupMode) bitRead(e[4], 13); }
  void setStartupMode(StartupMode startupMode);
  
  uint16_t highAlarmOn_Raw() const { return e[0]; }
  void setHighAlarmOn_Raw(uint16_t raw);

  uint16_t highAlarmOff_Raw() const { return e[1]; }
  void setHighAlarmOff_Raw(uint16_t raw);
  
  float highAlarmOn() const { return rawToHumidity(highAlarmOn_Raw()); }
  void setHighAlarmOn(float rh) { setHighAlarmOn_Raw(humidityToRaw(rh)); }
  
  float highAlarmOff() const { return rawToHumidity(highAlarmOff_Raw()); }
  void setHighAlarmOff(float rh) { setHighAlarmOff_Raw(humidityToRaw(rh)); }
  
  AlarmPolarity highAlarmPolarity() const { return (AlarmPolarity) bitRead(e[4], 9); }
  void setHighAlarmPolarity(AlarmPolarity polarity);
  
  AlarmOuputConfig highAlarmOutputConfig() const { return (AlarmOuputConfig) bitRead(e[4], 10); }
  void setHighAlarmOutputConfig(AlarmOuputConfig config);
  
  uint16_t lowAlarmOn_Raw() const { return e[2]; }
  void setLowAlarmOn_Raw(uint16_t raw);
  
  uint16_t lowAlarmOff_Raw() const { return e[3]; }
  void setLowAlarmOff_Raw(uint16_t raw);
  
  float lowAlarmOn() const { return rawToHumidity(lowAlarmOn_Raw()); }
  void setLowAlarmOn(float rh) { setLowAlarmOn_Raw(humidityToRaw(rh)); }
  
  float lowAlarmOff() const { return rawToHumidity(lowAlarmOff_Raw()); }
  void setLowAlarmOff(float rh) { setLowAlarmOff_Raw(humidityToRaw(rh)); }
  
  AlarmPolarity lowAlarmPolarity() const { return (AlarmPolarity) bitRead(e[4], 7); }
  void setLowAlarmPolarity(AlarmPolarity polarity);
  
  AlarmOuputConfig lowAlarmOutputConfig() const { return (AlarmOuputConfig) bitRead(e[4], 8); }
  void setLowAlarmOutputConfig(AlarmOuputConfig config);
  
  uint32_t customerId() const { return (uint32_t(e[5]) << 16) | e[6]; }
  void setCustomerId(uint32_t id);
  
  protected:
    
  uint8_t g;
  uint16_t e[7];
  
  uint8_t commandProcess(Stream& stream, uint8_t command);
  uint8_t commandWrite(uint8_t command, uint8_t data1 = 0, uint8_t data2 = 0);
  uint8_t commandWrite(uint8_t command, uint16_t data);
  uint8_t commandRead(uint8_t* status, uint8_t* data1 = 0, uint8_t* data2 = 0);
  uint8_t commandRead(uint8_t* status, uint16_t* data);
};



#endif
