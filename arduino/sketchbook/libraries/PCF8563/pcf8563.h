/**@file pcf8563.h */

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

#ifndef _PCF8563_H
#define _PCF8563_H

#include "Arduino.h"
#include <Wire.h>

/*!
\def PCF8563_READ_ADDRESS
\brief I2C read address.
*/
#define PCF8563_READ_ADDRESS                 (0xA3 >> 1)

/*!
\def PCF8563_WRITE_ADDRESS
\brief I2C write address.
*/
#define PCF8563_WRITE_ADDRESS                (0xA2)

/*!
\def PCF8563_CONTROL_STATUS_1_ADDRESS
\brief Control status 1 register i2c address.
*/
#define PCF8563_CONTROL_STATUS_1_ADDRESS     (0x00)

/*!
\def PCF8563_CONTROL_STATUS_2_ADDRESS
\brief Control status 2 register i2c address.
BIT 4: TI_TP, BIT 3: AF, BIT 2: TF, BIT 1: AIE, BIT 0: TIE
*/
#define PCF8563_CONTROL_STATUS_2_ADDRESS     (0x01)

/*!
\def PCF8563_VL_SECOND_ADDRESS
\brief VL second register address.
BIT 0-6: seconds [0-59], BIT 7: VL
*/
#define PCF8563_VL_SECOND_ADDRESS            (0x02)

/*!
\def PCF8563_MINUTE_ADDRESS
\brief Minute register address.
BIT 0-6: minutes [0-59]
*/
#define PCF8563_MINUTE_ADDRESS               (0x03)

/*!
\def PCF8563_HOUR_ADDRESS
\brief Hour register address.
BIT 0-5: hours [0-23]
*/
#define PCF8563_HOUR_ADDRESS                 (0x04)

/*!
\def PCF8563_DAY_ADDRESS
\brief Day register address.
BIT 0-5: days [1-31]
*/
#define PCF8563_DAY_ADDRESS                  (0x05)

/*!
\def PCF8563_WEEKDAY_ADDRESS
\brief Weekday register address.
BIT 0-2: weekdays [0-6] start from Sunday
*/
#define PCF8563_WEEKDAY_ADDRESS              (0x06)

/*!
\def PCF8563_CENTURY_MONTHS_ADDRESS
\brief Century months register address.
BIT 0-4: months [1-12], BIT 7: century [0-1]
*/
#define PCF8563_CENTURY_MONTHS_ADDRESS       (0x07)

/*!
\def PCF8563_YEAR_ADDRESS
\brief Year register address.
BIT 0-7: year [0-99]
*/
#define PCF8563_YEAR_ADDRESS                 (0x08)

/*!
\def PCF8563_MINUTE_ALARM_ADDRESS
\brief Minute alarm register address.
BIT 0-6: minutes alarm [0-59], BIT 7: AE_M (0: enable, 1: disable)
*/
#define PCF8563_MINUTE_ALARM_ADDRESS         (0x09)

/*!
\def PCF8563_HOUR_ALARM_ADDRESS
\brief Hour alarm register address.
BIT 0-5: hours alarm [0-23], BIT 7: AE_H (0: enable, 1: disable)
*/
#define PCF8563_HOUR_ALARM_ADDRESS           (0x0A)

/*!
\def PCF8563_DAY_ALARM_ADDRESS
\brief Day alarm register address.
BIT 0-5: days alarm [1-31], BIT 7: AE_D (0: enable, 1: disable)
*/
#define PCF8563_DAY_ALARM_ADDRESS            (0x0B)

/*!
\def PCF8563_WEEKDAY_ALARM_ADDRESS
\brief Weekday alarm register address.
BIT 0-2: weekdays alarm [0-6], BIT 7: AE_W (0: enable, 1: disable)
*/
#define PCF8563_WEEKDAY_ALARM_ADDRESS        (0x0C)

