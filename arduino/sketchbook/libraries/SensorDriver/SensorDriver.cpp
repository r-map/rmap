/**@file SensorDriver.cpp */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo patruno <p.patruno@iperbole.bologna.it>
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
\brief Serial debug level for this library.
*/
#define SERIAL_TRACE_LEVEL SENSOR_DRIVER_SERIAL_TRACE_LEVEL

#include "SensorDriver.h"

namespace _SensorDriver {
  #if (USE_SENSOR_ADT)
  bool _is_adt_setted;
  bool _is_adt_prepared;
  #endif

  #if (USE_SENSOR_HIH)
  bool _is_hih_setted;
  bool _is_hih_prepared;
  #endif

  #if (USE_SENSOR_HYT)
  bool _is_hyt_setted;
  bool _is_hyt_prepared;
  #endif

  #if (USE_SENSOR_TBS || USE_SENSOR_TBR)
  bool _is_tb_setted;
  bool _is_tb_prepared;
  #endif

  #if (USE_SENSOR_STH || USE_SENSOR_ITH || USE_SENSOR_MTH || USE_SENSOR_NTH || USE_SENSOR_XTH)
  bool _is_th_setted;
  bool _is_th_prepared;
  #endif

  #if (USE_SENSOR_DW1)
  bool _is_dw1_setted;
  bool _is_dw1_prepared;
  #endif

  #if (USE_SENSOR_DEP)
  bool _is_dep_setted;
  bool _is_dep_prepared;
  #endif
}

/*********************************************************************
* SensorDriver
*********************************************************************/

SensorDriver::SensorDriver(const char* driver, const char* type) {
  _driver = driver;
  _type = type;
}

SensorDriver *SensorDriver::create(const char* driver, const char* type) {
  if (strlen(driver) == 0 || strlen(type) == 0) {
    SERIAL_ERROR(F("SensorDriver %s-%s create... [ %s ]\r\n--> driver or type is null.\r\n"), driver, type, FAIL_STRING);
    return NULL;
  }

  #if (USE_SENSOR_ADT)
  else if (strcmp(type, SENSOR_TYPE_ADT) == 0)
  return new SensorDriverAdt7420(driver, type, &_SensorDriver::_is_adt_setted, &_SensorDriver::_is_adt_prepared);
  #endif

  #if (USE_SENSOR_HIH)
  else if (strcmp(type, SENSOR_TYPE_HIH) == 0)
  return new SensorDriverHih6100(driver, type, &_SensorDriver::_is_hih_setted, &_SensorDriver::_is_hih_prepared);
  #endif

  #if (USE_SENSOR_HYT)
  else if (strcmp(type, SENSOR_TYPE_HYT) == 0)
  return new SensorDriverHyt2X1(driver, type, &_SensorDriver::_is_hyt_setted, &_SensorDriver::_is_hyt_prepared);
  #endif

  #if (USE_SENSOR_HI7)
  else if (strcmp(type, SENSOR_TYPE_HI7) == 0)
  return new SensorDriverHyt2X1(driver, type, &_SensorDriver::_is_hyt_setted, &_SensorDriver::_is_hyt_prepared);
  #endif

  #if (USE_SENSOR_BMP)
  else if (strcmp(type, SENSOR_TYPE_BMP) == 0)
  return new SensorDriverHyt2X1(driver, type, &_SensorDriver::_is_hyt_setted, &_SensorDriver::_is_hyt_prepared);
  #endif

  #if (USE_SENSOR_DW1)
  else if (strcmp(type, SENSOR_TYPE_DW1) == 0)
  return new SensorDriverDw1(driver, type, &_SensorDriver::_is_dw1_setted, &_SensorDriver::_is_dw1_prepared);
  #endif

  #if (USE_SENSOR_TBS || USE_SENSOR_TBR)
  else if (strcmp(type, SENSOR_TYPE_TBS) == 0 || strcmp(type, SENSOR_TYPE_TBR) == 0)
  return new SensorDriverRain(driver, type, &_SensorDriver::_is_tb_setted, &_SensorDriver::_is_tb_prepared);
  #endif

  #if (USE_SENSOR_STH || USE_SENSOR_ITH || USE_SENSOR_MTH || USE_SENSOR_NTH || USE_SENSOR_XTH)
  else if (strcmp(type, SENSOR_TYPE_STH) == 0 || strcmp(type, SENSOR_TYPE_ITH) == 0 || strcmp(type, SENSOR_TYPE_MTH) == 0 || strcmp(type, SENSOR_TYPE_NTH) == 0 || strcmp(type, SENSOR_TYPE_XTH) == 0)
  return new SensorDriverTh(driver, type, &_SensorDriver::_is_th_setted, &_SensorDriver::_is_th_prepared);
  #endif

  #if (USE_SENSOR_SSD)
  else if (strcmp(type, SENSOR_TYPE_SSD) == 0)
  return new SensorDriverHyt2X1(driver, type, &_SensorDriver::_is_hyt_setted, &_SensorDriver::_is_hyt_prepared);
  #endif

  #if (USE_SENSOR_ISD)
  else if (strcmp(type, SENSOR_TYPE_ISD) == 0)
  return new SensorDriverHyt2X1(driver, type, &_SensorDriver::_is_hyt_setted, &_SensorDriver::_is_hyt_prepared);
  #endif

  #if (USE_SENSOR_MSD)
  else if (strcmp(type, SENSOR_TYPE_MSD) == 0)
  return new SensorDriverHyt2X1(driver, type, &_SensorDriver::_is_hyt_setted, &_SensorDriver::_is_hyt_prepared);
  #endif

  #if (USE_SENSOR_NSD)
  else if (strcmp(type, SENSOR_TYPE_NSD) == 0)
  return new SensorDriverHyt2X1(driver, type, &_SensorDriver::_is_hyt_setted, &_SensorDriver::_is_hyt_prepared);
  #endif

  #if (USE_SENSOR_XSD)
  else if (strcmp(type, SENSOR_TYPE_XSD) == 0)
  return new SensorDriverHyt2X1(driver, type, &_SensorDriver::_is_hyt_setted, &_SensorDriver::_is_hyt_prepared);
  #endif

  #if (USE_SENSOR_DEP)
  else if (strcmp(type, SENSOR_TYPE_DEP) == 0)
  return new SensorDriverDigitecoPower(driver, type, &_SensorDriver::_is_dep_setted, &_SensorDriver::_is_dep_prepared);
  #endif

  else {
    SERIAL_ERROR(F("SensorDriver %s-%s create... [ FAIL ]\r\n--> driver or type not found.\r\n"), driver, type);
    return NULL;
  }
}

void SensorDriver::setup(const uint8_t address, const uint8_t node) {
  _address = address;
  _node = node;
  _start_time_ms = 0;
}

const char *SensorDriver::getDriver() {
  return _driver;
}

