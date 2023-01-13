/**
  ******************************************************************************
  * @file    register_class.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Register class (Uavcan/Other) header file
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

#include <stdbool.h>
#include <stdint.h>
#include <uavcan/_register/Access_1_0.h>
#include <uavcan/_register/List_1_0.h>
#include "drivers/eeprom.h"

#ifndef _REGISTER_CLASS_H
#define _REGISTER_CLASS_H

// ***************************************************************************************
//               E2PROM STIMAV4 STM32 REGISTER ACCESS LOCATION AND SIZE CONFIG
// ***************************************************************************************

// Start Address BASE UAVCAN/CYPAL Register (128 Bytes x Register)
#define MEM_UAVCAN_LEN                      EEPROMSIZE
#define MEM_UAVCAN_ADDR_START               REGISTER_EEPROM_ADDRESS
#define MEM_UAVCAN_MAX_REG                  60u
#define MEM_UAVCAN_LEN_SIZE_T_REG           1u
#define MEM_UAVCAN_LEN_INTEST_REG           60u
#define MEM_UAVCAN_LEN_VALUE_REG            66u
#define MEM_UAVCAN_POS_LEN_NAME             0u
#define MEM_UAVCAN_POS_STR_NAME             MEM_UAVCAN_POS_LEN_NAME + MEM_UAVCAN_LEN_SIZE_T_REG
#define MEM_UAVCAN_POS_LEN_DATA             MEM_UAVCAN_POS_STR_NAME + MEM_UAVCAN_LEN_INTEST_REG
#define MEM_UAVCAN_POS_VALUE_DATA           MEM_UAVCAN_POS_LEN_DATA + MEM_UAVCAN_LEN_SIZE_T_REG
#define MEM_UAVCAN_LEN_NAME_REG             (MEM_UAVCAN_LEN_SIZE_T_REG + MEM_UAVCAN_LEN_INTEST_REG)
#define MEM_UAVCAN_LEN_DATA_REG             (MEM_UAVCAN_LEN_SIZE_T_REG + MEM_UAVCAN_LEN_VALUE_REG)
#define MEM_UAVCAN_LEN_REG                  (MEM_UAVCAN_LEN_NAME_REG + MEM_UAVCAN_LEN_DATA_REG)
#define MEM_UAVCAN_START_AREA_REG           (MEM_UAVCAN_ADDR_START + MEM_UAVCAN_MAX_REG)
#define MEM_UAVCAN_REG_UNDEF                0xFF
#define MEM_UAVCAN_GET_ADDR_FLAG()          (MEM_UAVCAN_ADDR_START)
#define MEM_UAVCAN_GET_ADDR_FLAG_REG(X)     (MEM_UAVCAN_ADDR_START + X)
#define MEM_UAVCAN_GET_ADDR_NAME_LEN(X)     (MEM_UAVCAN_START_AREA_REG + (MEM_UAVCAN_LEN_REG * X))
#define MEM_UAVCAN_GET_ADDR_NAME(X)         (MEM_UAVCAN_START_AREA_REG + (MEM_UAVCAN_LEN_REG * X) + MEM_UAVCAN_LEN_SIZE_T_REG)
#define MEM_UAVCAN_GET_ADDR_VALUE_LEN(X)    (MEM_UAVCAN_START_AREA_REG + (MEM_UAVCAN_LEN_REG * X) + MEM_UAVCAN_LEN_NAME_REG)
#define MEM_UAVCAN_GET_ADDR_VALUE(X)        (MEM_UAVCAN_START_AREA_REG + (MEM_UAVCAN_LEN_REG * X) + MEM_UAVCAN_LEN_NAME_REG + MEM_UAVCAN_LEN_SIZE_T_REG)
#define MEM_UAVCAN_GET_ADDR_BASE_REG(X)     MEM_UAVCAN_GET_ADDR_NAME_LEN(X)
// Start Address Eeprom Application Free usage
#define MEM_UAVCAN_ADDR_END                 (MEM_UAVCAN_START_AREA_REG + (MEM_UAVCAN_LEN_REG * MEM_UAVCAN_MAX_REG))
#define MEM_UAVCAN_USED_TOTAL               (MEM_UAVCAN_ADDR_END - MEM_UAVCAN_ADDR_START)
// Error Size Memory limit
#if (MEM_UAVCAN_ADDR_END > MEM_UAVCAN_LEN)
    #error Uavcan Register limit is more than MAX SIZE (MEM_UAVCAN_LEN). Please check Register configuration
#endif

// Class EEProm - Register Uavcan
class EERegister {

public:
  // Constructor
  EERegister();
  EERegister(TwoWire *wire, BinarySemaphore *wireLock, uint8_t i2c_address = EEPROM_AT24C64_DEFAULT_ADDRESS);

  // Inizializza lo spazio RAM/ROM/FLASH/SD dei registri, ai valori di default
  // N.B.! Azzera tutti registri e quelli non inizializzati devono essere impostati
  // nel relativo modulo di utilizzo
  void setup(void);

  /// Reads the specified register from the persistent storage into `inout_value`.
  /// If the register does not exist or it cannot be automatically converted to the type of the provided argument,
  /// the value will be stored in the persistent storage using @ref registerWrite(), overriding existing value.
  /// The default will not be initialized if the argument is empty.
  void read(const char* const register_name, uavcan_register_Value_1_0* const inout_value);

  /// Store the given register value into the persistent storage.
  void write(const char* const register_name, const uavcan_register_Value_1_0* const value);

  /// This function is mostly intended for implementing the standard RPC-service uavcan.register.List.
  /// It returns the name of the register at the specified index (where the ordering is undefined but guaranteed
  /// to be short-term stable), or empty name if the index is out of bounds.
  uavcan_register_Name_1_0 getNameByIndex(const uint16_t index);

  /// Erase all registers such that the defaults are used at the next launch.
  void doFactoryReset(void);

  /// Copy one value to the other if their types and dimensionality are the same or automatic conversion is possible.
  /// If the destination is empty, it is simply replaced with the source (assignment always succeeds).
  /// The return value is true if the assignment has been performed, false if it is not possible
  /// (in the latter case the destination is NOT modified).
  bool assign(uavcan_register_Value_1_0* const dst, const uavcan_register_Value_1_0* const src);

protected:
private:

  void _memory_write_block(uint16_t address, uint8_t *data, uint8_t len);
  void _memory_read_block(uint16_t address, uint8_t *data, uint8_t len);
  void _memory_write_byte(uint16_t address, uint8_t data);
  void _memory_read_byte(uint16_t address, uint8_t *data);
  void _eeprom_register_factory(void);
  void _eeprom_register_clear(uint8_t reg_numb);
  size_t _eeprom_register_get_fast(uint8_t reg_numb, uint8_t *reg_name, uint8_t *reg_value);
  size_t _eeprom_register_get_len_intest_fast(uint8_t reg_numb);
  void _eeprom_register_get_intest_fast(uint8_t reg_numb, uint8_t *reg_name, uint8_t name_len);
  bool _eeprom_register_get_name_from_index(uint8_t reg_numb, uint8_t *reg_name);
  size_t _eeprom_register_get_from_name(uint8_t const *reg_name, uint8_t *reg_numb, uint8_t *data);
  uint8_t _eeprom_register_get_index_from_name(uint8_t *reg_name);
  void _eeprom_register_set(uint8_t reg_numb, uint8_t *reg_name, uint8_t *data, size_t len_data);
  void _eeprom_register_get_next_id(uint8_t *current_register);
  uint8_t _eeprom_register_add(uint8_t *reg_name, uint8_t *data, size_t data_len);

  // Private Memory E2prom Object
  EEprom _Eprom;

};

#endif