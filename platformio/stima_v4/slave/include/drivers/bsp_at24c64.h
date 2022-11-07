/**@file bsp_at24c64.h */

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

#ifndef _BSP_AT24C64_H
#define _BSP_AT24C64_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAL_I2C_MODULE_ENABLED
#define EEPROM_SLVADDR		(0xA0)
extern I2C_HandleTypeDef hi2c2;
#define I2CDEV hi2c2
#define	EEPROMSIZE 8192
#define EEPAGESIZE 32
#define PAGEMASK (EEPROMSIZE-EEPAGESIZE)
#else
	#error "I2C device not defined!"
#endif

#ifdef __cplusplus
}
#endif

#endif