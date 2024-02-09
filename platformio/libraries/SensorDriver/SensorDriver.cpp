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

#include "SensorDriver.h"

namespace _SensorDriver {
  static bool _is_setted_pool[SENSORS_UNIQUE_MAX]= {false};
  static bool _is_prepared_pool[SENSORS_UNIQUE_MAX]= {false};
  static uint8_t _pool_new_pointer=0;
  static uint8_t _pool_pointers[SENSORS_MAX];
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
    LOGE(F("SensorDriver %s-%s create... [ %s ]--> driver or type is null."), driver, type, FAIL_STRING);
    return NULL;
  }

  #if (USE_SENSOR_ADT)
  else if (strcmp(type, SENSOR_TYPE_ADT) == 0)
  return new SensorDriverAdt7420(driver, type);
  #endif

  #if (USE_SENSOR_HIH)
  else if (strcmp(type, SENSOR_TYPE_HIH) == 0)
  return new SensorDriverHih6100(driver, type);
  #endif

  #if (USE_SENSOR_HYT)
  else if (strcmp(type, SENSOR_TYPE_HYT) == 0)
  return new SensorDriverHyt2X1(driver, type);
  #endif

  #if (USE_SENSOR_SHT)
  else if (strcmp(type, SENSOR_TYPE_SHT) == 0)
  return new SensorDriverSht(driver, type);
  #endif

  #if (USE_SENSOR_SPS)
  else if (strcmp(type, SENSOR_TYPE_SPS) == 0)
  return new SensorDriverSps(driver, type);
  #endif

  #if (USE_SENSOR_SCD)
  else if (strcmp(type, SENSOR_TYPE_SCD) == 0)
  return new SensorDriverScd(driver, type);
  #endif
  
  #if (USE_SENSOR_DW1)
  else if (strcmp(type, SENSOR_TYPE_DW1) == 0)
  return new SensorDriverDw1(driver, type);
  #endif

  #if (USE_SENSOR_TBR)
  else if (strcmp(type, SENSOR_TYPE_TBR) == 0)
  return new SensorDriverRain(driver, type);
  #endif

  #if (USE_SENSOR_STH || USE_SENSOR_ITH || USE_SENSOR_MTH || USE_SENSOR_NTH || USE_SENSOR_XTH)
  else if (strcmp(type, SENSOR_TYPE_STH) == 0 || strcmp(type, SENSOR_TYPE_ITH) == 0 || strcmp(type, SENSOR_TYPE_MTH) == 0 || strcmp(type, SENSOR_TYPE_NTH) == 0 || strcmp(type, SENSOR_TYPE_XTH) == 0)
  return new SensorDriverTh(driver, type);
  #endif

  #if (USE_SENSOR_DEP)
  else if (strcmp(type, SENSOR_TYPE_DEP) == 0)
  return new SensorDriverDigitecoPower(driver, type);
  #endif

  #if (USE_SENSOR_POW)
  else if (strcmp(type, SENSOR_TYPE_POW) == 0)
  return new SensorDriverPower(driver, type);
  #endif
  
  #if (USE_SENSOR_DWA || USE_SENSOR_DWB || USE_SENSOR_DWC || USE_SENSOR_DWD || USE_SENSOR_DWE || USE_SENSOR_DWF)
  else if ((strcmp(type, SENSOR_TYPE_DWA) == 0) || (strcmp(type, SENSOR_TYPE_DWB) == 0) || (strcmp(type, SENSOR_TYPE_DWC) == 0) || (strcmp(type, SENSOR_TYPE_DWD) == 0) || (strcmp(type, SENSOR_TYPE_DWE) == 0) || (strcmp(type, SENSOR_TYPE_DWF) == 0))
  return new SensorDriverWind(driver, type);
  #endif

  #if (USE_SENSOR_DSA)
  else if (strcmp(type, SENSOR_TYPE_DSA) == 0)
  return new SensorDriverSolarRadiation(driver, type);
  #endif

  #if (USE_SENSOR_OA2 || USE_SENSOR_OB2 || USE_SENSOR_OC2 || USE_SENSOR_OD2 || USE_SENSOR_OA3 || USE_SENSOR_OB3 || USE_SENSOR_OC3 || USE_SENSOR_OD3 || USE_SENSOR_OE3)
  else if ((strcmp(type, SENSOR_TYPE_OA2) == 0) || (strcmp(type, SENSOR_TYPE_OB2) == 0) || (strcmp(type, SENSOR_TYPE_OC2) == 0) || (strcmp(type, SENSOR_TYPE_OD2) == 0) || (strcmp(type, SENSOR_TYPE_OA3) == 0) || (strcmp(type, SENSOR_TYPE_OB3) == 0) || (strcmp(type, SENSOR_TYPE_OC3) == 0) || (strcmp(type, SENSOR_TYPE_OD3) == 0) || (strcmp(type, SENSOR_TYPE_OE3) == 0))
  return new SensorDriverOpc(driver, type);
  #endif

  #if (USE_SENSOR_LWT)
  else if (strcmp(type, SENSOR_TYPE_LWT) == 0)
  return new SensorDriverLeaf(driver, type);
  #endif

  else {
    LOGE(F("SensorDriver %s-%s create... [ FAIL ]--> driver or type not found."), driver, type);
    return NULL;
  }
}

void SensorDriver::init(const uint8_t address, const uint8_t node, bool& is_setted, bool& is_prepared) {
  _address = address;
  _node = node;
  _start_time_ms = 0;
  _is_setted = &is_setted;
  _is_previous_prepared = false;  
  _is_prepared = &is_prepared;
  _error_count = 0;
}


void SensorDriver::setup(){
}

void SensorDriver::prepare(bool is_test){
}

void SensorDriver::get(int32_t *values, uint8_t length, bool is_test){
}

#if (USE_JSON)
void SensorDriver::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test){
}
#endif

void SensorDriver::resetPrepared(bool is_test){
}

void SensorDriver::resetSetted(){
  *_is_setted = false;
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

bool SensorDriver::isSetted() {
  return *_is_setted;
}

bool SensorDriver::isPrepared() {
  return *_is_prepared;
}

uint16_t SensorDriver::getErrorCount() {
  return _error_count;
}

void SensorDriver::createAndSetup(const char* driver, const char* type, const uint8_t address, const uint8_t node, SensorDriver *sensors[], uint8_t& sensors_count) {

  uint8_t index;  
  bool found = false;

  if (sensors_count >= SENSORS_MAX) return;

  for (uint8_t i = 0; i < sensors_count; i++) {
    if (
	strcmp(sensors[i]->getDriver(),driver) == 0
	//&&
	//strcmp(sensors[i]->getType(),type)  == 0    // the type is different in the same poll
	&&
	sensors[i]->getAddress() == address
	)
      {
	index=_SensorDriver::_pool_pointers[i];
	LOGT(F("found pool index: %d"),index);
	found=true;
	break;
      }
  }

  if (!found){    
    if (_SensorDriver::_pool_new_pointer >= SENSORS_UNIQUE_MAX){
      LOGE(F("pool index: %d out of scope"),_SensorDriver::_pool_new_pointer);
      sensors[sensors_count] = NULL;
      sensors_count++;
      return;
    }

    LOGT(F("new pool index: %d"),_SensorDriver::_pool_new_pointer);
    index=_SensorDriver::_pool_new_pointer;
    _SensorDriver::_pool_pointers[sensors_count]=index;
    _SensorDriver::_pool_new_pointer++;
  }

  sensors[sensors_count] = SensorDriver::create(driver, type);
  if (sensors[sensors_count]) {
    sensors[sensors_count]->init(address, node, _SensorDriver::_is_setted_pool[index], _SensorDriver::_is_prepared_pool[index]);
    sensors[sensors_count]->setup();
    (sensors_count)++;
  }
}

void SensorDriver::printInfo() {
  //LOGV(F("SensorDriver %s-%s 0x%x (%d) on node %d"), _driver, _type, _address, _address, _node);
  LOGV(F("SensorDriver %s-%s 0x%x (%d) on node %d %T %T"), SensorDriver::getDriver(), SensorDriver::getType(),
       SensorDriver::getAddress(), SensorDriver::getAddress(), SensorDriver::getNode(),
       SensorDriver::isSetted(),SensorDriver::isPrepared());
}

//------------------------------------------------------------------------------
// ADT7420
//------------------------------------------------------------------------------
#if (USE_SENSOR_ADT)

void SensorDriverAdt7420::resetPrepared(bool is_test) {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverAdt7420::setup() {
  SensorDriver::printInfo();

  _delay_ms = 0;

  if (!*_is_setted) {
    Wire.beginTransmission(_address);
    Wire.write(0x03); // Set the register pointer to (0x01)
    Wire.write(0x20); // Set resolution and one shot

    if (Wire.endTransmission()) {
      LOGE(F("adt7420 setup... [ %s ]"), FAIL_STRING);
      _error_count++;
      return;
    }
    LOGT(F("adt7420 setup... [ %s ]"), OK_STRING);
    *_is_setted = true;
    _error_count = 0;
  }
  else {
    LOGT(F("adt7420 setup... [ %s ]"), YES_STRING);
  }

}

void SensorDriverAdt7420::prepare(bool is_test) {
  SensorDriver::printInfo();

  if (!*_is_prepared) {
    Wire.beginTransmission(_address);
    Wire.write(0x03); // Set the register pointer to (0x01)
    Wire.write(0x20); // Set resolution and one shot

    if (Wire.endTransmission()) {
      LOGE(F("adt7420 prepare... [ %s ]"), FAIL_STRING);
      _error_count++;
      return;
    }

    *_is_prepared = true;
    _delay_ms = 250;
    _error_count = 0;

    LOGT(F("adt7420 prepare... [ %s ]"), OK_STRING);
  }
  else {
    LOGT(F("adt7420 prepare... [ %s ]"), YES_STRING);
    _delay_ms = 0;
  }

  _start_time_ms = millis();
}

void SensorDriverAdt7420::get(int32_t *values, uint8_t length, bool is_test) {
  uint8_t msb;
  uint8_t lsb;

  switch (_get_state) {
    case INIT:
    temperature = 0;
    for (uint8_t i =0; i < length; i++) {
      values[i]=INT32_MAX;
    }

    _is_success = true;
    _is_readed = false;
    _is_end = false;

    if (*_is_prepared && length >= 1) {
      Wire.beginTransmission(_address);
      Wire.write(0x00); // Set the register pointer to (0x00)

      if (Wire.endTransmission()) {
	_error_count++;
        _is_success = false;
      }
    }
    else {
      _error_count = 0;
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
      _error_count++;
      _is_success = false;
    }
    else {
      msb = Wire.read();
      lsb = Wire.read();

      if ((msb == 255) && (lsb == 255)) {
	_error_count++;
        _is_success = false;
      }
      else {
	_error_count = 0;
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
        values[0] = INT32_MAX;
      }
    }

    SensorDriver::printInfo();
    if (_is_success){
      LOGT(F("adt7420 get... [ %s ]"), OK_STRING);
    }else{
      LOGE(F("adt7420 get... [ %s ]"), FAIL_STRING);
    }
    
    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        LOGT(F("adt7420--> temperature: %d"), values[0]);
      }
      else {
        LOGT(F("adt7420--> temperature: ---"));
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
void SensorDriverAdt7420::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverAdt7420::get(values, length, is_test);
  json_buffer[0]='\0';

  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;
    
    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        json["B12101"] = values[0];
      }
      else json["B12101"] = nullptr;
    }
    
    if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
      json_buffer[0]='\0';
    }
  }
}
#endif

#endif

//------------------------------------------------------------------------------
// HIH6100
//------------------------------------------------------------------------------
#if (USE_SENSOR_HIH)

void SensorDriverHih6100::resetPrepared(bool is_test) {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverHih6100::setup() {
  SensorDriver::printInfo();


  _delay_ms = 0;

  if (!*_is_setted) {
  
    Wire.beginTransmission(_address);
    
    if (Wire.endTransmission() == 0) {
      *_is_setted = true;
      _error_count = 0;
      LOGT(F("hih6100 setup... [ %s ]"), OK_STRING);
    }else{
      _error_count++;
      LOGE(F("hih6100 setup... [ %s ]"), ERROR_STRING);
    }
  }else {
    LOGT(F("hih6100 setup... [ %s ]"), YES_STRING);
  } 
}

void SensorDriverHih6100::prepare(bool is_test) {
  SensorDriver::printInfo();

  if (!*_is_prepared) {
    Wire.beginTransmission(_address);

    if (Wire.endTransmission()) {
      _error_count++;
      LOGE(F("hih6100 prepare... [ %s ]"), FAIL_STRING);
      return;
    }

    *_is_prepared = true;
    _delay_ms = 40;
    _error_count = 0;

    LOGT(F("hih6100 prepare... [ %s ]"), OK_STRING);
  }
  else {
    LOGT(F("hih6100 prepare... [ %s ]"), YES_STRING);
    _delay_ms = 0;
  }

  _start_time_ms = millis();
}

void SensorDriverHih6100::get(int32_t *values, uint8_t length, bool is_test) {
  uint8_t x;
  uint8_t y;
  uint8_t s;

  switch (_get_state) {
    case INIT:
    temperature = 0;
    humidity = 0;
    for (uint8_t i =0; i < length; i++) {
      values[i]=INT32_MAX;
    }

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
    Wire.requestFrom(_address, (uint8_t)  4);

    if (Wire.available() < 4) {
      _error_count++;
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

    // mmm what is this ?
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
        values[0] = round((float(temperature) / 16382. * 165. - 40.) * 100) + SENSOR_DRIVER_C_TO_K;
      }
      else {
        values[0] = INT32_MAX;
      }
    }

    if (length >= 2) {
      if (_is_success) {
        values[1] = round(float(humidity) / 16382. * 100);
      }
      else {
        values[1] = INT32_MAX;
      }
    }

    SensorDriver::printInfo();
    if (_is_success){
      LOGT(F("hih6100 get... [ %s ]"), OK_STRING);
    }else{
      LOGE(F("hih6100 get... [ %s ]"), FAIL_STRING);
    }

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        LOGT(F("hih6100--> temperature: %d"), values[0]);
      }
      else {
        LOGT(F("hih6100--> temperature: ---"));
      }
    }

    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
        LOGT(F("hih6100--> humidity: %d"), values[1]);
      }
      else {
        LOGT(F("hih6100--> humidity: ---"));
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
void SensorDriverHih6100::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverHih6100::get(values, length, is_test);
  json_buffer[0]='\0';

  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        json["B12101"] = values[0];
      }
      else json["B12101"] = nullptr;
    }

    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
        json["B13003"] = values[1];
      }
      else json["B13003"] = nullptr;
    }

    if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
      json_buffer[0]='\0';
    }
  }
}
#endif

#endif

//------------------------------------------------------------------------------
// HYT2X1
//------------------------------------------------------------------------------
#if (USE_SENSOR_HYT)

