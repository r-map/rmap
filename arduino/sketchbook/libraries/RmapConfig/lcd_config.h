/**@file lcd_config.h */

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

#ifndef _LCD_CONFIG_H
#define _LCD_CONFIG_H

/*!
\def LCD_I2C_ADDRESS
\brief Default LCD i2c address.
*/
#define LCD_I2C_ADDRESS          (0x3F)

/*!
\def LCD_COLUMNS
\brief Default LCD columns number.
*/
#define LCD_COLUMNS              (20)

/*!
\def LCD_ROWS
\brief Default LCD rows number.
*/
#define LCD_ROWS                 (4)

#endif
