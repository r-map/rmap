/**@file SensorDriver.h */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
authors:
Paolo patruno <p.patruno@iperbole.bologna.it>
Marco Baldinetti <m.baldinetti@digiteco.it>

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

#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include "SensorDriverSensors.h"
#include "debug_config.h"
#include "sensors_config.h"
#include "local_typedef.h"
#include "stima_utility.h"
#include "i2c_utility.h"
#include "os_port.h"

#include "debug_F.h"

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

#define SENSOR_DRIVER_STRING      ("SensorDriver")


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
   SensorDriver(const char* driver, const char* type, TwoWire *wire = &Wire);

   /*!
   \fn SensorDriver *create(const char* driver, const char* type)
   \brief Create an instance of SensorDriver for specific sensor.
   \param[in] *driver driver's type.
   \param[in] *type sensor's type.
   \return instance of SensorDriver for specified sensor.
   */
   static SensorDriver *create(const char* driver, const char* type, TwoWire *sdr_wire = &Wire);

   /*!
   \fn void createSensor(const char* driver, const char* type, uint8_t address, SensorDriver *sensors[], uint8_t *sensors_count)
   \brief Create and setup the specified sensor.
   \param[in] *driver driver's type.
   \param[in] *type sensor's type.
   \param[in] address sensor's address.
   \param[in] node sensor's node.
   \param[in] *sensors[] array of sensors.
   \param[in] *sensors_count setted sensors count.
   \return void.
   */
   static void createSensor(const char* driver, const char* type, const uint8_t address, const uint8_t node, SensorDriver *sensors[], TwoWire *sdr_wire = &Wire);


   /*!
   \fn void init(const uint8_t address, const uint8_t node, bool *is_setted, bool *is_prepared)
   \brief Initialize sensor.
   \param[in] address sensor's address.
   \param[in] node sensor's node.
   \param[in] *is_setted sensor's status.
   \param[in] *is_prepared sensor's status.
   \return void.
   */
   void init(const uint8_t address, const uint8_t node, bool *is_setted, bool *is_prepared);

   /*!
   \fn void setup()
   \brief Setup sensor.
   \param[in] address sensor's address.
   \param[in] node sensor's node.
   \param[in] *is_setted sensor's status.
   \param[in] *is_prepared sensor's status.
   \return void.
   */
   virtual void setup();

   /*!
   \fn void prepare()
   \brief Prepare sensor.
   \param[in] test set test mode.
   \return void.
   */
   virtual void prepare(bool is_test = false);

   /*!
   \fn void get(rmapdata_t *values, uint8_t length)
   \brief Get value from sensor.
   \param[out] *values pointer to array for getting multiple sensor's value.
   \param[in] length number of values readed from sensor.
   \param[in] test set test mode.
   \return void.
   */
   virtual void get(rmapdata_t *values, uint8_t length, bool is_test=false);

   #if (USE_JSON)
   /*!
   \fn void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH)
   \brief Get value from sensor in JSON string format.
   \param[out] *values pointer to array for getting multiple sensor's value.
   \param[in] length number of values readed from sensor.
   \param[out] *json_buffer pointer to buffer for getting JSON string.
   \param[in] json_buffer_length maximum length of JSON string.s
   \param[in] test set test mode.
   \return void.
   */
   virtual void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH, bool is_test=false);
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
   bool isSetted();

   /*!
   \fn bool isPrepared()
   \brief Check if sensor was preapared.
   \return true if preapared, false otherwise.
   */
   bool isPrepared();

   /*!
   \fn void resetSetted()
   \brief Reset setted internal state of sensor.
   \return void.
   */
   void resetSetted();

   /*!
   \fn void resetPrepared(bool is_test = false)
   \brief Reset preapred internal state of sensor.
   \param[in] test set test mode.
   \return void.
   */
   virtual void resetPrepared(bool is_test = false);

   /*!
   \fn uint8_t getErrorCount()
   \brief Get the sensor's error count.
   \return the sensor's errors count fron the last i2c transaction with success.
   */
   uint16_t getErrorCount();

   static uint8_t getSensorsCount();

