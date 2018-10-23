/**@file eeprom_utility.h */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>
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

#ifndef _EEPROM_UTILITY_H
#define _EEPROM_UTILITY_H

#include <avr/eeprom.h>

/*!
\def ee_read(data,address,size)
\brief Read size bytes of data in eeprom at specified address.
*/
#define ee_read(data,address,size)      (eeprom_read_block((void *)data, (const void *)address, size))

/*!
\def ee_write(data,address,size)
\brief Write size bytes of data in eeprom at specified address.
*/
#define ee_write(data,address,size)     (eeprom_write_block((void *)data, (void *)address, size))

#endif
