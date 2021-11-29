/**@file sdcard_config.h */

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

#ifndef _SDCARD_CONFIG_H
#define _SDCARD_CONFIG_H

// define used by SdFat library
#define SPI_DRIVER_SELECT 1
#define USE_SD_CRC 2
#define SDFAT_FILE_TYPE 1
#define USE_LONG_FILE_NAMES 1
#define USE_SEPARATE_FAT_CACHE 0
#define USE_EXFAT_BITMAP_CACHE 0
#define USE_UTF8_LONG_NAMES 0
#define USE_FAT_FILE_FLAG_CONTIGUOUS 0
#define ENABLE_DEDICATED_SPI 1


/*!
\def SDCARD_FILES_NAME_MAX_LENGTH
\brief Length in bytes for sdcard file name data buffer.
*/
#define SDCARD_FILES_NAME_MAX_LENGTH      (20)

#endif
