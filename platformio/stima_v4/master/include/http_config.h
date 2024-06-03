/**@file http_config.h */

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

#ifndef _HTTP_CONFIG_H
#define _HTTP_CONFIG_H

/// @brief HTTP client port
#define HTTP_CLIENT_PORT         (442)

/// @brief HTTP URI length
#define HTTP_URI_LENGTH          (10 + MQTT_USERNAME_LENGTH + STATIONSLUG_LENGTH + BOARDSLUG_LENGTH + 10)

/// @brief HTTP buffer size
#define HTTP_BUFFER_SIZE         (256)

/// @brief HTTP user agents length
// #define HTTP_USER_AGENTS_LENGTH  (STIMA_MODULE_NAME_LENGTH + 10)

/// @brief HTTP header size
#define HTTP_HEADER_SIZE         (128)

#endif
