/**@file local_typedef_config.h */

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

#ifndef _LOCAL_TYPEDEF_CONFIG_H
#define _LOCAL_TYPEDEF_CONFIG_H

#include "config.h"
#include "constantdata_config.h"
#include "mqtt_config.h"
#include "http_config.h"
#include "ntp_config.h"
#include "gsm_config.h"
#include "ethernet_config.h"

#define CLIENT_PSK_KEY_LENGTH (16)
#define CLIENT_PSK_IDENTITY_LENGTH (MQTT_USERNAME_LENGTH + STATIONSLUG_LENGTH + BOARDSLUG_LENGTH)

#endif
