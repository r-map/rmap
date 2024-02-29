/**
  ******************************************************************************
  * @file    Mppt.h
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Mppt cotroller Analog LTC4015 header file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
  * <h2><center>All rights reserved.</center></h2>
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
#define V_DQ1_OFFS    700
#define V_REF_IN      1.648f
#define V_REF_CVAL    1000.0f

// LTC4015 VBAT Conversion Factor
#define V_CELL_COUNT  1
#define V_REF_LH_BAT  192.264f
#define V_REF_PB_BAT  128.176f
#define V_REF_BVAL    1000.0f

// LTC4015 IBAT Conversion Factor
#define I_REF_CALC_5  0.29297f
#define I_REF_CALC_10 0.14658f
#define I_REF_IN      1.46487f
#define I_REF_RSNSB   LTC4015_RSNSB
#define I_REF_CVAL    1000.0f

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
  #define enable_AL_V_SYS_LO() LTC4015_write_register(LTC4015_EN_VSYS_LO_ALERT_BF, 1);    // Enable SMBAlert LO V_SYS
  #define disable_AL_V_SYS_LO() LTC4015_write_register(LTC4015_EN_VSYS_LO_ALERT_BF, 0);   // Disable SMBAlert LO V_SYS
  #define setflag_AL_V_SYS_LO(X) LTC4015_write_register(LTC4015_EN_VSYS_LO_ALERT_BF, X);  // Set request SMBAlert LO V_SYS

  // Float data reading value
  float get_V_IN(void);     // Get V_IN
  float get_V_SYS(void);    // Get V_SYS
  float get_V_BAT(void);    // Get V_BAT
  float get_I_BAT(void);    // Get I_BAT
  float get_I_IN(void);     // Get I_IN

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


