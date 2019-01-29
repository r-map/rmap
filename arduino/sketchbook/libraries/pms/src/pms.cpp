/*

Copyright (c) 2017 Jacek Banaszczyk
Boost Software License - Version 1.0 - August 17th, 2003

authors:
Paolo Paruno <p.patruno@iperbole.bologna.it>

*/

/*
It is based on [jbanaszczyk/Pms5003](https://github.com/jbanaszczyk/Pms5003) library for Arduino.

To port to python take a look at:
https://github.com/RigacciOrg/AirPi/blob/master/lib/pms5003
*/

#include <pms.h>

void Pmsx003::_sumBuffer(uint16_t *sum, const uint8_t *buffer, uint16_t cnt) {
  for (; cnt > 0; --cnt, ++buffer) {
    *sum += *buffer;
  }
}


void __attribute__((always_inline))swapEndianBig16(uint16_t* value) {
  constexpr union {
    // endian.test16 == 0x0001 for low endian
    // endian.test16 == 0x0100 for big endian
    // should be properly optimized by compiler
    uint16_t test16;
    uint8_t test8[2];
  } endian = { .test8 = {1, 0} };
  
  if (endian.test16 != 0x0100) {
    const uint16_t hi = (*value & 0xff00) >> 8;
    const uint16_t lo = (*value & 0x00ff) << 8;
    *value = lo | hi;
  }
}

void  Pmsx003::_doHwReset(void) {
  if (pinReset == 0xFF) {
    return;
  }

  digitalWrite(pinReset, LOW);
  delay(RESET_DURATION);
  digitalWrite(pinReset, HIGH);
  delay(WAKEUP_TIME);
}

void  Pmsx003::_doHwSleep(bool sleep) {
  if (pinSleep == 0xFF) {
    return;
  }
  
  digitalWrite(pinSleep, sleep== true ? LOW : HIGH);
}

Pmsx003::Pmsx003(int8_t pinReset, int8_t pinSleep) {
  this->pinReset = pinReset;
  this->pinSleep = pinSleep;
}

void Pmsx003::begin(Stream *serial) {
  pmsSerial = serial;

  if (pinSleep != 0xFF) pinMode(pinSleep, OUTPUT);
  _doHwSleep(false);
    
  if (pinReset != 0xFF) {
    pinMode(pinReset, OUTPUT);
    _doHwReset();
  }

  cmd(cmdSleep);
  this->pmsSerial->flush();

}

size_t Pmsx003::available(void) {
  while (this->pmsSerial->available()) {
    if (this->pmsSerial->peek() != sig[0]) {
      //Serial.print(F("skip: "));
      //Serial.println(this->pmsSerial->read());
      this->pmsSerial->read();
    } else {
      break;
    }
  }
  return this->pmsSerial->available();
}

Pmsx003::PmsStatus Pmsx003::read(uint16_t data[], const size_t nData) {

  //Serial.println("read");

  if ( nData < 13) {
    return bufferLenMismatch;
  }
  

  if (available() < 32) {
    return noData;
  }

  //Serial.print("available: ");
  //Serial.println(available());
  
  //if (available() < (nData + 2) * sizeof*data + sizeof(sig)) {
  //  return noData;
	//}
  
  this->pmsSerial->read(); // Value is equal to sig[0]. There is no need to check the value, it was checked by prior peek()
  
  if (this->pmsSerial->read() != sig[1]) { // The rest of the buffer will be invalidated during the next read attempt
    this->pmsSerial->flush();
    return readError;
  }
  
  uint16_t sum{ 0 };
  _sumBuffer(&sum, (uint8_t *)&sig, sizeof(sig));
  
  uint16_t thisFrameLen;
  if (this->pmsSerial->readBytes((uint8_t*)&thisFrameLen, sizeof(thisFrameLen)) != sizeof(thisFrameLen)) {
    this->pmsSerial->flush();
    return readError;
  };
  
  _sumBuffer(&sum, (uint8_t *)&thisFrameLen, sizeof(thisFrameLen));

  swapEndianBig16(&thisFrameLen);
  
  //Serial.print("framelen: ");
  //Serial.println(thisFrameLen);
  
  if (thisFrameLen  != 28) {
    this->pmsSerial->flush();
    return frameLenMismatch;
  }

  if (this->pmsSerial->readBytes((uint8_t*)data, 26) != 26) {
    this->pmsSerial->flush();
    return readError;
  }
  _sumBuffer(&sum, (uint8_t*)data, 26);

  for (size_t i = 0; i < nData; ++i) {
    swapEndianBig16(&data[i]);
  }
  
  uint16_t crc;
  if (this->pmsSerial->readBytes((uint8_t*)&crc, sizeof(crc)) != sizeof(crc)) {
    this->pmsSerial->flush();
    return readError;
  };

  swapEndianBig16(&crc);
  
  if (sum != crc) {
    this->pmsSerial->flush();
    return sumError;
  }
  
  return OK;
}


Pmsx003::PmsStatus Pmsx003::waitForData(uint16_t data[], const size_t nData, unsigned long maxTime) {
  const unsigned long t0 = millis();
  
  while ((millis()-t0) < maxTime )
    {
      yield();
      Pmsx003::PmsStatus stat= read(data, nData);
      if (stat != noData) return stat;
    }
  return noData;
}

void Pmsx003::cmd(const PmsCmd cmd) {

  switch  (cmd){
  case cmdHwReset:
    _doHwReset();
    break;
  case cmdHwSleep:
    _doHwSleep(true);
    break;
  case cmdHwWakeup:
    _doHwSleep(false);
    break;
  default:
    this->pmsSerial->write(sig, sizeof(sig));
    this->pmsSerial->write((uint8_t*)&cmd, 3);
  
    uint16_t sum{ 0 };
    _sumBuffer(&sum, sig, sizeof(sig));
    _sumBuffer(&sum, (uint8_t*)&cmd, 3);
    swapEndianBig16(&sum);
    this->pmsSerial->flush();
    this->pmsSerial->write((uint8_t*)&sum, sizeof(sum)); 
    break;
  }

  if ((cmd != cmdReadData) && (cmd != cmdWakeup)) {
    this->pmsSerial->flush();
  }
}
