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

void getStimaNameByType(char *name, uint8_t type) {
  switch (type) {
    case STIMA_MODULE_TYPE_MASTER_ETH:
    strcpy(name, STIMA_MODULE_NAME_MASTER_ETH);
    break;

    case STIMA_MODULE_TYPE_MASTER_GSM:
    strcpy(name, STIMA_MODULE_NAME_MASTER_GSM);
    break;

    case STIMA_MODULE_TYPE_RAIN:
    strcpy(name, STIMA_MODULE_NAME_RAIN);
    break;

    case STIMA_MODULE_TYPE_TH:
    strcpy(name, STIMA_MODULE_NAME_TH);
    break;

    case STIMA_MODULE_TYPE_THR:
    strcpy(name, STIMA_MODULE_NAME_THR);
    break;

    case STIMA_MODULE_TYPE_OPC:
    strcpy(name, STIMA_MODULE_NAME_OPC);
    break;

    case STIMA_MODULE_TYPE_LEAF:
    strcpy(name, STIMA_MODULE_NAME_LEAF);
    break;

    case STIMA_MODULE_TYPE_WIND:
    strcpy(name, STIMA_MODULE_NAME_WIND);
    break;

    case STIMA_MODULE_TYPE_SOLAR_RADIATION:
    strcpy(name, STIMA_MODULE_NAME_SOLAR_RADIATION);
    break;

    case STIMA_MODULE_TYPE_GAS:
    strcpy(name, STIMA_MODULE_NAME_GAS);
    break;

    default:
    strcpy(name, "ERROR");
    break;
  }
}
