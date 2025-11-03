/**@file sdcard_utility.h */

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

#ifndef _SDCARD_UTILITY_H
#define _SDCARD_UTILITY_H

#include <sdcard_config.h>
#include <SdFat.h>
#include <TimeLib.h>

/*!
\fn bool sdcard_init(SdFat *SD, uint8_t chip_select)
\brief Init sdcard and check if operation was succesful.
\param[in] *SD pointer to sdcard instance.
\param[in] chip_select corresponding chip select pin.
\return true if success, false otherwise.
*/
bool sdcard_init(SdFat *SD, uint8_t chip_select);

/*!
\fn bool sdcard_open_file(SdFat *SD, File *file, const char *file_name, oflag_t param)
\brief Open file on sdcard with relative file mode bits.
\param[in] *SD pointer to sdcard instance.
\param[in] *file pointer to file instance.
\param[in] *file_name file name.
\param[in] param parameter for open file (example: O_RDWR | O_CREAT | O_APPEND. See SdFat documentation).
\return true if success, false otherwise.
*/
bool sdcard_open_file(SdFat *SD, File *file, const char *file_name, oflag_t param);

/*!
\fn void sdcard_make_filename(time_t time, char *file_name)
\brief Generate a file name (dd_mm_yyyy.txt) starting from a variable containing a seconds elapsed from 00:00:00 01/01/1900.
\param[in] *time seconds since 1900.
\param[out] *file_name file name generated from time.
\return void.
*/
void sdcard_make_filename(time_t time, char *file_name);


/*!
\fn void sdcard_remove_firmware(const uint8_t main_version, const uint8_t minor_version)
\brief Remove the firmware corrispondig to the version requested. Remove file named "mainversion.minorversion" and "FIRMWARE.BIN" if the first exist.
\param[in] *SD pointer to sdcard instance.
\param[in] firmware main version.
\param[in] firmware minor version.
\return bool (true if firmware was removed).
*/
bool sdcard_remove_firmware(SdFat *SD, const uint8_t main_version, const uint8_t minor_version);

#endif
