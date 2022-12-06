/*
LTC4015: Multichemistry Buck Battery Charger Controller with Digital Telemetry System

@verbatim
The  LTC®4015  is  a  complete  synchronous  buck  controller/charger  with pin
selectable,  chemistry specific charging and termination algorithms. The LTC4015
can  charge  Li-Ion/Polymer,  LiFePO4,  or  leadacid  batteries.  Battery charge
voltage  is  pin  selectable and I²C adjustable. Input current limit and charge
current   can   be  accurately  programmed  with  sense  resistors  and  can  be
individually  adjusted  via  the  I²C  serial  port. A digital telemetry system
monitors  all  system  power  parameters.  Safety  timer and current termination
algorithms  are  supported  for  lithium  chemistry  batteries. The LTC4015 also
includes  automatic  recharge, precharge (Li-Ion) and NTC thermistor protection.
The LTC4015's I²C port allows user customization of charger algorithms, reading
of  charger  status  information, configuration of the maskable and programmable
alerts,  plus  use  and  configuration  of  the  Coulomb counter. Available in a
38-Lead 5mm × 7mm QFN package.
@endverbatim

http://www.linear.com/product/LTC4015

http://www.linear.com/product/LTC4015#demoboards

REVISION HISTORY
$Revision: $
$Date: $

Copyright (c) 2016, Linear Technology Corp.(LTC)
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1.  Redistributions  of source code must retain the above copyright notice, this
    list  of conditions and the following disclaimer.

2.  Redistributions  in  binary  form must reproduce the above copyright notice,
    this  list of conditions and  the following disclaimer in the  documentation
    and/or other materials provided with the distribution.

THIS  SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY  EXPRESS  OR  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES   OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY  DIRECT,  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING,  BUT  NOT  LIMITED  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS  OF  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT  LIABILITY,  OR TORT
(INCLUDING  NEGLIGENCE  OR  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The  views and conclusions contained in the software and documentation are those
of  the authors and should not be interpreted as representing official policies,
either expressed or implied, of Linear Technology Corp.

The Linear Technology Linduino is not affiliated with the official Arduino team.
However,  the Linduino is only possible because of the Arduino team's commitment
to   the   open-source   community.   Please,  visit  http://www.arduino.cc  and
http://store.arduino.cc  ,  and  consider  a  purchase that will help fund their
ongoing work.

Generated on: 2016-01-08
*/

/*! @file
 *  @ingroup LTC4015
 *  @brief LTC4015 Register Map Definition Header
 *
 *
 *  This file contains LTC4015 definitions for each command_code as well as
 *  each individual bit field for the case when a register contains multiple
 *  bit-packed fields smaller than the register width.
 *  Each bit field name is prepended with LTC4015_.
 *  Each bit field has individual definitions for its _SIZE, _OFFSET (LSB) and _MASK,
 *  as well as the three fields stored in a single 16-bit word for use with the access
 *  functions provided by LTC4015.c and LTC4015.h.
 *  In the case that the bit field contents represent an enumeration, _PRESET
 *  definitions exists to translate from human readable format to the encoded value.
 *  See @ref LTC4015_register_map for detailed descriptions of each bit field.
 */

/*! @defgroup LTC4015_register_map LTC4015 Register Map Definitions
 *  @ingroup LTC4015
 */

#ifndef LTC4015_REG_DEFS_H_
#define LTC4015_REG_DEFS_H_

#define LTC4015_ADDR_68 0x68 //!<LTC4015 I2C address in 7-bit format
/*! @defgroup LTC4015_VBAT_LO_ALERT_LIMIT VBAT_LO_ALERT_LIMIT
 *  @ingroup LTC4015_register_map
 *  @brief VBAT_LO_ALERT_LIMIT Register
 *
 * |                   15:0 |
 * |:----------------------:|
 * | VBAT_LO_ALERT_LIMIT_BF |
 *
 *   - CommandCode: 1
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VBAT_LO_ALERT_LIMIT_BF "VBAT_LO_ALERT_LIMIT_BF" : Battery voltage low alert limit as reported in VBAT
*/

//!@{
#define LTC4015_VBAT_LO_ALERT_LIMIT_SUBADDR 1
#define LTC4015_VBAT_LO_ALERT_LIMIT (0 << 12 | (16 - 1) << 8 | LTC4015_VBAT_LO_ALERT_LIMIT_SUBADDR)
//!@}
/*! @defgroup LTC4015_VBAT_LO_ALERT_LIMIT_BF VBAT_LO_ALERT_LIMIT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VBAT_LO_ALERT_LIMIT_BF Bit Field
 *
 *  Battery voltage low alert limit as reported in VBAT
 *   - Register: @ref LTC4015_VBAT_LO_ALERT_LIMIT "VBAT_LO_ALERT_LIMIT"
 *   - CommandCode: 1
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_VBAT_LO_ALERT_LIMIT_BF_SUBADDR LTC4015_VBAT_LO_ALERT_LIMIT_SUBADDR //!< @ref LTC4015_VBAT_LO_ALERT_LIMIT_BF "VBAT_LO_ALERT_LIMIT_BF"
#define LTC4015_VBAT_LO_ALERT_LIMIT_BF_SIZE 16
#define LTC4015_VBAT_LO_ALERT_LIMIT_BF_OFFSET 0
#define LTC4015_VBAT_LO_ALERT_LIMIT_BF_MASK 0xFFFF
#define LTC4015_VBAT_LO_ALERT_LIMIT_BF (LTC4015_VBAT_LO_ALERT_LIMIT_BF_OFFSET << 12 | (LTC4015_VBAT_LO_ALERT_LIMIT_BF_SIZE - 1) << 8 | LTC4015_VBAT_LO_ALERT_LIMIT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VBAT_HI_ALERT_LIMIT VBAT_HI_ALERT_LIMIT
 *  @ingroup LTC4015_register_map
 *  @brief VBAT_HI_ALERT_LIMIT Register
 *
 * |                   15:0 |
 * |:----------------------:|
 * | VBAT_HI_ALERT_LIMIT_BF |
 *
 *   - CommandCode: 2
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VBAT_HI_ALERT_LIMIT_BF "VBAT_HI_ALERT_LIMIT_BF" : Battery voltage high alert limit as reported in VBAT
*/

//!@{
#define LTC4015_VBAT_HI_ALERT_LIMIT_SUBADDR 2
#define LTC4015_VBAT_HI_ALERT_LIMIT (0 << 12 | (16 - 1) << 8 | LTC4015_VBAT_HI_ALERT_LIMIT_SUBADDR)
//!@}
/*! @defgroup LTC4015_VBAT_HI_ALERT_LIMIT_BF VBAT_HI_ALERT_LIMIT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VBAT_HI_ALERT_LIMIT_BF Bit Field
 *
 *  Battery voltage high alert limit as reported in VBAT
 *   - Register: @ref LTC4015_VBAT_HI_ALERT_LIMIT "VBAT_HI_ALERT_LIMIT"
 *   - CommandCode: 2
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_VBAT_HI_ALERT_LIMIT_BF_SUBADDR LTC4015_VBAT_HI_ALERT_LIMIT_SUBADDR //!< @ref LTC4015_VBAT_HI_ALERT_LIMIT_BF "VBAT_HI_ALERT_LIMIT_BF"
#define LTC4015_VBAT_HI_ALERT_LIMIT_BF_SIZE 16
#define LTC4015_VBAT_HI_ALERT_LIMIT_BF_OFFSET 0
#define LTC4015_VBAT_HI_ALERT_LIMIT_BF_MASK 0xFFFF
#define LTC4015_VBAT_HI_ALERT_LIMIT_BF (LTC4015_VBAT_HI_ALERT_LIMIT_BF_OFFSET << 12 | (LTC4015_VBAT_HI_ALERT_LIMIT_BF_SIZE - 1) << 8 | LTC4015_VBAT_HI_ALERT_LIMIT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VIN_LO_ALERT_LIMIT VIN_LO_ALERT_LIMIT
 *  @ingroup LTC4015_register_map
 *  @brief VIN_LO_ALERT_LIMIT Register
 *
 * |                  15:0 |
 * |:---------------------:|
 * | VIN_LO_ALERT_LIMIT_BF |
 *
 *   - CommandCode: 3
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VIN_LO_ALERT_LIMIT_BF "VIN_LO_ALERT_LIMIT_BF" : Input voltage low alert limit as reported in VIN
*/

//!@{
#define LTC4015_VIN_LO_ALERT_LIMIT_SUBADDR 3
#define LTC4015_VIN_LO_ALERT_LIMIT (0 << 12 | (16 - 1) << 8 | LTC4015_VIN_LO_ALERT_LIMIT_SUBADDR)
//!@}
/*! @defgroup LTC4015_VIN_LO_ALERT_LIMIT_BF VIN_LO_ALERT_LIMIT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VIN_LO_ALERT_LIMIT_BF Bit Field
 *
 *  Input voltage low alert limit as reported in VIN
 *   - Register: @ref LTC4015_VIN_LO_ALERT_LIMIT "VIN_LO_ALERT_LIMIT"
 *   - CommandCode: 3
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_VIN_LO_ALERT_LIMIT_BF_SUBADDR LTC4015_VIN_LO_ALERT_LIMIT_SUBADDR //!< @ref LTC4015_VIN_LO_ALERT_LIMIT_BF "VIN_LO_ALERT_LIMIT_BF"
#define LTC4015_VIN_LO_ALERT_LIMIT_BF_SIZE 16
#define LTC4015_VIN_LO_ALERT_LIMIT_BF_OFFSET 0
#define LTC4015_VIN_LO_ALERT_LIMIT_BF_MASK 0xFFFF
#define LTC4015_VIN_LO_ALERT_LIMIT_BF (LTC4015_VIN_LO_ALERT_LIMIT_BF_OFFSET << 12 | (LTC4015_VIN_LO_ALERT_LIMIT_BF_SIZE - 1) << 8 | LTC4015_VIN_LO_ALERT_LIMIT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VIN_HI_ALERT_LIMIT VIN_HI_ALERT_LIMIT
 *  @ingroup LTC4015_register_map
 *  @brief VIN_HI_ALERT_LIMIT Register
 *
 * |                  15:0 |
 * |:---------------------:|
 * | VIN_HI_ALERT_LIMIT_BF |
 *
 *   - CommandCode: 4
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VIN_HI_ALERT_LIMIT_BF "VIN_HI_ALERT_LIMIT_BF" : Input voltage high alert limit as reported in VIN
*/

//!@{
#define LTC4015_VIN_HI_ALERT_LIMIT_SUBADDR 4
#define LTC4015_VIN_HI_ALERT_LIMIT (0 << 12 | (16 - 1) << 8 | LTC4015_VIN_HI_ALERT_LIMIT_SUBADDR)
//!@}
/*! @defgroup LTC4015_VIN_HI_ALERT_LIMIT_BF VIN_HI_ALERT_LIMIT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VIN_HI_ALERT_LIMIT_BF Bit Field
 *
 *  Input voltage high alert limit as reported in VIN
 *   - Register: @ref LTC4015_VIN_HI_ALERT_LIMIT "VIN_HI_ALERT_LIMIT"
 *   - CommandCode: 4
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_VIN_HI_ALERT_LIMIT_BF_SUBADDR LTC4015_VIN_HI_ALERT_LIMIT_SUBADDR //!< @ref LTC4015_VIN_HI_ALERT_LIMIT_BF "VIN_HI_ALERT_LIMIT_BF"
#define LTC4015_VIN_HI_ALERT_LIMIT_BF_SIZE 16
#define LTC4015_VIN_HI_ALERT_LIMIT_BF_OFFSET 0
#define LTC4015_VIN_HI_ALERT_LIMIT_BF_MASK 0xFFFF
#define LTC4015_VIN_HI_ALERT_LIMIT_BF (LTC4015_VIN_HI_ALERT_LIMIT_BF_OFFSET << 12 | (LTC4015_VIN_HI_ALERT_LIMIT_BF_SIZE - 1) << 8 | LTC4015_VIN_HI_ALERT_LIMIT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VSYS_LO_ALERT_LIMIT VSYS_LO_ALERT_LIMIT
 *  @ingroup LTC4015_register_map
 *  @brief VSYS_LO_ALERT_LIMIT Register
 *
 * |                   15:0 |
 * |:----------------------:|
 * | VSYS_LO_ALERT_LIMIT_BF |
 *
 *   - CommandCode: 5
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VSYS_LO_ALERT_LIMIT_BF "VSYS_LO_ALERT_LIMIT_BF" : System voltage low alert limit as reported in VSYS
*/

//!@{
#define LTC4015_VSYS_LO_ALERT_LIMIT_SUBADDR 5
#define LTC4015_VSYS_LO_ALERT_LIMIT (0 << 12 | (16 - 1) << 8 | LTC4015_VSYS_LO_ALERT_LIMIT_SUBADDR)
//!@}
/*! @defgroup LTC4015_VSYS_LO_ALERT_LIMIT_BF VSYS_LO_ALERT_LIMIT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VSYS_LO_ALERT_LIMIT_BF Bit Field
 *
 *  System voltage low alert limit as reported in VSYS
 *   - Register: @ref LTC4015_VSYS_LO_ALERT_LIMIT "VSYS_LO_ALERT_LIMIT"
 *   - CommandCode: 5
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_VSYS_LO_ALERT_LIMIT_BF_SUBADDR LTC4015_VSYS_LO_ALERT_LIMIT_SUBADDR //!< @ref LTC4015_VSYS_LO_ALERT_LIMIT_BF "VSYS_LO_ALERT_LIMIT_BF"
#define LTC4015_VSYS_LO_ALERT_LIMIT_BF_SIZE 16
#define LTC4015_VSYS_LO_ALERT_LIMIT_BF_OFFSET 0
#define LTC4015_VSYS_LO_ALERT_LIMIT_BF_MASK 0xFFFF
#define LTC4015_VSYS_LO_ALERT_LIMIT_BF (LTC4015_VSYS_LO_ALERT_LIMIT_BF_OFFSET << 12 | (LTC4015_VSYS_LO_ALERT_LIMIT_BF_SIZE - 1) << 8 | LTC4015_VSYS_LO_ALERT_LIMIT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VSYS_HI_ALERT_LIMIT VSYS_HI_ALERT_LIMIT
 *  @ingroup LTC4015_register_map
 *  @brief VSYS_HI_ALERT_LIMIT Register
 *
 * |                   15:0 |
 * |:----------------------:|
 * | VSYS_HI_ALERT_LIMIT_BF |
 *
 *   - CommandCode: 6
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VSYS_HI_ALERT_LIMIT_BF "VSYS_HI_ALERT_LIMIT_BF" : System voltage high alert limit as reported in VSYS
*/

//!@{
#define LTC4015_VSYS_HI_ALERT_LIMIT_SUBADDR 6
#define LTC4015_VSYS_HI_ALERT_LIMIT (0 << 12 | (16 - 1) << 8 | LTC4015_VSYS_HI_ALERT_LIMIT_SUBADDR)
//!@}
/*! @defgroup LTC4015_VSYS_HI_ALERT_LIMIT_BF VSYS_HI_ALERT_LIMIT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VSYS_HI_ALERT_LIMIT_BF Bit Field
 *
 *  System voltage high alert limit as reported in VSYS
 *   - Register: @ref LTC4015_VSYS_HI_ALERT_LIMIT "VSYS_HI_ALERT_LIMIT"
 *   - CommandCode: 6
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_VSYS_HI_ALERT_LIMIT_BF_SUBADDR LTC4015_VSYS_HI_ALERT_LIMIT_SUBADDR //!< @ref LTC4015_VSYS_HI_ALERT_LIMIT_BF "VSYS_HI_ALERT_LIMIT_BF"
#define LTC4015_VSYS_HI_ALERT_LIMIT_BF_SIZE 16
#define LTC4015_VSYS_HI_ALERT_LIMIT_BF_OFFSET 0
#define LTC4015_VSYS_HI_ALERT_LIMIT_BF_MASK 0xFFFF
#define LTC4015_VSYS_HI_ALERT_LIMIT_BF (LTC4015_VSYS_HI_ALERT_LIMIT_BF_OFFSET << 12 | (LTC4015_VSYS_HI_ALERT_LIMIT_BF_SIZE - 1) << 8 | LTC4015_VSYS_HI_ALERT_LIMIT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_IIN_HI_ALERT_LIMIT IIN_HI_ALERT_LIMIT
 *  @ingroup LTC4015_register_map
 *  @brief IIN_HI_ALERT_LIMIT Register
 *
 * |                  15:0 |
 * |:---------------------:|
 * | IIN_HI_ALERT_LIMIT_BF |
 *
 *   - CommandCode: 7
 *   - Contains Bit Fields:
 *     + @ref LTC4015_IIN_HI_ALERT_LIMIT_BF "IIN_HI_ALERT_LIMIT_BF" : Input current high alert limit as reported in IIN
*/

//!@{
#define LTC4015_IIN_HI_ALERT_LIMIT_SUBADDR 7
#define LTC4015_IIN_HI_ALERT_LIMIT (0 << 12 | (16 - 1) << 8 | LTC4015_IIN_HI_ALERT_LIMIT_SUBADDR)
//!@}
/*! @defgroup LTC4015_IIN_HI_ALERT_LIMIT_BF IIN_HI_ALERT_LIMIT_BF
 *  @ingroup LTC4015_register_map
 *  @brief IIN_HI_ALERT_LIMIT_BF Bit Field
 *
 *  Input current high alert limit as reported in IIN
 *   - Register: @ref LTC4015_IIN_HI_ALERT_LIMIT "IIN_HI_ALERT_LIMIT"
 *   - CommandCode: 7
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_IIN_HI_ALERT_LIMIT_BF_SUBADDR LTC4015_IIN_HI_ALERT_LIMIT_SUBADDR //!< @ref LTC4015_IIN_HI_ALERT_LIMIT_BF "IIN_HI_ALERT_LIMIT_BF"
#define LTC4015_IIN_HI_ALERT_LIMIT_BF_SIZE 16
#define LTC4015_IIN_HI_ALERT_LIMIT_BF_OFFSET 0
#define LTC4015_IIN_HI_ALERT_LIMIT_BF_MASK 0xFFFF
#define LTC4015_IIN_HI_ALERT_LIMIT_BF (LTC4015_IIN_HI_ALERT_LIMIT_BF_OFFSET << 12 | (LTC4015_IIN_HI_ALERT_LIMIT_BF_SIZE - 1) << 8 | LTC4015_IIN_HI_ALERT_LIMIT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_IBAT_LO_ALERT_LIMIT IBAT_LO_ALERT_LIMIT
 *  @ingroup LTC4015_register_map
 *  @brief IBAT_LO_ALERT_LIMIT Register
 *
 * |                   15:0 |
 * |:----------------------:|
 * | IBAT_LO_ALERT_LIMIT_BF |
 *
 *   - CommandCode: 8
 *   - Contains Bit Fields:
 *     + @ref LTC4015_IBAT_LO_ALERT_LIMIT_BF "IBAT_LO_ALERT_LIMIT_BF" : Charge current low alert limit as reported in IBAT
*/

//!@{
#define LTC4015_IBAT_LO_ALERT_LIMIT_SUBADDR 8
#define LTC4015_IBAT_LO_ALERT_LIMIT (0 << 12 | (16 - 1) << 8 | LTC4015_IBAT_LO_ALERT_LIMIT_SUBADDR)
//!@}
/*! @defgroup LTC4015_IBAT_LO_ALERT_LIMIT_BF IBAT_LO_ALERT_LIMIT_BF
 *  @ingroup LTC4015_register_map
 *  @brief IBAT_LO_ALERT_LIMIT_BF Bit Field
 *
 *  Charge current low alert limit as reported in IBAT
 *   - Register: @ref LTC4015_IBAT_LO_ALERT_LIMIT "IBAT_LO_ALERT_LIMIT"
 *   - CommandCode: 8
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_IBAT_LO_ALERT_LIMIT_BF_SUBADDR LTC4015_IBAT_LO_ALERT_LIMIT_SUBADDR //!< @ref LTC4015_IBAT_LO_ALERT_LIMIT_BF "IBAT_LO_ALERT_LIMIT_BF"
#define LTC4015_IBAT_LO_ALERT_LIMIT_BF_SIZE 16
#define LTC4015_IBAT_LO_ALERT_LIMIT_BF_OFFSET 0
#define LTC4015_IBAT_LO_ALERT_LIMIT_BF_MASK 0xFFFF
#define LTC4015_IBAT_LO_ALERT_LIMIT_BF (LTC4015_IBAT_LO_ALERT_LIMIT_BF_OFFSET << 12 | (LTC4015_IBAT_LO_ALERT_LIMIT_BF_SIZE - 1) << 8 | LTC4015_IBAT_LO_ALERT_LIMIT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_DIE_TEMP_HI_ALERT_LIMIT DIE_TEMP_HI_ALERT_LIMIT
 *  @ingroup LTC4015_register_map
 *  @brief DIE_TEMP_HI_ALERT_LIMIT Register
 *
 * |                       15:0 |
 * |:--------------------------:|
 * | DIE_TEMP_HI_ALERT_LIMIT_BF |
 *
 *   - CommandCode: 9
 *   - Contains Bit Fields:
 *     + @ref LTC4015_DIE_TEMP_HI_ALERT_LIMIT_BF "DIE_TEMP_HI_ALERT_LIMIT_BF" : Die temperature high alert limit as reported in DIE_TEMP
*/

//!@{
#define LTC4015_DIE_TEMP_HI_ALERT_LIMIT_SUBADDR 9
#define LTC4015_DIE_TEMP_HI_ALERT_LIMIT (0 << 12 | (16 - 1) << 8 | LTC4015_DIE_TEMP_HI_ALERT_LIMIT_SUBADDR)
//!@}
/*! @defgroup LTC4015_DIE_TEMP_HI_ALERT_LIMIT_BF DIE_TEMP_HI_ALERT_LIMIT_BF
 *  @ingroup LTC4015_register_map
 *  @brief DIE_TEMP_HI_ALERT_LIMIT_BF Bit Field
 *
 *  Die temperature high alert limit as reported in DIE_TEMP
 *   - Register: @ref LTC4015_DIE_TEMP_HI_ALERT_LIMIT "DIE_TEMP_HI_ALERT_LIMIT"
 *   - CommandCode: 9
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_DIE_TEMP_HI_ALERT_LIMIT_BF_SUBADDR LTC4015_DIE_TEMP_HI_ALERT_LIMIT_SUBADDR //!< @ref LTC4015_DIE_TEMP_HI_ALERT_LIMIT_BF "DIE_TEMP_HI_ALERT_LIMIT_BF"
#define LTC4015_DIE_TEMP_HI_ALERT_LIMIT_BF_SIZE 16
#define LTC4015_DIE_TEMP_HI_ALERT_LIMIT_BF_OFFSET 0
#define LTC4015_DIE_TEMP_HI_ALERT_LIMIT_BF_MASK 0xFFFF
#define LTC4015_DIE_TEMP_HI_ALERT_LIMIT_BF (LTC4015_DIE_TEMP_HI_ALERT_LIMIT_BF_OFFSET << 12 | (LTC4015_DIE_TEMP_HI_ALERT_LIMIT_BF_SIZE - 1) << 8 | LTC4015_DIE_TEMP_HI_ALERT_LIMIT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_BSR_HI_ALERT_LIMIT BSR_HI_ALERT_LIMIT
 *  @ingroup LTC4015_register_map
 *  @brief BSR_HI_ALERT_LIMIT Register
 *
 * |                  15:0 |
 * |:---------------------:|
 * | BSR_HI_ALERT_LIMIT_BF |
 *
 *   - CommandCode: 10
 *   - Contains Bit Fields:
 *     + @ref LTC4015_BSR_HI_ALERT_LIMIT_BF "BSR_HI_ALERT_LIMIT_BF" : Battery series resistance high alert limit as reported in BSR
*/

//!@{
#define LTC4015_BSR_HI_ALERT_LIMIT_SUBADDR 10
#define LTC4015_BSR_HI_ALERT_LIMIT (0 << 12 | (16 - 1) << 8 | LTC4015_BSR_HI_ALERT_LIMIT_SUBADDR)
//!@}
/*! @defgroup LTC4015_BSR_HI_ALERT_LIMIT_BF BSR_HI_ALERT_LIMIT_BF
 *  @ingroup LTC4015_register_map
 *  @brief BSR_HI_ALERT_LIMIT_BF Bit Field
 *
 *  Battery series resistance high alert limit as reported in BSR
 *   - Register: @ref LTC4015_BSR_HI_ALERT_LIMIT "BSR_HI_ALERT_LIMIT"
 *   - CommandCode: 10
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_BSR_HI_ALERT_LIMIT_BF_SUBADDR LTC4015_BSR_HI_ALERT_LIMIT_SUBADDR //!< @ref LTC4015_BSR_HI_ALERT_LIMIT_BF "BSR_HI_ALERT_LIMIT_BF"
#define LTC4015_BSR_HI_ALERT_LIMIT_BF_SIZE 16
#define LTC4015_BSR_HI_ALERT_LIMIT_BF_OFFSET 0
#define LTC4015_BSR_HI_ALERT_LIMIT_BF_MASK 0xFFFF
#define LTC4015_BSR_HI_ALERT_LIMIT_BF (LTC4015_BSR_HI_ALERT_LIMIT_BF_OFFSET << 12 | (LTC4015_BSR_HI_ALERT_LIMIT_BF_SIZE - 1) << 8 | LTC4015_BSR_HI_ALERT_LIMIT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_NTC_RATIO_HI_ALERT_LIMIT NTC_RATIO_HI_ALERT_LIMIT
 *  @ingroup LTC4015_register_map
 *  @brief NTC_RATIO_HI_ALERT_LIMIT Register
 *
 * |                        15:0 |
 * |:---------------------------:|
 * | NTC_RATIO_HI_ALERT_LIMIT_BF |
 *
 *   - CommandCode: 11
 *   - Contains Bit Fields:
 *     + @ref LTC4015_NTC_RATIO_HI_ALERT_LIMIT_BF "NTC_RATIO_HI_ALERT_LIMIT_BF" : Thermistor ratio high (cold battery) alert limit as reported in NTC_RATIO
*/

//!@{
#define LTC4015_NTC_RATIO_HI_ALERT_LIMIT_SUBADDR 11
#define LTC4015_NTC_RATIO_HI_ALERT_LIMIT (0 << 12 | (16 - 1) << 8 | LTC4015_NTC_RATIO_HI_ALERT_LIMIT_SUBADDR)
//!@}
/*! @defgroup LTC4015_NTC_RATIO_HI_ALERT_LIMIT_BF NTC_RATIO_HI_ALERT_LIMIT_BF
 *  @ingroup LTC4015_register_map
 *  @brief NTC_RATIO_HI_ALERT_LIMIT_BF Bit Field
 *
 *  Thermistor ratio high (cold battery) alert limit as reported in NTC_RATIO
 *   - Register: @ref LTC4015_NTC_RATIO_HI_ALERT_LIMIT "NTC_RATIO_HI_ALERT_LIMIT"
 *   - CommandCode: 11
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_NTC_RATIO_HI_ALERT_LIMIT_BF_SUBADDR LTC4015_NTC_RATIO_HI_ALERT_LIMIT_SUBADDR //!< @ref LTC4015_NTC_RATIO_HI_ALERT_LIMIT_BF "NTC_RATIO_HI_ALERT_LIMIT_BF"
#define LTC4015_NTC_RATIO_HI_ALERT_LIMIT_BF_SIZE 16
#define LTC4015_NTC_RATIO_HI_ALERT_LIMIT_BF_OFFSET 0
#define LTC4015_NTC_RATIO_HI_ALERT_LIMIT_BF_MASK 0xFFFF
#define LTC4015_NTC_RATIO_HI_ALERT_LIMIT_BF (LTC4015_NTC_RATIO_HI_ALERT_LIMIT_BF_OFFSET << 12 | (LTC4015_NTC_RATIO_HI_ALERT_LIMIT_BF_SIZE - 1) << 8 | LTC4015_NTC_RATIO_HI_ALERT_LIMIT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_NTC_RATIO_LO_ALERT_LIMIT NTC_RATIO_LO_ALERT_LIMIT
 *  @ingroup LTC4015_register_map
 *  @brief NTC_RATIO_LO_ALERT_LIMIT Register
 *
 * |                        15:0 |
 * |:---------------------------:|
 * | NTC_RATIO_LO_ALERT_LIMIT_BF |
 *
 *   - CommandCode: 12
 *   - Contains Bit Fields:
 *     + @ref LTC4015_NTC_RATIO_LO_ALERT_LIMIT_BF "NTC_RATIO_LO_ALERT_LIMIT_BF" : Thermistor ratio low (hot battery) alert limit as reported in NTC_RATIO
*/

//!@{
#define LTC4015_NTC_RATIO_LO_ALERT_LIMIT_SUBADDR 12
#define LTC4015_NTC_RATIO_LO_ALERT_LIMIT (0 << 12 | (16 - 1) << 8 | LTC4015_NTC_RATIO_LO_ALERT_LIMIT_SUBADDR)
//!@}
/*! @defgroup LTC4015_NTC_RATIO_LO_ALERT_LIMIT_BF NTC_RATIO_LO_ALERT_LIMIT_BF
 *  @ingroup LTC4015_register_map
 *  @brief NTC_RATIO_LO_ALERT_LIMIT_BF Bit Field
 *
 *  Thermistor ratio low (hot battery) alert limit as reported in NTC_RATIO
 *   - Register: @ref LTC4015_NTC_RATIO_LO_ALERT_LIMIT "NTC_RATIO_LO_ALERT_LIMIT"
 *   - CommandCode: 12
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_NTC_RATIO_LO_ALERT_LIMIT_BF_SUBADDR LTC4015_NTC_RATIO_LO_ALERT_LIMIT_SUBADDR //!< @ref LTC4015_NTC_RATIO_LO_ALERT_LIMIT_BF "NTC_RATIO_LO_ALERT_LIMIT_BF"
#define LTC4015_NTC_RATIO_LO_ALERT_LIMIT_BF_SIZE 16
#define LTC4015_NTC_RATIO_LO_ALERT_LIMIT_BF_OFFSET 0
#define LTC4015_NTC_RATIO_LO_ALERT_LIMIT_BF_MASK 0xFFFF
#define LTC4015_NTC_RATIO_LO_ALERT_LIMIT_BF (LTC4015_NTC_RATIO_LO_ALERT_LIMIT_BF_OFFSET << 12 | (LTC4015_NTC_RATIO_LO_ALERT_LIMIT_BF_SIZE - 1) << 8 | LTC4015_NTC_RATIO_LO_ALERT_LIMIT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_EN_LIMIT_ALERTS EN_LIMIT_ALERTS
 *  @ingroup LTC4015_register_map
 *  @brief EN_LIMIT_ALERTS Register
 *
 * |                         15 |  14 |                     13 |                      12 |                  11 |                  10 |                  9 |                  8 |                   7 |                   6 |                  5 |                   4 |                       3 |                  2 |                        1 |                        0 |
 * |:--------------------------:|:---:|:----------------------:|:-----------------------:|:-------------------:|:-------------------:|:------------------:|:------------------:|:-------------------:|:-------------------:|:------------------:|:-------------------:|:-----------------------:|:------------------:|:------------------------:|:------------------------:|
 * | EN_MEAS_SYS_VALID_ALERT_BF | n/a | EN_QCOUNT_LOW_ALERT_BF | EN_QCOUNT_HIGH_ALERT_BF | EN_VBAT_LO_ALERT_BF | EN_VBAT_HI_ALERT_BF | EN_VIN_LO_ALERT_BF | EN_VIN_HI_ALERT_BF | EN_VSYS_LO_ALERT_BF | EN_VSYS_HI_ALERT_BF | EN_IIN_HI_ALERT_BF | EN_IBAT_LO_ALERT_BF | EN_DIE_TEMP_HI_ALERT_BF | EN_BSR_HI_ALERT_BF | EN_NTC_RATIO_HI_ALERT_BF | EN_NTC_RATIO_LO_ALERT_BF |
 *
 * Enable limit monitoring and alert notification via SMBALERT
 *   - CommandCode: 13
 *   - Contains Bit Fields:
 *     + @ref LTC4015_EN_MEAS_SYS_VALID_ALERT_BF "EN_MEAS_SYS_VALID_ALERT_BF" : Enable meas_sys_valid_alert
 *     + @ref LTC4015_EN_QCOUNT_LOW_ALERT_BF "EN_QCOUNT_LOW_ALERT_BF" : Enable coulomb counter value low alert
 *     + @ref LTC4015_EN_QCOUNT_HIGH_ALERT_BF "EN_QCOUNT_HIGH_ALERT_BF" : Enable coulomb counter value high alert
 *     + @ref LTC4015_EN_VBAT_LO_ALERT_BF "EN_VBAT_LO_ALERT_BF" : Enable battery undervoltage alert
 *     + @ref LTC4015_EN_VBAT_HI_ALERT_BF "EN_VBAT_HI_ALERT_BF" : Enable battery overvoltage alert
 *     + @ref LTC4015_EN_VIN_LO_ALERT_BF "EN_VIN_LO_ALERT_BF" : Enable input undervoltage alert
 *     + @ref LTC4015_EN_VIN_HI_ALERT_BF "EN_VIN_HI_ALERT_BF" : Enable input overvoltage alert
 *     + @ref LTC4015_EN_VSYS_LO_ALERT_BF "EN_VSYS_LO_ALERT_BF" : Enable output undervoltage alert
 *     + @ref LTC4015_EN_VSYS_HI_ALERT_BF "EN_VSYS_HI_ALERT_BF" : Enable output overvoltage alert
 *     + @ref LTC4015_EN_IIN_HI_ALERT_BF "EN_IIN_HI_ALERT_BF" : Enable input overcurrent alert
 *     + @ref LTC4015_EN_IBAT_LO_ALERT_BF "EN_IBAT_LO_ALERT_BF" : Enable battery current low alert
 *     + @ref LTC4015_EN_DIE_TEMP_HI_ALERT_BF "EN_DIE_TEMP_HI_ALERT_BF" : Enable die temperature high alert
 *     + @ref LTC4015_EN_BSR_HI_ALERT_BF "EN_BSR_HI_ALERT_BF" : Enable battery series resistance high alert
 *     + @ref LTC4015_EN_NTC_RATIO_HI_ALERT_BF "EN_NTC_RATIO_HI_ALERT_BF" : Enable thermistor ratio high (cold battery) alert
 *     + @ref LTC4015_EN_NTC_RATIO_LO_ALERT_BF "EN_NTC_RATIO_LO_ALERT_BF" : Enable thermistor ratio low (hot battery) alert
*/

