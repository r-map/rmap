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

#include <SdFat.h>
#include <Time.h>
#include <sdcard_config.h>

/*!
\fn bool sdcard_init(SdFat *SD, uint8_t chip_select)
\brief Init sdcard and check if operation was succesful.
\param[in] *SD pointer to sdcard instance.
\param[in] chip_select corresponding chip select pin.
\return true if success, false otherwise.
*/
bool sdcard_init(SdFat *SD, uint8_t chip_select);

/*!
\fn bool sdcard_open_file(SdFat *SD, File *file, const char *file_name, uint8_t param)
\brief Open file on sdcard with relative file mode bits.
\param[in] *SD pointer to sdcard instance.
\param[in] *file pointer to file instance.
\param[in] *file_name file name.
\param[in] param parameter for open file (example: O_RDWR | O_CREAT | O_APPEND. See SdFat documentation).
\return true if success, false otherwise.
*/
bool sdcard_open_file(SdFat *SD, File *file, const char *file_name, uint8_t param);

/*!
\fn void sdcard_make_filename(time_t time, char *file_name)
\brief Generate a file name (dd_mm_yyyy.txt) starting from a variable containing a seconds elapsed from 00:00:00 01/01/1900.
\param[in] *time seconds since 1900.
\param[out] *file_name file name generated from time.
\return void.
*/
void sdcard_make_filename(time_t time, char *file_name);

#endif
