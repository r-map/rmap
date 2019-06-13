/**@file Opcxx.cpp */

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

#include <debug_config.h>

/*!
\def SERIAL_TRACE_LEVEL
\brief Serial trace level debug for this library.
*/
#define SERIAL_TRACE_LEVEL OPC_SERIAL_TRACE_LEVEL

#include "opcxx.h"

/*********************************************************************
* Opcxx
*********************************************************************/

Opcxx::Opcxx(const uint8_t chip_select, const uint8_t power_pin, const uint8_t spi_power_pin, const float sampling_period_s) {
  this->chip_select = chip_select;
  this->power_pin = power_pin;
  this->spi_power_pin = spi_power_pin;
  this->sampling_period_s = sampling_period_s;
}

void Opcxx::init () {
  is_on = false;
  command_state = OPCXX_COMMAND_INIT;
}

void Opcxx::initPins () {
  pinMode(chip_select, OUTPUT);
  digitalWrite(chip_select, HIGH);

  pinMode(spi_power_pin, OUTPUT);
  digitalWrite(spi_power_pin, LOW);

  pinMode(power_pin, OUTPUT);
  digitalWrite(power_pin, LOW);
}

void Opcxx::switchOn() {
  digitalWrite(spi_power_pin, HIGH);
  digitalWrite(power_pin, HIGH);
  SERIAL_INFO(F("%s [ %s ]\r\n"), OPCNXX_STRING, ON_STRING);
}

void Opcxx::switchOff() {
  is_on = false;
  digitalWrite(spi_power_pin, LOW);
  digitalWrite(power_pin, LOW);
  SERIAL_INFO(F("%s [ %s ]\r\n"), OPCNXX_STRING, OFF_STRING);
}

void Opcxx::setOn () {
  is_on = true;
}

void Opcxx::setOff () {
  is_on = false;
}

bool Opcxx::isOn() {
  return is_on;
}

bool Opcxx::isOff() {
  return !is_on;
}

void Opcxx::beginSPI() {
  digitalWrite(chip_select, LOW);
  delayMicroseconds(100);
  SPI.beginTransaction(SPISettings(OPCXX_SPI_BUS_FREQUENCY_HZ, MSBFIRST, SPI_MODE1));
}

void Opcxx::writeSPI(const uint8_t cmd, uint8_t *result) {
  *result = SPI.transfer(cmd);
}

void Opcxx::endSPI() {
  SPI.endTransaction();
  delayMicroseconds(100);
  digitalWrite(chip_select, HIGH);
}

void Opcxx::resetCommand(const uint16_t length, const uint8_t response_length) {
  memset(buffer, 0, OPCXX_BUFFER_LENGTH);
  memset(response, 0, OPCXX_BUFFER_LENGTH);
  memset(expected_response, 0, OPCXX_EXPECTED_RESPONSE_LENGTH);
  this->length = length;
  this->response_length = response_length;
}

void Opcxx::setCommand(const uint16_t i, const uint8_t cmd, const uint8_t expected_result) {
  buffer[i] = cmd;
  if (i < OPCXX_EXPECTED_RESPONSE_LENGTH) {
    expected_response[i] = expected_result;
  }
}

uint8_t *Opcxx::getResponse(const uint16_t i) {
  return response + i;
}

opcxx_state_t Opcxx::setFanDacRst() {
  opcxx_state_t opcxx_state = sendCommand();

  if (opcxx_state == OPCXX_OK) {
    SERIAL_INFO(F("%s Set Fan Dac [ %s ]\r\n"), OPCNXX_STRING, OK_STRING);
  }
  else if (opcxx_state == OPCXX_ERROR) {
    SERIAL_ERROR(F("%s Set Fan Dac [ %s ]\r\n"), OPCNXX_STRING, ERROR_STRING);
  }

  return opcxx_state;
}

opcxx_state_t Opcxx::fanOnOffRst(bool is_on) {
  opcxx_state_t opcxx_state = sendCommand();

  if (opcxx_state == OPCXX_OK) {
    SERIAL_INFO(F("%s set Fan [ %s ] [ %s ]\r\n"), OPCNXX_STRING, is_on ? ON_STRING : OFF_STRING, OK_STRING);
  }
  else if (opcxx_state == OPCXX_ERROR) {
    SERIAL_ERROR(F("%s set Fan [ %s ] [ %s ]\r\n"), OPCNXX_STRING, is_on ? ON_STRING : OFF_STRING, ERROR_STRING);
  }

  return opcxx_state;
}

