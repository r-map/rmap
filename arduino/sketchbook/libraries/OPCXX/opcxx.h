/**@file Opcxx.h */

/*********************************************************************
Copyright (C) 2019  Marco Baldinetti <m.baldinetti@digiteco.it>
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

#ifndef _OPCXX_H
#define _OPCXX_H

#include <Arduino.h>
#include <debug.h>
#include <SPI.h>
#include <utility.h>
#include "i2c_utility.h"
#include "sensors_config.h"

#define OPCXX_SPI_BUS_FREQUENCY_HZ                (300000ul)

#define OPCXX_SWITCH_ON_DELAY_MS                  (3000)
#define OPCXX_GENERIC_OPERATION_DELAY_MS          (500)
#define OPCXX_FAN_ON_DELAY_MS                     (5000)
#define OPCXX_FAN_OFF_DELAY_MS                    (500)
#define OPCXX_LASER_ON_DELAY_MS                   (1000)
#define OPCXX_LASER_OFF_DELAY_MS                  (500)
#define OPCXX_RETRY_DELAY_MS                      (500)

#define OPCXX_CMD_GENERIC_DELAY_MS                (20)
#define OPCXX_PARA_GENERIC_DELAY_US               (10)

#define OPCXX_FAN_DAC_MAX                         (0xFF)
#define OPCXX_FAN_DAC_MIN                         (0x00)

#define OPCN2_CONFIGURATION_VARIABLES_LENGTH      (257)
#define OPCN2_CONFIGURATION_VARIABLES_2_LENGTH    (10)

#define OPCXX_PM_LENGTH                           (3)

#define OPCN2_BINS_LENGTH                         (16)
#define OPCN3_BINS_LENGTH                         (24)

#define OPCXX_BUFFER_LENGTH                       (260)
#define OPCXX_EXPECTED_RESPONSE_LENGTH            (10)

#define getBinXMToFToUS(binx_mtof)                ((float) (binx_mtof / 3.0))

#define OPCN3_BUSY_COUNT_MAX                      (20)

#define OPCN3_LASER_STATUS_OFF                    (100)

#define OPCXX_READ_INFORMATION_STRING_COMMAND     (0x3F)
#define OPCXX_READ_INFORMATION_STRING_RESULT      (0xF3)
#define OPCXX_READ_INFORMATION_STRING_LENGTH      (60)

#define OPCXX_FAN_ON_COMMAND_1                    (0x03)
#define OPCXX_FAN_ON_COMMAND_2                    (0x04)
#define OPCXX_FAN_ON_RESULT_1                     (0xF3)
#define OPCXX_FAN_ON_RESULT_2                     (0x03)

#define OPCN2_STRING                              ("OPC-N2")
#define OPCN3_STRING                              ("OPC-N3")

#define OPCN2_TYPE                                (2)
#define OPCN3_TYPE                                (3)

#if (USE_SENSOR_OA2 || USE_SENSOR_OB2 || USE_SENSOR_OC2 || USE_SENSOR_OD2)
#define OPCNXX_STRING                             (OPCN2_STRING)
#endif

#if (USE_SENSOR_OA3 || USE_SENSOR_OB3 || USE_SENSOR_OC3 || USE_SENSOR_OD3)
#define OPCNXX_STRING                             (OPCN3_STRING)
#endif

#define isValid(v)                                (((uint16_t) v != UINT16_MAX) && (!isnan(v)))

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

#if (USE_SENSOR_OA2 || USE_SENSOR_OB2 || USE_SENSOR_OC2 || USE_SENSOR_OD2)
/*!
\struct opcn2_histogram_t
\brief Value struct for storing opcxx histogram data.
*/
typedef struct {
  uint16_t bins[OPCN2_BINS_LENGTH];

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
} opcn2_histogram_t;

typedef struct {
  uint8_t is_fan_on;
  uint8_t is_laser_on;
  uint8_t fan_dac;
  uint8_t laser_dac;
} opcn2_status_t;
#endif