const char *SensorDriver::getType() {
  return _type;
}

uint8_t SensorDriver::getAddress() {
  return _address;
}

uint8_t SensorDriver::getNode() {
  return _node;
}

uint32_t SensorDriver::getDelay() {
  return _delay_ms;
}

uint32_t SensorDriver::getStartTime() {
  return _start_time_ms;
}

bool SensorDriver::isEnd() {
  return _is_end;
}

bool SensorDriver::isSuccess() {
  return _is_success;
}

bool SensorDriver::isReaded() {
  return _is_readed;
}

void SensorDriver::createAndSetup(const char* driver, const char* type, const uint8_t address, const uint8_t node, SensorDriver *sensors[], uint8_t *sensors_count) {
  sensors[*sensors_count] = SensorDriver::create(driver, type);
  if (sensors[*sensors_count]) {
    sensors[*sensors_count]->setup(address, node);
    (*sensors_count)++;
  }
}

void SensorDriver::printInfo(const char* driver, const char* type, const uint8_t address, const uint8_t node) {
  SERIAL_DEBUG(F("SensorDriver %s-%s"), driver, type);

  if (address) {
    SERIAL_DEBUG(F(" 0x%x (%d)"), address, address);
  }

  if (node) {
    SERIAL_DEBUG(F(" on node %d"), node);
  }
}

//------------------------------------------------------------------------------
// ADT7420
//------------------------------------------------------------------------------
#if (USE_SENSOR_ADT)
bool SensorDriverAdt7420::isSetted() {
  return *_is_setted;
}

bool SensorDriverAdt7420::isPrepared() {
  return *_is_prepared;
}

void SensorDriverAdt7420::resetPrepared() {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverAdt7420::setup(const uint8_t address, const uint8_t node) {
  SensorDriver::setup(address, node);
  SensorDriver::printInfo(_driver, _type, _address, _node);

  if (!*_is_setted) {
    Wire.beginTransmission(_address);
    Wire.write(0x03); // Set the register pointer to (0x01)
    Wire.write(0x20); // Set resolution and one shot

    if (Wire.endTransmission()) {
      SERIAL_DEBUG(F(" setup... [ %s ]\r\n"), FAIL_STRING);
      return;
    }

    *_is_setted = true;
    SERIAL_DEBUG(F(" setup... [ %s ]\r\n"), OK_STRING);
  }
  else {
    SERIAL_DEBUG(F(" setup... [ %s ]\r\n"), YES_STRING);
  }

  _delay_ms = 0;
  *_is_setted = true;
}

void SensorDriverAdt7420::prepare() {
  SensorDriver::printInfo(_driver, _type, _address, _node);

  if (!*_is_prepared) {
    Wire.beginTransmission(_address);
    Wire.write(0x03); // Set the register pointer to (0x01)
    Wire.write(0x20); // Set resolution and one shot

    if (Wire.endTransmission()) {
      SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), FAIL_STRING);
      return;
    }

    *_is_prepared = true;
    _delay_ms = 250;

    SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), OK_STRING);
  }
  else {
    SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), YES_STRING);
    _delay_ms = 0;
  }

  _start_time_ms = millis();
}

void SensorDriverAdt7420::get(int32_t *values, uint8_t length) {
  static int temperature;
  uint8_t msb;
  uint8_t lsb;

  switch (_get_state) {
    case INIT:
    temperature = 0;
    memset(values, UINT8_MAX, length * sizeof(int32_t));

    _is_success = true;
    _is_readed = false;
    _is_end = false;

    if (*_is_prepared && length >= 1) {
      Wire.beginTransmission(_address);
      Wire.write(0x00); // Set the register pointer to (0x00)

      if (Wire.endTransmission()) {
        _is_success = false;
      }
    }
    else {
      _is_success = false;
    }

    if (_is_success) {
      _get_state = READ;
    }
    else {
      _get_state = END;
    }

    _delay_ms = 0;
    _start_time_ms = millis();
    break;

    case READ:
    Wire.requestFrom(_address, (uint8_t) 2);

    if (Wire.available() < 2) {
      _is_success = false;
    }
    else {
      msb = Wire.read();
      lsb = Wire.read();

      if ((msb == 255) && (lsb == 255)) {
        _is_success = false;
      }
      else {
        _is_success = true;
      }

      // it's a 13bit int, using two's compliment for negative
      temperature = ((msb << 8) | lsb) >> 3 & 0xFFF;

      if (temperature & 0x800) {
        temperature -= 0x1000;
      }
    }

    _delay_ms = 0;
    _start_time_ms = millis();
    _get_state = END;
    break;

    case END:
    if (length >= 1) {
      if (_is_success) {
        values[0] = (temperature*6.25) + SENSOR_DRIVER_C_TO_K;
      }
      else {
        values[0] = UINT16_MAX;
      }
    }

    SensorDriver::printInfo(_driver, _type, _address, _node);
    SERIAL_DEBUG(F(" get... [ %s ]\r\n"), _is_success ? OK_STRING : FAIL_STRING);

    if (length >= 1) {
      if (isValid(values[0])) {
        SERIAL_DEBUG(F("--> temperature: %u\r\n"), values[0]);
      }
      else {
        SERIAL_DEBUG(F("--> temperature: ---\r\n"));
      }
    }

    _start_time_ms = millis();
    _delay_ms = 0;
    _is_end = true;
    _is_readed = false;
    _get_state = INIT;
    break;
  }
}

#if (USE_JSON)
void SensorDriverAdt7420::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length) {
  SensorDriverAdt7420::get(values, length);

  if (_is_end && !_is_readed) {
    StaticJsonBuffer<JSON_BUFFER_LENGTH> buffer;
    JsonObject &json = buffer.createObject();

    if (length >= 1) {
      if (isValid(values[0])) {
        json["B12101"] = values[0];
      }
      else json["B12101"] = RawJson("null");
    }

    json.printTo(json_buffer, json_buffer_length);
  }
}
#endif

#endif

//------------------------------------------------------------------------------
// HIH6100
//------------------------------------------------------------------------------
#if (USE_SENSOR_HIH)
bool SensorDriverHih6100::isSetted() {
  return *_is_setted;
}

bool SensorDriverHih6100::isPrepared() {
  return *_is_prepared;
}

void SensorDriverHih6100::resetPrepared() {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverHih6100::setup(const uint8_t address, const uint8_t node) {
  SensorDriver::setup(address, node);
  SensorDriver::printInfo(_driver, _type, _address, _node);
  SERIAL_DEBUG(F(" setup... [ %s ]\r\n"), OK_STRING);
  _delay_ms = 0;
  *_is_setted = true;
}

void SensorDriverHih6100::prepare() {
  SensorDriver::printInfo(_driver, _type, _address, _node);

  if (!*_is_prepared) {
    Wire.beginTransmission(_address);

    if (Wire.endTransmission()) {
      SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), FAIL_STRING);
      return;
    }

    *_is_prepared = true;
    _delay_ms = 40;

    SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), OK_STRING);
  }
  else {
    SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), YES_STRING);
    _delay_ms = 0;
  }

  _start_time_ms = millis();
}

