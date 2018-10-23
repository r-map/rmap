/**@file SensorDriver.h */

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

#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include <debug.h>
#include "SensorDriverSensors.h"
#include <sensors_config.h>
#include <Arduino.h>
#include <Wire.h>

/*!
\def SENSOR_DRIVER_ERROR
\brief Sensor driver's error state.
*/
#define SENSOR_DRIVER_ERROR       (1)

/*!
\def SENSOR_DRIVER_SUCCESS
\brief Sensor driver's success state.
*/
#define SENSOR_DRIVER_SUCCESS     (0)

/*!
\def SENSOR_DRIVER_C_TO_K
\brief Kelvin to Celsius constant conversion.
*/
#define SENSOR_DRIVER_C_TO_K      (27315l)

#define isValid(v)                ((uint16_t) v != UINT16_MAX)

#if (USE_JSON)
#include <json_config.h>
#include <ArduinoJson.h>
#endif

/*!
\class SensorDriver
\brief SensorDriver class.
*/
class SensorDriver {
public:

   /*!
   \fn SensorDriver(const char* driver, const char* type)
   \brief Constructor for SensorDriver.
   \param[in] *driver driver's type.
   \param[in] *type sensor's type.
   \return void.
   */
   SensorDriver(const char* driver, const char* type);

   /*!
   \fn SensorDriver *create(const char* driver, const char* type)
   \brief Create an instance of SensorDriver for specific sensor.
   \param[in] *driver driver's type.
   \param[in] *type sensor's type.
   \return instance of SensorDriver for specified sensor.
   */
   static SensorDriver *create(const char* driver, const char* type);

   /*!
   \fn void createAndSetup(const char* driver, const char* type, uint8_t address, SensorDriver *sensors[], uint8_t *sensors_count)
   \brief Create and setup the specified sensor.
   \param[in] *driver driver's type.
   \param[in] *type sensor's type.
   \param[in] address sensor's address.
   \param[in] node sensor's node.
   \param[in] *sensors[] array of sensors.
   \param[in] *sensors_count setted sensors count.
   \return void.
   */
   static void createAndSetup(const char* driver, const char* type, const uint8_t address, const uint8_t node, SensorDriver *sensors[], uint8_t *sensors_count);

   /*!
   \fn void setup(const uint8_t address, const uint8_t node = 0)
   \brief Setup sensor.
   \param[in] address sensor's address.
   \param[in] node sensor's node.
   \return void.
   */
   virtual void setup(const uint8_t address, const uint8_t node = 0);

   /*!
   \fn void prepare()
   \brief Prepare sensor.
   \return void.
   */
   virtual void prepare();

   /*!
   \fn void get(int32_t *values, uint8_t length)
   \brief Get value from sensor.
   \param[out] *values pointer to array for getting multiple sensor's value.
   \param[in] length number of values readed from sensor.
   \return void.
   */
   virtual void get(int32_t *values, uint8_t length);

   #if (USE_JSON)
   /*!
   \fn void getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH)
   \brief Get value from sensor in JSON string format.
   \param[out] *values pointer to array for getting multiple sensor's value.
   \param[in] length number of values readed from sensor.
   \param[out] *json_buffer pointer to buffer for getting JSON string.
   \param[in] json_buffer_length maximum length of JSON string.s
   \return void.
   */
   virtual void getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH);
   #endif

   /*!
   \fn char *getDriver()
   \brief Get the sensor's driver.
   \return the sensor's driver.
   */
   const char *getDriver();

   /*!
   \fn char *getType()
   \brief Get the sensor's type.
   \return the sensor's type.
   */
   const char *getType();

   /*!
   \fn uint8_t getAddress()
   \brief Get the sensor's address.
   \return the sensor's address.
   */
   uint8_t getAddress();

   /*!
   \fn uint8_t getNode()
   \brief Get the sensor's node.
   \return the sensor's node.
   */
   uint8_t getNode();

   /*!
   \fn uint32_t getStartTime()
   \brief Get the last setted sensor's start time.
   \return the sensor's start time.
   */
   uint32_t getStartTime();

   /*!
   \fn uint32_t getDelay()
   \brief Get the last setted sensor's delay.
   \return the sensor's delay.
   */
   uint32_t getDelay();

   /*!
   \fn bool isEnd()
   \brief Check if get sequence is complete.
   \return true if complete, false otherwise.
   */
   bool isEnd();

   /*!
   \fn bool isSuccess()
   \brief Check if get sequence is complete with success.
   \return true if success, false otherwise.
   */
   bool isSuccess();

   /*!
   \fn bool isReaded()
   \brief Check if values were readed from sensor.
   \return true if readed, false otherwise.
   */
   bool isReaded();

   /*!
   \fn bool isSetted()
   \brief Check if sensor was setted.
   \return true if setted, false otherwise.
   */
   virtual bool isSetted();

   /*!
   \fn bool isPrepared()
   \brief Check if sensor was preapared.
   \return true if preapared, false otherwise.
   */
   virtual bool isPrepared();

   /*!
   \fn void resetPrepared()
   \brief Reset preapred internal state of sensor.
   \return void.
   */
   virtual void resetPrepared();