protected:
   TwoWire *_sdr_wire;

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
   \var buffer
   \brief buffer.
   */
   uint8_t _buffer[I2C_MAX_DATA_LENGTH];

   /*!
   \var _error_count
   \brief Internal sensor's counter of errors. Every i2c transaction with success the counter is reset.
   */
   uint16_t _error_count;

   bool _is_test;
   bool *_is_setted;
   bool *_is_prepared;
   bool _is_previous_prepared;
   bool _is_current_prepared;

   /*!
   \fn void printInfo(const char* driver, const char* type, const uint8_t address = 0, const uint8_t node = 0)
   \brief Print information about sensor
   \param[in] *driver the sensor's driver.
   \param[in] *type the sensor's type.
   \param[in] address the sensor's address.
   \param[in] node the sensor's node.
   \return void.
   */
   void printInfo();
   void printCreate();
   void printInit();

};

#if (USE_SENSOR_ADT)
class SensorDriverAdt7420 : public SensorDriver {
public:
   SensorDriverAdt7420(const char* driver, const char* type, TwoWire *local_wire) : SensorDriver(driver, type) {
      _sdr_wire = local_wire;
      SensorDriver::printInfo();
   };
   void setup();
   void prepare(bool is_test = false);
   void get(rmapdata_t *values, uint8_t length, bool is_test=false);

   #if (USE_JSON)
   void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH, bool is_test=false);
   #endif

   void resetPrepared(bool is_test = false);

protected:

  int temperature;

   enum {
      INIT,
      READ,
      END
   } _get_state;

   /*!
   \var values[]
   \brief Internal sensor's variable for values readed from sensors.
   */
   rmapdata_t values[];

};
#endif

#if (USE_SENSOR_HIH)
class SensorDriverHih6100 : public SensorDriver {
public:
   SensorDriverHih6100(const char* driver, const char* type, TwoWire *local_wire) : SensorDriver(driver, type) {
      _sdr_wire = local_wire;
      SensorDriver::printInfo();      
   };
   void setup();
   void prepare(bool is_test = false);
   void get(rmapdata_t *values, uint8_t length, bool is_test=false);

   #if (USE_JSON)
   void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH, bool is_test=false);
   #endif

   void resetPrepared(bool is_test = false);

protected:

   uint16_t temperature;
   uint16_t humidity;

   enum {
      INIT,
      READ,
      END
   } _get_state;

   /*!
   \var values[]
   \brief Internal sensor's variable for values readed from sensors.
   */
   rmapdata_t values[];
};

#endif

#if (USE_SENSOR_HYT)
#include <hyt.h>
class SensorDriverHyt : public SensorDriver {

public:
  SensorDriverHyt(const char* driver, const char* type, TwoWire *wire = &Wire) : SensorDriver(driver, type, wire) {
    SensorDriver::printCreate();
  };
  void setup();
  void prepare(bool is_test = false);
  void get(rmapdata_t *values, uint8_t length, bool is_test=false);

  #if (USE_JSON)
  void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH, bool is_test=false);
  #endif

  void resetPrepared(bool is_test = false);

private:
  Hyt hyt;

protected:
  float humidity;
  float temperature;
  float humidity_confirmation;
  float temperature_confirmation;

  enum {
    INIT,
    READ,
    READ_CONFIRMATION,
    END
  } _get_state;

  /*!
  \var values[]
  \brief Internal sensor's variable for values readed from sensors.
  */
  rmapdata_t values[];
};
#endif

#if (USE_SENSOR_SHT)
#include <sht.h>
class SensorDriverSht : public SensorDriver {

public:
  SensorDriverSht(const char* driver, const char* type, TwoWire *wire = &Wire) : SensorDriver(driver, type, wire) {
    SensorDriver::printCreate();
  };
  void setup();
  void prepare(bool is_test = false);
  void get(rmapdata_t *values, uint8_t length, bool is_test=false);

  #if (USE_JSON)
  void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH, bool is_test=false);
  #endif

  void resetPrepared(bool is_test = false);

private:
  Sht sht;

protected:
  float humidity;
  float temperature;

  enum {
    INIT,
    READ,
    END
  } _get_state;

  /*!
  \var values[]
  \brief Internal sensor's variable for values readed from sensors.
  */
  rmapdata_t values[];
};
#endif

#if (USE_SENSOR_DW1)
#include <math.h>
#include "registers-windsonic.h"
class SensorDriverDw1 : public SensorDriver {
public:
   SensorDriverDw1(const char* driver, const char* type) : SensorDriver(driver, type) {
      SensorDriver::printInfo();
   };
   void setup();
   void prepare(bool is_test = false);
   void get(rmapdata_t *values, uint8_t length, bool is_test=false);
   void getSDfromUV(int32_t u, int32_t v, double *speed, double *direction);