void SensorDriverHih6100::get(int32_t *values, uint8_t length) {
  static uint16_t temperature;
  static uint16_t humidity;
  uint8_t x;
  uint8_t y;
  uint8_t s;

  switch (_get_state) {
    case INIT:
    temperature = 0;
    humidity = 0;
    memset(values, UINT8_MAX, length * sizeof(int32_t));

    _is_success = true;
    _is_readed = false;
    _is_end = false;

    if (*_is_prepared && length >= 1) {
      _get_state = READ;
    }
    else {
      _is_success = false;
      _get_state = END;
    }

    _delay_ms = 0;
    _start_time_ms = millis();
    break;

    case READ:
    Wire.requestFrom(_address, (uint8_t) 4);

    if (Wire.available() < 4) {
      _is_success = false;
    }
    else {
      x = Wire.read();
      s = (x & 0xC0 ) >> 6;

      if (s == 0) {
        // status 0 == OK
        // Normal Operation, Valid Data that has not been fetched since the last measurement cycle

        y = Wire.read();
        humidity = (((uint16_t) (x & 0x3F)) << 8) | y;

        x = Wire.read();
        y = Wire.read();

        temperature = (((uint16_t) x) << 6) | ((y & 0xFC) >> 2);
        _is_success = true;
      }
      else {
        // status 1
        // Stale Data: Data that has already been
        // fetched since the last measurement cycle, or
        // data fetched before the first measurement
        // has been completed

        // status 2
        // Device in Command Mode
        // Command Mode is used for programming the sensor.
        // This mode should not be seen during normal operation

        // status 3
        // Not Used
        _is_success = false;
      }
    }

    if (Wire.endTransmission()) {
      _is_success = false;
    }

    _delay_ms = 0;
    _start_time_ms = millis();
    _get_state = END;
    break;

    case END:
    if (length >= 1) {
      if (_is_success) {
        values[0] = round(float(humidity) / 16382. * 100);
      }
      else {
        values[0] = UINT16_MAX;
      }
    }

    if (length >= 2) {
      if (_is_success) {
        values[1] = round((float(temperature) / 16382. * 165. - 40.) * 100) + SENSOR_DRIVER_C_TO_K;
      }
      else {
        values[1] = UINT16_MAX;
      }
    }

    SensorDriver::printInfo(_driver, _type, _address, _node);
    SERIAL_DEBUG(F(" get... [ %s ]\r\n"), _is_success ? OK_STRING : FAIL_STRING);

    if (length >= 1) {
      if (isValid(values[0])) {
        SERIAL_DEBUG(F("--> humidity: %u\r\n"), values[0]);
      }
      else {
        SERIAL_DEBUG(F("--> humidity: ---\r\n"));
      }
    }

    if (length >= 2) {
      if (isValid(values[1])) {
        SERIAL_DEBUG(F("--> temperature: %u\r\n"), values[1]);
      }
      else {
        SERIAL_DEBUG(F("--> temperature: ---\r\n"));
      }
    }

    _start_time_ms = millis();
    _delay_ms = 0;
    _is_end = true;
    _is_readed = false;
    _get_state = INIT;
    break;
  }
}

#if (USE_JSON)
void SensorDriverHih6100::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length) {
  SensorDriverHih6100::get(values, length);

  if (_is_end && !_is_readed) {
    StaticJsonBuffer<JSON_BUFFER_LENGTH> buffer;
    JsonObject &json = buffer.createObject();

    if (length >= 1) {
      if (isValid(values[0])) {
        json["B13003"] = values[0];
      }
      else json["B13003"] = RawJson("null");
    }

    if (length >= 2) {
      if (isValid(values[1])) {
        json["B12101"] = values[1];
      }
      else json["B12101"] = RawJson("null");
    }

    json.printTo(json_buffer, json_buffer_length);
  }
}
#endif

#endif

//------------------------------------------------------------------------------
// HYT2X1
//------------------------------------------------------------------------------
#if (USE_SENSOR_HYT)
bool SensorDriverHyt2X1::isSetted() {
  return *_is_setted;
}

bool SensorDriverHyt2X1::isPrepared() {
  return *_is_prepared;
}

void SensorDriverHyt2X1::resetPrepared() {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverHyt2X1::setup(const uint8_t address, const uint8_t node) {
  SensorDriver::setup(address, node);
  SensorDriver::printInfo(_driver, _type, _address, _node);
  *_is_setted = true;
  _delay_ms = 0;
  SERIAL_DEBUG(F(" setup... [ %s ]\r\n"), OK_STRING);
}

void SensorDriverHyt2X1::prepare() {
  SensorDriver::printInfo(_driver, _type, _address, _node);

  if (!*_is_prepared) {
    *_is_prepared = Hyt2X1::hyt_initRead(_address);
    _delay_ms = HYT2X1_CONVERSION_TIME_MS;
    SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), OK_STRING);
  }
  else {
    _delay_ms = 0;
    SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), YES_STRING);
  }

  _start_time_ms = millis();
}

void SensorDriverHyt2X1::get(int32_t *values, uint8_t length) {
  static float humidity;
  static float temperature;

  switch (_get_state) {
    case INIT:
    humidity = UINT16_MAX;
    temperature = UINT16_MAX;
    memset(values, UINT8_MAX, length * sizeof(int32_t));

    _is_readed = false;
    _is_end = false;

    if (*_is_prepared && length >= 1) {
      _is_success = true;
      _get_state = READ;
    }
    else {
      _is_success = false;
      _get_state = END;
    }

    _delay_ms = 0;
    _start_time_ms = millis();
    break;

    case READ:
    _is_success = Hyt2X1::hyt_read(_address, &humidity, &temperature);
    _delay_ms = 0;
    _start_time_ms = millis();
    _get_state = END;
    break;

    case END:
    if (length >= 1) {
      if (_is_success) {
        values[0] = round(humidity);
      }
      else {
        values[0] = UINT16_MAX;
      }
    }

    if (length >= 2) {
      if (_is_success) {
        values[1] = SENSOR_DRIVER_C_TO_K + (int32_t)(temperature * 100.0);
      }
      else {
        values[1] = UINT16_MAX;
      }
    }

    SensorDriver::printInfo(_driver, _type, _address, _node);
    SERIAL_DEBUG(F(" get... [ %s ]\r\n"), _is_success ? OK_STRING : FAIL_STRING);

    if (length >= 1) {
      if (isValid(values[0])) {
        SERIAL_DEBUG(F("--> humidity: %u\r\n"), values[0]);
      }
      else {
        SERIAL_DEBUG(F("--> humidity: ---\r\n"));
      }
    }

    if (length >= 2) {
      if (isValid(values[1])) {
        SERIAL_DEBUG(F("--> temperature: %u\r\n"), values[1]);
      }
      else {
        SERIAL_DEBUG(F("--> temperature: ---\r\n"));
      }
    }

    _start_time_ms = millis();
    _delay_ms = 0;
    _is_end = true;
    _is_readed = false;
    _get_state = INIT;
    break;
  }
}

