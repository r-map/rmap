#ifndef _PMS_H_
#define _PMS_H_

#include <Arduino.h>

/*
 Important: Use 3.3V logic

 numbering pins from left to right with "plantower" stamps on the floor

 PIN1     VCC   Positive power 5V
 PIN2     GND   Negative power
 PIN3     SET   Set pin                   /TTL level@3.3V,high level or suspending is normal working status, while low level is sleeping mode.
 PIN4     RX    Serial port receiving pin /TTL level@3.3V
 PIN5     TX    Serial port sending pin   /TTL level@3.3V
 PIN6     RESET Module reset signal       /TTL level@3.3V,low reset.
 PIN7     NC
 PIN8     NC
*/

/*
The active mode is divided into two sub-modes: stable
mode and fast mode. If the concentration change is small the sensor
would run at stable mode with the real interval of 2.3s.And if the change is
big the sensor would be changed to fast mode automatically with the
interval of 200~800ms, the higher of the concentration, the shorter of the
interval.
*/
#define  RESET_DURATION  33U    // See _doHwReset()
#define  DATA_TIMEOUT 2500U     // Time to complete response for waitfor data
#define  WAKEUP_TIME  2500U // Experimentally, time to get ready after reset/wakeup


class Pmsx003 {
  
public:
  enum PmsStatus : uint8_t {
    OK = 0,
      noData,
      readError,
      frameLenMismatch,
      bufferLenMismatch,
      sumError,
      nValues_PmsStatus
      };
  
  enum PmsCmd {
    cmdReadData = 0x0000e2L,
    cmdModePassive = 0x0000e1L,
    cmdModeActive = 0x0100e1L,
    cmdSleep = 0x0000e4L,
    cmdWakeup = 0x0100e4L,
    cmdHwReset = 0x100000L,
    cmdHwSleep = 0x200000L,
    cmdHwWakeup = 0x300000L,
  };

private:

  uint8_t pinSleep;
  uint8_t pinReset;
  Stream *pmsSerial;
  const uint8_t sig[2]{ 0x42, 0x4D };

  
public:  
  Pmsx003(int8_t pinReset=0xFF, int8_t pinSleep=0xFF);
  void begin(Stream *serial);
  size_t available(void);
  PmsStatus read(uint16_t data[], const size_t nData);
  PmsStatus waitForData(uint16_t data[], const size_t nData, unsigned long maxTime=DATA_TIMEOUT);
  void cmd(const PmsCmd cmd);
  
private:
  void _sumBuffer(uint16_t *sum, const uint8_t *buffer, uint16_t cnt);
  void _doHwReset(void);
  void _doHwSleep(bool sleep);



};
#endif
