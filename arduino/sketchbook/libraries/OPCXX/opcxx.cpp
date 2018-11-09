/**@file opcxx.cpp */

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

#include <debug_config.h>

/*!
\def SERIAL_TRACE_LEVEL
\brief Serial trace level debug for this library.
*/
#define SERIAL_TRACE_LEVEL OPC_SERIAL_TRACE_LEVEL

#include "opcxx.h"

OPCXX::OPCXX (const uint8_t chip_select, const uint8_t power_pin, const float sampling_period_s) {
  this->chip_select = chip_select;
  this->power_pin = power_pin;
  this->sampling_period_s = sampling_period_s;
  memset(&status, 0, sizeof(opcxx_status_t));
  memset(&histogram, 0, sizeof(opcxx_histogram_t));
}

void OPCXX::setSamplingPeriod(const float sampling_period_s) {
  this->sampling_period_s = sampling_period_s;
}

void OPCXX::init () {
  is_on = false;
  command_state = OPCXX_COMMAND_INIT;
  pinMode(chip_select, OUTPUT);
  pinMode(power_pin, OUTPUT);
  digitalWrite(chip_select, HIGH);
  digitalWrite(power_pin, LOW);
}

void OPCXX::switchOn() {
  digitalWrite(power_pin, HIGH);
  is_on = true;
  SERIAL_INFO(F("OPCXX [ %s ]\r\n"), ON_STRING);
}

void OPCXX::switchOff() {
  digitalWrite(power_pin, LOW);
  is_on = false;
  SERIAL_INFO(F("OPCXX [ %s ]\r\n"), OFF_STRING);
}

void OPCXX::beginSPI() {
  digitalWrite(chip_select, LOW);
  delayMicroseconds(100);
  SPI.beginTransaction(SPISettings(OPCXX_SPI_BUS_FREQUENCY_HZ, MSBFIRST, SPI_MODE1));
}

void OPCXX::writeSPI(const uint8_t cmd, uint8_t *result) {
  *result = SPI.transfer(cmd);
}

void OPCXX::endSPI() {
  SPI.endTransaction();
  delayMicroseconds(100);
  digitalWrite(chip_select, HIGH);
}

void OPCXX::resetCommand(const uint16_t length, const uint8_t response_length) {
  memset(buffer, 0, OPCXX_BUFFER_LENGTH);
  memset(response, 0, OPCXX_RESPONSE_LENGTH);
  this->length = length;
  this->response_length = response_length;
}

void OPCXX::setCommand(const uint16_t i, const uint8_t cmd, const uint8_t expected_result) {
  buffer[i] = cmd;
  response[i] = expected_result;
}

uint8_t *OPCXX::getResponse(const uint16_t i) {
  return buffer + i;
}