//!@{
#define LTC4015_EN_LIMIT_ALERTS_SUBADDR 13
#define LTC4015_EN_LIMIT_ALERTS (0 << 12 | (16 - 1) << 8 | LTC4015_EN_LIMIT_ALERTS_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_MEAS_SYS_VALID_ALERT_BF EN_MEAS_SYS_VALID_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_MEAS_SYS_VALID_ALERT_BF Bit Field
 *
 *  Enable meas_sys_valid_alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 15
 *   - MSB: 15
 *   - MASK: 0x8000
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_MEAS_SYS_VALID_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_MEAS_SYS_VALID_ALERT_BF "EN_MEAS_SYS_VALID_ALERT_BF"
#define LTC4015_EN_MEAS_SYS_VALID_ALERT_BF_SIZE 1
#define LTC4015_EN_MEAS_SYS_VALID_ALERT_BF_OFFSET 15
#define LTC4015_EN_MEAS_SYS_VALID_ALERT_BF_MASK 0x8000
#define LTC4015_EN_MEAS_SYS_VALID_ALERT_BF (LTC4015_EN_MEAS_SYS_VALID_ALERT_BF_OFFSET << 12 | (LTC4015_EN_MEAS_SYS_VALID_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_MEAS_SYS_VALID_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_QCOUNT_LOW_ALERT_BF EN_QCOUNT_LOW_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_QCOUNT_LOW_ALERT_BF Bit Field
 *
 *  Enable coulomb counter value low alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 13
 *   - MSB: 13
 *   - MASK: 0x2000
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_QCOUNT_LOW_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_QCOUNT_LOW_ALERT_BF "EN_QCOUNT_LOW_ALERT_BF"
#define LTC4015_EN_QCOUNT_LOW_ALERT_BF_SIZE 1
#define LTC4015_EN_QCOUNT_LOW_ALERT_BF_OFFSET 13
#define LTC4015_EN_QCOUNT_LOW_ALERT_BF_MASK 0x2000
#define LTC4015_EN_QCOUNT_LOW_ALERT_BF (LTC4015_EN_QCOUNT_LOW_ALERT_BF_OFFSET << 12 | (LTC4015_EN_QCOUNT_LOW_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_QCOUNT_LOW_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_QCOUNT_HIGH_ALERT_BF EN_QCOUNT_HIGH_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_QCOUNT_HIGH_ALERT_BF Bit Field
 *
 *  Enable coulomb counter value high alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 12
 *   - MSB: 12
 *   - MASK: 0x1000
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_QCOUNT_HIGH_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_QCOUNT_HIGH_ALERT_BF "EN_QCOUNT_HIGH_ALERT_BF"
#define LTC4015_EN_QCOUNT_HIGH_ALERT_BF_SIZE 1
#define LTC4015_EN_QCOUNT_HIGH_ALERT_BF_OFFSET 12
#define LTC4015_EN_QCOUNT_HIGH_ALERT_BF_MASK 0x1000
#define LTC4015_EN_QCOUNT_HIGH_ALERT_BF (LTC4015_EN_QCOUNT_HIGH_ALERT_BF_OFFSET << 12 | (LTC4015_EN_QCOUNT_HIGH_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_QCOUNT_HIGH_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_VBAT_LO_ALERT_BF EN_VBAT_LO_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_VBAT_LO_ALERT_BF Bit Field
 *
 *  Enable battery undervoltage alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 11
 *   - MSB: 11
 *   - MASK: 0x0800
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_VBAT_LO_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_VBAT_LO_ALERT_BF "EN_VBAT_LO_ALERT_BF"
#define LTC4015_EN_VBAT_LO_ALERT_BF_SIZE 1
#define LTC4015_EN_VBAT_LO_ALERT_BF_OFFSET 11
#define LTC4015_EN_VBAT_LO_ALERT_BF_MASK 0x0800
#define LTC4015_EN_VBAT_LO_ALERT_BF (LTC4015_EN_VBAT_LO_ALERT_BF_OFFSET << 12 | (LTC4015_EN_VBAT_LO_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_VBAT_LO_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_VBAT_HI_ALERT_BF EN_VBAT_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_VBAT_HI_ALERT_BF Bit Field
 *
 *  Enable battery overvoltage alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 10
 *   - MSB: 10
 *   - MASK: 0x0400
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_VBAT_HI_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_VBAT_HI_ALERT_BF "EN_VBAT_HI_ALERT_BF"
#define LTC4015_EN_VBAT_HI_ALERT_BF_SIZE 1
#define LTC4015_EN_VBAT_HI_ALERT_BF_OFFSET 10
#define LTC4015_EN_VBAT_HI_ALERT_BF_MASK 0x0400
#define LTC4015_EN_VBAT_HI_ALERT_BF (LTC4015_EN_VBAT_HI_ALERT_BF_OFFSET << 12 | (LTC4015_EN_VBAT_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_VBAT_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_VIN_LO_ALERT_BF EN_VIN_LO_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_VIN_LO_ALERT_BF Bit Field
 *
 *  Enable input undervoltage alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 9
 *   - MSB: 9
 *   - MASK: 0x0200
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_VIN_LO_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_VIN_LO_ALERT_BF "EN_VIN_LO_ALERT_BF"
#define LTC4015_EN_VIN_LO_ALERT_BF_SIZE 1
#define LTC4015_EN_VIN_LO_ALERT_BF_OFFSET 9
#define LTC4015_EN_VIN_LO_ALERT_BF_MASK 0x0200
#define LTC4015_EN_VIN_LO_ALERT_BF (LTC4015_EN_VIN_LO_ALERT_BF_OFFSET << 12 | (LTC4015_EN_VIN_LO_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_VIN_LO_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_VIN_HI_ALERT_BF EN_VIN_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_VIN_HI_ALERT_BF Bit Field
 *
 *  Enable input overvoltage alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 8
 *   - MSB: 8
 *   - MASK: 0x0100
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_VIN_HI_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_VIN_HI_ALERT_BF "EN_VIN_HI_ALERT_BF"
#define LTC4015_EN_VIN_HI_ALERT_BF_SIZE 1
#define LTC4015_EN_VIN_HI_ALERT_BF_OFFSET 8
#define LTC4015_EN_VIN_HI_ALERT_BF_MASK 0x0100
#define LTC4015_EN_VIN_HI_ALERT_BF (LTC4015_EN_VIN_HI_ALERT_BF_OFFSET << 12 | (LTC4015_EN_VIN_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_VIN_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_VSYS_LO_ALERT_BF EN_VSYS_LO_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_VSYS_LO_ALERT_BF Bit Field
 *
 *  Enable output undervoltage alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 7
 *   - MSB: 7
 *   - MASK: 0x0080
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_VSYS_LO_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_VSYS_LO_ALERT_BF "EN_VSYS_LO_ALERT_BF"
#define LTC4015_EN_VSYS_LO_ALERT_BF_SIZE 1
#define LTC4015_EN_VSYS_LO_ALERT_BF_OFFSET 7
#define LTC4015_EN_VSYS_LO_ALERT_BF_MASK 0x0080
#define LTC4015_EN_VSYS_LO_ALERT_BF (LTC4015_EN_VSYS_LO_ALERT_BF_OFFSET << 12 | (LTC4015_EN_VSYS_LO_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_VSYS_LO_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_VSYS_HI_ALERT_BF EN_VSYS_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_VSYS_HI_ALERT_BF Bit Field
 *
 *  Enable output overvoltage alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 6
 *   - MSB: 6
 *   - MASK: 0x0040
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_VSYS_HI_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_VSYS_HI_ALERT_BF "EN_VSYS_HI_ALERT_BF"
#define LTC4015_EN_VSYS_HI_ALERT_BF_SIZE 1
#define LTC4015_EN_VSYS_HI_ALERT_BF_OFFSET 6
#define LTC4015_EN_VSYS_HI_ALERT_BF_MASK 0x0040
#define LTC4015_EN_VSYS_HI_ALERT_BF (LTC4015_EN_VSYS_HI_ALERT_BF_OFFSET << 12 | (LTC4015_EN_VSYS_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_VSYS_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_IIN_HI_ALERT_BF EN_IIN_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_IIN_HI_ALERT_BF Bit Field
 *
 *  Enable input overcurrent alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 5
 *   - MSB: 5
 *   - MASK: 0x0020
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_IIN_HI_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_IIN_HI_ALERT_BF "EN_IIN_HI_ALERT_BF"
#define LTC4015_EN_IIN_HI_ALERT_BF_SIZE 1
#define LTC4015_EN_IIN_HI_ALERT_BF_OFFSET 5
#define LTC4015_EN_IIN_HI_ALERT_BF_MASK 0x0020
#define LTC4015_EN_IIN_HI_ALERT_BF (LTC4015_EN_IIN_HI_ALERT_BF_OFFSET << 12 | (LTC4015_EN_IIN_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_IIN_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_IBAT_LO_ALERT_BF EN_IBAT_LO_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_IBAT_LO_ALERT_BF Bit Field
 *
 *  Enable battery current low alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 4
 *   - MSB: 4
 *   - MASK: 0x0010
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_IBAT_LO_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_IBAT_LO_ALERT_BF "EN_IBAT_LO_ALERT_BF"
#define LTC4015_EN_IBAT_LO_ALERT_BF_SIZE 1
#define LTC4015_EN_IBAT_LO_ALERT_BF_OFFSET 4
#define LTC4015_EN_IBAT_LO_ALERT_BF_MASK 0x0010
#define LTC4015_EN_IBAT_LO_ALERT_BF (LTC4015_EN_IBAT_LO_ALERT_BF_OFFSET << 12 | (LTC4015_EN_IBAT_LO_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_IBAT_LO_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_DIE_TEMP_HI_ALERT_BF EN_DIE_TEMP_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_DIE_TEMP_HI_ALERT_BF Bit Field
 *
 *  Enable die temperature high alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 3
 *   - MSB: 3
 *   - MASK: 0x0008
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_DIE_TEMP_HI_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_DIE_TEMP_HI_ALERT_BF "EN_DIE_TEMP_HI_ALERT_BF"
#define LTC4015_EN_DIE_TEMP_HI_ALERT_BF_SIZE 1
#define LTC4015_EN_DIE_TEMP_HI_ALERT_BF_OFFSET 3
#define LTC4015_EN_DIE_TEMP_HI_ALERT_BF_MASK 0x0008
#define LTC4015_EN_DIE_TEMP_HI_ALERT_BF (LTC4015_EN_DIE_TEMP_HI_ALERT_BF_OFFSET << 12 | (LTC4015_EN_DIE_TEMP_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_DIE_TEMP_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_BSR_HI_ALERT_BF EN_BSR_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_BSR_HI_ALERT_BF Bit Field
 *
 *  Enable battery series resistance high alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 2
 *   - MSB: 2
 *   - MASK: 0x0004
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_BSR_HI_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_BSR_HI_ALERT_BF "EN_BSR_HI_ALERT_BF"
#define LTC4015_EN_BSR_HI_ALERT_BF_SIZE 1
#define LTC4015_EN_BSR_HI_ALERT_BF_OFFSET 2
#define LTC4015_EN_BSR_HI_ALERT_BF_MASK 0x0004
#define LTC4015_EN_BSR_HI_ALERT_BF (LTC4015_EN_BSR_HI_ALERT_BF_OFFSET << 12 | (LTC4015_EN_BSR_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_BSR_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_NTC_RATIO_HI_ALERT_BF EN_NTC_RATIO_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_NTC_RATIO_HI_ALERT_BF Bit Field
 *
 *  Enable thermistor ratio high (cold battery) alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 1
 *   - MSB: 1
 *   - MASK: 0x0002
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_NTC_RATIO_HI_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_NTC_RATIO_HI_ALERT_BF "EN_NTC_RATIO_HI_ALERT_BF"
#define LTC4015_EN_NTC_RATIO_HI_ALERT_BF_SIZE 1
#define LTC4015_EN_NTC_RATIO_HI_ALERT_BF_OFFSET 1
#define LTC4015_EN_NTC_RATIO_HI_ALERT_BF_MASK 0x0002
#define LTC4015_EN_NTC_RATIO_HI_ALERT_BF (LTC4015_EN_NTC_RATIO_HI_ALERT_BF_OFFSET << 12 | (LTC4015_EN_NTC_RATIO_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_NTC_RATIO_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_NTC_RATIO_LO_ALERT_BF EN_NTC_RATIO_LO_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_NTC_RATIO_LO_ALERT_BF Bit Field
 *
 *  Enable thermistor ratio low (hot battery) alert
 *   - Register: @ref LTC4015_EN_LIMIT_ALERTS "EN_LIMIT_ALERTS"
 *   - CommandCode: 13
 *   - Size: 1
 *   - Offset: 0
 *   - MSB: 0
 *   - MASK: 0x0001
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_NTC_RATIO_LO_ALERT_BF_SUBADDR LTC4015_EN_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_EN_NTC_RATIO_LO_ALERT_BF "EN_NTC_RATIO_LO_ALERT_BF"
#define LTC4015_EN_NTC_RATIO_LO_ALERT_BF_SIZE 1
#define LTC4015_EN_NTC_RATIO_LO_ALERT_BF_OFFSET 0
#define LTC4015_EN_NTC_RATIO_LO_ALERT_BF_MASK 0x0001
#define LTC4015_EN_NTC_RATIO_LO_ALERT_BF (LTC4015_EN_NTC_RATIO_LO_ALERT_BF_OFFSET << 12 | (LTC4015_EN_NTC_RATIO_LO_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_NTC_RATIO_LO_ALERT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_EN_CHARGER_STATE_ALERTS EN_CHARGER_STATE_ALERTS
 *  @ingroup LTC4015_register_map
 *  @brief EN_CHARGER_STATE_ALERTS Register
 *
 * | 15:11 |                          10 |                         9 |                             8 |                     7 |                        6 |                     5 |                      4 |                         3 |                                 2 |                             1 |                           0 |
 * |:-----:|:---------------------------:|:-------------------------:|:-----------------------------:|:---------------------:|:------------------------:|:---------------------:|:----------------------:|:-------------------------:|:---------------------------------:|:-----------------------------:|:---------------------------:|
 * |   n/a | EN_EQUALIZE_CHARGE_ALERT_BF | EN_ABSORB_CHARGE_ALERT_BF | EN_CHARGER_SUSPENDED_ALERT_BF | EN_PRECHARGE_ALERT_BF | EN_CC_CV_CHARGE_ALERT_BF | EN_NTC_PAUSE_ALERT_BF | EN_TIMER_TERM_ALERT_BF | EN_C_OVER_X_TERM_ALERT_BF | EN_MAX_CHARGE_TIME_FAULT_ALERT_BF | EN_BAT_MISSING_FAULT_ALERT_BF | EN_BAT_SHORT_FAULT_ALERT_BF |
 *
 * Enable charger state notification via SMBALERT
 *   - CommandCode: 14
 *   - Contains Bit Fields:
 *     + @ref LTC4015_EN_EQUALIZE_CHARGE_ALERT_BF "EN_EQUALIZE_CHARGE_ALERT_BF" : Enable alert for lead-acid equalize charge state
 *     + @ref LTC4015_EN_ABSORB_CHARGE_ALERT_BF "EN_ABSORB_CHARGE_ALERT_BF" : Enable alert for absorb charge state
 *     + @ref LTC4015_EN_CHARGER_SUSPENDED_ALERT_BF "EN_CHARGER_SUSPENDED_ALERT_BF" : Enable alert for charger suspended state
 *     + @ref LTC4015_EN_PRECHARGE_ALERT_BF "EN_PRECHARGE_ALERT_BF" : Enable alert for precondition charge state
 *     + @ref LTC4015_EN_CC_CV_CHARGE_ALERT_BF "EN_CC_CV_CHARGE_ALERT_BF" : Enable alert for constant current constant voltage state
 *     + @ref LTC4015_EN_NTC_PAUSE_ALERT_BF "EN_NTC_PAUSE_ALERT_BF" : Enable alert for thermistor pause state
 *     + @ref LTC4015_EN_TIMER_TERM_ALERT_BF "EN_TIMER_TERM_ALERT_BF" : Enable alert for timer termination state
 *     + @ref LTC4015_EN_C_OVER_X_TERM_ALERT_BF "EN_C_OVER_X_TERM_ALERT_BF" : Enable alert for C/x termination state
 *     + @ref LTC4015_EN_MAX_CHARGE_TIME_FAULT_ALERT_BF "EN_MAX_CHARGE_TIME_FAULT_ALERT_BF" : Enable max_charge_time_fault alert
 *     + @ref LTC4015_EN_BAT_MISSING_FAULT_ALERT_BF "EN_BAT_MISSING_FAULT_ALERT_BF" : Enable alert for missing battery fault state
 *     + @ref LTC4015_EN_BAT_SHORT_FAULT_ALERT_BF "EN_BAT_SHORT_FAULT_ALERT_BF" : Enable alert for shorted battery fault state
*/

//!@{
#define LTC4015_EN_CHARGER_STATE_ALERTS_SUBADDR 14
#define LTC4015_EN_CHARGER_STATE_ALERTS (0 << 12 | (16 - 1) << 8 | LTC4015_EN_CHARGER_STATE_ALERTS_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_EQUALIZE_CHARGE_ALERT_BF EN_EQUALIZE_CHARGE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_EQUALIZE_CHARGE_ALERT_BF Bit Field
 *
 *  Enable alert for lead-acid equalize charge state
 *   - Register: @ref LTC4015_EN_CHARGER_STATE_ALERTS "EN_CHARGER_STATE_ALERTS"
 *   - CommandCode: 14
 *   - Size: 1
 *   - Offset: 10
 *   - MSB: 10
 *   - MASK: 0x0400
 *   - Access: R/W
 *   - Default: n/a
 */
//!@{
#define LTC4015_EN_EQUALIZE_CHARGE_ALERT_BF_SUBADDR LTC4015_EN_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_EN_EQUALIZE_CHARGE_ALERT_BF "EN_EQUALIZE_CHARGE_ALERT_BF"
#define LTC4015_EN_EQUALIZE_CHARGE_ALERT_BF_SIZE 1
#define LTC4015_EN_EQUALIZE_CHARGE_ALERT_BF_OFFSET 10
#define LTC4015_EN_EQUALIZE_CHARGE_ALERT_BF_MASK 0x0400
#define LTC4015_EN_EQUALIZE_CHARGE_ALERT_BF (LTC4015_EN_EQUALIZE_CHARGE_ALERT_BF_OFFSET << 12 | (LTC4015_EN_EQUALIZE_CHARGE_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_EQUALIZE_CHARGE_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_ABSORB_CHARGE_ALERT_BF EN_ABSORB_CHARGE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_ABSORB_CHARGE_ALERT_BF Bit Field
 *
 *  Enable alert for absorb charge state
 *   - Register: @ref LTC4015_EN_CHARGER_STATE_ALERTS "EN_CHARGER_STATE_ALERTS"
 *   - CommandCode: 14
 *   - Size: 1
 *   - Offset: 9
 *   - MSB: 9
 *   - MASK: 0x0200
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_ABSORB_CHARGE_ALERT_BF_SUBADDR LTC4015_EN_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_EN_ABSORB_CHARGE_ALERT_BF "EN_ABSORB_CHARGE_ALERT_BF"
#define LTC4015_EN_ABSORB_CHARGE_ALERT_BF_SIZE 1
#define LTC4015_EN_ABSORB_CHARGE_ALERT_BF_OFFSET 9
#define LTC4015_EN_ABSORB_CHARGE_ALERT_BF_MASK 0x0200
#define LTC4015_EN_ABSORB_CHARGE_ALERT_BF (LTC4015_EN_ABSORB_CHARGE_ALERT_BF_OFFSET << 12 | (LTC4015_EN_ABSORB_CHARGE_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_ABSORB_CHARGE_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_CHARGER_SUSPENDED_ALERT_BF EN_CHARGER_SUSPENDED_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_CHARGER_SUSPENDED_ALERT_BF Bit Field
 *
 *  Enable alert for charger suspended state
 *   - Register: @ref LTC4015_EN_CHARGER_STATE_ALERTS "EN_CHARGER_STATE_ALERTS"
 *   - CommandCode: 14
 *   - Size: 1
 *   - Offset: 8
 *   - MSB: 8
 *   - MASK: 0x0100
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_CHARGER_SUSPENDED_ALERT_BF_SUBADDR LTC4015_EN_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_EN_CHARGER_SUSPENDED_ALERT_BF "EN_CHARGER_SUSPENDED_ALERT_BF"
#define LTC4015_EN_CHARGER_SUSPENDED_ALERT_BF_SIZE 1
#define LTC4015_EN_CHARGER_SUSPENDED_ALERT_BF_OFFSET 8
#define LTC4015_EN_CHARGER_SUSPENDED_ALERT_BF_MASK 0x0100
#define LTC4015_EN_CHARGER_SUSPENDED_ALERT_BF (LTC4015_EN_CHARGER_SUSPENDED_ALERT_BF_OFFSET << 12 | (LTC4015_EN_CHARGER_SUSPENDED_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_CHARGER_SUSPENDED_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_PRECHARGE_ALERT_BF EN_PRECHARGE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_PRECHARGE_ALERT_BF Bit Field
 *
 *  Enable alert for precondition charge state
 *   - Register: @ref LTC4015_EN_CHARGER_STATE_ALERTS "EN_CHARGER_STATE_ALERTS"
 *   - CommandCode: 14
 *   - Size: 1
 *   - Offset: 7
 *   - MSB: 7
 *   - MASK: 0x0080
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_PRECHARGE_ALERT_BF_SUBADDR LTC4015_EN_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_EN_PRECHARGE_ALERT_BF "EN_PRECHARGE_ALERT_BF"
#define LTC4015_EN_PRECHARGE_ALERT_BF_SIZE 1
#define LTC4015_EN_PRECHARGE_ALERT_BF_OFFSET 7
#define LTC4015_EN_PRECHARGE_ALERT_BF_MASK 0x0080
#define LTC4015_EN_PRECHARGE_ALERT_BF (LTC4015_EN_PRECHARGE_ALERT_BF_OFFSET << 12 | (LTC4015_EN_PRECHARGE_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_PRECHARGE_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_CC_CV_CHARGE_ALERT_BF EN_CC_CV_CHARGE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_CC_CV_CHARGE_ALERT_BF Bit Field
 *
 *  Enable alert for constant current constant voltage state
 *   - Register: @ref LTC4015_EN_CHARGER_STATE_ALERTS "EN_CHARGER_STATE_ALERTS"
 *   - CommandCode: 14
 *   - Size: 1
 *   - Offset: 6
 *   - MSB: 6
 *   - MASK: 0x0040
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_CC_CV_CHARGE_ALERT_BF_SUBADDR LTC4015_EN_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_EN_CC_CV_CHARGE_ALERT_BF "EN_CC_CV_CHARGE_ALERT_BF"
#define LTC4015_EN_CC_CV_CHARGE_ALERT_BF_SIZE 1
#define LTC4015_EN_CC_CV_CHARGE_ALERT_BF_OFFSET 6
#define LTC4015_EN_CC_CV_CHARGE_ALERT_BF_MASK 0x0040
#define LTC4015_EN_CC_CV_CHARGE_ALERT_BF (LTC4015_EN_CC_CV_CHARGE_ALERT_BF_OFFSET << 12 | (LTC4015_EN_CC_CV_CHARGE_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_CC_CV_CHARGE_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_NTC_PAUSE_ALERT_BF EN_NTC_PAUSE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_NTC_PAUSE_ALERT_BF Bit Field
 *
 *  Enable alert for thermistor pause state
 *   - Register: @ref LTC4015_EN_CHARGER_STATE_ALERTS "EN_CHARGER_STATE_ALERTS"
 *   - CommandCode: 14
 *   - Size: 1
 *   - Offset: 5
 *   - MSB: 5
 *   - MASK: 0x0020
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_NTC_PAUSE_ALERT_BF_SUBADDR LTC4015_EN_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_EN_NTC_PAUSE_ALERT_BF "EN_NTC_PAUSE_ALERT_BF"
#define LTC4015_EN_NTC_PAUSE_ALERT_BF_SIZE 1
#define LTC4015_EN_NTC_PAUSE_ALERT_BF_OFFSET 5
#define LTC4015_EN_NTC_PAUSE_ALERT_BF_MASK 0x0020
#define LTC4015_EN_NTC_PAUSE_ALERT_BF (LTC4015_EN_NTC_PAUSE_ALERT_BF_OFFSET << 12 | (LTC4015_EN_NTC_PAUSE_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_NTC_PAUSE_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_TIMER_TERM_ALERT_BF EN_TIMER_TERM_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_TIMER_TERM_ALERT_BF Bit Field
 *
 *  Enable alert for timer termination state
 *   - Register: @ref LTC4015_EN_CHARGER_STATE_ALERTS "EN_CHARGER_STATE_ALERTS"
 *   - CommandCode: 14
 *   - Size: 1
 *   - Offset: 4
 *   - MSB: 4
 *   - MASK: 0x0010
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_TIMER_TERM_ALERT_BF_SUBADDR LTC4015_EN_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_EN_TIMER_TERM_ALERT_BF "EN_TIMER_TERM_ALERT_BF"
#define LTC4015_EN_TIMER_TERM_ALERT_BF_SIZE 1
#define LTC4015_EN_TIMER_TERM_ALERT_BF_OFFSET 4
#define LTC4015_EN_TIMER_TERM_ALERT_BF_MASK 0x0010
#define LTC4015_EN_TIMER_TERM_ALERT_BF (LTC4015_EN_TIMER_TERM_ALERT_BF_OFFSET << 12 | (LTC4015_EN_TIMER_TERM_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_TIMER_TERM_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_C_OVER_X_TERM_ALERT_BF EN_C_OVER_X_TERM_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_C_OVER_X_TERM_ALERT_BF Bit Field
 *
 *  Enable alert for C/x termination state
 *   - Register: @ref LTC4015_EN_CHARGER_STATE_ALERTS "EN_CHARGER_STATE_ALERTS"
 *   - CommandCode: 14
 *   - Size: 1
 *   - Offset: 3
 *   - MSB: 3
 *   - MASK: 0x0008
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_C_OVER_X_TERM_ALERT_BF_SUBADDR LTC4015_EN_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_EN_C_OVER_X_TERM_ALERT_BF "EN_C_OVER_X_TERM_ALERT_BF"
#define LTC4015_EN_C_OVER_X_TERM_ALERT_BF_SIZE 1
#define LTC4015_EN_C_OVER_X_TERM_ALERT_BF_OFFSET 3
#define LTC4015_EN_C_OVER_X_TERM_ALERT_BF_MASK 0x0008
#define LTC4015_EN_C_OVER_X_TERM_ALERT_BF (LTC4015_EN_C_OVER_X_TERM_ALERT_BF_OFFSET << 12 | (LTC4015_EN_C_OVER_X_TERM_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_C_OVER_X_TERM_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_MAX_CHARGE_TIME_FAULT_ALERT_BF EN_MAX_CHARGE_TIME_FAULT_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_MAX_CHARGE_TIME_FAULT_ALERT_BF Bit Field
 *
 *  Enable max_charge_time_fault alert
 *   - Register: @ref LTC4015_EN_CHARGER_STATE_ALERTS "EN_CHARGER_STATE_ALERTS"
 *   - CommandCode: 14
 *   - Size: 1
 *   - Offset: 2
 *   - MSB: 2
 *   - MASK: 0x0004
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_MAX_CHARGE_TIME_FAULT_ALERT_BF_SUBADDR LTC4015_EN_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_EN_MAX_CHARGE_TIME_FAULT_ALERT_BF "EN_MAX_CHARGE_TIME_FAULT_ALERT_BF"
#define LTC4015_EN_MAX_CHARGE_TIME_FAULT_ALERT_BF_SIZE 1
#define LTC4015_EN_MAX_CHARGE_TIME_FAULT_ALERT_BF_OFFSET 2
#define LTC4015_EN_MAX_CHARGE_TIME_FAULT_ALERT_BF_MASK 0x0004
#define LTC4015_EN_MAX_CHARGE_TIME_FAULT_ALERT_BF (LTC4015_EN_MAX_CHARGE_TIME_FAULT_ALERT_BF_OFFSET << 12 | (LTC4015_EN_MAX_CHARGE_TIME_FAULT_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_MAX_CHARGE_TIME_FAULT_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_BAT_MISSING_FAULT_ALERT_BF EN_BAT_MISSING_FAULT_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_BAT_MISSING_FAULT_ALERT_BF Bit Field
 *
 *  Enable alert for missing battery fault state
 *   - Register: @ref LTC4015_EN_CHARGER_STATE_ALERTS "EN_CHARGER_STATE_ALERTS"
 *   - CommandCode: 14
 *   - Size: 1
 *   - Offset: 1
 *   - MSB: 1
 *   - MASK: 0x0002
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_BAT_MISSING_FAULT_ALERT_BF_SUBADDR LTC4015_EN_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_EN_BAT_MISSING_FAULT_ALERT_BF "EN_BAT_MISSING_FAULT_ALERT_BF"
#define LTC4015_EN_BAT_MISSING_FAULT_ALERT_BF_SIZE 1
#define LTC4015_EN_BAT_MISSING_FAULT_ALERT_BF_OFFSET 1
#define LTC4015_EN_BAT_MISSING_FAULT_ALERT_BF_MASK 0x0002
#define LTC4015_EN_BAT_MISSING_FAULT_ALERT_BF (LTC4015_EN_BAT_MISSING_FAULT_ALERT_BF_OFFSET << 12 | (LTC4015_EN_BAT_MISSING_FAULT_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_BAT_MISSING_FAULT_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_BAT_SHORT_FAULT_ALERT_BF EN_BAT_SHORT_FAULT_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_BAT_SHORT_FAULT_ALERT_BF Bit Field
 *
 *  Enable alert for shorted battery fault state
 *   - Register: @ref LTC4015_EN_CHARGER_STATE_ALERTS "EN_CHARGER_STATE_ALERTS"
 *   - CommandCode: 14
 *   - Size: 1
 *   - Offset: 0
 *   - MSB: 0
 *   - MASK: 0x0001
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_BAT_SHORT_FAULT_ALERT_BF_SUBADDR LTC4015_EN_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_EN_BAT_SHORT_FAULT_ALERT_BF "EN_BAT_SHORT_FAULT_ALERT_BF"
#define LTC4015_EN_BAT_SHORT_FAULT_ALERT_BF_SIZE 1
#define LTC4015_EN_BAT_SHORT_FAULT_ALERT_BF_OFFSET 0
#define LTC4015_EN_BAT_SHORT_FAULT_ALERT_BF_MASK 0x0001
#define LTC4015_EN_BAT_SHORT_FAULT_ALERT_BF (LTC4015_EN_BAT_SHORT_FAULT_ALERT_BF_OFFSET << 12 | (LTC4015_EN_BAT_SHORT_FAULT_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_BAT_SHORT_FAULT_ALERT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_EN_CHARGE_STATUS_ALERTS EN_CHARGE_STATUS_ALERTS
 *  @ingroup LTC4015_register_map
 *  @brief EN_CHARGE_STATUS_ALERTS Register
 *
 * | 15:4 |                           3 |                            2 |                            1 |                            0 |
 * |:----:|:---------------------------:|:----------------------------:|:----------------------------:|:----------------------------:|
 * |  n/a | EN_VIN_UVCL_ACTIVE_ALERT_BF | EN_IIN_LIMIT_ACTIVE_ALERT_BF | EN_CONSTANT_CURRENT_ALERT_BF | EN_CONSTANT_VOLTAGE_ALERT_BF |
 *
 * Enable charge status notification via SMBALERT
 *   - CommandCode: 15
 *   - Contains Bit Fields:
 *     + @ref LTC4015_EN_VIN_UVCL_ACTIVE_ALERT_BF "EN_VIN_UVCL_ACTIVE_ALERT_BF" : Enable alert for input undervoltage current limit active
 *     + @ref LTC4015_EN_IIN_LIMIT_ACTIVE_ALERT_BF "EN_IIN_LIMIT_ACTIVE_ALERT_BF" : Enable alert for input current limit active
 *     + @ref LTC4015_EN_CONSTANT_CURRENT_ALERT_BF "EN_CONSTANT_CURRENT_ALERT_BF" : Enable alert for constant current status
 *     + @ref LTC4015_EN_CONSTANT_VOLTAGE_ALERT_BF "EN_CONSTANT_VOLTAGE_ALERT_BF" : Enable alert for constant voltage status
*/

//!@{
#define LTC4015_EN_CHARGE_STATUS_ALERTS_SUBADDR 15
#define LTC4015_EN_CHARGE_STATUS_ALERTS (0 << 12 | (16 - 1) << 8 | LTC4015_EN_CHARGE_STATUS_ALERTS_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_VIN_UVCL_ACTIVE_ALERT_BF EN_VIN_UVCL_ACTIVE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_VIN_UVCL_ACTIVE_ALERT_BF Bit Field
 *
 *  Enable alert for input undervoltage current limit active
 *   - Register: @ref LTC4015_EN_CHARGE_STATUS_ALERTS "EN_CHARGE_STATUS_ALERTS"
 *   - CommandCode: 15
 *   - Size: 1
 *   - Offset: 3
 *   - MSB: 3
 *   - MASK: 0x0008
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_VIN_UVCL_ACTIVE_ALERT_BF_SUBADDR LTC4015_EN_CHARGE_STATUS_ALERTS_SUBADDR //!< @ref LTC4015_EN_VIN_UVCL_ACTIVE_ALERT_BF "EN_VIN_UVCL_ACTIVE_ALERT_BF"
#define LTC4015_EN_VIN_UVCL_ACTIVE_ALERT_BF_SIZE 1
#define LTC4015_EN_VIN_UVCL_ACTIVE_ALERT_BF_OFFSET 3
#define LTC4015_EN_VIN_UVCL_ACTIVE_ALERT_BF_MASK 0x0008
#define LTC4015_EN_VIN_UVCL_ACTIVE_ALERT_BF (LTC4015_EN_VIN_UVCL_ACTIVE_ALERT_BF_OFFSET << 12 | (LTC4015_EN_VIN_UVCL_ACTIVE_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_VIN_UVCL_ACTIVE_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_IIN_LIMIT_ACTIVE_ALERT_BF EN_IIN_LIMIT_ACTIVE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_IIN_LIMIT_ACTIVE_ALERT_BF Bit Field
 *
 *  Enable alert for input current limit active
 *   - Register: @ref LTC4015_EN_CHARGE_STATUS_ALERTS "EN_CHARGE_STATUS_ALERTS"
 *   - CommandCode: 15
 *   - Size: 1
 *   - Offset: 2
 *   - MSB: 2
 *   - MASK: 0x0004
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_IIN_LIMIT_ACTIVE_ALERT_BF_SUBADDR LTC4015_EN_CHARGE_STATUS_ALERTS_SUBADDR //!< @ref LTC4015_EN_IIN_LIMIT_ACTIVE_ALERT_BF "EN_IIN_LIMIT_ACTIVE_ALERT_BF"
#define LTC4015_EN_IIN_LIMIT_ACTIVE_ALERT_BF_SIZE 1
#define LTC4015_EN_IIN_LIMIT_ACTIVE_ALERT_BF_OFFSET 2
#define LTC4015_EN_IIN_LIMIT_ACTIVE_ALERT_BF_MASK 0x0004
#define LTC4015_EN_IIN_LIMIT_ACTIVE_ALERT_BF (LTC4015_EN_IIN_LIMIT_ACTIVE_ALERT_BF_OFFSET << 12 | (LTC4015_EN_IIN_LIMIT_ACTIVE_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_IIN_LIMIT_ACTIVE_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_CONSTANT_CURRENT_ALERT_BF EN_CONSTANT_CURRENT_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_CONSTANT_CURRENT_ALERT_BF Bit Field
 *
 *  Enable alert for constant current status
 *   - Register: @ref LTC4015_EN_CHARGE_STATUS_ALERTS "EN_CHARGE_STATUS_ALERTS"
 *   - CommandCode: 15
 *   - Size: 1
 *   - Offset: 1
 *   - MSB: 1
 *   - MASK: 0x0002
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_CONSTANT_CURRENT_ALERT_BF_SUBADDR LTC4015_EN_CHARGE_STATUS_ALERTS_SUBADDR //!< @ref LTC4015_EN_CONSTANT_CURRENT_ALERT_BF "EN_CONSTANT_CURRENT_ALERT_BF"
#define LTC4015_EN_CONSTANT_CURRENT_ALERT_BF_SIZE 1
#define LTC4015_EN_CONSTANT_CURRENT_ALERT_BF_OFFSET 1
#define LTC4015_EN_CONSTANT_CURRENT_ALERT_BF_MASK 0x0002
#define LTC4015_EN_CONSTANT_CURRENT_ALERT_BF (LTC4015_EN_CONSTANT_CURRENT_ALERT_BF_OFFSET << 12 | (LTC4015_EN_CONSTANT_CURRENT_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_CONSTANT_CURRENT_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_CONSTANT_VOLTAGE_ALERT_BF EN_CONSTANT_VOLTAGE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_CONSTANT_VOLTAGE_ALERT_BF Bit Field
 *
 *  Enable alert for constant voltage status
 *   - Register: @ref LTC4015_EN_CHARGE_STATUS_ALERTS "EN_CHARGE_STATUS_ALERTS"
 *   - CommandCode: 15
 *   - Size: 1
 *   - Offset: 0
 *   - MSB: 0
 *   - MASK: 0x0001
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_CONSTANT_VOLTAGE_ALERT_BF_SUBADDR LTC4015_EN_CHARGE_STATUS_ALERTS_SUBADDR //!< @ref LTC4015_EN_CONSTANT_VOLTAGE_ALERT_BF "EN_CONSTANT_VOLTAGE_ALERT_BF"
#define LTC4015_EN_CONSTANT_VOLTAGE_ALERT_BF_SIZE 1
#define LTC4015_EN_CONSTANT_VOLTAGE_ALERT_BF_OFFSET 0
#define LTC4015_EN_CONSTANT_VOLTAGE_ALERT_BF_MASK 0x0001
#define LTC4015_EN_CONSTANT_VOLTAGE_ALERT_BF (LTC4015_EN_CONSTANT_VOLTAGE_ALERT_BF_OFFSET << 12 | (LTC4015_EN_CONSTANT_VOLTAGE_ALERT_BF_SIZE - 1) << 8 | LTC4015_EN_CONSTANT_VOLTAGE_ALERT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_QCOUNT_LO_ALERT_LIMIT QCOUNT_LO_ALERT_LIMIT
 *  @ingroup LTC4015_register_map
 *  @brief QCOUNT_LO_ALERT_LIMIT Register
 *
 * |                     15:0 |
 * |:------------------------:|
 * | QCOUNT_LO_ALERT_LIMIT_BF |
 *
 *   - CommandCode: 16
 *   - Contains Bit Fields:
 *     + @ref LTC4015_QCOUNT_LO_ALERT_LIMIT_BF "QCOUNT_LO_ALERT_LIMIT_BF" : Coulomb counter low alert limit as reported in QCOUNT
