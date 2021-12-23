/**@file sdcard_utility.cpp */

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

#include "sdcard_utility.h"

bool sdcard_init(SdFat *SD, uint8_t chip_select) {
#ifdef ARDUINO_ARCH_AVR
  return (SD->begin(chip_select) && SD->vol()->fatType());
#else
#define SPI_SPEED SD_SCK_MHZ(4)
  return (SD->begin(chip_select,SPI_SPEED) && SD->vol()->fatType());
#endif
  
}

bool sdcard_open_file(SdFat *SD, File *file, const char *file_name, oflag_t param) {
   *file = SD->open(file_name, param);

   if (*file) {
      return true;
   }

   return false;
}

void sdcard_make_filename(time_t time, char *file_name) {
   snprintf(file_name, SDCARD_FILES_NAME_MAX_LENGTH, "%04u_%02u_%02u.txt", year(time), month(time), day(time));
}


bool sdcard_remove_firmware(SdFat *SD, const uint8_t main_version, const uint8_t minor_version){

  char *firmware = "FIRMWARE.BIN";
  char firmware_done[11];
  char version[7];

  sprintf(version,"%d.%d",main_version,minor_version);
  sprintf(firmware_done,"%d_%d.BIN",main_version,minor_version);
  
  if (SD->exists(version)){
    SD->remove(firmware_done);  
    if (SD->rename(firmware, firmware_done)){  
      if (SD->remove(version)) return true;
    }
  }
  return false;
}
