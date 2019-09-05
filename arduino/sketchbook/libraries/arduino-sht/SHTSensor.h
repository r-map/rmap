/*
 *  Copyright (c) 2018, Sensirion AG <andreas.brauchli@sensirion.com>
 *  Copyright (c) 2015-2016, Johannes Winkelmann <jw@smts.ch>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of the Sensirion AG nor the names of its
 *        contributors may be used to endorse or promote products derived
 *        from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SHTSENSOR_H
#define SHTSENSOR_H

#include <inttypes.h>


/** class for i2c SHT Sensor drivers */
class SHTI2cSensor  {
public:
  /** Size of i2c commands to send */
  static const uint8_t CMD_SIZE;
  
  /** Size of i2c replies to expect */
  static const uint8_t EXPECTED_DATA_SIZE;
  
  uint8_t mI2cAddress;
  uint16_t mI2cPCommand;
  uint16_t mI2cSSCommand;
  uint8_t mDuration;
  float mTemperature;
  float mHumidity;


  /**
   * mps setting of measurement for Periodic Data Acquisition Mode
   */
  enum SHTmps {
		    SHT_MPS_05,
		    SHT_MPS_1,
		    SHT_MPS_2,
		    SHT_MPS_4,
		    SHT_MPS_10
  };

  
  /**
   * Repeatability setting of measurement.
   */
  enum SHTRepeatibility {
		    /** Highest repeatability at the cost of slower measurement */
		    SHT_REPEATABILITY_HIGH,
		    /** Balanced repeatability and speed of measurement */
		    SHT_REPEATABILITY_MEDIUM,
		    /** Fastest measurement but lowest repeatability */
		    SHT_REPEATABILITY_LOW
  };
  
  /** Highest repeatability at the cost of slower measurement */
  const uint16_t SHT_SINGLESHOT_REPEATABILITY_HIGH_COMMAND=0x2400;
  /** Balanced repeatability and speed of measurement */
  const uint16_t SHT_SINGLESHOT_REPEATABILITY_MEDIUM_COMMAND=0x240B;
  /** Fastest measurement but lowest repeatability */
  const uint16_t SHT_SINGLESHOT_REPEATABILITY_LOW_COMMAND=0x2416;

  
  //  duration is the duration in milliseconds of one measurement
  /** Highest repeatability at the cost of slower measurement */
  const uint8_t SHT_REPEATABILITY_HIGH_DURATION=16;
  /** Balanced repeatability and speed of measurement */
  const uint8_t SHT_REPEATABILITY_MEDIUM_DURATION=7;
  /** Fastest measurement but lowest repeatability */
  const uint8_t SHT_REPEATABILITY_LOW_DURATION=5;
  
  
  /**
   * Constructor for i2c SHT Sensors
   * Takes the `i2cAddress' to read, the `i2cCommand' issues when sampling
   * the sensor and the values `a', `b', `c' to convert the fixed-point
   * temperature value received by the sensor to a floating point value using
   * the formula: temperature = a + b * (rawTemperature / c)
   * and the values `x' and `y' to convert the fixed-point humidity value
   * received by the sensor to a floating point value using the formula:
   * humidity = x * (rawHumidity / y)
   */

  SHTI2cSensor(uint8_t i2cAddress=0x44, SHTRepeatibility repeatibility=SHT_REPEATABILITY_HIGH, SHTmps mps=SHT_MPS_1);
  
  /**
   * Change the sensor accurancy, if supported by the sensor
   * Returns true if the repeatability was changed
   */
  bool setRepeatability(SHTRepeatibility newRepeatability, SHTmps newmps);
  bool sendCommand(const uint8_t *i2cCommand,
		   uint8_t commandLength);
  bool checkStatus();
  bool singleShotDataAcquisition();
  bool periodicDataAcquisition();
  bool ART();
  bool stopPeriodicDataAcquisition();
  bool fetchData();
  bool getValues();
  bool readSample();
  bool softReset();
  bool heaterEnable();
  bool heaterDisable();  
  bool clearStatusRegister();
  
  /**
   * Get the relative humidity in percent read from the last sample
   * Use getValues() to trigger a new sensor reading
   */
  float getHumidity();
  
  /**
   * Get the humidity in percent read from the last sample
   * Use getValues() to trigger a new sensor reading
   */
  float getTemperature();
  
private:
  static uint8_t crc8(const uint8_t *data, uint8_t len);
};
  
#endif /* SHTSENSOR_H */