void SensorDriverHyt2X1::resetPrepared(bool is_test) {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverHyt2X1::setup() {
  SensorDriver::printInfo();

  _delay_ms = 0;

  if (!*_is_setted) {
    Wire.beginTransmission(_address);

    if (Wire.endTransmission() == 0) {
      *_is_setted = true;
      _error_count = 0;
      LOGT(F("hyt2x1 setup... [ %s ]"), OK_STRING);
    }else{
      _error_count++;
      LOGE(F("hyt2x1 setup... [ %s ]"), ERROR_STRING);
    }
  }else{
    LOGT(F("hyt2x1 setup... [ %s ]"), YES_STRING);
  } 
}

void SensorDriverHyt2X1::prepare(bool is_test) {
  SensorDriver::printInfo();
  *_is_prepared = Hyt2X1::hyt_initRead(_address);
  if (*_is_prepared){
    _error_count = 0;
    LOGT(F("hyt2x1 prepare... [ %s ]"), OK_STRING);
  }else{
    _error_count++;    
    LOGE(F("hyt2x1 prepare... [ %s ]"), FAIL_STRING);
  }

  _delay_ms = HYT2X1_CONVERSION_TIME_MS;
  _start_time_ms = millis();
}

void SensorDriverHyt2X1::get(int32_t *values, uint8_t length, bool is_test) {
  uint8_t status = HYT2X1_ERROR;

  switch (_get_state) {
    case INIT:
      humidity = FLT_MAX;
      temperature = FLT_MAX;
      humidity_confirmation = FLT_MAX;
      temperature_confirmation = FLT_MAX;
      for (uint8_t i =0; i < length; i++) {
	values[i]=INT32_MAX;
      }

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
      status = Hyt2X1::hyt_read(_address, &humidity, &temperature);
      if (status == HYT2X1_SUCCESS) {
        _is_success = true;
	_error_count = 0;
        _get_state = READ_CONFIRMATION;
      }
      else {
	LOGE(F("hyt2x1 get read error"));
	_error_count++;
        _is_success = false;
        _get_state = END;
      }
      _delay_ms = 0;
      _start_time_ms = millis();
    break;

    case READ_CONFIRMATION:
      status = Hyt2X1::hyt_read(_address, &humidity_confirmation, &temperature_confirmation);
      if ((status == HYT2X1_SUCCESS) || (status == HYT2X1_NO_NEW_DATA)) {
        // max 1% variation
        if ((abs(humidity - humidity_confirmation) <= 1.0) && (abs(temperature - temperature_confirmation) <= 0.5)) {
	  _error_count = 0;
          _is_success = true;
        }
        else {
	  LOGE(F("hyt2x1 get NO confirmation by values %D/%D %D/%D"),humidity,humidity_confirmation,temperature,temperature_confirmation );
	  _error_count++;
          _is_success = false;
        }
      }
      else {
	LOGE(F("hyt2x1 get NO confirmation by read error: %d"),status);
        _is_success = false;
      }
      _delay_ms = 0;
      _start_time_ms = millis();
      _get_state = END;
    break;

    case END:
      if (length >= 1) {
        if (_is_success  && ISVALID_FLOAT(temperature)) {
          values[0] = SENSOR_DRIVER_C_TO_K + (int32_t)(temperature * 100.0);
        }
        else {
          values[0] = INT32_MAX;
        }
      }

      if (length >= 2) {
	if (_is_success && ISVALID_FLOAT(humidity)) {
          values[1] = round(humidity);
        }
        else {
          values[1] = INT32_MAX;
        }
      }

      SensorDriver::printInfo();
      if (_is_success){
	LOGT(F("hyt2x1 get... [ %s ]"), OK_STRING);
      }else{
	LOGE(F("hyt2x1 get... [ %s ]"), FAIL_STRING);
      }

      if (length >= 1) {
        if (ISVALID_INT32(values[0])) {
          LOGT(F("hyt2x1--> temperature: %d"), values[0]);
        }
        else {
          LOGT(F("hyt2x1--> temperature: ---"));
        }
      }

      if (length >= 2) {
        if (ISVALID_INT32(values[1])) {
          LOGT(F("hyt2x1--> humidity: %d"), values[1]);
        }
        else {
          LOGT(F("hyt2x1--> humidity: ---"));
        }
      }

      _delay_ms = 0;
      _start_time_ms = millis();
      _is_end = true;
      _is_readed = false;
      _get_state = INIT;
    break;
  }
}

#if (USE_JSON)
void SensorDriverHyt2X1::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverHyt2X1::get(values, length, is_test);
  json_buffer[0]='\0';

  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        json["B12101"] = values[0];
      }
      else json["B12101"] = nullptr;
    }

    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
        json["B13003"] = values[1];
      }
      else json["B13003"] = nullptr;
    }

    if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
      json_buffer[0]='\0';
    }
  }
}
#endif

#endif


//------------------------------------------------------------------------------
// Sensor driver's SHT sensor type for Sensirion humidity and temperature sensor support
//------------------------------------------------------------------------------
#if (USE_SENSOR_SHT)
SHTI2cSensor _sht;

void SensorDriverSht::resetPrepared(bool is_test) {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverSht::setup() {
  SensorDriver::printInfo();

  _delay_ms = 0;

  if (!*_is_setted) {

    //  WARNING !!!!! address is not used; sensirion have only one address
    if (_address == 0x44) {
      _sht.softReset();
      delay(10);
    } else {
      _error_count++;
      LOGE(F("sht setup... wrong i2c address [ %s ]"), ERROR_STRING);
      return;
    }

    if(_sht.clearStatusRegister() == true) {  //Start continuous measurements      
      
      *_is_setted = true;
      _error_count = 0;
      LOGT(F("sht setup... [ %s ]"), OK_STRING);
    } else {
      _error_count++;
      LOGE(F("sht setup... [ %s ]"), ERROR_STRING);
    }
  }else{
    LOGT(F("sht setup... [ %s ]"), YES_STRING);
  } 
}

void SensorDriverSht::prepare(bool is_test) {
  SensorDriver::printInfo();
  
  *_is_prepared = _sht.singleShotDataAcquisition();
  if (*_is_prepared){
    //_error_count = 0;          // prepare ok is not enough to say sensor is in a good state
    LOGT(F("SHT prepare... [ %s ]"), OK_STRING);
  }else{
    _error_count++;    
    LOGE(F("SHT prepare... [ %s ]"), FAIL_STRING);
  }

  _delay_ms = _sht.mDuration;
  _start_time_ms = millis();
}

void SensorDriverSht::get(int32_t *values, uint8_t length, bool is_test) {
  
  switch (_get_state) {
  case INIT:
    for (uint8_t i =0; i < length; i++) {
      values[i]=INT32_MAX;
    }
    
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
    
    if (_sht.getValues() && _sht.checkStatus()){
      _is_success = true;
      _error_count = 0;
      _get_state = END;
    } else {
      LOGE(F("sht get read error"));
      _error_count++;
      _is_success = false;
      _get_state = END;
    }
    _delay_ms = 0;
    _start_time_ms = millis();
    break;
    
  case END:
    
    if (_is_success ) {
      if (length >= 1)  values[0] = (int32_t) round(_sht.getTemperature() * 100.) + SENSOR_DRIVER_C_TO_K ;
      if (length >= 2)  values[1] = (int32_t) round (_sht.getHumidity()) ;
    }
  
    SensorDriver::printInfo();
    if (_is_success){
      LOGT(F("sht get... [ %s ]"), OK_STRING);
    }else{
      LOGE(F("sht get... [ %s ]"), FAIL_STRING);
    }
    
    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
	LOGT(F("sht--> humidity: %d"), values[0]);
      }
      else {
	LOGT(F("sht--> humidity: ---"));
      }
    }
    
    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
	LOGT(F("sht--> temperature: %d"), values[1]);
      }
      else {
	LOGT(F("sht--> temperature: ---"));
      }
    }
    
    _delay_ms = 0;
    _start_time_ms = millis();
    _is_end = true;
    _is_readed = false;
    _get_state = INIT;
    break;
  }
}

#if (USE_JSON)
void SensorDriverSht::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverSht::get(values, length, is_test);
  json_buffer[0]='\0';

  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        json["B12101"] = values[0];
      }
      else json["B12101"] = nullptr;
    }

    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
        json["B13003"] = values[1];
      }
      else json["B13003"] = nullptr;
    }

    if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
      json_buffer[0]='\0';
    }
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

void SensorDriverDw1::resetPrepared(bool is_test) {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverDw1::setup() {
  SensorDriver::printInfo();

  _delay_ms = 0;

  Wire.beginTransmission(_address);

  if (Wire.endTransmission() == 0) {
    _error_count = 0;
    *_is_setted = true;
    LOGT(F("dw1 setup... [ %s ]"), OK_STRING);
  }else{
    _error_count++;
    LOGE(F("dw1 setup... [ %s ]"), ERROR_STRING);
  }
}

void SensorDriverDw1::prepare(bool is_test) {
  SensorDriver::printInfo();

  if (!*_is_prepared) {
    Wire.beginTransmission(_address);
    Wire.write(I2C_WINDSONIC_COMMAND);
    Wire.write(I2C_WINDSONIC_COMMAND_STOP);
    _delay_ms = 3000;

    if (Wire.endTransmission()) {
      _error_count++;
      LOGT(F("dw1 prepare... [ %s ]"), FAIL_STRING);
      return;
    }

    *_is_prepared = true;
    _error_count = 0;

    LOGT(F("dw1 prepare... [ %s ]"), OK_STRING);
  }
  else {
    LOGT(F("dw1 prepare... [ %s ]"), YES_STRING);
    _delay_ms = 0;
  }

  _start_time_ms = millis();
}

void SensorDriverDw1::get(int32_t *values, uint8_t length, bool is_test) {
  speed = INT32_MAX;
  direction = INT32_MAX;
  uint16_t msb;
  uint16_t lsb;

  switch (_get_state) {
    case INIT:
      for (uint8_t i =0; i < length; i++) {
	values[i]=INT32_MAX;
      }

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
      _error_count = 0;
      _get_state = READ_MEANU;
    }
    else {
      _error_count++;
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

      if (ISVALID_UINT8(lsb) || ISVALID_UINT8(msb)) {
	values[0] = ((int) (msb << 8) | lsb);
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
      _error_count = 0;
      _get_state = READ_MEANV;
    }
    else {
      _error_count++;
      _get_state = END;
    }
    break;

    case READ_MEANV:
    Wire.requestFrom(_address, (uint8_t) 2);

    if (Wire.available() < 2) {
      _error_count++;
      _is_success = false;
    }

    if (_is_success) {
      _error_count = 0;
      lsb = Wire.read();
      msb = Wire.read();

      if (ISVALID_UINT8(lsb) || ISVALID_UINT8(msb)) {
	values[1] = ((uint16_t) (msb << 8) | lsb);
        values[1] -= OFFSET;
      }
    }

    _delay_ms = 0;
    _start_time_ms = millis();

    _get_state = ELABORATE;
    break;

    case ELABORATE:
    if (ISVALID(values[0]) && ISVALID(values[1]) && length >= 2) {
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
    SensorDriver::printInfo();
    if (_is_success){
      LOGT(F("dw1 get... [ %s ]"), OK_STRING);
    }else{
      LOGE(F("dw1 get... [ %s ]"), FAIL_STRING);
    }

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        LOGT(F("dw1--> mean u: %d"), values[0]);
      }
      else {
        LOGT(F("dw1--> mean u: ---"));
      }
    }

    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
        LOGT(F("dw1--> mean v: %d"), values[1]);
      }
      else {
        LOGT(F("dw1--> mean v: ---"));
      }
    }

    if (ISVALID_INT32(values[0]) && ISVALID_INT32(values[1]) && length >= 2) {
      values[0] = (int32_t) direction;
      values[1] = (int32_t) round(speed);
      LOGT(F("dw1--> direction: %d"), values[0]);
      LOGT(F("dw1--> speed: %d"), values[1]);
    } else {
      LOGT(F("dw1--> direction: ---"));
      LOGT(F("dw1--> speed: ---"));
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
void SensorDriverDw1::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverDw1::get(values, length, is_test);
  json_buffer[0]='\0';

  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        json["B11001"] = values[0];
      }
      else json["B11001"] = nullptr;
    }

    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
        json["B11002"] = values[1];
      }
      else json["B11002"] = nullptr;
    }

    if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
      json_buffer[0]='\0';
    }
  }
}
#endif

#endif

//------------------------------------------------------------------------------
// I2C-Rain
// TBR: oneshot
//------------------------------------------------------------------------------

#if (USE_SENSOR_TBR)

void SensorDriverRain::resetPrepared(bool is_test) {


  _get_state = INIT;
  if (!is_test) {
    _is_previous_prepared = _is_current_prepared;
    _is_current_prepared = false;
  }
  *_is_prepared = false;
}

void SensorDriverRain::setup() {
  SensorDriver::printInfo();

  _delay_ms = 0;

  Wire.beginTransmission(_address);

  if (Wire.endTransmission() == 0) {
    _error_count = 0;
    *_is_setted = true;
    LOGT(F("rain setup... [ %s ]"), OK_STRING);
  }else{
    _error_count++;
    LOGE(F("rain setup... [ %s ]"), ERROR_STRING);
  }
}

void SensorDriverRain::prepare(bool is_test) {
  SensorDriver::printInfo();
  bool is_i2c_write;
  uint8_t i;

  if (!*_is_prepared) {
    memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
    is_i2c_write = false;
    i = 0;

    is_i2c_write = true;
    _buffer[i++] = I2C_COMMAND_ID;
    
    if (is_test) {
      _buffer[i++] = I2C_RAIN_COMMAND_TEST_READ;
    }
    else {
      _buffer[i++] = I2C_RAIN_COMMAND_ONESHOT_START_STOP;
    }
    _buffer[i] = crc8(_buffer, i);
    _delay_ms = 10;

    if (is_i2c_write) {
      Wire.beginTransmission(_address);
      Wire.write(_buffer, i+1);

      if (Wire.endTransmission() == 0) {
	_error_count = 0;
        _is_success = true;
        *_is_prepared = true;
      }else{
	_error_count++;
	_is_success = false;
      }
    }
  }
  else {
    _is_success = true;
    _delay_ms = 0;
  }

  if(!is_test)_is_current_prepared = *_is_prepared;
  LOGT(F(" prepare... [ %s ]"), _is_success ? OK_STRING : ERROR_STRING);

  _start_time_ms = millis();
}

