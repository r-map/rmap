/**@file rmap_utility.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

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

#include "stima_utility.h"

#include "ctype.h"
#include "stdlib.h"
#include "stdio.h"

void getStimaNameByType(char *name, uint8_t type, uint8_t offset) {
  switch (type) {
    case STIMA_MODULE_TYPE_MASTER_ETH:
      strncpy(name, STIMA_MODULE_NAME_MASTER_ETH + offset, STIMA_MODULE_NAME_LENGTH);
      break;

    case STIMA_MODULE_TYPE_MASTER_GSM:
      strncpy(name, STIMA_MODULE_NAME_MASTER_GSM + offset, STIMA_MODULE_NAME_LENGTH);
      break;

    case STIMA_MODULE_TYPE_RAIN:
      strncpy(name, STIMA_MODULE_NAME_RAIN + offset, STIMA_MODULE_NAME_LENGTH);
      break;

    case STIMA_MODULE_TYPE_TH:
      strncpy(name, STIMA_MODULE_NAME_TH + offset, STIMA_MODULE_NAME_LENGTH);
      break;

    case STIMA_MODULE_TYPE_THR:
      strncpy(name, STIMA_MODULE_NAME_THR + offset, STIMA_MODULE_NAME_LENGTH);
      break;

    case STIMA_MODULE_TYPE_OPC:
      strncpy(name, STIMA_MODULE_NAME_OPC + offset, STIMA_MODULE_NAME_LENGTH);
      break;

    case STIMA_MODULE_TYPE_LEAF:
      strncpy(name, STIMA_MODULE_NAME_LEAF + offset, STIMA_MODULE_NAME_LENGTH);
      break;

    case STIMA_MODULE_TYPE_WIND:
      strncpy(name, STIMA_MODULE_NAME_WIND + offset, STIMA_MODULE_NAME_LENGTH);
      break;

    case STIMA_MODULE_TYPE_SOLAR_RADIATION:
      strncpy(name, STIMA_MODULE_NAME_SOLAR_RADIATION + offset, STIMA_MODULE_NAME_LENGTH);
      break;

    case STIMA_MODULE_TYPE_GAS:
      strncpy(name, STIMA_MODULE_NAME_GAS + offset, STIMA_MODULE_NAME_LENGTH);
      break;

    case STIMA_MODULE_TYPE_POWER_MPPT:
      strncpy(name, STIMA_MODULE_NAME_POWER_MPPT + offset, STIMA_MODULE_NAME_LENGTH);
      break;

    case STIMA_MODULE_TYPE_VVC:
      strncpy(name, STIMA_MODULE_NAME_VVC + offset, STIMA_MODULE_NAME_LENGTH);
      break;

    default:
      strncpy(name, "ERROR", STIMA_MODULE_NAME_LENGTH);
      break;
  }
}

void getStimaDescriptionByType(char *description, uint8_t type) {
  switch (type) {
    case STIMA_MODULE_TYPE_MASTER_ETH:
      strncpy(description, STIMA_MODULE_DESCRIPTION_MASTER_ETH, STIMA_MODULE_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_MASTER_GSM:
      strncpy(description, STIMA_MODULE_DESCRIPTION_MASTER_GSM, STIMA_MODULE_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_RAIN:
      strncpy(description, STIMA_MODULE_DESCRIPTION_RAIN, STIMA_MODULE_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_TH:
      strncpy(description, STIMA_MODULE_DESCRIPTION_TH, STIMA_MODULE_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_THR:
      strncpy(description, STIMA_MODULE_DESCRIPTION_THR, STIMA_MODULE_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_OPC:
      strncpy(description, STIMA_MODULE_DESCRIPTION_OPC, STIMA_MODULE_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_LEAF:
      strncpy(description, STIMA_MODULE_DESCRIPTION_LEAF, STIMA_MODULE_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_WIND:
      strncpy(description, STIMA_MODULE_DESCRIPTION_WIND, STIMA_MODULE_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_SOLAR_RADIATION:
      strncpy(description, STIMA_MODULE_DESCRIPTION_SOLAR_RADIATION, STIMA_MODULE_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_GAS:
      strncpy(description, STIMA_MODULE_DESCRIPTION_GAS, STIMA_MODULE_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_POWER_MPPT:
      strncpy(description, STIMA_MODULE_DESCRIPTION_POWER_MPPT, STIMA_MODULE_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_VVC:
      strncpy(description, STIMA_MODULE_DESCRIPTION_VVC, STIMA_MODULE_DESCRIPTION_LENGTH);
      break;

    default:
      strncpy(description, "ERROR", STIMA_MODULE_DESCRIPTION_LENGTH);
      break;
  }
}

void getStimaLcdDescriptionByType(char *lcd_description_A, char* lcd_description_B, uint8_t type) {
  switch (type) {
    case STIMA_MODULE_TYPE_RAIN:
      strncpy(lcd_description_A, STIMA_LCD_DESCRIPTION_RAIN, STIMA_LCD_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_TH:
      strncpy(lcd_description_A, STIMA_LCD_DESCRIPTION_TEMPERATURE, STIMA_LCD_DESCRIPTION_LENGTH);
      strncpy(lcd_description_B, STIMA_LCD_DESCRIPTION_HUMIDITY, STIMA_LCD_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_THR:
      strncpy(lcd_description_A, STIMA_LCD_DESCRIPTION_TEMPERATURE, STIMA_MODULE_DESCRIPTION_LENGTH);
      strncpy(lcd_description_B, STIMA_LCD_DESCRIPTION_HUMIDITY, STIMA_LCD_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_OPC:
      strncpy(lcd_description_A, STIMA_LCD_DESCRIPTION_OPC, STIMA_MODULE_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_LEAF:
      strncpy(lcd_description_A, STIMA_LCD_DESCRIPTION_LEAF, STIMA_LCD_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_WIND:
      strncpy(lcd_description_A, STIMA_LCD_DESCRIPTION_WIND_DIRECTION, STIMA_LCD_DESCRIPTION_LENGTH);
      strncpy(lcd_description_B, STIMA_LCD_DESCRIPTION_WIND_SPEED, STIMA_LCD_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_SOLAR_RADIATION:
      strncpy(lcd_description_A, STIMA_LCD_DESCRIPTION_SOLAR_RADIATION, STIMA_LCD_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_GAS:
      break;

    case STIMA_MODULE_TYPE_POWER_MPPT:
      strncpy(lcd_description_A, STIMA_LCD_DESCRIPTION_POWER_MPPT, STIMA_LCD_DESCRIPTION_LENGTH);
      break;

    case STIMA_MODULE_TYPE_VVC:
      strncpy(lcd_description_A, STIMA_LCD_DESCRIPTION_VVC, STIMA_LCD_DESCRIPTION_LENGTH);
      break;

    default:
      strncpy(lcd_description_A, "UNDEFINED", STIMA_LCD_DESCRIPTION_LENGTH);
      break;
  }
}

void getStimaLcdUnitTypeByType(char *lcd_unit_type_A, char *lcd_unit_type_B, uint8_t type) {
  switch (type) {
    case STIMA_MODULE_TYPE_RAIN:
      strncpy(lcd_unit_type_A, STIMA_LCD_UNIT_TYPE_MILLIMETERS, STIMA_LCD_UNIT_TYPE_LENGTH);
      break;

    case STIMA_MODULE_TYPE_TH:
      strncpy(lcd_unit_type_A, STIMA_LCD_UNIT_TYPE_CELSIUS_DEGREES, STIMA_LCD_UNIT_TYPE_LENGTH);
      strncpy(lcd_unit_type_B, STIMA_LCD_UNIT_TYPE_PERCENTS, STIMA_LCD_UNIT_TYPE_LENGTH);
      break;

    case STIMA_MODULE_TYPE_THR:
      strncpy(lcd_unit_type_A, STIMA_LCD_UNIT_TYPE_CELSIUS_DEGREES, STIMA_LCD_UNIT_TYPE_LENGTH);
      strncpy(lcd_unit_type_B, STIMA_LCD_UNIT_TYPE_PERCENTS, STIMA_LCD_UNIT_TYPE_LENGTH);
      break;

    case STIMA_MODULE_TYPE_OPC:
      break;

    case STIMA_MODULE_TYPE_LEAF:
      strncpy(lcd_unit_type_A, STIMA_LCD_UNIT_TYPE_PERCENTS, STIMA_LCD_UNIT_TYPE_LENGTH);
      break;

    case STIMA_MODULE_TYPE_WIND:
      strncpy(lcd_unit_type_A, STIMA_LCD_UNIT_TYPE_DEGREES, STIMA_LCD_UNIT_TYPE_LENGTH);
      strncpy(lcd_unit_type_B, STIMA_LCD_UNIT_TYPE_METERS_PER_SECOND, STIMA_LCD_UNIT_TYPE_LENGTH);
      break;

    case STIMA_MODULE_TYPE_SOLAR_RADIATION:
      strncpy(lcd_unit_type_A, STIMA_LCD_UNIT_TYPE_WATTS_PER_SQUARE_METER, STIMA_LCD_UNIT_TYPE_LENGTH);
      break;

    case STIMA_MODULE_TYPE_GAS:
      break;

    case STIMA_MODULE_TYPE_POWER_MPPT:
      strncpy(lcd_unit_type_A, STIMA_LCD_UNIT_TYPE_VOLTS, STIMA_LCD_UNIT_TYPE_LENGTH);
      break;

    case STIMA_MODULE_TYPE_VVC:
      strncpy(lcd_unit_type_A, STIMA_LCD_UNIT_TYPE_VOLTS, STIMA_LCD_UNIT_TYPE_LENGTH);
      break;

    default:
      strncpy(lcd_unit_type_A, "--", STIMA_LCD_UNIT_TYPE_LENGTH);
      break;
  }
}

void getStimaLcdDecimalsByType(uint8_t *decimals_A, uint8_t *decimals_B, uint8_t type) {
  switch (type) {
    case STIMA_MODULE_TYPE_RAIN:
      *decimals_A = STIMA_LCD_DECIMALS_TWO;
      break;

    case STIMA_MODULE_TYPE_TH:
      *decimals_A = STIMA_LCD_DECIMALS_TWO;
      *decimals_B = STIMA_LCD_DECIMALS_ZERO;
      break;

    case STIMA_MODULE_TYPE_THR:
      *decimals_A = STIMA_LCD_DECIMALS_TWO;
      *decimals_B = STIMA_LCD_DECIMALS_ZERO;
      break;

    case STIMA_MODULE_TYPE_OPC:
      break;

    case STIMA_MODULE_TYPE_LEAF:
      *decimals_A = STIMA_LCD_DECIMALS_ZERO;
      break;

    case STIMA_MODULE_TYPE_WIND:
      *decimals_A = STIMA_LCD_DECIMALS_ZERO;
      *decimals_B = STIMA_LCD_DECIMALS_ONE;
      break;

    case STIMA_MODULE_TYPE_SOLAR_RADIATION:
      *decimals_A = STIMA_LCD_DECIMALS_ZERO;
      break;

    case STIMA_MODULE_TYPE_GAS:
      break;

    case STIMA_MODULE_TYPE_POWER_MPPT:
      *decimals_A = STIMA_LCD_DECIMALS_TWO;
      break;

    case STIMA_MODULE_TYPE_VVC:
      *decimals_A = STIMA_LCD_DECIMALS_TWO;
      break;

    default:
      *decimals_A = STIMA_LCD_DECIMALS_ZERO;
      break;
  }
}

bool checkStimaFirmwareType(char *file_name, uint8_t *type, uint8_t *version, uint8_t *revision) {
  // Standard Cyphal-Yakut file Name
  // node_name-Ver.Rev.app.hex -> stima4.module_th-4.1-app.hex
  bool is_firmware_file = false;
  char *ptrcheck = strstr(file_name, ".app.hex");
  char field[3];
  uint8_t idx;

  // File suffix not found
  if (ptrcheck == NULL) return false;

  // Check module type from file name
  if (strstr(file_name, STIMA_MODULE_NAME_MASTER_ETH)) {
    *type = STIMA_MODULE_TYPE_MASTER_ETH;
    ptrcheck = file_name + sizeof(STIMA_MODULE_NAME_MASTER_ETH);
  } else if (strstr(file_name, STIMA_MODULE_NAME_MASTER_GSM)) {
    *type = STIMA_MODULE_TYPE_MASTER_GSM;
    ptrcheck = file_name + sizeof(STIMA_MODULE_NAME_MASTER_GSM);
  } else if (strstr(file_name, STIMA_MODULE_NAME_RAIN)) {
    *type = STIMA_MODULE_TYPE_RAIN;
    ptrcheck = file_name + sizeof(STIMA_MODULE_NAME_RAIN);
  } else if (strstr(file_name, STIMA_MODULE_NAME_TH)) {
    // stima4.module_th
    *type = STIMA_MODULE_TYPE_TH;
    ptrcheck = file_name + sizeof(STIMA_MODULE_NAME_TH);
  } else if (strstr(file_name, STIMA_MODULE_NAME_THR)) {
    *type = STIMA_MODULE_TYPE_THR;
    ptrcheck = file_name + sizeof(STIMA_MODULE_NAME_THR);
  } else if (strstr(file_name, STIMA_MODULE_NAME_OPC)) {
    *type = STIMA_MODULE_TYPE_OPC;
    ptrcheck = file_name + sizeof(STIMA_MODULE_NAME_OPC);
  } else if (strstr(file_name, STIMA_MODULE_NAME_LEAF)) {
    *type = STIMA_MODULE_TYPE_LEAF;
    ptrcheck = file_name + sizeof(STIMA_MODULE_NAME_LEAF);
  } else if (strstr(file_name, STIMA_MODULE_NAME_WIND)) {
    *type = STIMA_MODULE_TYPE_WIND;
    ptrcheck = file_name + sizeof(STIMA_MODULE_NAME_WIND);
  } else if (strstr(file_name, STIMA_MODULE_NAME_SOLAR_RADIATION)) {
    *type = STIMA_MODULE_TYPE_SOLAR_RADIATION;
    ptrcheck = file_name + sizeof(STIMA_MODULE_NAME_SOLAR_RADIATION);
  } else if (strstr(file_name, STIMA_MODULE_NAME_GAS)) {
    *type = STIMA_MODULE_TYPE_GAS;
    ptrcheck = file_name + sizeof(STIMA_MODULE_NAME_GAS);
  } else if (strstr(file_name, STIMA_MODULE_NAME_POWER_MPPT)) {
    *type = STIMA_MODULE_TYPE_POWER_MPPT;
    ptrcheck = file_name + sizeof(STIMA_MODULE_NAME_MASTER_ETH);
  } else {
    // Error unknown module file
    return false;
  }

  // ptrcheck Point to divider field, check divider char
  ptrcheck--;
  if (*ptrcheck != '-') return false;

  // Get Version
  ptrcheck++;
  memset(field, 0, 3);
  if (isdigit(*ptrcheck)) {
    idx = 0;
    while (isdigit(*ptrcheck)) {
      field[idx++] = *(ptrcheck++);
      // Error len field
      if (idx > 3) return false;
    }
  }
  *version = (uint8_t)atoi(field);

  // check separator field
  if (*ptrcheck != '.') return false;

  // Get Revision
  ptrcheck++;
  memset(field, 0, 3);
  if (isdigit(*ptrcheck)) {
    idx = 0;
    while (isdigit(*ptrcheck)) {
      field[idx++] = *(ptrcheck++);
      // Error len field
      if (idx > 3) return false;
    }
  }
  *revision = (uint8_t)atoi(field);

  // check separator field (enter suffix...)
  if (*ptrcheck != '.') return false;

  return true;
}

void setStimaFirmwareName(char *file_name, uint8_t type, uint8_t version, uint8_t revision) {
  // Standard Cyphal-Yakut file Name
  // node_name-Ver.Rev.app.hex -> stima4.module_th-4.1-app.hex
  char suffix[16] = {0};
  getStimaNameByType(file_name, type);
  // Add suffix
  sprintf(suffix, "-%d.%d.app.hex", version, revision);
  strcat(file_name, suffix);
}