opcxx_state_t Opcxx::laserOnOffRst(bool is_on) {
  opcxx_state_t opcxx_state = sendCommand();

  if (opcxx_state == OPCXX_OK) {
    SERIAL_INFO(F("%s set Laser [ %s ] [ %s ]\r\n"), OPCNXX_STRING, is_on ? ON_STRING : OFF_STRING, OK_STRING);
  }
  else if (opcxx_state == OPCXX_ERROR) {
    SERIAL_ERROR(F("%s set Laser [ %s ] [ %s ]\r\n"), OPCNXX_STRING, is_on ? ON_STRING : OFF_STRING, ERROR_STRING);
  }

  return opcxx_state;
}

opcxx_state_t Opcxx::laserSwitchOnOffRst(bool is_on) {
  opcxx_state_t opcxx_state = sendCommand();

  if (opcxx_state == OPCXX_OK) {
    SERIAL_INFO(F("%s set Laser Switch [ %s ] [ %s ]\r\n"), OPCNXX_STRING, is_on ? ON_STRING : OFF_STRING, OK_STRING);
  }
  else if (opcxx_state == OPCXX_ERROR) {
    SERIAL_ERROR(F("%s set Laser [ %s ] [ %s ]\r\n"), OPCNXX_STRING, is_on ? ON_STRING : OFF_STRING, ERROR_STRING);
  }

  return opcxx_state;
}

opcxx_state_t Opcxx::fanLaserOnOffRst(bool is_on) {
  opcxx_state_t opcxx_state = sendCommand();

  if (opcxx_state == OPCXX_OK) {
    SERIAL_INFO(F("%s set Fan-Laser [ %s ] [ %s ]\r\n"), OPCNXX_STRING, is_on ? ON_STRING : OFF_STRING, OK_STRING);
  }
  else if (opcxx_state == OPCXX_ERROR) {
    SERIAL_ERROR(F("%s set Fan-Laser [ %s ] [ %s ]\r\n"), OPCNXX_STRING, is_on ? ON_STRING : OFF_STRING, ERROR_STRING);
  }

  return opcxx_state;
}

opcxx_state_t Opcxx::highLowGainRst(bool is_high) {
  opcxx_state_t opcxx_state = sendCommand();

  if (opcxx_state == OPCXX_OK) {
    SERIAL_INFO(F("%s set Gain [ %s ] [ %s ]\r\n"), OPCNXX_STRING, is_high ? "HIGH" : "LOW", OK_STRING);
  }
  else if (opcxx_state == OPCXX_ERROR) {
    SERIAL_ERROR(F("%s set Gain [ %s ] [ %s ]\r\n"), OPCNXX_STRING, is_high ? "HIGH" : "LOW", ERROR_STRING);
  }

  return opcxx_state;
}

#if (USE_SENSOR_OA2 || USE_SENSOR_OB2 || USE_SENSOR_OC2 || USE_SENSOR_OD2)
opcxx_state_t Opcn2::sendCommand() {
  static opcxx_command_state_t state_after_wait;
  static uint32_t start_time_ms;
  static bool is_error;
  static opcxx_state_t status;

  switch (command_state) {
    case OPCXX_COMMAND_INIT:
      is_error = false;
      status = OPCXX_BUSY;
      command_state = OPCXX_COMMAND_WRITE_CMD;
      break;

    case OPCXX_COMMAND_WRITE_CMD:
      beginSPI();
      writeSPI(buffer[0], &response[0]);
      endSPI();
      start_time_ms = millis();
      state_after_wait = OPCXX_COMMAND_WRITE_PARAM;
      command_state = OPCXX_COMMAND_WAIT_STATE;
      break;

    case OPCXX_COMMAND_WRITE_PARAM:
      beginSPI();
      for (uint16_t i = 1; i < length; i++) {
        writeSPI(buffer[i], &response[i]);
        delayMicroseconds(OPCXX_PARA_GENERIC_DELAY_US);
      }
      endSPI();
      command_state = OPCXX_COMMAND_END;
      break;

    case OPCXX_COMMAND_END:
      for (uint16_t k = 0; k < response_length; k++) {
        if (expected_response[k] != *getResponse(k)) {
          is_error = true;
          SERIAL_TRACE_CLEAN(F("%s Command [ %s ]\r\n"), OPCNXX_STRING, ERROR_STRING);
          break;
        }
      }

      for (uint8_t k = 0; k < length; k++) {
        SERIAL_TRACE(F("%0x"), *getResponse(k));
      }
      SERIAL_TRACE(F("\r\n"));

      // success
      if (!is_error) {
        status = OPCXX_OK;
        command_state = OPCXX_COMMAND_INIT;
      }
      // fail
      else {
        status = OPCXX_ERROR;
        command_state = OPCXX_COMMAND_INIT;
      }
      break;

    case OPCXX_COMMAND_WAIT_STATE:
      if (millis() - start_time_ms > OPCXX_CMD_GENERIC_DELAY_MS) {
        command_state = state_after_wait;
      }
      break;
  }

  return status;
}

