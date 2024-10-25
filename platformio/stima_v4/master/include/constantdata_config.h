/**@file constantdata_config.h */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
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

/// @brief Maximum lenght of btable code plus terminator that describe one constant data.
#define CONSTANTDATA_BTABLE_LENGTH                    (7)

/// @brief Maximum lenght of value plus terminator for one constant data.
#define CONSTANTDATA_VALUE_LENGTH                    (33)

/// @brief Network length
#define NETWORK_LENGTH     (20)
/// @brief Ident length
#define IDENT_LENGTH       (20)

/// @brief Length in bytes for station slug.
#define STATIONSLUG_LENGTH (30)

/// @brief Length in bytes for board slug.
#define BOARDSLUG_LENGTH (30)

/// @brief Default station slug.
#define DEFAULT_STATIONSLUG ("")

/// @brief Default board slug.
#define DEFAULT_BOARDSLUG ("")

#endif