#if (USE_JSON)
void SensorDriverHyt2X1::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length) {
  SensorDriverHyt2X1::get(values, length);

  if (_is_end && !_is_readed) {
    StaticJsonBuffer<JSON_BUFFER_LENGTH> buffer;
    JsonObject &json = buffer.createObject();

    if (length >= 1) {
      if (isValid(values[0])) {
        json["B13003"] = values[0];
      }
      else json["B13003"] = RawJson("null");
    }

    if (length >= 2) {
      if (isValid(values[1])) {
        json["B12101"] = values[1];
      }
      else json["B12101"] = RawJson("null");
    }

    json.printTo(json_buffer, json_buffer_length);
  }
}
#endif

#endif

//------------------------------------------------------------------------------
// I2C-Wind
// DW1: oneshot
//------------------------------------------------------------------------------

#if (USE_SENSOR_DW1)
void SensorDriverDw1::getSDfromUV(int32_t u, int32_t v, double *speed, double *direction) {
  *speed = sqrt(pow(u, 2) + pow(v, 2));
  *direction = atan2(-u, -v);
  *direction *= 180 / M_PI;
  *direction = 360 + round(*direction);
  *direction = ((uint16_t) *direction) % 360;
}

bool SensorDriverDw1::isSetted() {
  return *_is_setted;
}

bool SensorDriverDw1::isPrepared() {
  return *_is_prepared;
}

void SensorDriverDw1::resetPrepared() {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverDw1::setup(const uint8_t address, const uint8_t node) {
  SensorDriver::setup(address, node);
  SensorDriver::printInfo(_driver, _type, _address, _node);
  *_is_setted = true;
  _delay_ms = 0;
  SERIAL_DEBUG(F(" setup... [ %s ]\r\n"), OK_STRING);
}

void SensorDriverDw1::prepare() {
  SensorDriver::printInfo(_driver, _type, _address, _node);

  if (!*_is_prepared) {
    Wire.beginTransmission(_address);
    Wire.write(I2C_WINDSONIC_COMMAND);
    Wire.write(I2C_WINDSONIC_COMMAND_STOP);
    _delay_ms = 3000;

    if (Wire.endTransmission()) {
      SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), FAIL_STRING);
      return;
    }

    *_is_prepared = true;

    SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), OK_STRING);
  }
  else {
    SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), YES_STRING);
    _delay_ms = 0;
  }

  _start_time_ms = millis();
}

void SensorDriverDw1::get(int32_t *values, uint8_t length) {
  const float raddeg = 180 / M_PI;
  static double speed = UINT16_MAX;
  static double direction = UINT16_MAX;
  uint16_t msb;
  uint16_t lsb;

  switch (_get_state) {
    case INIT:
    memset(values, UINT8_MAX, length * sizeof(int32_t));

    _is_readed = false;
    _is_end = false;

    if (*_is_prepared && length >= 1) {
      _is_success = true;
      _get_state = SET_MEANU_ADDRESS;
    }
    else {
      _is_success = false;
      _get_state = END;
    }

    _delay_ms = 0;
    _start_time_ms = millis();
    break;

    case SET_MEANU_ADDRESS:
    Wire.beginTransmission(_address);
    Wire.write(I2C_WINDSONIC_MEANU);

    if (Wire.endTransmission()) {
      _is_success = false;
    }

    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success) {
      _get_state = READ_MEANU;
    }
    else {
      _get_state = END;
    }
    break;

    case READ_MEANU:
    Wire.requestFrom(_address, (uint8_t) 2);

    if (Wire.available() < 2) {
      _is_success = false;
    }

    if (_is_success) {
      lsb = Wire.read();
      msb = Wire.read();
      values[0] = ((int) (msb << 8) | lsb);
      if (isValid(values[0])) {
        values[0] -= OFFSET;
      }
    }

    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success && length >= 2) {
      _get_state = SET_MEANV_ADDRESS;
    }
    else {
      _get_state = ELABORATE;
    }
    break;

    case SET_MEANV_ADDRESS:
    Wire.beginTransmission(_address);
    Wire.write(I2C_WINDSONIC_MEANV);

    if (Wire.endTransmission()) {
      _is_success = false;
    }

    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success) {
      _get_state = READ_MEANV;
    }
    else {
      _get_state = END;
    }
    break;

    case READ_MEANV:
    Wire.requestFrom(_address, (uint8_t) 2);

    if (Wire.available() < 2) {
      _is_success = false;
    }

    if (_is_success) {
      lsb = Wire.read();
      msb = Wire.read();
      values[1] = ((uint16_t) (msb << 8) | lsb);
      if (isValid(values[1])) {
        values[1] -= OFFSET;
      }
    }

    _delay_ms = 0;
    _start_time_ms = millis();

    _get_state = ELABORATE;
    break;

    case ELABORATE:
    if (isValid(values[0]) && isValid(values[1]) && length >= 2) {
      if (values[0] || values[1]) {
        SensorDriverDw1::getSDfromUV(values[0], values[1], &speed, &direction);

        if (direction == 0) {
          direction = 360;
        }
      }
      else {
        speed = 0;
        direction = 0;
      }
    }
    _get_state = END;
    break;

    case END:
    SensorDriver::printInfo(_driver, _type, _address, _node);
    SERIAL_DEBUG(F(" get... [ %s ]\r\n"), _is_success ? OK_STRING : FAIL_STRING);

    if (length >= 1) {
      if (isValid(values[0])) {
        SERIAL_DEBUG(F("--> mean u: %u\r\n"), values[0]);
      }
      else {
        SERIAL_DEBUG(F("--> mean u: ---\r\n"));
      }
    }

    if (length >= 2) {
      if (isValid(values[1])) {
        SERIAL_DEBUG(F("--> mean v: %u\r\n"), values[1]);
      }
      else {
        SERIAL_DEBUG(F("--> mean v: ---\r\n"));
      }
    }

    if (isValid(values[0]) && isValid(values[1]) && length >= 2) {
      values[0] = (int32_t) direction;
      values[1] = (int32_t) round(speed);
      SERIAL_DEBUG(F("--> direction: %u\r\n"), values[0]);
      SERIAL_DEBUG(F("--> speed: %u\r\n"), values[1]);
    } else {
      SERIAL_DEBUG(F("--> direction: ---\r\n"));
      SERIAL_DEBUG(F("--> speed: ---\r\n"));
    }

    _start_time_ms = millis();
    _delay_ms = 0;
    _is_end = true;
    _is_readed = false;
    _get_state = INIT;
    break;
  }
}