*/

//!@{
#define LTC4015_QCOUNT_LO_ALERT_LIMIT_SUBADDR 16
#define LTC4015_QCOUNT_LO_ALERT_LIMIT (0 << 12 | (16 - 1) << 8 | LTC4015_QCOUNT_LO_ALERT_LIMIT_SUBADDR)
//!@}
/*! @defgroup LTC4015_QCOUNT_LO_ALERT_LIMIT_BF QCOUNT_LO_ALERT_LIMIT_BF
 *  @ingroup LTC4015_register_map
 *  @brief QCOUNT_LO_ALERT_LIMIT_BF Bit Field
 *
 *  Coulomb counter low alert limit as reported in QCOUNT
 *   - Register: @ref LTC4015_QCOUNT_LO_ALERT_LIMIT "QCOUNT_LO_ALERT_LIMIT"
 *   - CommandCode: 16
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_QCOUNT_LO_ALERT_LIMIT_BF_SUBADDR LTC4015_QCOUNT_LO_ALERT_LIMIT_SUBADDR //!< @ref LTC4015_QCOUNT_LO_ALERT_LIMIT_BF "QCOUNT_LO_ALERT_LIMIT_BF"
#define LTC4015_QCOUNT_LO_ALERT_LIMIT_BF_SIZE 16
#define LTC4015_QCOUNT_LO_ALERT_LIMIT_BF_OFFSET 0
#define LTC4015_QCOUNT_LO_ALERT_LIMIT_BF_MASK 0xFFFF
#define LTC4015_QCOUNT_LO_ALERT_LIMIT_BF (LTC4015_QCOUNT_LO_ALERT_LIMIT_BF_OFFSET << 12 | (LTC4015_QCOUNT_LO_ALERT_LIMIT_BF_SIZE - 1) << 8 | LTC4015_QCOUNT_LO_ALERT_LIMIT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_QCOUNT_HI_ALERT_LIMIT QCOUNT_HI_ALERT_LIMIT
 *  @ingroup LTC4015_register_map
 *  @brief QCOUNT_HI_ALERT_LIMIT Register
 *
 * |                     15:0 |
 * |:------------------------:|
 * | QCOUNT_HI_ALERT_LIMIT_BF |
 *
 *   - CommandCode: 17
 *   - Contains Bit Fields:
 *     + @ref LTC4015_QCOUNT_HI_ALERT_LIMIT_BF "QCOUNT_HI_ALERT_LIMIT_BF" : Coulomb counter high alert limit as reported in QCOUNT
*/

//!@{
#define LTC4015_QCOUNT_HI_ALERT_LIMIT_SUBADDR 17
#define LTC4015_QCOUNT_HI_ALERT_LIMIT (0 << 12 | (16 - 1) << 8 | LTC4015_QCOUNT_HI_ALERT_LIMIT_SUBADDR)
//!@}
/*! @defgroup LTC4015_QCOUNT_HI_ALERT_LIMIT_BF QCOUNT_HI_ALERT_LIMIT_BF
 *  @ingroup LTC4015_register_map
 *  @brief QCOUNT_HI_ALERT_LIMIT_BF Bit Field
 *
 *  Coulomb counter high alert limit as reported in QCOUNT
 *   - Register: @ref LTC4015_QCOUNT_HI_ALERT_LIMIT "QCOUNT_HI_ALERT_LIMIT"
 *   - CommandCode: 17
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_QCOUNT_HI_ALERT_LIMIT_BF_SUBADDR LTC4015_QCOUNT_HI_ALERT_LIMIT_SUBADDR //!< @ref LTC4015_QCOUNT_HI_ALERT_LIMIT_BF "QCOUNT_HI_ALERT_LIMIT_BF"
#define LTC4015_QCOUNT_HI_ALERT_LIMIT_BF_SIZE 16
#define LTC4015_QCOUNT_HI_ALERT_LIMIT_BF_OFFSET 0
#define LTC4015_QCOUNT_HI_ALERT_LIMIT_BF_MASK 0xFFFF
#define LTC4015_QCOUNT_HI_ALERT_LIMIT_BF (LTC4015_QCOUNT_HI_ALERT_LIMIT_BF_OFFSET << 12 | (LTC4015_QCOUNT_HI_ALERT_LIMIT_BF_SIZE - 1) << 8 | LTC4015_QCOUNT_HI_ALERT_LIMIT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_QCOUNT_PRESCALE_FACTOR QCOUNT_PRESCALE_FACTOR
 *  @ingroup LTC4015_register_map
 *  @brief QCOUNT_PRESCALE_FACTOR Register
 *
 * |                      15:0 |
 * |:-------------------------:|
 * | QCOUNT_PRESCALE_FACTOR_BF |
 *
 *   - CommandCode: 18
 *   - Contains Bit Fields:
 *     + @ref LTC4015_QCOUNT_PRESCALE_FACTOR_BF "QCOUNT_PRESCALE_FACTOR_BF" : Coulomb counter prescale factor
*/

//!@{
#define LTC4015_QCOUNT_PRESCALE_FACTOR_SUBADDR 18
#define LTC4015_QCOUNT_PRESCALE_FACTOR (0 << 12 | (16 - 1) << 8 | LTC4015_QCOUNT_PRESCALE_FACTOR_SUBADDR)
//!@}
/*! @defgroup LTC4015_QCOUNT_PRESCALE_FACTOR_BF QCOUNT_PRESCALE_FACTOR_BF
 *  @ingroup LTC4015_register_map
 *  @brief QCOUNT_PRESCALE_FACTOR_BF Bit Field
 *
 *  Coulomb counter prescale factor
 *   - Register: @ref LTC4015_QCOUNT_PRESCALE_FACTOR "QCOUNT_PRESCALE_FACTOR"
 *   - CommandCode: 18
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 512
 */
//!@{
#define LTC4015_QCOUNT_PRESCALE_FACTOR_BF_SUBADDR LTC4015_QCOUNT_PRESCALE_FACTOR_SUBADDR //!< @ref LTC4015_QCOUNT_PRESCALE_FACTOR_BF "QCOUNT_PRESCALE_FACTOR_BF"
#define LTC4015_QCOUNT_PRESCALE_FACTOR_BF_SIZE 16
#define LTC4015_QCOUNT_PRESCALE_FACTOR_BF_OFFSET 0
#define LTC4015_QCOUNT_PRESCALE_FACTOR_BF_MASK 0xFFFF
#define LTC4015_QCOUNT_PRESCALE_FACTOR_BF (LTC4015_QCOUNT_PRESCALE_FACTOR_BF_OFFSET << 12 | (LTC4015_QCOUNT_PRESCALE_FACTOR_BF_SIZE - 1) << 8 | LTC4015_QCOUNT_PRESCALE_FACTOR_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_QCOUNT QCOUNT
 *  @ingroup LTC4015_register_map
 *  @brief QCOUNT Register
 *
 * |      15:0 |
 * |:---------:|
 * | QCOUNT_BF |
 *
 *   - CommandCode: 19
 *   - Contains Bit Fields:
 *     + @ref LTC4015_QCOUNT_BF "QCOUNT_BF" : Coulomb counter value
*/

//!@{
#define LTC4015_QCOUNT_SUBADDR 19
#define LTC4015_QCOUNT (0 << 12 | (16 - 1) << 8 | LTC4015_QCOUNT_SUBADDR)
//!@}
/*! @defgroup LTC4015_QCOUNT_BF QCOUNT_BF
 *  @ingroup LTC4015_register_map
 *  @brief QCOUNT_BF Bit Field
 *
 *  Coulomb counter value
 *   - Register: @ref LTC4015_QCOUNT "QCOUNT"
 *   - CommandCode: 19
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 32768
 */
//!@{
#define LTC4015_QCOUNT_BF_SUBADDR LTC4015_QCOUNT_SUBADDR //!< @ref LTC4015_QCOUNT_BF "QCOUNT_BF"
#define LTC4015_QCOUNT_BF_SIZE 16
#define LTC4015_QCOUNT_BF_OFFSET 0
#define LTC4015_QCOUNT_BF_MASK 0xFFFF
#define LTC4015_QCOUNT_BF (LTC4015_QCOUNT_BF_OFFSET << 12 | (LTC4015_QCOUNT_BF_SIZE - 1) << 8 | LTC4015_QCOUNT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_CONFIG_BITS CONFIG_BITS
 *  @ingroup LTC4015_register_map
 *  @brief CONFIG_BITS Register
 *
 * | 15:9 |                  8 | 7:6 |          5 |                    4 |              3 |            2 |
 * |:----:|:------------------:|:---:|:----------:|:--------------------:|:--------------:|:------------:|
 * |  n/a | SUSPEND_CHARGER_BF | n/a | RUN_BSR_BF | FORCE_MEAS_SYS_ON_BF | MPPT_EN_I2C_BF | EN_QCOUNT_BF |
 *
 * Configuration Settings
 *   - CommandCode: 20
 *   - Contains Bit Fields:
 *     + @ref LTC4015_SUSPEND_CHARGER_BF "SUSPEND_CHARGER_BF" : Suspend battery charger operation
 *     + @ref LTC4015_RUN_BSR_BF "RUN_BSR_BF" : Perform a battery series resistance measurement
 *     + @ref LTC4015_FORCE_MEAS_SYS_ON_BF "FORCE_MEAS_SYS_ON_BF" : Force measurement system to operate
 *     + @ref LTC4015_MPPT_EN_I2C_BF "MPPT_EN_I2C_BF" : Enable Maximum Power Point Tracking
 *     + @ref LTC4015_EN_QCOUNT_BF "EN_QCOUNT_BF" : Enable coulomb counter
*/

//!@{
#define LTC4015_CONFIG_BITS_SUBADDR 20
#define LTC4015_CONFIG_BITS (0 << 12 | (16 - 1) << 8 | LTC4015_CONFIG_BITS_SUBADDR)
//!@}
/*! @defgroup LTC4015_SUSPEND_CHARGER_BF SUSPEND_CHARGER_BF
 *  @ingroup LTC4015_register_map
 *  @brief SUSPEND_CHARGER_BF Bit Field
 *
 *  Suspend battery charger operation
 *   - Register: @ref LTC4015_CONFIG_BITS "CONFIG_BITS"
 *   - CommandCode: 20
 *   - Size: 1
 *   - Offset: 8
 *   - MSB: 8
 *   - MASK: 0x0100
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_SUSPEND_CHARGER_BF_SUBADDR LTC4015_CONFIG_BITS_SUBADDR //!< @ref LTC4015_SUSPEND_CHARGER_BF "SUSPEND_CHARGER_BF"
#define LTC4015_SUSPEND_CHARGER_BF_SIZE 1
#define LTC4015_SUSPEND_CHARGER_BF_OFFSET 8
#define LTC4015_SUSPEND_CHARGER_BF_MASK 0x0100
#define LTC4015_SUSPEND_CHARGER_BF (LTC4015_SUSPEND_CHARGER_BF_OFFSET << 12 | (LTC4015_SUSPEND_CHARGER_BF_SIZE - 1) << 8 | LTC4015_SUSPEND_CHARGER_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_RUN_BSR_BF RUN_BSR_BF
 *  @ingroup LTC4015_register_map
 *  @brief RUN_BSR_BF Bit Field
 *
 *  Perform a battery series resistance measurement
 *   - Register: @ref LTC4015_CONFIG_BITS "CONFIG_BITS"
 *   - CommandCode: 20
 *   - Size: 1
 *   - Offset: 5
 *   - MSB: 5
 *   - MASK: 0x0020
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_RUN_BSR_BF_SUBADDR LTC4015_CONFIG_BITS_SUBADDR //!< @ref LTC4015_RUN_BSR_BF "RUN_BSR_BF"
#define LTC4015_RUN_BSR_BF_SIZE 1
#define LTC4015_RUN_BSR_BF_OFFSET 5
#define LTC4015_RUN_BSR_BF_MASK 0x0020
#define LTC4015_RUN_BSR_BF (LTC4015_RUN_BSR_BF_OFFSET << 12 | (LTC4015_RUN_BSR_BF_SIZE - 1) << 8 | LTC4015_RUN_BSR_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_FORCE_MEAS_SYS_ON_BF FORCE_MEAS_SYS_ON_BF
 *  @ingroup LTC4015_register_map
 *  @brief FORCE_MEAS_SYS_ON_BF Bit Field
 *
 *  Force measurement system to operate
 *   - Register: @ref LTC4015_CONFIG_BITS "CONFIG_BITS"
 *   - CommandCode: 20
 *   - Size: 1
 *   - Offset: 4
 *   - MSB: 4
 *   - MASK: 0x0010
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_FORCE_MEAS_SYS_ON_BF_SUBADDR LTC4015_CONFIG_BITS_SUBADDR //!< @ref LTC4015_FORCE_MEAS_SYS_ON_BF "FORCE_MEAS_SYS_ON_BF"
#define LTC4015_FORCE_MEAS_SYS_ON_BF_SIZE 1
#define LTC4015_FORCE_MEAS_SYS_ON_BF_OFFSET 4
#define LTC4015_FORCE_MEAS_SYS_ON_BF_MASK 0x0010
#define LTC4015_FORCE_MEAS_SYS_ON_BF (LTC4015_FORCE_MEAS_SYS_ON_BF_OFFSET << 12 | (LTC4015_FORCE_MEAS_SYS_ON_BF_SIZE - 1) << 8 | LTC4015_FORCE_MEAS_SYS_ON_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_MPPT_EN_I2C_BF MPPT_EN_I2C_BF
 *  @ingroup LTC4015_register_map
 *  @brief MPPT_EN_I2C_BF Bit Field
 *
 *  Enable Maximum Power Point Tracking
 *   - Register: @ref LTC4015_CONFIG_BITS "CONFIG_BITS"
 *   - CommandCode: 20
 *   - Size: 1
 *   - Offset: 3
 *   - MSB: 3
 *   - MASK: 0x0008
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_MPPT_EN_I2C_BF_SUBADDR LTC4015_CONFIG_BITS_SUBADDR //!< @ref LTC4015_MPPT_EN_I2C_BF "MPPT_EN_I2C_BF"
#define LTC4015_MPPT_EN_I2C_BF_SIZE 1
#define LTC4015_MPPT_EN_I2C_BF_OFFSET 3
#define LTC4015_MPPT_EN_I2C_BF_MASK 0x0008
#define LTC4015_MPPT_EN_I2C_BF (LTC4015_MPPT_EN_I2C_BF_OFFSET << 12 | (LTC4015_MPPT_EN_I2C_BF_SIZE - 1) << 8 | LTC4015_MPPT_EN_I2C_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_QCOUNT_BF EN_QCOUNT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_QCOUNT_BF Bit Field
 *
 *  Enable coulomb counter
 *   - Register: @ref LTC4015_CONFIG_BITS "CONFIG_BITS"
 *   - CommandCode: 20
 *   - Size: 1
 *   - Offset: 2
 *   - MSB: 2
 *   - MASK: 0x0004
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_QCOUNT_BF_SUBADDR LTC4015_CONFIG_BITS_SUBADDR //!< @ref LTC4015_EN_QCOUNT_BF "EN_QCOUNT_BF"
#define LTC4015_EN_QCOUNT_BF_SIZE 1
#define LTC4015_EN_QCOUNT_BF_OFFSET 2
#define LTC4015_EN_QCOUNT_BF_MASK 0x0004
#define LTC4015_EN_QCOUNT_BF (LTC4015_EN_QCOUNT_BF_OFFSET << 12 | (LTC4015_EN_QCOUNT_BF_SIZE - 1) << 8 | LTC4015_EN_QCOUNT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_IIN_LIMIT_SETTING IIN_LIMIT_SETTING
 *  @ingroup LTC4015_register_map
 *  @brief IIN_LIMIT_SETTING Register
 *
 * | 15:6 |                  5:0 |
 * |:----:|:--------------------:|
 * |  n/a | IIN_LIMIT_SETTING_BF |
 *
 *   - CommandCode: 21
 *   - Contains Bit Fields:
 *     + @ref LTC4015_IIN_LIMIT_SETTING_BF "IIN_LIMIT_SETTING_BF" : Input current limit setting = (IIN_LIMIT_SETTING + 1) * 500uV / R<sub>SNSI</sub>
*/

//!@{
#define LTC4015_IIN_LIMIT_SETTING_SUBADDR 21
#define LTC4015_IIN_LIMIT_SETTING (0 << 12 | (16 - 1) << 8 | LTC4015_IIN_LIMIT_SETTING_SUBADDR)
//!@}
/*! @defgroup LTC4015_IIN_LIMIT_SETTING_BF IIN_LIMIT_SETTING_BF
 *  @ingroup LTC4015_register_map
 *  @brief IIN_LIMIT_SETTING_BF Bit Field
 *
 *  Input current limit setting = (IIN_LIMIT_SETTING + 1) * 500uV / R<sub>SNSI</sub>
 *   - Register: @ref LTC4015_IIN_LIMIT_SETTING "IIN_LIMIT_SETTING"
 *   - CommandCode: 21
 *   - Size: 6
 *   - Offset: 0
 *   - MSB: 5
 *   - MASK: 0x003F
 *   - Access: R/W
 *   - Default: 63
 */
//!@{
#define LTC4015_IIN_LIMIT_SETTING_BF_SUBADDR LTC4015_IIN_LIMIT_SETTING_SUBADDR //!< @ref LTC4015_IIN_LIMIT_SETTING_BF "IIN_LIMIT_SETTING_BF"
#define LTC4015_IIN_LIMIT_SETTING_BF_SIZE 6
#define LTC4015_IIN_LIMIT_SETTING_BF_OFFSET 0
#define LTC4015_IIN_LIMIT_SETTING_BF_MASK 0x003F
#define LTC4015_IIN_LIMIT_SETTING_BF (LTC4015_IIN_LIMIT_SETTING_BF_OFFSET << 12 | (LTC4015_IIN_LIMIT_SETTING_BF_SIZE - 1) << 8 | LTC4015_IIN_LIMIT_SETTING_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VIN_UVCL_SETTING VIN_UVCL_SETTING
 *  @ingroup LTC4015_register_map
 *  @brief VIN_UVCL_SETTING Register
 *
 * | 15:8 |                 7:0 |
 * |:----:|:-------------------:|
 * |  n/a | VIN_UVCL_SETTING_BF |
 *
 *   - CommandCode: 22
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VIN_UVCL_SETTING_BF "VIN_UVCL_SETTING_BF" : UVCLFB input undervoltage limit = (VIN_UVCL_SETTING + 1) * 4.6875mV
*/

//!@{
#define LTC4015_VIN_UVCL_SETTING_SUBADDR 22
#define LTC4015_VIN_UVCL_SETTING (0 << 12 | (16 - 1) << 8 | LTC4015_VIN_UVCL_SETTING_SUBADDR)
//!@}
/*! @defgroup LTC4015_VIN_UVCL_SETTING_BF VIN_UVCL_SETTING_BF
 *  @ingroup LTC4015_register_map
 *  @brief VIN_UVCL_SETTING_BF Bit Field
 *
 *  UVCLFB input undervoltage limit = (VIN_UVCL_SETTING + 1) * 4.6875mV
 *   - Register: @ref LTC4015_VIN_UVCL_SETTING "VIN_UVCL_SETTING"
 *   - CommandCode: 22
 *   - Size: 8
 *   - Offset: 0
 *   - MSB: 7
 *   - MASK: 0x00FF
 *   - Access: R/W
 *   - Default: 255
 */
//!@{
#define LTC4015_VIN_UVCL_SETTING_BF_SUBADDR LTC4015_VIN_UVCL_SETTING_SUBADDR //!< @ref LTC4015_VIN_UVCL_SETTING_BF "VIN_UVCL_SETTING_BF"
#define LTC4015_VIN_UVCL_SETTING_BF_SIZE 8
#define LTC4015_VIN_UVCL_SETTING_BF_OFFSET 0
#define LTC4015_VIN_UVCL_SETTING_BF_MASK 0x00FF
#define LTC4015_VIN_UVCL_SETTING_BF (LTC4015_VIN_UVCL_SETTING_BF_OFFSET << 12 | (LTC4015_VIN_UVCL_SETTING_BF_SIZE - 1) << 8 | LTC4015_VIN_UVCL_SETTING_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_RESERVED_0X17 RESERVED_0X17
 *  @ingroup LTC4015_register_map
 *  @brief RESERVED_0X17 Register
 *
 * |             15:0 |
 * |:----------------:|
 * | RESERVED_0X17_BF |
 *
 *   - CommandCode: 23
 *   - Contains Bit Fields:
 *     + @ref LTC4015_RESERVED_0X17_BF "RESERVED_0X17_BF" : Scratchpad register
*/

//!@{
#define LTC4015_RESERVED_0X17_SUBADDR 23
#define LTC4015_RESERVED_0X17 (0 << 12 | (16 - 1) << 8 | LTC4015_RESERVED_0X17_SUBADDR)
//!@}
/*! @defgroup LTC4015_RESERVED_0X17_BF RESERVED_0X17_BF
 *  @ingroup LTC4015_register_map
 *  @brief RESERVED_0X17_BF Bit Field
 *
 *  Scratchpad register
 *   - Register: @ref LTC4015_RESERVED_0X17 "RESERVED_0X17"
 *   - CommandCode: 23
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_RESERVED_0X17_BF_SUBADDR LTC4015_RESERVED_0X17_SUBADDR //!< @ref LTC4015_RESERVED_0X17_BF "RESERVED_0X17_BF"
#define LTC4015_RESERVED_0X17_BF_SIZE 16
#define LTC4015_RESERVED_0X17_BF_OFFSET 0
#define LTC4015_RESERVED_0X17_BF_MASK 0xFFFF
#define LTC4015_RESERVED_0X17_BF (LTC4015_RESERVED_0X17_BF_OFFSET << 12 | (LTC4015_RESERVED_0X17_BF_SIZE - 1) << 8 | LTC4015_RESERVED_0X17_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_RESERVED_0X18 RESERVED_0X18
 *  @ingroup LTC4015_register_map
 *  @brief RESERVED_0X18 Register
 *
 * |             15:0 |
 * |:----------------:|
 * | RESERVED_0X18_BF |
 *
 *   - CommandCode: 24
 *   - Contains Bit Fields:
 *     + @ref LTC4015_RESERVED_0X18_BF "RESERVED_0X18_BF" : Password
*/

//!@{
#define LTC4015_RESERVED_0X18_SUBADDR 24
#define LTC4015_RESERVED_0X18 (0 << 12 | (16 - 1) << 8 | LTC4015_RESERVED_0X18_SUBADDR)
//!@}
/*! @defgroup LTC4015_RESERVED_0X18_BF RESERVED_0X18_BF
 *  @ingroup LTC4015_register_map
 *  @brief RESERVED_0X18_BF Bit Field
 *
 *  Password
 *   - Register: @ref LTC4015_RESERVED_0X18 "RESERVED_0X18"
 *   - CommandCode: 24
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: n/a
 */
//!@{
#define LTC4015_RESERVED_0X18_BF_SUBADDR LTC4015_RESERVED_0X18_SUBADDR //!< @ref LTC4015_RESERVED_0X18_BF "RESERVED_0X18_BF"
#define LTC4015_RESERVED_0X18_BF_SIZE 16
#define LTC4015_RESERVED_0X18_BF_OFFSET 0
#define LTC4015_RESERVED_0X18_BF_MASK 0xFFFF
#define LTC4015_RESERVED_0X18_BF (LTC4015_RESERVED_0X18_BF_OFFSET << 12 | (LTC4015_RESERVED_0X18_BF_SIZE - 1) << 8 | LTC4015_RESERVED_0X18_BF_SUBADDR)
#define LTC4015_RESERVED_0X18_BF_PRESET_UNLOCKKEY 19540
#define LTC4015_RESERVED_0X18_BF_PRESET_OFF 0
//!@}

/*! @defgroup LTC4015_ARM_SHIP_MODE ARM_SHIP_MODE
 *  @ingroup LTC4015_register_map
 *  @brief ARM_SHIP_MODE Register
 *
 * |             15:0 |
 * |:----------------:|
 * | ARM_SHIP_MODE_BF |
 *
 *   - CommandCode: 25
 *   - Contains Bit Fields:
 *     + @ref LTC4015_ARM_SHIP_MODE_BF "ARM_SHIP_MODE_BF" : Arm low-current ship mode to activate upon VIN power removal
*/

//!@{
#define LTC4015_ARM_SHIP_MODE_SUBADDR 25
#define LTC4015_ARM_SHIP_MODE (0 << 12 | (16 - 1) << 8 | LTC4015_ARM_SHIP_MODE_SUBADDR)
//!@}
/*! @defgroup LTC4015_ARM_SHIP_MODE_BF ARM_SHIP_MODE_BF
 *  @ingroup LTC4015_register_map
 *  @brief ARM_SHIP_MODE_BF Bit Field
 *
 *  Arm low-current ship mode to activate upon VIN power removal
 *   - Register: @ref LTC4015_ARM_SHIP_MODE "ARM_SHIP_MODE"
 *   - CommandCode: 25
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_ARM_SHIP_MODE_BF_SUBADDR LTC4015_ARM_SHIP_MODE_SUBADDR //!< @ref LTC4015_ARM_SHIP_MODE_BF "ARM_SHIP_MODE_BF"
#define LTC4015_ARM_SHIP_MODE_BF_SIZE 16
#define LTC4015_ARM_SHIP_MODE_BF_OFFSET 0
#define LTC4015_ARM_SHIP_MODE_BF_MASK 0xFFFF
#define LTC4015_ARM_SHIP_MODE_BF (LTC4015_ARM_SHIP_MODE_BF_OFFSET << 12 | (LTC4015_ARM_SHIP_MODE_BF_SIZE - 1) << 8 | LTC4015_ARM_SHIP_MODE_BF_SUBADDR)
#define LTC4015_ARM_SHIP_MODE_BF_PRESET_ARM 21325
//!@}

/*! @defgroup LTC4015_ICHARGE_TARGET ICHARGE_TARGET
 *  @ingroup LTC4015_register_map
 *  @brief ICHARGE_TARGET Register
 *
 * | 15:5 |               4:0 |
 * |:----:|:-----------------:|
 * |  n/a | ICHARGE_TARGET_BF |
 *
 *   - CommandCode: 26
 *   - Contains Bit Fields:
 *     + @ref LTC4015_ICHARGE_TARGET_BF "ICHARGE_TARGET_BF" : Maximum charge current target = (ICHARGE_TARGET + 1) * 1mV / R<sub>SNSB</sub>
*/

//!@{
#define LTC4015_ICHARGE_TARGET_SUBADDR 26
#define LTC4015_ICHARGE_TARGET (0 << 12 | (16 - 1) << 8 | LTC4015_ICHARGE_TARGET_SUBADDR)
//!@}
/*! @defgroup LTC4015_ICHARGE_TARGET_BF ICHARGE_TARGET_BF
 *  @ingroup LTC4015_register_map
 *  @brief ICHARGE_TARGET_BF Bit Field
 *
 *  Maximum charge current target = (ICHARGE_TARGET + 1) * 1mV / R<sub>SNSB</sub>
 *   - Register: @ref LTC4015_ICHARGE_TARGET "ICHARGE_TARGET"
 *   - CommandCode: 26
 *   - Size: 5
 *   - Offset: 0
 *   - MSB: 4
 *   - MASK: 0x001F
 *   - Access: R/W
 *   - Default: 31
 */
//!@{
#define LTC4015_ICHARGE_TARGET_BF_SUBADDR LTC4015_ICHARGE_TARGET_SUBADDR //!< @ref LTC4015_ICHARGE_TARGET_BF "ICHARGE_TARGET_BF"
#define LTC4015_ICHARGE_TARGET_BF_SIZE 5
#define LTC4015_ICHARGE_TARGET_BF_OFFSET 0
#define LTC4015_ICHARGE_TARGET_BF_MASK 0x001F
#define LTC4015_ICHARGE_TARGET_BF (LTC4015_ICHARGE_TARGET_BF_OFFSET << 12 | (LTC4015_ICHARGE_TARGET_BF_SIZE - 1) << 8 | LTC4015_ICHARGE_TARGET_BF_SUBADDR)
#define LTC4015_ICHARGE_TARGET_BF_PRESET_MAX 31
#define LTC4015_ICHARGE_TARGET_BF_PRESET_MID 15
#define LTC4015_ICHARGE_TARGET_BF_PRESET_MIN 0
//!@}

/*! @defgroup LTC4015_VCHARGE_SETTING VCHARGE_SETTING
 *  @ingroup LTC4015_register_map
 *  @brief VCHARGE_SETTING Register
 *
 * | 15:6 |                5:0 |
 * |:----:|:------------------:|
 * |  n/a | VCHARGE_SETTING_BF |
 *
 *   - CommandCode: 27
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VCHARGE_SETTING_BF "VCHARGE_SETTING_BF" : For Li-Ion batteries, only the lower five bits (4:0) are active, charge voltage level is given by (VCHARGE_SETTING/80.0 + 3.8125)V/cell.
				For LiFePO4 batteries, only the lower five bits (4:0) are active, charge voltage level is given by (VCHARGE_SETTING/80.0 + 3.4125)V/cell.
				For lead-acid batteries, charge voltage level is given by (VCHARGE_SETTING/105.0 + 2.0)V/cell. 
*/

//!@{
#define LTC4015_VCHARGE_SETTING_SUBADDR 27
#define LTC4015_VCHARGE_SETTING (0 << 12 | (16 - 1) << 8 | LTC4015_VCHARGE_SETTING_SUBADDR)
//!@}
/*! @defgroup LTC4015_VCHARGE_SETTING_BF VCHARGE_SETTING_BF
 *  @ingroup LTC4015_register_map
 *  @brief VCHARGE_SETTING_BF Bit Field
 *
 *  For Li-Ion batteries, only the lower five bits (4:0) are active, charge voltage level is given by (VCHARGE_SETTING/80.0 + 3.8125)V/cell.
				For LiFePO4 batteries, only the lower five bits (4:0) are active, charge voltage level is given by (VCHARGE_SETTING/80.0 + 3.4125)V/cell.
				For lead-acid batteries, charge voltage level is given by (VCHARGE_SETTING/105.0 + 2.0)V/cell. 
 *   - Register: @ref LTC4015_VCHARGE_SETTING "VCHARGE_SETTING"
 *   - CommandCode: 27
 *   - Size: 6
 *   - Offset: 0
 *   - MSB: 5
 *   - MASK: 0x003F
 *   - Access: R/W
 *   - Default: 63
 */
//!@{
#define LTC4015_VCHARGE_SETTING_BF_SUBADDR LTC4015_VCHARGE_SETTING_SUBADDR //!< @ref LTC4015_VCHARGE_SETTING_BF "VCHARGE_SETTING_BF"
#define LTC4015_VCHARGE_SETTING_BF_SIZE 6
#define LTC4015_VCHARGE_SETTING_BF_OFFSET 0
#define LTC4015_VCHARGE_SETTING_BF_MASK 0x003F
#define LTC4015_VCHARGE_SETTING_BF (LTC4015_VCHARGE_SETTING_BF_OFFSET << 12 | (LTC4015_VCHARGE_SETTING_BF_SIZE - 1) << 8 | LTC4015_VCHARGE_SETTING_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_C_OVER_X_THRESHOLD C_OVER_X_THRESHOLD
 *  @ingroup LTC4015_register_map
 *  @brief C_OVER_X_THRESHOLD Register
 *
 * |                  15:0 |
 * |:---------------------:|
 * | C_OVER_X_THRESHOLD_BF |
 *
 *   - CommandCode: 28
 *   - Contains Bit Fields:
 *     + @ref LTC4015_C_OVER_X_THRESHOLD_BF "C_OVER_X_THRESHOLD_BF" : Low IBAT threshold for C/x termination
*/

//!@{
#define LTC4015_C_OVER_X_THRESHOLD_SUBADDR 28
#define LTC4015_C_OVER_X_THRESHOLD (0 << 12 | (16 - 1) << 8 | LTC4015_C_OVER_X_THRESHOLD_SUBADDR)
//!@}
/*! @defgroup LTC4015_C_OVER_X_THRESHOLD_BF C_OVER_X_THRESHOLD_BF
 *  @ingroup LTC4015_register_map
 *  @brief C_OVER_X_THRESHOLD_BF Bit Field
 *
 *  Low IBAT threshold for C/x termination
 *   - Register: @ref LTC4015_C_OVER_X_THRESHOLD "C_OVER_X_THRESHOLD"
 *   - CommandCode: 28
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 2184
 */
//!@{
#define LTC4015_C_OVER_X_THRESHOLD_BF_SUBADDR LTC4015_C_OVER_X_THRESHOLD_SUBADDR //!< @ref LTC4015_C_OVER_X_THRESHOLD_BF "C_OVER_X_THRESHOLD_BF"
#define LTC4015_C_OVER_X_THRESHOLD_BF_SIZE 16
#define LTC4015_C_OVER_X_THRESHOLD_BF_OFFSET 0
#define LTC4015_C_OVER_X_THRESHOLD_BF_MASK 0xFFFF
#define LTC4015_C_OVER_X_THRESHOLD_BF (LTC4015_C_OVER_X_THRESHOLD_BF_OFFSET << 12 | (LTC4015_C_OVER_X_THRESHOLD_BF_SIZE - 1) << 8 | LTC4015_C_OVER_X_THRESHOLD_BF_SUBADDR)
#define LTC4015_C_OVER_X_THRESHOLD_BF_PRESET_C_OVER_10 2184
//!@}

