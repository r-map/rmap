/**
  ******************************************************************************
  * @file    eeprom.h
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   eeprom AT24C64 header file
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


#ifndef _EEPROM_H
#define _EEPROM_H

#include <STM32FreeRTOS.h>
#include "ticks.hpp"
#include "thread.hpp"
#include "semaphore.hpp"
#include "Wire.h"

using namespace cpp_freertos;

#define EEPROM_AT24C64_DEFAULT_ADDRESS (0x50)
#define	EEPROMSIZE  8192
#define EEPAGESIZE  32
#define PAGEMASK    (EEPROMSIZE-EEPAGESIZE)
#define WR_TIME_MS  5
#define EEPROM_SEMAPHORE_MAX_WAITING_TIME_MS  (10000)

class EEprom {

public:
  EEprom();
  EEprom(TwoWire *wire, BinarySemaphore *wireLock, uint8_t i2c_address = EEPROM_AT24C64_DEFAULT_ADDRESS);
  bool Write(uint16_t address, uint8_t *buffer, uint16_t length);
  bool Write(uint16_t address, uint8_t value);
  bool Read(uint16_t address, uint8_t *buffer, uint16_t length);
  bool Read(uint16_t address, uint8_t *value);

protected:
private:
  TwoWire *_wire;
  BinarySemaphore *_wireLock;
  uint8_t _i2c_address;
};

#endif
