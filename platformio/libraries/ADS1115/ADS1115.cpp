/**@file ADS1115.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
<http://www.gnu.org/licenses/>.
**********************************************************************/

#include <debug_config.h>

/*!
\def SERIAL_TRACE_LEVEL
\brief Serial debug level for this library.
*/
#define SERIAL_TRACE_LEVEL ADS1115_SERIAL_TRACE_LEVEL

#include "ADS1115.h"

adc_result_t ADS1115::writeRegister(uint8_t i2c_address, uint8_t reg, uint16_t value) {
  Wire.beginTransmission(i2c_address);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)(value>>8));
  Wire.write((uint8_t)(value & 0xFF));
  bool status = !Wire.endTransmission();

  if (status) {
    return ADC_OK;
  }
  else {
    return ADC_ERROR;
  }
}

adc_result_t ADS1115::readRegister(uint8_t i2c_address, uint8_t reg, uint16_t *value) {
  *value = UINT16_MAX;
  Wire.beginTransmission(i2c_address);
  Wire.write(ADS1115_REG_POINTER_CONVERT);
  bool status = !Wire.endTransmission();

  if (status) {
    Wire.requestFrom(i2c_address, (uint8_t)2);
    *value = ((Wire.read() << 8) | Wire.read());
    return ADC_OK;
  }
  else {
    return ADC_ERROR;
  }
}

ADS1115::ADS1115(uint8_t i2c_address) {
  m_i2c_address = i2c_address;
  m_gain = GAIN_TWOTHIRDS;
  adc_state = ADC_INIT;
}

void ADS1115::setGain(adsGain_t gain) {
  m_gain = gain;
}

adsGain_t ADS1115::getGain() {
  return m_gain;
}

adc_result_t ADS1115::initSingleEnded(uint8_t channel) {
  if (channel > 3) {
    return ADC_ERROR;
  }

  // Start with default values
  uint16_t config = ADS1115_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
  ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
  ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
  ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
  ADS1115_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
  ADS1115_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= m_gain;

  // Set single-ended input channel
  switch (channel) {
    case (0):
    config |= ADS1115_REG_CONFIG_MUX_SINGLE_0;
    break;
    case (1):
    config |= ADS1115_REG_CONFIG_MUX_SINGLE_1;
    break;
    case (2):
    config |= ADS1115_REG_CONFIG_MUX_SINGLE_2;
    break;
    case (3):
    config |= ADS1115_REG_CONFIG_MUX_SINGLE_3;
    break;
  }

  // Set 'start single-conversion' bit
  config |= ADS1115_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  return writeRegister(m_i2c_address, ADS1115_REG_POINTER_CONFIG, config);
}

adc_result_t ADS1115::readSingleEnded(int16_t *value) {
  uint16_t result = 0;

  adc_result_t status = readRegister(m_i2c_address, ADS1115_REG_POINTER_CONVERT, &result);

  *value = (int16_t) (result);
  return status;
}