   #if (USE_JSON)
   void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH, bool is_test=false);
   #endif

   void resetPrepared(bool is_test = false);

protected:

   double speed;
   double direction;

   enum {
      INIT,
      SET_MEANU_ADDRESS,
      READ_MEANU,
      SET_MEANV_ADDRESS,
      READ_MEANV,
      ELABORATE,
      END
   } _get_state;

   /*!
   \var values[]
   \brief Internal sensor's variable for values readed from sensors.
   */
   rmapdata_t values[];

};
#endif

#if (USE_SENSOR_TBS || USE_SENSOR_TBR)
#include "registers-rain.h"
class SensorDriverRain : public SensorDriver {
public:
  SensorDriverRain(const char* driver, const char* type) : SensorDriver(driver, type) {
      SensorDriver::printInfo();
   };
   void setup();
   void prepare(bool is_test = false);
   void get(rmapdata_t *values, uint8_t length, bool is_test=false);

   #if (USE_JSON)
   void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH, bool is_test=false);
   #endif

   void resetPrepared(bool is_test = false);

protected:

   uint8_t rain_data[I2C_RAIN_LENGTH];

   enum {
      INIT,
      SET_RAIN_ADDRESS,
      READ_RAIN,
      END
   } _get_state;

   /*!
   \var values[]
   \brief Internal sensor's variable for values readed from sensors.
   */
   rmapdata_t values[];

};
#endif

#if (USE_SENSOR_STH || USE_SENSOR_ITH || USE_SENSOR_MTH || USE_SENSOR_NTH || USE_SENSOR_XTH)
#include "registers-th.h"
class SensorDriverTh : public SensorDriver {
public:
  SensorDriverTh(const char* driver, const char* type, TwoWire *local_wire) : SensorDriver(driver, type) {
      _sdr_wire = local_wire;
      SensorDriver::printInfo();
   };
   void setup();
   void prepare(bool is_test = false);
   void get(rmapdata_t *values, uint8_t length, bool is_test=false);

   #if (USE_JSON)
   void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH, bool is_test=false);
   #endif

   void resetPrepared(bool is_test = false);

protected:

   uint8_t temperature_data[I2C_TH_TEMPERATURE_DATA_MAX_LENGTH];
   uint8_t humidity_data[I2C_TH_HUMIDITY_DATA_MAX_LENGTH];

   enum {
      INIT,
      SET_TEMPERATURE_ADDRESS,
      READ_TEMPERATURE,
      SET_HUMIDITY_ADDRESS,
      READ_HUMIDITY,
      END
   } _get_state;

   /*!
   \var values[]
   \brief Internal sensor's variable for values readed from sensors.
   */
   rmapdata_t values[];

};
#endif

#if (USE_SENSOR_STH_V2 || USE_SENSOR_ITH_V2 || USE_SENSOR_MTH_V2 || USE_SENSOR_NTH_V2 || USE_SENSOR_XTH_V2)
#include "registers-th.h"
#include "registers-th_v2.h"
class SensorDriverTh : public SensorDriver {
public:
  SensorDriverTh(const char* driver, const char* type, TwoWire *local_wire) : SensorDriver(driver, type) {
      _sdr_wire = local_wire;
      SensorDriver::printInfo();
   };
   void setup();
   void prepare(bool is_test = false);
   void get(rmapdata_t *values, uint8_t length, bool is_test=false);

   #if (USE_JSON)
   void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH, bool is_test=false);
   #endif

   void resetPrepared(bool is_test = false);

protected:

   uint8_t temperature_data[I2C_TH_TEMPERATURE_DATA_MAX_LENGTH];
   uint8_t humidity_data[I2C_TH_TEMPERATURE_DATA_MAX_LENGTH];

   enum {
      INIT,
      SET_TEMPERATURE_ADDRESS,
      READ_TEMPERATURE,
      SET_HUMIDITY_ADDRESS,
      READ_HUMIDITY,
      END
   } _get_state;

   /*!
   \var values[]
   \brief Internal sensor's variable for values readed from sensors.
   */
   rmapdata_t values[];

};
#endif

#if (USE_SENSOR_DEP)
#include "digiteco_power.h"
class SensorDriverDigitecoPower : public SensorDriver {
public:
   SensorDriverDigitecoPower(const char* driver, const char* type) : SensorDriver(driver, type) {
      SensorDriver::printInfo();
   };
   void setup();
   void prepare(bool is_test = false);
   void get(rmapdata_t *values, uint8_t length, bool is_test=false);

