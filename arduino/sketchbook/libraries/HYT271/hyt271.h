/**********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
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

#ifndef _HYT271_h
#define _HYT271_h

#ifdef __cplusplus
extern "C"
{
#endif
	
#define I2C_HYT271_DEFAULT_ADDRESS				(0x28)
#define I2C_HYT271_READ_HT_DATA_LENGTH				(4)

unsigned char HYT271_getHT(unsigned long, float *, float *);
	
#ifdef __cplusplus
}
#endif

#endif