protected:
   /*!
   \var _driver
   \brief Internal sensor's variable for driver.
   */
   const char* _driver;

   /*!
   \var _type
   \brief Internal sensor's variable for type.
   */
   const char* _type;

   /*!
   \var _address
   \brief Internal sensor's variable for address.
   */
   uint8_t _address;

   /*!
   \var _node
   \brief Internal sensor's variable for node.
   */
   uint8_t _node;

   /*!
   \var _delay_ms
   \brief Internal sensor's variable for delay.
   */
   uint32_t _delay_ms;

   /*!
   \var _start_time_ms
   \brief Internal sensor's variable for start time milliseconds.
   */
   uint32_t _start_time_ms;

   /*!
   \var values[]
   \brief Internal sensor's variable for values readed from sensors.
   */
   int32_t values[];

   /*!
   \var _is_end
   \brief Internal sensor's variable for save end of reading values.
   */
   bool _is_end;

   /*!
   \var _is_success
   \brief Internal sensor's variable for save if readed was successful.
   */
   bool _is_success;

   /*!
   \var _is_readed
   \brief Internal sensor's variable for save is readed.
   */
   bool _is_readed;

   /*!
   \fn void printInfo(const char* driver, const char* type, const uint8_t address = 0, const uint8_t node = 0)
   \brief Print information about sensor
   \param[in] *driver the sensor's driver.
   \param[in] *type the sensor's type.
   \param[in] address the sensor's address.
   \param[in] node the sensor's node.
   \return void.
   */
   static void printInfo(const char* driver, const char* type, const uint8_t address = 0, const uint8_t node = 0);
};

#if (USE_SENSOR_ADT)
class SensorDriverAdt7420 : public SensorDriver {
public:
   SensorDriverAdt7420(const char* driver, const char* type, bool *is_setted, bool *is_prepared) : SensorDriver(driver, type) {
      _is_setted = is_setted;
      _is_prepared = is_prepared;

      *_is_setted = false;
      *_is_prepared = false;

      SensorDriver::printInfo(driver, type);
      SERIAL_DEBUG(F(" create... [ %s ]\r\n"), OK_STRING);
   };
   void setup(const uint8_t address, const uint8_t node = 0);
   void prepare();
   void get(int32_t *values, uint8_t length);

   #if (USE_JSON)
   void getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH);
   #endif

   bool isSetted();
   bool isPrepared();
   void resetPrepared();

protected:
   bool *_is_setted;
   bool *_is_prepared;

   enum {
      INIT,
      READ,
      END
   } _get_state;
};
#endif

#if (USE_SENSOR_HIH)
class SensorDriverHih6100 : public SensorDriver {
public:
   SensorDriverHih6100(const char* driver, const char* type, bool *is_setted, bool *is_prepared) : SensorDriver(driver, type) {
      _is_setted = is_setted;
      _is_prepared = is_prepared;

      *_is_setted = false;
      *_is_prepared = false;

      SensorDriver::printInfo(driver, type);
      SERIAL_DEBUG(F(" create... [ %s ]\r\n"), OK_STRING);
   };
   void setup(const uint8_t address, const uint8_t node = 0);
   void prepare();
   void get(int32_t *values, uint8_t length);

   #if (USE_JSON)
   void getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH);
   #endif

   bool isSetted();
   bool isPrepared();
   void resetPrepared();

protected:
   bool *_is_setted;
   bool *_is_prepared;

   enum {
      INIT,
      READ,
      END
   } _get_state;
};
#endif

#if (USE_SENSOR_HYT)
#include <hyt2x1.h>
class SensorDriverHyt2X1 : public SensorDriver {
public:
   SensorDriverHyt2X1(const char* driver, const char* type, bool *is_setted, bool *is_prepared) : SensorDriver(driver, type) {
      _is_setted = is_setted;
      _is_prepared = is_prepared;

      *_is_setted = false;
      *_is_prepared = false;

      SensorDriver::printInfo(driver, type);
      SERIAL_DEBUG(F(" create... [ %s ]\r\n"), OK_STRING);
   };
   void setup(const uint8_t address, const uint8_t node = 0);
   void prepare();
   void get(int32_t *values, uint8_t length);

   #if (USE_JSON)
   void getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH);
   #endif

   bool isSetted();
   bool isPrepared();
   void resetPrepared();

protected:
   bool *_is_setted;
   bool *_is_prepared;

   enum {
      INIT,
      READ,
      END
   } _get_state;
};
#endif