#if (USE_JSON)
void SensorDriverDw1::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length) {
  SensorDriverDw1::get(values, length);

  if (_is_end && !_is_readed) {
    StaticJsonBuffer<JSON_BUFFER_LENGTH> buffer;
    JsonObject &json = buffer.createObject();

    if (length >= 1) {
      if (isValid(values[0])) {
        json["B11001"] = values[0];
      }
      else json["B11001"] = RawJson("null");
    }

    if (length >= 2) {
      if (isValid(values[1])) {
        json["B11002"] = values[1];
      }
      else json["B11002"] = RawJson("null");
    }

    json.printTo(json_buffer, json_buffer_length);
  }
}
#endif

#endif

//------------------------------------------------------------------------------
// I2C-Rain
// TBS: oneshot
// TBR: oneshot
//------------------------------------------------------------------------------

#if (USE_SENSOR_TBS || USE_SENSOR_TBR)
bool SensorDriverRain::isSetted() {
  return *_is_setted;
}

bool SensorDriverRain::isPrepared() {
  return *_is_prepared;
}

void SensorDriverRain::resetPrepared() {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverRain::setup(const uint8_t address, const uint8_t node) {
  SensorDriver::setup(address, node);
  SensorDriver::printInfo(_driver, _type, _address, _node);
  *_is_setted = true;
  _delay_ms = 0;
  SERIAL_DEBUG(F(" setup... [ %s ]\r\n"), OK_STRING);
}

void SensorDriverRain::prepare() {
  SensorDriver::printInfo(_driver, _type, _address, _node);

  if (!*_is_prepared) {
    Wire.beginTransmission(_address);
    Wire.write(I2C_COMMAND_ID);

    if (strcmp(_type, SENSOR_TYPE_TBS) == 0 || strcmp(_type, SENSOR_TYPE_TBR) == 0) {
      Wire.write(I2C_RAIN_COMMAND_ONESHOT_START_STOP);
      _delay_ms = 0;
    }
    else {
      _delay_ms = 0;
      SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), FAIL_STRING);
      return;
    }

    if (Wire.endTransmission()) {
      SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), FAIL_STRING);
      return;
    }

    *_is_prepared = true;

    SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), OK_STRING);
  }
  else {
    SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), YES_STRING);
    _delay_ms = 0;
  }

  _start_time_ms = millis();
}

void SensorDriverRain::get(int32_t *values, uint8_t length) {
  static uint8_t rain_data[I2C_RAIN_TIPS_LENGTH];
  uint8_t data_length;

  switch (_get_state) {
    case INIT:
    memset(values, UINT8_MAX, length * sizeof(int32_t));
    memset(rain_data, UINT8_MAX, I2C_RAIN_TIPS_LENGTH);

    _is_readed = false;
    _is_end = false;

    if (*_is_prepared && length >= 1) {
      _is_success = true;
      _get_state = SET_RAIN_ADDRESS;
    }
    else {
      _is_success = false;
      _get_state = END;
    }

    _delay_ms = 0;
    _start_time_ms = millis();
    break;

    case SET_RAIN_ADDRESS:
    Wire.beginTransmission(_address);

    if (strcmp(_type, SENSOR_TYPE_TBS) == 0 || strcmp(_type, SENSOR_TYPE_TBR) == 0) {
      Wire.write(I2C_RAIN_TIPS_ADDRESS);
      Wire.write(I2C_RAIN_TIPS_LENGTH);
    }
    else _is_success = false;

    if (Wire.endTransmission()) {
      _is_success = false;
    }

    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success) {
      _get_state = READ_RAIN;
    }
    else {
      _get_state = END;
    }
    break;

    case READ_RAIN:
    if (strcmp(_type, SENSOR_TYPE_TBS) == 0 || strcmp(_type, SENSOR_TYPE_TBR) == 0) {
      data_length = I2C_RAIN_TIPS_LENGTH;
    }
    else _is_success = false;

    Wire.requestFrom(_address, data_length);

    if (Wire.available() < data_length) {
      _is_success = false;
    }

    if (_is_success) {
      for (uint8_t i=0; i<data_length; i++) {
        rain_data[i] = Wire.read();
      }
    }

    _start_time_ms = millis();
    _delay_ms = 0;
    _is_end = false;

    if (_is_success) {
      _get_state = END;
    }
    else {
      _get_state = END;
    }
    break;

    case END:
    if (length >= 1) {
      values[0] = (uint16_t)(rain_data[1] << 8) | (rain_data[0]);
      values[0] *= RAIN_FOR_TIP;
    }

    SensorDriver::printInfo(_driver, _type, _address, _node);
    SERIAL_DEBUG(F(" get... [ %s ]\r\n"), _is_success ? OK_STRING : FAIL_STRING);

    if (length >= 1) {
      if (isValid(values[0])) {
        SERIAL_DEBUG(F("--> rain tips: %u\r\n"), values[0]);
      }
      else {
        SERIAL_DEBUG(F("--> rain tips: ---\r\n"));
      }
    }

    _start_time_ms = millis();
    _delay_ms = 20;
    _is_end = true;
    _is_readed = false;
    _get_state = INIT;
    break;
  }
}

#if (USE_JSON)
void SensorDriverRain::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length) {
  SensorDriverRain::get(values, length);

  if (_is_end && !_is_readed) {
    StaticJsonBuffer<JSON_BUFFER_LENGTH> buffer;
    JsonObject &json = buffer.createObject();

    if (length >= 1) {
      if (isValid(values[0])) {
        json["B13011"] = values[0];
      }
      else json["B13011"] = RawJson("null");
    }

    json.printTo(json_buffer, json_buffer_length);
  }
}
#endif

#endif

//------------------------------------------------------------------------------
// I2C-TH
// STH: oneshot
// ITH: continuous istantaneous
// MTH: continuous average
// NTH: continuous min
// XTH: continuous max
//------------------------------------------------------------------------------

#if (USE_SENSOR_STH || USE_SENSOR_ITH || USE_SENSOR_MTH || USE_SENSOR_NTH || USE_SENSOR_XTH)
bool SensorDriverTh::isSetted() {
  return *_is_setted;
}

bool SensorDriverTh::isPrepared() {
  return *_is_prepared;
}