opcxx_state_t OPCXX::sendCommand() {
  static opcxx_command_state_t state_after_wait;
  static uint32_t start_time_ms;
  static bool is_error;
  static opcxx_state_t status;
  static uint8_t i;

  switch (command_state) {
    case OPCXX_COMMAND_INIT:
      i = 0;
      is_error = false;
      status = OPCXX_BUSY;
      command_state = OPCXX_COMMAND_WRITE_CMD;
      break;

    case OPCXX_COMMAND_WRITE_CMD:
      beginSPI();
      writeSPI(buffer[i], &buffer[i]);
      endSPI();
      start_time_ms = millis();
      state_after_wait = OPCXX_COMMAND_WRITE_PARAM;
      command_state = OPCXX_COMMAND_WAIT_STATE;
      break;

    case OPCXX_COMMAND_WRITE_PARAM:
      beginSPI();
      for (i = 1; i < length; i++) {
        writeSPI(buffer[i], &buffer[i]);
        delayMicroseconds(OPCXX_PARA_GENERIC_DELAY_US);
      }
      endSPI();
      command_state = OPCXX_COMMAND_END;
      break;

    case OPCXX_COMMAND_END:
      for (uint8_t k = 0; k < response_length; k++) {
        if (response[k] != *getResponse(k)) {
          is_error = true;
          SERIAL_TRACE(F("OPCXX Command [ %s ]\r\n"), ERROR_STRING);
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

void OPCXX::readStatusCmd() {
  memset(&status, 0, sizeof(opcxx_status_t));
  resetCommand(5, 1);
  setCommand(0, 0x13, 0xF3);
}

opcxx_state_t OPCXX::readStatusRst() {
  opcxx_state_t opcxx_state = sendCommand();

  if (opcxx_state == OPCXX_OK) {
    status.is_fan_on = *getResponse(1);
    status.is_laser_on = *getResponse(2);
    status.fan_dac = *getResponse(3);
    status.laser_dac = *getResponse(4);

    SERIAL_INFO(F("OPCXX Read status [ %s ] FAN DAC [ %u ] Laser DAC [ %u ]\r\n"), OK_STRING, status.fan_dac, status.laser_dac);
  }
  else if (opcxx_state == OPCXX_ERROR) {
    SERIAL_ERROR(F("OPCXX Read status [ %s ]\r\n"), ERROR_STRING);
    memset(&status, 0, sizeof(opcxx_status_t));
  }

  return opcxx_state;
}

bool OPCXX::isOn() {
  return is_on;
}

bool OPCXX::isOff() {
  return !is_on;
}

bool OPCXX::isFanOn() {
  return status.is_fan_on;
}

bool OPCXX::isLaserOn() {
  return status.is_laser_on;
}

uint8_t OPCXX::getFanDac() {
  return status.fan_dac;
}

uint8_t OPCXX::getLaserDac() {
  return status.laser_dac;
}

void OPCXX::setFanPowerCmd(uint8_t fan_dac) {
  resetCommand(3, 3);
  setCommand(0, 0x42, 0xF3);
  setCommand(1, 0x00, 0x42);
  setCommand(2, fan_dac, 0x00);
}

opcxx_state_t OPCXX::setFanPowerRst() {
  opcxx_state_t opcxx_state = sendCommand();

  if (opcxx_state == OPCXX_OK) {
    SERIAL_INFO(F("OPCXX Set Fan Power [ %s ]\r\n"), OK_STRING);
  }
  else if (opcxx_state == OPCXX_ERROR) {
    SERIAL_ERROR(F("OPCXX Set Fan Power [ %s ]\r\n"), ERROR_STRING);
  }

  return opcxx_state;
}

void OPCXX::switchFanLaserOnCmd() {
  resetCommand(2, 2);
  setCommand(0, 0x03, 0xF3);
  setCommand(1, 0x00, 0x03);
}

opcxx_state_t OPCXX::switchFanLaserOnRst() {
  opcxx_state_t opcxx_state = sendCommand();

  if (opcxx_state == OPCXX_OK) {
    SERIAL_INFO(F("OPCXX switch Fan-Laser [ %s ] [ %s ]\r\n"), ON_STRING, OK_STRING);
  }
  else if (opcxx_state == OPCXX_ERROR) {
    SERIAL_ERROR(F("OPCXX switch Fan-Laser [ %s ] [ %s ]\r\n"), ON_STRING, ERROR_STRING);
  }

  return opcxx_state;
}

void OPCXX::switchFanLaserOffCmd() {
  resetCommand(2, 2);
  setCommand(0, 0x03, 0xF3);
  setCommand(1, 0x01, 0x03);
}

opcxx_state_t OPCXX::switchFanLaserOffRst() {
  opcxx_state_t opcxx_state = sendCommand();

  if (opcxx_state == OPCXX_OK) {
    SERIAL_INFO(F("OPCXX switch Fan-Laser [ %s ] [ %s ]\r\n"), OFF_STRING, OK_STRING);
  }
  else if (opcxx_state == OPCXX_ERROR) {
    SERIAL_ERROR(F("OPCXX switch Fan-Laser [ %s ] [ %s ]\r\n"), OFF_STRING, ERROR_STRING);
  }

  return opcxx_state;
}

void OPCXX::readHistogramCmd() {
  memset(&histogram, 0, sizeof(opcxx_histogram_t));
  resetCommand(63, 1);
  setCommand(0, 0x30, 0xF3);
  for (uint8_t i = 0; i < 63; i++) {
    setCommand(i, 0x30, 0xF3);
  }
}

opcxx_state_t OPCXX::readHistogramRst() {
  opcxx_state_t opcxx_state = sendCommand();
  uint16_t checksum = 0;
  float sampling_period_error_percentage = 0;
  bool is_sampling_period_error = false;

  if (opcxx_state == OPCXX_OK) {
    memset(&histogram, 0, sizeof(opcxx_histogram_t));
    uint8_t index = 1;

    for (uint8_t i = 0; i < OPCXX_BINS_LENGTH; i++) {
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

    if (histogram.checksum != checksum || is_sampling_period_error) {
      opcxx_state = OPCXX_ERROR_RESULT;
    }
  }

  if (opcxx_state == OPCXX_OK) {
    SERIAL_INFO(F("OPCXX Read Histogram [ %s ]\r\n"), OK_STRING);
  } else if (opcxx_state == OPCXX_ERROR || opcxx_state == OPCXX_ERROR_RESULT) {
    SERIAL_ERROR(F("OPCXX Read Histogram [ %s ]\r\n"), ERROR_STRING);
  }

  if (opcxx_state != OPCXX_BUSY) {
    #if (SERIAL_TRACE_LEVEL == SERIAL_TRACE_LEVEL_INFO)
    SERIAL_INFO(F("--> BIN [0-15]:\t[ "));
    SERIAL_INFO_ARRAY_CLEAN(histogram.bins, OPCXX_BINS_LENGTH, UINT16, F("%u  "));
    SERIAL_INFO_CLEAN(F(" ]\r\n"));

    SERIAL_INFO(F("--> PM [1,2.5,10]:\t[ %.3f ug/m3 ] [ %.3f ug/m3 ] [ %.3f ug/m3 ]\r\n"), histogram.pm1, histogram.pm25, histogram.pm10);
    SERIAL_INFO(F("--> Sampling Period:\t[ %.3f s ] [ %.3f s ] [ %s ]\r\n"), histogram.sampling_period, sampling_period_s, is_sampling_period_error ? ERROR_STRING : OK_STRING);
    #endif

    #if (SERIAL_TRACE_LEVEL >= SERIAL_TRACE_LEVEL_DEBUG)
    for (uint8_t i = 0; i < OPCXX_BINS_LENGTH; i++) {
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
    memset(histogram.bins, UINT8_MAX, sizeof(uint16_t) * OPCXX_BINS_LENGTH);
    histogram.pm1 = UINT16_MAX;
    histogram.pm25 = UINT16_MAX;
    histogram.pm10 = UINT16_MAX;
  }

  return opcxx_state;
}

void OPCXX::getBins(uint16_t *destination) {
  memcpy(histogram.bins, destination, sizeof(uint16_t) * OPCXX_BINS_LENGTH);
}

uint16_t OPCXX::getBinAtIndex(uint16_t index) {
  return histogram.bins[index];
}

float OPCXX::getPm1() {
  return histogram.pm1;
}

float OPCXX::getPm25() {
  return histogram.pm25;
}

float OPCXX::getPm10() {
  return histogram.pm10;
}
