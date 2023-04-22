/**
  ******************************************************************************
  * @file    Mppt.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   Mppt cotroller Analog LTC4015 driver file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
  * All rights reserved.</center></h2>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  * 
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  * <http://www.gnu.org/licenses/>.
  * 
  ******************************************************************************
*/
#include "drivers/mppt.h"

/// @brief Constructor base
Mppt::Mppt()
{
}

/// @brief Constructor Overload Wire && Semaphore
/// @param wire I2C Wire used on system
/// @param wireLock BinarySemaphore Lock I2C devices
/// @param i2c_address (opz. I2C device address)
Mppt::Mppt(TwoWire *wire, BinarySemaphore *wireLock, uint8_t i2c_address)
{
	_wire = wire;
	_wireLock = wireLock;
	_i2c_address = i2c_address;
}

/// @brief LTC4015 Interface Write Register SMBus
/// @param registerinfo LTC4015 Register
/// @param data Data to write (LTC4015 Format Register)
/// @return 
bool Mppt::LTC4015_write_register(uint16_t registerinfo, uint16_t data)
{
  uint8_t command_code = get_subaddr(registerinfo);
  if (get_size(registerinfo) != 16)
  {
    uint8_t offset = get_offset(registerinfo);
    uint16_t mask = get_mask(registerinfo);
    uint16_t read_data;
    // Check register
    if(!smbus_read_reg(command_code, &read_data)) return false;
    data = (read_data & ~mask) | (data << offset);
  }
  return smbus_write_reg(command_code, data);
}

/// @brief LTC4015 Interface Read Register SMBus
/// @param registerinfo Register LTC4015 Supported Format (RegDef && Format)
/// @param data Data readed from LTC4015 device
/// @return true if reading OK
bool Mppt::LTC4015_read_register(uint16_t registerinfo, uint16_t *data)
{
  bool result;
  uint8_t command_code = get_subaddr(registerinfo);
  uint8_t offset = get_offset(registerinfo);
  uint16_t mask = get_mask(registerinfo);
  result = smbus_read_reg(command_code, data);
  *data &= mask;
  *data = *data >> offset;
  return result;
}

/// @brief Read LTC4015 V_IN parameter
/// @param  None
/// @return V_IN Value converted to V
float Mppt::get_V_IN(void) {
  uint16_t data;
  float iBatt = get_I_BAT();
  LTC4015_read_register(LTC4015_VIN, &data);
  float vIn = ((float)data * V_REF_IN + V_DQ1_OFFS) / V_REF_CVAL;
  if(iBatt>V_REF_A_RCHG) {
    vIn+=V_REF_V_DCHG;
  } else if(iBatt>0) vIn+=(V_REF_V_DCHG * iBatt / V_REF_A_RCHG);
  return vIn;
}

/// @brief Read LTC4015 V_SYS parameter
/// @param  None
/// @return V_SYS Value converted to V
float Mppt::get_V_SYS(void) {
  uint16_t data;
  LTC4015_read_register(LTC4015_VSYS, &data);
  float vSys = ((float)data * V_REF_IN + V_DQ1_OFFS) / V_REF_CVAL;
  return vSys;
}

/// @brief Read LTC4015 V_BAT parameter
/// @param  None
/// @return V_BAT Value converted to V
float Mppt::get_V_BAT(void) {
  uint16_t data;
  float iBatt = get_I_BAT();
  LTC4015_read_register(LTC4015_VBAT, &data);
  float vBatt = ((float)data / V_REF_LH_BAT) / V_REF_BVAL;
  if(iBatt>V_REF_A_RCHG) {
    vBatt-=V_REF_V_DCHG;
  } else if(iBatt>0) vBatt-=(V_REF_V_DCHG * iBatt / V_REF_A_RCHG);
  return vBatt;
}

/// @brief Read LTC4015 I_BAT parameter
/// @param  None
/// @return I_BAT Value converted to I Ampere
float Mppt::get_I_BAT(void) {
  int16_t data;
  LTC4015_read_register(LTC4015_IBAT, (uint16_t*) &data);
  float iBatt = ((float)data * I_REF_BATT) / I_REF_CVAL;
  if (iBatt<0) iBatt = 0;
  return iBatt;
}

/// @brief Read LTC4015 I_IN parameter
/// @param  None
/// @return I_IN Value converted to I Ampere
float Mppt::get_I_IN(void) {
  uint16_t data;
  LTC4015_read_register(LTC4015_IIN, &data);
  float iIn = ((float)data * I_REF_IN) / I_REF_CVAL;
  return iIn;
}

