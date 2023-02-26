/**@file http_config.h */

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

#ifndef _HTTP_CONFIG_H
#define _HTTP_CONFIG_H

#define HTTP_CLIENT_PORT         (873)

#define HTTP_URI_LENGTH          (10 + MQTT_USERNAME_LENGTH + STATIONSLUG_LENGTH + BOARDSLUG_LENGTH + 10)

#define HTTP_BUFFER_SIZE         (128)

// #define HTTP_USER_AGENTS_LENGTH  (STIMA_MODULE_NAME_LENGTH + 10)

#define HTTP_HEADER_SIZE         (128)

#endif