/*! @defgroup LTC4015_MAX_CV_TIME MAX_CV_TIME
 *  @ingroup LTC4015_register_map
 *  @brief MAX_CV_TIME Register
 *
 * |           15:0 |
 * |:--------------:|
 * | MAX_CV_TIME_BF |
 *
 *   - CommandCode: 29
 *   - Contains Bit Fields:
 *     + @ref LTC4015_MAX_CV_TIME_BF "MAX_CV_TIME_BF" : Time in seconds with Charger in CV state before timer termination occurs [(Li Only)]
*/

//!@{
#define LTC4015_MAX_CV_TIME_SUBADDR 29
#define LTC4015_MAX_CV_TIME (0 << 12 | (16 - 1) << 8 | LTC4015_MAX_CV_TIME_SUBADDR)
//!@}
/*! @defgroup LTC4015_MAX_CV_TIME_BF MAX_CV_TIME_BF
 *  @ingroup LTC4015_register_map
 *  @brief MAX_CV_TIME_BF Bit Field
 *
 *  Time in seconds with Charger in CV state before timer termination occurs [(Li Only)]
 *   - Register: @ref LTC4015_MAX_CV_TIME "MAX_CV_TIME"
 *   - CommandCode: 29
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 65535
 */
//!@{
#define LTC4015_MAX_CV_TIME_BF_SUBADDR LTC4015_MAX_CV_TIME_SUBADDR //!< @ref LTC4015_MAX_CV_TIME_BF "MAX_CV_TIME_BF"
#define LTC4015_MAX_CV_TIME_BF_SIZE 16
#define LTC4015_MAX_CV_TIME_BF_OFFSET 0
#define LTC4015_MAX_CV_TIME_BF_MASK 0xFFFF
#define LTC4015_MAX_CV_TIME_BF (LTC4015_MAX_CV_TIME_BF_OFFSET << 12 | (LTC4015_MAX_CV_TIME_BF_SIZE - 1) << 8 | LTC4015_MAX_CV_TIME_BF_SUBADDR)
#define LTC4015_MAX_CV_TIME_BF_PRESET__30SEC 30
#define LTC4015_MAX_CV_TIME_BF_PRESET__1MIN 60
#define LTC4015_MAX_CV_TIME_BF_PRESET__30MINS 1800
#define LTC4015_MAX_CV_TIME_BF_PRESET__1HOUR 3600
#define LTC4015_MAX_CV_TIME_BF_PRESET__2HOURS 7200
#define LTC4015_MAX_CV_TIME_BF_PRESET__4HOURS 14400
#define LTC4015_MAX_CV_TIME_BF_PRESET__8HOURS 28800
#define LTC4015_MAX_CV_TIME_BF_PRESET_MAX 65535
//!@}

/*! @defgroup LTC4015_MAX_CHARGE_TIME MAX_CHARGE_TIME
 *  @ingroup LTC4015_register_map
 *  @brief MAX_CHARGE_TIME Register
 *
 * |               15:0 |
 * |:------------------:|
 * | MAX_CHARGE_TIME_BF |
 *
 *   - CommandCode: 30
 *   - Contains Bit Fields:
 *     + @ref LTC4015_MAX_CHARGE_TIME_BF "MAX_CHARGE_TIME_BF" : Time in seconds before a max_charge_time fault is declared.
*/

//!@{
#define LTC4015_MAX_CHARGE_TIME_SUBADDR 30
#define LTC4015_MAX_CHARGE_TIME (0 << 12 | (16 - 1) << 8 | LTC4015_MAX_CHARGE_TIME_SUBADDR)
//!@}
/*! @defgroup LTC4015_MAX_CHARGE_TIME_BF MAX_CHARGE_TIME_BF
 *  @ingroup LTC4015_register_map
 *  @brief MAX_CHARGE_TIME_BF Bit Field
 *
 *  Time in seconds before a max_charge_time fault is declared.
 *   - Register: @ref LTC4015_MAX_CHARGE_TIME "MAX_CHARGE_TIME"
 *   - CommandCode: 30
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 65535
 */
//!@{
#define LTC4015_MAX_CHARGE_TIME_BF_SUBADDR LTC4015_MAX_CHARGE_TIME_SUBADDR //!< @ref LTC4015_MAX_CHARGE_TIME_BF "MAX_CHARGE_TIME_BF"
#define LTC4015_MAX_CHARGE_TIME_BF_SIZE 16
#define LTC4015_MAX_CHARGE_TIME_BF_OFFSET 0
#define LTC4015_MAX_CHARGE_TIME_BF_MASK 0xFFFF
#define LTC4015_MAX_CHARGE_TIME_BF (LTC4015_MAX_CHARGE_TIME_BF_OFFSET << 12 | (LTC4015_MAX_CHARGE_TIME_BF_SIZE - 1) << 8 | LTC4015_MAX_CHARGE_TIME_BF_SUBADDR)
#define LTC4015_MAX_CHARGE_TIME_BF_PRESET__30SEC 30
#define LTC4015_MAX_CHARGE_TIME_BF_PRESET__1MIN 60
#define LTC4015_MAX_CHARGE_TIME_BF_PRESET__30MINS 1800
#define LTC4015_MAX_CHARGE_TIME_BF_PRESET__1HOUR 3600
#define LTC4015_MAX_CHARGE_TIME_BF_PRESET__2HOURS 7200
#define LTC4015_MAX_CHARGE_TIME_BF_PRESET__4HOURS 14400
#define LTC4015_MAX_CHARGE_TIME_BF_PRESET__8HOURS 28800
#define LTC4015_MAX_CHARGE_TIME_BF_PRESET_MAX 65535
//!@}

/*! @defgroup LTC4015_JEITA_T1 JEITA_T1
 *  @ingroup LTC4015_register_map
 *  @brief JEITA_T1 Register
 *
 * |        15:0 |
 * |:-----------:|
 * | JEITA_T1_BF |
 *
 *   - CommandCode: 31
 *   - Contains Bit Fields:
 *     + @ref LTC4015_JEITA_T1_BF "JEITA_T1_BF" : Value of NTC_RATIO for transition between JEITA regions 2 and 1 (off)
*/

//!@{
#define LTC4015_JEITA_T1_SUBADDR 31
#define LTC4015_JEITA_T1 (0 << 12 | (16 - 1) << 8 | LTC4015_JEITA_T1_SUBADDR)
//!@}
/*! @defgroup LTC4015_JEITA_T1_BF JEITA_T1_BF
 *  @ingroup LTC4015_register_map
 *  @brief JEITA_T1_BF Bit Field
 *
 *  Value of NTC_RATIO for transition between JEITA regions 2 and 1 (off)
 *   - Register: @ref LTC4015_JEITA_T1 "JEITA_T1"
 *   - CommandCode: 31
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 16128
 */
//!@{
#define LTC4015_JEITA_T1_BF_SUBADDR LTC4015_JEITA_T1_SUBADDR //!< @ref LTC4015_JEITA_T1_BF "JEITA_T1_BF"
#define LTC4015_JEITA_T1_BF_SIZE 16
#define LTC4015_JEITA_T1_BF_OFFSET 0
#define LTC4015_JEITA_T1_BF_MASK 0xFFFF
#define LTC4015_JEITA_T1_BF (LTC4015_JEITA_T1_BF_OFFSET << 12 | (LTC4015_JEITA_T1_BF_SIZE - 1) << 8 | LTC4015_JEITA_T1_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_JEITA_T2 JEITA_T2
 *  @ingroup LTC4015_register_map
 *  @brief JEITA_T2 Register
 *
 * |        15:0 |
 * |:-----------:|
 * | JEITA_T2_BF |
 *
 *   - CommandCode: 32
 *   - Contains Bit Fields:
 *     + @ref LTC4015_JEITA_T2_BF "JEITA_T2_BF" : Value of NTC_RATIO for transition between JEITA regions 3 and 2
*/

//!@{
#define LTC4015_JEITA_T2_SUBADDR 32
#define LTC4015_JEITA_T2 (0 << 12 | (16 - 1) << 8 | LTC4015_JEITA_T2_SUBADDR)
//!@}
/*! @defgroup LTC4015_JEITA_T2_BF JEITA_T2_BF
 *  @ingroup LTC4015_register_map
 *  @brief JEITA_T2_BF Bit Field
 *
 *  Value of NTC_RATIO for transition between JEITA regions 3 and 2
 *   - Register: @ref LTC4015_JEITA_T2 "JEITA_T2"
 *   - CommandCode: 32
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 14122
 */
//!@{
#define LTC4015_JEITA_T2_BF_SUBADDR LTC4015_JEITA_T2_SUBADDR //!< @ref LTC4015_JEITA_T2_BF "JEITA_T2_BF"
#define LTC4015_JEITA_T2_BF_SIZE 16
#define LTC4015_JEITA_T2_BF_OFFSET 0
#define LTC4015_JEITA_T2_BF_MASK 0xFFFF
#define LTC4015_JEITA_T2_BF (LTC4015_JEITA_T2_BF_OFFSET << 12 | (LTC4015_JEITA_T2_BF_SIZE - 1) << 8 | LTC4015_JEITA_T2_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_JEITA_T3 JEITA_T3
 *  @ingroup LTC4015_register_map
 *  @brief JEITA_T3 Register
 *
 * |        15:0 |
 * |:-----------:|
 * | JEITA_T3_BF |
 *
 *   - CommandCode: 33
 *   - Contains Bit Fields:
 *     + @ref LTC4015_JEITA_T3_BF "JEITA_T3_BF" : Value of NTC_RATIO for transition between JEITA regions 4 and 3
*/

//!@{
#define LTC4015_JEITA_T3_SUBADDR 33
#define LTC4015_JEITA_T3 (0 << 12 | (16 - 1) << 8 | LTC4015_JEITA_T3_SUBADDR)
//!@}
/*! @defgroup LTC4015_JEITA_T3_BF JEITA_T3_BF
 *  @ingroup LTC4015_register_map
 *  @brief JEITA_T3_BF Bit Field
 *
 *  Value of NTC_RATIO for transition between JEITA regions 4 and 3
 *   - Register: @ref LTC4015_JEITA_T3 "JEITA_T3"
 *   - CommandCode: 33
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 7975
 */
//!@{
#define LTC4015_JEITA_T3_BF_SUBADDR LTC4015_JEITA_T3_SUBADDR //!< @ref LTC4015_JEITA_T3_BF "JEITA_T3_BF"
#define LTC4015_JEITA_T3_BF_SIZE 16
#define LTC4015_JEITA_T3_BF_OFFSET 0
#define LTC4015_JEITA_T3_BF_MASK 0xFFFF
#define LTC4015_JEITA_T3_BF (LTC4015_JEITA_T3_BF_OFFSET << 12 | (LTC4015_JEITA_T3_BF_SIZE - 1) << 8 | LTC4015_JEITA_T3_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_JEITA_T4 JEITA_T4
 *  @ingroup LTC4015_register_map
 *  @brief JEITA_T4 Register
 *
 * |        15:0 |
 * |:-----------:|
 * | JEITA_T4_BF |
 *
 *   - CommandCode: 34
 *   - Contains Bit Fields:
 *     + @ref LTC4015_JEITA_T4_BF "JEITA_T4_BF" : Value of NTC_RATIO for transition between JEITA regions 5 and 4
*/

//!@{
#define LTC4015_JEITA_T4_SUBADDR 34
#define LTC4015_JEITA_T4 (0 << 12 | (16 - 1) << 8 | LTC4015_JEITA_T4_SUBADDR)
//!@}
/*! @defgroup LTC4015_JEITA_T4_BF JEITA_T4_BF
 *  @ingroup LTC4015_register_map
 *  @brief JEITA_T4_BF Bit Field
 *
 *  Value of NTC_RATIO for transition between JEITA regions 5 and 4
 *   - Register: @ref LTC4015_JEITA_T4 "JEITA_T4"
 *   - CommandCode: 34
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 7116
 */
//!@{
#define LTC4015_JEITA_T4_BF_SUBADDR LTC4015_JEITA_T4_SUBADDR //!< @ref LTC4015_JEITA_T4_BF "JEITA_T4_BF"
#define LTC4015_JEITA_T4_BF_SIZE 16
#define LTC4015_JEITA_T4_BF_OFFSET 0
#define LTC4015_JEITA_T4_BF_MASK 0xFFFF
#define LTC4015_JEITA_T4_BF (LTC4015_JEITA_T4_BF_OFFSET << 12 | (LTC4015_JEITA_T4_BF_SIZE - 1) << 8 | LTC4015_JEITA_T4_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_JEITA_T5 JEITA_T5
 *  @ingroup LTC4015_register_map
 *  @brief JEITA_T5 Register
 *
 * |        15:0 |
 * |:-----------:|
 * | JEITA_T5_BF |
 *
 *   - CommandCode: 35
 *   - Contains Bit Fields:
 *     + @ref LTC4015_JEITA_T5_BF "JEITA_T5_BF" : Value of NTC_RATIO for transition between JEITA regions 6 and 5
*/

//!@{
#define LTC4015_JEITA_T5_SUBADDR 35
#define LTC4015_JEITA_T5 (0 << 12 | (16 - 1) << 8 | LTC4015_JEITA_T5_SUBADDR)
//!@}
/*! @defgroup LTC4015_JEITA_T5_BF JEITA_T5_BF
 *  @ingroup LTC4015_register_map
 *  @brief JEITA_T5_BF Bit Field
 *
 *  Value of NTC_RATIO for transition between JEITA regions 6 and 5
 *   - Register: @ref LTC4015_JEITA_T5 "JEITA_T5"
 *   - CommandCode: 35
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 6329
 */
//!@{
#define LTC4015_JEITA_T5_BF_SUBADDR LTC4015_JEITA_T5_SUBADDR //!< @ref LTC4015_JEITA_T5_BF "JEITA_T5_BF"
#define LTC4015_JEITA_T5_BF_SIZE 16
#define LTC4015_JEITA_T5_BF_OFFSET 0
#define LTC4015_JEITA_T5_BF_MASK 0xFFFF
#define LTC4015_JEITA_T5_BF (LTC4015_JEITA_T5_BF_OFFSET << 12 | (LTC4015_JEITA_T5_BF_SIZE - 1) << 8 | LTC4015_JEITA_T5_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_JEITA_T6 JEITA_T6
 *  @ingroup LTC4015_register_map
 *  @brief JEITA_T6 Register
 *
 * |        15:0 |
 * |:-----------:|
 * | JEITA_T6_BF |
 *
 *   - CommandCode: 36
 *   - Contains Bit Fields:
 *     + @ref LTC4015_JEITA_T6_BF "JEITA_T6_BF" : Value of NTC_RATIO for transition between JEITA regions 7 (off) and 6
*/

//!@{
#define LTC4015_JEITA_T6_SUBADDR 36
#define LTC4015_JEITA_T6 (0 << 12 | (16 - 1) << 8 | LTC4015_JEITA_T6_SUBADDR)
//!@}
/*! @defgroup LTC4015_JEITA_T6_BF JEITA_T6_BF
 *  @ingroup LTC4015_register_map
 *  @brief JEITA_T6_BF Bit Field
 *
 *  Value of NTC_RATIO for transition between JEITA regions 7 (off) and 6
 *   - Register: @ref LTC4015_JEITA_T6 "JEITA_T6"
 *   - CommandCode: 36
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 4973
 */
//!@{
#define LTC4015_JEITA_T6_BF_SUBADDR LTC4015_JEITA_T6_SUBADDR //!< @ref LTC4015_JEITA_T6_BF "JEITA_T6_BF"
#define LTC4015_JEITA_T6_BF_SIZE 16
#define LTC4015_JEITA_T6_BF_OFFSET 0
#define LTC4015_JEITA_T6_BF_MASK 0xFFFF
#define LTC4015_JEITA_T6_BF (LTC4015_JEITA_T6_BF_OFFSET << 12 | (LTC4015_JEITA_T6_BF_SIZE - 1) << 8 | LTC4015_JEITA_T6_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VCHARGE_JEITA_6_5 VCHARGE_JEITA_6_5
 *  @ingroup LTC4015_register_map
 *  @brief VCHARGE_JEITA_6_5 Register
 *
 * | 15:10 |                9:5 |                4:0 |
 * |:-----:|:------------------:|:------------------:|
 * |   n/a | VCHARGE_JEITA_6_BF | VCHARGE_JEITA_5_BF |
 *
 * VCHARGE values for JEITA temperature regions 6 and 5
 *   - CommandCode: 37
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VCHARGE_JEITA_6_BF "VCHARGE_JEITA_6_BF" : vcharge_jeita_6
 *     + @ref LTC4015_VCHARGE_JEITA_5_BF "VCHARGE_JEITA_5_BF" : vcharge_jeita_5
*/

//!@{
#define LTC4015_VCHARGE_JEITA_6_5_SUBADDR 37
#define LTC4015_VCHARGE_JEITA_6_5 (0 << 12 | (16 - 1) << 8 | LTC4015_VCHARGE_JEITA_6_5_SUBADDR)
//!@}
/*! @defgroup LTC4015_VCHARGE_JEITA_6_BF VCHARGE_JEITA_6_BF
 *  @ingroup LTC4015_register_map
 *  @brief VCHARGE_JEITA_6_BF Bit Field
 *
 *  vcharge_jeita_6
 *   - Register: @ref LTC4015_VCHARGE_JEITA_6_5 "VCHARGE_JEITA_6_5"
 *   - CommandCode: 37
 *   - Size: 5
 *   - Offset: 5
 *   - MSB: 9
 *   - MASK: 0x03E0
 *   - Access: R/W
 *   - Default: n/a
 */
//!@{
#define LTC4015_VCHARGE_JEITA_6_BF_SUBADDR LTC4015_VCHARGE_JEITA_6_5_SUBADDR //!< @ref LTC4015_VCHARGE_JEITA_6_BF "VCHARGE_JEITA_6_BF"
#define LTC4015_VCHARGE_JEITA_6_BF_SIZE 5
#define LTC4015_VCHARGE_JEITA_6_BF_OFFSET 5
#define LTC4015_VCHARGE_JEITA_6_BF_MASK 0x03E0
#define LTC4015_VCHARGE_JEITA_6_BF (LTC4015_VCHARGE_JEITA_6_BF_OFFSET << 12 | (LTC4015_VCHARGE_JEITA_6_BF_SIZE - 1) << 8 | LTC4015_VCHARGE_JEITA_6_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_VCHARGE_JEITA_5_BF VCHARGE_JEITA_5_BF
 *  @ingroup LTC4015_register_map
 *  @brief VCHARGE_JEITA_5_BF Bit Field
 *
 *  vcharge_jeita_5
 *   - Register: @ref LTC4015_VCHARGE_JEITA_6_5 "VCHARGE_JEITA_6_5"
 *   - CommandCode: 37
 *   - Size: 5
 *   - Offset: 0
 *   - MSB: 4
 *   - MASK: 0x001F
 *   - Access: R/W
 *   - Default: n/a
 */
//!@{
#define LTC4015_VCHARGE_JEITA_5_BF_SUBADDR LTC4015_VCHARGE_JEITA_6_5_SUBADDR //!< @ref LTC4015_VCHARGE_JEITA_5_BF "VCHARGE_JEITA_5_BF"
#define LTC4015_VCHARGE_JEITA_5_BF_SIZE 5
#define LTC4015_VCHARGE_JEITA_5_BF_OFFSET 0
#define LTC4015_VCHARGE_JEITA_5_BF_MASK 0x001F
#define LTC4015_VCHARGE_JEITA_5_BF (LTC4015_VCHARGE_JEITA_5_BF_OFFSET << 12 | (LTC4015_VCHARGE_JEITA_5_BF_SIZE - 1) << 8 | LTC4015_VCHARGE_JEITA_5_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VCHARGE_JEITA_4_3_2 VCHARGE_JEITA_4_3_2
 *  @ingroup LTC4015_register_map
 *  @brief VCHARGE_JEITA_4_3_2 Register
 *
 * |  15 |              14:10 |                9:5 |                4:0 |
 * |:---:|:------------------:|:------------------:|:------------------:|
 * | n/a | VCHARGE_JEITA_4_BF | VCHARGE_JEITA_3_BF | VCHARGE_JEITA_2_BF |
 *
 * VCHARGE values for JEITA temperature regions 4, 3, and 2
 *   - CommandCode: 38
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VCHARGE_JEITA_4_BF "VCHARGE_JEITA_4_BF" : vcharge_jeita_4
 *     + @ref LTC4015_VCHARGE_JEITA_3_BF "VCHARGE_JEITA_3_BF" : vcharge_jeita_3
 *     + @ref LTC4015_VCHARGE_JEITA_2_BF "VCHARGE_JEITA_2_BF" : vcharge_jeita_2
*/

//!@{
#define LTC4015_VCHARGE_JEITA_4_3_2_SUBADDR 38
#define LTC4015_VCHARGE_JEITA_4_3_2 (0 << 12 | (16 - 1) << 8 | LTC4015_VCHARGE_JEITA_4_3_2_SUBADDR)
//!@}
/*! @defgroup LTC4015_VCHARGE_JEITA_4_BF VCHARGE_JEITA_4_BF
 *  @ingroup LTC4015_register_map
 *  @brief VCHARGE_JEITA_4_BF Bit Field
 *
 *  vcharge_jeita_4
 *   - Register: @ref LTC4015_VCHARGE_JEITA_4_3_2 "VCHARGE_JEITA_4_3_2"
 *   - CommandCode: 38
 *   - Size: 5
 *   - Offset: 10
 *   - MSB: 14
 *   - MASK: 0x7C00
 *   - Access: R/W
 *   - Default: n/a
 */
//!@{
#define LTC4015_VCHARGE_JEITA_4_BF_SUBADDR LTC4015_VCHARGE_JEITA_4_3_2_SUBADDR //!< @ref LTC4015_VCHARGE_JEITA_4_BF "VCHARGE_JEITA_4_BF"
#define LTC4015_VCHARGE_JEITA_4_BF_SIZE 5
#define LTC4015_VCHARGE_JEITA_4_BF_OFFSET 10
#define LTC4015_VCHARGE_JEITA_4_BF_MASK 0x7C00
#define LTC4015_VCHARGE_JEITA_4_BF (LTC4015_VCHARGE_JEITA_4_BF_OFFSET << 12 | (LTC4015_VCHARGE_JEITA_4_BF_SIZE - 1) << 8 | LTC4015_VCHARGE_JEITA_4_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_VCHARGE_JEITA_3_BF VCHARGE_JEITA_3_BF
 *  @ingroup LTC4015_register_map
 *  @brief VCHARGE_JEITA_3_BF Bit Field
 *
 *  vcharge_jeita_3
 *   - Register: @ref LTC4015_VCHARGE_JEITA_4_3_2 "VCHARGE_JEITA_4_3_2"
 *   - CommandCode: 38
 *   - Size: 5
 *   - Offset: 5
 *   - MSB: 9
 *   - MASK: 0x03E0
 *   - Access: R/W
 *   - Default: n/a
 */
//!@{
#define LTC4015_VCHARGE_JEITA_3_BF_SUBADDR LTC4015_VCHARGE_JEITA_4_3_2_SUBADDR //!< @ref LTC4015_VCHARGE_JEITA_3_BF "VCHARGE_JEITA_3_BF"
#define LTC4015_VCHARGE_JEITA_3_BF_SIZE 5
#define LTC4015_VCHARGE_JEITA_3_BF_OFFSET 5
#define LTC4015_VCHARGE_JEITA_3_BF_MASK 0x03E0
#define LTC4015_VCHARGE_JEITA_3_BF (LTC4015_VCHARGE_JEITA_3_BF_OFFSET << 12 | (LTC4015_VCHARGE_JEITA_3_BF_SIZE - 1) << 8 | LTC4015_VCHARGE_JEITA_3_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_VCHARGE_JEITA_2_BF VCHARGE_JEITA_2_BF
 *  @ingroup LTC4015_register_map
 *  @brief VCHARGE_JEITA_2_BF Bit Field
 *
 *  vcharge_jeita_2
 *   - Register: @ref LTC4015_VCHARGE_JEITA_4_3_2 "VCHARGE_JEITA_4_3_2"
 *   - CommandCode: 38
 *   - Size: 5
 *   - Offset: 0
 *   - MSB: 4
 *   - MASK: 0x001F
 *   - Access: R/W
 *   - Default: n/a
 */
//!@{
#define LTC4015_VCHARGE_JEITA_2_BF_SUBADDR LTC4015_VCHARGE_JEITA_4_3_2_SUBADDR //!< @ref LTC4015_VCHARGE_JEITA_2_BF "VCHARGE_JEITA_2_BF"
#define LTC4015_VCHARGE_JEITA_2_BF_SIZE 5
#define LTC4015_VCHARGE_JEITA_2_BF_OFFSET 0
#define LTC4015_VCHARGE_JEITA_2_BF_MASK 0x001F
#define LTC4015_VCHARGE_JEITA_2_BF (LTC4015_VCHARGE_JEITA_2_BF_OFFSET << 12 | (LTC4015_VCHARGE_JEITA_2_BF_SIZE - 1) << 8 | LTC4015_VCHARGE_JEITA_2_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_ICHARGE_JEITA_6_5 ICHARGE_JEITA_6_5
 *  @ingroup LTC4015_register_map
 *  @brief ICHARGE_JEITA_6_5 Register
 *
 * | 15:10 |                9:5 |                4:0 |
 * |:-----:|:------------------:|:------------------:|
 * |   n/a | ICHARGE_JEITA_6_BF | ICHARGE_JEITA_5_BF |
 *
 * ICHARGE_TARGET values for JEITA temperature regions 6 and 5
 *   - CommandCode: 39
 *   - Contains Bit Fields:
 *     + @ref LTC4015_ICHARGE_JEITA_6_BF "ICHARGE_JEITA_6_BF" : icharge_jeita_6
 *     + @ref LTC4015_ICHARGE_JEITA_5_BF "ICHARGE_JEITA_5_BF" : icharge_jeita_5
*/

//!@{
#define LTC4015_ICHARGE_JEITA_6_5_SUBADDR 39
#define LTC4015_ICHARGE_JEITA_6_5 (0 << 12 | (16 - 1) << 8 | LTC4015_ICHARGE_JEITA_6_5_SUBADDR)
//!@}
/*! @defgroup LTC4015_ICHARGE_JEITA_6_BF ICHARGE_JEITA_6_BF
 *  @ingroup LTC4015_register_map
 *  @brief ICHARGE_JEITA_6_BF Bit Field
 *
 *  icharge_jeita_6
 *   - Register: @ref LTC4015_ICHARGE_JEITA_6_5 "ICHARGE_JEITA_6_5"
 *   - CommandCode: 39
 *   - Size: 5
 *   - Offset: 5
 *   - MSB: 9
 *   - MASK: 0x03E0
 *   - Access: R/W
 *   - Default: 15
 */
//!@{
#define LTC4015_ICHARGE_JEITA_6_BF_SUBADDR LTC4015_ICHARGE_JEITA_6_5_SUBADDR //!< @ref LTC4015_ICHARGE_JEITA_6_BF "ICHARGE_JEITA_6_BF"
#define LTC4015_ICHARGE_JEITA_6_BF_SIZE 5
#define LTC4015_ICHARGE_JEITA_6_BF_OFFSET 5
#define LTC4015_ICHARGE_JEITA_6_BF_MASK 0x03E0
#define LTC4015_ICHARGE_JEITA_6_BF (LTC4015_ICHARGE_JEITA_6_BF_OFFSET << 12 | (LTC4015_ICHARGE_JEITA_6_BF_SIZE - 1) << 8 | LTC4015_ICHARGE_JEITA_6_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_ICHARGE_JEITA_5_BF ICHARGE_JEITA_5_BF
 *  @ingroup LTC4015_register_map
 *  @brief ICHARGE_JEITA_5_BF Bit Field
 *
 *  icharge_jeita_5
 *   - Register: @ref LTC4015_ICHARGE_JEITA_6_5 "ICHARGE_JEITA_6_5"
 *   - CommandCode: 39
 *   - Size: 5
 *   - Offset: 0
 *   - MSB: 4
 *   - MASK: 0x001F
 *   - Access: R/W
 *   - Default: 15
 */
//!@{
#define LTC4015_ICHARGE_JEITA_5_BF_SUBADDR LTC4015_ICHARGE_JEITA_6_5_SUBADDR //!< @ref LTC4015_ICHARGE_JEITA_5_BF "ICHARGE_JEITA_5_BF"
#define LTC4015_ICHARGE_JEITA_5_BF_SIZE 5
#define LTC4015_ICHARGE_JEITA_5_BF_OFFSET 0
#define LTC4015_ICHARGE_JEITA_5_BF_MASK 0x001F
#define LTC4015_ICHARGE_JEITA_5_BF (LTC4015_ICHARGE_JEITA_5_BF_OFFSET << 12 | (LTC4015_ICHARGE_JEITA_5_BF_SIZE - 1) << 8 | LTC4015_ICHARGE_JEITA_5_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_ICHARGE_JEITA_4_3_2 ICHARGE_JEITA_4_3_2
 *  @ingroup LTC4015_register_map
 *  @brief ICHARGE_JEITA_4_3_2 Register
 *
 * |  15 |              14:10 |                9:5 |                4:0 |
 * |:---:|:------------------:|:------------------:|:------------------:|
 * | n/a | ICHARGE_JEITA_4_BF | ICHARGE_JEITA_3_BF | ICHARGE_JEITA_2_BF |
 *
 * ICHARGE_TARGET value for JEITA temperature regions 4, 3, and 2
 *   - CommandCode: 40
 *   - Contains Bit Fields:
 *     + @ref LTC4015_ICHARGE_JEITA_4_BF "ICHARGE_JEITA_4_BF" : icharge_jeita_4
 *     + @ref LTC4015_ICHARGE_JEITA_3_BF "ICHARGE_JEITA_3_BF" : icharge_jeita_3
 *     + @ref LTC4015_ICHARGE_JEITA_2_BF "ICHARGE_JEITA_2_BF" : icharge_jeita_2
*/

//!@{
#define LTC4015_ICHARGE_JEITA_4_3_2_SUBADDR 40
#define LTC4015_ICHARGE_JEITA_4_3_2 (0 << 12 | (16 - 1) << 8 | LTC4015_ICHARGE_JEITA_4_3_2_SUBADDR)
//!@}
/*! @defgroup LTC4015_ICHARGE_JEITA_4_BF ICHARGE_JEITA_4_BF
 *  @ingroup LTC4015_register_map
 *  @brief ICHARGE_JEITA_4_BF Bit Field
 *
 *  icharge_jeita_4
 *   - Register: @ref LTC4015_ICHARGE_JEITA_4_3_2 "ICHARGE_JEITA_4_3_2"
 *   - CommandCode: 40
 *   - Size: 5
 *   - Offset: 10
 *   - MSB: 14
 *   - MASK: 0x7C00
 *   - Access: R/W
 *   - Default: 31
 */
//!@{
#define LTC4015_ICHARGE_JEITA_4_BF_SUBADDR LTC4015_ICHARGE_JEITA_4_3_2_SUBADDR //!< @ref LTC4015_ICHARGE_JEITA_4_BF "ICHARGE_JEITA_4_BF"
#define LTC4015_ICHARGE_JEITA_4_BF_SIZE 5
#define LTC4015_ICHARGE_JEITA_4_BF_OFFSET 10
#define LTC4015_ICHARGE_JEITA_4_BF_MASK 0x7C00
#define LTC4015_ICHARGE_JEITA_4_BF (LTC4015_ICHARGE_JEITA_4_BF_OFFSET << 12 | (LTC4015_ICHARGE_JEITA_4_BF_SIZE - 1) << 8 | LTC4015_ICHARGE_JEITA_4_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_ICHARGE_JEITA_3_BF ICHARGE_JEITA_3_BF
 *  @ingroup LTC4015_register_map
 *  @brief ICHARGE_JEITA_3_BF Bit Field
 *
 *  icharge_jeita_3
 *   - Register: @ref LTC4015_ICHARGE_JEITA_4_3_2 "ICHARGE_JEITA_4_3_2"
 *   - CommandCode: 40
 *   - Size: 5
 *   - Offset: 5
 *   - MSB: 9
 *   - MASK: 0x03E0
 *   - Access: R/W
 *   - Default: 31
 */
//!@{
#define LTC4015_ICHARGE_JEITA_3_BF_SUBADDR LTC4015_ICHARGE_JEITA_4_3_2_SUBADDR //!< @ref LTC4015_ICHARGE_JEITA_3_BF "ICHARGE_JEITA_3_BF"
#define LTC4015_ICHARGE_JEITA_3_BF_SIZE 5
#define LTC4015_ICHARGE_JEITA_3_BF_OFFSET 5
#define LTC4015_ICHARGE_JEITA_3_BF_MASK 0x03E0
#define LTC4015_ICHARGE_JEITA_3_BF (LTC4015_ICHARGE_JEITA_3_BF_OFFSET << 12 | (LTC4015_ICHARGE_JEITA_3_BF_SIZE - 1) << 8 | LTC4015_ICHARGE_JEITA_3_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_ICHARGE_JEITA_2_BF ICHARGE_JEITA_2_BF
 *  @ingroup LTC4015_register_map
 *  @brief ICHARGE_JEITA_2_BF Bit Field
 *
 *  icharge_jeita_2
 *   - Register: @ref LTC4015_ICHARGE_JEITA_4_3_2 "ICHARGE_JEITA_4_3_2"
 *   - CommandCode: 40
 *   - Size: 5
 *   - Offset: 0
 *   - MSB: 4
 *   - MASK: 0x001F
 *   - Access: R/W
 *   - Default: 15
 */
//!@{
#define LTC4015_ICHARGE_JEITA_2_BF_SUBADDR LTC4015_ICHARGE_JEITA_4_3_2_SUBADDR //!< @ref LTC4015_ICHARGE_JEITA_2_BF "ICHARGE_JEITA_2_BF"
#define LTC4015_ICHARGE_JEITA_2_BF_SIZE 5
#define LTC4015_ICHARGE_JEITA_2_BF_OFFSET 0
#define LTC4015_ICHARGE_JEITA_2_BF_MASK 0x001F
#define LTC4015_ICHARGE_JEITA_2_BF (LTC4015_ICHARGE_JEITA_2_BF_OFFSET << 12 | (LTC4015_ICHARGE_JEITA_2_BF_SIZE - 1) << 8 | LTC4015_ICHARGE_JEITA_2_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_CHARGER_CONFIG_BITS CHARGER_CONFIG_BITS
 *  @ingroup LTC4015_register_map
 *  @brief CHARGER_CONFIG_BITS Register
 *
 * | 15:3 |                   2 |                         1 |           0 |
 * |:----:|:-------------------:|:-------------------------:|:-----------:|
 * |  n/a | EN_C_OVER_X_TERM_BF | EN_LEAD_ACID_TEMP_COMP_BF | EN_JEITA_BF |
 *
 * Battery charger configuration setting
 *   - CommandCode: 41
 *   - Contains Bit Fields:
 *     + @ref LTC4015_EN_C_OVER_X_TERM_BF "EN_C_OVER_X_TERM_BF" : Enable C/x termination
 *     + @ref LTC4015_EN_LEAD_ACID_TEMP_COMP_BF "EN_LEAD_ACID_TEMP_COMP_BF" : Enable lead acid charge voltage temperature compensation
 *     + @ref LTC4015_EN_JEITA_BF "EN_JEITA_BF" : Enable JEITA temperature profile