/*!
\def PCF8563_CLKOUT_CONTROL_ADDRESS
\brief Clockout alarm register address.
*/
#define PCF8563_CLKOUT_CONTROL_ADDRESS       (0x0D)

/*!
\def PCF8563_TIMER_CONTROL_ADDRESS
\brief Timer control register address.
*/
#define PCF8563_TIMER_CONTROL_ADDRESS        (0x0E)

/*!
\def PCF8563_TIMER_ADDRESS
\brief Timer register address.
*/
#define PCF8563_TIMER_ADDRESS                (0x0F)

/*!
\def PCF8563_SECOND_MASK
\brief Second mask.
*/
#define PCF8563_SECOND_MASK                  (0b01111111)

/*!
\def PCF8563_MINUTE_MASK
\brief Minute mask.
*/
#define PCF8563_MINUTE_MASK                  (0b01111111)

/*!
\def PCF8563_HOUR_MASK
\brief Hour mask.
*/
#define PCF8563_HOUR_MASK                    (0b00111111)

/*!
\def PCF8563_DAY_MASK
\brief Day mask.
*/
#define PCF8563_DAY_MASK                     (0b00111111)

/*!
\def PCF8563_WEEKDAY_MASK
\brief Weekday mask.
*/
#define PCF8563_WEEKDAY_MASK                 (0b00000111)

/*!
\def PCF8563_CENTURY_MASK
\brief Alarm century mask.
*/
#define PCF8563_CENTURY_MASK                 (0b10000000)

/*!
\def PCF8563_MONTH_MASK
\brief Alarm month mask.
*/
#define PCF8563_MONTH_MASK                   (0b00011111)

/*!
\def PCF8563_ALARM_MINUTE_MASK
\brief Alarm minute mask.
*/
#define PCF8563_ALARM_MINUTE_MASK            (0b01111111)

/*!
\def PCF8563_ALARM_HOUR_MASK
\brief Alarm hour mask.
*/
#define PCF8563_ALARM_HOUR_MASK              (0b00111111)

/*!
\def PCF8563_ALARM_DAY_MASK
\brief Alarm day mask.
*/
#define PCF8563_ALARM_DAY_MASK               (0b00111111)

/*!
\def PCF8563_ALARM_WEEKDAY_MASK
\brief Alarm weekday mask.
*/
#define PCF8563_ALARM_WEEKDAY_MASK           (0b00000111)

/*!
\def PCF8563_ALARM_ENABLE_BIT
\brief Alarm enable bit.
BIT 7: AE_M, AE_H, AE_D, AE_W (0: enable, 1: disable)
*/
#define PCF8563_ALARM_ENABLE_BIT             (0b10000000)

/*!
\def PCF8563_CONTROL_STATUS_1_STOP_BIT
\brief Control status 1 stop bit.
BIT 5: RTC source clock runs (0: enable, 1: disable)
*/
#define PCF8563_CONTROL_STATUS_1_STOP_BIT    (0b00100000)

/*!
\def PCF8563_CONTROL_STATUS_2_TI_TP_BIT
\brief Control status 2 TI TP bit.
BIT 4: Pulse generator (0: disable, 1: enable)
*/
#define PCF8563_CONTROL_STATUS_2_TI_TP_BIT   (0b00010000)

/*!
\def PCF8563_CONTROL_STATUS_2_AF_BIT
\brief Control status 2 AF bit.
BIT 3: Alarm Flag (0: inactive, 1: active)
*/
#define PCF8563_CONTROL_STATUS_2_AF_BIT      (0b00001000)

/*!
\def PCF8563_CONTROL_STATUS_2_TF_BIT
\brief Control status 2 TF bit.
BIT 2: Timer Flag (0: inactive, 1: active)
*/
#define PCF8563_CONTROL_STATUS_2_TF_BIT      (0b00000100)

/*!
\def PCF8563_CONTROL_STATUS_2_AIE_BIT
\brief Control status 2 AIE bit.
BIT 1: Alarm Interrupt Enable (0: disable, 1: enable)
*/
#define PCF8563_CONTROL_STATUS_2_AIE_BIT     (0b00000010)

