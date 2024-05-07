/**@file stima_utility.h */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>
Moreno Gasperini <m.gasperini@digiteco.it>

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

#ifndef _STIMA_UTILITY_H
#define _STIMA_UTILITY_H

#include <cstdint>
#include <cstring>
#include "stima_config.h"

/*!
\fn void getStimaNameByType(char *name, uint8_t type)
\brief Return a STIMA's name starting from a module type stored in configuration.
\param[out] *name STIMA's name.
\param[in] *type module type stored in configuration.
\return void.
*/
void getStimaNameByType(char *name, uint8_t type, uint8_t offset = 0);

/*!
\fn void getStimaDescriptionByType(char *name, uint8_t type)
\brief Return a STIMA's description Cyphal from a module type stored in configuration.
\param[out] *description STIMA's description.
\param[in] *type module type stored in configuration.
\return void.
*/

void getStimaDescriptionByType(char *description, uint8_t type);

/*!
\fn void getStimaLcdDescriptionByType(char *lcd_description_X, uint8_t type)
\brief Return a STIMA's description to print with LCD display from a module type stored in configuration.
\param[out] *description STIMA's description.
\param[in] *type module type stored in configuration.
\return void.
*/
void getStimaLcdDescriptionByType(char *lcd_description_A, char *lcd_description_B, char *lcd_description_C, uint8_t type);

/*!
\fn void getStimaLcdUnitTypeByType(char *lcd_unit_type_X, uint8_t type)
\brief Return a STIMA's Unit type to print with LCD display from a module type stored in configuration.
\param[out] *description STIMA's description.
\param[in] *type module type stored in configuration.
\return void.
*/
void getStimaLcdUnitTypeByType(char *lcd_unit_type_A, char *lcd_unit_type_B, char *lcd_unit_type_C, uint8_t type);

/*!
\fn void getStimaLcdDecimalsByType(uint8_t *decimals_X, uint8_t type)
\brief Return a STIMA's Decimals for use to print value of measurement with LCD display from a module type stored in configuration.
\param[out] *description STIMA's description.
\param[in] *type module type stored in configuration.
\return void.
*/
void getStimaLcdDecimalsByType(uint8_t *decimals_A, uint8_t *decimals_B, uint8_t *decimals_C, uint8_t type);

/*!
\fn void checkStimaFirmwareType(char *file_name, uint8_t *type, uint8_t *version, uint8_t *revision)
\brief Return a STIMA's check file firmware name Type Version and Revision.
\param[out] *types Type of module checked.
\param[out] *version Version firmware of module checked.
\param[out] *revision Revision firmware of module checked.
\param[in] *file_name module name to check.
\return true if file name is correct, false if not
*/
bool checkStimaFirmwareType(char *file_name, uint8_t *type, uint8_t *version, uint8_t *revision);

/*!
\fn void setStimaFirmwareName(char *file_name, uint8_t type, uint8_t version, uint8_t revision)
\brief Return a STIMA's name file firmware by Type Version and Revision requested.
\param[in] types Type of module checked.
\param[in] version Version firmware of module checked.
\param[in] revision Revision firmware of module checked.
\param[out] *file_name module named correct cyphal.
\return true if file name is correct, false if not
*/
void setStimaFirmwareName(char *file_name, uint8_t type, uint8_t version, uint8_t revision);

#endif
