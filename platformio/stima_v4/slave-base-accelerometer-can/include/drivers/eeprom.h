/**@file eeprom.h */

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

#ifndef _EEPROM_H
#define _EEPROM_H

#include <STM32FreeRTOS.h>
#include "ticks.hpp"
#include "thread.hpp"
#include "semaphore.hpp"
#include "Wire.h"

using namespace cpp_freertos;

#define EEPROM_AT24C64_DEFAULT_ADDRESS (0x50)

class EEprom {

public:
  EEprom();
  EEprom(TwoWire *_wire, BinarySemaphore *_wireLock, uint8_t _i2c_address = EEPROM_AT24C64_DEFAULT_ADDRESS);
  bool Write(uint16_t address, uint8_t *buffer, uint16_t length);
  bool Read(uint16_t address, uint8_t *buffer, uint16_t length);

protected:
private:
  TwoWire *wire;
  BinarySemaphore *wireLock;
  uint8_t i2c_address;
};

#endif