void SensorDriverRain::get(int32_t *values, uint8_t length, bool is_test) {
  uint8_t data_length;
  bool is_i2c_write;
  uint8_t i;

  switch (_get_state) {
    case INIT:
      for (uint8_t i =0; i < length; i++) {
	values[i]=INT32_MAX;
      }
    memset(rain_data, UINT8_MAX, I2C_RAIN_LENGTH);

    _is_readed = false;
    _is_end = false;

    if ( *_is_prepared && length >= 1) {
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
      memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
      is_i2c_write = false;
      i = 0;

      is_i2c_write = true;
      _buffer[i++] = I2C_RAIN_ADDRESS;
      _buffer[i++] = I2C_RAIN_LENGTH;
      _buffer[i] = crc8(_buffer, i);

      if (is_i2c_write) {
        Wire.beginTransmission(_address);
        Wire.write(_buffer, i+1);

        if (Wire.endTransmission()) {
	  _error_count++;
          _is_success = false;
        }else{
	  _error_count = 0;
	  _is_success = true;
	}
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
      data_length = I2C_RAIN_LENGTH;

      if (_is_success) {
        Wire.requestFrom(_address, (uint8_t)(data_length + 1));
        if (Wire.available() < (data_length + 1)) {
          _is_success = false;
	  _error_count++;
	}else{
	  _error_count = 0;
	  _is_success = true;	  
        }
      }

      if (_is_success) {
        for (i = 0; i < data_length; i++) {
          rain_data[i] = Wire.read();
        }

        if (crc8(rain_data, data_length) != Wire.read()) {
          _is_success = false;
        }
      }

    _start_time_ms = millis();
    _delay_ms = 0;
    _is_end = false;
    _get_state = END;

    break;

    case END:

    if (((_is_previous_prepared && !is_test) || (_is_current_prepared && is_test))){
      if (length >= 1) {
	if ( ISVALID_UINT8(rain_data[0]) || ISVALID_UINT8(rain_data[1])){
	  values[0] = (uint16_t)(rain_data[1] << 8) | (rain_data[0]);
	}
      }
    } else {
      LOGE(F("rain driver status error -> previous:%T current:%T"),_is_previous_prepared,_is_current_prepared );
    }

    SensorDriver::printInfo();
    if (_is_success){
      LOGT(F("rain get... [ %s ]"), OK_STRING);
    }else{
      LOGE(F("rain get... [ %s ]"), FAIL_STRING);
    }

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        LOGT(F("rain--> rain : %d"), values[0]);
      }
      else {
        LOGT(F("rain--> rain: ---"));
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
void SensorDriverRain::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverRain::get(values, length, is_test);
  json_buffer[0]='\0';

  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        json["B13011"] = values[0];
      }
      else json["B13011"] = nullptr;
    }

    if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
      json_buffer[0]='\0';
    }
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

void SensorDriverTh::resetPrepared(bool is_test) {
  _get_state = INIT;
  if (!is_test) {
    _is_previous_prepared = _is_current_prepared;
    _is_current_prepared = false;
  }
  *_is_prepared = false;
}

void SensorDriverTh::setup() {
  SensorDriver::printInfo();
  bool is_i2c_write;
  uint8_t i;
  _delay_ms = 0;
  _is_success = false;

  if (!*_is_setted) {
    memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
    is_i2c_write = false;
    i = 0;

    if (strcmp(_type, SENSOR_TYPE_ITH) == 0 || strcmp(_type, SENSOR_TYPE_MTH) == 0 || strcmp(_type, SENSOR_TYPE_NTH) == 0 || strcmp(_type, SENSOR_TYPE_XTH) == 0) {
      is_i2c_write = true;
      _buffer[i++] = I2C_COMMAND_ID;
      _buffer[i++] = I2C_TH_COMMAND_CONTINUOUS_START;
      _buffer[i] = crc8(_buffer, i);
    }

    if (is_i2c_write) {
      Wire.beginTransmission(_address);
      Wire.write(_buffer, i+1);

      if (Wire.endTransmission() == 0) {
	_error_count = 0;
        _is_success = true;
        *_is_setted = true;
      }else{
	_error_count++;
      }
    }
  }
  else {
    _is_success = true;
  }

  LOGT(F("th setup... [ %s ]"), _is_success ? OK_STRING : ERROR_STRING);
}

void SensorDriverTh::prepare(bool is_test) {
  SensorDriver::printInfo();
  bool is_i2c_write;
  uint8_t i;
  _is_success = false;

  if (!*_is_prepared) {
    memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
    is_i2c_write = false;
    i = 0;

    if (strcmp(_type, SENSOR_TYPE_STH) == 0) {
      is_i2c_write = true;
      _buffer[i++] = I2C_COMMAND_ID;
      _buffer[i++] = I2C_TH_COMMAND_ONESHOT_START_STOP;
      _buffer[i] = crc8(_buffer, i);
      i++;
      _delay_ms = 150;
    }
    else if (strcmp(_type, SENSOR_TYPE_ITH) == 0 || strcmp(_type, SENSOR_TYPE_MTH) == 0 || strcmp(_type, SENSOR_TYPE_NTH) == 0 || strcmp(_type, SENSOR_TYPE_XTH) == 0) {
      is_i2c_write = true;
      _buffer[i++] = I2C_COMMAND_ID;

      if (is_test) {
        _buffer[i++] = I2C_TH_COMMAND_TEST_READ;
      }
      else {
        _buffer[i++] = I2C_TH_COMMAND_CONTINUOUS_START_STOP;
      }
      _buffer[i] = crc8(_buffer, i);
      _delay_ms = 150;
    }

    if (is_i2c_write) {
      Wire.beginTransmission(_address);
      Wire.write(_buffer, i+1);

      if (Wire.endTransmission() == 0) {
	_error_count = 0;
        _is_success = true;
        *_is_prepared = true;
      }else{
	_error_count++;
	LOGE(F("th prepare... endTransmission"));
      }
    }
  }
  else {
    _is_success = true;
    _delay_ms = 0;
  }

  if(!is_test)_is_current_prepared = *_is_prepared;
  LOGT(F("th prepare... [ %s ]"), _is_success ? OK_STRING : ERROR_STRING);

  _start_time_ms = millis();
}

void SensorDriverTh::get(int32_t *values, uint8_t length, bool is_test) {
  uint8_t data_length;
  bool is_i2c_write;
  uint8_t i;

  switch (_get_state) {
    case INIT:

      for (uint8_t i =0; i < length; i++) {
	values[i]=INT32_MAX;
      }
      memset(temperature_data, UINT8_MAX, I2C_TH_TEMPERATURE_DATA_MAX_LENGTH);
      memset(humidity_data, UINT8_MAX, I2C_TH_HUMIDITY_DATA_MAX_LENGTH);

    _is_readed = false;
    _is_end = false;

    if ( *_is_prepared && length >= 1) {
      LOGT(F("th get INIT"));
      _is_success = true;
      _get_state = SET_TEMPERATURE_ADDRESS;
    }
    else {
      LOGE(F("th get INIT"));
      _is_success = false;
      _get_state = END;
    }

    _delay_ms = 0;
    _start_time_ms = millis();
    break;

    case SET_TEMPERATURE_ADDRESS:
      memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
      is_i2c_write = false;
      i = 0;

      if (strcmp(_type, SENSOR_TYPE_STH) == 0) {
        is_i2c_write = true;
        _buffer[i++] = I2C_TH_TEMPERATURE_SAMPLE_ADDRESS;
        _buffer[i++] = I2C_TH_TEMPERATURE_SAMPLE_LENGTH;
        _buffer[i] = crc8(_buffer, i);
      }
      else if (strcmp(_type, SENSOR_TYPE_ITH) == 0) {
        is_i2c_write = true;
        _buffer[i++] = I2C_TH_TEMPERATURE_MED60_ADDRESS;
        _buffer[i++] = I2C_TH_TEMPERATURE_MED60_LENGTH;
        _buffer[i] = crc8(_buffer, i);
      }
      else if (strcmp(_type, SENSOR_TYPE_MTH) == 0) {
        is_i2c_write = true;
        _buffer[i++] = I2C_TH_TEMPERATURE_MED_ADDRESS;
        _buffer[i++] = I2C_TH_TEMPERATURE_MED_LENGTH;
        _buffer[i] = crc8(_buffer, i);
      }
      else if (strcmp(_type, SENSOR_TYPE_NTH) == 0) {
        is_i2c_write = true;
        _buffer[i++] = I2C_TH_TEMPERATURE_MIN_ADDRESS;
        _buffer[i++] = I2C_TH_TEMPERATURE_MIN_LENGTH;
        _buffer[i] = crc8(_buffer, i);
      }
      else if (strcmp(_type, SENSOR_TYPE_XTH) == 0) {
        is_i2c_write = true;
        _buffer[i++] = I2C_TH_TEMPERATURE_MAX_ADDRESS;
        _buffer[i++] = I2C_TH_TEMPERATURE_MAX_LENGTH;
        _buffer[i] = crc8(_buffer, i);
      }
      else{
	LOGE(F("th type mismatch %s for temp write"),_type);
	_is_success = false;
      }

      if (is_i2c_write) {
        Wire.beginTransmission(_address);
        Wire.write(_buffer, i+1);

        if (Wire.endTransmission()) {
	  LOGT(F("th get... ERROR SET_TEMPERATURE_ADDRESS"));
	  _error_count++;
          _is_success = false;
        }else{
	  _error_count = 0;
	}	  
      }

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
    else{
      _is_success = false;
      LOGE(F("th type mismatch %s for temp read"),_type);
    }

      if (_is_success) {
        Wire.requestFrom(_address, (uint8_t)(data_length + 1));
        if (Wire.available() < (data_length + 1)) {
	  LOGT(F("th get... ERROR READ_TEMPERATURE"));
	  _error_count++;
          _is_success = false;
        }else{
	  _error_count = 0;
	}	  
      }

      if (_is_success) {
        for (i = 0; i < data_length; i++) {
          temperature_data[i] = Wire.read();
        }

        if (crc8(temperature_data, data_length) != Wire.read()) {
	  LOGT(F("th get... ERROR READ_TEMPERATURE CRC error"));
          _is_success = false;
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
      memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
      is_i2c_write = false;
      i = 0;

    if (strcmp(_type, SENSOR_TYPE_STH) == 0) {
      is_i2c_write = true;
      _buffer[i++] = I2C_TH_HUMIDITY_SAMPLE_ADDRESS;
      _buffer[i++] = I2C_TH_HUMIDITY_SAMPLE_LENGTH;
      _buffer[i] = crc8(_buffer, i);
    }
    else if (strcmp(_type, SENSOR_TYPE_ITH) == 0) {
      is_i2c_write = true;
      _buffer[i++] = I2C_TH_HUMIDITY_MED60_ADDRESS;
      _buffer[i++] = I2C_TH_HUMIDITY_MED60_LENGTH;
      _buffer[i] = crc8(_buffer, i);
    }
    else if (strcmp(_type, SENSOR_TYPE_MTH) == 0) {
      is_i2c_write = true;
      _buffer[i++] = I2C_TH_HUMIDITY_MED_ADDRESS;
      _buffer[i++] = I2C_TH_HUMIDITY_MED_LENGTH;
      _buffer[i] = crc8(_buffer, i);
    }
    else if (strcmp(_type, SENSOR_TYPE_NTH) == 0) {
      is_i2c_write = true;
      _buffer[i++] = I2C_TH_HUMIDITY_MIN_ADDRESS;
      _buffer[i++] = I2C_TH_HUMIDITY_MIN_LENGTH;
      _buffer[i] = crc8(_buffer, i);
    }
    else if (strcmp(_type, SENSOR_TYPE_XTH) == 0) {
      is_i2c_write = true;
      _buffer[i++] = I2C_TH_HUMIDITY_MAX_ADDRESS;
      _buffer[i++] = I2C_TH_HUMIDITY_MAX_LENGTH;
      _buffer[i] = crc8(_buffer, i);
    }
    else{
      LOGE(F("th type mismatch %s for humid write"),_type);
      _is_success = false;
    }
    
    if (is_i2c_write) {
      Wire.beginTransmission(_address);
      Wire.write(_buffer, i+1);

      if (Wire.endTransmission()) {
	LOGT(F("th get... ERROR SET_HUMIDITY_ADDRESS"));
	_error_count++;
        _is_success = false;
      }else{
	_error_count = 0;
      }	
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
      else{
	LOGE(F("th type mismatch %s for humid read"),_type);
	_is_success = false;
      }
      
      if (_is_success) {
        Wire.requestFrom(_address, (uint8_t)(data_length + 1));
        if (Wire.available() < (data_length + 1)) {
	  LOGT(F("th get... ERROR READ_HUMIDITY"));
	  _error_count++;
          _is_success = false;
        }else{
	  _error_count = 0;
	}
      }

      if (_is_success) {
        for (i = 0; i < data_length; i++) {
          humidity_data[i] = Wire.read();
        }

        if (crc8(humidity_data, data_length) != Wire.read()) {
	  LOGT(F("th get... ERROR READ_HUMIDITY CRC error"));
          _is_success = false;
        }
      }

    _delay_ms = 0;
    _start_time_ms = millis();
    _get_state = END;
    break;

    case END:
    if ((_is_previous_prepared && !is_test) || (_is_current_prepared && is_test)) {
      if (length >= 1) {
	if (( ISVALID_UINT8(temperature_data[0]) || ISVALID_UINT8(temperature_data[1] ))) {	
	  values[0] = ((int32_t)(temperature_data[1] << 8) | (temperature_data[0]));
	}
      }

      if (length >= 2) {
	if (( ISVALID_UINT8(humidity_data[0]) || ISVALID_UINT8(humidity_data[1] ))) {	
	  values[1] = ((int32_t)(humidity_data[1] << 8) | (humidity_data[0]));
	}
      }
    } else {
      LOGE(F("th driver status error -> previous:%T current:%T"),_is_previous_prepared,_is_current_prepared );
    }


    SensorDriver::printInfo();
    if (_is_success){
      LOGT(F("th get... [ %s ]"), OK_STRING);
    }else{
      LOGE(F("th get... [ %s ]"), FAIL_STRING);
    }

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        LOGT(F("th--> temperature: %d"), values[0]);
      }
      else {
        LOGT(F("th--> temperature: ---"));
      }
    }

    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
        LOGT(F("th--> humidity: %d"), values[1]);
      }
      else {
        LOGT(F("th--> humidity: ---"));
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
void SensorDriverTh::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverTh::get(values, length, is_test);
  json_buffer[0]='\0';

  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        json["B12101"] = values[0];
      }
      else json["B12101"] = nullptr;
    }

    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
        json["B13003"] = values[1];
      }
      else json["B13003"] = nullptr;
    }

    if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
      json_buffer[0]='\0';
    }
  }
}
#endif

#endif

//------------------------------------------------------------------------------
// Power panel and battery monitor
//------------------------------------------------------------------------------

#if (USE_SENSOR_POW)

void SensorDriverPower::resetPrepared(bool is_test) {
  _get_state = INIT;
  if (!is_test) {
    _is_previous_prepared = _is_current_prepared;
    _is_current_prepared = false;
  }
  *_is_prepared = false;
}

void SensorDriverPower::setup() {
  SensorDriver::printInfo();
  bool is_i2c_write;
  uint8_t i;
  _delay_ms = 0;
  _is_success = false;

  if (!*_is_setted) {
    memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
    is_i2c_write = false;
    i = 0;

    if (strcmp(_type, SENSOR_TYPE_POW) == 0) {
      is_i2c_write = true;
      _buffer[i++] = I2C_COMMAND_ID;
      _buffer[i++] = I2C_POWER_COMMAND_CONTINUOUS_START;
      _buffer[i] = crc8(_buffer, i);
    }

    if (is_i2c_write) {
      Wire.beginTransmission(_address);
      Wire.write(_buffer, i+1);

      if (Wire.endTransmission() == 0) {
	_error_count = 0;
        _is_success = true;
        *_is_setted = true;
      }else{
	_error_count++;
      }	
    }
  }
  else {
    _is_success = true;
  }

  LOGT(F("power setup... [ %s ]"), _is_success ? OK_STRING : ERROR_STRING);
}

void SensorDriverPower::prepare(bool is_test) {
  SensorDriver::printInfo();
  bool is_i2c_write;
  uint8_t i;
  _is_success = false;

  if (!*_is_prepared) {
    memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
    is_i2c_write = false;
    i = 0;

    if (strcmp(_type, SENSOR_TYPE_POW) == 0) {
      is_i2c_write = true;
      _buffer[i++] = I2C_COMMAND_ID;
      if (is_test) {
        _buffer[i++] = I2C_POWER_COMMAND_TEST_READ;
      }
      else {
        _buffer[i++] = I2C_POWER_COMMAND_CONTINUOUS_START_STOP;
      }
      _buffer[i] = crc8(_buffer, i);
      _delay_ms = 50;
    }

    if (is_i2c_write) {
      Wire.beginTransmission(_address);
      Wire.write(_buffer, i+1);

      if (Wire.endTransmission() == 0) {
	_error_count = 0;
        _is_success = true;
        *_is_prepared = true;
      }else{
	_error_count++;
	LOGE(F("power prepare... endTransmission"));
      }
    }
  }
  else {
    _is_success = true;
    _delay_ms = 0;
  }
  
  if(!is_test)_is_current_prepared = *_is_prepared;
  LOGT(F("power prepare... [ %s ]"), _is_success ? OK_STRING : ERROR_STRING);
  _start_time_ms = millis();
}