uint8_t Opcn2::getOpcType() {
  return OPCN2_TYPE;
}

bool Opcn2::isFanOn() {
  return status.is_fan_on;
}

bool Opcn2::isLaserOn() {
  return status.is_laser_on;
}

uint8_t Opcn2::getFanDac() {
  return status.fan_dac;
}

uint8_t Opcn2::getLaserDac() {
  return status.laser_dac;
}

uint16_t Opcn2::getBinAtIndex(uint16_t index) {
  return histogram.bins[index];
}

uint16_t Opcn2::getBinNormalizedAtIndex(uint16_t index) {
  float bin = (float) histogram.bins[index];
  bin = (bin / histogram.sample_flow_rate) * 10.0;
  return (uint16_t) (bin);
}

float Opcn2::getPm1() {
  return histogram.pm1;
}

float Opcn2::getPm25() {
  return histogram.pm25;
}

float Opcn2::getPm10() {
  return histogram.pm10;
}

float Opcn2::getSamplingPeriod() {
  return histogram.sampling_period;
}

void Opcn2::readStatusCmd() {
  memset(&status, 0, sizeof(opcn2_status_t));
  Opcxx::resetCommand(5, 1);
  Opcxx::setCommand(0, 0x13, 0xF3);
}

opcxx_state_t Opcn2::readStatusRst() {
  opcxx_state_t opcxx_state = sendCommand();

  if (*Opcxx::getResponse(1) > 1 || *Opcxx::getResponse(2) > 1) {
    opcxx_state = OPCXX_ERROR;
  }

  if (opcxx_state == OPCXX_OK) {
    status.is_fan_on = *Opcxx::getResponse(1);
    status.is_laser_on = *Opcxx::getResponse(2);
    status.fan_dac = *Opcxx::getResponse(3);
    status.laser_dac = *Opcxx::getResponse(4);

    SERIAL_INFO(F("%s Read status [ %s ] FAN DAC [ %u ] Laser DAC [ %u ]\r\n"), OPCNXX_STRING, OK_STRING, status.fan_dac, status.laser_dac);
  }
  else if (opcxx_state == OPCXX_ERROR) {
    SERIAL_ERROR(F("%s Read status [ %s ]\r\n"), OPCNXX_STRING, ERROR_STRING);
    memset(&status, 0, sizeof(opcn2_status_t));
  }

  return opcxx_state;
}

void Opcn2::setFanDacCmd(uint8_t fan_dac) {
  resetCommand(3, 3);
  setCommand(0, 0x42, 0xF3);
  setCommand(1, 0x00, 0x42);
  setCommand(2, fan_dac, 0x00);
}

void Opcn2::fanLaserOnCmd() {
  resetCommand(2, 2);
  setCommand(0, 0x03, 0xF3);
  setCommand(1, 0x00, 0x03);
}

void Opcn2::fanLaserOffCmd() {
  resetCommand(2, 2);
  setCommand(0, 0x03, 0xF3);
  setCommand(1, 0x01, 0x03);
}

void Opcn2::HighGainCmd() {
}

void Opcn2::fanOnCmd() {
}

void Opcn2::fanOffCmd() {
}

void Opcn2::laserOnCmd() {
}

void Opcn2::laserOffCmd() {
}

void Opcn2::laserSwitchOnCmd() {
}

void Opcn2::laserSwitchOffCmd() {
}

void Opcn2::readHistogramCmd() {
  resetHistogram();
  resetCommand(63, 1);
  for (uint16_t i = 0; i < 63; i++) {
    setCommand(i, 0x30, 0xF3);
  }
}

void Opcn2::resetHistogram() {
  memset(&histogram, 0, sizeof(opcn2_histogram_t));
  memset(histogram.bins, UINT8_MAX, sizeof(uint16_t) * OPCN2_BINS_LENGTH);
  histogram.pm1 = UINT16_MAX;
  histogram.pm25 = UINT16_MAX;
  histogram.pm10 = UINT16_MAX;
}