/*!
\def PCF8563_CONTROL_STATUS_2_TIE_BIT
\brief Control status 2 TIE bit.
BIT 0: Timer Interrupt Enable (0: disable, 1: enable)
*/
#define PCF8563_CONTROL_STATUS_2_TIE_BIT     (0b00000001)

/*!
\def PCF8563_TIMER_CONTROL_TE_BIT
\brief Timer control TE bit.
BIT 7: Timer Enable (0: disable, 1: enable)
*/
#define PCF8563_TIMER_CONTROL_TE_BIT         (0b10000000)

/*!
\def PCF8563_TIMER_CONTROL_TD_BIT
\brief Timer control TD bit.
BIT 0-1: 00: 4096 KHz, 01: 64 Hz, 10: 1 Hz, 11: 1/60 Hz
*/
#define PCF8563_TIMER_CONTROL_TD_BIT         (0b00000011)

/*!
\def PCF8563_CLKOUT_CONTROL_FE_BIT
\brief Clockout control FE bit.
BIT 7: Clkout Enable (0: disable, 1: enable)
*/
#define PCF8563_CLKOUT_CONTROL_FE_BIT        (0b10000000)

/*!
\def PCF8563_CLKOUT_CONTROL_FD_BIT
\brief Clockout control FD bit.
BIT 0-1: Frequency clock: 00: 32768 KHz, 01: 1024 Hz, 10: 32 Hz, 11: 1 Hz
*/
#define PCF8563_CLKOUT_CONTROL_FD_BIT        (0b00000011)


/*!
\def PCF8563_CONTROL_STATUS_1_LENGTH
\brief Length in bytes for control status 1 register.
*/
#define PCF8563_CONTROL_STATUS_1_LENGTH      (1)

/*!
\def PCF8563_CONTROL_STATUS_2_LENGTH
\brief Length in bytes for control status 2 register.
*/
#define PCF8563_CONTROL_STATUS_2_LENGTH      (1)

/*!
\def PCF8563_TIMER_CONTROL_LENGTH
\brief Length in bytes for timer control register.
*/
#define PCF8563_TIMER_CONTROL_LENGTH         (1)

/*!
\def PCF8563_CLKOUT_CONTROL_LENGTH
\brief Length in bytes for clockout control register.
*/
#define PCF8563_CLKOUT_CONTROL_LENGTH        (1)

/*!
\def PCF8563_DATE_LENGTH
\brief Length in bytes for date register.
*/
#define PCF8563_DATE_LENGTH                  (4)

/*!
\def PCF8563_TIME_LENGTH
\brief Length in bytes for time register.
*/
#define PCF8563_TIME_LENGTH                  (3)

/*!
\def PCF8563_ALARM_LENGTH
\brief Length in bytes for alarm register.
*/
#define PCF8563_ALARM_LENGTH                 (4)

/*!
\def PCF8563_ALARM_DISABLE
\brief Value for disable alarm in alarm register.
*/
#define PCF8563_ALARM_DISABLE                (255)

/*!
\def PCF8563_TIMER_FREQUENCY_4096_KHZ
\brief Timer frequency is 4096 KHz.
*/
#define PCF8563_TIMER_FREQUENCY_4096_KHZ     (0b00000000)

/*!
\def PCF8563_TIMER_FREQUENCY_64_HZ
\brief Timer frequency is 64 Hz.
*/
#define PCF8563_TIMER_FREQUENCY_64_HZ        (0b00000001)

/*!
\def PCF8563_TIMER_FREQUENCY_1_HZ
\brief Timer frequency is 1 Hz.
*/
#define PCF8563_TIMER_FREQUENCY_1_HZ         (0b00000010)

/*!
\def PCF8563_TIMER_FREQUENCY_1_60_HZ
\brief Timer frequency is 1/60 Hz.
*/
#define PCF8563_TIMER_FREQUENCY_1_60_HZ      (0b00000011)

