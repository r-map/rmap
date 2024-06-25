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
 *  @brief LTC4015 library file defining data conversion macros and constants used
 *  by LTC4015.c
 *
 *
 *  This file contains macros and constants which can be used to represent real-world
 *  values in the source code, with automatic compile-time conversion to LTC4015
 *  internal register scaling.
 *
 *  Passing runtime variables to these macros should be avoided as it will likely result
 *  in runtime calculations accompanied by associated processor loading and memory usage.
 */

#ifndef LTC4015_FORMATS_H_
#define LTC4015_FORMATS_H_

/*! @name Format Definitions
 *  Constants used by the macros below to convert from real world to LTC4015 referenced numbers.
 */
/*! @name Constants used in real world conversion macros below
 */
#define LTC4015_RSNSI 0.005
#define LTC4015_RSNSB 0.005
#define LTC4015_RNTCBIAS 10000.0
#define LTC4015_RNTCSER 0.0
#define LTC4015_VINDIV 30.0
#define LTC4015_SYSDIV 30.0
#define LTC4015_BATDIV 3.5
#define LTC4015_AVPROG 37.5
#define LTC4015_AVCLPROG 37.5
#define LTC4015_ADCGAIN 18204.1667
#define LTC4015_VREF 1.2
#define LTC4015_Rm40 214063.67
#define LTC4015_Rm34 152840.30
#define LTC4015_Rm28 110480.73
#define LTC4015_Rm21 76798.02
#define LTC4015_Rm14 54214.99
#define LTC4015_Rm6 37075.65
#define LTC4015_R4 23649.71
#define LTC4015_R33 7400.97
#define LTC4015_R44 5001.22
#define LTC4015_R53 3693.55
#define LTC4015_R62 2768.21
#define LTC4015_R70 2167.17
#define LTC4015_R78 1714.08
#define LTC4015_R86 1368.87
#define LTC4015_R94 1103.18
#define LTC4015_R102 896.73
#define LTC4015_R110 734.86
#define LTC4015_R118 606.86
#define LTC4015_R126 504.80
#define LTC4015_R134 422.81
#define LTC4015_R142 356.45
#define LTC4015_R150 302.36
/*! @name Use the macros below to convert from real world to LTC4015 referenced numbers.
 */