opcxx_state_t Opcn2::readHistogramRst() {
  opcxx_state_t opcxx_state = sendCommand();
  uint32_t checksum = 0;
  float sampling_period_error_percentage = 0;
  bool is_sampling_period_error = false;
  bool is_reading_error = false;

  if (opcxx_state == OPCXX_OK) {
    memset(&histogram, 0, sizeof(opcn2_histogram_t));
    uint8_t index = 1;

    for (uint8_t i = 0; i < OPCN2_BINS_LENGTH; i++) {
      histogram.bins[i] = getUINT16FromUINT8(getResponse(index));
      checksum += histogram.bins[i];
      index += 2;
    }

    histogram.bin1_mtof = *getResponse(index++);
    histogram.bin3_mtof = *getResponse(index++);
    histogram.bin5_mtof = *getResponse(index++);
    histogram.bin7_mtof = *getResponse(index++);

    histogram.sample_flow_rate = getIEE754FloatFrom4UINT8(getResponse(index));
    index += 4;

    histogram.temp_pressure = getUINT32FromUINT8(getResponse(index));
    index += 4;

    histogram.sampling_period = getIEE754FloatFrom4UINT8(getResponse(index));
    index += 4;

    histogram.checksum = getUINT16FromUINT8(getResponse(index));
    index += 2;

    histogram.pm1 = getIEE754FloatFrom4UINT8(getResponse(index));
    index += 4;

    histogram.pm25 = getIEE754FloatFrom4UINT8(getResponse(index));
    index += 4;

    histogram.pm10 = getIEE754FloatFrom4UINT8(getResponse(index));
    index += 4;

    if (sampling_period_s) {
      sampling_period_error_percentage = abs((histogram.sampling_period / sampling_period_s) * 100.0 - 100);
      is_sampling_period_error = (sampling_period_error_percentage >= 10);
    }

    if (histogram.pm1 == 0 && histogram.pm25 == 0 && histogram.pm10 == 0) {
      is_reading_error = true;
    }

    if (histogram.checksum != checksum || is_sampling_period_error || is_reading_error) {
      opcxx_state = OPCXX_ERROR_RESULT;
    }
  }

  if (opcxx_state == OPCXX_OK) {
    SERIAL_INFO(F("%s Read Histogram [ %s ]\r\n"), OPCNXX_STRING, OK_STRING);
  } else if (opcxx_state == OPCXX_ERROR || opcxx_state == OPCXX_ERROR_RESULT) {
    SERIAL_ERROR(F("%s Read Histogram [ %s ]\r\n"), OPCNXX_STRING, ERROR_STRING);
  }

  if (opcxx_state != OPCXX_BUSY) {
    #if (SERIAL_TRACE_LEVEL == SERIAL_TRACE_LEVEL_INFO)
    SERIAL_INFO(F("--> BIN [0-15]:\t[ "));
    SERIAL_INFO_ARRAY_CLEAN(histogram.bins, OPCN2_BINS_LENGTH, UINT16, F("%u  "));
    SERIAL_INFO_CLEAN(F(" ] [ %s ]\r\n"), is_reading_error ? ERROR_STRING : OK_STRING);

    SERIAL_INFO(F("--> PM [1,2.5,10]:\t[ %.1f ug/m3 ] [ %.1f ug/m3 ] [ %.1f ug/m3 ] [ %s ]\r\n"), histogram.pm1, histogram.pm25, histogram.pm10, is_reading_error ? ERROR_STRING : OK_STRING);
    SERIAL_INFO(F("--> Sample Flow Rate\t[ %.3f ml/s ]\r\n"), histogram.sample_flow_rate);
    SERIAL_INFO(F("--> Sampling Period:\t[ %.3f s ] [ %.3f s ] [ %s ]\r\n"), histogram.sampling_period, sampling_period_s, is_sampling_period_error ? ERROR_STRING : OK_STRING);
    #endif

    #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_DEBUG)
    for (uint8_t i = 0; i < OPCN2_BINS_LENGTH; i++) {
      SERIAL_DEBUG(F("--> BIN %02u\t\t[ %u ]\r\n"), i, histogram.bins[i]);
    }

    SERIAL_DEBUG(F("--> BIN 1 MToF\t\t[ %u ] [ %.3f us ]\r\n"), histogram.bin1_mtof, getBinXMToFToUS(histogram.bin1_mtof));
    SERIAL_DEBUG(F("--> BIN 3 MToF\t\t[ %u ] [ %.3f us ]\r\n"), histogram.bin3_mtof, getBinXMToFToUS(histogram.bin3_mtof));
    SERIAL_DEBUG(F("--> BIN 5 MToF\t\t[ %u ] [ %.3f us ]\r\n"), histogram.bin5_mtof, getBinXMToFToUS(histogram.bin5_mtof));
    SERIAL_DEBUG(F("--> BIN 7 MToF\t\t[ %u ] [ %.3f us ]\r\n"), histogram.bin7_mtof, getBinXMToFToUS(histogram.bin7_mtof));
    SERIAL_DEBUG(F("--> PM 1\t\t[ %.3f ug/m3 ]\r\n"), histogram.pm1);
    SERIAL_DEBUG(F("--> PM 2.5\t\t[ %.3f ug/m3 ]\r\n"), histogram.pm25);
    SERIAL_DEBUG(F("--> PM 10\t\t[ %.3f ug/m3 ]\r\n"), histogram.pm10);
    SERIAL_DEBUG(F("--> Sample Flow Rate\t[ %.3f ml/s ]\r\n"), histogram.sample_flow_rate);
    SERIAL_DEBUG(F("--> Sampling Period\t[ %.3f s ] [ %.3f s ] [ %s ]\r\n"), histogram.sampling_period, sampling_period_s, is_sampling_period_error ? ERROR_STRING : OK_STRING);
    // SERIAL_DEBUG(F("--> Temperature\t[ %.1f C ]\r\n"), histogram.temp_pressure);
    SERIAL_DEBUG(F("--> Checksum\t\t[ %u ] [ %s ]\r\n"), histogram.checksum, histogram.checksum == checksum ? OK_STRING : ERROR_STRING);
    #endif
  }

  if (opcxx_state == OPCXX_ERROR || opcxx_state == OPCXX_ERROR_RESULT) {
    resetHistogram();
  }

  return opcxx_state;
}

