/**@file ntp_config.h */

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
#define NTP_DEFAULT_SERVER    ("it.pool.ntp.org")

#endif