/*!
\def PCF8563_TIMER_FREQUENCY_SECONDS
\brief Timer frequency is 1 second.
*/
#define PCF8563_TIMER_FREQUENCY_SECONDS      (PCF8563_TIMER_FREQUENCY_1_HZ)

/*!
\def PCF8563_TIMER_FREQUENCY_MINUTES
\brief Timer frequency is 1 minute.
*/
#define PCF8563_TIMER_FREQUENCY_MINUTES      (PCF8563_TIMER_FREQUENCY_1_60_HZ)

/*!
\def PCF8563_CLKOUT_FREQUENCY_32768_KHZ
\brief Clockout frequency is 32768 KHz.
*/
#define PCF8563_CLKOUT_FREQUENCY_32768_KHZ   (0b00000000)

/*!
\def PCF8563_CLKOUT_FREQUENCY_1024_HZ
\brief Clockout frequency is 1024 Hz.
*/
#define PCF8563_CLKOUT_FREQUENCY_1024_HZ     (0b00000001)

/*!
\def PCF8563_CLKOUT_FREQUENCY_32_HZ
\brief Clockout frequency is 32 Hz.
*/
#define PCF8563_CLKOUT_FREQUENCY_32_HZ       (0b00000010)

/*!
\def PCF8563_CLKOUT_FREQUENCY_1_HZ
\brief Clockout frequency is 1 Hz.
*/
#define PCF8563_CLKOUT_FREQUENCY_1_HZ        (0b00000011)

/*!
\def PCF8563_CLKOUT_FREQUENCY_SECONDS
\brief Clockout frequency is 1 second.
*/
#define PCF8563_CLKOUT_FREQUENCY_SECONDS     (PCF8563_CLKOUT_FREQUENCY_1_HZ)

/*!
\namespace Pcf8563
\brief Pcf8563 namespace.
*/
namespace Pcf8563 {
   /*!
   \fn bool reset()
   \brief Reset PCF8563.
   \return true if succesful, false otherwise.
   */
   bool reset(void);

   /*!
   \fn bool enable()
   \brief Enable PCF8563 counter.
   \return true if succesful, false otherwise.
   */
   bool enable(void);

   /*!
   \fn bool disable()
   \brief Disable PCF8563 counter.
   \return true if succesful, false otherwise.
   */
   bool disable(void);

   /*!
   \fn bool getControlStatus1(uint8_t *control_status_1)
   \brief Read control status 1 register.
   \param[out] *control_status_1 readed register value.
   \return true if succesful, false otherwise.
   */
   bool getControlStatus1(uint8_t *control_status_1);

   /*!
   \fn bool setControlStatus1(uint8_t control_status_1)
   \brief Write control status 1 register.
   \param[in] *control_status_1 value to write on register.
   \return true if succesful, false otherwise.
   */
   bool setControlStatus1(uint8_t control_status_1);

   /*!
   \fn bool getControlStatus2(uint8_t *control_status_2)
   \brief Read control status 2 register.
   \param[out] *control_status_2 readed register value.
   \return true if succesful, false otherwise.
   */
   bool getControlStatus2(uint8_t *control_status_2);

   /*!
   \fn bool setControlStatus2(uint8_t control_status_2)
   \brief Write control status 2 register.
   \param[in] *control_status_2 value to write on register.
   \return true if succesful, false otherwise.
   */
   bool setControlStatus2(uint8_t control_status_2);

   /*!
   \fn bool getClockoutControl(uint8_t *clockout_control)
   \brief Read clockout control register.
   \param[out] *clockout_control readed register value.
   \return true if succesful, false otherwise.
   */
   bool getClockoutControl(uint8_t *clockout_control);

   /*!
   \fn bool setClockoutControl(uint8_t clockout_control)
   \brief Write clockout control register.
   \param[in] clockout_control value to write on register.
   \return true if succesful, false otherwise.
   */
   bool setClockoutControl(uint8_t clockout_control);

