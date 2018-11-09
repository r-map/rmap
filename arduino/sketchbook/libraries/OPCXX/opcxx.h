/**@file opcxx.h */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>
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

#ifndef _OPCXX_H
#define _OPCXX_H

#include <Arduino.h>
#include <debug.h>
#include <SPI.h>
#include <utility.h>

#define OPCXX_SPI_BUS_FREQUENCY_HZ                (500000ul)

#define OPCXX_SWITCH_ON_DELAY_MS                  (1500)
#define OPCXX_GENERIC_OPERATION_DELAY_MS          (500)
#define OPCXX_FAN_POWER_DELAY_MS                  (500)
#define OPCXX_FAN_LASER_DELAY_MS                  (1000)
#define OPCXX_RETRY_DELAY_MS                      (500)

#define OPCXX_CMD_GENERIC_DELAY_MS                (10)
#define OPCXX_PARA_GENERIC_DELAY_US               (10)

#define OPCXX_FAN_DAC_MAX                         (0xFF)
#define OPCXX_FAN_DAC_MIN                         (0x00)

#define OPCXX_BINS_LENGTH                         (16)

#define OPCXX_BUFFER_LENGTH                       (257)
#define OPCXX_RESPONSE_LENGTH                     (5)

#define getBinXMToFToUS(binx_mtof)                ((float) (binx_mtof / 3.0))

#define OPCXX_READ_INFORMATION_STRING_COMMAND     (0x3F)
#define OPCXX_READ_INFORMATION_STRING_RESULT      (0xF3)
#define OPCXX_READ_INFORMATION_STRING_LENGTH      (60)

#define OPCXX_FAN_ON_COMMAND_1                    (0x03)
#define OPCXX_FAN_ON_COMMAND_2                    (0x04)
#define OPCXX_FAN_ON_RESULT_1                     (0xF3)
#define OPCXX_FAN_ON_RESULT_2                     (0x03)

/*!
\enum opcxx_command_state_t
\brief Main loop finite state machine.
*/
typedef enum {
  OPCXX_COMMAND_INIT,           //!< init task variables
  OPCXX_COMMAND_WRITE_CMD,      //!<
  OPCXX_COMMAND_WRITE_PARAM,    //!<
  OPCXX_COMMAND_END,            //!< performs end operations and deactivate task
  OPCXX_COMMAND_WAIT_STATE      //!< non-blocking waiting time
} opcxx_command_state_t;

/*!
\enum opcxx_state_t
\brief Main loop finite state machine.
*/
typedef enum {
  OPCXX_BUSY,         //!< busy: opcxx is doing something
  OPCXX_OK,           //!< operation complete with success
  OPCXX_ERROR,        //!< operation abort due to error
  OPCXX_ERROR_RESULT  //!< operation abort due to logic result error
} opcxx_state_t;

typedef struct {
  bool is_fan_on;
  bool is_laser_on;
  uint8_t fan_dac;
  uint8_t laser_dac;
} opcxx_status_t;

/*!
\struct opcxx_histogram_t
\brief Value struct for storing opcxx histogram data.
*/
typedef struct {
  uint16_t bins[OPCXX_BINS_LENGTH];

  //! Mass Time-of-Flight average amount of time that particles sized
  //! in the stated bin took to cross the OPS's laser beam.
  //! Each value is in 1/3 us. i.e. a value of 10 would represent 3.33us
  uint8_t bin1_mtof;
  uint8_t bin3_mtof;
  uint8_t bin5_mtof;
  uint8_t bin7_mtof;

  //! Sample Flow Rate in ml/s
  float sample_flow_rate;

  //! Temperature in °C multiplied by 10 or Pressure in pascals
  uint32_t temp_pressure;

  //! histogram's actual sampling period in seconds
  float sampling_period;

  //! the least significant 16bits of the sum of the counts in all the histogram bins
  uint16_t checksum;

  //! air particle counter in ug/m3
  float pm1;
  float pm25;
  float pm10;
} opcxx_histogram_t;

class OPCXX {
protected:
  bool is_on;
  uint8_t chip_select;
  uint8_t power_pin;
  float sampling_period_s;
  uint16_t length;
  uint8_t response_length;
  uint8_t buffer[OPCXX_BUFFER_LENGTH];
  uint8_t response[OPCXX_RESPONSE_LENGTH];
  opcxx_status_t status;
  opcxx_histogram_t histogram;

  /*!
  \var opcxx_command_state_t
  \brief sim800 connection sequence state.
  */
  opcxx_command_state_t command_state;

  void beginSPI();
  void writeSPI(const uint8_t cmd, uint8_t *result);
  void endSPI();
  void resetCommand(const uint16_t length, const uint8_t response_length);
  void setCommand(const uint16_t i, const uint8_t cmd, const uint8_t response_length);
  uint8_t *getResponse(const uint16_t i = 0);
  opcxx_state_t sendCommand();

public:
  OPCXX (const uint8_t chip_select, const uint8_t power_pin, const float sampling_period_s = 0);

  void setSamplingPeriod(const float sampling_period_s = 0);

  void init();
  void switchOn();
  void switchOff();

  void readStatusCmd();
  opcxx_state_t readStatusRst();
  bool isOn();
  bool isOff();
  bool isFanOn();
  bool isLaserOn();
  uint8_t getFanDac();
  uint8_t getLaserDac();

  void setFanPowerCmd(uint8_t fan_dac);
  opcxx_state_t setFanPowerRst();

  void switchFanLaserOnCmd();
  opcxx_state_t switchFanLaserOnRst();

  void switchFanLaserOffCmd();
  opcxx_state_t switchFanLaserOffRst();

  void readHistogramCmd();
  opcxx_state_t readHistogramRst();
  void getBins(uint16_t *destination);
  uint16_t getBinAtIndex(uint16_t index);
  float getPm1();
  float getPm25();
  float getPm10();
};

#endif