#if (USE_SENSOR_OA3 || USE_SENSOR_OB3 || USE_SENSOR_OC3 || USE_SENSOR_OD3)
/*!
\struct opcn3_histogram_t
\brief Value struct for storing opcxx histogram data.
*/
typedef struct {
  uint16_t bins[OPCN3_BINS_LENGTH];

  //! Mass Time-of-Flight average amount of time that particles sized
  //! in the stated bin took to cross the OPS's laser beam.
  //! Each value is in 1/3 us. i.e. a value of 10 would represent 3.33us
  uint8_t bin1_mtof;
  uint8_t bin3_mtof;
  uint8_t bin5_mtof;
  uint8_t bin7_mtof;

  //! histogram's actual sampling period in seconds x 100
  float sampling_period;

  //! Sample Flow Rate in ml/s x 100
  float sample_flow_rate;

  //! Temperature
  float temperature;

  //! Humidity
  float humidity;

  //! air particle counter in ug/m3
  float pm1;
  float pm25;
  float pm10;

  //! Reject count glitch
  uint16_t reject_count_glitch;

  //! Reject count long TOF
  uint16_t reject_count_long_tof;

  //! Reject count ratio
  uint16_t reject_count_ratio;

  //! Reject count out of range
  uint16_t reject_count_out_of_range;

  //! Fan rev count
  uint16_t fan_rev_count;

  //! Laser status
  uint16_t laser_status;

  //! 16 bit CRC with generator polynomial value 0xA001 and initialised to 0xFFFF
  uint16_t checksum;
} opcn3_histogram_t;

typedef struct {
  bool is_fan_on;
  bool is_laser_on;
  uint8_t fan_dac;
  uint8_t laser_dac;
  uint8_t laser_switch;         // TODO: controllare
  bool is_high_gain_on;         // TODO: controllare
  bool is_auto_gain_toggle_on;  // TODO: controllare
} opcn3_status_t;
#endif

/*!
\class Opcxx
\brief Opcxx class.
*/
class Opcxx {
public:

  /*!
  \fn Opcxx(const uint8_t chip_select, const uint8_t power_pin, const uint8_t spi_power_pin, const float sampling_period_s)
  \brief Constructor for Opcxx.
  \param[in] *driver driver's type.
  \param[in] *type sensor's type.
  \return void.
  */
  Opcxx(const uint8_t chip_select, const uint8_t power_pin, const uint8_t spi_power_pin, const float sampling_period_s);

  void init();
  void initPins();
  void switchOn();
  void switchOff();
  void setOn();
  void setOff();
  bool isOn();
  bool isOff();

  opcxx_state_t setFanDacRst();
  opcxx_state_t fanOnOffRst(bool is_on);
  opcxx_state_t laserOnOffRst(bool is_on);
  opcxx_state_t laserSwitchOnOffRst(bool is_on);
  opcxx_state_t fanLaserOnOffRst(bool is_on);
  opcxx_state_t highLowGainRst(bool is_high);

  void setSamplingPeriod(const float sampling_period_s = 0);

  virtual float getBinAtIndex(uint16_t index) = 0;
  virtual float getBinNormalizedAtIndex(uint16_t index) = 0;
  virtual float getPm1() = 0;
  virtual float getPm25() = 0;
  virtual float getPm10() = 0;
  virtual float getSamplingPeriod() = 0;

  virtual uint8_t getOpcType() = 0;
  virtual bool isFanOn() = 0;
  virtual bool isLaserOn() = 0;
  virtual uint8_t getFanDac() = 0;
  virtual uint8_t getLaserDac() = 0;

  virtual void setFanDacCmd(uint8_t fan_dac) = 0;

  virtual void readStatusCmd() = 0;
  virtual opcxx_state_t readStatusRst() = 0;
  virtual void fanOnCmd() = 0;
  virtual void fanOffCmd() = 0;
  virtual void laserOnCmd() = 0;
  virtual void laserOffCmd() = 0;
  virtual void laserSwitchOnCmd() = 0;
  virtual void laserSwitchOffCmd() = 0;
  virtual void fanLaserOnCmd() = 0;
  virtual void fanLaserOffCmd() = 0;
  virtual void HighGainCmd() = 0;
  virtual void readHistogramCmd() = 0;
  virtual void resetHistogram() = 0;
  virtual opcxx_state_t readHistogramRst() = 0;
  virtual void getBins(uint16_t *destination) = 0;

protected:
  bool is_on;
  uint8_t chip_select;
  uint8_t power_pin;
  uint8_t spi_power_pin;
  float sampling_period_s;
  uint16_t length;
  uint8_t response_length;
  uint8_t buffer[OPCXX_BUFFER_LENGTH];
  uint8_t response[OPCXX_BUFFER_LENGTH];
  uint8_t expected_response[OPCXX_EXPECTED_RESPONSE_LENGTH];
  opcxx_command_state_t command_state;

