/**@file pcf8563.cpp */

/**********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "pcf8563.h"

namespace Pcf8563 {
  uint8_t bcdToDec(uint8_t val) {
    return (((val / 16) * 10) + (val % 16));
  }

  uint8_t decToBcd(uint8_t val) {
    return (((val / 10) << 4) + (val % 10));
  }

  bool reset() {
    Wire.beginTransmission(PCF8563_READ_ADDRESS);
    Wire.write(PCF8563_CONTROL_STATUS_1_ADDRESS); // start address
    Wire.write(0x00); // reset control status 1
    Wire.write(0x00); // reset control status 2
    Wire.write(0x00); // reset seconds
    Wire.write(0x00);	// reset minutes
    Wire.write(0x00);	// reset hour
    Wire.write(0x01);	// reset day
    Wire.write(0x00);	// reset weekday
    Wire.write(0x01); // reset month, century
    Wire.write(0x00);	// reset year
    Wire.write(0x80);	// reset minute alarm
    Wire.write(0x80);	// reset hour alarm
    Wire.write(0x80);	// reset day alarm
    Wire.write(0x80);	// reset weekday alarm
    Wire.write(0x00); // reset clockout
    Wire.write(0x00); // reset timer
    if (Wire.endTransmission())
      return false;

    return true;
  }

  bool enable() {
    uint8_t control_status_1;

    if (!getControlStatus1(&control_status_1))
      return false;

    // reset STOP bit (RTC stop)
    control_status_1 &= ~PCF8563_CONTROL_STATUS_1_STOP_BIT;

    if (!setControlStatus1(control_status_1))
      return false;

    return true;
  }

  bool disable() {
    uint8_t control_status_1;

    if (!getControlStatus1(&control_status_1))
      return false;

    // set STOP bit (RTC start)
    control_status_1 |= PCF8563_CONTROL_STATUS_1_STOP_BIT;

    if (!setControlStatus1(control_status_1))
      return false;

    return true;
  }

  bool getControlStatus1(uint8_t *control_status_1) {
    Wire.beginTransmission(PCF8563_READ_ADDRESS);
    Wire.write(PCF8563_CONTROL_STATUS_1_ADDRESS);
    if (Wire.endTransmission())
      return false;

    Wire.requestFrom(PCF8563_READ_ADDRESS, PCF8563_CONTROL_STATUS_1_LENGTH);
    if (Wire.available() < PCF8563_CONTROL_STATUS_1_LENGTH)
      return false;

    *control_status_1 = Wire.read();
    return true;
  }

  bool setControlStatus1(uint8_t control_status_1) {
    Wire.beginTransmission(PCF8563_READ_ADDRESS);
    Wire.write(PCF8563_CONTROL_STATUS_1_ADDRESS);
    Wire.write(control_status_1);
    if (Wire.endTransmission())
      return false;

    return true;
  }

  bool getControlStatus2(uint8_t *control_status_2) {
    Wire.beginTransmission(PCF8563_READ_ADDRESS);
    Wire.write(PCF8563_CONTROL_STATUS_2_ADDRESS);
    if (Wire.endTransmission())
      return false;

    Wire.requestFrom(PCF8563_READ_ADDRESS, PCF8563_CONTROL_STATUS_2_LENGTH);
    if (Wire.available() < PCF8563_CONTROL_STATUS_2_LENGTH)
      return false;

    *control_status_2 = Wire.read();
    return true;
  }

  bool setControlStatus2(uint8_t control_status_2) {
    Wire.beginTransmission(PCF8563_READ_ADDRESS);
    Wire.write(PCF8563_CONTROL_STATUS_2_ADDRESS);
    Wire.write(control_status_2);
    if (Wire.endTransmission())
      return false;

    return true;
  }

  bool getClockoutControl(uint8_t *clockout_control) {
    Wire.beginTransmission(PCF8563_READ_ADDRESS);
    Wire.write(PCF8563_CLKOUT_CONTROL_ADDRESS);
    if (Wire.endTransmission())
      return false;

    Wire.requestFrom(PCF8563_READ_ADDRESS, PCF8563_CLKOUT_CONTROL_LENGTH);
    if (Wire.available() < PCF8563_CLKOUT_CONTROL_LENGTH)
      return false;

    *clockout_control = Wire.read();
    return true;
  }

  bool setClockoutControl(uint8_t clockout_control) {
    Wire.beginTransmission(PCF8563_READ_ADDRESS);
    Wire.write(PCF8563_CLKOUT_CONTROL_ADDRESS);
    Wire.write(clockout_control);
    if (Wire.endTransmission())
      return false;

    return true;
  }

  bool getTimerControl(uint8_t *timer_control) {
    Wire.beginTransmission(PCF8563_READ_ADDRESS);
    Wire.write(PCF8563_TIMER_CONTROL_ADDRESS);
    if (Wire.endTransmission())
      return false;

    Wire.requestFrom(PCF8563_READ_ADDRESS, PCF8563_TIMER_CONTROL_LENGTH);
    if (Wire.available() < PCF8563_TIMER_CONTROL_LENGTH)
      return false;

    *timer_control = Wire.read();
    return true;
  }

  bool setTimerControl(uint8_t timer_control) {
    Wire.beginTransmission(PCF8563_READ_ADDRESS);
    Wire.write(PCF8563_TIMER_CONTROL_ADDRESS);
    Wire.write(timer_control);
    if (Wire.endTransmission())
      return false;

    return true;
  }

  bool getDate(uint8_t *day, uint8_t *month, uint8_t *year, uint8_t *weekday, uint8_t *century) {
  	Wire.beginTransmission(PCF8563_READ_ADDRESS);
   	Wire.write(PCF8563_DAY_ADDRESS);
   	if (Wire.endTransmission())
      return false;

   	Wire.requestFrom(PCF8563_READ_ADDRESS, PCF8563_DATE_LENGTH);
    if (Wire.available() < PCF8563_DATE_LENGTH)
      return false;

  	*day = bcdToDec(Wire.read() & PCF8563_DAY_MASK);

    if (weekday)
  	 *weekday = bcdToDec(Wire.read() & PCF8563_WEEKDAY_MASK);
    else Wire.read();

  	*month = Wire.read();

    if (century)
      *century = (*month & PCF8563_CENTURY_MASK) ? 0 : 1;

  	*month = bcdToDec(*month & PCF8563_MONTH_MASK);
  	*year = bcdToDec(Wire.read());

    return true;
  }

  bool setDate(uint8_t day, uint8_t month, uint8_t year, uint8_t weekday, uint8_t century) {
     if (day < 1)
      return false;

    switch (month) {
      case 1:
      case 3:
      case 5:
      case 7:
      case 8:
      case 10:
      case 12:
        if (day > 31)
          return false;
        break;

      case 2:
         if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
            if (day > 29)
               return false;
         }
         else {
           if (day > 28)
             return false;
         }
        break;

      case 4:
      case 6:
      case 9:
      case 11:
          if (day > 30)
            return false;
        break;

      default:
        return false;
        break;
    }

    if (year > 99 || weekday > 6 || century > 1)
      return false;

    month = decToBcd(month);

    if (century == 1)
      month &= ~PCF8563_CENTURY_MASK;
    else month |= PCF8563_CENTURY_MASK;

   //  fai una getDateTime in sequenza più veloce ?
   //  wire.xxx è sempre in sequenza ? start-stop

    Wire.beginTransmission(PCF8563_READ_ADDRESS);
    Wire.write(PCF8563_DAY_ADDRESS);
    Wire.write(decToBcd(day));
    Wire.write(decToBcd(weekday));
    Wire.write(month);
    Wire.write(decToBcd(year));
    if (Wire.endTransmission())
      return false;

      return true;
  }

  bool getDateTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds, uint8_t *day, uint8_t *month, uint8_t *year, uint8_t *weekday, uint8_t *century) {
    Wire.beginTransmission(PCF8563_READ_ADDRESS);
   	Wire.write(PCF8563_VL_SECOND_ADDRESS);
    if (Wire.endTransmission())
      return false;

   	Wire.requestFrom(PCF8563_READ_ADDRESS, PCF8563_TIME_LENGTH+PCF8563_DATE_LENGTH);
    if (Wire.available() < PCF8563_TIME_LENGTH+PCF8563_DATE_LENGTH)
      return false;

   *seconds = bcdToDec(Wire.read() & PCF8563_SECOND_MASK);
  	*minutes = bcdToDec(Wire.read() & PCF8563_MINUTE_MASK);
  	*hours = bcdToDec(Wire.read() & PCF8563_HOUR_MASK);

   *day = bcdToDec(Wire.read() & PCF8563_DAY_MASK);

    if (weekday)
  	   *weekday = bcdToDec(Wire.read() & PCF8563_WEEKDAY_MASK);
    else Wire.read();

  	*month = Wire.read();

    if (century)
      *century = (*month & PCF8563_CENTURY_MASK) ? 0 : 1;

  	*month = bcdToDec(*month & PCF8563_MONTH_MASK);
  	*year = bcdToDec(Wire.read());

    return true;
  }

  bool getTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds) {
    Wire.beginTransmission(PCF8563_READ_ADDRESS);
   	Wire.write(PCF8563_VL_SECOND_ADDRESS);
    if (Wire.endTransmission())
      return false;

   	Wire.requestFrom(PCF8563_READ_ADDRESS, PCF8563_TIME_LENGTH);
    if (Wire.available() < PCF8563_TIME_LENGTH)
      return false;

   	*seconds = bcdToDec(Wire.read() & PCF8563_SECOND_MASK);
  	*minutes = bcdToDec(Wire.read() & PCF8563_MINUTE_MASK);
  	*hours = bcdToDec(Wire.read() & PCF8563_HOUR_MASK);

    return true;
  }

  bool setTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    if (hours > 23 || minutes > 59 || seconds > 59)
      return false;

    Wire.beginTransmission(PCF8563_READ_ADDRESS);
    Wire.write(PCF8563_VL_SECOND_ADDRESS);
    Wire.write(decToBcd(seconds));
    Wire.write(decToBcd(minutes));
    Wire.write(decToBcd(hours));
    if (Wire.endTransmission())
      return false;

    return true;
  }

  bool enableClockout() {
    uint8_t clockout_control;

    if (!getClockoutControl(&clockout_control))
      return false;

    // set FE (enable clockout)
    clockout_control |= PCF8563_CLKOUT_CONTROL_FE_BIT;

    if (!setClockoutControl(clockout_control))
      return false;

    return true;
  }

  bool disableClockout() {
    uint8_t clockout_control;

    if (!getClockoutControl(&clockout_control))
      return false;

    // reset FE (disable clockout)
    clockout_control &= ~PCF8563_CLKOUT_CONTROL_FE_BIT;

    if (!setClockoutControl(clockout_control))
      return false;

    return true;
  }

  bool setClockoutFrequency(uint8_t frequency) {
    uint8_t clockout_control;

    if (!getClockoutControl(&clockout_control))
      return false;

    // reset FD (frequency)
    clockout_control &= ~PCF8563_CLKOUT_CONTROL_FD_BIT;

    // set FD (frequency)
    clockout_control |= frequency;

    if (!setClockoutControl(clockout_control))
      return false;

    return true;
  }

  bool isClockoutActive() {
    uint8_t clockout_control;

    if (!getClockoutControl(&clockout_control))
      return false;

    return clockout_control & PCF8563_CLKOUT_CONTROL_FE_BIT;
  }

  bool enableAlarm() {
    uint8_t control_status_2;

    if (!getControlStatus2(&control_status_2))
      return false;

    // reset AF
    control_status_2 &= ~PCF8563_CONTROL_STATUS_2_AF_BIT;

    // set AIE (enable alarm interrupt)
    control_status_2 |= PCF8563_CONTROL_STATUS_2_AIE_BIT;

    if (!setControlStatus2(control_status_2))
      return false;

    return true;
  }

  bool disableAlarm() {
    uint8_t control_status_2;

    if (!getControlStatus2(&control_status_2))
      return false;

    // reset AF
    control_status_2 &= ~PCF8563_CONTROL_STATUS_2_AF_BIT;

    // reset AIE (disable alarm interrupt)
    control_status_2 &= ~PCF8563_CONTROL_STATUS_2_AIE_BIT;

    if (!setControlStatus2(control_status_2))
      return false;

    return true;
  }

  bool resetAlarm() {
    uint8_t control_status_2;

    if (!getControlStatus2(&control_status_2))
      return false;

    // reset AF
    control_status_2 &= ~PCF8563_CONTROL_STATUS_2_AF_BIT;

    if (!setControlStatus2(control_status_2))
      return false;

    return true;
  }

  bool isAlarmActive() {
    uint8_t control_status_2;

    if (!getControlStatus2(&control_status_2))
      return false;

    return control_status_2 & PCF8563_CONTROL_STATUS_2_AF_BIT;
  }

  bool getAlarm(uint8_t *hours, uint8_t *minutes, uint8_t *day, uint8_t *weekday) {
    Wire.beginTransmission(PCF8563_READ_ADDRESS);
    Wire.write(PCF8563_MINUTE_ALARM_ADDRESS);
    if (Wire.endTransmission())
      return false;

    Wire.requestFrom(PCF8563_READ_ADDRESS, PCF8563_ALARM_LENGTH);
    if (Wire.available() < PCF8563_ALARM_LENGTH)
      return false;

    *minutes = Wire.read();
    *hours = Wire.read();
    *day = Wire.read();
    *weekday = Wire.read();

    if (PCF8563_ALARM_ENABLE_BIT & *hours)
        *hours = PCF8563_ALARM_DISABLE;
    else *hours = bcdToDec(*hours & PCF8563_ALARM_HOUR_MASK);

    if (PCF8563_ALARM_ENABLE_BIT & *minutes)
        *minutes = PCF8563_ALARM_DISABLE;
    else *minutes = bcdToDec(*minutes & PCF8563_ALARM_MINUTE_MASK);

    if (PCF8563_ALARM_ENABLE_BIT & *day)
        *day = PCF8563_ALARM_DISABLE;
    else *day = bcdToDec(*day & PCF8563_ALARM_DAY_MASK);

    if (PCF8563_ALARM_ENABLE_BIT & *weekday)
        *weekday = PCF8563_ALARM_DISABLE;
    else *weekday = bcdToDec(*weekday & PCF8563_ALARM_WEEKDAY_MASK);

    return true;
  }

  bool setAlarm(uint8_t hours, uint8_t minutes, uint8_t day, uint8_t weekday) {
    if ((hours != PCF8563_ALARM_DISABLE && hours > 23) || (minutes != PCF8563_ALARM_DISABLE && minutes > 59) || (day != PCF8563_ALARM_DISABLE && day > 31) || (weekday != PCF8563_ALARM_DISABLE && weekday > 6))
      return false;

    if (hours == PCF8563_ALARM_DISABLE) {
      hours = 0;
      hours |= PCF8563_ALARM_ENABLE_BIT;
    }
    else {
      hours = decToBcd(hours);
      hours &= ~PCF8563_ALARM_ENABLE_BIT;
    }

    if (minutes == PCF8563_ALARM_DISABLE) {
      minutes = 0;
      minutes |= PCF8563_ALARM_ENABLE_BIT;
    }
    else {
      minutes = decToBcd(minutes);
      minutes &= ~PCF8563_ALARM_ENABLE_BIT;
    }

    if (day == PCF8563_ALARM_DISABLE) {
      day = 0;
      day |= PCF8563_ALARM_ENABLE_BIT;
    }
    else {
      day = decToBcd(day);
      day &= ~PCF8563_ALARM_ENABLE_BIT;
    }

    if (weekday == PCF8563_ALARM_DISABLE) {
      weekday = 0;
      weekday |= PCF8563_ALARM_ENABLE_BIT;
    }
    else {
      weekday = decToBcd(weekday);
      weekday &= ~PCF8563_ALARM_ENABLE_BIT;
    }

    Wire.beginTransmission(PCF8563_READ_ADDRESS);
    Wire.write(PCF8563_MINUTE_ALARM_ADDRESS);
    Wire.write(minutes);
    Wire.write(hours);
    Wire.write(day);
    Wire.write(weekday);
    if (Wire.endTransmission())
      return false;

    return true;
  }

  bool enableTimer() {
    uint8_t control_status_2;
    uint8_t timer_control;

    if (!getControlStatus2(&control_status_2))
      return false;

    if (!getTimerControl(&timer_control))
      return false;

    // reset TI_TP
    control_status_2 &= ~PCF8563_CONTROL_STATUS_2_TI_TP_BIT;

    // reset TF
    control_status_2 &= ~PCF8563_CONTROL_STATUS_2_TF_BIT;

    // set TIE (enable timer interrupt)
    control_status_2 |= PCF8563_CONTROL_STATUS_2_TIE_BIT;

    // set TE (timer enable)
    timer_control |= PCF8563_TIMER_CONTROL_TE_BIT;

    if (!setControlStatus2(control_status_2))
      return false;

    if (!setTimerControl(timer_control))
      return false;

    return true;
  }

  bool disableTimer() {
    uint8_t control_status_2;
    uint8_t timer_control;

    if (!getControlStatus2(&control_status_2))
      return false;

    if (!getTimerControl(&timer_control))
      return false;

    // reset TF
    control_status_2 &= ~PCF8563_CONTROL_STATUS_2_TF_BIT;

    // reset TIE (disable timer interrupt)
    control_status_2 &= ~PCF8563_CONTROL_STATUS_2_TIE_BIT;

    // reset TE (timer disable)
    timer_control &= ~PCF8563_TIMER_CONTROL_TE_BIT;

    if (!setControlStatus2(control_status_2))
      return false;

    if (!setTimerControl(timer_control))
      return false;

    return true;
  }

  bool resetTimer() {
    uint8_t control_status_2;

    if (!getControlStatus2(&control_status_2))
      return false;

    // reset TF
    control_status_2 &= ~PCF8563_CONTROL_STATUS_2_TF_BIT;

    if (!setControlStatus2(control_status_2))
      return false;

    return true;
  }

  bool isTimerActive() {
    uint8_t control_status_2;

    if (!getControlStatus2(&control_status_2))
      return false;

    return control_status_2 & PCF8563_CONTROL_STATUS_2_TF_BIT;
  }

  bool setTimer(uint8_t frequency, uint8_t timer) {
    uint8_t timer_control;

    if (frequency > PCF8563_TIMER_FREQUENCY_1_60_HZ)
      return false;

    if (!getTimerControl(&timer_control))
      return false;

    timer_control |= frequency;

    if (!setTimerControl(timer_control))
      return false;

    Wire.beginTransmission(PCF8563_READ_ADDRESS);
    Wire.write(PCF8563_TIMER_ADDRESS);
    Wire.write(timer);
    if (Wire.endTransmission())
      return false;

    return true;
  }

  int16_t getDaysFromTwoDate (int16_t d1, int16_t m1, int16_t y1, int16_t d2, int16_t m2, int16_t y2) {
    m1 = (m1 + 9) % 12;
    y1 = y1 - m1 / 10;
    int16_t x1 = 365 * y1 + y1 / 4 - y1 / 100 + y1 / 400 + (m1 * 306 + 5) / 10 + (d1 - 1);
    m2 = (m2 + 9) % 12;
    y2 = y2 - m2 / 10;
    int16_t x2 = 365 * y2 + y2 / 4 - y2 / 100 + y2 / 400 + (m2 * 306 + 5) / 10 + (d2 - 1);
    return x2 - x1;
  }

  uint32_t getTime() {
    uint32_t seconds_since_1970;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    if (!getDateTime(&hours, &minutes, &seconds, &day, &month, &year))
      return 0;

    seconds_since_1970 = getDaysFromTwoDate(1, 1, 1970, day, month, year+2000) * 86400UL;
    seconds_since_1970 += hours * 3600UL;
    seconds_since_1970 += minutes * 60UL;
    seconds_since_1970 += seconds;

    return seconds_since_1970;
  }
}
