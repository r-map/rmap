/**@file ADS1115.h */

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

#ifndef _ADS1115_H
#define _ADS1115_H

#include <Arduino.h>
#include <Wire.h>
#include <debug.h>

// 10 millis for ADS1115_REG_CONFIG_DR_128SPS  (for other DR adjust as 1000/DR)
#define ADS1115_CONVERSION_DELAY_MS     (10)
#define ADS1115_CHANNEL_COUNT           (4)

#define ADS1115_CHN_OFF_ID              (0)
#define ADS1115_VOLTAGE_ID              (1)
#define ADS1115_CURRENT_ID              (2)

#define isAdsChnEnabled(x)              (x > ADS1115_CHN_OFF_ID)
#define isAdsChnDisabled(x)             (x == ADS1115_CHN_OFF_ID)
#define isAdsChnVoltage(x)              (x == ADS1115_VOLTAGE_ID)
#define isAdsChnCurrent(x)              (x == ADS1115_CURRENT_ID)

/*=========================================================================
POINTER REGISTER
-----------------------------------------------------------------------*/
#define ADS1115_REG_POINTER_MASK        (0x03)
#define ADS1115_REG_POINTER_CONVERT     (0x00)
#define ADS1115_REG_POINTER_CONFIG      (0x01)
#define ADS1115_REG_POINTER_LOWTHRESH   (0x02)
#define ADS1115_REG_POINTER_HITHRESH    (0x03)
/*=========================================================================*/

/*=========================================================================
CONFIG REGISTER
-----------------------------------------------------------------------*/
#define ADS1115_REG_CONFIG_OS_MASK      (0x8000)
#define ADS1115_REG_CONFIG_OS_SINGLE    (0x8000)  // Write: Set to start a single-conversion
#define ADS1115_REG_CONFIG_OS_BUSY      (0x0000)  // Read: Bit = 0 when conversion is in progress
#define ADS1115_REG_CONFIG_OS_NOTBUSY   (0x8000)  // Read: Bit = 1 when device is not performing a conversion

#define ADS1115_REG_CONFIG_MUX_MASK     (0x7000)
#define ADS1115_REG_CONFIG_MUX_DIFF_0_1 (0x0000)  // Differential P = AIN0, N = AIN1 (default)
#define ADS1115_REG_CONFIG_MUX_DIFF_0_3 (0x1000)  // Differential P = AIN0, N = AIN3
#define ADS1115_REG_CONFIG_MUX_DIFF_1_3 (0x2000)  // Differential P = AIN1, N = AIN3
#define ADS1115_REG_CONFIG_MUX_DIFF_2_3 (0x3000)  // Differential P = AIN2, N = AIN3
#define ADS1115_REG_CONFIG_MUX_SINGLE_0 (0x4000)  // Single-ended AIN0
#define ADS1115_REG_CONFIG_MUX_SINGLE_1 (0x5000)  // Single-ended AIN1
#define ADS1115_REG_CONFIG_MUX_SINGLE_2 (0x6000)  // Single-ended AIN2
#define ADS1115_REG_CONFIG_MUX_SINGLE_3 (0x7000)  // Single-ended AIN3

#define ADS1115_REG_CONFIG_PGA_MASK     (0x0E00)
#define ADS1115_REG_CONFIG_PGA_6_144V   (0x0000)  // +/-6.144V range = Gain 2/3
#define ADS1115_REG_CONFIG_PGA_4_096V   (0x0200)  // +/-4.096V range = Gain 1
#define ADS1115_REG_CONFIG_PGA_2_048V   (0x0400)  // +/-2.048V range = Gain 2 (default)
#define ADS1115_REG_CONFIG_PGA_1_024V   (0x0600)  // +/-1.024V range = Gain 4
#define ADS1115_REG_CONFIG_PGA_0_512V   (0x0800)  // +/-0.512V range = Gain 8
#define ADS1115_REG_CONFIG_PGA_0_256V   (0x0A00)  // +/-0.256V range = Gain 16

#define ADS1115_REG_CONFIG_MODE_MASK    (0x0100)
#define ADS1115_REG_CONFIG_MODE_CONTIN  (0x0000)  // Continuous conversion mode
#define ADS1115_REG_CONFIG_MODE_SINGLE  (0x0100)  // Power-down single-shot mode (default)