*/

//!@{
#define LTC4015_CHARGER_CONFIG_BITS_SUBADDR 41
#define LTC4015_CHARGER_CONFIG_BITS (0 << 12 | (16 - 1) << 8 | LTC4015_CHARGER_CONFIG_BITS_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_C_OVER_X_TERM_BF EN_C_OVER_X_TERM_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_C_OVER_X_TERM_BF Bit Field
 *
 *  Enable C/x termination
 *   - Register: @ref LTC4015_CHARGER_CONFIG_BITS "CHARGER_CONFIG_BITS"
 *   - CommandCode: 41
 *   - Size: 1
 *   - Offset: 2
 *   - MSB: 2
 *   - MASK: 0x0004
 *   - Access: R/W
 *   - Default: 1
 */
//!@{
#define LTC4015_EN_C_OVER_X_TERM_BF_SUBADDR LTC4015_CHARGER_CONFIG_BITS_SUBADDR //!< @ref LTC4015_EN_C_OVER_X_TERM_BF "EN_C_OVER_X_TERM_BF"
#define LTC4015_EN_C_OVER_X_TERM_BF_SIZE 1
#define LTC4015_EN_C_OVER_X_TERM_BF_OFFSET 2
#define LTC4015_EN_C_OVER_X_TERM_BF_MASK 0x0004
#define LTC4015_EN_C_OVER_X_TERM_BF (LTC4015_EN_C_OVER_X_TERM_BF_OFFSET << 12 | (LTC4015_EN_C_OVER_X_TERM_BF_SIZE - 1) << 8 | LTC4015_EN_C_OVER_X_TERM_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_LEAD_ACID_TEMP_COMP_BF EN_LEAD_ACID_TEMP_COMP_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_LEAD_ACID_TEMP_COMP_BF Bit Field
 *
 *  Enable lead acid charge voltage temperature compensation
 *   - Register: @ref LTC4015_CHARGER_CONFIG_BITS "CHARGER_CONFIG_BITS"
 *   - CommandCode: 41
 *   - Size: 1
 *   - Offset: 1
 *   - MSB: 1
 *   - MASK: 0x0002
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_EN_LEAD_ACID_TEMP_COMP_BF_SUBADDR LTC4015_CHARGER_CONFIG_BITS_SUBADDR //!< @ref LTC4015_EN_LEAD_ACID_TEMP_COMP_BF "EN_LEAD_ACID_TEMP_COMP_BF"
#define LTC4015_EN_LEAD_ACID_TEMP_COMP_BF_SIZE 1
#define LTC4015_EN_LEAD_ACID_TEMP_COMP_BF_OFFSET 1
#define LTC4015_EN_LEAD_ACID_TEMP_COMP_BF_MASK 0x0002
#define LTC4015_EN_LEAD_ACID_TEMP_COMP_BF (LTC4015_EN_LEAD_ACID_TEMP_COMP_BF_OFFSET << 12 | (LTC4015_EN_LEAD_ACID_TEMP_COMP_BF_SIZE - 1) << 8 | LTC4015_EN_LEAD_ACID_TEMP_COMP_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EN_JEITA_BF EN_JEITA_BF
 *  @ingroup LTC4015_register_map
 *  @brief EN_JEITA_BF Bit Field
 *
 *  Enable JEITA temperature profile
 *   - Register: @ref LTC4015_CHARGER_CONFIG_BITS "CHARGER_CONFIG_BITS"
 *   - CommandCode: 41
 *   - Size: 1
 *   - Offset: 0
 *   - MSB: 0
 *   - MASK: 0x0001
 *   - Access: R/W
 *   - Default: 1
 */
//!@{
#define LTC4015_EN_JEITA_BF_SUBADDR LTC4015_CHARGER_CONFIG_BITS_SUBADDR //!< @ref LTC4015_EN_JEITA_BF "EN_JEITA_BF"
#define LTC4015_EN_JEITA_BF_SIZE 1
#define LTC4015_EN_JEITA_BF_OFFSET 0
#define LTC4015_EN_JEITA_BF_MASK 0x0001
#define LTC4015_EN_JEITA_BF (LTC4015_EN_JEITA_BF_OFFSET << 12 | (LTC4015_EN_JEITA_BF_SIZE - 1) << 8 | LTC4015_EN_JEITA_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VABSORB_DELTA VABSORB_DELTA
 *  @ingroup LTC4015_register_map
 *  @brief VABSORB_DELTA Register
 *
 * | 15:6 |              5:0 |
 * |:----:|:----------------:|
 * |  n/a | VABSORB_DELTA_BF |
 *
 *   - CommandCode: 42
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VABSORB_DELTA_BF "VABSORB_DELTA_BF" : LiFePO4[/lead-acid] absorb voltage adder
*/

//!@{
#define LTC4015_VABSORB_DELTA_SUBADDR 42
#define LTC4015_VABSORB_DELTA (0 << 12 | (16 - 1) << 8 | LTC4015_VABSORB_DELTA_SUBADDR)
//!@}
/*! @defgroup LTC4015_VABSORB_DELTA_BF VABSORB_DELTA_BF
 *  @ingroup LTC4015_register_map
 *  @brief VABSORB_DELTA_BF Bit Field
 *
 *  LiFePO4[/lead-acid] absorb voltage adder
 *   - Register: @ref LTC4015_VABSORB_DELTA "VABSORB_DELTA"
 *   - CommandCode: 42
 *   - Size: 6
 *   - Offset: 0
 *   - MSB: 5
 *   - MASK: 0x003F
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_VABSORB_DELTA_BF_SUBADDR LTC4015_VABSORB_DELTA_SUBADDR //!< @ref LTC4015_VABSORB_DELTA_BF "VABSORB_DELTA_BF"
#define LTC4015_VABSORB_DELTA_BF_SIZE 6
#define LTC4015_VABSORB_DELTA_BF_OFFSET 0
#define LTC4015_VABSORB_DELTA_BF_MASK 0x003F
#define LTC4015_VABSORB_DELTA_BF (LTC4015_VABSORB_DELTA_BF_OFFSET << 12 | (LTC4015_VABSORB_DELTA_BF_SIZE - 1) << 8 | LTC4015_VABSORB_DELTA_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_MAX_ABSORB_TIME MAX_ABSORB_TIME
 *  @ingroup LTC4015_register_map
 *  @brief MAX_ABSORB_TIME Register
 *
 * |               15:0 |
 * |:------------------:|
 * | MAX_ABSORB_TIME_BF |
 *
 *   - CommandCode: 43
 *   - Contains Bit Fields:
 *     + @ref LTC4015_MAX_ABSORB_TIME_BF "MAX_ABSORB_TIME_BF" : Maximum time for LiFePO4[/lead-acid] absorb charge
*/

//!@{
#define LTC4015_MAX_ABSORB_TIME_SUBADDR 43
#define LTC4015_MAX_ABSORB_TIME (0 << 12 | (16 - 1) << 8 | LTC4015_MAX_ABSORB_TIME_SUBADDR)
//!@}
/*! @defgroup LTC4015_MAX_ABSORB_TIME_BF MAX_ABSORB_TIME_BF
 *  @ingroup LTC4015_register_map
 *  @brief MAX_ABSORB_TIME_BF Bit Field
 *
 *  Maximum time for LiFePO4[/lead-acid] absorb charge
 *   - Register: @ref LTC4015_MAX_ABSORB_TIME "MAX_ABSORB_TIME"
 *   - CommandCode: 43
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_MAX_ABSORB_TIME_BF_SUBADDR LTC4015_MAX_ABSORB_TIME_SUBADDR //!< @ref LTC4015_MAX_ABSORB_TIME_BF "MAX_ABSORB_TIME_BF"
#define LTC4015_MAX_ABSORB_TIME_BF_SIZE 16
#define LTC4015_MAX_ABSORB_TIME_BF_OFFSET 0
#define LTC4015_MAX_ABSORB_TIME_BF_MASK 0xFFFF
#define LTC4015_MAX_ABSORB_TIME_BF (LTC4015_MAX_ABSORB_TIME_BF_OFFSET << 12 | (LTC4015_MAX_ABSORB_TIME_BF_SIZE - 1) << 8 | LTC4015_MAX_ABSORB_TIME_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VEQUALIZE_DELTA VEQUALIZE_DELTA
 *  @ingroup LTC4015_register_map
 *  @brief VEQUALIZE_DELTA Register
 *
 * | 15:6 |                5:0 |
 * |:----:|:------------------:|
 * |  n/a | VEQUALIZE_DELTA_BF |
 *
 *   - CommandCode: 44
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VEQUALIZE_DELTA_BF "VEQUALIZE_DELTA_BF" : Lead acid equalize charge voltage adder
*/

//!@{
#define LTC4015_VEQUALIZE_DELTA_SUBADDR 44
#define LTC4015_VEQUALIZE_DELTA (0 << 12 | (16 - 1) << 8 | LTC4015_VEQUALIZE_DELTA_SUBADDR)
//!@}
/*! @defgroup LTC4015_VEQUALIZE_DELTA_BF VEQUALIZE_DELTA_BF
 *  @ingroup LTC4015_register_map
 *  @brief VEQUALIZE_DELTA_BF Bit Field
 *
 *  Lead acid equalize charge voltage adder
 *   - Register: @ref LTC4015_VEQUALIZE_DELTA "VEQUALIZE_DELTA"
 *   - CommandCode: 44
 *   - Size: 6
 *   - Offset: 0
 *   - MSB: 5
 *   - MASK: 0x003F
 *   - Access: R/W
 *   - Default: 42
 */
//!@{
#define LTC4015_VEQUALIZE_DELTA_BF_SUBADDR LTC4015_VEQUALIZE_DELTA_SUBADDR //!< @ref LTC4015_VEQUALIZE_DELTA_BF "VEQUALIZE_DELTA_BF"
#define LTC4015_VEQUALIZE_DELTA_BF_SIZE 6
#define LTC4015_VEQUALIZE_DELTA_BF_OFFSET 0
#define LTC4015_VEQUALIZE_DELTA_BF_MASK 0x003F
#define LTC4015_VEQUALIZE_DELTA_BF (LTC4015_VEQUALIZE_DELTA_BF_OFFSET << 12 | (LTC4015_VEQUALIZE_DELTA_BF_SIZE - 1) << 8 | LTC4015_VEQUALIZE_DELTA_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_EQUALIZE_TIME EQUALIZE_TIME
 *  @ingroup LTC4015_register_map
 *  @brief EQUALIZE_TIME Register
 *
 * |             15:0 |
 * |:----------------:|
 * | EQUALIZE_TIME_BF |
 *
 *   - CommandCode: 45
 *   - Contains Bit Fields:
 *     + @ref LTC4015_EQUALIZE_TIME_BF "EQUALIZE_TIME_BF" : Lead acid equalization time
*/

//!@{
#define LTC4015_EQUALIZE_TIME_SUBADDR 45
#define LTC4015_EQUALIZE_TIME (0 << 12 | (16 - 1) << 8 | LTC4015_EQUALIZE_TIME_SUBADDR)
//!@}
/*! @defgroup LTC4015_EQUALIZE_TIME_BF EQUALIZE_TIME_BF
 *  @ingroup LTC4015_register_map
 *  @brief EQUALIZE_TIME_BF Bit Field
 *
 *  Lead acid equalization time
 *   - Register: @ref LTC4015_EQUALIZE_TIME "EQUALIZE_TIME"
 *   - CommandCode: 45
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 3600
 */
//!@{
#define LTC4015_EQUALIZE_TIME_BF_SUBADDR LTC4015_EQUALIZE_TIME_SUBADDR //!< @ref LTC4015_EQUALIZE_TIME_BF "EQUALIZE_TIME_BF"
#define LTC4015_EQUALIZE_TIME_BF_SIZE 16
#define LTC4015_EQUALIZE_TIME_BF_OFFSET 0
#define LTC4015_EQUALIZE_TIME_BF_MASK 0xFFFF
#define LTC4015_EQUALIZE_TIME_BF (LTC4015_EQUALIZE_TIME_BF_OFFSET << 12 | (LTC4015_EQUALIZE_TIME_BF_SIZE - 1) << 8 | LTC4015_EQUALIZE_TIME_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_LIFEPO4_RECHARGE_THRESHOLD LIFEPO4_RECHARGE_THRESHOLD
 *  @ingroup LTC4015_register_map
 *  @brief LIFEPO4_RECHARGE_THRESHOLD Register
 *
 * |                          15:0 |
 * |:-----------------------------:|
 * | LIFEPO4_RECHARGE_THRESHOLD_BF |
 *
 *   - CommandCode: 46
 *   - Contains Bit Fields:
 *     + @ref LTC4015_LIFEPO4_RECHARGE_THRESHOLD_BF "LIFEPO4_RECHARGE_THRESHOLD_BF" : LIFEPO4_RECHARGE_THRESHOLD
*/

//!@{
#define LTC4015_LIFEPO4_RECHARGE_THRESHOLD_SUBADDR 46
#define LTC4015_LIFEPO4_RECHARGE_THRESHOLD (0 << 12 | (16 - 1) << 8 | LTC4015_LIFEPO4_RECHARGE_THRESHOLD_SUBADDR)
//!@}
/*! @defgroup LTC4015_LIFEPO4_RECHARGE_THRESHOLD_BF LIFEPO4_RECHARGE_THRESHOLD_BF
 *  @ingroup LTC4015_register_map
 *  @brief LIFEPO4_RECHARGE_THRESHOLD_BF Bit Field
 *
 *  LIFEPO4_RECHARGE_THRESHOLD
 *   - Register: @ref LTC4015_LIFEPO4_RECHARGE_THRESHOLD "LIFEPO4_RECHARGE_THRESHOLD"
 *   - CommandCode: 46
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 17424
 */
//!@{
#define LTC4015_LIFEPO4_RECHARGE_THRESHOLD_BF_SUBADDR LTC4015_LIFEPO4_RECHARGE_THRESHOLD_SUBADDR //!< @ref LTC4015_LIFEPO4_RECHARGE_THRESHOLD_BF "LIFEPO4_RECHARGE_THRESHOLD_BF"
#define LTC4015_LIFEPO4_RECHARGE_THRESHOLD_BF_SIZE 16
#define LTC4015_LIFEPO4_RECHARGE_THRESHOLD_BF_OFFSET 0
#define LTC4015_LIFEPO4_RECHARGE_THRESHOLD_BF_MASK 0xFFFF
#define LTC4015_LIFEPO4_RECHARGE_THRESHOLD_BF (LTC4015_LIFEPO4_RECHARGE_THRESHOLD_BF_OFFSET << 12 | (LTC4015_LIFEPO4_RECHARGE_THRESHOLD_BF_SIZE - 1) << 8 | LTC4015_LIFEPO4_RECHARGE_THRESHOLD_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_RESERVED_0X2F RESERVED_0X2F
 *  @ingroup LTC4015_register_map
 *  @brief RESERVED_0X2F Register
 *
 * |             15:0 |
 * |:----------------:|
 * | RESERVED_0X2F_BF |
 *
 *   - CommandCode: 47
 *   - Contains Bit Fields:
 *     + @ref LTC4015_RESERVED_0X2F_BF "RESERVED_0X2F_BF" : RESERVED
*/

//!@{
#define LTC4015_RESERVED_0X2F_SUBADDR 47
#define LTC4015_RESERVED_0X2F (0 << 12 | (16 - 1) << 8 | LTC4015_RESERVED_0X2F_SUBADDR)
//!@}
/*! @defgroup LTC4015_RESERVED_0X2F_BF RESERVED_0X2F_BF
 *  @ingroup LTC4015_register_map
 *  @brief RESERVED_0X2F_BF Bit Field
 *
 *  RESERVED
 *   - Register: @ref LTC4015_RESERVED_0X2F "RESERVED_0X2F"
 *   - CommandCode: 47
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R/W
 *   - Default: 0
 */
//!@{
#define LTC4015_RESERVED_0X2F_BF_SUBADDR LTC4015_RESERVED_0X2F_SUBADDR //!< @ref LTC4015_RESERVED_0X2F_BF "RESERVED_0X2F_BF"
#define LTC4015_RESERVED_0X2F_BF_SIZE 16
#define LTC4015_RESERVED_0X2F_BF_OFFSET 0
#define LTC4015_RESERVED_0X2F_BF_MASK 0xFFFF
#define LTC4015_RESERVED_0X2F_BF (LTC4015_RESERVED_0X2F_BF_OFFSET << 12 | (LTC4015_RESERVED_0X2F_BF_SIZE - 1) << 8 | LTC4015_RESERVED_0X2F_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_MAX_CHARGE_TIMER MAX_CHARGE_TIMER
 *  @ingroup LTC4015_register_map
 *  @brief MAX_CHARGE_TIMER Register
 *
 * |                15:0 |
 * |:-------------------:|
 * | MAX_CHARGE_TIMER_BF |
 *
 *   - CommandCode: 48
 *   - Contains Bit Fields:
 *     + @ref LTC4015_MAX_CHARGE_TIMER_BF "MAX_CHARGE_TIMER_BF" : For lithium chemistries, indicates the time (in sec) that the battery has been charging
*/

//!@{
#define LTC4015_MAX_CHARGE_TIMER_SUBADDR 48
#define LTC4015_MAX_CHARGE_TIMER (0 << 12 | (16 - 1) << 8 | LTC4015_MAX_CHARGE_TIMER_SUBADDR)
//!@}
/*! @defgroup LTC4015_MAX_CHARGE_TIMER_BF MAX_CHARGE_TIMER_BF
 *  @ingroup LTC4015_register_map
 *  @brief MAX_CHARGE_TIMER_BF Bit Field
 *
 *  For lithium chemistries, indicates the time (in sec) that the battery has been charging
 *   - Register: @ref LTC4015_MAX_CHARGE_TIMER "MAX_CHARGE_TIMER"
 *   - CommandCode: 48
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_MAX_CHARGE_TIMER_BF_SUBADDR LTC4015_MAX_CHARGE_TIMER_SUBADDR //!< @ref LTC4015_MAX_CHARGE_TIMER_BF "MAX_CHARGE_TIMER_BF"
#define LTC4015_MAX_CHARGE_TIMER_BF_SIZE 16
#define LTC4015_MAX_CHARGE_TIMER_BF_OFFSET 0
#define LTC4015_MAX_CHARGE_TIMER_BF_MASK 0xFFFF
#define LTC4015_MAX_CHARGE_TIMER_BF (LTC4015_MAX_CHARGE_TIMER_BF_OFFSET << 12 | (LTC4015_MAX_CHARGE_TIMER_BF_SIZE - 1) << 8 | LTC4015_MAX_CHARGE_TIMER_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_CV_TIMER CV_TIMER
 *  @ingroup LTC4015_register_map
 *  @brief CV_TIMER Register
 *
 * |        15:0 |
 * |:-----------:|
 * | CV_TIMER_BF |
 *
 *   - CommandCode: 49
 *   - Contains Bit Fields:
 *     + @ref LTC4015_CV_TIMER_BF "CV_TIMER_BF" : For lithium chemistries, indicates the time (in sec) that the battery has been in constant-voltage regulation
*/

//!@{
#define LTC4015_CV_TIMER_SUBADDR 49
#define LTC4015_CV_TIMER (0 << 12 | (16 - 1) << 8 | LTC4015_CV_TIMER_SUBADDR)
//!@}
/*! @defgroup LTC4015_CV_TIMER_BF CV_TIMER_BF
 *  @ingroup LTC4015_register_map
 *  @brief CV_TIMER_BF Bit Field
 *
 *  For lithium chemistries, indicates the time (in sec) that the battery has been in constant-voltage regulation
 *   - Register: @ref LTC4015_CV_TIMER "CV_TIMER"
 *   - CommandCode: 49
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_CV_TIMER_BF_SUBADDR LTC4015_CV_TIMER_SUBADDR //!< @ref LTC4015_CV_TIMER_BF "CV_TIMER_BF"
#define LTC4015_CV_TIMER_BF_SIZE 16
#define LTC4015_CV_TIMER_BF_OFFSET 0
#define LTC4015_CV_TIMER_BF_MASK 0xFFFF
#define LTC4015_CV_TIMER_BF (LTC4015_CV_TIMER_BF_OFFSET << 12 | (LTC4015_CV_TIMER_BF_SIZE - 1) << 8 | LTC4015_CV_TIMER_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_ABSORB_TIMER ABSORB_TIMER
 *  @ingroup LTC4015_register_map
 *  @brief ABSORB_TIMER Register
 *
 * |            15:0 |
 * |:---------------:|
 * | ABSORB_TIMER_BF |
 *
 *   - CommandCode: 50
 *   - Contains Bit Fields:
 *     + @ref LTC4015_ABSORB_TIMER_BF "ABSORB_TIMER_BF" : For LiFePO4 and lead-acid batteries, indicates the time (in sec) that the battery has been in absorb phase
*/

//!@{
#define LTC4015_ABSORB_TIMER_SUBADDR 50
#define LTC4015_ABSORB_TIMER (0 << 12 | (16 - 1) << 8 | LTC4015_ABSORB_TIMER_SUBADDR)
//!@}
/*! @defgroup LTC4015_ABSORB_TIMER_BF ABSORB_TIMER_BF
 *  @ingroup LTC4015_register_map
 *  @brief ABSORB_TIMER_BF Bit Field
 *
 *  For LiFePO4 and lead-acid batteries, indicates the time (in sec) that the battery has been in absorb phase
 *   - Register: @ref LTC4015_ABSORB_TIMER "ABSORB_TIMER"
 *   - CommandCode: 50
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_ABSORB_TIMER_BF_SUBADDR LTC4015_ABSORB_TIMER_SUBADDR //!< @ref LTC4015_ABSORB_TIMER_BF "ABSORB_TIMER_BF"
#define LTC4015_ABSORB_TIMER_BF_SIZE 16
#define LTC4015_ABSORB_TIMER_BF_OFFSET 0
#define LTC4015_ABSORB_TIMER_BF_MASK 0xFFFF
#define LTC4015_ABSORB_TIMER_BF (LTC4015_ABSORB_TIMER_BF_OFFSET << 12 | (LTC4015_ABSORB_TIMER_BF_SIZE - 1) << 8 | LTC4015_ABSORB_TIMER_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_EQUALIZE_TIMER EQUALIZE_TIMER
 *  @ingroup LTC4015_register_map
 *  @brief EQUALIZE_TIMER Register
 *
 * |              15:0 |
 * |:-----------------:|
 * | EQUALIZE_TIMER_BF |
 *
 *   - CommandCode: 51
 *   - Contains Bit Fields:
 *     + @ref LTC4015_EQUALIZE_TIMER_BF "EQUALIZE_TIMER_BF" : For lead-acid batteries, indicates the time (in sec) that the battery has been in EQUALIZE phase
*/

//!@{
#define LTC4015_EQUALIZE_TIMER_SUBADDR 51
#define LTC4015_EQUALIZE_TIMER (0 << 12 | (16 - 1) << 8 | LTC4015_EQUALIZE_TIMER_SUBADDR)
//!@}
/*! @defgroup LTC4015_EQUALIZE_TIMER_BF EQUALIZE_TIMER_BF
 *  @ingroup LTC4015_register_map
 *  @brief EQUALIZE_TIMER_BF Bit Field
 *
 *  For lead-acid batteries, indicates the time (in sec) that the battery has been in EQUALIZE phase
 *   - Register: @ref LTC4015_EQUALIZE_TIMER "EQUALIZE_TIMER"
 *   - CommandCode: 51
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_EQUALIZE_TIMER_BF_SUBADDR LTC4015_EQUALIZE_TIMER_SUBADDR //!< @ref LTC4015_EQUALIZE_TIMER_BF "EQUALIZE_TIMER_BF"
#define LTC4015_EQUALIZE_TIMER_BF_SIZE 16
#define LTC4015_EQUALIZE_TIMER_BF_OFFSET 0
#define LTC4015_EQUALIZE_TIMER_BF_MASK 0xFFFF
#define LTC4015_EQUALIZE_TIMER_BF (LTC4015_EQUALIZE_TIMER_BF_OFFSET << 12 | (LTC4015_EQUALIZE_TIMER_BF_SIZE - 1) << 8 | LTC4015_EQUALIZE_TIMER_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_CHARGER_STATE CHARGER_STATE
 *  @ingroup LTC4015_register_map
 *  @brief CHARGER_STATE Register
 *
 * | 15:11 |                 10 |                9 |                    8 |            7 |               6 |            5 |             4 |                3 |                        2 |                    1 |                  0 |
 * |:-----:|:------------------:|:----------------:|:--------------------:|:------------:|:---------------:|:------------:|:-------------:|:----------------:|:------------------------:|:--------------------:|:------------------:|
 * |   n/a | EQUALIZE_CHARGE_BF | ABSORB_CHARGE_BF | CHARGER_SUSPENDED_BF | PRECHARGE_BF | CC_CV_CHARGE_BF | NTC_PAUSE_BF | TIMER_TERM_BF | C_OVER_X_TERM_BF | MAX_CHARGE_TIME_FAULT_BF | BAT_MISSING_FAULT_BF | BAT_SHORT_FAULT_BF |
 *
 * Real time battery charger state indicator. Individual bits are mutually exclusive. Bits 15:11 are reserved.
 *   - CommandCode: 52
 *   - Contains Bit Fields:
 *     + @ref LTC4015_EQUALIZE_CHARGE_BF "EQUALIZE_CHARGE_BF" : Indicates battery charger is in lead-acid equalization charge state
 *     + @ref LTC4015_ABSORB_CHARGE_BF "ABSORB_CHARGE_BF" : Indicates battery charger is in absorb charge state
 *     + @ref LTC4015_CHARGER_SUSPENDED_BF "CHARGER_SUSPENDED_BF" : Indicates battery charger is in charger suspended state
 *     + @ref LTC4015_PRECHARGE_BF "PRECHARGE_BF" : Indicates battery charger is in precondition charge state
 *     + @ref LTC4015_CC_CV_CHARGE_BF "CC_CV_CHARGE_BF" : Indicates battery charger is in CC-CV state
 *     + @ref LTC4015_NTC_PAUSE_BF "NTC_PAUSE_BF" : Indicates battery charger is in thermistor pause state
 *     + @ref LTC4015_TIMER_TERM_BF "TIMER_TERM_BF" : Indicates battery charger is in timer termination state
 *     + @ref LTC4015_C_OVER_X_TERM_BF "C_OVER_X_TERM_BF" : Indicates battery charger is in C/x termination state
 *     + @ref LTC4015_MAX_CHARGE_TIME_FAULT_BF "MAX_CHARGE_TIME_FAULT_BF" : indicates battery charger is in max_charge_time_fault state
 *     + @ref LTC4015_BAT_MISSING_FAULT_BF "BAT_MISSING_FAULT_BF" : Indicates battery charger is in missing battery fault state
 *     + @ref LTC4015_BAT_SHORT_FAULT_BF "BAT_SHORT_FAULT_BF" : Indicates battery charger is in shorted battery fault state
*/

//!@{
#define LTC4015_CHARGER_STATE_SUBADDR 52
#define LTC4015_CHARGER_STATE (0 << 12 | (16 - 1) << 8 | LTC4015_CHARGER_STATE_SUBADDR)
//!@}
/*! @defgroup LTC4015_EQUALIZE_CHARGE_BF EQUALIZE_CHARGE_BF
 *  @ingroup LTC4015_register_map
 *  @brief EQUALIZE_CHARGE_BF Bit Field
 *
 *  Indicates battery charger is in lead-acid equalization charge state
 *   - Register: @ref LTC4015_CHARGER_STATE "CHARGER_STATE"
 *   - CommandCode: 52
 *   - Size: 1
 *   - Offset: 10
 *   - MSB: 10
 *   - MASK: 0x0400
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_EQUALIZE_CHARGE_BF_SUBADDR LTC4015_CHARGER_STATE_SUBADDR //!< @ref LTC4015_EQUALIZE_CHARGE_BF "EQUALIZE_CHARGE_BF"
#define LTC4015_EQUALIZE_CHARGE_BF_SIZE 1
#define LTC4015_EQUALIZE_CHARGE_BF_OFFSET 10
#define LTC4015_EQUALIZE_CHARGE_BF_MASK 0x0400
#define LTC4015_EQUALIZE_CHARGE_BF (LTC4015_EQUALIZE_CHARGE_BF_OFFSET << 12 | (LTC4015_EQUALIZE_CHARGE_BF_SIZE - 1) << 8 | LTC4015_EQUALIZE_CHARGE_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_ABSORB_CHARGE_BF ABSORB_CHARGE_BF
 *  @ingroup LTC4015_register_map
 *  @brief ABSORB_CHARGE_BF Bit Field
 *
 *  Indicates battery charger is in absorb charge state
 *   - Register: @ref LTC4015_CHARGER_STATE "CHARGER_STATE"
 *   - CommandCode: 52
 *   - Size: 1
 *   - Offset: 9
 *   - MSB: 9
 *   - MASK: 0x0200
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_ABSORB_CHARGE_BF_SUBADDR LTC4015_CHARGER_STATE_SUBADDR //!< @ref LTC4015_ABSORB_CHARGE_BF "ABSORB_CHARGE_BF"
#define LTC4015_ABSORB_CHARGE_BF_SIZE 1
#define LTC4015_ABSORB_CHARGE_BF_OFFSET 9
#define LTC4015_ABSORB_CHARGE_BF_MASK 0x0200
#define LTC4015_ABSORB_CHARGE_BF (LTC4015_ABSORB_CHARGE_BF_OFFSET << 12 | (LTC4015_ABSORB_CHARGE_BF_SIZE - 1) << 8 | LTC4015_ABSORB_CHARGE_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_CHARGER_SUSPENDED_BF CHARGER_SUSPENDED_BF
 *  @ingroup LTC4015_register_map
 *  @brief CHARGER_SUSPENDED_BF Bit Field
 *
 *  Indicates battery charger is in charger suspended state
 *   - Register: @ref LTC4015_CHARGER_STATE "CHARGER_STATE"
 *   - CommandCode: 52
 *   - Size: 1
 *   - Offset: 8
 *   - MSB: 8
 *   - MASK: 0x0100
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_CHARGER_SUSPENDED_BF_SUBADDR LTC4015_CHARGER_STATE_SUBADDR //!< @ref LTC4015_CHARGER_SUSPENDED_BF "CHARGER_SUSPENDED_BF"
#define LTC4015_CHARGER_SUSPENDED_BF_SIZE 1
#define LTC4015_CHARGER_SUSPENDED_BF_OFFSET 8
#define LTC4015_CHARGER_SUSPENDED_BF_MASK 0x0100
#define LTC4015_CHARGER_SUSPENDED_BF (LTC4015_CHARGER_SUSPENDED_BF_OFFSET << 12 | (LTC4015_CHARGER_SUSPENDED_BF_SIZE - 1) << 8 | LTC4015_CHARGER_SUSPENDED_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_PRECHARGE_BF PRECHARGE_BF
 *  @ingroup LTC4015_register_map
 *  @brief PRECHARGE_BF Bit Field
 *
 *  Indicates battery charger is in precondition charge state
 *   - Register: @ref LTC4015_CHARGER_STATE "CHARGER_STATE"
 *   - CommandCode: 52
 *   - Size: 1
 *   - Offset: 7
 *   - MSB: 7
 *   - MASK: 0x0080
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_PRECHARGE_BF_SUBADDR LTC4015_CHARGER_STATE_SUBADDR //!< @ref LTC4015_PRECHARGE_BF "PRECHARGE_BF"
#define LTC4015_PRECHARGE_BF_SIZE 1
#define LTC4015_PRECHARGE_BF_OFFSET 7
#define LTC4015_PRECHARGE_BF_MASK 0x0080
#define LTC4015_PRECHARGE_BF (LTC4015_PRECHARGE_BF_OFFSET << 12 | (LTC4015_PRECHARGE_BF_SIZE - 1) << 8 | LTC4015_PRECHARGE_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_CC_CV_CHARGE_BF CC_CV_CHARGE_BF
 *  @ingroup LTC4015_register_map
 *  @brief CC_CV_CHARGE_BF Bit Field
 *
 *  Indicates battery charger is in CC-CV state
 *   - Register: @ref LTC4015_CHARGER_STATE "CHARGER_STATE"
 *   - CommandCode: 52
 *   - Size: 1
 *   - Offset: 6
 *   - MSB: 6
 *   - MASK: 0x0040
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_CC_CV_CHARGE_BF_SUBADDR LTC4015_CHARGER_STATE_SUBADDR //!< @ref LTC4015_CC_CV_CHARGE_BF "CC_CV_CHARGE_BF"
#define LTC4015_CC_CV_CHARGE_BF_SIZE 1
#define LTC4015_CC_CV_CHARGE_BF_OFFSET 6
#define LTC4015_CC_CV_CHARGE_BF_MASK 0x0040
#define LTC4015_CC_CV_CHARGE_BF (LTC4015_CC_CV_CHARGE_BF_OFFSET << 12 | (LTC4015_CC_CV_CHARGE_BF_SIZE - 1) << 8 | LTC4015_CC_CV_CHARGE_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_NTC_PAUSE_BF NTC_PAUSE_BF
 *  @ingroup LTC4015_register_map
 *  @brief NTC_PAUSE_BF Bit Field
 *
 *  Indicates battery charger is in thermistor pause state
 *   - Register: @ref LTC4015_CHARGER_STATE "CHARGER_STATE"
 *   - CommandCode: 52
 *   - Size: 1
 *   - Offset: 5
 *   - MSB: 5
 *   - MASK: 0x0020
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_NTC_PAUSE_BF_SUBADDR LTC4015_CHARGER_STATE_SUBADDR //!< @ref LTC4015_NTC_PAUSE_BF "NTC_PAUSE_BF"
#define LTC4015_NTC_PAUSE_BF_SIZE 1
#define LTC4015_NTC_PAUSE_BF_OFFSET 5
#define LTC4015_NTC_PAUSE_BF_MASK 0x0020
#define LTC4015_NTC_PAUSE_BF (LTC4015_NTC_PAUSE_BF_OFFSET << 12 | (LTC4015_NTC_PAUSE_BF_SIZE - 1) << 8 | LTC4015_NTC_PAUSE_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_TIMER_TERM_BF TIMER_TERM_BF
 *  @ingroup LTC4015_register_map
 *  @brief TIMER_TERM_BF Bit Field
 *
 *  Indicates battery charger is in timer termination state
 *   - Register: @ref LTC4015_CHARGER_STATE "CHARGER_STATE"
 *   - CommandCode: 52
 *   - Size: 1
 *   - Offset: 4
 *   - MSB: 4
 *   - MASK: 0x0010
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_TIMER_TERM_BF_SUBADDR LTC4015_CHARGER_STATE_SUBADDR //!< @ref LTC4015_TIMER_TERM_BF "TIMER_TERM_BF"
#define LTC4015_TIMER_TERM_BF_SIZE 1
#define LTC4015_TIMER_TERM_BF_OFFSET 4
#define LTC4015_TIMER_TERM_BF_MASK 0x0010
#define LTC4015_TIMER_TERM_BF (LTC4015_TIMER_TERM_BF_OFFSET << 12 | (LTC4015_TIMER_TERM_BF_SIZE - 1) << 8 | LTC4015_TIMER_TERM_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_C_OVER_X_TERM_BF C_OVER_X_TERM_BF
 *  @ingroup LTC4015_register_map
 *  @brief C_OVER_X_TERM_BF Bit Field
 *
 *  Indicates battery charger is in C/x termination state
 *   - Register: @ref LTC4015_CHARGER_STATE "CHARGER_STATE"
 *   - CommandCode: 52
 *   - Size: 1
 *   - Offset: 3
 *   - MSB: 3
 *   - MASK: 0x0008
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_C_OVER_X_TERM_BF_SUBADDR LTC4015_CHARGER_STATE_SUBADDR //!< @ref LTC4015_C_OVER_X_TERM_BF "C_OVER_X_TERM_BF"
#define LTC4015_C_OVER_X_TERM_BF_SIZE 1
#define LTC4015_C_OVER_X_TERM_BF_OFFSET 3
#define LTC4015_C_OVER_X_TERM_BF_MASK 0x0008
#define LTC4015_C_OVER_X_TERM_BF (LTC4015_C_OVER_X_TERM_BF_OFFSET << 12 | (LTC4015_C_OVER_X_TERM_BF_SIZE - 1) << 8 | LTC4015_C_OVER_X_TERM_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_MAX_CHARGE_TIME_FAULT_BF MAX_CHARGE_TIME_FAULT_BF
 *  @ingroup LTC4015_register_map
 *  @brief MAX_CHARGE_TIME_FAULT_BF Bit Field
 *
 *  indicates battery charger is in max_charge_time_fault state
 *   - Register: @ref LTC4015_CHARGER_STATE "CHARGER_STATE"
 *   - CommandCode: 52
 *   - Size: 1
 *   - Offset: 2
 *   - MSB: 2
 *   - MASK: 0x0004
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_MAX_CHARGE_TIME_FAULT_BF_SUBADDR LTC4015_CHARGER_STATE_SUBADDR //!< @ref LTC4015_MAX_CHARGE_TIME_FAULT_BF "MAX_CHARGE_TIME_FAULT_BF"
#define LTC4015_MAX_CHARGE_TIME_FAULT_BF_SIZE 1
#define LTC4015_MAX_CHARGE_TIME_FAULT_BF_OFFSET 2
#define LTC4015_MAX_CHARGE_TIME_FAULT_BF_MASK 0x0004
#define LTC4015_MAX_CHARGE_TIME_FAULT_BF (LTC4015_MAX_CHARGE_TIME_FAULT_BF_OFFSET << 12 | (LTC4015_MAX_CHARGE_TIME_FAULT_BF_SIZE - 1) << 8 | LTC4015_MAX_CHARGE_TIME_FAULT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_BAT_MISSING_FAULT_BF BAT_MISSING_FAULT_BF
 *  @ingroup LTC4015_register_map
 *  @brief BAT_MISSING_FAULT_BF Bit Field
 *
 *  Indicates battery charger is in missing battery fault state
 *   - Register: @ref LTC4015_CHARGER_STATE "CHARGER_STATE"
 *   - CommandCode: 52
 *   - Size: 1
 *   - Offset: 1
 *   - MSB: 1
 *   - MASK: 0x0002
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_BAT_MISSING_FAULT_BF_SUBADDR LTC4015_CHARGER_STATE_SUBADDR //!< @ref LTC4015_BAT_MISSING_FAULT_BF "BAT_MISSING_FAULT_BF"
#define LTC4015_BAT_MISSING_FAULT_BF_SIZE 1
#define LTC4015_BAT_MISSING_FAULT_BF_OFFSET 1
#define LTC4015_BAT_MISSING_FAULT_BF_MASK 0x0002
#define LTC4015_BAT_MISSING_FAULT_BF (LTC4015_BAT_MISSING_FAULT_BF_OFFSET << 12 | (LTC4015_BAT_MISSING_FAULT_BF_SIZE - 1) << 8 | LTC4015_BAT_MISSING_FAULT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_BAT_SHORT_FAULT_BF BAT_SHORT_FAULT_BF
 *  @ingroup LTC4015_register_map
 *  @brief BAT_SHORT_FAULT_BF Bit Field
 *
 *  Indicates battery charger is in shorted battery fault state
 *   - Register: @ref LTC4015_CHARGER_STATE "CHARGER_STATE"
 *   - CommandCode: 52
 *   - Size: 1
 *   - Offset: 0
 *   - MSB: 0
 *   - MASK: 0x0001
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_BAT_SHORT_FAULT_BF_SUBADDR LTC4015_CHARGER_STATE_SUBADDR //!< @ref LTC4015_BAT_SHORT_FAULT_BF "BAT_SHORT_FAULT_BF"
#define LTC4015_BAT_SHORT_FAULT_BF_SIZE 1
#define LTC4015_BAT_SHORT_FAULT_BF_OFFSET 0
#define LTC4015_BAT_SHORT_FAULT_BF_MASK 0x0001
#define LTC4015_BAT_SHORT_FAULT_BF (LTC4015_BAT_SHORT_FAULT_BF_OFFSET << 12 | (LTC4015_BAT_SHORT_FAULT_BF_SIZE - 1) << 8 | LTC4015_BAT_SHORT_FAULT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_CHARGE_STATUS CHARGE_STATUS
 *  @ingroup LTC4015_register_map
 *  @brief CHARGE_STATUS Register
 *
 * | 15:4 |                  3 |                   2 |                   1 |                   0 |
 * |:----:|:------------------:|:-------------------:|:-------------------:|:-------------------:|
 * |  n/a | VIN_UVCL_ACTIVE_BF | IIN_LIMIT_ACTIVE_BF | CONSTANT_CURRENT_BF | CONSTANT_VOLTAGE_BF |
 *
 * Charge status indicator. Individual bits are mutually exclusive. Only active in charging states.
 *   - CommandCode: 53
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VIN_UVCL_ACTIVE_BF "VIN_UVCL_ACTIVE_BF" : Indicates the input undervoltage control loop is actively controlling power delivery based on VIN_UVCL_SETTING
 *     + @ref LTC4015_IIN_LIMIT_ACTIVE_BF "IIN_LIMIT_ACTIVE_BF" : Indicates the input current limit control loop is actively controlling power delivery based on IIN_LIMIT[_DAC][_SETTING]
 *     + @ref LTC4015_CONSTANT_CURRENT_BF "CONSTANT_CURRENT_BF" : Indicates the charge current control loop is actively controlling power delivery based on ICHARGE_DAC
 *     + @ref LTC4015_CONSTANT_VOLTAGE_BF "CONSTANT_VOLTAGE_BF" : Indicates the battery voltage control loop is actively controlling power delivery based on VCHARGE_DAC
*/