void SensorDriverPower::get(int32_t *values, uint8_t length, bool is_test) {
  uint8_t data_length;
  bool is_i2c_write;
  uint8_t i;

  switch (_get_state) {
    case INIT:

      for (uint8_t i =0; i < length; i++) {
	values[i]=INT32_MAX;
      }

      memset(power_data_p, UINT8_MAX, I2C_POWER_AVERAGE_PANEL_LENGTH);
      memset(power_data_b, UINT8_MAX, I2C_POWER_AVERAGE_BATTERY_LENGTH);

      _is_readed = false;
      _is_end = false;

      if ( *_is_prepared && length >= 1) {
	LOGT(F("power get INIT"));
	_is_success = true;
	_get_state = SET_PANEL_ADDRESS;
      }
      else {
	LOGE(F("power get INIT"));
	_is_success = false;
	_get_state = END;
      }
      
      _delay_ms = 0;
      _start_time_ms = millis();      
      break;

    case SET_PANEL_ADDRESS:
      memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
      is_i2c_write = false;
      i = 0;

      if (strcmp(_type, SENSOR_TYPE_POW) == 0) {
        is_i2c_write = true;
	if (is_test){
	  _buffer[i++] = I2C_POWER_SAMPLE_PANEL_ADDRESS;
	  _buffer[i++] = I2C_POWER_SAMPLE_PANEL_LENGTH;
	}else{
	  _buffer[i++] = I2C_POWER_AVERAGE_PANEL_ADDRESS;
	  _buffer[i++] = I2C_POWER_AVERAGE_PANEL_LENGTH;
	}	
        _buffer[i] = crc8(_buffer, i);
      }
      else{
	LOGE(F("POW type mismatch %s for panel write"),_type);
	_is_success = false;
      }

      if (is_i2c_write) {
        Wire.beginTransmission(_address);
        Wire.write(_buffer, i+1);

        if (Wire.endTransmission()) {
	  LOGE(F("power get... ERROR SET panel ADDRESS"));
	  _error_count++;
          _is_success = false;
        }else{
	  _error_count = 0;
	}
      }

      if (_is_success) {
        _get_state = READ_PANEL_VALUE;
      }
      else {
        _get_state = END;
      }
      break;

    case READ_PANEL_VALUE:

      if (strcmp(_type, SENSOR_TYPE_POW) == 0) {
	data_length = I2C_POWER_AVERAGE_PANEL_LENGTH;
      }
      else{
	_is_success = false;
	LOGE(F("power type mismatch %s for panel read"),_type);
      }
      
      if (_is_success) {
	  Wire.requestFrom(_address, (uint8_t)(data_length + 1));
        if (Wire.available() < (data_length + 1)) {
	  LOGE(F("power get... ERROR READ_PANEL_VALUE"));
	  _error_count++;
          _is_success = false;
        }else{
	  _error_count = 0;
	}
      }

      if (_is_success) {
        for (i = 0; i < data_length; i++) {
          power_data_p[i] = Wire.read();
        }

        if (crc8(power_data_p, data_length) != Wire.read()) {
	  LOGE(F("power get... ERROR READ PANEL CRC error"));
          _is_success = false;
        }
      }


      if (_is_success) {
        _get_state = SET_BATTERY_ADDRESS;
      }
      else {
        _get_state = END;
      }


      break;


    case SET_BATTERY_ADDRESS:
      memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
      is_i2c_write = false;
      i = 0;

      if (strcmp(_type, SENSOR_TYPE_POW) == 0) {
        is_i2c_write = true;
	if (is_test){
	  _buffer[i++] = I2C_POWER_SAMPLE_BATTERY_ADDRESS;
	  _buffer[i++] = I2C_POWER_SAMPLE_BATTERY_LENGTH;
	}else{
	  _buffer[i++] = I2C_POWER_AVERAGE_BATTERY_ADDRESS;
	  _buffer[i++] = I2C_POWER_AVERAGE_BATTERY_LENGTH;
	}	
        _buffer[i] = crc8(_buffer, i);
      }
      else{
	LOGE(F("POW type mismatch %s for battery write"),_type);
	_is_success = false;
      }

      if (is_i2c_write) {
        Wire.beginTransmission(_address);
        Wire.write(_buffer, i+1);

        if (Wire.endTransmission()) {
	  LOGE(F("power get... ERROR SET _BATTERY ADDRESS"));
	  _error_count++;
          _is_success = false;
        }else{
	  _error_count = 0;
	}
      }

      if (_is_success) {
        _get_state = READ_BATTERY_VALUE;
      }
      else {
        _get_state = END;
      }
      break;

    case READ_BATTERY_VALUE:

      if (strcmp(_type, SENSOR_TYPE_POW) == 0) {
	data_length = I2C_POWER_AVERAGE_PANEL_LENGTH;
      }
      else{
	_is_success = false;
	LOGE(F("power type mismatch %s for  battery read"),_type);
      }
      
      if (_is_success) {
	  Wire.requestFrom(_address, (uint8_t)(data_length + 1));
        if (Wire.available() < (data_length + 1)) {
	  LOGE(F("power get... ERROR READ_BATTERY_VALUE"));
	  _error_count++;
          _is_success = false;
        }else{
	  _error_count = 0;
	}
      }

      if (_is_success) {
        for (i = 0; i < data_length; i++) {
          power_data_b[i] = Wire.read();
        }

        if (crc8(power_data_b, data_length) != Wire.read()) {
	  LOGE(F("power get... ERROR READ_AVERAGE CRC error"));
          _is_success = false;
        }
      }

      _delay_ms = 0;
      _start_time_ms = millis();
      _get_state = END;
      
      break;

    case END:
      if ((_is_previous_prepared && !is_test) || (_is_current_prepared && is_test)) {
	if (length >= 1) {
	  int16_t value= (int16_t)(power_data_b[1] << 8) | power_data_b[0];
	  if ( ISVALID_INT16(value)) {	
	    values[0] = value;
	  }
	}

	if (length >= 2) {
	  int16_t value= (int16_t)(power_data_p[1] << 8) | power_data_p[0];
	  if ( ISVALID_INT16(value)) {	
	    values[1] = value;
	  }
	}

      } else {
	LOGE(F("power driver status error -> previous:%T current:%T"),_is_previous_prepared,_is_current_prepared );
      }
      
      SensorDriver::printInfo();

      if (_is_success){
	LOGT(F("power get... [ %s ]"), OK_STRING);
      }else{
	LOGE(F("power get... [ %s ]"), FAIL_STRING);
      }

      if (length >= 1) {
	if (ISVALID_INT32(values[0])) {
	  LOGT(F("power--> Battery: %l"), values[0]);
	}
	else {
	  LOGT(F("power--> Battery: ---"));
	}
      }
      if (length >= 2) {
	if (ISVALID_INT32(values[1])) {
	  LOGT(F("power--> Panel: %l"), values[1]);
	}
	else {
	  LOGT(F("power--> Panel: ---"));
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
void SensorDriverPower::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverPower::get(values, length, is_test);
  json_buffer[0]='\0';
  
  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;


    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
	json["B25025"] = values[0];
      }
      else json["B25025"] = nullptr;
    }
    
    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
	json["B25194"] = values[1];
      }
      else json["B25194"] = nullptr;
    }

     if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
       json_buffer[0]='\0';
     }
  }
}
#endif
#endif

//------------------------------------------------------------------------------
// DigitEco Power
//------------------------------------------------------------------------------

#if (USE_SENSOR_DEP)

void SensorDriverDigitecoPower::resetPrepared(bool is_test) {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverDigitecoPower::setup() {
  SensorDriver::printInfo();

  _delay_ms = 0;

  Wire.beginTransmission(_address);

  if (Wire.endTransmission() == 0) {
    *_is_setted = true;
    _error_count = 0;
    LOGT(F("digitecopower setup... [ %s ]"), OK_STRING);
  }else{
    _error_count++;
    LOGE(F("digitecopower setup... [ %s ]"), ERROR_STRING);
  }
}

void SensorDriverDigitecoPower::prepare(bool is_test) {
  SensorDriver::printInfo();
  _delay_ms = 0;
  *_is_prepared = true;
  _start_time_ms = millis();
  LOGT(F("DEP prepare... [ %s ]"), OK_STRING);
}

void SensorDriverDigitecoPower::get(int32_t *values, uint8_t length, bool is_test) {

  switch (_get_state) {
  case INIT:
    for (uint8_t i =0; i < length; i++) {
      values[i]=INT32_MAX;
    }

    battery_charge = FLT_MAX;
    battery_voltage = FLT_MAX;
    battery_current = FLT_MAX;
    input_voltage = FLT_MAX;
    input_current = FLT_MAX;
    output_voltage = FLT_MAX;
   
    _is_readed = false;
    _is_end = false;
    _error_count = 0;
    
    if (*_is_prepared && length >= 1) {
      _is_success = true;
      _get_state = SET_BATTERY_VOLTAGE_ADDRESS;
    } else {
      _error_count++;
      _is_success = false;
      _get_state = END;
    }
    
    _delay_ms = 0;
    _start_time_ms = millis();
    break;
    
  case SET_BATTERY_VOLTAGE_ADDRESS:
    _is_success = DigitecoPower::de_send(_address, DIGITECO_POWER_BATTERY_VOLTAGE_ADDRESS);
    _delay_ms = 0;
    _start_time_ms = millis();
    
    if (_is_success) {
      _error_count = 0;
      _get_state = READ_BATTERY_VOLTAGE;
    }
    else {
      //LOGE(F("digitecopower get set battery voltage"));      
      _error_count++;
      _get_state = END;
    }
    break;
    
  case READ_BATTERY_VOLTAGE:
    _is_success = DigitecoPower::de_read(_address, &battery_voltage);
    _delay_ms = 0;
    _start_time_ms = millis();
    
    // There is NO CRC on DigitecoPower: workaround until it will to be implemented
    if (_is_success && ((battery_voltage >= 0.0) && (battery_voltage < 50.0))) {
      values[0] = battery_voltage * 10;
      _error_count = 0;
      if (length >= 2) {
	_get_state = SET_INPUT_VOLTAGE_ADDRESS;
      }else {
	_get_state = END;
      }
    }else {
      //LOGE(F("digitecopower get read battery voltage"));      
      _error_count++;
      _get_state = END;
    }
    break;
    
  case SET_INPUT_VOLTAGE_ADDRESS:
    _is_success = DigitecoPower::de_send(_address, DIGITECO_POWER_INPUT_VOLTAGE_ADDRESS);
    _delay_ms = 0;
    _start_time_ms = millis();
    
    if (_is_success) {
      _error_count = 0;
      _get_state = READ_INPUT_VOLTAGE;
    }
    else {
      //LOGE(F("digitecopower get set input voltage"));      
      _error_count++;
      _get_state = END;
    }
    break;
    
  case READ_INPUT_VOLTAGE:
    _is_success = DigitecoPower::de_read(_address, &input_voltage);
    _delay_ms = 0;
    _start_time_ms = millis();
    
    // There is NO CRC on DigitecoPower: workaround until it will to be implemented
    if (_is_success && ((input_voltage > -10.0) && (input_voltage < 50.0))) {
      values[1] = input_voltage * 10;
      _error_count = 0;
      if (length >= 3) {
	_get_state = SET_BATTERY_CHARGE_ADDRESS;
      }else {
	_get_state = END;
      }
    }
    else {
      //LOGE(F("digitecopower get read input voltage"));      
      _error_count++;
      _get_state = END;
    }
    break;
    
  case SET_BATTERY_CHARGE_ADDRESS:
    _is_success = DigitecoPower::de_send(_address, DIGITECO_POWER_BATTERY_CHARGE_ADDRESS);
    _delay_ms = 0;
    _start_time_ms = millis();
    
    if (_is_success) {
      _error_count = 0;
      _get_state = READ_BATTERY_CHARGE;
    }
    else {
      //LOGE(F("digitecopower get set battery charge"));      
      _error_count++;
      _get_state = END;
    }
    break;
    
  case READ_BATTERY_CHARGE:
    _is_success = DigitecoPower::de_read(_address, &battery_charge);
    _delay_ms = 0;
    _start_time_ms = millis();
    
    // There is NO CRC on DigitecoPower: workaround until it will to be implemented
    if (_is_success && ((battery_charge >= 0.0) && (battery_charge <= 100.0))) {
      values[2] = battery_charge;
      _error_count = 0;
      if (length >= 4) {
	_get_state = SET_BATTERY_CURRENT_ADDRESS;
      }else {
	_get_state = END;
      }
    }
    else {
      //LOGE(F("digitecopower get read battery charge"));      
      _error_count++;      
      _get_state = END;
    }
    break;
    
  case SET_BATTERY_CURRENT_ADDRESS:
    _is_success = DigitecoPower::de_send(_address, DIGITECO_POWER_BATTERY_CURRENT_ADDRESS);
    _delay_ms = 0;
    _start_time_ms = millis();
    
    if (_is_success) {
      _error_count = 0;
      _get_state = READ_BATTERY_CURRENT;
    }
    else {
      //LOGE(F("digitecopower get set battery current"));      
      _error_count++;
      _get_state = END;
    }
    break;
    
  case READ_BATTERY_CURRENT:
    _is_success = DigitecoPower::de_read(_address, &battery_current);
    _delay_ms = 0;
    _start_time_ms = millis();
    
    // There is NO CRC on DigitecoPower: workaround until it will to be implemented
    if (_is_success && ((battery_current >= -5000.0) && (battery_current <= 5000.0))) {
      values[3] = battery_current * 100.0;
      _error_count = 0;
      if (length >= 5) {
	_get_state = SET_INPUT_CURRENT_ADDRESS;
      }else {
	_get_state = END;
      }
    }
    else {
      //LOGE(F("digitecopower get read battery current"));      
      _error_count++;
      _get_state = END;
    }
    break;
    
  case SET_INPUT_CURRENT_ADDRESS:
    _is_success = DigitecoPower::de_send(_address, DIGITECO_POWER_INPUT_CURRENT_ADDRESS);
    _delay_ms = 0;
    _start_time_ms = millis();
    
    if (_is_success) {
      _error_count = 0;
      _get_state = READ_INPUT_CURRENT;
    }
    else {
      //LOGE(F("digitecopower get set input current"));      
      _error_count++;
      _get_state = END;
    }
    break;
    
  case READ_INPUT_CURRENT:
    _is_success = DigitecoPower::de_read(_address, &input_current);
    _delay_ms = 0;
    _start_time_ms = millis();
    
    // There is NO CRC on DigitecoPower: workaround until it will to be implemented
    if (_is_success && ((input_current >= 0.0) && (input_current <= 5000.0))) {
      values[4] = input_current * 1000.0;
      _error_count = 0;
      if (length >= 6) {
	_get_state = SET_OUTPUT_VOLTAGE_ADDRESS;
      }else {
	_get_state = END;
      }
    }
    else {
      //LOGE(F("digitecopower get read input current"));      
      _error_count++;
      _get_state = END;
    }
    break;
    
  case SET_OUTPUT_VOLTAGE_ADDRESS:
    _is_success = DigitecoPower::de_send(_address, DIGITECO_POWER_OUTPUT_VOLTAGE_ADDRESS);
    _delay_ms = 0;
    _start_time_ms = millis();
    
    if (_is_success) {
      _error_count = 0;
      _get_state = READ_OUTPUT_VOLTAGE;
    }
    else {
      //LOGE(F("digitecopower get set output voltage"));      
      _error_count++;
      _get_state = END;
    }
    break;
    
  case READ_OUTPUT_VOLTAGE:
    _is_success = DigitecoPower::de_read(_address, &output_voltage);
    
    // There is NO CRC on DigitecoPower: workaround until it will to be implemented
    if (_is_success && ((output_voltage <0.0) && (output_voltage > 50.0))){
      values[5] = output_voltage * 10.0;
      _error_count = 0;
    } else {
      //LOGE(F("digitecopower get read output voltage"));      
      _error_count++;
    }
    _delay_ms = 0;
    _start_time_ms = millis();
    _get_state = END;
    break;
    
  case END:
        
    SensorDriver::printInfo();
    if (_is_success){
      LOGT(F("digitecopower get... [ %s ]"), OK_STRING);
    }else{
      LOGE(F("digitecopower get... [ %s ]"), FAIL_STRING);
    }

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        LOGT(F("digitecopower--> battery voltage: %l V"), values[0]);
      }
      else {
	LOGT(F("digitecopower--> battery voltage: ---"));
      }
    }
    
    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
	LOGT(F("digitecopower--> input voltage: %l V"), values[1]);
      }
      else {
	LOGT(F("digitecopower--> input voltage: ---"));
      }
    }
    
    if (length >= 3) {
      if (ISVALID_INT32(values[2])) {
	LOGT(F("digitecopower--> battery charge: %l %%"), values[2]);
      }
      else {
	LOGT(F("digitecopower--> battery charge: ---"));
      }
    }
    
    if (length >= 4) {
      if (ISVALID_INT32(values[3])) {
	LOGT(F("digitecopower--> battery current: %l mA"), values[3]);
      }
      else {
	LOGT(F("digitecopower--> battery current: ---"));
      }
    }
    
    if (length >= 5) {
      if (ISVALID_INT32(values[4])) {
	LOGT(F("digitecopower--> input current: %l mA"), values[4]);
      }
      else {
        LOGT(F("digitecopower--> input current: ---"));
      }
    }
    
    if (length >= 6) {
      if (ISVALID_INT32(values[5])) {
	LOGT(F("digitecopower--> output voltage: %l V"), values[5]);
      }
      else {
        LOGT(F("digitecopower--> output voltage: ---"));
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
void SensorDriverDigitecoPower::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverDigitecoPower::get(values, length, is_test);
  json_buffer[0]='\0';

  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;

    if (length >= 1) {
         if (ISVALID_INT32(values[0])) {
            json["B25025"] = values[0];
      }
         else json["B25025"] = nullptr;
    }

    if (length >= 2) {
         if (ISVALID_INT32(values[1])) {
            json["B25194"] = values[1];
      }
         else json["B25194"] = nullptr;
      }

      if (length >= 3) {
         if (ISVALID_INT32(values[2])) {
            json["B25192"] = values[2];
         }
         else json["B25192"] = nullptr;
    }

    if (length >= 4) {
         if (ISVALID_INT32(values[3])) {
        json["B25193"] = values[3];
      }
      else json["B25193"] = nullptr;
     }

     if (length >= 5) {
         if (ISVALID_INT32(values[4])) {
         json["B00005"] = values[4];
       }
       else json["B00005"] = nullptr;
     }

     if (length >= 6) {
         if (ISVALID_INT32(values[5])) {
         json["B00006"] = values[5];
       }
       else json["B00006"] = nullptr;
     }

     if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
       json_buffer[0]='\0';
     }
  }
}
#endif

