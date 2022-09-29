/**@file i2c_config.h */

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

#ifndef _I2C_CONFIG_H
#define _I2C_CONFIG_H

/*!
\def I2C_BUS_CLOCK
\brief I2C bus clock in Hertz.
30418,25 Hz  : minimum freq with prescaler set to 1 and CPU clock to 16MHz
*/
#define I2C_BUS_CLOCK                   (100000L)

/*!
\def I2C_MAX_DATA_LENGTH
\brief Max length in bytes for i2c bus data buffer.
*/
#define I2C_MAX_DATA_LENGTH             (32)

/*!
\def I2C_MAX_ERROR_COUNT
\brief Max i2c error for bus restart.
*/
#define I2C_MAX_ERROR_COUNT             (3)

#endif