void SensorDriverTh::resetPrepared() {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverTh::setup(const uint8_t address, const uint8_t node) {
  SensorDriver::setup(address, node);
  SensorDriver::printInfo(_driver, _type, _address, _node);

  if (!*_is_setted) {
    Wire.beginTransmission(_address);
    Wire.write(I2C_COMMAND_ID);

    if (strcmp(_type, SENSOR_TYPE_ITH) == 0 || strcmp(_type, SENSOR_TYPE_MTH) == 0 || strcmp(_type, SENSOR_TYPE_NTH) == 0 || strcmp(_type, SENSOR_TYPE_XTH) == 0) {
      Wire.write(I2C_TH_COMMAND_CONTINUOUS_START);
    }
    else {
      SERIAL_DEBUG(F(" setup... [ %s ]\r\n"), FAIL_STRING);
      return;
    }

    if (Wire.endTransmission()) {
      SERIAL_DEBUG(F(" setup... [ %s ]\r\n"), FAIL_STRING);
      return;
    }

    *_is_setted = true;

    SERIAL_DEBUG(F(" setup... [ %s ]\r\n"), OK_STRING);
  }
  else {
    SERIAL_DEBUG(F(" setup... [ %s ]\r\n"), YES_STRING);
  }
}

void SensorDriverTh::prepare() {
  SensorDriver::printInfo(_driver, _type, _address, _node);

  if (!*_is_prepared) {
    Wire.beginTransmission(_address);
    Wire.write(I2C_COMMAND_ID);

    if (strcmp(_type, SENSOR_TYPE_STH) == 0) {
      Wire.write(I2C_TH_COMMAND_ONESHOT_START_STOP);
      _delay_ms = 150;
    }
    else if (strcmp(_type, SENSOR_TYPE_ITH) == 0 || strcmp(_type, SENSOR_TYPE_MTH) == 0 || strcmp(_type, SENSOR_TYPE_NTH) == 0 || strcmp(_type, SENSOR_TYPE_XTH) == 0) {
      Wire.write(I2C_TH_COMMAND_CONTINUOUS_START_STOP);
      _delay_ms = 0;
    }
    else {
      SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), FAIL_STRING);
      return;
    }

    if (Wire.endTransmission()) {
      SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), FAIL_STRING);
      return;
    }

    *_is_prepared = true;

    SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), OK_STRING);
  }
  else {
    SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), YES_STRING);
    _delay_ms = 0;
  }

  _start_time_ms = millis();
}

void SensorDriverTh::get(int32_t *values, uint8_t length) {
  static uint8_t temperature_data[I2C_TH_TEMPERATURE_DATA_MAX_LENGTH];
  static uint8_t humidity_data[I2C_TH_HUMIDITY_DATA_MAX_LENGTH];
  uint8_t data_length;

  switch (_get_state) {
    case INIT:
    memset(values, UINT8_MAX, length * sizeof(int32_t));
    memset(temperature_data, UINT8_MAX, I2C_TH_TEMPERATURE_DATA_MAX_LENGTH);
    memset(humidity_data, UINT8_MAX, I2C_TH_HUMIDITY_DATA_MAX_LENGTH);

    _is_readed = false;
    _is_end = false;

    if (*_is_prepared && length >= 1) {
      _is_success = true;
      _get_state = SET_TEMPERATURE_ADDRESS;
    }
    else {
      _is_success = false;
      _get_state = END;
    }

    _delay_ms = 0;
    _start_time_ms = millis();
    break;

    case SET_TEMPERATURE_ADDRESS:
    Wire.beginTransmission(_address);

    if (strcmp(_type, SENSOR_TYPE_STH) == 0) {
      Wire.write(I2C_TH_TEMPERATURE_SAMPLE_ADDRESS);
      Wire.write(I2C_TH_TEMPERATURE_SAMPLE_LENGTH);
    }
    else if (strcmp(_type, SENSOR_TYPE_ITH) == 0) {
      Wire.write(I2C_TH_TEMPERATURE_MED60_ADDRESS);
      Wire.write(I2C_TH_TEMPERATURE_MED60_LENGTH);
    }
    else if (strcmp(_type, SENSOR_TYPE_MTH) == 0) {
      Wire.write(I2C_TH_TEMPERATURE_MED_ADDRESS);
      Wire.write(I2C_TH_TEMPERATURE_MED_LENGTH);
    }
    else if (strcmp(_type, SENSOR_TYPE_NTH) == 0) {
      Wire.write(I2C_TH_TEMPERATURE_MIN_ADDRESS);
      Wire.write(I2C_TH_TEMPERATURE_MIN_LENGTH);
    }
    else if (strcmp(_type, SENSOR_TYPE_XTH) == 0) {
      Wire.write(I2C_TH_TEMPERATURE_MAX_ADDRESS);
      Wire.write(I2C_TH_TEMPERATURE_MAX_LENGTH);
    }
    else _is_success = false;

    if (Wire.endTransmission()) {
      _is_success = false;
    }

    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success) {
      _get_state = READ_TEMPERATURE;
    }
    else {
      _get_state = END;
    }
    break;

    case READ_TEMPERATURE:
    if (strcmp(_type, SENSOR_TYPE_STH) == 0) {
      data_length = I2C_TH_TEMPERATURE_SAMPLE_LENGTH;
    }
    else if (strcmp(_type, SENSOR_TYPE_ITH) == 0) {
      data_length = I2C_TH_TEMPERATURE_MED60_LENGTH;
    }
    else if (strcmp(_type, SENSOR_TYPE_MTH) == 0) {
      data_length = I2C_TH_TEMPERATURE_MED_LENGTH;
    }
    else if (strcmp(_type, SENSOR_TYPE_NTH) == 0) {
      data_length = I2C_TH_TEMPERATURE_MIN_LENGTH;
    }
    else if (strcmp(_type, SENSOR_TYPE_XTH) == 0) {
      data_length = I2C_TH_TEMPERATURE_MAX_LENGTH;
    }
    else _is_success = false;

    Wire.requestFrom(_address, data_length);

    if (Wire.available() < data_length) {
      _is_success = false;
    }

    if (_is_success) {
      for (uint8_t i=0; i<data_length; i++) {
        temperature_data[i] = Wire.read();
      }
    }

    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success && length >= 2) {
      _get_state = SET_HUMIDITY_ADDRESS;
    }
    else {
      _get_state = END;
    }
    break;

    case SET_HUMIDITY_ADDRESS:
    Wire.beginTransmission(_address);

    if (strcmp(_type, SENSOR_TYPE_STH) == 0) {
      Wire.write(I2C_TH_HUMIDITY_SAMPLE_ADDRESS);
      Wire.write(I2C_TH_HUMIDITY_SAMPLE_LENGTH);
    }
    else if (strcmp(_type, SENSOR_TYPE_ITH) == 0) {
      Wire.write(I2C_TH_HUMIDITY_MED60_ADDRESS);
      Wire.write(I2C_TH_HUMIDITY_MED60_LENGTH);
    }
    else if (strcmp(_type, SENSOR_TYPE_MTH) == 0) {
      Wire.write(I2C_TH_HUMIDITY_MED_ADDRESS);
      Wire.write(I2C_TH_HUMIDITY_MED_LENGTH);
    }
    else if (strcmp(_type, SENSOR_TYPE_NTH) == 0) {
      Wire.write(I2C_TH_HUMIDITY_MIN_ADDRESS);
      Wire.write(I2C_TH_HUMIDITY_MIN_LENGTH);
    }
    else if (strcmp(_type, SENSOR_TYPE_XTH) == 0) {
      Wire.write(I2C_TH_HUMIDITY_MAX_ADDRESS);
      Wire.write(I2C_TH_HUMIDITY_MAX_LENGTH);
    }
    else _is_success = false;

    if (Wire.endTransmission()) {
      _is_success = false;
    }

    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success) {
      _get_state = READ_HUMIDITY;
    }
    else {
      _get_state = END;
    }
    break;

    case READ_HUMIDITY:
    if (strcmp(_type, SENSOR_TYPE_STH) == 0) {
      data_length = I2C_TH_HUMIDITY_SAMPLE_LENGTH;
    }
    else if (strcmp(_type, SENSOR_TYPE_ITH) == 0) {
      data_length = I2C_TH_HUMIDITY_MED60_LENGTH;
    }
    else if (strcmp(_type, SENSOR_TYPE_MTH) == 0) {
      data_length = I2C_TH_HUMIDITY_MED_LENGTH;
    }
    else if (strcmp(_type, SENSOR_TYPE_NTH) == 0) {
      data_length = I2C_TH_HUMIDITY_MIN_LENGTH;
    }
    else if (strcmp(_type, SENSOR_TYPE_XTH) == 0) {
      data_length = I2C_TH_HUMIDITY_MAX_LENGTH;
    }
    else _is_success = false;

    Wire.requestFrom(_address, data_length);

    if (Wire.available() < data_length) {
      _is_success = false;
    }

    if (_is_success) {
      for (uint8_t i=0; i<data_length; i++) {
        humidity_data[i] = Wire.read();
      }
    }

    _delay_ms = 0;
    _start_time_ms = millis();
    _get_state = END;
    break;

    case END:
    if (length >= 1) {
      if (_is_success) {
        values[0] = ((uint16_t)(temperature_data[1] << 8) | (temperature_data[0]));
      }
      else {
        _is_success = false;
      }
    }

    if (length >= 2) {
      if (_is_success) {
        values[1] = ((uint16_t)(humidity_data[1] << 8) | (humidity_data[0]));
      }
      else {
        _is_success = false;
      }
    }

    SensorDriver::printInfo(_driver, _type, _address, _node);
    SERIAL_DEBUG(F(" get... [ %s ]\r\n"), _is_success ? OK_STRING : FAIL_STRING);

    if (length >= 1) {
      if (isValid(values[0])) {
        SERIAL_DEBUG(F("--> temperature: %u\r\n"), values[0]);
      }
      else {
        SERIAL_DEBUG(F("--> temperature: ---\r\n"));
      }
    }

    if (length >= 2) {
      if (isValid(values[1])) {
        SERIAL_DEBUG(F("--> humidity: %u\r\n"), values[1]);
      }
      else {
        SERIAL_DEBUG(F("--> humidity: ---\r\n"));
      }
    }

    _start_time_ms = millis();
    _delay_ms = 0;
    _is_end = true;
    _is_readed = false;
    _get_state = INIT;
    break;
  }
}