void Opcn2::getBins(uint16_t *destination) {
  memcpy(destination, histogram.bins, sizeof(uint16_t) * OPCN2_BINS_LENGTH);
}
#endif

#if (USE_SENSOR_OA3 || USE_SENSOR_OB3 || USE_SENSOR_OC3 || USE_SENSOR_OD3)
opcxx_state_t Opcn3::sendCommand() {
  static opcxx_command_state_t state_after_wait;
  static uint32_t start_time_ms;
  static bool is_error;
  static opcxx_state_t status;
  static uint8_t busy_count;
  static uint16_t i;

  switch (command_state) {
    case OPCXX_COMMAND_INIT:
      i = 0;
      busy_count = 0;
      is_error = false;
      status = OPCXX_BUSY;
      command_state = OPCXX_COMMAND_WRITE_CMD;
      break;

    case OPCXX_COMMAND_WRITE_CMD:
      beginSPI();
      writeSPI(buffer[i], &response[i]);
      endSPI();

      // SERIAL_INFO(F("%i --->> 0x%X ---> 0x%X\r\n"), i, buffer[i], response[i]);

      start_time_ms = millis();
      command_state = OPCXX_COMMAND_WAIT_STATE;

      //! success
      if (response[i] == expected_response[i]) {
        i++;
        if (i == 2) {
          state_after_wait = OPCXX_COMMAND_WRITE_PARAM;
        }
        else {
          state_after_wait = OPCXX_COMMAND_WRITE_CMD;
        }
      }
      //! retry
      else if ((++busy_count) < OPCN3_BUSY_COUNT_MAX) {
        state_after_wait = OPCXX_COMMAND_WRITE_CMD;
      }
      //! fail
      else {
        is_error = true;
        state_after_wait = OPCXX_COMMAND_END;
      }
      break;

    case OPCXX_COMMAND_WRITE_PARAM:
      beginSPI();
      for (i = 2; i < length; i++) {
        writeSPI(buffer[i], &response[i]);
        delayMicroseconds(OPCXX_PARA_GENERIC_DELAY_US);
        // SERIAL_INFO(F("%i --->> 0x%X ---> 0x%X\r\n"), i - 2, buffer[i], response[i]);
      }
      endSPI();
      command_state = OPCXX_COMMAND_END;
      break;

    case OPCXX_COMMAND_END:
      for (uint16_t k = 0; k < response_length; k++) {
        if (expected_response[k] != *getResponse(k)) {
          is_error = true;
          SERIAL_TRACE_CLEAN(F("%s Command [ %s ]\r\n"), OPCNXX_STRING, ERROR_STRING);
          break;
        }
      }

      SERIAL_TRACE_CLEAN(F(""));
      for (uint16_t k = 0; k < length; k++) {
        SERIAL_TRACE_CLEAN(F("%0x"), *getResponse(k));
      }
      SERIAL_TRACE_CLEAN(F("\r\n"));

      // success
      if (!is_error) {
        status = OPCXX_OK;
        command_state = OPCXX_COMMAND_INIT;
      }
      // fail
      else {
        status = OPCXX_ERROR;
        command_state = OPCXX_COMMAND_INIT;
      }
      break;

    case OPCXX_COMMAND_WAIT_STATE:
      if (millis() - start_time_ms > OPCXX_CMD_GENERIC_DELAY_MS) {
        command_state = state_after_wait;
      }
      break;
  }

  return status;
}

uint8_t Opcn3::getOpcType() {
  return OPCN3_TYPE;
}

bool Opcn3::isFanOn() {
  return status.is_fan_on;
}

bool Opcn3::isLaserOn() {
  return status.is_laser_on;
}

uint8_t Opcn3::getFanDac() {
  return status.fan_dac;
}

uint8_t Opcn3::getLaserDac() {
  return status.laser_dac;
}

uint16_t Opcn3::getBinAtIndex(uint16_t index) {
  return histogram.bins[index];
}