#endif

//------------------------------------------------------------------------------
// Wind Speed and Direction i2-wind module V3
// USE_SENSOR_DWA:
// USE_SENSOR_DWB:
// USE_SENSOR_DWC:
// USE_SENSOR_DWD:
// USE_SENSOR_DWE:
// USE_SENSOR_DWF:
//------------------------------------------------------------------------------
#if (USE_SENSOR_DWA || USE_SENSOR_DWB || USE_SENSOR_DWC || USE_SENSOR_DWD || USE_SENSOR_DWE || USE_SENSOR_DWF)

void SensorDriverWind::resetPrepared(bool is_test) {
  _get_state = INIT;
  if (!is_test) {
    _is_previous_prepared = _is_current_prepared;
    _is_current_prepared = false;
  }
  *_is_prepared = false;
}

void SensorDriverWind::setup() {
  SensorDriver::printInfo();
  bool is_i2c_write;
  uint8_t i;
  _delay_ms = 0;
  _is_success = false;

  if (!*_is_setted) {
    memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
    is_i2c_write = false;
    i = 0;

    is_i2c_write = true;
    _buffer[i++] = I2C_COMMAND_ID;
    _buffer[i++] = I2C_WIND_COMMAND_CONTINUOUS_START;
    _buffer[i] = crc8(_buffer, i);

    if (is_i2c_write) {
      Wire.beginTransmission(_address);
      Wire.write(_buffer, i+1);

      if (Wire.endTransmission() == 0) {
	LOGV("wind setup I2C %d, %d",_address,i+1);      
	_error_count = 0;
        _is_success = true;
        *_is_setted = true;
      }else{
	LOGE("wind setup error writing I2C");
	_error_count++;
      }	
    }
  }
  else {
    _is_success = true;
  }

  LOGT(F("wind setup... [ %s ]"), _is_success ? OK_STRING : ERROR_STRING);
}

void SensorDriverWind::prepare(bool is_test) {
  SensorDriver::printInfo();
  bool is_i2c_write;
  uint8_t i;
  _is_success = false;
  _is_test = is_test;
  _delay_ms = 0;

  if (!*_is_prepared) {
    memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
    is_i2c_write = false;
    i = 0;

    is_i2c_write = true;
    _buffer[i++] = I2C_COMMAND_ID;
    if (is_test) {
      _buffer[i++] = I2C_WIND_COMMAND_TEST_READ;
    } else {
	_buffer[i++] = I2C_WIND_COMMAND_CONTINUOUS_START_STOP;
    }
    _buffer[i] = crc8(_buffer, i);

    if (is_i2c_write) {
      Wire.beginTransmission(_address);
      Wire.write(_buffer, i+1);

      if (Wire.endTransmission() == 0) {
	_delay_ms = 600;   // no less then 550 ms
	_error_count = 0;
        _is_success = true;
        *_is_prepared = true;
      }else{
	_error_count++;
      }
    }
  }
  else {
    _is_success = true;
  }

  if(!is_test)_is_current_prepared = *_is_prepared;  
  LOGT(F("wind prepare... [ %s ]"), _is_success ? OK_STRING : ERROR_STRING);
  _start_time_ms = millis();
}

void SensorDriverWind::get(int32_t *values, uint8_t length, bool is_test) {

  bool is_i2c_write;
  uint8_t i;  
  uint16_t val16;
  uint8_t  val8;
  uint8_t *val_ptr;

  switch (_get_state) {
    case INIT:
      for (uint8_t i =0; i < length; i++) {
	values[i]=INT32_MAX;
      }

      if (strcmp(_type, SENSOR_TYPE_DWA) == 0) {
        val16 = UINT16_MAX;
        variable_length = 2;
      }
      else if (strcmp(_type, SENSOR_TYPE_DWB) == 0) {
        val16 = UINT16_MAX;
        variable_length = 2;
      }
      else if (strcmp(_type, SENSOR_TYPE_DWC) == 0) {
        val16 = UINT16_MAX;
        variable_length = 2;
      }
      else if (strcmp(_type, SENSOR_TYPE_DWD) == 0) {
        val16 = UINT16_MAX;
        variable_length = 1;
      }
      else if (strcmp(_type, SENSOR_TYPE_DWE) == 0) {
        val8 = UINT8_MAX;
        variable_length = 6;
      }
      else if (strcmp(_type, SENSOR_TYPE_DWF) == 0) {
        val16 = UINT16_MAX;
        variable_length = 2;
      }

      variable_count = 0;
      data_length = 0;

      _is_readed = false;
      _is_end = false;

      if (*_is_prepared && length >= 1) {
        _is_success = true;
        _get_state = SET_ADDRESS;
      }
      else {
        _is_success = false;
        _get_state = END;
      }

      _delay_ms = 0;
      _start_time_ms = millis();
      break;

    case SET_ADDRESS:
      memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
      is_i2c_write = false;
      offset = 0;
      i = 0;

      #if (USE_SENSOR_DWA)
      if (strcmp(_type, SENSOR_TYPE_DWA) == 0) {
	if (is_test){      //TODO with samples
	  data_length = I2C_WIND_SAMPLE_LENGTH;
	  _buffer[i++] = I2C_WIND_SAMPLE_ADDRESS;
	  _buffer[i++] = I2C_WIND_SAMPLE_LENGTH;
	} else {
	  data_length = I2C_WIND_VAVG10_LENGTH;
	  _buffer[i++] = I2C_WIND_VAVG10_ADDRESS;
	  _buffer[i++] = I2C_WIND_VAVG10_LENGTH;
	}
	is_i2c_write = true;
	_buffer[i] = crc8(_buffer, i);
      }
      #endif
      
      #if (USE_SENSOR_DWB)
      if (strcmp(_type, SENSOR_TYPE_DWB) == 0) {
	if (is_test){      //TODO with samples
	  data_length = I2C_WIND_SAMPLE_LENGTH;
	  _buffer[i++] = I2C_WIND_SAMPLE_ADDRESS;
	  _buffer[i++] = I2C_WIND_SAMPLE_LENGTH;
	} else {
	  data_length = I2C_WIND_VAVG_LENGTH;
	  _buffer[i++] = I2C_WIND_VAVG_ADDRESS;
	  _buffer[i++] = I2C_WIND_VAVG_LENGTH;
	}	
	is_i2c_write = true;
	_buffer[i] = crc8(_buffer, i);
      }
        #endif
      
      #if (USE_SENSOR_DWC)
      if (strcmp(_type, SENSOR_TYPE_DWC) == 0) {
	if (is_test){      //TODO with samples
	  data_length = I2C_WIND_SAMPLE_LENGTH;
	  _buffer[i++] = I2C_WIND_SAMPLE_ADDRESS;
	  _buffer[i++] = I2C_WIND_SAMPLE_LENGTH;
	} else {
	  data_length = I2C_WIND_GUST_SPEED_LENGTH;
	  _buffer[i++] = I2C_WIND_GUST_SPEED_ADDRESS;
	  _buffer[i++] = I2C_WIND_GUST_SPEED_LENGTH;
	}
	is_i2c_write = true;
	_buffer[i] = crc8(_buffer, i);
      }
      #endif
      
      #if (USE_SENSOR_DWD)
      if (strcmp(_type, SENSOR_TYPE_DWD) == 0) {
	if (is_test){
	  data_length = I2C_WIND_SPEED_LENGTH;;
	  _buffer[i++] = I2C_WIND_SAMPLE_ADDRESS;  // only one!
	  _buffer[i++] = I2C_WIND_SPEED_LENGTH;
	} else {
	  data_length = I2C_WIND_SPEED_LENGTH;
	  _buffer[i++] = I2C_WIND_SPEED_ADDRESS;
	  _buffer[i++] = I2C_WIND_SPEED_LENGTH;
	}
	is_i2c_write = true;
	_buffer[i] = crc8(_buffer, i);
      }
      #endif
      
      #if (USE_SENSOR_DWE)
      if (strcmp(_type, SENSOR_TYPE_DWE) == 0) {
	is_i2c_write = true;
	data_length = I2C_WIND_CLASS_LENGTH;
	_buffer[i++] = I2C_WIND_CLASS_ADDRESS;
	_buffer[i++] = I2C_WIND_CLASS_LENGTH;
	_buffer[i] = crc8(_buffer, i);
      }
      #endif
      
      #if (USE_SENSOR_DWF)
      if (strcmp(_type, SENSOR_TYPE_DWF) == 0) {
	if (is_test){
	  data_length = I2C_WIND_SAMPLE_LENGTH;
	  _buffer[i++] = I2C_WIND_SAMPLE_ADDRESS;
	  _buffer[i++] = I2C_WIND_SAMPLE_LENGTH;
	} else {
	  data_length = I2C_WIND_GUST_DIRECTION_LENGTH;
	  _buffer[i++] = I2C_WIND_GUST_DIRECTION_ADDRESS;
	  _buffer[i++] = I2C_WIND_GUST_DIRECTION_LENGTH;
	}
	is_i2c_write = true;
	_buffer[i] = crc8(_buffer, i);
      }
      #endif
      
      if (is_i2c_write) {
        Wire.beginTransmission(_address);
        Wire.write(_buffer, i+1);

        if (Wire.endTransmission()) {
	  _error_count = 0;
          _is_success = false;
        }else{
	  _error_count++;
	}
      }

      _delay_ms = 0;
      _start_time_ms = millis();

      if (_is_success) {
        _get_state = READ_VALUE;
      }
      else {
        _get_state = END;
      }
      break;

    case READ_VALUE:
      if (_is_success) {
        Wire.requestFrom(_address, (uint8_t) (data_length + 1));
        if (Wire.available() < (data_length + 1)) {
	  _error_count++;
          _is_success = false;
        }else{
	  _error_count = 0;
	}
      }

      if (_is_success) {
        memset(_buffer, UINT8_MAX, I2C_MAX_DATA_LENGTH * sizeof(uint8_t));
        for (i = 0; i < data_length; i++) {
          _buffer[i] = Wire.read();
        }

        if (crc8(_buffer, data_length) != Wire.read()) {
          _is_success = false;
        }
      }

      _delay_ms = 0;
      _start_time_ms = millis();

      if (_is_success) {
        _get_state = GET_VALUE;
      }
      else {
        _get_state = END;
      }
      break;

    case GET_VALUE:

      if (length >= variable_count + 1) {
	if ((strcmp(_type, SENSOR_TYPE_DWA) == 0) || (strcmp(_type, SENSOR_TYPE_DWB) == 0) || (strcmp(_type, SENSOR_TYPE_DWC) == 0) || (strcmp(_type, SENSOR_TYPE_DWD) == 0) || (strcmp(_type, SENSOR_TYPE_DWF) == 0)) {
          val_ptr = (uint8_t*) &val16;
	  
          for (i = 0; i < 2; i++) {
            *(val_ptr + i) = _buffer[offset + i];
          }
          offset += 2;

	  if (_is_success && ISVALID(val16)) {
	    // speed or direction
	    values[variable_count] = (int32_t) val16;
	  }
	}else if (strcmp(_type, SENSOR_TYPE_DWE) == 0) {

          val_ptr = (uint8_t*) &val8;
	  
          for (i = 0; i < 1; i++) {
            *(val_ptr + i) = _buffer[offset + i];
          }
          offset += 1;
	  
	  // speed class percent
	  if (_is_success && ISVALID_UINT8(val8)) {
	    values[variable_count] = (int32_t) val8;
	  }
	}
      } else {
	values[variable_count] = INT32_MAX;
	_is_success = false;
      }

      variable_count++;

      _delay_ms = 0;
      _start_time_ms = millis();
      
      if ((variable_count >= length) || (variable_count >= variable_length)) {
        _get_state = END;
      }
      //      else if (variable_count == 7) {
      //  _get_state = SET_ADDRESS;
      //}
      break;
      
    case END:
      SensorDriver::printInfo();
      if (_is_success){
	LOGT(F("wind get... [ %s ]"), OK_STRING);
      }else{
	LOGE(F("wind get... [ %s ]"), FAIL_STRING);
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
void SensorDriverWind::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverWind::get(values, length, is_test);
  json_buffer[0]='\0';

  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;

    #if (USE_SENSOR_DWA || USE_SENSOR_DWB)
    if ((strcmp(_type, SENSOR_TYPE_DWA) == 0) || (strcmp(_type, SENSOR_TYPE_DWB) == 0)) {
      if (length >= 1) {
        if (ISVALID_INT32(values[0])) {
          json["B11002"] = values[0];
        }
        else json["B11002"] = nullptr;
      }

      if (length >= 2) {
        if (ISVALID_INT32(values[1])) {
          json["B11001"] = values[1];
        }
        else json["B11001"] = nullptr;
      }
    }
    #endif

    #if (USE_SENSOR_DWC)
    if (strcmp(_type, SENSOR_TYPE_DWC) == 0) {
      if (length >= 1) {
        if (ISVALID_INT32(values[0])) {
          json["B11041"] = values[0];
        }
        else json["B11041"] = nullptr;
      }

      if (length >= 2) {
        if (ISVALID_INT32(values[1])) {
          json["B11209"] = values[1];
        }
        else json["B11209"] = nullptr;
      }
    }
    #endif

    #if (USE_SENSOR_DWD)
    if (strcmp(_type, SENSOR_TYPE_DWD) == 0) {
      if (length >= 1) {
        if (ISVALID_INT32(values[0])) {
          json["B11002"] = values[0];
        }
        else json["B11002"] = nullptr;
      }
    }
    #endif

    #if (USE_SENSOR_DWE)
    if (strcmp(_type, SENSOR_TYPE_DWE) == 0) {
      if (length >= 1) {
        json["d"] = 51;

        JsonArray p = json.createNestedArray("p");

        for (uint8_t i = 0; i < length; i++) {
          if (ISVALID_INT32(values[i])) {
            p.add(values[i]);
          }
          else p.add(nullptr);;
        }
      }
    }
    #endif

    #if (USE_SENSOR_DWF)
    if (strcmp(_type, SENSOR_TYPE_DWF) == 0) {
      if (length >= 1) {
        if (ISVALID_INT32(values[0])) {
          json["B11043"] = values[0];
        }
        else json["B11043"] = nullptr;
      }

      if (length >= 2) {
        if (ISVALID_INT32(values[1])) {
          json["B11210"] = values[1];
        }
        else json["B11210"] = nullptr;
      }
    }
    #endif

    if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
      json_buffer[0]='\0';
    }
  }
}
#endif

#endif

//------------------------------------------------------------------------------
// Solar Radiation Sensor
// USE_SENSOR_DSA: average solar radiation
//------------------------------------------------------------------------------
#if (USE_SENSOR_DSA)

void SensorDriverSolarRadiation::resetPrepared(bool is_test) {
  _get_state = INIT;
  if (!is_test) {
    _is_previous_prepared = _is_current_prepared;
    _is_current_prepared = false;
  }
  *_is_prepared = false;
}

void SensorDriverSolarRadiation::setup() {
  SensorDriver::printInfo();
  bool is_i2c_write;
  uint8_t i;
  _delay_ms = 0;
  _is_success = false;

  if (!*_is_setted) {
    memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
    is_i2c_write = false;
    i = 0;

    if (strcmp(_type, SENSOR_TYPE_DSA) == 0) {
      is_i2c_write = true;
      _buffer[i++] = I2C_COMMAND_ID;
      _buffer[i++] = I2C_SOLAR_RADIATION_COMMAND_CONTINUOUS_START;
      _buffer[i] = crc8(_buffer, i);
    }

    if (is_i2c_write) {
      Wire.beginTransmission(_address);
      Wire.write(_buffer, i+1);

      if (Wire.endTransmission() == 0) {
	_error_count = 0;
        _is_success = true;
        *_is_setted = true;
      }else{
	_error_count++;
      }	
    }
  }
  else {
    _is_success = true;
  }

  LOGT(F("solarradiation setup... [ %s ]"), _is_success ? OK_STRING : ERROR_STRING);
}