#if (USE_SENSOR_DW1)
#include <math.h>
#include "registers-windsonic.h"
class SensorDriverDw1 : public SensorDriver {
public:
   SensorDriverDw1(const char* driver, const char* type, bool *is_setted, bool *is_prepared) : SensorDriver(driver, type) {
      _is_setted = is_setted;
      _is_prepared = is_prepared;

      *_is_setted = false;
      *_is_prepared = false;

      SensorDriver::printInfo(driver, type);
      SERIAL_DEBUG(F(" create... [ %S ]\r\n"), OK_STRING);
   };
   void setup(const uint8_t address, const uint8_t node = 0);
   void prepare();
   void get(int32_t *values, uint8_t length);
   void getSDfromUV(int32_t u, int32_t v, double *speed, double *direction);

   #if (USE_JSON)
   void getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH);
   #endif

   bool isSetted();
   bool isPrepared();
   void resetPrepared();

protected:
   bool *_is_setted;
   bool *_is_prepared;

   enum {
      INIT,
      SET_MEANU_ADDRESS,
      READ_MEANU,
      SET_MEANV_ADDRESS,
      READ_MEANV,
      ELABORATE,
      END
   } _get_state;
};
#endif

#if (USE_SENSOR_TBS || USE_SENSOR_TBR)
#include "registers-rain.h"
class SensorDriverRain : public SensorDriver {
public:
   SensorDriverRain(const char* driver, const char* type, bool *is_setted, bool *is_prepared) : SensorDriver(driver, type) {
      _is_setted = is_setted;
      _is_prepared = is_prepared;

      *_is_setted = false;
      *_is_prepared = false;

      SensorDriver::printInfo(driver, type);
      SERIAL_DEBUG(F(" create... [ %s ]\r\n"), OK_STRING);
   };
   void setup(const uint8_t address, const uint8_t node = 0);
   void prepare();
   void get(int32_t *values, uint8_t length);

   #if (USE_JSON)
   void getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH);
   #endif

   bool isSetted();
   bool isPrepared();
   void resetPrepared();

protected:
   bool *_is_setted;
   bool *_is_prepared;

   enum {
      INIT,
      SET_RAIN_ADDRESS,
      READ_RAIN,
      END
   } _get_state;
};
#endif

#if (USE_SENSOR_STH || USE_SENSOR_ITH || USE_SENSOR_MTH || USE_SENSOR_NTH || USE_SENSOR_XTH)
#include "registers-th.h"
class SensorDriverTh : public SensorDriver {
public:
   SensorDriverTh(const char* driver, const char* type, bool *is_setted, bool *is_prepared) : SensorDriver(driver, type) {
      _is_setted = is_setted;
      _is_prepared = is_prepared;

      *_is_setted = false;
      *_is_prepared = false;

      SensorDriver::printInfo(driver, type);
      SERIAL_DEBUG(F(" create... [ %S ]\r\n"), OK_STRING);
   };
   void setup(const uint8_t address, const uint8_t node = 0);
   void prepare();
   void get(int32_t *values, uint8_t length);

   #if (USE_JSON)
   void getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH);
   #endif

   bool isSetted();
   bool isPrepared();
   void resetPrepared();

protected:
   bool *_is_setted;
   bool *_is_prepared;

   enum {
      INIT,
      SET_TEMPERATURE_ADDRESS,
      READ_TEMPERATURE,
      SET_HUMIDITY_ADDRESS,
      READ_HUMIDITY,
      END
   } _get_state;
};
#endif

#if (USE_SENSOR_DEP)
#include "digiteco_power.h"
class SensorDriverDigitecoPower : public SensorDriver {
public:
   SensorDriverDigitecoPower(const char* driver, const char* type, bool *is_setted, bool *is_prepared) : SensorDriver(driver, type) {
      _is_setted = is_setted;
      _is_prepared = is_prepared;

      *_is_setted = false;
      *_is_prepared = false;

      SensorDriver::printInfo(driver, type);
      SERIAL_DEBUG(F(" create... [ %s ]\r\n"), OK_STRING);
   };
   void setup(const uint8_t address, const uint8_t node = 0);
   void prepare();
   void get(int32_t *values, uint8_t length);

   #if (USE_JSON)
   void getJson(int32_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH);
   #endif

   bool isSetted();
   bool isPrepared();
   void resetPrepared();

protected:
   bool *_is_setted;
   bool *_is_prepared;

   enum {
      INIT,
      SET_BATTERY_CHARGE_ADDRESS,
      READ_BATTERY_CHARGE,
      SET_BATTERY_VOLTAGE_ADDRESS,
      READ_BATTERY_VOLTAGE,
      SET_BATTERY_CURRENT_ADDRESS,
      READ_BATTERY_CURRENT,
      SET_INPUT_VOLTAGE_ADDRESS,
      READ_INPUT_VOLTAGE,
      SET_INPUT_CURRENT_ADDRESS,
      READ_INPUT_CURRENT,
      SET_OUTPUT_VOLTAGE_ADDRESS,
      READ_OUTPUT_VOLTAGE,
      END
   } _get_state;
};
#endif

#endif