uint16_t Opcn3::getBinNormalizedAtIndex(uint16_t index) {
  float bin = (float) histogram.bins[index];
  bin = (bin / histogram.sample_flow_rate) * 10.0;
  return (uint16_t) (bin);
}

float Opcn3::getPm1() {
  return histogram.pm1;
}

float Opcn3::getPm25() {
  return histogram.pm25;
}

float Opcn3::getPm10() {
  return histogram.pm10;
}

float Opcn3::getTemperature() {
  return histogram.temperature;
}

float Opcn3::getHumidity() {
  return histogram.humidity;
}

float Opcn3::getSamplingPeriod() {
  return histogram.sampling_period;
}

void Opcn3::readStatusCmd() {
  memset(&status, 0, sizeof(opcn3_status_t));
  Opcxx::resetCommand(7, 2);
  Opcxx::setCommand(0, 0x13, 0x31);
  Opcxx::setCommand(1, 0x13, 0xF3);
}

opcxx_state_t Opcn3::readStatusRst() {
  opcxx_state_t opcxx_state = sendCommand();

  if (*Opcxx::getResponse(2) > 1 || *Opcxx::getResponse(3) > 1) {
    opcxx_state = OPCXX_ERROR;
  }

  if (opcxx_state == OPCXX_OK) {
    status.is_fan_on = *Opcxx::getResponse(2);
    status.is_laser_on = *Opcxx::getResponse(3);
    status.fan_dac = *Opcxx::getResponse(4);
    status.laser_dac = *Opcxx::getResponse(5);
    status.laser_switch = *Opcxx::getResponse(6);
    status.is_high_gain_on = (*Opcxx::getResponse(7)) & 0b00000001;
    status.is_auto_gain_toggle_on = (*Opcxx::getResponse(7)) & 0b00000010;

    SERIAL_INFO(F("%s Read status [ %s ]\r\n"), OPCNXX_STRING, OK_STRING);
    SERIAL_INFO(F("--> Fan ON [ %s ] Fan DAC [ %u ]\r\n"), status.is_fan_on ? YES_STRING : NO_STRING, status.fan_dac);
    SERIAL_INFO(F("--> Laser ON [ %s ] Laser DAC [ %u ]\r\n"), status.is_laser_on ? YES_STRING : NO_STRING, status.laser_dac);
    SERIAL_INFO(F("--> Laser switch [ %u ] Gain [ %s ] Auto Gain [ %s ]\r\n"), status.laser_switch, status.is_high_gain_on ? "HIGH" : "LOW", status.is_auto_gain_toggle_on ? ON_STRING : OFF_STRING);
  }
  else if (opcxx_state == OPCXX_ERROR) {
    SERIAL_ERROR(F("%s Read status [ %s ]\r\n"), OPCNXX_STRING, ERROR_STRING);
    memset(&status, 0, sizeof(opcn3_status_t));
  }

  return opcxx_state;
}

void Opcn3::setFanDacCmd(uint8_t fan_dac) {
  resetCommand(4, 4);
  setCommand(0, 0x42, 0x31);
  setCommand(1, 0x42, 0xF3);
  setCommand(2, 0x00, 0x42);
  setCommand(3, fan_dac, 0x00);
}

void Opcn3::fanLaserOnCmd() {
  // Non implementabile
}

void Opcn3::fanLaserOffCmd() {
  // Non implementabile
}

void Opcn3::fanOnCmd() {
  resetCommand(3, 3);
  setCommand(0, 0x03, 0x31);
  setCommand(1, 0x03, 0xF3);
  setCommand(2, 0x03, 0x03);
}

void Opcn3::fanOffCmd() {
}

void Opcn3::laserOnCmd() {
  resetCommand(3, 3);
  setCommand(0, 0x03, 0x31);
  setCommand(1, 0x03, 0xF3);
  setCommand(2, 0x07, 0x03);
}

void Opcn3::laserOffCmd() {
}

void Opcn3::laserSwitchOnCmd() {
  resetCommand(3, 3);
  setCommand(0, 0x03, 0x31);
  setCommand(1, 0x03, 0xF3);
  setCommand(2, 0x05, 0x03);
}

void Opcn3::laserSwitchOffCmd() {
}

void Opcn3::HighGainCmd() {
  resetCommand(3, 3);
  setCommand(0, 0x03, 0x31);
  setCommand(1, 0x03, 0xF3);
  setCommand(2, 0x09, 0x03);
}

float Opcn3::getTemperatureFromRaw(uint16_t temperature_raw) {
  return (float) (-45.0 + (175.0 * temperature_raw / 65535.0));
}

float Opcn3::getHumidityFromRaw(uint16_t humidity_raw) {
  return (float) (100.0 * humidity_raw / 65535.0);
}