   #if (USE_JSON)
   void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH, bool is_test=false);
   #endif

   void resetPrepared(bool is_test = false);

protected:

   float battery_charge;
   float battery_voltage;
   float battery_current;
   float input_voltage;
   float input_current;
   float output_voltage;

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

   /*!
   \var values[]
   \brief Internal sensor's variable for values readed from sensors.
   */
   rmapdata_t values[];

};
#endif

#if (USE_SENSOR_DWA || USE_SENSOR_DWB || USE_SENSOR_DWC || USE_SENSOR_DWD || USE_SENSOR_DWE || USE_SENSOR_DWF)
#include "registers-wind.h"
class SensorDriverWind : public SensorDriver {
public:
  SensorDriverWind(const char* driver, const char* type) : SensorDriver(driver, type) {
    SensorDriver::printInfo();
  };
  void setup();
  void prepare(bool is_test = false);
  void get(rmapdata_t *values, uint8_t length, bool is_test=false);

  #if (USE_JSON)
  void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH, bool is_test=false);
  #endif

  void resetPrepared(bool is_test = false);

protected:

   uint8_t variable_length;
   uint8_t data_length;

   uint8_t variable_count;
   uint8_t offset;

  enum {
    INIT,
    SET_ADDRESS,
    READ_VALUE,
    GET_VALUE,
    END
  } _get_state;
};
#endif

#if (USE_SENSOR_DSA)
#include "registers-radiation.h"
class SensorDriverSolarRadiation : public SensorDriver {
public:
  SensorDriverSolarRadiation(const char* driver, const char* type) : SensorDriver(driver, type) {
    SensorDriver::printInfo();
  };
  void setup();
  void prepare(bool is_test = false);
  void get(rmapdata_t *values, uint8_t length, bool is_test=false);

  #if (USE_JSON)
  void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH, bool is_test=false);
  #endif

  void resetPrepared(bool is_test = false);

protected:

  uint8_t variable_length;
  uint8_t data_length;

  uint8_t variable_count;
  uint8_t offset;

  enum {
    INIT,
    SET_ADDRESS,
    READ_VALUE,
    GET_VALUE,
    END
  } _get_state;
};
#endif

#if (USE_SENSOR_OA2 || USE_SENSOR_OB2 || USE_SENSOR_OC2 || USE_SENSOR_OD2 || USE_SENSOR_OA3 || USE_SENSOR_OB3 || USE_SENSOR_OC3 || USE_SENSOR_OD3 || USE_SENSOR_OE3)
#include "registers-opc.h"
class SensorDriverOpc : public SensorDriver {
public:
   SensorDriverOpc(const char* driver, const char* type) : SensorDriver(driver, type) {
      SensorDriver::printInfo();
   };
   void setup();
   void prepare(bool is_test = false);
   void get(rmapdata_t *values, uint8_t length, bool is_test=false);

   #if (USE_JSON)
   void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH, bool is_test=false);
   #endif

   void resetPrepared(bool is_test = false);

protected:

   uint8_t variable_length;
   uint8_t data_length;
   uint8_t data[VALUES_TO_READ_FROM_SENSOR_COUNT];

   uint8_t variable_count;
   uint8_t offset;

   enum {
      INIT,
      SET_ADDRESS,
      READ_VALUE,
      GET_VALUE,
      END
   } _get_state;

   /*!
   \var values[]
   \brief Internal sensor's variable for values readed from sensors.
   */
   rmapdata_t values[];

};
#endif

#endif

#if (USE_SENSOR_LWT)
#include "registers-leaf.h"
class SensorDriverLeaf : public SensorDriver {
public:
  SensorDriverLeaf(const char* driver, const char* type) : SensorDriver(driver, type) {
    SensorDriver::printInfo();
  };
  void setup();
  void prepare(bool is_test = false);
  void get(rmapdata_t *values, uint8_t length, bool is_test=false);

  #if (USE_JSON)
  void getJson(rmapdata_t *values, uint8_t length, char *json_buffer, size_t json_buffer_length = JSON_BUFFER_LENGTH, bool is_test=false);
  #endif

  void resetPrepared(bool is_test = false);

protected:

  uint8_t variable_length;
  uint8_t data_length;

  uint8_t variable_count;
  uint8_t offset;

  enum {
    INIT,
    SET_ADDRESS,
    READ_VALUE,
    GET_VALUE,
    END
  } _get_state;
};
#endif