//!@{
#define LTC4015_CHARGE_STATUS_SUBADDR 53
#define LTC4015_CHARGE_STATUS (0 << 12 | (16 - 1) << 8 | LTC4015_CHARGE_STATUS_SUBADDR)
//!@}
/*! @defgroup LTC4015_VIN_UVCL_ACTIVE_BF VIN_UVCL_ACTIVE_BF
 *  @ingroup LTC4015_register_map
 *  @brief VIN_UVCL_ACTIVE_BF Bit Field
 *
 *  Indicates the input undervoltage control loop is actively controlling power delivery based on VIN_UVCL_SETTING
 *   - Register: @ref LTC4015_CHARGE_STATUS "CHARGE_STATUS"
 *   - CommandCode: 53
 *   - Size: 1
 *   - Offset: 3
 *   - MSB: 3
 *   - MASK: 0x0008
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VIN_UVCL_ACTIVE_BF_SUBADDR LTC4015_CHARGE_STATUS_SUBADDR //!< @ref LTC4015_VIN_UVCL_ACTIVE_BF "VIN_UVCL_ACTIVE_BF"
#define LTC4015_VIN_UVCL_ACTIVE_BF_SIZE 1
#define LTC4015_VIN_UVCL_ACTIVE_BF_OFFSET 3
#define LTC4015_VIN_UVCL_ACTIVE_BF_MASK 0x0008
#define LTC4015_VIN_UVCL_ACTIVE_BF (LTC4015_VIN_UVCL_ACTIVE_BF_OFFSET << 12 | (LTC4015_VIN_UVCL_ACTIVE_BF_SIZE - 1) << 8 | LTC4015_VIN_UVCL_ACTIVE_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_IIN_LIMIT_ACTIVE_BF IIN_LIMIT_ACTIVE_BF
 *  @ingroup LTC4015_register_map
 *  @brief IIN_LIMIT_ACTIVE_BF Bit Field
 *
 *  Indicates the input current limit control loop is actively controlling power delivery based on IIN_LIMIT[_DAC][_SETTING]
 *   - Register: @ref LTC4015_CHARGE_STATUS "CHARGE_STATUS"
 *   - CommandCode: 53
 *   - Size: 1
 *   - Offset: 2
 *   - MSB: 2
 *   - MASK: 0x0004
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_IIN_LIMIT_ACTIVE_BF_SUBADDR LTC4015_CHARGE_STATUS_SUBADDR //!< @ref LTC4015_IIN_LIMIT_ACTIVE_BF "IIN_LIMIT_ACTIVE_BF"
#define LTC4015_IIN_LIMIT_ACTIVE_BF_SIZE 1
#define LTC4015_IIN_LIMIT_ACTIVE_BF_OFFSET 2
#define LTC4015_IIN_LIMIT_ACTIVE_BF_MASK 0x0004
#define LTC4015_IIN_LIMIT_ACTIVE_BF (LTC4015_IIN_LIMIT_ACTIVE_BF_OFFSET << 12 | (LTC4015_IIN_LIMIT_ACTIVE_BF_SIZE - 1) << 8 | LTC4015_IIN_LIMIT_ACTIVE_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_CONSTANT_CURRENT_BF CONSTANT_CURRENT_BF
 *  @ingroup LTC4015_register_map
 *  @brief CONSTANT_CURRENT_BF Bit Field
 *
 *  Indicates the charge current control loop is actively controlling power delivery based on ICHARGE_DAC
 *   - Register: @ref LTC4015_CHARGE_STATUS "CHARGE_STATUS"
 *   - CommandCode: 53
 *   - Size: 1
 *   - Offset: 1
 *   - MSB: 1
 *   - MASK: 0x0002
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_CONSTANT_CURRENT_BF_SUBADDR LTC4015_CHARGE_STATUS_SUBADDR //!< @ref LTC4015_CONSTANT_CURRENT_BF "CONSTANT_CURRENT_BF"
#define LTC4015_CONSTANT_CURRENT_BF_SIZE 1
#define LTC4015_CONSTANT_CURRENT_BF_OFFSET 1
#define LTC4015_CONSTANT_CURRENT_BF_MASK 0x0002
#define LTC4015_CONSTANT_CURRENT_BF (LTC4015_CONSTANT_CURRENT_BF_OFFSET << 12 | (LTC4015_CONSTANT_CURRENT_BF_SIZE - 1) << 8 | LTC4015_CONSTANT_CURRENT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_CONSTANT_VOLTAGE_BF CONSTANT_VOLTAGE_BF
 *  @ingroup LTC4015_register_map
 *  @brief CONSTANT_VOLTAGE_BF Bit Field
 *
 *  Indicates the battery voltage control loop is actively controlling power delivery based on VCHARGE_DAC
 *   - Register: @ref LTC4015_CHARGE_STATUS "CHARGE_STATUS"
 *   - CommandCode: 53
 *   - Size: 1
 *   - Offset: 0
 *   - MSB: 0
 *   - MASK: 0x0001
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_CONSTANT_VOLTAGE_BF_SUBADDR LTC4015_CHARGE_STATUS_SUBADDR //!< @ref LTC4015_CONSTANT_VOLTAGE_BF "CONSTANT_VOLTAGE_BF"
#define LTC4015_CONSTANT_VOLTAGE_BF_SIZE 1
#define LTC4015_CONSTANT_VOLTAGE_BF_OFFSET 0
#define LTC4015_CONSTANT_VOLTAGE_BF_MASK 0x0001
#define LTC4015_CONSTANT_VOLTAGE_BF (LTC4015_CONSTANT_VOLTAGE_BF_OFFSET << 12 | (LTC4015_CONSTANT_VOLTAGE_BF_SIZE - 1) << 8 | LTC4015_CONSTANT_VOLTAGE_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_LIMIT_ALERTS LIMIT_ALERTS
 *  @ingroup LTC4015_register_map
 *  @brief LIMIT_ALERTS Register
 *
 * |                      15 |  14 |                 13 |                 12 |               11 |               10 |               9 |               8 |                7 |                6 |               5 |                4 |                    3 |               2 |                     1 |                     0 |
 * |:-----------------------:|:---:|:------------------:|:------------------:|:----------------:|:----------------:|:---------------:|:---------------:|:----------------:|:----------------:|:---------------:|:----------------:|:--------------------:|:---------------:|:---------------------:|:---------------------:|
 * | MEAS_SYS_VALID_ALERT_BF | n/a | QCOUNT_LO_ALERT_BF | QCOUNT_HI_ALERT_BF | VBAT_LO_ALERT_BF | VBAT_HI_ALERT_BF | VIN_LO_ALERT_BF | VIN_HI_ALERT_BF | VSYS_LO_ALERT_BF | VSYS_HI_ALERT_BF | IIN_HI_ALERT_BF | IBAT_LO_ALERT_BF | DIE_TEMP_HI_ALERT_BF | BSR_HI_ALERT_BF | NTC_RATIO_HI_ALERT_BF | NTC_RATIO_LO_ALERT_BF |
 *
 * Limit alert register.<br />Individual bits are enabled by EN_LIMIT_ALERTS.<br />Writing 1 to any bit clears that alert.<br />Once set, alert bits remain high until cleared or disabled.
 *   - CommandCode: 54
 *   - Contains Bit Fields:
 *     + @ref LTC4015_MEAS_SYS_VALID_ALERT_BF "MEAS_SYS_VALID_ALERT_BF" : Indicates that measurement system results have become valid.
 *     + @ref LTC4015_QCOUNT_LO_ALERT_BF "QCOUNT_LO_ALERT_BF" : Indicates QCOUNT has fallen below QCOUNT_LO_ALERT_LIMIT
 *     + @ref LTC4015_QCOUNT_HI_ALERT_BF "QCOUNT_HI_ALERT_BF" : Indicates QCOUNT has exceeded QCOUNT_HI_ALERT_LIMIT
 *     + @ref LTC4015_VBAT_LO_ALERT_BF "VBAT_LO_ALERT_BF" : Indicates VBAT has fallen below VBAT_LO_ALERT_LIMIT
 *     + @ref LTC4015_VBAT_HI_ALERT_BF "VBAT_HI_ALERT_BF" : Indicates VBAT has exceeded VBAT_HI_ALERT_LIMIT
 *     + @ref LTC4015_VIN_LO_ALERT_BF "VIN_LO_ALERT_BF" : Indicates VIN has fallen below VIN_LO_ALERT_LIMIT
 *     + @ref LTC4015_VIN_HI_ALERT_BF "VIN_HI_ALERT_BF" : Indicates VIN has exceeded VIN_HI_ALERT_LIMIT
 *     + @ref LTC4015_VSYS_LO_ALERT_BF "VSYS_LO_ALERT_BF" : Indicates VSYS has fallen below VSYS_LO_ALERT_LIMIT
 *     + @ref LTC4015_VSYS_HI_ALERT_BF "VSYS_HI_ALERT_BF" : Indicates VSYS has exceeded VSYS_HI_ALERT_LIMIT
 *     + @ref LTC4015_IIN_HI_ALERT_BF "IIN_HI_ALERT_BF" : Indicates IIN has exceeded IIN_HI_ALERT_LIMIT
 *     + @ref LTC4015_IBAT_LO_ALERT_BF "IBAT_LO_ALERT_BF" : Indicates IBAT has fallen below IBAT_LO_ALERT_LIMIT
 *     + @ref LTC4015_DIE_TEMP_HI_ALERT_BF "DIE_TEMP_HI_ALERT_BF" : Indicates DIE_TEMP has exceeded DIE_TEMP_HI_ALERT_LIMIT
 *     + @ref LTC4015_BSR_HI_ALERT_BF "BSR_HI_ALERT_BF" : Indicates BSR has exceeded BSR_HI_ALERT_LIMIT
 *     + @ref LTC4015_NTC_RATIO_HI_ALERT_BF "NTC_RATIO_HI_ALERT_BF" : Indicates NTC_RATIO has exceeded NTC_RATIO_HI_ALERT_LIMIT
 *     + @ref LTC4015_NTC_RATIO_LO_ALERT_BF "NTC_RATIO_LO_ALERT_BF" : Indicates NTC_RATIO has fallen below NTC_RATIO_LO_ALERT_LIMIT
*/

//!@{
#define LTC4015_LIMIT_ALERTS_SUBADDR 54
#define LTC4015_LIMIT_ALERTS (0 << 12 | (16 - 1) << 8 | LTC4015_LIMIT_ALERTS_SUBADDR)
//!@}
/*! @defgroup LTC4015_MEAS_SYS_VALID_ALERT_BF MEAS_SYS_VALID_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief MEAS_SYS_VALID_ALERT_BF Bit Field
 *
 *  Indicates that measurement system results have become valid.
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 15
 *   - MSB: 15
 *   - MASK: 0x8000
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_MEAS_SYS_VALID_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_MEAS_SYS_VALID_ALERT_BF "MEAS_SYS_VALID_ALERT_BF"
#define LTC4015_MEAS_SYS_VALID_ALERT_BF_SIZE 1
#define LTC4015_MEAS_SYS_VALID_ALERT_BF_OFFSET 15
#define LTC4015_MEAS_SYS_VALID_ALERT_BF_MASK 0x8000
#define LTC4015_MEAS_SYS_VALID_ALERT_BF (LTC4015_MEAS_SYS_VALID_ALERT_BF_OFFSET << 12 | (LTC4015_MEAS_SYS_VALID_ALERT_BF_SIZE - 1) << 8 | LTC4015_MEAS_SYS_VALID_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_QCOUNT_LO_ALERT_BF QCOUNT_LO_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief QCOUNT_LO_ALERT_BF Bit Field
 *
 *  Indicates QCOUNT has fallen below QCOUNT_LO_ALERT_LIMIT
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 13
 *   - MSB: 13
 *   - MASK: 0x2000
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_QCOUNT_LO_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_QCOUNT_LO_ALERT_BF "QCOUNT_LO_ALERT_BF"
#define LTC4015_QCOUNT_LO_ALERT_BF_SIZE 1
#define LTC4015_QCOUNT_LO_ALERT_BF_OFFSET 13
#define LTC4015_QCOUNT_LO_ALERT_BF_MASK 0x2000
#define LTC4015_QCOUNT_LO_ALERT_BF (LTC4015_QCOUNT_LO_ALERT_BF_OFFSET << 12 | (LTC4015_QCOUNT_LO_ALERT_BF_SIZE - 1) << 8 | LTC4015_QCOUNT_LO_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_QCOUNT_HI_ALERT_BF QCOUNT_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief QCOUNT_HI_ALERT_BF Bit Field
 *
 *  Indicates QCOUNT has exceeded QCOUNT_HI_ALERT_LIMIT
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 12
 *   - MSB: 12
 *   - MASK: 0x1000
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_QCOUNT_HI_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_QCOUNT_HI_ALERT_BF "QCOUNT_HI_ALERT_BF"
#define LTC4015_QCOUNT_HI_ALERT_BF_SIZE 1
#define LTC4015_QCOUNT_HI_ALERT_BF_OFFSET 12
#define LTC4015_QCOUNT_HI_ALERT_BF_MASK 0x1000
#define LTC4015_QCOUNT_HI_ALERT_BF (LTC4015_QCOUNT_HI_ALERT_BF_OFFSET << 12 | (LTC4015_QCOUNT_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_QCOUNT_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_VBAT_LO_ALERT_BF VBAT_LO_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VBAT_LO_ALERT_BF Bit Field
 *
 *  Indicates VBAT has fallen below VBAT_LO_ALERT_LIMIT
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 11
 *   - MSB: 11
 *   - MASK: 0x0800
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VBAT_LO_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_VBAT_LO_ALERT_BF "VBAT_LO_ALERT_BF"
#define LTC4015_VBAT_LO_ALERT_BF_SIZE 1
#define LTC4015_VBAT_LO_ALERT_BF_OFFSET 11
#define LTC4015_VBAT_LO_ALERT_BF_MASK 0x0800
#define LTC4015_VBAT_LO_ALERT_BF (LTC4015_VBAT_LO_ALERT_BF_OFFSET << 12 | (LTC4015_VBAT_LO_ALERT_BF_SIZE - 1) << 8 | LTC4015_VBAT_LO_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_VBAT_HI_ALERT_BF VBAT_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VBAT_HI_ALERT_BF Bit Field
 *
 *  Indicates VBAT has exceeded VBAT_HI_ALERT_LIMIT
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 10
 *   - MSB: 10
 *   - MASK: 0x0400
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VBAT_HI_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_VBAT_HI_ALERT_BF "VBAT_HI_ALERT_BF"
#define LTC4015_VBAT_HI_ALERT_BF_SIZE 1
#define LTC4015_VBAT_HI_ALERT_BF_OFFSET 10
#define LTC4015_VBAT_HI_ALERT_BF_MASK 0x0400
#define LTC4015_VBAT_HI_ALERT_BF (LTC4015_VBAT_HI_ALERT_BF_OFFSET << 12 | (LTC4015_VBAT_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_VBAT_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_VIN_LO_ALERT_BF VIN_LO_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VIN_LO_ALERT_BF Bit Field
 *
 *  Indicates VIN has fallen below VIN_LO_ALERT_LIMIT
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 9
 *   - MSB: 9
 *   - MASK: 0x0200
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VIN_LO_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_VIN_LO_ALERT_BF "VIN_LO_ALERT_BF"
#define LTC4015_VIN_LO_ALERT_BF_SIZE 1
#define LTC4015_VIN_LO_ALERT_BF_OFFSET 9
#define LTC4015_VIN_LO_ALERT_BF_MASK 0x0200
#define LTC4015_VIN_LO_ALERT_BF (LTC4015_VIN_LO_ALERT_BF_OFFSET << 12 | (LTC4015_VIN_LO_ALERT_BF_SIZE - 1) << 8 | LTC4015_VIN_LO_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_VIN_HI_ALERT_BF VIN_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VIN_HI_ALERT_BF Bit Field
 *
 *  Indicates VIN has exceeded VIN_HI_ALERT_LIMIT
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 8
 *   - MSB: 8
 *   - MASK: 0x0100
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VIN_HI_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_VIN_HI_ALERT_BF "VIN_HI_ALERT_BF"
#define LTC4015_VIN_HI_ALERT_BF_SIZE 1
#define LTC4015_VIN_HI_ALERT_BF_OFFSET 8
#define LTC4015_VIN_HI_ALERT_BF_MASK 0x0100
#define LTC4015_VIN_HI_ALERT_BF (LTC4015_VIN_HI_ALERT_BF_OFFSET << 12 | (LTC4015_VIN_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_VIN_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_VSYS_LO_ALERT_BF VSYS_LO_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VSYS_LO_ALERT_BF Bit Field
 *
 *  Indicates VSYS has fallen below VSYS_LO_ALERT_LIMIT
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 7
 *   - MSB: 7
 *   - MASK: 0x0080
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VSYS_LO_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_VSYS_LO_ALERT_BF "VSYS_LO_ALERT_BF"
#define LTC4015_VSYS_LO_ALERT_BF_SIZE 1
#define LTC4015_VSYS_LO_ALERT_BF_OFFSET 7
#define LTC4015_VSYS_LO_ALERT_BF_MASK 0x0080
#define LTC4015_VSYS_LO_ALERT_BF (LTC4015_VSYS_LO_ALERT_BF_OFFSET << 12 | (LTC4015_VSYS_LO_ALERT_BF_SIZE - 1) << 8 | LTC4015_VSYS_LO_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_VSYS_HI_ALERT_BF VSYS_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VSYS_HI_ALERT_BF Bit Field
 *
 *  Indicates VSYS has exceeded VSYS_HI_ALERT_LIMIT
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 6
 *   - MSB: 6
 *   - MASK: 0x0040
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VSYS_HI_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_VSYS_HI_ALERT_BF "VSYS_HI_ALERT_BF"
#define LTC4015_VSYS_HI_ALERT_BF_SIZE 1
#define LTC4015_VSYS_HI_ALERT_BF_OFFSET 6
#define LTC4015_VSYS_HI_ALERT_BF_MASK 0x0040
#define LTC4015_VSYS_HI_ALERT_BF (LTC4015_VSYS_HI_ALERT_BF_OFFSET << 12 | (LTC4015_VSYS_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_VSYS_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_IIN_HI_ALERT_BF IIN_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief IIN_HI_ALERT_BF Bit Field
 *
 *  Indicates IIN has exceeded IIN_HI_ALERT_LIMIT
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 5
 *   - MSB: 5
 *   - MASK: 0x0020
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_IIN_HI_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_IIN_HI_ALERT_BF "IIN_HI_ALERT_BF"
#define LTC4015_IIN_HI_ALERT_BF_SIZE 1
#define LTC4015_IIN_HI_ALERT_BF_OFFSET 5
#define LTC4015_IIN_HI_ALERT_BF_MASK 0x0020
#define LTC4015_IIN_HI_ALERT_BF (LTC4015_IIN_HI_ALERT_BF_OFFSET << 12 | (LTC4015_IIN_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_IIN_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_IBAT_LO_ALERT_BF IBAT_LO_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief IBAT_LO_ALERT_BF Bit Field
 *
 *  Indicates IBAT has fallen below IBAT_LO_ALERT_LIMIT
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 4
 *   - MSB: 4
 *   - MASK: 0x0010
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_IBAT_LO_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_IBAT_LO_ALERT_BF "IBAT_LO_ALERT_BF"
#define LTC4015_IBAT_LO_ALERT_BF_SIZE 1
#define LTC4015_IBAT_LO_ALERT_BF_OFFSET 4
#define LTC4015_IBAT_LO_ALERT_BF_MASK 0x0010
#define LTC4015_IBAT_LO_ALERT_BF (LTC4015_IBAT_LO_ALERT_BF_OFFSET << 12 | (LTC4015_IBAT_LO_ALERT_BF_SIZE - 1) << 8 | LTC4015_IBAT_LO_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_DIE_TEMP_HI_ALERT_BF DIE_TEMP_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief DIE_TEMP_HI_ALERT_BF Bit Field
 *
 *  Indicates DIE_TEMP has exceeded DIE_TEMP_HI_ALERT_LIMIT
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 3
 *   - MSB: 3
 *   - MASK: 0x0008
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_DIE_TEMP_HI_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_DIE_TEMP_HI_ALERT_BF "DIE_TEMP_HI_ALERT_BF"
#define LTC4015_DIE_TEMP_HI_ALERT_BF_SIZE 1
#define LTC4015_DIE_TEMP_HI_ALERT_BF_OFFSET 3
#define LTC4015_DIE_TEMP_HI_ALERT_BF_MASK 0x0008
#define LTC4015_DIE_TEMP_HI_ALERT_BF (LTC4015_DIE_TEMP_HI_ALERT_BF_OFFSET << 12 | (LTC4015_DIE_TEMP_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_DIE_TEMP_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_BSR_HI_ALERT_BF BSR_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief BSR_HI_ALERT_BF Bit Field
 *
 *  Indicates BSR has exceeded BSR_HI_ALERT_LIMIT
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 2
 *   - MSB: 2
 *   - MASK: 0x0004
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_BSR_HI_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_BSR_HI_ALERT_BF "BSR_HI_ALERT_BF"
#define LTC4015_BSR_HI_ALERT_BF_SIZE 1
#define LTC4015_BSR_HI_ALERT_BF_OFFSET 2
#define LTC4015_BSR_HI_ALERT_BF_MASK 0x0004
#define LTC4015_BSR_HI_ALERT_BF (LTC4015_BSR_HI_ALERT_BF_OFFSET << 12 | (LTC4015_BSR_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_BSR_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_NTC_RATIO_HI_ALERT_BF NTC_RATIO_HI_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief NTC_RATIO_HI_ALERT_BF Bit Field
 *
 *  Indicates NTC_RATIO has exceeded NTC_RATIO_HI_ALERT_LIMIT
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 1
 *   - MSB: 1
 *   - MASK: 0x0002
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_NTC_RATIO_HI_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_NTC_RATIO_HI_ALERT_BF "NTC_RATIO_HI_ALERT_BF"
#define LTC4015_NTC_RATIO_HI_ALERT_BF_SIZE 1
#define LTC4015_NTC_RATIO_HI_ALERT_BF_OFFSET 1
#define LTC4015_NTC_RATIO_HI_ALERT_BF_MASK 0x0002
#define LTC4015_NTC_RATIO_HI_ALERT_BF (LTC4015_NTC_RATIO_HI_ALERT_BF_OFFSET << 12 | (LTC4015_NTC_RATIO_HI_ALERT_BF_SIZE - 1) << 8 | LTC4015_NTC_RATIO_HI_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_NTC_RATIO_LO_ALERT_BF NTC_RATIO_LO_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief NTC_RATIO_LO_ALERT_BF Bit Field
 *
 *  Indicates NTC_RATIO has fallen below NTC_RATIO_LO_ALERT_LIMIT
 *   - Register: @ref LTC4015_LIMIT_ALERTS "LIMIT_ALERTS"
 *   - CommandCode: 54
 *   - Size: 1
 *   - Offset: 0
 *   - MSB: 0
 *   - MASK: 0x0001
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_NTC_RATIO_LO_ALERT_BF_SUBADDR LTC4015_LIMIT_ALERTS_SUBADDR //!< @ref LTC4015_NTC_RATIO_LO_ALERT_BF "NTC_RATIO_LO_ALERT_BF"
#define LTC4015_NTC_RATIO_LO_ALERT_BF_SIZE 1
#define LTC4015_NTC_RATIO_LO_ALERT_BF_OFFSET 0
#define LTC4015_NTC_RATIO_LO_ALERT_BF_MASK 0x0001
#define LTC4015_NTC_RATIO_LO_ALERT_BF (LTC4015_NTC_RATIO_LO_ALERT_BF_OFFSET << 12 | (LTC4015_NTC_RATIO_LO_ALERT_BF_SIZE - 1) << 8 | LTC4015_NTC_RATIO_LO_ALERT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_CHARGER_STATE_ALERTS CHARGER_STATE_ALERTS
 *  @ingroup LTC4015_register_map
 *  @brief CHARGER_STATE_ALERTS Register
 *
 * | 15:11 |                       10 |                      9 |                          8 |                  7 |                     6 |                  5 |                   4 |                      3 |                              2 |                          1 |                        0 |
 * |:-----:|:------------------------:|:----------------------:|:--------------------------:|:------------------:|:---------------------:|:------------------:|:-------------------:|:----------------------:|:------------------------------:|:--------------------------:|:------------------------:|
 * |   n/a | EQUALIZE_CHARGE_ALERT_BF | ABSORB_CHARGE_ALERT_BF | CHARGER_SUSPENDED_ALERT_BF | PRECHARGE_ALERT_BF | CC_CV_CHARGE_ALERT_BF | NTC_PAUSE_ALERT_BF | TIMER_TERM_ALERT_BF | C_OVER_X_TERM_ALERT_BF | MAX_CHARGE_TIME_FAULT_ALERT_BF | BAT_MISSING_FAULT_ALERT_BF | BAT_SHORT_FAULT_ALERT_BF |
 *
 * Indicates that charger states have occurred.<br />Individual bits are enabled by EN_CHARGER_STATE_ALERTS.<br />Writing 1 to any bit clears that alert.<br />Once set, alert bits remain high until cleared or disabled.
 *   - CommandCode: 55
 *   - Contains Bit Fields:
 *     + @ref LTC4015_EQUALIZE_CHARGE_ALERT_BF "EQUALIZE_CHARGE_ALERT_BF" : Alert indicates charger has entered equalize charge state
 *     + @ref LTC4015_ABSORB_CHARGE_ALERT_BF "ABSORB_CHARGE_ALERT_BF" : Alert indicates charger has entered absorb charge state
 *     + @ref LTC4015_CHARGER_SUSPENDED_ALERT_BF "CHARGER_SUSPENDED_ALERT_BF" : Alert indicates charger has been suspended
 *     + @ref LTC4015_PRECHARGE_ALERT_BF "PRECHARGE_ALERT_BF" : Alert indicates charger has entered preconditioning charge state
 *     + @ref LTC4015_CC_CV_CHARGE_ALERT_BF "CC_CV_CHARGE_ALERT_BF" : Alert indicates charger has entered CC-CV charge state
 *     + @ref LTC4015_NTC_PAUSE_ALERT_BF "NTC_PAUSE_ALERT_BF" : Alert indicates charger has entered thermistor pause state
 *     + @ref LTC4015_TIMER_TERM_ALERT_BF "TIMER_TERM_ALERT_BF" : Alert indicates timer termination has occurred
 *     + @ref LTC4015_C_OVER_X_TERM_ALERT_BF "C_OVER_X_TERM_ALERT_BF" : Alert indicates C/x termination has occurred
 *     + @ref LTC4015_MAX_CHARGE_TIME_FAULT_ALERT_BF "MAX_CHARGE_TIME_FAULT_ALERT_BF" : Alert indicates charger has entered max_charge_time_fault state
 *     + @ref LTC4015_BAT_MISSING_FAULT_ALERT_BF "BAT_MISSING_FAULT_ALERT_BF" : Alert indicates battery missing fault has occurred
 *     + @ref LTC4015_BAT_SHORT_FAULT_ALERT_BF "BAT_SHORT_FAULT_ALERT_BF" : Alert indicates battery short fault has occurred
*/