void SensorDriverSolarRadiation::prepare(bool is_test) {
  SensorDriver::printInfo();
  bool is_i2c_write;
  uint8_t i;
  _is_success = false;

  if (!*_is_prepared) {
    memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
    is_i2c_write = false;
    i = 0;

    if (strcmp(_type, SENSOR_TYPE_DSA) == 0) {
      is_i2c_write = true;
      _buffer[i++] = I2C_COMMAND_ID;
      if (is_test) {
        _buffer[i++] = I2C_SOLAR_RADIATION_COMMAND_TEST_READ;
      }
      else {
        _buffer[i++] = I2C_SOLAR_RADIATION_COMMAND_CONTINUOUS_START_STOP;
      }
      _buffer[i] = crc8(_buffer, i);
      _delay_ms = 50;
    }

    if (is_i2c_write) {
      Wire.beginTransmission(_address);
      Wire.write(_buffer, i+1);

      if (Wire.endTransmission() == 0) {
	_error_count = 0;
        _is_success = true;
        *_is_prepared = true;
      }else{
	_error_count++;
	LOGE(F("radiation prepare... endTransmission"));
      }
    }
  }
  else {
    _is_success = true;
    _delay_ms = 0;
  }
  
  if(!is_test)_is_current_prepared = *_is_prepared;
  LOGT(F("solarradiation prepare... [ %s ]"), _is_success ? OK_STRING : ERROR_STRING);
  _start_time_ms = millis();
}

void SensorDriverSolarRadiation::get(int32_t *values, uint8_t length, bool is_test) {
  uint8_t data_length;
  bool is_i2c_write;
  uint8_t i;

  switch (_get_state) {
    case INIT:

      for (uint8_t i =0; i < length; i++) {
	values[i]=INT32_MAX;
      }

      memset(radiation_data, UINT8_MAX, I2C_SOLAR_RADIATION_AVERAGE_LENGTH);

      _is_readed = false;
      _is_end = false;

      if ( *_is_prepared && length >= 1) {
	LOGT(F("radiation get INIT"));
	_is_success = true;
	_get_state = SET_ADDRESS;
      }
      else {
	LOGE(F("radiation get INIT"));
	_is_success = false;
	_get_state = END;
      }
      
      _delay_ms = 0;
      _start_time_ms = millis();      
      break;

    case SET_ADDRESS:
      memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
      is_i2c_write = false;
      i = 0;

      if (strcmp(_type, SENSOR_TYPE_DSA) == 0) {
        is_i2c_write = true;
	if (is_test){
	  _buffer[i++] = I2C_SOLAR_RADIATION_SAMPLE_ADDRESS;
	  _buffer[i++] = I2C_SOLAR_RADIATION_SAMPLE_LENGTH;
	}else{
	  _buffer[i++] = I2C_SOLAR_RADIATION_AVERAGE_ADDRESS;
	  _buffer[i++] = I2C_SOLAR_RADIATION_AVERAGE_LENGTH;
	}	
        _buffer[i] = crc8(_buffer, i);
      }
      else{
	LOGE(F("dsa type mismatch %s for average write"),_type);
	_is_success = false;
      }

      if (is_i2c_write) {
        Wire.beginTransmission(_address);
        Wire.write(_buffer, i+1);

        if (Wire.endTransmission()) {
	  LOGE(F("radiation get... ERROR SET_AVERAGE_ADDRESS"));
	  _error_count++;
          _is_success = false;
        }else{
	  _error_count = 0;
	}
      }

      if (_is_success) {
        _get_state = READ_VALUE;
      }
      else {
        _get_state = END;
      }
      break;

    case READ_VALUE:

      if (strcmp(_type, SENSOR_TYPE_DSA) == 0) {
	data_length = I2C_SOLAR_RADIATION_AVERAGE_LENGTH;
      }
      else{
	_is_success = false;
	LOGE(F("radiation type mismatch %s for average read"),_type);
      }
      
      if (_is_success) {
	  Wire.requestFrom(_address, (uint8_t)(data_length + 1));
        if (Wire.available() < (data_length + 1)) {
	  LOGE(F("radiation get... ERROR READ_AVERAGE"));
	  _error_count++;
          _is_success = false;
        }else{
	  _error_count = 0;
	}
      }

      if (_is_success) {
        for (i = 0; i < data_length; i++) {
          radiation_data[i] = Wire.read();
        }

        if (crc8(radiation_data, data_length) != Wire.read()) {
	  LOGE(F("radiation get... ERROR READ_AVERAGE CRC error"));
          _is_success = false;
        }
      }

      _delay_ms = 0;
      _start_time_ms = millis();
      _get_state = END;
      
      break;

    case END:
      if ((_is_previous_prepared && !is_test) || (_is_current_prepared && is_test)) {
	if (length >= 1) {
	  int16_t value= (int16_t)(radiation_data[1] << 8) | radiation_data[0];
	  if ( ISVALID_INT16(value)) {	
	    values[0] = value;
	  }
	}
      } else {
	LOGE(F("radiation driver status error -> previous:%T current:%T"),_is_previous_prepared,_is_current_prepared );
      }
      
      SensorDriver::printInfo();

      if (_is_success){
	LOGT(F("solarradiation get... [ %s ]"), OK_STRING);
      }else{
	LOGE(F("solarradiation get... [ %s ]"), FAIL_STRING);
      }

      if (length >= 1) {
	if (ISVALID_INT32(values[0])) {
	  LOGT(F("solarradiation--> solar radiation: %l"), values[0]);
	}
	else {
	  LOGT(F("solarradiation--> solar radiation: ---"));
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
void SensorDriverSolarRadiation::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverSolarRadiation::get(values, length, is_test);
  json_buffer[0]='\0';
  
  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
	json["B14198"] = values[0];
      }
      else json["B14198"] = nullptr;
    }

    if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
      json_buffer[0]='\0';
    }
  }
}
#endif

#endif

//------------------------------------------------------------------------------
// Optical Particle Countr (OPC)
// USE_SENSOR_OAx: PM1, PM2.5, PM10 average continuous
// USE_SENSOR_OBx: PM1, PM2.5, PM10 standard deviation continuous
// USE_SENSOR_OCx: Bins average continuous
// USE_SENSOR_ODx: Bins standard deviation continuous
// USE_SENSOR_OE3: Temperature and Humidity average continuous
//------------------------------------------------------------------------------
#if (USE_SENSOR_OA2 || USE_SENSOR_OB2 || USE_SENSOR_OC2 || USE_SENSOR_OD2 || USE_SENSOR_OA3 || USE_SENSOR_OB3 || USE_SENSOR_OC3 || USE_SENSOR_OD3 || USE_SENSOR_OE3)

