/**@file ntp_config.h */

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

#ifndef _NTP_CONFIG_H
#define _NTP_CONFIG_H

/*!
\def NTP_SERVER_LENGTH
\brief Length in bytes for ntp server data buffer.
*/
#define NTP_SERVER_LENGTH     (30)

/*!
\def NTP_DEFAULT_SERVER
\brief Default NTP server.
*/
#define NTP_DEFAULT_SERVER    ("pool.ntp.org")

#endif