#define ADS1115_REG_CONFIG_DR_MASK      (0x00E0)
#define ADS1115_REG_CONFIG_DR_8SPS      (0x0000)  // 8 samples per second
#define ADS1115_REG_CONFIG_DR_16SPS     (0x0020)  // 16 samples per second
#define ADS1115_REG_CONFIG_DR_32SPS     (0x0040)  // 32 samples per second
#define ADS1115_REG_CONFIG_DR_64SPS     (0x0060)  // 64 samples per second
#define ADS1115_REG_CONFIG_DR_128SPS    (0x0080)  // 128 samples per second (default)
#define ADS1115_REG_CONFIG_DR_250SPS    (0x00A0)  // 250 samples per second
#define ADS1115_REG_CONFIG_DR_475SPS    (0x00C0)  // 475 samples per second
#define ADS1115_REG_CONFIG_DR_860SPS    (0x00E0)  // 860 samples per second

#define ADS1115_REG_CONFIG_CMODE_MASK   (0x0010)
#define ADS1115_REG_CONFIG_CMODE_TRAD   (0x0000)  // Traditional comparator with hysteresis (default)
#define ADS1115_REG_CONFIG_CMODE_WINDOW (0x0010)  // Window comparator

#define ADS1115_REG_CONFIG_CPOL_MASK    (0x0008)
#define ADS1115_REG_CONFIG_CPOL_ACTVLOW (0x0000)  // ALERT/RDY pin is low when active (default)
#define ADS1115_REG_CONFIG_CPOL_ACTVHI  (0x0008)  // ALERT/RDY pin is high when active

#define ADS1115_REG_CONFIG_CLAT_MASK    (0x0004)  // Determines if ALERT/RDY pin latches once asserted
#define ADS1115_REG_CONFIG_CLAT_NONLAT  (0x0000)  // Non-latching comparator (default)
#define ADS1115_REG_CONFIG_CLAT_LATCH   (0x0004)  // Latching comparator

#define ADS1115_REG_CONFIG_CQUE_MASK    (0x0003)
#define ADS1115_REG_CONFIG_CQUE_1CONV   (0x0000)  // Assert ALERT/RDY after one conversions
#define ADS1115_REG_CONFIG_CQUE_2CONV   (0x0001)  // Assert ALERT/RDY after two conversions
#define ADS1115_REG_CONFIG_CQUE_4CONV   (0x0002)  // Assert ALERT/RDY after four conversions
#define ADS1115_REG_CONFIG_CQUE_NONE    (0x0003)  // Disable the comparator and put ALERT/RDY in high state (default)
/*=========================================================================*/

#define ADC_SET_CHANNEL_RETRY_COUNT_MAX (20)
#define ADC_SET_CHANNEL_RETRY_DELAY_MS  (2)

#define ADC_READ_RETRY_COUNT_MAX        (20)
#define ADC_READ_RETRY_DELAY_MS         (2)
#define ADC_READ_DELAY_MS               (2)

#define ADC_CHECK_RETRY_COUNT_MAX       (20)
#define ADC_CHECK_RETRY_DELAY_MS        (2)

#define ADC_CHECK_BUFFER_LENGTH         (3)
#define ADC_CHECK_COUNT                 (3)

typedef enum {
  GAIN_TWOTHIRDS    = ADS1115_REG_CONFIG_PGA_6_144V,
  GAIN_ONE          = ADS1115_REG_CONFIG_PGA_4_096V,
  GAIN_TWO          = ADS1115_REG_CONFIG_PGA_2_048V,
  GAIN_FOUR         = ADS1115_REG_CONFIG_PGA_1_024V,
  GAIN_EIGHT        = ADS1115_REG_CONFIG_PGA_0_512V,
  GAIN_SIXTEEN      = ADS1115_REG_CONFIG_PGA_0_256V
} adsGain_t;

typedef enum {
  ADC_INIT,
  ADC_SET_CHANNEL,
  ADC_READ,
  ADC_CHECK,
  ADC_END,
  ADC_WAIT_STATE
} adc_state_t;

typedef enum {
  ADC_BUSY,
  ADC_OK,
  ADC_ERROR
} adc_result_t;

class ADS1115 {
protected:
  uint8_t   m_i2c_address;
  adsGain_t m_gain;
  adc_state_t adc_state;

  uint8_t retry;
  bool is_error;
  adc_state_t state_after_wait;
  uint32_t delay_ms;
  uint32_t start_time_ms;
  int16_t adc_value[ADC_CHECK_COUNT];
  uint8_t adc_value_count;
  adc_result_t adc_result;
  
  adc_result_t readRegister(uint8_t i2c_address, uint8_t reg, uint16_t *value);
  adc_result_t writeRegister(uint8_t i2c_address, uint8_t reg, uint16_t value);

public:
  ADS1115(uint8_t i2c_address);
  adc_result_t initSingleEnded(uint8_t channel);
  adc_result_t readSingleEnded(int16_t *value);
  void setGain(adsGain_t gain);
  adsGain_t getGain(void);
  adc_result_t readSingleChannel(uint8_t channel, int16_t *value);

private:
};

#endif