void SensorDriverOpc::resetPrepared(bool is_test) {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverOpc::setup() {
  SensorDriver::printInfo();
  bool is_i2c_write;
  uint8_t i;
  _delay_ms = 0;
  _is_success = false;

  if (!*_is_setted) {
    memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
    is_i2c_write = false;
    i = 0;

    if ((strcmp(_type, SENSOR_TYPE_OA2) == 0) || (strcmp(_type, SENSOR_TYPE_OB2) == 0) || (strcmp(_type, SENSOR_TYPE_OC2) == 0) || (strcmp(_type, SENSOR_TYPE_OD2) == 0) || (strcmp(_type, SENSOR_TYPE_OA3) == 0) || (strcmp(_type, SENSOR_TYPE_OB3) == 0) || (strcmp(_type, SENSOR_TYPE_OC3) == 0) || (strcmp(_type, SENSOR_TYPE_OD3) == 0) || (strcmp(_type, SENSOR_TYPE_OE3) == 0)) {
      is_i2c_write = true;
      _buffer[i++] = I2C_COMMAND_ID;
      _buffer[i++] = I2C_OPC_COMMAND_CONTINUOUS_START;
      _buffer[i] = crc8(_buffer, i);
    }

    if (is_i2c_write) {
      Wire.beginTransmission(_address);
      Wire.write(_buffer, i+1);

      if (Wire.endTransmission() == 0) {
	_error_count = 0;
        _is_success = true;
        *_is_setted = true;
      }else{
	_error_count++;
      }
    }
  }
  else {
    _is_success = true;
  }

  LOGT(F("opc setup... [ %s ]"), _is_success ? OK_STRING : ERROR_STRING);
}

void SensorDriverOpc::prepare(bool is_test) {
  SensorDriver::printInfo();
  bool is_i2c_write;
  uint8_t i;
  _is_success = false;
  _is_test = is_test;

  if (!*_is_prepared) {
    memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
    is_i2c_write = false;
    i = 0;

    if ((strcmp(_type, SENSOR_TYPE_OA2) == 0) || (strcmp(_type, SENSOR_TYPE_OB2) == 0) || (strcmp(_type, SENSOR_TYPE_OC2) == 0) || (strcmp(_type, SENSOR_TYPE_OD2) == 0) || (strcmp(_type, SENSOR_TYPE_OA3) == 0) || (strcmp(_type, SENSOR_TYPE_OB3) == 0) || (strcmp(_type, SENSOR_TYPE_OC3) == 0) || (strcmp(_type, SENSOR_TYPE_OD3) == 0) || (strcmp(_type, SENSOR_TYPE_OE3) == 0)) {
      is_i2c_write = true;
      _buffer[i++] = I2C_COMMAND_ID;
      _buffer[i++] = I2C_OPC_COMMAND_CONTINUOUS_START_STOP;
      _buffer[i] = crc8(_buffer, i);
      _delay_ms = 0;
    }

    if (is_i2c_write) {
      Wire.beginTransmission(_address);
      Wire.write(_buffer, i+1);

      if (Wire.endTransmission() == 0) {
	_error_count = 0;
        _is_success = true;
        *_is_prepared = true;
      }else{
	_error_count++;
      }
    }
  }
  else {
    _is_success = true;
    _delay_ms = 0;
  }

  LOGT(F("opc prepare... [ %s ]"), _is_success ? OK_STRING : ERROR_STRING);
  _start_time_ms = millis();
}

void SensorDriverOpc::get(int32_t *values, uint8_t length, bool is_test) {

  bool is_i2c_write;
  uint8_t i;

  float val;
  uint8_t *val_ptr;

  switch (_get_state) {
    case INIT:
      for (uint8_t i =0; i < length; i++) {
	values[i]=INT32_MAX;
      }
      val = FLT_MAX;

      if ((strcmp(_type, SENSOR_TYPE_OA2) == 0) || (strcmp(_type, SENSOR_TYPE_OA3) == 0) || (strcmp(_type, SENSOR_TYPE_OB2) == 0) || (strcmp(_type, SENSOR_TYPE_OB3) == 0)) {
        variable_length = 3;
      }
      else if ((strcmp(_type, SENSOR_TYPE_OC2) == 0) || (strcmp(_type, SENSOR_TYPE_OD2) == 0)) {
        variable_length = 16;
      }
      else if ((strcmp(_type, SENSOR_TYPE_OC3) == 0) || (strcmp(_type, SENSOR_TYPE_OD3) == 0)) {
        variable_length = 24;
      }
      else if (strcmp(_type, SENSOR_TYPE_OE3) == 0) {
        variable_length = 2;
      }

      variable_count = 0;
      data_length = 0;

      _is_readed = false;
      _is_end = false;

      if (*_is_prepared && length >= 1) {
        _is_success = true;
        _get_state = SET_ADDRESS;
      }
      else {
        _is_success = false;
        _get_state = END;
      }

      _delay_ms = 0;
      _start_time_ms = millis();
      break;

    case SET_ADDRESS:
      memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
      is_i2c_write = false;
      offset = 0;
      i = 0;

      #if (USE_SENSOR_OA2 || USE_SENSOR_OA3)
      if ((strcmp(_type, SENSOR_TYPE_OA2) == 0) || (strcmp(_type, SENSOR_TYPE_OA3) == 0)) {
        is_i2c_write = true;
        data_length = I2C_OPC_PM_MED_LENGTH;
        _buffer[i++] = I2C_OPC_PM_MED_ADDRESS;
        _buffer[i++] = I2C_OPC_PM_MED_LENGTH;
        _buffer[i] = crc8(_buffer, i);
	i++;
      }
      #endif

      #if (USE_SENSOR_OB2 || USE_SENSOR_OB3)
      if ((strcmp(_type, SENSOR_TYPE_OB2) == 0) || (strcmp(_type, SENSOR_TYPE_OB3) == 0)) {
        is_i2c_write = true;
        data_length = I2C_OPC_PM_SIGMA_LENGTH;
        _buffer[i++] = I2C_OPC_PM_SIGMA_ADDRESS;
        _buffer[i++] = I2C_OPC_PM_SIGMA_LENGTH;
        _buffer[i] = crc8(_buffer, i);
	i++;
      }
      #endif

      #if (USE_SENSOR_OC2 || USE_SENSOR_OC3)
      if ((strcmp(_type, SENSOR_TYPE_OC2) == 0) || (strcmp(_type, SENSOR_TYPE_OC3) == 0)) {
        is_i2c_write = true;

        // BIN [18-23]
        if (variable_count >= 18) {
          data_length = I2C_OPC_BIN_18_23_MED_LENGTH;
          _buffer[i++] = I2C_OPC_BIN_18_23_MED_ADDRESS;
          _buffer[i++] = I2C_OPC_BIN_18_23_MED_LENGTH;
        }
        // BIN [12-17]
        else if (variable_count >= 12) {
          data_length = I2C_OPC_BIN_12_17_MED_LENGTH;
          _buffer[i++] = I2C_OPC_BIN_12_17_MED_ADDRESS;
          _buffer[i++] = I2C_OPC_BIN_12_17_MED_LENGTH;
        }
        // BIN [6-11]
        else if (variable_count >= 6) {
          data_length = I2C_OPC_BIN_6_11_MED_LENGTH;
          _buffer[i++] = I2C_OPC_BIN_6_11_MED_ADDRESS;
          _buffer[i++] = I2C_OPC_BIN_6_11_MED_LENGTH;
        }
        // BIN [0-5]
        else if (variable_count >= 0) {
          data_length = I2C_OPC_BIN_0_5_MED_LENGTH;
          _buffer[i++] = I2C_OPC_BIN_0_5_MED_ADDRESS;
          _buffer[i++] = I2C_OPC_BIN_0_5_MED_LENGTH;
        }

        _buffer[i] = crc8(_buffer, i);
	i++;
      }
      #endif

      #if (USE_SENSOR_OD2 || USE_SENSOR_OD3)
      if ((strcmp(_type, SENSOR_TYPE_OD2) == 0) || (strcmp(_type, SENSOR_TYPE_OD3) == 0)) {
        is_i2c_write = true;

        // BIN [18-23]
        if (variable_count >= 18) {
          data_length = I2C_OPC_BIN_18_23_SIGMA_LENGTH;
          _buffer[i++] = I2C_OPC_BIN_18_23_SIGMA_ADDRESS;
          _buffer[i++] = I2C_OPC_BIN_18_23_SIGMA_LENGTH;
        }
        // BIN [12-17]
        else if (variable_count >= 12) {
          data_length = I2C_OPC_BIN_12_17_SIGMA_LENGTH;
          _buffer[i++] = I2C_OPC_BIN_12_17_SIGMA_ADDRESS;
          _buffer[i++] = I2C_OPC_BIN_12_17_SIGMA_LENGTH;
        }
        // BIN [6-11]
        else if (variable_count >= 6) {
          data_length = I2C_OPC_BIN_6_11_SIGMA_LENGTH;
          _buffer[i++] = I2C_OPC_BIN_6_11_SIGMA_ADDRESS;
          _buffer[i++] = I2C_OPC_BIN_6_11_SIGMA_LENGTH;
        }
        // BIN [0-5]
        else if (variable_count >= 0) {
          data_length = I2C_OPC_BIN_0_5_SIGMA_LENGTH;
          _buffer[i++] = I2C_OPC_BIN_0_5_SIGMA_ADDRESS;
          _buffer[i++] = I2C_OPC_BIN_0_5_SIGMA_LENGTH;
        }

        _buffer[i] = crc8(_buffer, i);
	i++;
      }
      #endif

      #if (USE_SENSOR_OE3)
      if (strcmp(_type, SENSOR_TYPE_OE3) == 0) {
        is_i2c_write = true;
        data_length = I2C_OPC_TH_MED_LENGTH;
        _buffer[i++] = I2C_OPC_TH_MED_ADDRESS;
        _buffer[i++] = I2C_OPC_TH_MED_LENGTH;
        _buffer[i] = crc8(_buffer, i);
      }
      #endif

      if (is_i2c_write) {
        Wire.beginTransmission(_address);
        Wire.write(_buffer, i+1);

        if (Wire.endTransmission()) {
	  _error_count++;
          _is_success = false;
        }else{
	  _error_count = 0;
	}
      }

      _delay_ms = 0;
      _start_time_ms = millis();

      if (_is_success) {
        _get_state = READ_VALUE;
      }
      else {
        _get_state = END;
      }
      break;

    case READ_VALUE:

      if (_is_success) {
        Wire.requestFrom(_address, (uint8_t)(data_length + 1));
        if (Wire.available() < (data_length + 1)) {
          _is_success = false;
        }
      }

      if (_is_success) {
        memset(_buffer, UINT8_MAX, I2C_MAX_DATA_LENGTH * sizeof(uint8_t));
        for (i = 0; i < data_length; i++) {
          _buffer[i] = Wire.read();
        }

        if (crc8(_buffer, data_length) != Wire.read()) {
          _is_success = false;
        }
      }

      _delay_ms = 0;
      _start_time_ms = millis();

      if (_is_success) {
        _get_state = GET_VALUE;
      }
      else {
        _get_state = END;
      }
      break;

    case GET_VALUE:
      #if (USE_SENSOR_OA2 || USE_SENSOR_OA3 || USE_SENSOR_OB2 || USE_SENSOR_OB3)
      if ((strcmp(_type, SENSOR_TYPE_OA2) == 0) || (strcmp(_type, SENSOR_TYPE_OA3) == 0) || (strcmp(_type, SENSOR_TYPE_OB2) == 0) || (strcmp(_type, SENSOR_TYPE_OB3) == 0)) {
        if (length >= variable_count + 1) {
          val_ptr = (uint8_t*) &val;

          for (i = 0; i < 4; i++) {
            *(val_ptr + i) = _buffer[offset + i];
          }

          if (_is_success && ISVALID_FLOAT(val)) {
            values[variable_count] = (int32_t) round(val * 10.0);
          }
          else {
            values[variable_count] = INT32_MAX;
            _is_success = false;
          }

          offset += 4;
        }
      }
      #endif

      #if (USE_SENSOR_OC2 || USE_SENSOR_OC3 || USE_SENSOR_OD2 || USE_SENSOR_OD3)
      if ((strcmp(_type, SENSOR_TYPE_OC2) == 0) || (strcmp(_type, SENSOR_TYPE_OC3) == 0) || (strcmp(_type, SENSOR_TYPE_OD2) == 0) || (strcmp(_type, SENSOR_TYPE_OD3) == 0)) {
        if (length >= variable_count + 1) {
          val_ptr = (uint8_t*) &val;

          for (i = 0; i < 4; i++) {
            *(val_ptr + i) = _buffer[offset + i];
          }

          if (_is_success && ISVALID_FLOAT(val)) {
            if (variable_count >= 13) {
              val = val * 100000.0;
            }
            else if (variable_count >= 8) {
              val = val * 10000.0;
            }
            else if (variable_count >= 5) {
              val = val * 1000.0;
            }
            else if (variable_count >= 3) {
              val = val * 100.0;
            }
            else if (variable_count >= 2) {
              val = val * 10.0;
            }

            values[variable_count] = (int32_t) round(val);
          }
          else {
            values[variable_count] = INT32_MAX;
            _is_success = false;
          }

          offset += 4;
        }
      }
      #endif

      #if (USE_SENSOR_OE3)
      if (strcmp(_type, SENSOR_TYPE_OE3) == 0) {
        if (length >= variable_count + 1) {
          val_ptr = (uint8_t*) &val;

          for (i = 0; i < 4; i++) {
            *(val_ptr + i) = _buffer[offset + i];
          }

          if (_is_success && ISVALID_FLOAT(val)) {
            if (variable_count == 0) {
              values[variable_count] = (int32_t) round(val * 100.0) + SENSOR_DRIVER_C_TO_K;
            }
            else if (variable_count == 1) {
              values[variable_count] = (int32_t) round(val);
            }
          }
          else {
            values[variable_count] = INT32_MAX;
            _is_success = false;
          }

          offset += 4;
        }
      }
      #endif

      variable_count++;

      _delay_ms = 0;
      _start_time_ms = millis();

      if ((variable_count >= length) || (variable_count >= variable_length)) {
        _get_state = END;
      }
      else if (variable_count == 6 || variable_count == 12 || variable_count == 18 || variable_count == 24) {
        _get_state = SET_ADDRESS;
      }
      break;

    case END:
      SensorDriver::printInfo();
      if (_is_success){
	LOGT(F("opc get... [ %s ]"), OK_STRING);
      }else{
	LOGE(F("opc get... [ %s ]"), FAIL_STRING);
      }

      #if (USE_SENSOR_OA2 || USE_SENSOR_OA3)
      if ((strcmp(_type, SENSOR_TYPE_OA2) == 0) || (strcmp(_type, SENSOR_TYPE_OA3) == 0)) {
        if (length >= 1) {
          if (ISVALID_INT32(values[0])) {
            LOGT(F("opc--> PM 1: %ld"), values[0]);
          }
          else {
            LOGT(F("opc--> PM 1: ---"));
          }
        }

        if (length >= 2) {
          if (ISVALID_INT32(values[1])) {
            LOGT(F("opc--> PM 2.5: %ld"), values[1]);
          }
          else {
            LOGT(F("opc--> PM 2.5: ---"));
          }
        }

        if (length >= 3) {
          if (ISVALID_INT32(values[2])) {
            LOGT(F("opc--> PM 10: %ld"), values[2]);
          }
          else {
            LOGT(F("opc--> PM 10: ---"));
          }
        }
      }
      #endif

      #if (USE_SENSOR_OB2 || USE_SENSOR_OB3)
      if ((strcmp(_type, SENSOR_TYPE_OB2) == 0) || (strcmp(_type, SENSOR_TYPE_OB3) == 0)) {
        if (length >= 1) {
          if (ISVALID_INT32(values[0])) {
            LOGT(F("opc--> PM 1 sigma: %ld"), values[0]);
          }
          else {
            LOGT(F("opc--> PM 1 sigma: ---"));
          }
        }

        if (length >= 2) {
          if (ISVALID_INT32(values[1])) {
            LOGT(F("opc--> PM 2.5 sigma: %ld"), values[1]);
          }
          else {
            LOGT(F("opc--> PM 2.5 sigma: ---"));
          }
        }

        if (length >= 3) {
          if (ISVALID_INT32(values[2])) {
            LOGT(F("opc--> PM 10 sigma: %ld"), values[2]);
          }
          else {
            LOGT(F("opc--> PM 10 sigma: ---"));
          }
        }
      }
      #endif

      #if (USE_SENSOR_OC2 || USE_SENSOR_OC3)
      if ((strcmp(_type, SENSOR_TYPE_OC2) == 0) || (strcmp(_type, SENSOR_TYPE_OC3) == 0)) {
        if (length >= 1) {
          if (ISVALID_INT32(values[0])) {
            LOGT(F("opc--> BIN [0-%d]:\t[ "), variable_length - 1);
	    for (int i=0; i<variable_length; i++) {
	      LOGT(F("%l"),values[i]);
	    }
            LOGT(F(" ]"));
          }
          else {
            LOGT(F("opc--> BIN [0-%d]:\t[ "), variable_length - 1);
	    for (int i=0; i<variable_length; i++) {
	      LOGT(F("-"));
	    }
            LOGT(F(" ]"));
          }
        }
      }
      #endif

      #if (USE_SENSOR_OD2 || USE_SENSOR_OD3)
      if ((strcmp(_type, SENSOR_TYPE_OD2) == 0) || (strcmp(_type, SENSOR_TYPE_OD3) == 0)) {
        if (length >= 1) {
          if (ISVALID_INT32(values[0])) {
            LOGT(F("opc--> BIN sigma [0-%d]:\t[ "), variable_length - 1);
	    for (int i=0; i<variable_length; i++) {
	      LOGT(F("%l"),values[i]);
	    }
            LOGT(F(" ]"));
          }
          else {
            LOGT(F("opc--> BIN sigma [0-%d]:\t[ "), variable_length - 1);
	    for (int i=0; i<variable_length; i++) {
	      LOGT(F("-"));
	    }
            LOGT(F(" ]"));
          }
        }
      }
      #endif

      #if (USE_SENSOR_OE3)
      if (strcmp(_type, SENSOR_TYPE_OE3) == 0) {
        if (length >= 1) {
          if (ISVALID_INT32(values[0])) {
            LOGT(F("opc--> Temperature: %ld"), values[0]);
          }
          else {
            LOGT(F("opc--> Temperature: ---"));
          }
        }

        if (length >= 2) {
          if (ISVALID_INT32(values[1])) {
            LOGT(F("opc--> Humidity: %ld"), values[1]);
          }
          else {
            LOGT(F("opc--> Humidity: ---"));
          }
        }
      }
      #endif

      _start_time_ms = millis();
      _delay_ms = 0;
      _is_end = true;
      _is_readed = false;
      _get_state = INIT;
      break;
  }
}

#if (USE_JSON)
void SensorDriverOpc::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverOpc::get(values, length, is_test);
  json_buffer[0]='\0';
  uint8_t variable_length;

  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;

    #if (USE_SENSOR_OA2 || USE_SENSOR_OA3 || USE_SENSOR_OB2 || USE_SENSOR_OB3)
    if ((strcmp(_type, SENSOR_TYPE_OA2) == 0) || (strcmp(_type, SENSOR_TYPE_OA3) == 0) || (strcmp(_type, SENSOR_TYPE_OB2) == 0) || (strcmp(_type, SENSOR_TYPE_OB3) == 0)) {
      if (length >= 1) {
        if (ISVALID_INT32(values[0])) {
          json["B15203"] = values[0];
        }
        else json["B15203"] = nullptr;
      }

      if (length >= 2) {
        if (ISVALID_INT32(values[1])) {
          json["B15198"] = values[1];
        }
        else json["B15198"] = nullptr;
      }

      if (length >= 3) {
        if (ISVALID_INT32(values[2])) {
          json["B15195"] = values[2];
        }
        else json["B15195"] = nullptr;
      }
    }
    #endif

    #if (USE_SENSOR_OC2 || USE_SENSOR_OC3)
    if ((strcmp(_type, SENSOR_TYPE_OC2) == 0) || (strcmp(_type, SENSOR_TYPE_OD2) == 0)) {
      variable_length = 16;
    }
    else if ((strcmp(_type, SENSOR_TYPE_OC3) == 0) || (strcmp(_type, SENSOR_TYPE_OD3) == 0)) {
      variable_length = 13;
    }

    if ((strcmp(_type, SENSOR_TYPE_OC2) == 0) || (strcmp(_type, SENSOR_TYPE_OC3) == 0) || (strcmp(_type, SENSOR_TYPE_OD2) == 0) || (strcmp(_type, SENSOR_TYPE_OD3) == 0)) {
      if (length >= 1) {
        json["d"] = 52;

        JsonArray p = json.createNestedArray("p");

        for (uint8_t i = 0; i < variable_length; i++) {
          if (ISVALID_INT32(values[i])) {
            p.add(values[i]);
          }
          else p.add(nullptr);;
        }
      }
    }
    #endif

    #if (USE_SENSOR_OE3)
    if (strcmp(_type, SENSOR_TYPE_OE3) == 0) {
      if (length >= 1) {
        if (ISVALID_INT32(values[0])) {
          json["B12101"] = values[0];
        }
        else json["B12101"] = nullptr;
      }

      if (length >= 2) {
        if (ISVALID_INT32(values[1])) {
          json["B13003"] = values[1];
        }
        else json["B13003"] = nullptr;
      }
    }
    #endif

    if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
      json_buffer[0]='\0';
    }
  }
}
#endif

#endif

//------------------------------------------------------------------------------
// Leaf Wetness
// USE_SENSOR_LWT: Wetness timer continuous
//------------------------------------------------------------------------------
#if (USE_SENSOR_LWT)

void SensorDriverLeaf::resetPrepared(bool is_test) {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverLeaf::setup() {
  SensorDriver::printInfo();
  *_is_setted = true;
  _delay_ms = 0;
  LOGT(F(" setup... [ %s ]"), OK_STRING);
}

void SensorDriverLeaf::prepare(bool is_test) {
  SensorDriver::printInfo();
  bool is_i2c_write = false;
  uint8_t i = 0;
  _is_success = false;

  if (!*_is_prepared) {
    memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
    is_i2c_write = false;
    i = 0;

    if (strcmp(_type, SENSOR_TYPE_LWT) == 0) {
      is_i2c_write = true;
      _buffer[i++] = I2C_COMMAND_ID;

      if (is_test) {
        _buffer[i++] = I2C_LEAF_COMMAND_TEST_READ;
        _buffer[i] = crc8(_buffer, i);
      }
      else {
        _buffer[i++] = I2C_LEAF_COMMAND_ONESHOT_START_STOP;
        _buffer[i] = crc8(_buffer, i);
      }
      _delay_ms = 0;
    }

    if (is_i2c_write) {
      Wire.beginTransmission(_address);
      Wire.write(_buffer, i+1);

      if (Wire.endTransmission() == 0) {
	_error_count = 0;
        _is_success = true;
        *_is_prepared = true;
      }else{
	_error_count++;
      }
    }
  }
  else {
    _is_success = true;
    _delay_ms = 0;
  }

  LOGT(F("leaf prepare... [ %s ]"), _is_success ? OK_STRING : ERROR_STRING);

  _start_time_ms = millis();
}

void SensorDriverLeaf::get(int32_t *values, uint8_t length, bool is_test) {

  bool is_i2c_write;
  uint8_t i;

  float val;
  uint8_t *val_ptr;

  switch (_get_state) {
    case INIT:
      for (uint8_t i =0; i < length; i++) {
	values[i]=INT32_MAX;
      }

      if (strcmp(_type, SENSOR_TYPE_LWT) == 0) {
        val = FLT_MAX;
        variable_length = 1;
      }

      variable_count = 0;
      data_length = 0;

      _is_readed = false;
      _is_end = false;

      if (*_is_prepared && length >= 1) {
        _is_success = true;
        _get_state = SET_ADDRESS;
      }
      else {
        _is_success = false;
        _get_state = END;
      }

      _delay_ms = 0;
      _start_time_ms = millis();
      break;

    case SET_ADDRESS:
      memset(_buffer, 0, I2C_MAX_DATA_LENGTH);
      is_i2c_write = false;
      offset = 0;
      i = 0;

      #if (USE_SENSOR_LWT)
      if (strcmp(_type, SENSOR_TYPE_LWT) == 0) {
        is_i2c_write = true;
        data_length = I2C_LEAF_TIMER_LENGTH;
        _buffer[i++] = I2C_LEAF_TIMER_ADDRESS;
        _buffer[i++] = I2C_LEAF_TIMER_LENGTH;
        _buffer[i] = crc8(_buffer, i);
      }
      #endif

      if (is_i2c_write) {
        Wire.beginTransmission(_address);
        Wire.write(_buffer, i+1);

        if (Wire.endTransmission()) {
	  _error_count++;
          _is_success = false;
        }else{
	  _error_count = 0;
	}
      }

      _delay_ms = 0;
      _start_time_ms = millis();

      if (_is_success) {
        _get_state = READ_VALUE;
      }
      else {
        _get_state = END;
      }
      break;

    case READ_VALUE:
      if (_is_success) {
        Wire.requestFrom(_address,(uint8_t) ( data_length + 1));
        if (Wire.available() < (data_length + 1)) {
	  _error_count++;
          _is_success = false;
        }else{
	  _error_count = 0;
	}
      }

      if (_is_success) {
        memset(_buffer, UINT8_MAX, I2C_MAX_DATA_LENGTH * sizeof(uint8_t));
        for (i = 0; i < data_length; i++) {
          _buffer[i] = Wire.read();
        }

        if (crc8(_buffer, data_length) != Wire.read()) {
          _is_success = false;
        }
      }

      _delay_ms = 0;
      _start_time_ms = millis();

      if (_is_success) {
        _get_state = GET_VALUE;
      }
      else {
        _get_state = END;
      }
      break;

    case GET_VALUE:
      #if (USE_SENSOR_LWT)
      if (strcmp(_type, SENSOR_TYPE_LWT) == 0) {
        if (length >= variable_count + 1) {
          val_ptr = (uint8_t*) &val;

          for (i = 0; i < 4; i++) {
            *(val_ptr + i) = _buffer[offset + i];
          }

          if (_is_success && ISVALID_FLOAT(val)) {
            values[variable_count] = (uint16_t)(round(val / 10.0));
          }
          else {
            values[variable_count] = INT32_MAX;
            _is_success = false;
          }

          offset += 4;
        }
      }
      #endif

      variable_count++;

      _delay_ms = 0;
      _start_time_ms = millis();

      if ((variable_count >= length) || (variable_count >= variable_length)) {
        _get_state = END;
      }
      break;

    case END:
      SensorDriver::printInfo();
      if (_is_success){
	LOGT(F("leaf get... [ %s ]"), OK_STRING);
      }else{
	LOGE(F("leaf get... [ %s ]"), FAIL_STRING);
      }

      #if (USE_SENSOR_LWT)
      if (strcmp(_type, SENSOR_TYPE_LWT) == 0) {
        if (length >= 1) {
          if (ISVALID_INT32(values[0])) {
            LOGN(F("leaf--> Leaf Wet Time: %ld minutes"), values[0]);
          }
          else {
            LOGT(F("leaf--> Leaf Wet Time: --- minutes"));
          }
        }
      }
      #endif

      _start_time_ms = millis();
      _delay_ms = 0;
      _is_end = true;
      _is_readed = false;
      _get_state = INIT;
      break;
  }
}

#if (USE_JSON)
void SensorDriverLeaf::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverLeaf::get(values, length, is_test);
  json_buffer[0]='\0';

  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        json["B13212"] = values[0];
      }
      else json["B13212"] = nullptr;
    }

    if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
      json_buffer[0]='\0';
    }
  }
}
#endif