//!@{
#define LTC4015_CHARGER_STATE_ALERTS_SUBADDR 55
#define LTC4015_CHARGER_STATE_ALERTS (0 << 12 | (16 - 1) << 8 | LTC4015_CHARGER_STATE_ALERTS_SUBADDR)
//!@}
/*! @defgroup LTC4015_EQUALIZE_CHARGE_ALERT_BF EQUALIZE_CHARGE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief EQUALIZE_CHARGE_ALERT_BF Bit Field
 *
 *  Alert indicates charger has entered equalize charge state
 *   - Register: @ref LTC4015_CHARGER_STATE_ALERTS "CHARGER_STATE_ALERTS"
 *   - CommandCode: 55
 *   - Size: 1
 *   - Offset: 10
 *   - MSB: 10
 *   - MASK: 0x0400
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_EQUALIZE_CHARGE_ALERT_BF_SUBADDR LTC4015_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_EQUALIZE_CHARGE_ALERT_BF "EQUALIZE_CHARGE_ALERT_BF"
#define LTC4015_EQUALIZE_CHARGE_ALERT_BF_SIZE 1
#define LTC4015_EQUALIZE_CHARGE_ALERT_BF_OFFSET 10
#define LTC4015_EQUALIZE_CHARGE_ALERT_BF_MASK 0x0400
#define LTC4015_EQUALIZE_CHARGE_ALERT_BF (LTC4015_EQUALIZE_CHARGE_ALERT_BF_OFFSET << 12 | (LTC4015_EQUALIZE_CHARGE_ALERT_BF_SIZE - 1) << 8 | LTC4015_EQUALIZE_CHARGE_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_ABSORB_CHARGE_ALERT_BF ABSORB_CHARGE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief ABSORB_CHARGE_ALERT_BF Bit Field
 *
 *  Alert indicates charger has entered absorb charge state
 *   - Register: @ref LTC4015_CHARGER_STATE_ALERTS "CHARGER_STATE_ALERTS"
 *   - CommandCode: 55
 *   - Size: 1
 *   - Offset: 9
 *   - MSB: 9
 *   - MASK: 0x0200
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_ABSORB_CHARGE_ALERT_BF_SUBADDR LTC4015_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_ABSORB_CHARGE_ALERT_BF "ABSORB_CHARGE_ALERT_BF"
#define LTC4015_ABSORB_CHARGE_ALERT_BF_SIZE 1
#define LTC4015_ABSORB_CHARGE_ALERT_BF_OFFSET 9
#define LTC4015_ABSORB_CHARGE_ALERT_BF_MASK 0x0200
#define LTC4015_ABSORB_CHARGE_ALERT_BF (LTC4015_ABSORB_CHARGE_ALERT_BF_OFFSET << 12 | (LTC4015_ABSORB_CHARGE_ALERT_BF_SIZE - 1) << 8 | LTC4015_ABSORB_CHARGE_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_CHARGER_SUSPENDED_ALERT_BF CHARGER_SUSPENDED_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief CHARGER_SUSPENDED_ALERT_BF Bit Field
 *
 *  Alert indicates charger has been suspended
 *   - Register: @ref LTC4015_CHARGER_STATE_ALERTS "CHARGER_STATE_ALERTS"
 *   - CommandCode: 55
 *   - Size: 1
 *   - Offset: 8
 *   - MSB: 8
 *   - MASK: 0x0100
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_CHARGER_SUSPENDED_ALERT_BF_SUBADDR LTC4015_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_CHARGER_SUSPENDED_ALERT_BF "CHARGER_SUSPENDED_ALERT_BF"
#define LTC4015_CHARGER_SUSPENDED_ALERT_BF_SIZE 1
#define LTC4015_CHARGER_SUSPENDED_ALERT_BF_OFFSET 8
#define LTC4015_CHARGER_SUSPENDED_ALERT_BF_MASK 0x0100
#define LTC4015_CHARGER_SUSPENDED_ALERT_BF (LTC4015_CHARGER_SUSPENDED_ALERT_BF_OFFSET << 12 | (LTC4015_CHARGER_SUSPENDED_ALERT_BF_SIZE - 1) << 8 | LTC4015_CHARGER_SUSPENDED_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_PRECHARGE_ALERT_BF PRECHARGE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief PRECHARGE_ALERT_BF Bit Field
 *
 *  Alert indicates charger has entered preconditioning charge state
 *   - Register: @ref LTC4015_CHARGER_STATE_ALERTS "CHARGER_STATE_ALERTS"
 *   - CommandCode: 55
 *   - Size: 1
 *   - Offset: 7
 *   - MSB: 7
 *   - MASK: 0x0080
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_PRECHARGE_ALERT_BF_SUBADDR LTC4015_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_PRECHARGE_ALERT_BF "PRECHARGE_ALERT_BF"
#define LTC4015_PRECHARGE_ALERT_BF_SIZE 1
#define LTC4015_PRECHARGE_ALERT_BF_OFFSET 7
#define LTC4015_PRECHARGE_ALERT_BF_MASK 0x0080
#define LTC4015_PRECHARGE_ALERT_BF (LTC4015_PRECHARGE_ALERT_BF_OFFSET << 12 | (LTC4015_PRECHARGE_ALERT_BF_SIZE - 1) << 8 | LTC4015_PRECHARGE_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_CC_CV_CHARGE_ALERT_BF CC_CV_CHARGE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief CC_CV_CHARGE_ALERT_BF Bit Field
 *
 *  Alert indicates charger has entered CC-CV charge state
 *   - Register: @ref LTC4015_CHARGER_STATE_ALERTS "CHARGER_STATE_ALERTS"
 *   - CommandCode: 55
 *   - Size: 1
 *   - Offset: 6
 *   - MSB: 6
 *   - MASK: 0x0040
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_CC_CV_CHARGE_ALERT_BF_SUBADDR LTC4015_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_CC_CV_CHARGE_ALERT_BF "CC_CV_CHARGE_ALERT_BF"
#define LTC4015_CC_CV_CHARGE_ALERT_BF_SIZE 1
#define LTC4015_CC_CV_CHARGE_ALERT_BF_OFFSET 6
#define LTC4015_CC_CV_CHARGE_ALERT_BF_MASK 0x0040
#define LTC4015_CC_CV_CHARGE_ALERT_BF (LTC4015_CC_CV_CHARGE_ALERT_BF_OFFSET << 12 | (LTC4015_CC_CV_CHARGE_ALERT_BF_SIZE - 1) << 8 | LTC4015_CC_CV_CHARGE_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_NTC_PAUSE_ALERT_BF NTC_PAUSE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief NTC_PAUSE_ALERT_BF Bit Field
 *
 *  Alert indicates charger has entered thermistor pause state
 *   - Register: @ref LTC4015_CHARGER_STATE_ALERTS "CHARGER_STATE_ALERTS"
 *   - CommandCode: 55
 *   - Size: 1
 *   - Offset: 5
 *   - MSB: 5
 *   - MASK: 0x0020
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_NTC_PAUSE_ALERT_BF_SUBADDR LTC4015_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_NTC_PAUSE_ALERT_BF "NTC_PAUSE_ALERT_BF"
#define LTC4015_NTC_PAUSE_ALERT_BF_SIZE 1
#define LTC4015_NTC_PAUSE_ALERT_BF_OFFSET 5
#define LTC4015_NTC_PAUSE_ALERT_BF_MASK 0x0020
#define LTC4015_NTC_PAUSE_ALERT_BF (LTC4015_NTC_PAUSE_ALERT_BF_OFFSET << 12 | (LTC4015_NTC_PAUSE_ALERT_BF_SIZE - 1) << 8 | LTC4015_NTC_PAUSE_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_TIMER_TERM_ALERT_BF TIMER_TERM_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief TIMER_TERM_ALERT_BF Bit Field
 *
 *  Alert indicates timer termination has occurred
 *   - Register: @ref LTC4015_CHARGER_STATE_ALERTS "CHARGER_STATE_ALERTS"
 *   - CommandCode: 55
 *   - Size: 1
 *   - Offset: 4
 *   - MSB: 4
 *   - MASK: 0x0010
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_TIMER_TERM_ALERT_BF_SUBADDR LTC4015_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_TIMER_TERM_ALERT_BF "TIMER_TERM_ALERT_BF"
#define LTC4015_TIMER_TERM_ALERT_BF_SIZE 1
#define LTC4015_TIMER_TERM_ALERT_BF_OFFSET 4
#define LTC4015_TIMER_TERM_ALERT_BF_MASK 0x0010
#define LTC4015_TIMER_TERM_ALERT_BF (LTC4015_TIMER_TERM_ALERT_BF_OFFSET << 12 | (LTC4015_TIMER_TERM_ALERT_BF_SIZE - 1) << 8 | LTC4015_TIMER_TERM_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_C_OVER_X_TERM_ALERT_BF C_OVER_X_TERM_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief C_OVER_X_TERM_ALERT_BF Bit Field
 *
 *  Alert indicates C/x termination has occurred
 *   - Register: @ref LTC4015_CHARGER_STATE_ALERTS "CHARGER_STATE_ALERTS"
 *   - CommandCode: 55
 *   - Size: 1
 *   - Offset: 3
 *   - MSB: 3
 *   - MASK: 0x0008
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_C_OVER_X_TERM_ALERT_BF_SUBADDR LTC4015_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_C_OVER_X_TERM_ALERT_BF "C_OVER_X_TERM_ALERT_BF"
#define LTC4015_C_OVER_X_TERM_ALERT_BF_SIZE 1
#define LTC4015_C_OVER_X_TERM_ALERT_BF_OFFSET 3
#define LTC4015_C_OVER_X_TERM_ALERT_BF_MASK 0x0008
#define LTC4015_C_OVER_X_TERM_ALERT_BF (LTC4015_C_OVER_X_TERM_ALERT_BF_OFFSET << 12 | (LTC4015_C_OVER_X_TERM_ALERT_BF_SIZE - 1) << 8 | LTC4015_C_OVER_X_TERM_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_MAX_CHARGE_TIME_FAULT_ALERT_BF MAX_CHARGE_TIME_FAULT_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief MAX_CHARGE_TIME_FAULT_ALERT_BF Bit Field
 *
 *  Alert indicates charger has entered max_charge_time_fault state
 *   - Register: @ref LTC4015_CHARGER_STATE_ALERTS "CHARGER_STATE_ALERTS"
 *   - CommandCode: 55
 *   - Size: 1
 *   - Offset: 2
 *   - MSB: 2
 *   - MASK: 0x0004
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_MAX_CHARGE_TIME_FAULT_ALERT_BF_SUBADDR LTC4015_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_MAX_CHARGE_TIME_FAULT_ALERT_BF "MAX_CHARGE_TIME_FAULT_ALERT_BF"
#define LTC4015_MAX_CHARGE_TIME_FAULT_ALERT_BF_SIZE 1
#define LTC4015_MAX_CHARGE_TIME_FAULT_ALERT_BF_OFFSET 2
#define LTC4015_MAX_CHARGE_TIME_FAULT_ALERT_BF_MASK 0x0004
#define LTC4015_MAX_CHARGE_TIME_FAULT_ALERT_BF (LTC4015_MAX_CHARGE_TIME_FAULT_ALERT_BF_OFFSET << 12 | (LTC4015_MAX_CHARGE_TIME_FAULT_ALERT_BF_SIZE - 1) << 8 | LTC4015_MAX_CHARGE_TIME_FAULT_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_BAT_MISSING_FAULT_ALERT_BF BAT_MISSING_FAULT_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief BAT_MISSING_FAULT_ALERT_BF Bit Field
 *
 *  Alert indicates battery missing fault has occurred
 *   - Register: @ref LTC4015_CHARGER_STATE_ALERTS "CHARGER_STATE_ALERTS"
 *   - CommandCode: 55
 *   - Size: 1
 *   - Offset: 1
 *   - MSB: 1
 *   - MASK: 0x0002
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_BAT_MISSING_FAULT_ALERT_BF_SUBADDR LTC4015_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_BAT_MISSING_FAULT_ALERT_BF "BAT_MISSING_FAULT_ALERT_BF"
#define LTC4015_BAT_MISSING_FAULT_ALERT_BF_SIZE 1
#define LTC4015_BAT_MISSING_FAULT_ALERT_BF_OFFSET 1
#define LTC4015_BAT_MISSING_FAULT_ALERT_BF_MASK 0x0002
#define LTC4015_BAT_MISSING_FAULT_ALERT_BF (LTC4015_BAT_MISSING_FAULT_ALERT_BF_OFFSET << 12 | (LTC4015_BAT_MISSING_FAULT_ALERT_BF_SIZE - 1) << 8 | LTC4015_BAT_MISSING_FAULT_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_BAT_SHORT_FAULT_ALERT_BF BAT_SHORT_FAULT_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief BAT_SHORT_FAULT_ALERT_BF Bit Field
 *
 *  Alert indicates battery short fault has occurred
 *   - Register: @ref LTC4015_CHARGER_STATE_ALERTS "CHARGER_STATE_ALERTS"
 *   - CommandCode: 55
 *   - Size: 1
 *   - Offset: 0
 *   - MSB: 0
 *   - MASK: 0x0001
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_BAT_SHORT_FAULT_ALERT_BF_SUBADDR LTC4015_CHARGER_STATE_ALERTS_SUBADDR //!< @ref LTC4015_BAT_SHORT_FAULT_ALERT_BF "BAT_SHORT_FAULT_ALERT_BF"
#define LTC4015_BAT_SHORT_FAULT_ALERT_BF_SIZE 1
#define LTC4015_BAT_SHORT_FAULT_ALERT_BF_OFFSET 0
#define LTC4015_BAT_SHORT_FAULT_ALERT_BF_MASK 0x0001
#define LTC4015_BAT_SHORT_FAULT_ALERT_BF (LTC4015_BAT_SHORT_FAULT_ALERT_BF_OFFSET << 12 | (LTC4015_BAT_SHORT_FAULT_ALERT_BF_SIZE - 1) << 8 | LTC4015_BAT_SHORT_FAULT_ALERT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_CHARGE_STATUS_ALERTS CHARGE_STATUS_ALERTS
 *  @ingroup LTC4015_register_map
 *  @brief CHARGE_STATUS_ALERTS Register
 *
 * | 15:4 |                        3 |                         2 |                         1 |                         0 |
 * |:----:|:------------------------:|:-------------------------:|:-------------------------:|:-------------------------:|
 * |  n/a | VIN_UVCL_ACTIVE_ALERT_BF | IIN_LIMIT_ACTIVE_ALERT_BF | CONSTANT_CURRENT_ALERT_BF | CONSTANT_VOLTAGE_ALERT_BF |
 *
 * Alerts that CHARGE_STATUS indicators have occurred.<br />Individual bits are enabled by EN_CHARGE_STATUS_ALERTS.<br />Writing 1 to any bit clears that alert.<br />Once set, alert bits remain high until cleared or disabled.
 *   - CommandCode: 56
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VIN_UVCL_ACTIVE_ALERT_BF "VIN_UVCL_ACTIVE_ALERT_BF" : Alert indicates that vin_uvcl_active has occurred
 *     + @ref LTC4015_IIN_LIMIT_ACTIVE_ALERT_BF "IIN_LIMIT_ACTIVE_ALERT_BF" : Alert indicates iin_limit_active has occurred
 *     + @ref LTC4015_CONSTANT_CURRENT_ALERT_BF "CONSTANT_CURRENT_ALERT_BF" : Alert indicates constant_current has occurred
 *     + @ref LTC4015_CONSTANT_VOLTAGE_ALERT_BF "CONSTANT_VOLTAGE_ALERT_BF" : Alert indicates constant_voltage has occurred
*/

//!@{
#define LTC4015_CHARGE_STATUS_ALERTS_SUBADDR 56
#define LTC4015_CHARGE_STATUS_ALERTS (0 << 12 | (16 - 1) << 8 | LTC4015_CHARGE_STATUS_ALERTS_SUBADDR)
//!@}
/*! @defgroup LTC4015_VIN_UVCL_ACTIVE_ALERT_BF VIN_UVCL_ACTIVE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VIN_UVCL_ACTIVE_ALERT_BF Bit Field
 *
 *  Alert indicates that vin_uvcl_active has occurred
 *   - Register: @ref LTC4015_CHARGE_STATUS_ALERTS "CHARGE_STATUS_ALERTS"
 *   - CommandCode: 56
 *   - Size: 1
 *   - Offset: 3
 *   - MSB: 3
 *   - MASK: 0x0008
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VIN_UVCL_ACTIVE_ALERT_BF_SUBADDR LTC4015_CHARGE_STATUS_ALERTS_SUBADDR //!< @ref LTC4015_VIN_UVCL_ACTIVE_ALERT_BF "VIN_UVCL_ACTIVE_ALERT_BF"
#define LTC4015_VIN_UVCL_ACTIVE_ALERT_BF_SIZE 1
#define LTC4015_VIN_UVCL_ACTIVE_ALERT_BF_OFFSET 3
#define LTC4015_VIN_UVCL_ACTIVE_ALERT_BF_MASK 0x0008
#define LTC4015_VIN_UVCL_ACTIVE_ALERT_BF (LTC4015_VIN_UVCL_ACTIVE_ALERT_BF_OFFSET << 12 | (LTC4015_VIN_UVCL_ACTIVE_ALERT_BF_SIZE - 1) << 8 | LTC4015_VIN_UVCL_ACTIVE_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_IIN_LIMIT_ACTIVE_ALERT_BF IIN_LIMIT_ACTIVE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief IIN_LIMIT_ACTIVE_ALERT_BF Bit Field
 *
 *  Alert indicates iin_limit_active has occurred
 *   - Register: @ref LTC4015_CHARGE_STATUS_ALERTS "CHARGE_STATUS_ALERTS"
 *   - CommandCode: 56
 *   - Size: 1
 *   - Offset: 2
 *   - MSB: 2
 *   - MASK: 0x0004
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_IIN_LIMIT_ACTIVE_ALERT_BF_SUBADDR LTC4015_CHARGE_STATUS_ALERTS_SUBADDR //!< @ref LTC4015_IIN_LIMIT_ACTIVE_ALERT_BF "IIN_LIMIT_ACTIVE_ALERT_BF"
#define LTC4015_IIN_LIMIT_ACTIVE_ALERT_BF_SIZE 1
#define LTC4015_IIN_LIMIT_ACTIVE_ALERT_BF_OFFSET 2
#define LTC4015_IIN_LIMIT_ACTIVE_ALERT_BF_MASK 0x0004
#define LTC4015_IIN_LIMIT_ACTIVE_ALERT_BF (LTC4015_IIN_LIMIT_ACTIVE_ALERT_BF_OFFSET << 12 | (LTC4015_IIN_LIMIT_ACTIVE_ALERT_BF_SIZE - 1) << 8 | LTC4015_IIN_LIMIT_ACTIVE_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_CONSTANT_CURRENT_ALERT_BF CONSTANT_CURRENT_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief CONSTANT_CURRENT_ALERT_BF Bit Field
 *
 *  Alert indicates constant_current has occurred
 *   - Register: @ref LTC4015_CHARGE_STATUS_ALERTS "CHARGE_STATUS_ALERTS"
 *   - CommandCode: 56
 *   - Size: 1
 *   - Offset: 1
 *   - MSB: 1
 *   - MASK: 0x0002
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_CONSTANT_CURRENT_ALERT_BF_SUBADDR LTC4015_CHARGE_STATUS_ALERTS_SUBADDR //!< @ref LTC4015_CONSTANT_CURRENT_ALERT_BF "CONSTANT_CURRENT_ALERT_BF"
#define LTC4015_CONSTANT_CURRENT_ALERT_BF_SIZE 1
#define LTC4015_CONSTANT_CURRENT_ALERT_BF_OFFSET 1
#define LTC4015_CONSTANT_CURRENT_ALERT_BF_MASK 0x0002
#define LTC4015_CONSTANT_CURRENT_ALERT_BF (LTC4015_CONSTANT_CURRENT_ALERT_BF_OFFSET << 12 | (LTC4015_CONSTANT_CURRENT_ALERT_BF_SIZE - 1) << 8 | LTC4015_CONSTANT_CURRENT_ALERT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_CONSTANT_VOLTAGE_ALERT_BF CONSTANT_VOLTAGE_ALERT_BF
 *  @ingroup LTC4015_register_map
 *  @brief CONSTANT_VOLTAGE_ALERT_BF Bit Field
 *
 *  Alert indicates constant_voltage has occurred
 *   - Register: @ref LTC4015_CHARGE_STATUS_ALERTS "CHARGE_STATUS_ALERTS"
 *   - CommandCode: 56
 *   - Size: 1
 *   - Offset: 0
 *   - MSB: 0
 *   - MASK: 0x0001
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_CONSTANT_VOLTAGE_ALERT_BF_SUBADDR LTC4015_CHARGE_STATUS_ALERTS_SUBADDR //!< @ref LTC4015_CONSTANT_VOLTAGE_ALERT_BF "CONSTANT_VOLTAGE_ALERT_BF"
#define LTC4015_CONSTANT_VOLTAGE_ALERT_BF_SIZE 1
#define LTC4015_CONSTANT_VOLTAGE_ALERT_BF_OFFSET 0
#define LTC4015_CONSTANT_VOLTAGE_ALERT_BF_MASK 0x0001
#define LTC4015_CONSTANT_VOLTAGE_ALERT_BF (LTC4015_CONSTANT_VOLTAGE_ALERT_BF_OFFSET << 12 | (LTC4015_CONSTANT_VOLTAGE_ALERT_BF_SIZE - 1) << 8 | LTC4015_CONSTANT_VOLTAGE_ALERT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_SYSTEM_STATUS SYSTEM_STATUS
 *  @ingroup LTC4015_register_map
 *  @brief SYSTEM_STATUS Register
 *
 * | 15:14 |                 13 |  12 |             11 |              10 |             9 |                   8 |   7 |               6 |        5 |                   4 |           3 |              2 |                 1 |                 0 |
 * |:-----:|:------------------:|:---:|:--------------:|:---------------:|:-------------:|:-------------------:|:---:|:---------------:|:--------:|:-------------------:|:-----------:|:--------------:|:-----------------:|:-----------------:|
 * |   n/a | CHARGER_ENABLED_BF | n/a | MPPT_EN_PIN_BF | EQUALIZE_REQ_BF | DRVCC_GOOD_BF | CELL_COUNT_ERROR_BF | n/a | OK_TO_CHARGE_BF | NO_RT_BF | THERMAL_SHUTDOWN_BF | VIN_OVLO_BF | VIN_GT_VBAT_BF | INTVCC_GT_4P3V_BF | INTVCC_GT_2P8V_BF |
 *
 * Real time system status indicator bits
 *   - CommandCode: 57
 *   - Contains Bit Fields:
 *     + @ref LTC4015_CHARGER_ENABLED_BF "CHARGER_ENABLED_BF" : Indicates that the battery charger is active
 *     + @ref LTC4015_MPPT_EN_PIN_BF "MPPT_EN_PIN_BF" : Indicates the MPPT pin is set to enable Maximum Power Point Tracking
 *     + @ref LTC4015_EQUALIZE_REQ_BF "EQUALIZE_REQ_BF" : Indicates a rising edge has been detected at the EQ pin, and an equalize charge is queued
 *     + @ref LTC4015_DRVCC_GOOD_BF "DRVCC_GOOD_BF" : Indicates DRVCC voltage is above switching regulator undervoltage lockout level (4.3V typ)
 *     + @ref LTC4015_CELL_COUNT_ERROR_BF "CELL_COUNT_ERROR_BF" : Indicates an invalid combination of CELLS pin settings
 *     + @ref LTC4015_OK_TO_CHARGE_BF "OK_TO_CHARGE_BF" : Indicates all system conditions are met to allow battery charger operation.
 *     + @ref LTC4015_NO_RT_BF "NO_RT_BF" : Indicates no resistor has been detected at the RT pin
 *     + @ref LTC4015_THERMAL_SHUTDOWN_BF "THERMAL_SHUTDOWN_BF" : Indicates die temperature is greater than thermal shutdown level (160C typical)
 *     + @ref LTC4015_VIN_OVLO_BF "VIN_OVLO_BF" : Indicates VIN voltage is greater than overvoltage lockout level (38.6V typical)
 *     + @ref LTC4015_VIN_GT_VBAT_BF "VIN_GT_VBAT_BF" : Indicates VIN voltage is sufficiently greater than BATSENSE for switching regulator operation (200mV typical)
 *     + @ref LTC4015_INTVCC_GT_4P3V_BF "INTVCC_GT_4P3V_BF" : Indicates INTVCC voltage is above switching regulator undervoltage lockout level (4.3V typ)
 *     + @ref LTC4015_INTVCC_GT_2P8V_BF "INTVCC_GT_2P8V_BF" : Indicates INTVCC voltage is greater than measurement system lockout level (2.8V typical)
*/

//!@{
#define LTC4015_SYSTEM_STATUS_SUBADDR 57
#define LTC4015_SYSTEM_STATUS (0 << 12 | (16 - 1) << 8 | LTC4015_SYSTEM_STATUS_SUBADDR)
//!@}
/*! @defgroup LTC4015_CHARGER_ENABLED_BF CHARGER_ENABLED_BF
 *  @ingroup LTC4015_register_map
 *  @brief CHARGER_ENABLED_BF Bit Field
 *
 *  Indicates that the battery charger is active
 *   - Register: @ref LTC4015_SYSTEM_STATUS "SYSTEM_STATUS"
 *   - CommandCode: 57
 *   - Size: 1
 *   - Offset: 13
 *   - MSB: 13
 *   - MASK: 0x2000
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_CHARGER_ENABLED_BF_SUBADDR LTC4015_SYSTEM_STATUS_SUBADDR //!< @ref LTC4015_CHARGER_ENABLED_BF "CHARGER_ENABLED_BF"
#define LTC4015_CHARGER_ENABLED_BF_SIZE 1
#define LTC4015_CHARGER_ENABLED_BF_OFFSET 13
#define LTC4015_CHARGER_ENABLED_BF_MASK 0x2000
#define LTC4015_CHARGER_ENABLED_BF (LTC4015_CHARGER_ENABLED_BF_OFFSET << 12 | (LTC4015_CHARGER_ENABLED_BF_SIZE - 1) << 8 | LTC4015_CHARGER_ENABLED_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_MPPT_EN_PIN_BF MPPT_EN_PIN_BF
 *  @ingroup LTC4015_register_map
 *  @brief MPPT_EN_PIN_BF Bit Field
 *
 *  Indicates the MPPT pin is set to enable Maximum Power Point Tracking
 *   - Register: @ref LTC4015_SYSTEM_STATUS "SYSTEM_STATUS"
 *   - CommandCode: 57
 *   - Size: 1
 *   - Offset: 11
 *   - MSB: 11
 *   - MASK: 0x0800
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_MPPT_EN_PIN_BF_SUBADDR LTC4015_SYSTEM_STATUS_SUBADDR //!< @ref LTC4015_MPPT_EN_PIN_BF "MPPT_EN_PIN_BF"
#define LTC4015_MPPT_EN_PIN_BF_SIZE 1
#define LTC4015_MPPT_EN_PIN_BF_OFFSET 11
#define LTC4015_MPPT_EN_PIN_BF_MASK 0x0800
#define LTC4015_MPPT_EN_PIN_BF (LTC4015_MPPT_EN_PIN_BF_OFFSET << 12 | (LTC4015_MPPT_EN_PIN_BF_SIZE - 1) << 8 | LTC4015_MPPT_EN_PIN_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_EQUALIZE_REQ_BF EQUALIZE_REQ_BF
 *  @ingroup LTC4015_register_map
 *  @brief EQUALIZE_REQ_BF Bit Field
 *
 *  Indicates a rising edge has been detected at the EQ pin, and an equalize charge is queued
 *   - Register: @ref LTC4015_SYSTEM_STATUS "SYSTEM_STATUS"
 *   - CommandCode: 57
 *   - Size: 1
 *   - Offset: 10
 *   - MSB: 10
 *   - MASK: 0x0400
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_EQUALIZE_REQ_BF_SUBADDR LTC4015_SYSTEM_STATUS_SUBADDR //!< @ref LTC4015_EQUALIZE_REQ_BF "EQUALIZE_REQ_BF"
#define LTC4015_EQUALIZE_REQ_BF_SIZE 1
#define LTC4015_EQUALIZE_REQ_BF_OFFSET 10
#define LTC4015_EQUALIZE_REQ_BF_MASK 0x0400
#define LTC4015_EQUALIZE_REQ_BF (LTC4015_EQUALIZE_REQ_BF_OFFSET << 12 | (LTC4015_EQUALIZE_REQ_BF_SIZE - 1) << 8 | LTC4015_EQUALIZE_REQ_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_DRVCC_GOOD_BF DRVCC_GOOD_BF
 *  @ingroup LTC4015_register_map
 *  @brief DRVCC_GOOD_BF Bit Field
 *
 *  Indicates DRVCC voltage is above switching regulator undervoltage lockout level (4.3V typ)
 *   - Register: @ref LTC4015_SYSTEM_STATUS "SYSTEM_STATUS"
 *   - CommandCode: 57
 *   - Size: 1
 *   - Offset: 9
 *   - MSB: 9
 *   - MASK: 0x0200
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_DRVCC_GOOD_BF_SUBADDR LTC4015_SYSTEM_STATUS_SUBADDR //!< @ref LTC4015_DRVCC_GOOD_BF "DRVCC_GOOD_BF"
#define LTC4015_DRVCC_GOOD_BF_SIZE 1
#define LTC4015_DRVCC_GOOD_BF_OFFSET 9
#define LTC4015_DRVCC_GOOD_BF_MASK 0x0200
#define LTC4015_DRVCC_GOOD_BF (LTC4015_DRVCC_GOOD_BF_OFFSET << 12 | (LTC4015_DRVCC_GOOD_BF_SIZE - 1) << 8 | LTC4015_DRVCC_GOOD_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_CELL_COUNT_ERROR_BF CELL_COUNT_ERROR_BF
 *  @ingroup LTC4015_register_map
 *  @brief CELL_COUNT_ERROR_BF Bit Field
 *
 *  Indicates an invalid combination of CELLS pin settings
 *   - Register: @ref LTC4015_SYSTEM_STATUS "SYSTEM_STATUS"
 *   - CommandCode: 57
 *   - Size: 1
 *   - Offset: 8
 *   - MSB: 8
 *   - MASK: 0x0100
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_CELL_COUNT_ERROR_BF_SUBADDR LTC4015_SYSTEM_STATUS_SUBADDR //!< @ref LTC4015_CELL_COUNT_ERROR_BF "CELL_COUNT_ERROR_BF"
#define LTC4015_CELL_COUNT_ERROR_BF_SIZE 1
#define LTC4015_CELL_COUNT_ERROR_BF_OFFSET 8
#define LTC4015_CELL_COUNT_ERROR_BF_MASK 0x0100
#define LTC4015_CELL_COUNT_ERROR_BF (LTC4015_CELL_COUNT_ERROR_BF_OFFSET << 12 | (LTC4015_CELL_COUNT_ERROR_BF_SIZE - 1) << 8 | LTC4015_CELL_COUNT_ERROR_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_OK_TO_CHARGE_BF OK_TO_CHARGE_BF
 *  @ingroup LTC4015_register_map
 *  @brief OK_TO_CHARGE_BF Bit Field
 *
 *  Indicates all system conditions are met to allow battery charger operation.
 *   - Register: @ref LTC4015_SYSTEM_STATUS "SYSTEM_STATUS"
 *   - CommandCode: 57
 *   - Size: 1
 *   - Offset: 6
 *   - MSB: 6
 *   - MASK: 0x0040
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_OK_TO_CHARGE_BF_SUBADDR LTC4015_SYSTEM_STATUS_SUBADDR //!< @ref LTC4015_OK_TO_CHARGE_BF "OK_TO_CHARGE_BF"
#define LTC4015_OK_TO_CHARGE_BF_SIZE 1
#define LTC4015_OK_TO_CHARGE_BF_OFFSET 6
#define LTC4015_OK_TO_CHARGE_BF_MASK 0x0040
#define LTC4015_OK_TO_CHARGE_BF (LTC4015_OK_TO_CHARGE_BF_OFFSET << 12 | (LTC4015_OK_TO_CHARGE_BF_SIZE - 1) << 8 | LTC4015_OK_TO_CHARGE_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_NO_RT_BF NO_RT_BF
 *  @ingroup LTC4015_register_map
 *  @brief NO_RT_BF Bit Field
 *
 *  Indicates no resistor has been detected at the RT pin
 *   - Register: @ref LTC4015_SYSTEM_STATUS "SYSTEM_STATUS"
 *   - CommandCode: 57
 *   - Size: 1
 *   - Offset: 5
 *   - MSB: 5
 *   - MASK: 0x0020
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_NO_RT_BF_SUBADDR LTC4015_SYSTEM_STATUS_SUBADDR //!< @ref LTC4015_NO_RT_BF "NO_RT_BF"
#define LTC4015_NO_RT_BF_SIZE 1
#define LTC4015_NO_RT_BF_OFFSET 5
#define LTC4015_NO_RT_BF_MASK 0x0020
#define LTC4015_NO_RT_BF (LTC4015_NO_RT_BF_OFFSET << 12 | (LTC4015_NO_RT_BF_SIZE - 1) << 8 | LTC4015_NO_RT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_THERMAL_SHUTDOWN_BF THERMAL_SHUTDOWN_BF
 *  @ingroup LTC4015_register_map
 *  @brief THERMAL_SHUTDOWN_BF Bit Field
 *
 *  Indicates die temperature is greater than thermal shutdown level (160C typical)
 *   - Register: @ref LTC4015_SYSTEM_STATUS "SYSTEM_STATUS"
 *   - CommandCode: 57
 *   - Size: 1
 *   - Offset: 4
 *   - MSB: 4
 *   - MASK: 0x0010
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_THERMAL_SHUTDOWN_BF_SUBADDR LTC4015_SYSTEM_STATUS_SUBADDR //!< @ref LTC4015_THERMAL_SHUTDOWN_BF "THERMAL_SHUTDOWN_BF"
#define LTC4015_THERMAL_SHUTDOWN_BF_SIZE 1
#define LTC4015_THERMAL_SHUTDOWN_BF_OFFSET 4
#define LTC4015_THERMAL_SHUTDOWN_BF_MASK 0x0010
#define LTC4015_THERMAL_SHUTDOWN_BF (LTC4015_THERMAL_SHUTDOWN_BF_OFFSET << 12 | (LTC4015_THERMAL_SHUTDOWN_BF_SIZE - 1) << 8 | LTC4015_THERMAL_SHUTDOWN_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_VIN_OVLO_BF VIN_OVLO_BF
 *  @ingroup LTC4015_register_map
 *  @brief VIN_OVLO_BF Bit Field
 *
 *  Indicates VIN voltage is greater than overvoltage lockout level (38.6V typical)
 *   - Register: @ref LTC4015_SYSTEM_STATUS "SYSTEM_STATUS"
 *   - CommandCode: 57
 *   - Size: 1
 *   - Offset: 3
 *   - MSB: 3
 *   - MASK: 0x0008
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VIN_OVLO_BF_SUBADDR LTC4015_SYSTEM_STATUS_SUBADDR //!< @ref LTC4015_VIN_OVLO_BF "VIN_OVLO_BF"
#define LTC4015_VIN_OVLO_BF_SIZE 1
#define LTC4015_VIN_OVLO_BF_OFFSET 3
#define LTC4015_VIN_OVLO_BF_MASK 0x0008
#define LTC4015_VIN_OVLO_BF (LTC4015_VIN_OVLO_BF_OFFSET << 12 | (LTC4015_VIN_OVLO_BF_SIZE - 1) << 8 | LTC4015_VIN_OVLO_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_VIN_GT_VBAT_BF VIN_GT_VBAT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VIN_GT_VBAT_BF Bit Field
 *
 *  Indicates VIN voltage is sufficiently greater than BATSENSE for switching regulator operation (200mV typical)
 *   - Register: @ref LTC4015_SYSTEM_STATUS "SYSTEM_STATUS"
 *   - CommandCode: 57
 *   - Size: 1
 *   - Offset: 2
 *   - MSB: 2
 *   - MASK: 0x0004
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VIN_GT_VBAT_BF_SUBADDR LTC4015_SYSTEM_STATUS_SUBADDR //!< @ref LTC4015_VIN_GT_VBAT_BF "VIN_GT_VBAT_BF"
#define LTC4015_VIN_GT_VBAT_BF_SIZE 1
#define LTC4015_VIN_GT_VBAT_BF_OFFSET 2
#define LTC4015_VIN_GT_VBAT_BF_MASK 0x0004
#define LTC4015_VIN_GT_VBAT_BF (LTC4015_VIN_GT_VBAT_BF_OFFSET << 12 | (LTC4015_VIN_GT_VBAT_BF_SIZE - 1) << 8 | LTC4015_VIN_GT_VBAT_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_INTVCC_GT_4P3V_BF INTVCC_GT_4P3V_BF
 *  @ingroup LTC4015_register_map
 *  @brief INTVCC_GT_4P3V_BF Bit Field
 *
 *  Indicates INTVCC voltage is above switching regulator undervoltage lockout level (4.3V typ)
 *   - Register: @ref LTC4015_SYSTEM_STATUS "SYSTEM_STATUS"
 *   - CommandCode: 57
 *   - Size: 1
 *   - Offset: 1
 *   - MSB: 1
 *   - MASK: 0x0002
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_INTVCC_GT_4P3V_BF_SUBADDR LTC4015_SYSTEM_STATUS_SUBADDR //!< @ref LTC4015_INTVCC_GT_4P3V_BF "INTVCC_GT_4P3V_BF"
#define LTC4015_INTVCC_GT_4P3V_BF_SIZE 1
#define LTC4015_INTVCC_GT_4P3V_BF_OFFSET 1
#define LTC4015_INTVCC_GT_4P3V_BF_MASK 0x0002
#define LTC4015_INTVCC_GT_4P3V_BF (LTC4015_INTVCC_GT_4P3V_BF_OFFSET << 12 | (LTC4015_INTVCC_GT_4P3V_BF_SIZE - 1) << 8 | LTC4015_INTVCC_GT_4P3V_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_INTVCC_GT_2P8V_BF INTVCC_GT_2P8V_BF
 *  @ingroup LTC4015_register_map
 *  @brief INTVCC_GT_2P8V_BF Bit Field
 *
 *  Indicates INTVCC voltage is greater than measurement system lockout level (2.8V typical)
 *   - Register: @ref LTC4015_SYSTEM_STATUS "SYSTEM_STATUS"
 *   - CommandCode: 57
 *   - Size: 1
 *   - Offset: 0
 *   - MSB: 0
 *   - MASK: 0x0001
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_INTVCC_GT_2P8V_BF_SUBADDR LTC4015_SYSTEM_STATUS_SUBADDR //!< @ref LTC4015_INTVCC_GT_2P8V_BF "INTVCC_GT_2P8V_BF"
#define LTC4015_INTVCC_GT_2P8V_BF_SIZE 1
#define LTC4015_INTVCC_GT_2P8V_BF_OFFSET 0
#define LTC4015_INTVCC_GT_2P8V_BF_MASK 0x0001
#define LTC4015_INTVCC_GT_2P8V_BF (LTC4015_INTVCC_GT_2P8V_BF_OFFSET << 12 | (LTC4015_INTVCC_GT_2P8V_BF_SIZE - 1) << 8 | LTC4015_INTVCC_GT_2P8V_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VBAT VBAT
 *  @ingroup LTC4015_register_map
 *  @brief VBAT Register
 *
 * |    15:0 |
 * |:-------:|
 * | VBAT_BF |
 *
 *   - CommandCode: 58
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VBAT_BF "VBAT_BF" : VBATSENS/cellcount = [VBAT] • 192.264μV for lithium chemistries.
				VBATSENS/cellcount = [VBAT] • 128.176μV for lead-acid.
*/