/*! Convert from amperes to the iinlim setting. */
#define LTC4015_IINLIM(x) __LTC4015_LINE__((LTC4015_VREF / 64 / LTC4015_AVCLPROG / LTC4015_RSNSI), (LTC4015_VREF / 64 / LTC4015_AVCLPROG / LTC4015_RSNSI * 2), (0), (1), x)
/*! Convert from volts to the vcharge_liion setting. */
#define LTC4015_VCHARGE_LIION(x) __LTC4015_LINE__((3.8125), (3.8125 + 0.0125), (0), (1), x)
/*! Convert from volts to the vcharge_life setting. */
#define LTC4015_VCHARGE_LIFE(x) __LTC4015_LINE__((3.4125), (3.4125 + 0.0125), (0), (1), x)
/*! Convert from volts to the vcharge_sla setting. */
#define LTC4015_VCHARGE_SLA(x) __LTC4015_LINE__((6), (6 + 0.028125), (0), (1), x)
/*! Convert from °C to the thermistor ADC reading. */
#define LTC4015_NTCS0402E3103FLT(x) (\
__LTC4015_BETWEEN_INCLUSIVE__((-40), (-34), x) ? __LTC4015_LINE__((-40), (-34), ((LTC4015_Rm40 + LTC4015_RNTCSER) / (LTC4015_Rm40 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_Rm34 + LTC4015_RNTCSER) / (LTC4015_Rm34 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((-34), (-28), x) ? __LTC4015_LINE__((-34), (-28), ((LTC4015_Rm34 + LTC4015_RNTCSER) / (LTC4015_Rm34 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_Rm28 + LTC4015_RNTCSER) / (LTC4015_Rm28 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((-28), (-21), x) ? __LTC4015_LINE__((-28), (-21), ((LTC4015_Rm28 + LTC4015_RNTCSER) / (LTC4015_Rm28 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_Rm21 + LTC4015_RNTCSER) / (LTC4015_Rm21 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((-21), (-14), x) ? __LTC4015_LINE__((-21), (-14), ((LTC4015_Rm21 + LTC4015_RNTCSER) / (LTC4015_Rm21 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_Rm14 + LTC4015_RNTCSER) / (LTC4015_Rm14 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((-14), (-6), x) ? __LTC4015_LINE__((-14), (-6), ((LTC4015_Rm14 + LTC4015_RNTCSER) / (LTC4015_Rm14 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_Rm6 + LTC4015_RNTCSER) / (LTC4015_Rm6 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((-6), (4), x) ? __LTC4015_LINE__((-6), (4), ((LTC4015_Rm6 + LTC4015_RNTCSER) / (LTC4015_Rm6 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R4 + LTC4015_RNTCSER) / (LTC4015_R4 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((4), (33), x) ? __LTC4015_LINE__((4), (33), ((LTC4015_R4 + LTC4015_RNTCSER) / (LTC4015_R4 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R33 + LTC4015_RNTCSER) / (LTC4015_R33 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((33), (44), x) ? __LTC4015_LINE__((33), (44), ((LTC4015_R33 + LTC4015_RNTCSER) / (LTC4015_R33 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R44 + LTC4015_RNTCSER) / (LTC4015_R44 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((44), (53), x) ? __LTC4015_LINE__((44), (53), ((LTC4015_R44 + LTC4015_RNTCSER) / (LTC4015_R44 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R53 + LTC4015_RNTCSER) / (LTC4015_R53 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((53), (62), x) ? __LTC4015_LINE__((53), (62), ((LTC4015_R53 + LTC4015_RNTCSER) / (LTC4015_R53 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R62 + LTC4015_RNTCSER) / (LTC4015_R62 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((62), (70), x) ? __LTC4015_LINE__((62), (70), ((LTC4015_R62 + LTC4015_RNTCSER) / (LTC4015_R62 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R70 + LTC4015_RNTCSER) / (LTC4015_R70 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((70), (78), x) ? __LTC4015_LINE__((70), (78), ((LTC4015_R70 + LTC4015_RNTCSER) / (LTC4015_R70 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R78 + LTC4015_RNTCSER) / (LTC4015_R78 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((78), (86), x) ? __LTC4015_LINE__((78), (86), ((LTC4015_R78 + LTC4015_RNTCSER) / (LTC4015_R78 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R86 + LTC4015_RNTCSER) / (LTC4015_R86 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((86), (94), x) ? __LTC4015_LINE__((86), (94), ((LTC4015_R86 + LTC4015_RNTCSER) / (LTC4015_R86 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R94 + LTC4015_RNTCSER) / (LTC4015_R94 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((94), (102), x) ? __LTC4015_LINE__((94), (102), ((LTC4015_R94 + LTC4015_RNTCSER) / (LTC4015_R94 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R102 + LTC4015_RNTCSER) / (LTC4015_R102 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((102), (110), x) ? __LTC4015_LINE__((102), (110), ((LTC4015_R102 + LTC4015_RNTCSER) / (LTC4015_R102 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R110 + LTC4015_RNTCSER) / (LTC4015_R110 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((110), (118), x) ? __LTC4015_LINE__((110), (118), ((LTC4015_R110 + LTC4015_RNTCSER) / (LTC4015_R110 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R118 + LTC4015_RNTCSER) / (LTC4015_R118 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((118), (126), x) ? __LTC4015_LINE__((118), (126), ((LTC4015_R118 + LTC4015_RNTCSER) / (LTC4015_R118 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R126 + LTC4015_RNTCSER) / (LTC4015_R126 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((126), (134), x) ? __LTC4015_LINE__((126), (134), ((LTC4015_R126 + LTC4015_RNTCSER) / (LTC4015_R126 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R134 + LTC4015_RNTCSER) / (LTC4015_R134 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((134), (142), x) ? __LTC4015_LINE__((134), (142), ((LTC4015_R134 + LTC4015_RNTCSER) / (LTC4015_R134 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R142 + LTC4015_RNTCSER) / (LTC4015_R142 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : \
__LTC4015_BETWEEN_INCLUSIVE__((142), (150), x) ? __LTC4015_LINE__((142), (150), ((LTC4015_R142 + LTC4015_RNTCSER) / (LTC4015_R142 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), ((LTC4015_R150 + LTC4015_RNTCSER) / (LTC4015_R150 + LTC4015_RNTCSER + LTC4015_RNTCBIAS) * LTC4015_ADCGAIN * LTC4015_VREF), x) : -1)
/*! Convert from volts to the vin_uvcl setting. */
#define LTC4015_VIN_UVCL(x) __LTC4015_LINE__((LTC4015_VREF / 256 * LTC4015_VINDIV), (LTC4015_VREF / 256 * LTC4015_VINDIV * 2), (0), (1), x)
/*! Convert from amperes to the icharge setting. */
#define LTC4015_ICHARGE(x) __LTC4015_LINE__((LTC4015_VREF / 32 / LTC4015_AVPROG / LTC4015_RSNSB), (LTC4015_VREF / 32 / LTC4015_AVPROG / LTC4015_RSNSB * 2), (0), (1), x)
/*! Convert from volts to the Lithium Chemistry per-cell vbat ADC reading. */
#define LTC4015_VBAT_LITHIUM_FORMAT(x) __LTC4015_LINE__((0), (LTC4015_BATDIV / LTC4015_ADCGAIN), (0), (1), x)
/*! Convert from volts to the Lead Acid per-cell vbat ADC reading. */
#define LTC4015_VBAT_SLA_FORMAT(x) __LTC4015_LINE__((0), (LTC4015_BATDIV / LTC4015_ADCGAIN * 2 / 3), (0), (1), x)
/*! Convert from amperes to the ibat ADC reading. */
#define LTC4015_IBAT_FORMAT(x) __LTC4015_LINE__((0), (1 / LTC4015_RSNSB / LTC4015_AVPROG / LTC4015_ADCGAIN), (0), (1), x)
/*! Convert from volts to the SYS ADC reading. */
#define LTC4015_VSYS_FORMAT(x) __LTC4015_LINE__((0), (LTC4015_SYSDIV / LTC4015_ADCGAIN), (0), (1), x)
/*! Convert from volts to the vin ADC reading. */
#define LTC4015_VIN_FORMAT(x) __LTC4015_LINE__((0), (LTC4015_VINDIV / LTC4015_ADCGAIN), (0), (1), x)
/*! Convert from amperes to the iin ADC reading. */
#define LTC4015_IIN_FORMAT(x) __LTC4015_LINE__((0), (1 / LTC4015_RSNSI / LTC4015_AVCLPROG / LTC4015_ADCGAIN), (0), (1), x)
/*! Convert from ohms to the per-cell bsr ADC reading. */
#define LTC4015_BSR_FORMAT(x) __LTC4015_LINE__((0), (LTC4015_RSNSB * LTC4015_AVPROG * LTC4015_BATDIV / 65536), (0), (1), x)
/*! Convert from degrees C to the die_temp ADC reading. */
#define LTC4015_DIE_TEMP_FORMAT(x) __LTC4015_LINE__((-258.8), (-258.8 + 1 / 46.271), (0), (1), x)
/*! Convert from volts to any ADC reading. */
#define LTC4015_ADC18(x) __LTC4015_LINE__((0), (1), (0), (LTC4015_ADCGAIN), x)
/*! @name Private macros for use by formats above.
 */
#define __LTC4015_LINE__(x0,x1,y0,y1,x) (uint16_t)((y0) + ((y1) - (y0))/((x1) - (x0)) * ((x) - (x0)) + 0.5)
#define __LTC4015_BETWEEN_INCLUSIVE__(x0,x1,x) (((x) > (x0) && (x) < (x1)) || (x) == (x0) || (x) == (x1) ? 1 : 0)


#endif /* LTC4015_FORMATS_H_ */
