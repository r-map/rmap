/*
 * DS1307RTC.h - library for DS1307 RTC
 * This library is intended to be uses with Arduino Time.h library functions
 */

#ifndef DS1307RTC_h
#define DS1307RTC_h

#include <Time.h>

// library interface description
class DS1307RTC
{
  // user-accessible "public" interface
  public:
    DS1307RTC();
    static time_t get();
	static uint8_t set(time_t t);
	static uint8_t read(tmElements_t &tm);
	static uint8_t write(tmElements_t &tm);

  private:
	static uint8_t dec2bcd(uint8_t num);
    static uint8_t bcd2dec(uint8_t num);
};

extern DS1307RTC RTC;

#endif
 