//!@{
#define LTC4015_VBAT_SUBADDR 58
#define LTC4015_VBAT (0 << 12 | (16 - 1) << 8 | LTC4015_VBAT_SUBADDR)
//!@}
/*! @defgroup LTC4015_VBAT_BF VBAT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VBAT_BF Bit Field
 *
 *  VBATSENS/cellcount = [VBAT] • 192.264μV for lithium chemistries.
				VBATSENS/cellcount = [VBAT] • 128.176μV for lead-acid.
 *   - Register: @ref LTC4015_VBAT "VBAT"
 *   - CommandCode: 58
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VBAT_BF_SUBADDR LTC4015_VBAT_SUBADDR //!< @ref LTC4015_VBAT_BF "VBAT_BF"
#define LTC4015_VBAT_BF_SIZE 16
#define LTC4015_VBAT_BF_OFFSET 0
#define LTC4015_VBAT_BF_MASK 0xFFFF
#define LTC4015_VBAT_BF (LTC4015_VBAT_BF_OFFSET << 12 | (LTC4015_VBAT_BF_SIZE - 1) << 8 | LTC4015_VBAT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VIN VIN
 *  @ingroup LTC4015_register_map
 *  @brief VIN Register
 *
 * |   15:0 |
 * |:------:|
 * | VIN_BF |
 *
 *   - CommandCode: 59
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VIN_BF "VIN_BF" : Two's complement ADC measurement result for VIN. VVIN = [VIN] • 1.648mV
*/

//!@{
#define LTC4015_VIN_SUBADDR 59
#define LTC4015_VIN (0 << 12 | (16 - 1) << 8 | LTC4015_VIN_SUBADDR)
//!@}
/*! @defgroup LTC4015_VIN_BF VIN_BF
 *  @ingroup LTC4015_register_map
 *  @brief VIN_BF Bit Field
 *
 *  Two's complement ADC measurement result for VIN. VVIN = [VIN] • 1.648mV
 *   - Register: @ref LTC4015_VIN "VIN"
 *   - CommandCode: 59
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VIN_BF_SUBADDR LTC4015_VIN_SUBADDR //!< @ref LTC4015_VIN_BF "VIN_BF"
#define LTC4015_VIN_BF_SIZE 16
#define LTC4015_VIN_BF_OFFSET 0
#define LTC4015_VIN_BF_MASK 0xFFFF
#define LTC4015_VIN_BF (LTC4015_VIN_BF_OFFSET << 12 | (LTC4015_VIN_BF_SIZE - 1) << 8 | LTC4015_VIN_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VSYS VSYS
 *  @ingroup LTC4015_register_map
 *  @brief VSYS Register
 *
 * |    15:0 |
 * |:-------:|
 * | VSYS_BF |
 *
 *   - CommandCode: 60
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VSYS_BF "VSYS_BF" : Two's complement ADC measurement result for VSYS. VSYS = [VSYS] • 1.648mV
*/

//!@{
#define LTC4015_VSYS_SUBADDR 60
#define LTC4015_VSYS (0 << 12 | (16 - 1) << 8 | LTC4015_VSYS_SUBADDR)
//!@}
/*! @defgroup LTC4015_VSYS_BF VSYS_BF
 *  @ingroup LTC4015_register_map
 *  @brief VSYS_BF Bit Field
 *
 *  Two's complement ADC measurement result for VSYS. VSYS = [VSYS] • 1.648mV
 *   - Register: @ref LTC4015_VSYS "VSYS"
 *   - CommandCode: 60
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VSYS_BF_SUBADDR LTC4015_VSYS_SUBADDR //!< @ref LTC4015_VSYS_BF "VSYS_BF"
#define LTC4015_VSYS_BF_SIZE 16
#define LTC4015_VSYS_BF_OFFSET 0
#define LTC4015_VSYS_BF_MASK 0xFFFF
#define LTC4015_VSYS_BF (LTC4015_VSYS_BF_OFFSET << 12 | (LTC4015_VSYS_BF_SIZE - 1) << 8 | LTC4015_VSYS_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_IBAT IBAT
 *  @ingroup LTC4015_register_map
 *  @brief IBAT Register
 *
 * |    15:0 |
 * |:-------:|
 * | IBAT_BF |
 *
 *   - CommandCode: 61
 *   - Contains Bit Fields:
 *     + @ref LTC4015_IBAT_BF "IBAT_BF" : Two's complement ADC measurement result for battery charge current. Battery current = [IBAT] • 1.46487μV/RSNSB
*/

//!@{
#define LTC4015_IBAT_SUBADDR 61
#define LTC4015_IBAT (0 << 12 | (16 - 1) << 8 | LTC4015_IBAT_SUBADDR)
//!@}
/*! @defgroup LTC4015_IBAT_BF IBAT_BF
 *  @ingroup LTC4015_register_map
 *  @brief IBAT_BF Bit Field
 *
 *  Two's complement ADC measurement result for battery charge current. Battery current = [IBAT] • 1.46487μV/RSNSB
 *   - Register: @ref LTC4015_IBAT "IBAT"
 *   - CommandCode: 61
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_IBAT_BF_SUBADDR LTC4015_IBAT_SUBADDR //!< @ref LTC4015_IBAT_BF "IBAT_BF"
#define LTC4015_IBAT_BF_SIZE 16
#define LTC4015_IBAT_BF_OFFSET 0
#define LTC4015_IBAT_BF_MASK 0xFFFF
#define LTC4015_IBAT_BF (LTC4015_IBAT_BF_OFFSET << 12 | (LTC4015_IBAT_BF_SIZE - 1) << 8 | LTC4015_IBAT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_IIN IIN
 *  @ingroup LTC4015_register_map
 *  @brief IIN Register
 *
 * |   15:0 |
 * |:------:|
 * | IIN_BF |
 *
 *   - CommandCode: 62
 *   - Contains Bit Fields:
 *     + @ref LTC4015_IIN_BF "IIN_BF" : Two's complement ADC measurement result for (VCLP – VCLN). Input current = [IIN] • 1.46487μV/RSNSI * [18204][682656])
*/

//!@{
#define LTC4015_IIN_SUBADDR 62
#define LTC4015_IIN (0 << 12 | (16 - 1) << 8 | LTC4015_IIN_SUBADDR)
//!@}
/*! @defgroup LTC4015_IIN_BF IIN_BF
 *  @ingroup LTC4015_register_map
 *  @brief IIN_BF Bit Field
 *
 *  Two's complement ADC measurement result for (VCLP – VCLN). Input current = [IIN] • 1.46487μV/RSNSI * [18204][682656])
 *   - Register: @ref LTC4015_IIN "IIN"
 *   - CommandCode: 62
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_IIN_BF_SUBADDR LTC4015_IIN_SUBADDR //!< @ref LTC4015_IIN_BF "IIN_BF"
#define LTC4015_IIN_BF_SIZE 16
#define LTC4015_IIN_BF_OFFSET 0
#define LTC4015_IIN_BF_MASK 0xFFFF
#define LTC4015_IIN_BF (LTC4015_IIN_BF_OFFSET << 12 | (LTC4015_IIN_BF_SIZE - 1) << 8 | LTC4015_IIN_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_DIE_TEMP DIE_TEMP
 *  @ingroup LTC4015_register_map
 *  @brief DIE_TEMP Register
 *
 * |        15:0 |
 * |:-----------:|
 * | DIE_TEMP_BF |
 *
 *   - CommandCode: 63
 *   - Contains Bit Fields:
 *     + @ref LTC4015_DIE_TEMP_BF "DIE_TEMP_BF" : Two's complement ADC measurement result for die temperature. Temperature = (DIE_TEMP – 12010)/45.6°C
*/

//!@{
#define LTC4015_DIE_TEMP_SUBADDR 63
#define LTC4015_DIE_TEMP (0 << 12 | (16 - 1) << 8 | LTC4015_DIE_TEMP_SUBADDR)
//!@}
/*! @defgroup LTC4015_DIE_TEMP_BF DIE_TEMP_BF
 *  @ingroup LTC4015_register_map
 *  @brief DIE_TEMP_BF Bit Field
 *
 *  Two's complement ADC measurement result for die temperature. Temperature = (DIE_TEMP – 12010)/45.6°C
 *   - Register: @ref LTC4015_DIE_TEMP "DIE_TEMP"
 *   - CommandCode: 63
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_DIE_TEMP_BF_SUBADDR LTC4015_DIE_TEMP_SUBADDR //!< @ref LTC4015_DIE_TEMP_BF "DIE_TEMP_BF"
#define LTC4015_DIE_TEMP_BF_SIZE 16
#define LTC4015_DIE_TEMP_BF_OFFSET 0
#define LTC4015_DIE_TEMP_BF_MASK 0xFFFF
#define LTC4015_DIE_TEMP_BF (LTC4015_DIE_TEMP_BF_OFFSET << 12 | (LTC4015_DIE_TEMP_BF_SIZE - 1) << 8 | LTC4015_DIE_TEMP_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_NTC_RATIO NTC_RATIO
 *  @ingroup LTC4015_register_map
 *  @brief NTC_RATIO Register
 *
 * |         15:0 |
 * |:------------:|
 * | NTC_RATIO_BF |
 *
 *   - CommandCode: 64
 *   - Contains Bit Fields:
 *     + @ref LTC4015_NTC_RATIO_BF "NTC_RATIO_BF" : ADC measurement result for NTC thermistor ratio. R<sub>NTC</sub> = NTC_RATIO * R<sub>NTCBIAS</sub> / (21845 - NTC_RATIO)
*/

//!@{
#define LTC4015_NTC_RATIO_SUBADDR 64
#define LTC4015_NTC_RATIO (0 << 12 | (16 - 1) << 8 | LTC4015_NTC_RATIO_SUBADDR)
//!@}
/*! @defgroup LTC4015_NTC_RATIO_BF NTC_RATIO_BF
 *  @ingroup LTC4015_register_map
 *  @brief NTC_RATIO_BF Bit Field
 *
 *  ADC measurement result for NTC thermistor ratio. R<sub>NTC</sub> = NTC_RATIO * R<sub>NTCBIAS</sub> / (21845 - NTC_RATIO)
 *   - Register: @ref LTC4015_NTC_RATIO "NTC_RATIO"
 *   - CommandCode: 64
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_NTC_RATIO_BF_SUBADDR LTC4015_NTC_RATIO_SUBADDR //!< @ref LTC4015_NTC_RATIO_BF "NTC_RATIO_BF"
#define LTC4015_NTC_RATIO_BF_SIZE 16
#define LTC4015_NTC_RATIO_BF_OFFSET 0
#define LTC4015_NTC_RATIO_BF_MASK 0xFFFF
#define LTC4015_NTC_RATIO_BF (LTC4015_NTC_RATIO_BF_OFFSET << 12 | (LTC4015_NTC_RATIO_BF_SIZE - 1) << 8 | LTC4015_NTC_RATIO_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_BSR BSR
 *  @ingroup LTC4015_register_map
 *  @brief BSR Register
 *
 * |   15:0 |
 * |:------:|
 * | BSR_BF |
 *
 *   - CommandCode: 65
 *   - Contains Bit Fields:
 *     + @ref LTC4015_BSR_BF "BSR_BF" : Calculated battery series resistance.  Series resistance = BSR * R<sub>PROG</sub> / 500
*/

//!@{
#define LTC4015_BSR_SUBADDR 65
#define LTC4015_BSR (0 << 12 | (16 - 1) << 8 | LTC4015_BSR_SUBADDR)
//!@}
/*! @defgroup LTC4015_BSR_BF BSR_BF
 *  @ingroup LTC4015_register_map
 *  @brief BSR_BF Bit Field
 *
 *  Calculated battery series resistance.  Series resistance = BSR * R<sub>PROG</sub> / 500
 *   - Register: @ref LTC4015_BSR "BSR"
 *   - CommandCode: 65
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_BSR_BF_SUBADDR LTC4015_BSR_SUBADDR //!< @ref LTC4015_BSR_BF "BSR_BF"
#define LTC4015_BSR_BF_SIZE 16
#define LTC4015_BSR_BF_OFFSET 0
#define LTC4015_BSR_BF_MASK 0xFFFF
#define LTC4015_BSR_BF (LTC4015_BSR_BF_OFFSET << 12 | (LTC4015_BSR_BF_SIZE - 1) << 8 | LTC4015_BSR_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_JEITA_REGION JEITA_REGION
 *  @ingroup LTC4015_register_map
 *  @brief JEITA_REGION Register
 *
 * | 15:3 |             2:0 |
 * |:----:|:---------------:|
 * |  n/a | JEITA_REGION_BF |
 *
 *   - CommandCode: 66
 *   - Contains Bit Fields:
 *     + @ref LTC4015_JEITA_REGION_BF "JEITA_REGION_BF" : JEITA temperature region of the NTC thermistor[(Li Only)]. Active only when EN_JEITA=1
*/

//!@{
#define LTC4015_JEITA_REGION_SUBADDR 66
#define LTC4015_JEITA_REGION (0 << 12 | (16 - 1) << 8 | LTC4015_JEITA_REGION_SUBADDR)
//!@}
/*! @defgroup LTC4015_JEITA_REGION_BF JEITA_REGION_BF
 *  @ingroup LTC4015_register_map
 *  @brief JEITA_REGION_BF Bit Field
 *
 *  JEITA temperature region of the NTC thermistor[(Li Only)]. Active only when EN_JEITA=1
 *   - Register: @ref LTC4015_JEITA_REGION "JEITA_REGION"
 *   - CommandCode: 66
 *   - Size: 3
 *   - Offset: 0
 *   - MSB: 2
 *   - MASK: 0x0007
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_JEITA_REGION_BF_SUBADDR LTC4015_JEITA_REGION_SUBADDR //!< @ref LTC4015_JEITA_REGION_BF "JEITA_REGION_BF"
#define LTC4015_JEITA_REGION_BF_SIZE 3
#define LTC4015_JEITA_REGION_BF_OFFSET 0
#define LTC4015_JEITA_REGION_BF_MASK 0x0007
#define LTC4015_JEITA_REGION_BF (LTC4015_JEITA_REGION_BF_OFFSET << 12 | (LTC4015_JEITA_REGION_BF_SIZE - 1) << 8 | LTC4015_JEITA_REGION_BF_SUBADDR)
#define LTC4015_JEITA_REGION_BF_PRESET_T1 1
#define LTC4015_JEITA_REGION_BF_PRESET_T2 2
#define LTC4015_JEITA_REGION_BF_PRESET_T3 3
#define LTC4015_JEITA_REGION_BF_PRESET_T4 4
#define LTC4015_JEITA_REGION_BF_PRESET_T5 5
#define LTC4015_JEITA_REGION_BF_PRESET_T6 6
#define LTC4015_JEITA_REGION_BF_PRESET_T7 7
//!@}

/*! @defgroup LTC4015_CHEM_CELLS CHEM_CELLS
 *  @ingroup LTC4015_register_map
 *  @brief CHEM_CELLS Register
 *
 * | 15:12 |    11:8 |              7:4 |                3:0 |
 * |:-----:|:-------:|:----------------:|:------------------:|
 * |   n/a | CHEM_BF | RESERVED_0X67_BF | CELL_COUNT_PINS_BF |
 *
 * Readout of CHEM and CELLS pin settings
 *   - CommandCode: 67
 *   - Contains Bit Fields:
 *     + @ref LTC4015_CHEM_BF "CHEM_BF" : programmed battery chemistry
 *     + @ref LTC4015_RESERVED_0X67_BF "RESERVED_0X67_BF" : Reserved
 *     + @ref LTC4015_CELL_COUNT_PINS_BF "CELL_COUNT_PINS_BF" : Cell count as set by CELLS pins
*/

//!@{
#define LTC4015_CHEM_CELLS_SUBADDR 67
#define LTC4015_CHEM_CELLS (0 << 12 | (16 - 1) << 8 | LTC4015_CHEM_CELLS_SUBADDR)
//!@}
/*! @defgroup LTC4015_CHEM_BF CHEM_BF
 *  @ingroup LTC4015_register_map
 *  @brief CHEM_BF Bit Field
 *
 *  programmed battery chemistry
 *   - Register: @ref LTC4015_CHEM_CELLS "CHEM_CELLS"
 *   - CommandCode: 67
 *   - Size: 4
 *   - Offset: 8
 *   - MSB: 11
 *   - MASK: 0x0F00
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_CHEM_BF_SUBADDR LTC4015_CHEM_CELLS_SUBADDR //!< @ref LTC4015_CHEM_BF "CHEM_BF"
#define LTC4015_CHEM_BF_SIZE 4
#define LTC4015_CHEM_BF_OFFSET 8
#define LTC4015_CHEM_BF_MASK 0x0F00
#define LTC4015_CHEM_BF (LTC4015_CHEM_BF_OFFSET << 12 | (LTC4015_CHEM_BF_SIZE - 1) << 8 | LTC4015_CHEM_BF_SUBADDR)
#define LTC4015_CHEM_BF_PRESET_LI_ION_PROGRAMMABLE 0
#define LTC4015_CHEM_BF_PRESET_LI_ION_FIXED_4P2V 1
#define LTC4015_CHEM_BF_PRESET_LI_ION_FIXED_4P1V 2
#define LTC4015_CHEM_BF_PRESET_LI_ION_FIXED_4P0V 3
#define LTC4015_CHEM_BF_PRESET_LIFEPO4_PROGRAMMABLE 4
#define LTC4015_CHEM_BF_PRESET_LIFEPO4_FIXED_FAST_CHARGE 5
#define LTC4015_CHEM_BF_PRESET_LIFEPO4_FIXED_3P6V 6
#define LTC4015_CHEM_BF_PRESET_LEAD_ACID_FIXED 7
#define LTC4015_CHEM_BF_PRESET_LEAD_ACID_PROGRAMMABLE 8
//!@}
/*! @defgroup LTC4015_RESERVED_0X67_BF RESERVED_0X67_BF
 *  @ingroup LTC4015_register_map
 *  @brief RESERVED_0X67_BF Bit Field
 *
 *  Reserved
 *   - Register: @ref LTC4015_CHEM_CELLS "CHEM_CELLS"
 *   - CommandCode: 67
 *   - Size: 4
 *   - Offset: 4
 *   - MSB: 7
 *   - MASK: 0x00F0
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_RESERVED_0X67_BF_SUBADDR LTC4015_CHEM_CELLS_SUBADDR //!< @ref LTC4015_RESERVED_0X67_BF "RESERVED_0X67_BF"
#define LTC4015_RESERVED_0X67_BF_SIZE 4
#define LTC4015_RESERVED_0X67_BF_OFFSET 4
#define LTC4015_RESERVED_0X67_BF_MASK 0x00F0
#define LTC4015_RESERVED_0X67_BF (LTC4015_RESERVED_0X67_BF_OFFSET << 12 | (LTC4015_RESERVED_0X67_BF_SIZE - 1) << 8 | LTC4015_RESERVED_0X67_BF_SUBADDR)
//!@}
/*! @defgroup LTC4015_CELL_COUNT_PINS_BF CELL_COUNT_PINS_BF
 *  @ingroup LTC4015_register_map
 *  @brief CELL_COUNT_PINS_BF Bit Field
 *
 *  Cell count as set by CELLS pins
 *   - Register: @ref LTC4015_CHEM_CELLS "CHEM_CELLS"
 *   - CommandCode: 67
 *   - Size: 4
 *   - Offset: 0
 *   - MSB: 3
 *   - MASK: 0x000F
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_CELL_COUNT_PINS_BF_SUBADDR LTC4015_CHEM_CELLS_SUBADDR //!< @ref LTC4015_CELL_COUNT_PINS_BF "CELL_COUNT_PINS_BF"
#define LTC4015_CELL_COUNT_PINS_BF_SIZE 4
#define LTC4015_CELL_COUNT_PINS_BF_OFFSET 0
#define LTC4015_CELL_COUNT_PINS_BF_MASK 0x000F
#define LTC4015_CELL_COUNT_PINS_BF (LTC4015_CELL_COUNT_PINS_BF_OFFSET << 12 | (LTC4015_CELL_COUNT_PINS_BF_SIZE - 1) << 8 | LTC4015_CELL_COUNT_PINS_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_ICHARGE_DAC ICHARGE_DAC
 *  @ingroup LTC4015_register_map
 *  @brief ICHARGE_DAC Register
 *
 * | 15:5 |            4:0 |
 * |:----:|:--------------:|
 * |  n/a | ICHARGE_DAC_BF |
 *
 *   - CommandCode: 68
 *   - Contains Bit Fields:
 *     + @ref LTC4015_ICHARGE_DAC_BF "ICHARGE_DAC_BF" : Charge current control DAC control bits
*/

//!@{
#define LTC4015_ICHARGE_DAC_SUBADDR 68
#define LTC4015_ICHARGE_DAC (0 << 12 | (16 - 1) << 8 | LTC4015_ICHARGE_DAC_SUBADDR)
//!@}
/*! @defgroup LTC4015_ICHARGE_DAC_BF ICHARGE_DAC_BF
 *  @ingroup LTC4015_register_map
 *  @brief ICHARGE_DAC_BF Bit Field
 *
 *  Charge current control DAC control bits
 *   - Register: @ref LTC4015_ICHARGE_DAC "ICHARGE_DAC"
 *   - CommandCode: 68
 *   - Size: 5
 *   - Offset: 0
 *   - MSB: 4
 *   - MASK: 0x001F
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_ICHARGE_DAC_BF_SUBADDR LTC4015_ICHARGE_DAC_SUBADDR //!< @ref LTC4015_ICHARGE_DAC_BF "ICHARGE_DAC_BF"
#define LTC4015_ICHARGE_DAC_BF_SIZE 5
#define LTC4015_ICHARGE_DAC_BF_OFFSET 0
#define LTC4015_ICHARGE_DAC_BF_MASK 0x001F
#define LTC4015_ICHARGE_DAC_BF (LTC4015_ICHARGE_DAC_BF_OFFSET << 12 | (LTC4015_ICHARGE_DAC_BF_SIZE - 1) << 8 | LTC4015_ICHARGE_DAC_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VCHARGE_DAC VCHARGE_DAC
 *  @ingroup LTC4015_register_map
 *  @brief VCHARGE_DAC Register
 *
 * | 15:6 |            5:0 |
 * |:----:|:--------------:|
 * |  n/a | VCHARGE_DAC_BF |
 *
 *   - CommandCode: 69
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VCHARGE_DAC_BF "VCHARGE_DAC_BF" : Charge voltage control DAC control bits
*/

//!@{
#define LTC4015_VCHARGE_DAC_SUBADDR 69
#define LTC4015_VCHARGE_DAC (0 << 12 | (16 - 1) << 8 | LTC4015_VCHARGE_DAC_SUBADDR)
//!@}
/*! @defgroup LTC4015_VCHARGE_DAC_BF VCHARGE_DAC_BF
 *  @ingroup LTC4015_register_map
 *  @brief VCHARGE_DAC_BF Bit Field
 *
 *  Charge voltage control DAC control bits
 *   - Register: @ref LTC4015_VCHARGE_DAC "VCHARGE_DAC"
 *   - CommandCode: 69
 *   - Size: 6
 *   - Offset: 0
 *   - MSB: 5
 *   - MASK: 0x003F
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VCHARGE_DAC_BF_SUBADDR LTC4015_VCHARGE_DAC_SUBADDR //!< @ref LTC4015_VCHARGE_DAC_BF "VCHARGE_DAC_BF"
#define LTC4015_VCHARGE_DAC_BF_SIZE 6
#define LTC4015_VCHARGE_DAC_BF_OFFSET 0
#define LTC4015_VCHARGE_DAC_BF_MASK 0x003F
#define LTC4015_VCHARGE_DAC_BF (LTC4015_VCHARGE_DAC_BF_OFFSET << 12 | (LTC4015_VCHARGE_DAC_BF_SIZE - 1) << 8 | LTC4015_VCHARGE_DAC_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_IIN_LIMIT_DAC IIN_LIMIT_DAC
 *  @ingroup LTC4015_register_map
 *  @brief IIN_LIMIT_DAC Register
 *
 * | 15:6 |              5:0 |
 * |:----:|:----------------:|
 * |  n/a | IIN_LIMIT_DAC_BF |
 *
 *   - CommandCode: 70
 *   - Contains Bit Fields:
 *     + @ref LTC4015_IIN_LIMIT_DAC_BF "IIN_LIMIT_DAC_BF" : Input current limit control DAC control word
*/

//!@{
#define LTC4015_IIN_LIMIT_DAC_SUBADDR 70
#define LTC4015_IIN_LIMIT_DAC (0 << 12 | (16 - 1) << 8 | LTC4015_IIN_LIMIT_DAC_SUBADDR)
//!@}
/*! @defgroup LTC4015_IIN_LIMIT_DAC_BF IIN_LIMIT_DAC_BF
 *  @ingroup LTC4015_register_map
 *  @brief IIN_LIMIT_DAC_BF Bit Field
 *
 *  Input current limit control DAC control word
 *   - Register: @ref LTC4015_IIN_LIMIT_DAC "IIN_LIMIT_DAC"
 *   - CommandCode: 70
 *   - Size: 6
 *   - Offset: 0
 *   - MSB: 5
 *   - MASK: 0x003F
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_IIN_LIMIT_DAC_BF_SUBADDR LTC4015_IIN_LIMIT_DAC_SUBADDR //!< @ref LTC4015_IIN_LIMIT_DAC_BF "IIN_LIMIT_DAC_BF"
#define LTC4015_IIN_LIMIT_DAC_BF_SIZE 6
#define LTC4015_IIN_LIMIT_DAC_BF_OFFSET 0
#define LTC4015_IIN_LIMIT_DAC_BF_MASK 0x003F
#define LTC4015_IIN_LIMIT_DAC_BF (LTC4015_IIN_LIMIT_DAC_BF_OFFSET << 12 | (LTC4015_IIN_LIMIT_DAC_BF_SIZE - 1) << 8 | LTC4015_IIN_LIMIT_DAC_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_VBAT_FILT VBAT_FILT
 *  @ingroup LTC4015_register_map
 *  @brief VBAT_FILT Register
 *
 * |         15:0 |
 * |:------------:|
 * | VBAT_FILT_BF |
 *
 *   - CommandCode: 71
 *   - Contains Bit Fields:
 *     + @ref LTC4015_VBAT_FILT_BF "VBAT_FILT_BF" : Digitally filtered ADC measurement result for battery voltage
*/

//!@{
#define LTC4015_VBAT_FILT_SUBADDR 71
#define LTC4015_VBAT_FILT (0 << 12 | (16 - 1) << 8 | LTC4015_VBAT_FILT_SUBADDR)
//!@}
/*! @defgroup LTC4015_VBAT_FILT_BF VBAT_FILT_BF
 *  @ingroup LTC4015_register_map
 *  @brief VBAT_FILT_BF Bit Field
 *
 *  Digitally filtered ADC measurement result for battery voltage
 *   - Register: @ref LTC4015_VBAT_FILT "VBAT_FILT"
 *   - CommandCode: 71
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_VBAT_FILT_BF_SUBADDR LTC4015_VBAT_FILT_SUBADDR //!< @ref LTC4015_VBAT_FILT_BF "VBAT_FILT_BF"
#define LTC4015_VBAT_FILT_BF_SIZE 16
#define LTC4015_VBAT_FILT_BF_OFFSET 0
#define LTC4015_VBAT_FILT_BF_MASK 0xFFFF
#define LTC4015_VBAT_FILT_BF (LTC4015_VBAT_FILT_BF_OFFSET << 12 | (LTC4015_VBAT_FILT_BF_SIZE - 1) << 8 | LTC4015_VBAT_FILT_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_ICHARGE_BSR ICHARGE_BSR
 *  @ingroup LTC4015_register_map
 *  @brief ICHARGE_BSR Register
 *
 * |           15:0 |
 * |:--------------:|
 * | ICHARGE_BSR_BF |
 *
 *   - CommandCode: 72
 *   - Contains Bit Fields:
 *     + @ref LTC4015_ICHARGE_BSR_BF "ICHARGE_BSR_BF" : This 16-bit two's complement word is the value of IBAT (0x3D) used in calculating BSR.
*/

//!@{
#define LTC4015_ICHARGE_BSR_SUBADDR 72
#define LTC4015_ICHARGE_BSR (0 << 12 | (16 - 1) << 8 | LTC4015_ICHARGE_BSR_SUBADDR)
//!@}
/*! @defgroup LTC4015_ICHARGE_BSR_BF ICHARGE_BSR_BF
 *  @ingroup LTC4015_register_map
 *  @brief ICHARGE_BSR_BF Bit Field
 *
 *  This 16-bit two's complement word is the value of IBAT (0x3D) used in calculating BSR.
 *   - Register: @ref LTC4015_ICHARGE_BSR "ICHARGE_BSR"
 *   - CommandCode: 72
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_ICHARGE_BSR_BF_SUBADDR LTC4015_ICHARGE_BSR_SUBADDR //!< @ref LTC4015_ICHARGE_BSR_BF "ICHARGE_BSR_BF"
#define LTC4015_ICHARGE_BSR_BF_SIZE 16
#define LTC4015_ICHARGE_BSR_BF_OFFSET 0
#define LTC4015_ICHARGE_BSR_BF_MASK 0xFFFF
#define LTC4015_ICHARGE_BSR_BF (LTC4015_ICHARGE_BSR_BF_OFFSET << 12 | (LTC4015_ICHARGE_BSR_BF_SIZE - 1) << 8 | LTC4015_ICHARGE_BSR_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_RESERVED_0X49 RESERVED_0X49
 *  @ingroup LTC4015_register_map
 *  @brief RESERVED_0X49 Register
 *
 * |             15:0 |
 * |:----------------:|
 * | RESERVED_0X49_BF |
 *
 *   - CommandCode: 73
 *   - Contains Bit Fields:
 *     + @ref LTC4015_RESERVED_0X49_BF "RESERVED_0X49_BF" : RESERVED
*/

//!@{
#define LTC4015_RESERVED_0X49_SUBADDR 73
#define LTC4015_RESERVED_0X49 (0 << 12 | (16 - 1) << 8 | LTC4015_RESERVED_0X49_SUBADDR)
//!@}
/*! @defgroup LTC4015_RESERVED_0X49_BF RESERVED_0X49_BF
 *  @ingroup LTC4015_register_map
 *  @brief RESERVED_0X49_BF Bit Field
 *
 *  RESERVED
 *   - Register: @ref LTC4015_RESERVED_0X49 "RESERVED_0X49"
 *   - CommandCode: 73
 *   - Size: 16
 *   - Offset: 0
 *   - MSB: 15
 *   - MASK: 0xFFFF
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_RESERVED_0X49_BF_SUBADDR LTC4015_RESERVED_0X49_SUBADDR //!< @ref LTC4015_RESERVED_0X49_BF "RESERVED_0X49_BF"
#define LTC4015_RESERVED_0X49_BF_SIZE 16
#define LTC4015_RESERVED_0X49_BF_OFFSET 0
#define LTC4015_RESERVED_0X49_BF_MASK 0xFFFF
#define LTC4015_RESERVED_0X49_BF (LTC4015_RESERVED_0X49_BF_OFFSET << 12 | (LTC4015_RESERVED_0X49_BF_SIZE - 1) << 8 | LTC4015_RESERVED_0X49_BF_SUBADDR)
//!@}

/*! @defgroup LTC4015_MEAS_SYS_VALID MEAS_SYS_VALID
 *  @ingroup LTC4015_register_map
 *  @brief MEAS_SYS_VALID Register
 *
 * | 15:1 |                 0 |
 * |:----:|:-----------------:|
 * |  n/a | MEAS_SYS_VALID_BF |
 *
 *   - CommandCode: 74
 *   - Contains Bit Fields:
 *     + @ref LTC4015_MEAS_SYS_VALID_BF "MEAS_SYS_VALID_BF" : Measurement valid bit, bit 0 is a 1 when the telemetry(ADC) system is ready
*/

//!@{
#define LTC4015_MEAS_SYS_VALID_SUBADDR 74
#define LTC4015_MEAS_SYS_VALID (0 << 12 | (16 - 1) << 8 | LTC4015_MEAS_SYS_VALID_SUBADDR)
//!@}
/*! @defgroup LTC4015_MEAS_SYS_VALID_BF MEAS_SYS_VALID_BF
 *  @ingroup LTC4015_register_map
 *  @brief MEAS_SYS_VALID_BF Bit Field
 *
 *  Measurement valid bit, bit 0 is a 1 when the telemetry(ADC) system is ready
 *   - Register: @ref LTC4015_MEAS_SYS_VALID "MEAS_SYS_VALID"
 *   - CommandCode: 74
 *   - Size: 1
 *   - Offset: 0
 *   - MSB: 0
 *   - MASK: 0x0001
 *   - Access: R
 *   - Default: n/a
 */
//!@{
#define LTC4015_MEAS_SYS_VALID_BF_SUBADDR LTC4015_MEAS_SYS_VALID_SUBADDR //!< @ref LTC4015_MEAS_SYS_VALID_BF "MEAS_SYS_VALID_BF"
#define LTC4015_MEAS_SYS_VALID_BF_SIZE 1
#define LTC4015_MEAS_SYS_VALID_BF_OFFSET 0
#define LTC4015_MEAS_SYS_VALID_BF_MASK 0x0001
#define LTC4015_MEAS_SYS_VALID_BF (LTC4015_MEAS_SYS_VALID_BF_OFFSET << 12 | (LTC4015_MEAS_SYS_VALID_BF_SIZE - 1) << 8 | LTC4015_MEAS_SYS_VALID_BF_SUBADDR)
//!@}

#endif
