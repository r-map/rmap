/**@file hardware_config.h */

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

#ifndef _HARDWARE_CONFIG_H
#define _HARDWARE_CONFIG_H

/*!
\def I2C_BUS_CLOCK
\brief I2C bus clock in Hertz.
*/
#define I2C_BUS_CLOCK                   (50000L)

/*!
\def I2C_MAX_DATA_LENGTH
\brief Max length in bytes for i2c bus data buffer.
*/
#define I2C_MAX_DATA_LENGTH             (32)

#endif