float Mppt::get_P_CHG(void) {
  uint16_t data;
  float pChg;
  float vBatt = get_V_BAT();
  if(vBatt>13.35) pChg = 100.0;
  else if(vBatt>13.27) { pChg = 98.73624 + ((vBatt - 13.27) * 18.05371429); }
  else if(vBatt>13.17) { pChg = 94.667243 + ((vBatt - 13.17) * 40.68997); }
  else if(vBatt>13.06) { pChg = 90.23132 + ((vBatt - 13.06) * 40.32657273); }
  else if(vBatt>12.91) { pChg = 83.93915757 + ((vBatt - 12.91) * 41.94774952); }
  else if(vBatt>12.77) { pChg = 77.13744076 + ((vBatt - 12.77) * 48.5836915); }
  else if(vBatt>12.63) { pChg = 64.27316294 + ((vBatt - 12.63) * 91.88769871); }
  else if(vBatt>12.47) { pChg = 49.35379644 + ((vBatt - 12.47) * 93.24604062); }
  else if(vBatt>12.30) { pChg = 39.47583948 + ((vBatt - 12.30) * 58.10562922); }
  else if(vBatt>12.07) { pChg = 29.40298507 + ((vBatt - 12.07) * 43.79501914); }
  else if(vBatt>11.75) { pChg = 19.62711864 + ((vBatt - 11.75) * 40.7327768); }
  else if(vBatt>11.40) { pChg = 14.54347826 + ((vBatt - 11.40) * 11.82241949); }
  else if(vBatt>11.05) { pChg = 9.713518356 + ((vBatt - 11.10) * 16.09986635); }
  else if(vBatt>10.75) { pChg = 4.838709677 + ((vBatt - 10.75) * 13.9280248); }
  else pChg = 0.0;  
  return pChg;
}


// ****************************************************************************************
// ******************************* PRIVATE FUNCTION ***************************************
// ****************************************************************************************

/// @brief Read generic SMBus register
/// @param reg register to read
/// @param data data readed in register
/// @return true -> no Error
bool Mppt::smbus_read_reg(uint16_t reg, uint16_t *data)
{
  // Try lock Semaphore
	if (_wireLock->Take(Ticks::MsToTicks(1000)))
	{
    uint8_t lsb;
    uint8_t msb;
    /* Read SMBus register LTC4015 */
    _wire->beginTransmission(_i2c_address);
    _wire->write(reg);
    _wire->endTransmission();
    // Read Operation request x Word Register
    _wire->requestFrom(_i2c_address, 2);
    lsb = _wire->read();
    msb = _wire->read();
    _wire->endTransmission();
		_wireLock->Give();
    *data = (msb << 8) | lsb;
    return true;
  } else 
    return false;
}

/// @brief Write generic SMBus register
/// @param reg register to write
/// @param data data to write in register
/// @return true -> no Error
bool Mppt::smbus_write_reg(uint16_t reg, uint16_t data)
{
  // Try lock Semaphore
  bool retVal = false;
	if (_wireLock->Take(Ticks::MsToTicks(1000)))
	{
    /* Write SMBus register LTC4015 */
    _wire->beginTransmission(_i2c_address);
    _wire->write(reg);
    // Write LSB, MSB Order Byte
    _wire->write((uint8_t)(data));
    _wire->write((uint8_t)(data >> 8));
    retVal = _wire->endTransmission() == 0;
		_wireLock->Give();
  } 
  return retVal;
}

/// @brief LTC_4015 Get Size register
/// @param registerinfo Register command
/// @return LTC4015 Format Register Size
uint8_t Mppt::get_size(uint16_t registerinfo)
{
  return ((registerinfo >> 8) & 0x0F) + 1;
}

/// @brief LTC_4015 Get Subaddress register
/// @param registerinfo Register command
/// @return LTC4015 Format Register Subaddress
uint8_t Mppt::get_subaddr(uint16_t registerinfo)
{
  return (registerinfo) & 0xFF;
}

/// @brief LTC_4015 Get offset register
/// @param registerinfo Register command
/// @return LTC4015 Format Register Offset
uint8_t Mppt::get_offset(uint16_t registerinfo)
{
  return (registerinfo >> 12) & 0x0F;
}

/// @brief LTC_4015 Get Bit Mask register
/// @param registerinfo Register command
/// @return LTC4015 Format Register Bit Mask
uint16_t Mppt::get_mask(uint16_t registerinfo)
{
  uint16_t mask = 1 << get_offset(registerinfo);
  uint8_t size = get_size(registerinfo);
  uint8_t i;
  for (i=0; i<size-1; i++)
  {
    mask |= mask << 1;
  }
  return mask;
}

/**
  * @}
  *
  */

/************************ (C) COPYRIGHT Digiteco s.r.l. *****END OF FILE****/
