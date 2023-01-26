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

void getStimaNameByType(char *name, uint8_t type)
{
  switch (type)
  {
  case STIMA_MODULE_TYPE_MASTER_ETH:
    strncpy(name, STIMA_MODULE_NAME_MASTER_ETH, STIMA_MODULE_NAME_LENGTH);
    break;

  case STIMA_MODULE_TYPE_MASTER_GSM:
    strncpy(name, STIMA_MODULE_NAME_MASTER_GSM, STIMA_MODULE_NAME_LENGTH);
    break;

  case STIMA_MODULE_TYPE_RAIN:
    strncpy(name, STIMA_MODULE_NAME_RAIN, STIMA_MODULE_NAME_LENGTH);
    break;

  case STIMA_MODULE_TYPE_TH:
    strncpy(name, STIMA_MODULE_NAME_TH, STIMA_MODULE_NAME_LENGTH);
    break;

  case STIMA_MODULE_TYPE_THR:
    strncpy(name, STIMA_MODULE_NAME_THR, STIMA_MODULE_NAME_LENGTH);
    break;

  case STIMA_MODULE_TYPE_OPC:
    strncpy(name, STIMA_MODULE_NAME_OPC, STIMA_MODULE_NAME_LENGTH);
    break;

  case STIMA_MODULE_TYPE_LEAF:
    strncpy(name, STIMA_MODULE_NAME_LEAF, STIMA_MODULE_NAME_LENGTH);
    break;

  case STIMA_MODULE_TYPE_WIND:
    strncpy(name, STIMA_MODULE_NAME_WIND, STIMA_MODULE_NAME_LENGTH);
    break;

  case STIMA_MODULE_TYPE_SOLAR_RADIATION:
    strncpy(name, STIMA_MODULE_NAME_SOLAR_RADIATION, STIMA_MODULE_NAME_LENGTH);
    break;

  case STIMA_MODULE_TYPE_GAS:
    strncpy(name, STIMA_MODULE_NAME_GAS, STIMA_MODULE_NAME_LENGTH);
    break;

  case STIMA_MODULE_TYPE_POWER_MPPT:
    strncpy(name, STIMA_MODULE_NAME_POWER_MPPT, STIMA_MODULE_NAME_LENGTH);
    break;

  case STIMA_MODULE_TYPE_VVC:
    strncpy(name, STIMA_MODULE_NAME_VVC, STIMA_MODULE_NAME_LENGTH);
    break;

  default:
    strncpy(name, "ERROR", STIMA_MODULE_NAME_LENGTH);
    break;
    }
}
