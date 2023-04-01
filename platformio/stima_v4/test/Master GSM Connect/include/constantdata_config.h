/**@file constantdata_config.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>

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

#define DATA_LEVEL_LENGTH  (20)
#define NETWORK_LENGTH     (20)
#define IDENT_LENGTH       (20)

/*!
\def STATIONSLUG_LENGTH
\brief Length in bytes for station slug.
*/
#define STATIONSLUG_LENGTH (30)

/*!
\def BOARDSLUG_LENGTH
\brief Length in bytes for board slug.
*/
#define BOARDSLUG_LENGTH (30)

/*!
\def DEFAULT_STATIONSLUG
\brief Default station slug.
*/
#define DEFAULT_STATIONSLUG ("")

/*!
\def DEFAULT_BOARDSLUG
\brief Default board slug.
*/
#define DEFAULT_BOARDSLUG ("")

#endif