  void beginSPI();
  void writeSPI(const uint8_t cmd, uint8_t *result);
  void endSPI();
  void resetCommand(const uint16_t length, const uint8_t response_length);
  void setCommand(const uint16_t i, const uint8_t cmd, const uint8_t response_length);
  uint8_t *getResponse(const uint16_t i = 0);
  virtual opcxx_state_t sendCommand() = 0;

};

#if (USE_SENSOR_OA2 || USE_SENSOR_OB2 || USE_SENSOR_OC2 || USE_SENSOR_OD2)
class Opcn2 : public Opcxx {
public:
  Opcn2(const uint8_t chip_select, const uint8_t power_pin, const uint8_t spi_power_pin, const float sampling_period_s) : Opcxx(chip_select, power_pin, spi_power_pin, sampling_period_s) {
    memset(&status, 0, sizeof(opcn2_status_t));
    memset(&histogram, 0, sizeof(opcn2_histogram_t));
  };

  uint8_t getOpcType();
  bool isFanOn();
  bool isLaserOn();
  uint8_t getFanDac();
  uint8_t getLaserDac();

  float getBinAtIndex(uint16_t index);
  float getBinNormalizedAtIndex(uint16_t index);
  float getPm1();
  float getPm25();
  float getPm10();
  float getSamplingPeriod();

  void setFanDacCmd(uint8_t fan_dac = 0xFF);

  void readStatusCmd();
  opcxx_state_t readStatusRst();
  void fanOnCmd();
  void fanOffCmd();
  void laserOnCmd();
  void laserOffCmd();
  void laserSwitchOnCmd();
  void laserSwitchOffCmd();
  void fanLaserOnCmd();
  void fanLaserOffCmd();
  void HighGainCmd();
  void readHistogramCmd();
  void resetHistogram();
  opcxx_state_t readHistogramRst();
  void getBins(uint16_t *destination);

protected:
  opcn2_status_t status;
  opcn2_histogram_t histogram;

  opcxx_state_t sendCommand();
};
#endif

#if (USE_SENSOR_OA3 || USE_SENSOR_OB3 || USE_SENSOR_OC3 || USE_SENSOR_OD3)
class Opcn3 : public Opcxx {
public:
  Opcn3(const uint8_t chip_select, const uint8_t power_pin, const uint8_t spi_power_pin, const float sampling_period_s) : Opcxx(chip_select, power_pin, spi_power_pin, sampling_period_s) {
    memset(&status, 0, sizeof(opcn3_status_t));
    memset(&histogram, 0, sizeof(opcn3_histogram_t));
  };

  uint8_t getOpcType();
  bool isFanOn();
  bool isLaserOn();
  uint8_t getFanDac();
  uint8_t getLaserDac();

  float getBinAtIndex(uint16_t index);
  float getBinNormalizedAtIndex(uint16_t index);
  float getPm1();
  float getPm25();
  float getPm10();
  float getTemperature();
  float getHumidity();
  float getSamplingPeriod();

  void setFanDacCmd(uint8_t fan_dac = 0xFF);

  void readStatusCmd();
  opcxx_state_t readStatusRst();
  void fanOnCmd();
  void fanOffCmd();
  void laserOnCmd();
  void laserOffCmd();
  void laserSwitchOnCmd();
  void laserSwitchOffCmd();
  void fanLaserOnCmd();
  void fanLaserOffCmd();
  void HighGainCmd();
  void readHistogramCmd();
  void resetHistogram();
  opcxx_state_t readHistogramRst();
  void getBins(uint16_t *destination);

protected:
  opcn3_status_t status;
  opcn3_histogram_t histogram;

  opcxx_state_t sendCommand();
  float getTemperatureFromRaw(uint16_t temperature_raw);
  float getHumidityFromRaw(uint16_t humidity_raw);
};
#endif

#endif