#if (USE_JSON)
void SensorDriverTh::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length) {
  SensorDriverTh::get(values, length);

  if (_is_end && !_is_readed) {
    StaticJsonBuffer<JSON_BUFFER_LENGTH> buffer;
    JsonObject &json = buffer.createObject();

    if (length >= 1) {
      if (isValid(values[0])) {
        json["B12101"] = values[0];
      }
      else json["B12101"] = RawJson("null");
    }

    if (length >= 2) {
      if (isValid(values[1])) {
        json["B13003"] = values[1];
      }
      else json["B13003"] = RawJson("null");
    }

    json.printTo(json_buffer, json_buffer_length);
  }
}
#endif

#endif

//------------------------------------------------------------------------------
// DigitEco Power
//------------------------------------------------------------------------------

#if (USE_SENSOR_DEP)
bool SensorDriverDigitecoPower::isSetted() {
  return *_is_setted;
}

bool SensorDriverDigitecoPower::isPrepared() {
  return *_is_prepared;
}

void SensorDriverDigitecoPower::resetPrepared() {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverDigitecoPower::setup(const uint8_t address, const uint8_t node) {
  SensorDriver::setup(address, node);
  SensorDriver::printInfo(_driver, _type, _address, _node);
  *_is_setted = true;
  _delay_ms = 0;
  SERIAL_DEBUG(F(" setup... [ %s ]\r\n"), OK_STRING);
}

void SensorDriverDigitecoPower::prepare() {
  SensorDriver::printInfo(_driver, _type, _address, _node);
  _delay_ms = 0;
  *_is_prepared = true;
  _start_time_ms = millis();
  SERIAL_DEBUG(F(" prepare... [ %s ]\r\n"), OK_STRING);
}

void SensorDriverDigitecoPower::get(int32_t *values, uint8_t length) {
  static float battery_charge;
  static float battery_voltage;
  static float battery_current;
  static float input_voltage;
  static float input_current;
  static float output_voltage;

  switch (_get_state) {
    case INIT:
    memset(values, UINT8_MAX, length * sizeof(int32_t));

    _is_readed = false;
    _is_end = false;

    if (*_is_prepared && length >= 1) {
      _is_success = true;
      _get_state = SET_BATTERY_CHARGE_ADDRESS;
    }
    else {
      _is_success = false;
      _get_state = END;
    }

    _delay_ms = 0;
    _start_time_ms = millis();
    break;

    case SET_BATTERY_CHARGE_ADDRESS:
    _is_success = DigitecoPower::de_send(_address, DIGITECO_POWER_BATTERY_CHARGE_ADDRESS);
    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success) {
      _get_state = READ_BATTERY_CHARGE;
    }
    else {
      _get_state = END;
    }
    break;

    case READ_BATTERY_CHARGE:
    _is_success = DigitecoPower::de_read(_address, &battery_charge);
    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success && length >= 2) {
      _get_state = SET_BATTERY_VOLTAGE_ADDRESS;
    }
    else {
      _get_state = END;
    }
    break;

    case SET_BATTERY_VOLTAGE_ADDRESS:
    _is_success = DigitecoPower::de_send(_address, DIGITECO_POWER_BATTERY_VOLTAGE_ADDRESS);
    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success) {
      _get_state = READ_BATTERY_VOLTAGE;
    }
    else {
      _get_state = END;
    }
    break;

    case READ_BATTERY_VOLTAGE:
    _is_success = DigitecoPower::de_read(_address, &battery_voltage);
    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success && length >= 3) {
      _get_state = SET_BATTERY_CURRENT_ADDRESS;
    }
    else {
      _get_state = END;
    }
    break;

    case SET_BATTERY_CURRENT_ADDRESS:
    _is_success = DigitecoPower::de_send(_address, DIGITECO_POWER_BATTERY_CURRENT_ADDRESS);
    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success) {
      _get_state = READ_BATTERY_CURRENT;
    }
    else {
      _get_state = END;
    }
    break;

    case READ_BATTERY_CURRENT:
    _is_success = DigitecoPower::de_read(_address, &battery_current);
    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success && length >= 4) {
      _get_state = SET_INPUT_VOLTAGE_ADDRESS;
    }
    else {
      _get_state = END;
    }
    break;

    case SET_INPUT_VOLTAGE_ADDRESS:
    _is_success = DigitecoPower::de_send(_address, DIGITECO_POWER_INPUT_VOLTAGE_ADDRESS);
    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success) {
      _get_state = READ_INPUT_VOLTAGE;
    }
    else {
      _get_state = END;
    }
    break;

    case READ_INPUT_VOLTAGE:
    _is_success = DigitecoPower::de_read(_address, &input_voltage);
    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success && length >= 5) {
      _get_state = SET_INPUT_CURRENT_ADDRESS;
    }
    else {
      _get_state = END;
    }
    break;

    case SET_INPUT_CURRENT_ADDRESS:
    _is_success = DigitecoPower::de_send(_address, DIGITECO_POWER_INPUT_CURRENT_ADDRESS);
    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success) {
      _get_state = READ_INPUT_CURRENT;
    }
    else {
      _get_state = END;
    }
    break;

    case READ_INPUT_CURRENT:
    _is_success = DigitecoPower::de_read(_address, &input_current);
    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success && length >= 6) {
      _get_state = SET_OUTPUT_VOLTAGE_ADDRESS;
    }
    else {
      _get_state = END;
    }
    break;

    case SET_OUTPUT_VOLTAGE_ADDRESS:
    _is_success = DigitecoPower::de_send(_address, DIGITECO_POWER_OUTPUT_VOLTAGE_ADDRESS);
    _delay_ms = 0;
    _start_time_ms = millis();

    if (_is_success) {
      _get_state = READ_OUTPUT_VOLTAGE;
    }
    else {
      _get_state = END;
    }
    break;

    case READ_OUTPUT_VOLTAGE:
    _is_success = DigitecoPower::de_read(_address, &output_voltage);
    _delay_ms = 0;
    _start_time_ms = millis();
    _get_state = END;
    break;

    case END:
    if (length >= 1) {
      if (_is_success) {
        values[0] = battery_charge;
      }
      else {
        _is_success = false;
      }
    }

    if (length >= 2) {
      if (_is_success) {
        values[1] = battery_voltage * 10;
      }
      else {
        _is_success = false;
      }
    }

    if (length >= 3) {
      if (_is_success) {
        values[2] = battery_current;
      }
      else {
        _is_success = false;
      }
    }

    if (length >= 4) {
      if (_is_success) {
        values[3] = input_voltage * 10;
      }
      else {
        _is_success = false;
      }
    }

    if (length >= 5) {
      if (_is_success) {
        values[4] = input_current * 1000;
      }
      else {
        _is_success = false;
      }
    }

    if (length >= 6) {
      if (_is_success) {
        values[5] = output_voltage * 10;
      }
      else {
        _is_success = false;
      }
    }

    SensorDriver::printInfo(_driver, _type, _address, _node);
    SERIAL_DEBUG(F(" get... [ %s ]\r\n"), _is_success ? OK_STRING : FAIL_STRING);

    if (length >= 1) {
      if (isValid(values[0])) {
        SERIAL_DEBUG(F("--> battery charge: %ld %%\r\n"), values[0]);
      }
      else {
        SERIAL_DEBUG(F("--> battery charge: ---\r\n"));
      }
    }

    if (length >= 2) {
      if (isValid(values[1])) {
        SERIAL_DEBUG(F("--> battery voltage: %ld V\r\n"), values[1]);
      }
      else {
        SERIAL_DEBUG(F("--> battery voltage: ---\r\n"));
      }
    }

    if (length >= 3) {
      if (isValid(values[2])) {
        SERIAL_DEBUG(F("--> battery current: %ld mA\r\n"), values[2]);
      }
      else {
        SERIAL_DEBUG(F("--> battery current: ---\r\n"));
      }
    }

    if (length >= 4) {
      if (isValid(values[3])) {
        SERIAL_DEBUG(F("--> input voltage: %ld V\r\n"), values[3]);
      }
      else {
        SERIAL_DEBUG(F("--> input voltage: ---\r\n"));
      }
    }

    if (length >= 5) {
      if (isValid(values[4])) {
        SERIAL_DEBUG(F("--> input current: %ld mA\r\n"), values[4]);
      }
      else {
        SERIAL_DEBUG(F("--> input current: ---\r\n"));
      }
    }

    if (length >= 6) {
      if (isValid(values[5])) {
        SERIAL_DEBUG(F("--> output voltage: %ld V\r\n"), values[5]);
      }
      else {
        SERIAL_DEBUG(F("--> output voltage: ---\r\n"));
      }
    }

    _start_time_ms = millis();
    _delay_ms = 0;
    _is_end = true;
    _is_readed = false;
    _get_state = INIT;
    break;
  }
}