adc_result_t ADS1115::readSingleChannel(uint8_t channel, int16_t *value) {
  adc_result_t cmd_result;

  switch (adc_state) {
    case ADC_INIT:
      memset(adc_value, UINT8_MAX, sizeof(int16_t) * ADC_CHECK_COUNT);
      retry = 0;
      is_error = false;
      state_after_wait = ADC_INIT;
      adc_state = ADC_SET_CHANNEL;
      adc_result = ADC_BUSY;
      adc_value_count = 0;
      // SERIAL_TRACE(F("ADC_INIT --> ADC_SET_CHANNEL\r\n"));
    break;

    case ADC_SET_CHANNEL:
      cmd_result = initSingleEnded(channel);

      if (cmd_result == ADC_OK) {
        retry = 0;
        start_time_ms = millis();
        delay_ms = ADS1115_CONVERSION_DELAY_MS;
        adc_state = ADC_WAIT_STATE;
        state_after_wait = ADC_READ;
        // SERIAL_TRACE(F("ADC_SET_CHANNEL --> ADC_READ\r\n"));
      }
      else if ((cmd_result == ADC_ERROR) && ((retry++) < ADC_SET_CHANNEL_RETRY_COUNT_MAX)) {
        // SERIAL_DEBUG(F("ADC_SET_CHANNEL retry %u\r\n"), retry);
        start_time_ms = millis();
        delay_ms = ADC_SET_CHANNEL_RETRY_DELAY_MS;
        adc_state = ADC_WAIT_STATE;
        state_after_wait = ADC_SET_CHANNEL;
      }
      else {
        retry = 0;
        is_error = true;
        adc_state = ADC_END;
        // SERIAL_TRACE(F("ADC_SET_CHANNEL --> ADC_END\r\n"));
      }
    break;

    case ADC_READ:
      cmd_result = readSingleEnded(&adc_value[adc_value_count]);

      if (cmd_result == ADC_OK) {
        adc_state = ADC_CHECK;
        // SERIAL_DEBUG(F("canale %u adc_value_count %u valore %d\r\n"), channel, adc_value_count, adc_value[adc_value_count]);
        // SERIAL_TRACE(F("ADC_READ --> ADC_CHECK\r\n"));
      }
      else if ((cmd_result == ADC_ERROR) && ((retry++) < ADC_READ_RETRY_COUNT_MAX)) {
        // SERIAL_DEBUG(F("ADC_READ retry %u\r\n"), retry);
        start_time_ms = millis();
        delay_ms = ADC_READ_RETRY_DELAY_MS;
        adc_state = ADC_WAIT_STATE;
        state_after_wait = ADC_READ;
      }
      else {
        retry = 0;
        is_error = true;
        adc_state = ADC_END;
        // SERIAL_TRACE(F("ADC_READ --> ADC_END\r\n"));
      }
    break;

    case ADC_CHECK:
      // OK: 3 valori consecutivi identici
      if ((adc_value_count >= (ADC_CHECK_COUNT - 1)) && (adc_value[0] == adc_value[1]) && (adc_value[0] == adc_value[2])) {
        retry = 0;
        *value = adc_value[0];
        adc_state = ADC_END;
        // SERIAL_TRACE(F("ADC_CHECK --> ADC_END\r\n"));
      }
      // RETRY: 3 valori consecutivi NON identici
      else if ((adc_value_count >= (ADC_CHECK_COUNT - 1)) && ((retry++) < ADC_CHECK_RETRY_COUNT_MAX)) {
        start_time_ms = millis();
        delay_ms = ADC_CHECK_RETRY_DELAY_MS;
        adc_state = ADC_WAIT_STATE;
        state_after_wait = ADC_READ;
      }
      // ERRORE: 3 valori consecutivi NON identici e numero massimo di tentativi raggiunto
      else if ((adc_value_count >= (ADC_CHECK_COUNT - 1)) && (retry >= ADC_CHECK_RETRY_COUNT_MAX)) {
        retry = 0;
        adc_state = ADC_END;
        is_error = true;
        // SERIAL_TRACE(F("ADC_CHECK --> ADC_END\r\n"));
      }
      // OK: nuova acquisizione
      else {
        start_time_ms = millis();
        delay_ms = ADC_READ_DELAY_MS;
        adc_state = ADC_WAIT_STATE;
        state_after_wait = ADC_READ;
        // SERIAL_TRACE(F("ADC_CHECK --> ADC_READ\r\n"));
      }

      if (adc_value_count < (ADC_CHECK_COUNT - 1)) {
        adc_value_count++;
      }
      else {
        adc_value_count = 0;
      }
    break;

    case ADC_END:
      if (is_error) {
        adc_result = ADC_ERROR;
      }
      else {
        adc_result = ADC_OK;
      }
      adc_state = ADC_INIT;
      // SERIAL_TRACE(F("ADC_END --> ADC_INIT\r\n"));
    break;

    case ADC_WAIT_STATE:
      if (millis() - start_time_ms > delay_ms) {
        adc_state = state_after_wait;
      }
    break;
  }

  return adc_result;
}