void Opcn3::readHistogramCmd() {
  resetHistogram();
  resetCommand(88, 2);
  setCommand(0, 0x30, 0x31);
  setCommand(1, 0x30, 0xF3);

  for (uint16_t i = 2; i < 88; i++) {
    setCommand(i, 0x30, 0xF3);
  }
}

void Opcn3::resetHistogram() {
  memset(&histogram, 0, sizeof(opcn3_histogram_t));
  memset(histogram.bins, UINT8_MAX, sizeof(uint16_t) * OPCN3_BINS_LENGTH);
  histogram.pm1 = UINT16_MAX;
  histogram.pm25 = UINT16_MAX;
  histogram.pm10 = UINT16_MAX;
  histogram.temperature = UINT16_MAX;
  histogram.humidity = UINT16_MAX;
}

opcxx_state_t Opcn3::readHistogramRst() {
  opcxx_state_t opcxx_state = sendCommand();
  uint16_t checksum = 0;
  float sampling_period_error_percentage = 0;
  bool is_sampling_period_error = false;
  bool is_reading_error = false;

  if (opcxx_state == OPCXX_OK) {
    memset(&histogram, 0, sizeof(opcn3_histogram_t));
    uint8_t index = 2;

    for (uint8_t i = 0; i < OPCN3_BINS_LENGTH; i++) {
      histogram.bins[i] = getUINT16FromUINT8(getResponse(index));
      index += 2;
    }

    histogram.bin1_mtof = *getResponse(index++);
    histogram.bin3_mtof = *getResponse(index++);
    histogram.bin5_mtof = *getResponse(index++);
    histogram.bin7_mtof = *getResponse(index++);

    histogram.sampling_period = getUINT16FromUINT8(getResponse(index));
    index += 2;

    histogram.sample_flow_rate = getUINT16FromUINT8(getResponse(index));
    index += 2;

    histogram.temperature = getUINT16FromUINT8(getResponse(index));
    index += 2;

    histogram.humidity = getUINT16FromUINT8(getResponse(index));
    index += 2;

    histogram.pm1 = getIEE754FloatFrom4UINT8(getResponse(index));
    index += 4;

    histogram.pm25 = getIEE754FloatFrom4UINT8(getResponse(index));
    index += 4;

    histogram.pm10 = getIEE754FloatFrom4UINT8(getResponse(index));
    index += 4;

    histogram.reject_count_glitch = getUINT16FromUINT8(getResponse(index));
    index += 2;

    histogram.reject_count_long_tof = getUINT16FromUINT8(getResponse(index));
    index += 2;

    histogram.reject_count_ratio = getUINT16FromUINT8(getResponse(index));
    index += 2;

    histogram.reject_count_out_of_range = getUINT16FromUINT8(getResponse(index));
    index += 2;

    histogram.fan_rev_count = getUINT16FromUINT8(getResponse(index));
    index += 2;

    histogram.laser_status = getUINT16FromUINT8(getResponse(index));
    index += 2;

    histogram.checksum = getUINT16FromUINT8(getResponse(index));
    index += 2;

    histogram.sampling_period /= 100.0;
    histogram.sampling_period *= 2.0;
    histogram.sample_flow_rate /= 100.0;

    histogram.temperature = getTemperatureFromRaw(histogram.temperature);
    histogram.humidity = getHumidityFromRaw(histogram.humidity);

    checksum = crc16(getResponse(2), 84);

    if (sampling_period_s) {
      sampling_period_error_percentage = abs((histogram.sampling_period / sampling_period_s) * 100.0 - 100);
      is_sampling_period_error = (sampling_period_error_percentage >= 10);
      is_reading_error = true;
    }

    if (histogram.checksum != checksum || is_sampling_period_error || is_reading_error) {
      opcxx_state = OPCXX_ERROR_RESULT;
    }
  }

  if (opcxx_state == OPCXX_OK) {
    SERIAL_INFO(F("%s Read Histogram [ %s ]\r\n"), OPCNXX_STRING, OK_STRING);
  } else if (opcxx_state == OPCXX_ERROR || opcxx_state == OPCXX_ERROR_RESULT) {
    SERIAL_ERROR(F("%s Read Histogram [ %s ]\r\n"), OPCNXX_STRING, ERROR_STRING);
  }

  if (opcxx_state != OPCXX_BUSY) {
    #if (SERIAL_TRACE_LEVEL == SERIAL_TRACE_LEVEL_INFO)
    SERIAL_INFO(F("--> BIN [0-23]\t\t[ "));
    SERIAL_INFO_ARRAY_CLEAN(histogram.bins, OPCN3_BINS_LENGTH, UINT16, F("%u  "));
    SERIAL_INFO_CLEAN(F(" ] [ %s ]\r\n"), is_reading_error ? ERROR_STRING : OK_STRING);

    SERIAL_INFO(F("--> PM [1,2.5,10]\t[ %.1f ug/m3 ] [ %.1f ug/m3 ] [ %.1f ug/m3 ] [ %s ]\r\n"), histogram.pm1, histogram.pm25, histogram.pm10, is_reading_error ? ERROR_STRING : OK_STRING);
    SERIAL_INFO(F("--> Temperature\t[ %.1f C ]\r\n"), histogram.temperature);
    SERIAL_INFO(F("--> Humidity\t\t[ %.0f %% ]\r\n"), histogram.humidity);
    SERIAL_INFO(F("--> Sample Flow Rate\t[ %.3f ml/s ]\r\n"), histogram.sample_flow_rate);
    SERIAL_INFO(F("--> Sampling Period\t[ %.3f s ] [ %.3f s ] [ %s ]\r\n"), histogram.sampling_period, sampling_period_s, is_sampling_period_error ? ERROR_STRING : OK_STRING);
    SERIAL_INFO(F("--> Checksum\t\t[ 0x%X 0x%X ] [ %s ]\r\n"), histogram.checksum, checksum, histogram.checksum == checksum ? OK_STRING : ERROR_STRING);
    #endif

    #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_DEBUG)
    for (uint8_t i = 0; i < OPCN3_BINS_LENGTH; i++) {
      SERIAL_DEBUG(F("--> BIN %02u\t\t\t[ %u ]\r\n"), i, histogram.bins[i]);
    }

    SERIAL_DEBUG(F("--> BIN 1 MToF\t\t\t[ %u ] [ %.3f us ]\r\n"), histogram.bin1_mtof, getBinXMToFToUS(histogram.bin1_mtof));
    SERIAL_DEBUG(F("--> BIN 3 MToF\t\t\t[ %u ] [ %.3f us ]\r\n"), histogram.bin3_mtof, getBinXMToFToUS(histogram.bin3_mtof));
    SERIAL_DEBUG(F("--> BIN 5 MToF\t\t\t[ %u ] [ %.3f us ]\r\n"), histogram.bin5_mtof, getBinXMToFToUS(histogram.bin5_mtof));
    SERIAL_DEBUG(F("--> BIN 7 MToF\t\t\t[ %u ] [ %.3f us ]\r\n"), histogram.bin7_mtof, getBinXMToFToUS(histogram.bin7_mtof));
    SERIAL_DEBUG(F("--> PM 1\t\t\t[ %.3f ug/m3 ]\r\n"), histogram.pm1);
    SERIAL_DEBUG(F("--> PM 2.5\t\t\t[ %.3f ug/m3 ]\r\n"), histogram.pm25);
    SERIAL_DEBUG(F("--> PM 10\t\t\t[ %.3f ug/m3 ]\r\n"), histogram.pm10);
    SERIAL_DEBUG(F("--> Temperature\t\t[ %.1f C ]\r\n"), histogram.temperature);
    SERIAL_DEBUG(F("--> Humidity\t\t\t[ %.0f %% ]\r\n"), histogram.humidity);
    SERIAL_DEBUG(F("--> Sample Flow Rate\t\t[ %.3f ml/s ]\r\n"), histogram.sample_flow_rate);
    SERIAL_DEBUG(F("--> Sampling Period\t\t[ %.3f s ] [ %.3f s ] [ %s ]\r\n"), histogram.sampling_period, sampling_period_s, is_sampling_period_error ? ERROR_STRING : OK_STRING);
    SERIAL_DEBUG(F("--> Reject count Glitch\t[ %u ]\r\n"), histogram.reject_count_glitch);
    SERIAL_DEBUG(F("--> Reject count LongTOF\t[ %u ]\r\n"), histogram.reject_count_long_tof);
    SERIAL_DEBUG(F("--> Reject count Ratio\t\t[ %u ]\r\n"), histogram.reject_count_ratio);
    SERIAL_DEBUG(F("--> Reject count Out of Range\t[ %u ]\r\n"), histogram.reject_count_out_of_range);
    SERIAL_DEBUG(F("--> Fan rev count\t\t[ %u ]\r\n"), histogram.fan_rev_count);
    SERIAL_DEBUG(F("--> Laser Status\t\t[ %u ]\r\n"), histogram.laser_status);
    SERIAL_DEBUG(F("--> Checksum\t\t[ 0x%X 0x%X ] [ %s ]\r\n"), histogram.checksum, checksum, histogram.checksum == checksum ? OK_STRING : ERROR_STRING);
    #endif
  }

  if (opcxx_state == OPCXX_ERROR || opcxx_state == OPCXX_ERROR_RESULT) {
    resetHistogram();
  }

  return opcxx_state;
}

void Opcn3::getBins(uint16_t *destination) {
  memcpy(destination, histogram.bins, sizeof(uint16_t) * OPCN3_BINS_LENGTH);
}
#endif