#if (USE_JSON)
void SensorDriverDigitecoPower::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length) {
  SensorDriverDigitecoPower::get(values, length);

  if (_is_end && !_is_readed) {
    StaticJsonBuffer<JSON_BUFFER_LENGTH> buffer;
    JsonObject &json = buffer.createObject();

    if (length >= 1) {
      if (isValid(values[0])) {
        json["B25192"] = values[0];
      }
      else json["B25192"] = RawJson("null");
    }

    if (length >= 2) {
      if (isValid(values[1])) {
        json["B25025"] = values[1];
      }
      else json["B25025"] = RawJson("null");
    }

    if (length >= 3) {
      if (isValid(values[2])) {
        json["B25193"] = values[2];
      }
      else json["B25193"] = RawJson("null");
    }

    if (length >= 4) {
      if (isValid(values[3])) {
        json["B00004"] = values[3];
      }
      else json["B00004"] = RawJson("null");
    }

    if (length >= 5) {
      if (isValid(values[4])) {
        json["B00005"] = values[4];
      }
      else json["B00005"] = RawJson("null");
    }

    if (length >= 6) {
      if (isValid(values[5])) {
        json["B00006"] = values[5];
      }
      else json["B00006"] = RawJson("null");
    }

    json.printTo(json_buffer, json_buffer_length);
  }
}
#endif

#endif