#endif


//------------------------------------------------------------------------------
// Sensor driver's SPS30 sensor type for Sensirion PM sensor support
//------------------------------------------------------------------------------
#if (USE_SENSOR_SPS)

void SensorDriverSps::resetPrepared(bool is_test) {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverSps::setup() {
  SensorDriver::printInfo();

  _delay_ms = 0;
  if (!*_is_setted) {

    //  WARNING !!!!! address is not used; sensirion have only one address
    if (_address != SPS30_ADDRESS) {
      _error_count++;
      LOGE(F("sps setup... wrong i2c address [ %s ]"), ERROR_STRING);
      return;
    }

    if (!_sps30.begin(I2C_COMMS)){
      _error_count++;
      LOGV(F("SPS probe error"));
      return;
    }
    if (!_sps30.probe()){
      _error_count++;
      LOGV(F("SPS probe error"));
      return;
    }
    
    if (!_sps30.reset()){
      _error_count++;
      LOGV(F("SPS reset error"));
      return;
    }
    delay(100);
    
    char buf[32];    
    if (_sps30.GetSerialNumber(buf, 32) != SPS30ERR_OK) {
      _error_count++;
      LOGE(F("SPS GetSerialNumber error [ %s ]"), ERROR_STRING);      
      return;
    }

    delay(10);

    if(!_sps30.start()){
      _error_count++;
      LOGE(F("sps setup... [ %s ]"), ERROR_STRING);
    }

    *_is_setted = true;
    _error_count = 0;
    LOGT(F("sps setup... [ %s ]"), YES_STRING);
  } 
}

void SensorDriverSps::prepare(bool is_test) {
  SensorDriver::printInfo();
  
  *_is_prepared =_sps30.start();
  if (*_is_prepared){
    _error_count = 0;
    LOGT(F("SPS prepare... [ %s ]"), OK_STRING);
  }else{
    _error_count++;    
    LOGE(F("SPS prepare... [ %s ]"), FAIL_STRING);
  }

  _delay_ms = 1000ul;
  _start_time_ms = millis();
}

void SensorDriverSps::get(int32_t *values, uint8_t length, bool is_test) {
  
  switch (_get_state) {
  case INIT:
    for (uint8_t i =0; i < length; i++) {
      values[i]=INT32_MAX;
    }
    
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

    // get data
    // data might not have been ready

    if (_sps30.GetValues(&val) == SPS30ERR_OK){
      _is_success = true;
      _error_count = 0;
      _get_state = END;
    } else {
      LOGE(F("sps get read error"));
      _error_count++;
      _is_success = false;
      _get_state = END;
    }
    _delay_ms = 0;
    _start_time_ms = millis();
    break;
    
  case END:
    
    if (_is_success ) {
      
      // get pm1
      if (length >= 1) {
	values[0] = round(val.MassPM1*10.) ;
      }
      
       // get pm2
      if (length >= 2) {
	values[1] = round(val.MassPM2*10.) ;
      }
      
      // get pm4
      if (length >= 3) {
	values[2] = round(val.MassPM4*10.) ;
      }
      
      // get pm10
      if (length >= 4) {
	values[3] = round(val.MassPM10*10.) ;
      }
      
      if (_sps30.I2C_expect() == 4){
	LOGV(F(" !!! Due to I2C buffersize only the SPS30 MASS concentration is available !!! \n"));
      }else{
	
	// number of particles with diameter 0.3 to 0.5 um in 0.1 L of air.
	if (length >= 5) {
	  values[4] = round(val.NumPM0 *10.);
	}
	
	// number of particles with diameter 0.5 to 1.0  um in 0.1 L of air.
	if (length >= 6) {
	  values[5] = round((val.NumPM1-val.NumPM0)*100.) ;
	}
	
	// number of particles with diameter 1.0 to 2.5 um in 0.1 L of air.
	if (length >= 7) {
	  values[6] = round((val.NumPM2-val.NumPM1)*1000.) ;
	}
	 
	// number of particles with diameter 2.5 to 5.0 (4.0) um in 0.1 L of air.
	if (length >= 8) {
	  values[7] = round((val.NumPM4-val.NumPM2)*10000.) ;
	}
	
	// number of particles with diameter 5.0 to 10 um in 0.1 L of air.
	if (length >= 9) {
	   values[8] = round((val.NumPM10-val.NumPM4)*100000.) ;
	}
	
	/*
	// get particulte size
	if (length >= 10) {
	values[9] = val.PartSize ;
	}
	*/
      }
    }
    
    SensorDriver::printInfo();

    if (_is_success){
      LOGT(F("sps get... [ %s ]"), OK_STRING);
    }else{
      LOGE(F("sps get... [ %s ]"), FAIL_STRING);
    }
    
    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
	LOGT(F("sps--> PM1: %d"), values[0]);
      } else {
	LOGT(F("sps--> PM1: ---"));
      }
    }
    
    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
	LOGT(F("sps--> PM2.5: %d"), values[1]);
      } else {
	LOGT(F("sps--> PM2.5: ---"));
      }
    }
    
    if (length >= 3) {
      if (ISVALID_INT32(values[2])) {
	LOGT(F("sps--> PM5: %d"), values[2]);
      } else {
	LOGT(F("sps--> PM5: ---"));
      }
    }
    
    if (length >= 4) {
      if (ISVALID_INT32(values[3])) {
	LOGT(F("sps--> PM10: %d"), values[3]);
      } else {
	LOGT(F("sps--> PM10: ---"));
      }
    }
    
    if (length >= 5) {
      if (ISVALID_INT32(values[4])) {
	LOGT(F("sps-->number 0.3 to 0.5 um in 0.1 L: %d"), values[4]);
      } else {
	LOGT(F("sps--> number 1: ---"));
      }
    }
    
    if (length >= 6) {
      if (ISVALID_INT32(values[5])) {
	LOGT(F("sps-->number 0.5 to 1 um in 0.1 L: %d"), values[5]);
      } else {
	LOGT(F("sps--> number 2: ---"));
      }
    }
    
    if (length >= 7) {
      if (ISVALID_INT32(values[6])) {
	LOGT(F("sps-->number 1.0 to 2.5 um in 0.1 L: %d"), values[6]);
      } else {
	LOGT(F("sps--> number 3: ---"));
      }
    }
    
    if (length >= 8) {
      if (ISVALID_INT32(values[7])) {
	LOGT(F("sps-->number 2.5 to 5.0 um in 0.1 L: %d"), values[7]);
      } else {
	LOGT(F("sps--> number 4: ---"));
      }
    }
    
    if (length >= 9) {
      if (ISVALID_INT32(values[8])) {
	LOGT(F("sps-->number 5.0 to 10.0 um in 0.1 L: %d"), values[8]);
      } else {
	LOGT(F("sps--> number 5: ---"));
      }
    }
    
    _delay_ms = 0;
    _start_time_ms = millis();
    _is_end = true;
    _is_readed = false;
    _get_state = INIT;
    break;
  }
}

#if (USE_JSON)
void SensorDriverSps::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverSps::get(values, length, is_test);
  json_buffer[0]='\0';

  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        json["B15203"] = values[0];
      }
      else json["B15203"] = nullptr;
    }

    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
        json["B15198"] = values[1];
      }
      else json["B15198"] = nullptr;
    }

    if (length >= 3) {
      if (ISVALID_INT32(values[2])) {
        json["B15202"] = values[2];
      }
      else json["B15202"] = nullptr;
    }
    
    if (length >= 4) {
      if (ISVALID_INT32(values[3])) {
        json["B15195"] = values[3];
      }
      else json["B15195"] = nullptr;
    }
    
    if (length >= 5) {
      if (ISVALID_INT32(values[4])) {
        json["B49193"] = values[4];
      }
      else json["B49193"] = nullptr;
    }
    
    if (length >= 6) {
      if (ISVALID_INT32(values[5])) {
        json["B49194"] = values[5];
      }
      else json["B49194"] = nullptr;
    }

    if (length >= 7) {
      if (ISVALID_INT32(values[6])) {
        json["B49195"] = values[6];
      }
      else json["B49195"] = nullptr;
    }
    
    if (length >= 8) {
      if (ISVALID_INT32(values[7])) {
        json["B49196"] = values[7];
      }
      else json["B49196"] = nullptr;
    }
    
    if (length >= 9) {
      if (ISVALID_INT32(values[8])) {
        json["B49197"] = values[8];
      }
      else json["B49197"] = nullptr;
    }
    
    if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
      json_buffer[0]='\0';
    }
  }
}
#endif

#endif
    

//------------------------------------------------------------------------------
// Sensor driver's SCD30 sensor type for Sensirion NDIR CO2 sensor support
//------------------------------------------------------------------------------
#if (USE_SENSOR_SCD)

void SensorDriverScd::resetPrepared(bool is_test) {
  _get_state = INIT;
  *_is_prepared = false;
}

void SensorDriverScd::setup() {
  SensorDriver::printInfo();

  _delay_ms = 0;
  if (!*_is_setted) {
    _scd30.begin(_address);  //This will cause readings to occur every two seconds

    /*
      Maximal I2C speed is 100 kHz and the master has to support clock
      stretching. Clock stretching period in write- and read- frames is 12
      ms, however, due to internal calibration processes a maximal clock
      stretching of 150 ms may occur once per day.  For detailed information
      to the I2C protocol, refer to NXP I2C-bus specification 1 . SCD30 does
      not support repeated start condition. Clock stretching is necessary to
      start the microcontroller and might occur before every ACK. I2C master
      clock stretching needs to be implemented according to the NXP
      specification. The boot-up time is < 2 s.
  */
    _scd30.sendCommand(COMMAND_SOFT_RESET);
    delay(50);  // ??? not explained in documentation
    if(!_scd30.beginMeasuring()) { //Start continuous measurements
      _error_count++;
      LOGE(F("SCD beginMeasuring error"));
      //LOGE(F("SCD setup... [ %s ]"), ERROR_STRING);
      return;
    }
    if(!_scd30.setMeasurementInterval(2)) { //2 seconds between measurements
      _error_count++;
      LOGE(F("SCD setMeasurementInterval error"));
      //LOGE(F("SCD setup... [ %s ]"), ERROR_STRING);
      return;
    }
    if (!_scd30.setAutoSelfCalibration(true)) { //Enable auto-self-calibration
      _error_count++;
      LOGE(F("SCD setAutoSelfCalibration error"));
      //LOGE(F("SCD setup... [ %s ]"), ERROR_STRING);
      return;
    }	  

    *_is_setted = true;
    _error_count = 0;
    LOGT(F("SCD setup... [ %s ]"), YES_STRING);
  } 
}

void SensorDriverScd::prepare(bool is_test) {
  SensorDriver::printInfo();
  
  // clear previous measurements
  _scd30.getCO2();
  _scd30.getTemperature();
  _scd30.getHumidity();
  
  //_scd30->beginMeasuring();
  *_is_prepared =true;
  _error_count = 0;
  LOGT(F("SCD prepare... [ %s ]"), OK_STRING);

  /*
  if (*_is_prepared){
    _error_count = 0;
    LOGT(F("SCD prepare... [ %s ]"), OK_STRING);
  }else{
    _error_count++;    
    LOGE(F("SCD prepare... [ %s ]"), FAIL_STRING);
  }
  */
  
  _delay_ms = 2500ul;
  _start_time_ms = millis();
}

void SensorDriverScd::get(int32_t *values, uint8_t length, bool is_test) {
  
  switch (_get_state) {
  case INIT:
    for (uint8_t i =0; i < length; i++) {
      values[i]=INT32_MAX;
    }
    
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

    // get data
    if (_scd30.dataAvailable()){
  
      // measure
      // get CO2
      if (length >= 1) {
	
	// Campo di misura di uno strumento tipco da 0 a 10000 ppm CO₂ Risoluzione 1 ppm CO₂
	//https://www.lenntech.com/calculators/ppm/converter-parts-per-million.htm 1.938   ?????
	// 1,800009 coefficiente di conversione ppm-> mg/m3   (riferito a 25°C e 760 mm Hg)
	// quindi assumioamo 0-0.020 Kg/m3 con risoluzione 0.000002  circa
	// in interi fattore di scala in tabella B 10**6 quindi mg/m3
	
	values[0] =  _scd30.getCO2()* 1.8;
      }
      
      // get temperature
      if (length >= 2) {
	values[1] = _scd30.getTemperature()*100+27315 ;
      }
      
      // get humidity
      if (length >= 3) {
	values[2] = _scd30.getHumidity() ;
      }

      //_scd30.sendCommand(COMMAND_STOP_CONTINUOS_MEASUREMENT);
      
      _is_success = true;
      _error_count = 0;
      _get_state = END;
    } else {
      LOGE(F("SCD get read error"));
      _error_count++;
      _is_success = false;
      _get_state = END;
    }
    _delay_ms = 0;
    _start_time_ms = millis();
    break;
    
  case END:
    
    SensorDriver::printInfo();

    if (_is_success){
      LOGT(F("SCD get... [ %s ]"), OK_STRING);
    }else{
      LOGE(F("SCD get... [ %s ]"), FAIL_STRING);
    }
    
    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
	LOGT(F("SCD--> CO2: %d"), values[0]);
      } else {
	LOGT(F("SCD--> CO2: ---"));
      }
    }
    
    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
	LOGT(F("SCD--> temp: %d"), values[1]);
      } else {
	LOGT(F("SCD--> temp: ---"));
      }
    }
    
    if (length >= 3) {
      if (ISVALID_INT32(values[2])) {
	LOGT(F("SCD--> humid: %d"), values[2]);
      } else {
	LOGT(F("SCD--> humid: ---"));
      }
    }
    
    _delay_ms = 0;
    _start_time_ms = millis();
    _is_end = true;
    _is_readed = false;
    _get_state = INIT;
    break;
  }
}

#if (USE_JSON)
void SensorDriverScd::getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length, bool is_test) {
  SensorDriverScd::get(values, length, is_test);
  json_buffer[0]='\0';

  if (_is_end && !_is_readed) {
    StaticJsonDocument<JSON_BUFFER_LENGTH> json;

    if (length >= 1) {
      if (ISVALID_INT32(values[0])) {
        json["B15242"] = values[0];
      }
      else json["B15242"] = nullptr;
    }

    if (length >= 2) {
      if (ISVALID_INT32(values[1])) {
        json["B12101"] = values[1];
      }
      else json["B12101"] = nullptr;
    }

    if (length >= 3) {
      if (ISVALID_INT32(values[2])) {
        json["B13003"] = values[2];
      }
      else json["B13003"] = nullptr;
    }
    
    if (serializeJson(json,json_buffer, json_buffer_length) == json_buffer_length){
      json_buffer[0]='\0';
    }
  }
}
#endif

#endif
	
