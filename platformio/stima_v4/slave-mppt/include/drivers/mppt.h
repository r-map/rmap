/**
  ******************************************************************************
  * @file    Mppt.h
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Mppt cotroller Analog LTC4015 header file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
  * All rights reserved.</center></h2>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  * 
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  * <http://www.gnu.org/licenses/>.
  * 
  ******************************************************************************
*/
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _MPPT_H
#define _MPPT_H

/* Includes ------------------------------------------------------------------*/
#include <STM32FreeRTOS.h>
#include "ticks.hpp"
#include "thread.hpp"
#include "semaphore.hpp"
#include "LTC4015_reg_defs.h"
#include "LTC4015_formats.h"
#include "Wire.h"

using namespace cpp_freertos;

/** MPPT LTC4015
  * I2C Device Address 8 bit format */
#define MPPT_LTC4015_I2C_ADDR_DEFAULT     (LTC4015_ADDR_68)

// LTC4015 VIN Conversion Factor
#define V_DQ1_OFFS    500
#define V_REF_IN      1.6807f
#define V_REF_CVAL    1000.0f

// LTC4015 VBAT Conversion Factor
#define V_REF_LH_BAT  1.298f
#define V_REF_PB_BAT  0.865f
#define V_REF_OF_BAT  0.05f
#define V_REF_BVAL    1000.0f
#define V_REF_V_DCHG  0.75f
#define V_REF_A_RCHG  3.00f

// LTC4015 IBAT Conversion Factor
#define I_REF_IN      1.41895f
#define I_REF_BATT    1.47807f
#define I_REF_CVAL    10000.0f

// Class Mppt LT4015 implementation
class Mppt {

public:

  // *************** Class Constructor *************** //

  Mppt();
  Mppt(TwoWire *wire, BinarySemaphore *wireLock, uint8_t i2c_address = MPPT_LTC4015_I2C_ADDR_DEFAULT);

  // ************ Library implemenmtation ************ //

  bool LTC4015_read_register(uint16_t registerinfo, uint16_t *data);
  bool LTC4015_write_register(uint16_t registerinfo, uint16_t data);

  // *********** User facility data access *********** //

  // Register Access with macro examples
  #define enable_AL_V_SYS_LO()          LTC4015_write_register(LTC4015_EN_VSYS_LO_ALERT_BF, 1);   // Enable SMBAlert LO V_SYS
  #define disable_AL_V_SYS_LO()         LTC4015_write_register(LTC4015_EN_VSYS_LO_ALERT_BF, 0);   // Disable SMBAlert LO V_SYS
  #define setflag_AL_V_SYS_LO(X)        LTC4015_write_register(LTC4015_EN_VSYS_LO_ALERT_BF, X);   // Set request SMBAlert LO V_SYS
  #define enable_FRZ_MIS_SYS_VBAT()     LTC4015_write_register(LTC4015_FORCE_MEAS_SYS_ON_BF, 1);  // Enable Measure without VIN
  #define disable_FRZ_MIS_SYS_VBAT()    LTC4015_write_register(LTC4015_FORCE_MEAS_SYS_ON_BF, 0);  // Disable Measure without VIN
  #define setflag_FRZ_MIS_SYS_VBAT(X)   LTC4015_write_register(LTC4015_FORCE_MEAS_SYS_ON_BF, X);  // Set request Measure without VIN

  // Setting command
  bool set_Full_Measure(bool is_enable);

  // Float data reading value
  float get_V_IN(bool *is_ok);     // Get V_IN
  float get_V_SYS(bool *is_ok);    // Get V_SYS
  float get_V_BAT(bool *is_ok);    // Get V_BAT
  float get_I_BAT(bool *is_ok);    // Get I_BAT
  float get_I_IN(bool *is_ok);     // Get I_IN
  float get_P_CHG(bool *is_ok);    // Get P_CHG

protected:
private:

  // ********** Private Acces I2C SMBusProt ********** //

  bool smbus_read_reg(uint16_t reg, uint16_t *data);
  bool smbus_write_reg(uint16_t reg, uint16_t data);

  // ********** Private LTC4015 Reg_Format ********** //

  uint8_t get_size(uint16_t registerinfo);
  uint8_t get_subaddr(uint16_t registerinfo);
  uint8_t get_offset(uint16_t registerinfo);
  uint16_t get_mask(uint16_t registerinfo);

  // ********** Private Variables Semaphore ********** //

  TwoWire *_wire;
  BinarySemaphore *_wireLock;
  uint8_t _i2c_address;
};

#endif /* _MPPT_H */

/************************ (C) COPYRIGHT Digiteco s.r.l. *****END OF FILE****/
