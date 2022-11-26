/**@file constantdata_config.h */

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

#ifndef CONSTANTDATA_CONFIG_H
#define CONSTANTDATA_CONFIG_H

/*!
\def CONSTANTDATA_BTABLE_LENGTH
\brief Maximum lenght of btable code plus terminator that describe one constant data.
*/
#define CONSTANTDATA_BTABLE_LENGTH                    (7)

/*!
\def CONSTANTDATA_VALUE_LENGTH
\brief Maximum lenght of value plus terminator for one constant data.
*/
#define CONSTANTDATA_VALUE_LENGTH                    (33)

#endif