   /*!
   \fn bool getTimerControl(uint8_t *timer_control)
   \brief Read timer control register.
   \param[out] *timer_control readed register value.
   \return true if succesful, false otherwise.
   */
   bool getTimerControl(uint8_t *timer_control);

   /*!
   \fn bool setTimerControl(uint8_t timer_control)
   \brief Write timer control register.
   \param[in] *timer_control value to write on register.
   \return true if succesful, false otherwise.
   */
   bool setTimerControl(uint8_t timer_control);

   /*!
   \fn bool enableClockout()
   \brief Enable clockout frequency.
   \return true if succesful, false otherwise.
   */
   bool enableClockout(void);

   /*!
   \fn bool disableClockout()
   \brief Disable clockout frequency.
   \return true if succesful, false otherwise.
   */
   bool disableClockout(void);

   /*!
   \fn bool setClockoutFrequency(uint8_t frequency)
   \brief Set clockout frequency.
   \param[in] frequency frequency value (see relative define).
   \return true if succesful, false otherwise.
   */
   bool setClockoutFrequency(uint8_t frequency);

   /*!
   \fn bool isClockoutActive()
   \brief Check if clockout frequency is enabled.
   \return true if enabled, false otherwise.
   */
   bool isClockoutActive(void);

   /*!
   \fn bool enableAlarm()
   \brief Enable Alarm interrupt.
   \return true if succesful, false otherwise.
   */
   bool enableAlarm(void);

   /*!
   \fn bool disableAlarm()
   \brief Disable alarm interrupt.
   \return true if succesful, false otherwise.
   */
   bool disableAlarm(void);

   /*!
   \fn bool resetAlarm()
   \brief Reset alarm.
   \return true if succesful, false otherwise.
   */
   bool resetAlarm(void);

   /*!
   \fn bool isAlarmActive()
   \brief Check if alarm interrupt is enabled.
   \return true if enabled, false otherwise.
   */
   bool isAlarmActive(void);

   /*!
   \fn bool getAlarm(uint8_t *hours, uint8_t *minutes, uint8_t *day, uint8_t *weekday)
   \brief Read alarm register.
   \param[out] *hours hours alarm.
   \param[out] *minutes minutes alarm.
   \param[out] *day day alarm.
   \param[out] *weekday weekday alarm.
   \return true if succesful, false otherwise.
   */
   bool getAlarm(uint8_t *hours, uint8_t *minutes, uint8_t *day, uint8_t *weekday);

   /*!
   \fn bool setAlarm(uint8_t hours, uint8_t minutes, uint8_t day = PCF8563_ALARM_DISABLE, uint8_t weekday = PCF8563_ALARM_DISABLE)
   \brief Write alarm register.
   \param[in] hours hours alarm.
   \param[in] minutes minutes alarm.
   \param[in] day day alarm.
   \param[in] weekday weekday alarm.
   \return true if succesful, false otherwise.
   */
   bool setAlarm(uint8_t hours, uint8_t minutes, uint8_t day = PCF8563_ALARM_DISABLE, uint8_t weekday = PCF8563_ALARM_DISABLE);

   /*!
   \fn bool enableTimer()
   \brief Enable timer interrupt.
   \return true if succesful, false otherwise.
   */
   bool enableTimer(void);

   /*!
   \fn bool disableTimer()
   \brief Disable timer interrupt.
   \return true if succesful, false otherwise.
   */
   bool disableTimer(void);

   /*!
   \fn bool resetTimer()
   \brief Reset timer.
   \return true if succesful, false otherwise.
   */
   bool resetTimer(void);

   /*!
   \fn bool isTimerActive()
   \brief Check if timer is enabled.
   \return true if enabled, false otherwise.
   */
   bool isTimerActive(void);

   /*!
   \fn bool setTimer(uint8_t frequency, uint8_t timer)
   \brief Write timer register.
   \param[in] frequency frequency for timer.
   \param[in] timer value for timer.
   \return true if succesful, false otherwise.
   */
   bool setTimer(uint8_t frequency, uint8_t timer);

   /*!
   \fn bool getDateTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds, uint8_t *day, uint8_t *month, uint8_t *year, uint8_t *weekday = NULL, uint8_t *century = NULL);
   \brief Read date and time register.
   \param[out] *day hours alarm.
   \param[out] *month minutes alarm.
   \param[out] *year day alarm.
   \param[out] *weekday weekday alarm.
   \param[out] *century weekday alarm.
   \param[out] *hours hours alarm.
   \param[out] *minutes minutes alarm.
   \param[out] *seconds day alarm.
   \return true if succesful, false otherwise.
   */
   bool getDateTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds, uint8_t *day, uint8_t *month, uint8_t *year, uint8_t *weekday = NULL, uint8_t *century = NULL);

   /*!
   \fn bool getDate(uint8_t *day, uint8_t *month, uint8_t *year, uint8_t *weekday = NULL, uint8_t *century = NULL)
   \brief Read date register.
   \param[out] *day hours alarm.
   \param[out] *month minutes alarm.
   \param[out] *year day alarm.
   \param[out] *weekday weekday alarm.
   \param[out] *century weekday alarm.
   \return true if succesful, false otherwise.
   */
   bool getDate(uint8_t *day, uint8_t *month, uint8_t *year, uint8_t *weekday = NULL, uint8_t *century = NULL);

   /*!
   \fn bool setDate(uint8_t day, uint8_t month, uint8_t year, uint8_t weekday, uint8_t century = 0)
   \brief Write date register.
   \param[in] day hours alarm.
   \param[in] month minutes alarm.
   \param[in] year day alarm.
   \param[in] weekday weekday alarm.
   \param[in] century weekday alarm.
   \return true if succesful, false otherwise.
   */
   bool setDate(uint8_t day, uint8_t month, uint8_t year, uint8_t weekday, uint8_t century = 0);

   /*!
   \fn bool getTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
   \brief Read time register.
   \param[out] *hours hours alarm.
   \param[out] *minutes minutes alarm.
   \param[out] *seconds day alarm.
   \return true if succesful, false otherwise.
   */
   bool getTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds);

   /*!
   \fn bool setTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
   \brief Write time register.
   \param[in] hours hours alarm.
   \param[in] minutes minutes alarm.
   \param[in] seconds day alarm.
   \return true if succesful, false otherwise.
   */
   bool setTime(uint8_t hours, uint8_t minutes, uint8_t seconds);

   /*!
   \fn int16_t getDaysFromTwoDate (int16_t d1, int16_t m1, int16_t y1, int16_t d2, int16_t m2, int16_t y2)
   \brief Calculate numbers of days from two date.
   \param[in] d1 day for first date.
   \param[in] m1 month for first date.
   \param[in] y1 year for first date.
   \param[in] d2 day for second date.
   \param[in] m2 month for second date.
   \param[in] y2 year for second date.
   \return numbers of days from first and second date.
   */
   int16_t getDaysFromTwoDate (int16_t d1, int16_t m1, int16_t y1, int16_t d2, int16_t m2, int16_t y2);

   /*!
   \fn uint32_t getTime()
   \brief Read date and time and return they as seconds elapsed from 00:00:00 01/01/1970.
   \return seconds elapsed from 00:00:00 01/01/1970.
   */
   uint32_t getTime();

   /*!
   \fn uint8_t bcdToDec(uint8_t val)
   \brief Convert a value from BCD format into decimal format.
   \param val BCD value.
   \return decimal value.
   */
   uint8_t bcdToDec(uint8_t val);

   /*!
   \fn uint8_t decToBcd(uint8_t val)
   \brief Convert a value from decimal format into BCD format.
   \param val decimal value.
   \return BCD value.
   */
   uint8_t decToBcd(uint8_t val);
};

#endif
