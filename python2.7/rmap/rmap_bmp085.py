# Copyright (c) 2014 Adafruit Industries
# Author: Tony DiCola
# changed by Paolo Patruno 2014
# changed by dariomas 2016
#
# This library use floating-point equations from the Weather Station Data Logger project
# http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
import logging
import time

import Adafruit_GPIO.I2C as I2C


# BMP085 default address.
BMP085_I2CADDR           = 0x77

# Operating Modes
BMP085_ULTRALOWPOWER     = 0
BMP085_STANDARD          = 1
BMP085_HIGHRES           = 2
BMP085_ULTRAHIGHRES      = 3

# BMP085 Registers
BMP085_CAL_AC1           = 0xAA  # R   Calibration data (16 bits)
BMP085_CAL_AC2           = 0xAC  # R   Calibration data (16 bits)
BMP085_CAL_AC3           = 0xAE  # R   Calibration data (16 bits)
BMP085_CAL_AC4           = 0xB0  # R   Calibration data (16 bits)
BMP085_CAL_AC5           = 0xB2  # R   Calibration data (16 bits)
BMP085_CAL_AC6           = 0xB4  # R   Calibration data (16 bits)
BMP085_CAL_B1            = 0xB6  # R   Calibration data (16 bits)
BMP085_CAL_B2            = 0xB8  # R   Calibration data (16 bits)
BMP085_CAL_MB            = 0xBA  # R   Calibration data (16 bits)
BMP085_CAL_MC            = 0xBC  # R   Calibration data (16 bits)
BMP085_CAL_MD            = 0xBE  # R   Calibration data (16 bits)
BMP085_CONTROL           = 0xF4
BMP085_TEMPDATA          = 0xF6
BMP085_PRESSUREDATA      = 0xF6

# Commands
BMP085_READTEMPCMD       = 0x2E
BMP085_READPRESSURECMD   = 0x34


class BMP085(object):
    def __init__(self, mode=BMP085_STANDARD, address=BMP085_I2CADDR, 
                             busnum=I2C.get_default_bus()):
        self._logger = logging.getLogger('Adafruit_BMP.BMP085')
        # Check that mode is valid.
        if mode not in [BMP085_ULTRALOWPOWER, BMP085_STANDARD, BMP085_HIGHRES, BMP085_ULTRAHIGHRES]:
            raise ValueError('Unexpected mode value {0}.  Set mode to one of BMP085_ULTRALOWPOWER, BMP085_STANDARD, BMP085_HIGHRES, or BMP085_ULTRAHIGHRES'.format(mode))
        self._mode = mode
        # Create I2C device.
        self._device = I2C.Device(address, busnum)
        #(chip_id, version) = bus.read_i2c_block_data(addr, 0xD0, 2)
        chip_id = self._device.readU8(0xD0)
        version = self._device.readU8(0xD0 + 1)
        self._logger.debug('Chip Id: {0} Version: {1}'.format(chip_id, version))
        # Load calibration values.
        self._load_calibration()
        self._compute_polynomials()

        self.temperature=None

    def _load_calibration(self):
        self.cal_AC1 = self._device.readS16BE(BMP085_CAL_AC1)   # INT16
        self.cal_AC2 = self._device.readS16BE(BMP085_CAL_AC2)   # INT16
        self.cal_AC3 = self._device.readS16BE(BMP085_CAL_AC3)   # INT16
        self.cal_AC4 = self._device.readU16BE(BMP085_CAL_AC4)   # UINT16
        self.cal_AC5 = self._device.readU16BE(BMP085_CAL_AC5)   # UINT16
        self.cal_AC6 = self._device.readU16BE(BMP085_CAL_AC6)   # UINT16
        self.cal_B1 = self._device.readS16BE(BMP085_CAL_B1)     # INT16
        self.cal_B2 = self._device.readS16BE(BMP085_CAL_B2)     # INT16
        # not used
        # self.cal_MB = self._device.readS16BE(BMP085_CAL_MB)     # INT16
        self.cal_MC = self._device.readS16BE(BMP085_CAL_MC)     # INT16
        self.cal_MD = self._device.readS16BE(BMP085_CAL_MD)     # INT16
        self._logger.debug('AC1 = {0:6d}'.format(self.cal_AC1))
        self._logger.debug('AC2 = {0:6d}'.format(self.cal_AC2))
        self._logger.debug('AC3 = {0:6d}'.format(self.cal_AC3))
        self._logger.debug('AC4 = {0:6d}'.format(self.cal_AC4))
        self._logger.debug('AC5 = {0:6d}'.format(self.cal_AC5))
        self._logger.debug('AC6 = {0:6d}'.format(self.cal_AC6))
        self._logger.debug('B1 = {0:6d}'.format(self.cal_B1))
        self._logger.debug('B2 = {0:6d}'.format(self.cal_B2))
        #self._logger.debug('MB = {0:6d}'.format(self.cal_MB))
        self._logger.debug('MC = {0:6d}'.format(self.cal_MC))
        self._logger.debug('MD = {0:6d}'.format(self.cal_MD))
        
    def _compute_polynomials(self):
        # The first three values are only used in computing the final constants below
        # and can be discarded afterwards
        fc3 = self.cal_AC3 * 160.0 * pow(2,-15)
        fc4 = self.cal_AC4 * pow(10,-3) * pow(2,-15)
        fb1 = self.cal_B1 * pow(160,2) * pow(2,-30)
        # The next four constants are used in the computation of temperature.
        self.cal_fc5 = self.cal_AC5 * (pow(2,-15) / 160)
        self.cal_fc6 = float(self.cal_AC6)
        self.cal_fmc = self.cal_MC * (float(pow(2,11)) / pow(160,2))
        self.cal_fmd = self.cal_MD / 160.0
        # Three second order polynomials are used to compute pressure, and they
        # require another nine constants
        self.cal_fx0 = float(self.cal_AC1)
        self.cal_fx1 = self.cal_AC2 * 160.0 * pow(2,-13)
        self.cal_fx2 = self.cal_B2 * pow(160,2) * pow(2,-25)
        
        self.cal_fy0 = fc4 * pow(2,15)
        self.cal_fy1 = fc4 * fc3
        self.cal_fy2 = fc4 * fb1
        # In subtracting 8 in the numerator of the equation for p0, it is assumed the
        # original value was offset for the purpose of integer rounding.
        # If this assumption is wrong, there will be a small offset in the result of 0.005mb.
        self.cal_fp0 = (3791.0 - 8.0) / 1600.0
        self.cal_fp1 = 1.0 - 7357.0 * pow(2,-20)
        self.cal_fp2 = 3038.0 * 100.0 * pow(2,-36)
        
        self._logger.debug('fc3 = {0}'.format(fc3))
        self._logger.debug('fc4 = {0}'.format(fc4))
        self._logger.debug('fb1 = {0}'.format(fb1))
        self._logger.debug('fc5 = {0}'.format(self.cal_fc5))
        self._logger.debug('fc6 = {0}'.format(self.cal_fc6))
        self._logger.debug('fmc = {0}'.format(self.cal_fmc))
        self._logger.debug('fmd = {0}'.format(self.cal_fmd))
        self._logger.debug('fx0 = {0}'.format(self.cal_fx0))
        self._logger.debug('fx1 = {0}'.format(self.cal_fx1))
        self._logger.debug('fx2 = {0}'.format(self.cal_fx2))
        self._logger.debug('fy0 = {0}'.format(self.cal_fy0))
        self._logger.debug('fy1 = {0}'.format(self.cal_fy1))
        self._logger.debug('fy2 = {0}'.format(self.cal_fy2))
        self._logger.debug('fp0 = {0}'.format(self.cal_fp0))
        self._logger.debug('fp1 = {0}'.format(self.cal_fp1))
        self._logger.debug('fp2 = {0}'.format(self.cal_fp2))

    def _load_datasheet_calibration(self):
        # Set calibration from values in the datasheet example.  Useful for debugging the
        # temp and pressure calculation accuracy.
        self.cal_AC1 = 408
        self.cal_AC2 = -72
        self.cal_AC3 = -14383
        self.cal_AC4 = 32741
        self.cal_AC5 = 32757
        self.cal_AC6 = 23153
        self.cal_B1 = 6190
        self.cal_B2 = 4
        self.cal_MB = -32767
        self.cal_MC = -8711
        self.cal_MD = 2868

    def read_raw_temp(self):
        """Reads the raw (uncompensated) temperature from the sensor."""

        self._device.write8(BMP085_CONTROL, BMP085_READTEMPCMD)
        time.sleep(0.005)  # Wait 5ms
        raw = self._device.readU16BE(BMP085_TEMPDATA)
        self._logger.debug('Raw temp 0x{0:X} ({1})'.format(raw & 0xFFFF, raw))
        return raw

    def read_temperature(self):
        """Gets the compensated temperature."""

        UT = self.read_raw_temp()
        # Datasheet value for debugging:
        #UT = 27898
        # Calculations below are taken straight from section 3.5 of the datasheet.
        alpha = (UT - self.cal_fc6) * self.cal_fc5
        T = (self.cal_fmc / (alpha + self.cal_fmd)) + alpha
        self._logger.debug('Comp. temp {0}  (alpha: {1})'.format(T, alpha))
        return T

    def prepare_pressure(self):
        """Measure the raw (uncompensated) pressure level from the sensor."""

        self._device.write8(BMP085_CONTROL, BMP085_READPRESSURECMD + (self._mode << 6))
                
        if self._mode == BMP085_ULTRALOWPOWER:
            return 5.
        elif self._mode == BMP085_HIGHRES:
            return 14.
        elif self._mode == BMP085_ULTRAHIGHRES:
            return 26.
        else:
            return 8.

    def prepare(self):

        self.temperature=self.read_temperature()
        return self.prepare_pressure()

    def read_raw_pressure(self):
        """Reads the raw (uncompensated) pressure level from the sensor."""

        raw = self._device.readU16BE(BMP085_PRESSUREDATA)
        fxlsb = self._device.readU8(BMP085_PRESSUREDATA+2) / 256.0 if self._mode != BMP085_ULTRALOWPOWER else 0.0
        pu = raw + fxlsb
        self._logger.debug('Raw pressure 0x{0:04X} ({1})'.format(raw & 0xFFFF, pu))
        return pu

    def get_values(self):
        """Gets :
        the compensated temperature in Celsius
        the compensated pressure in Pascals

        after prepare and sleep called by user."""

        return self.temperature,self.calculate_calibrated_pressure()

    def calculate_calibrated_pressure(self):
        """Gets the compensated pressure in Pascals."""

        pu = self.read_raw_pressure()
        s = self.temperature - 25.0
        # Datasheet values for debugging:
        #UT = 27898
        #UP = 23843
        #
        #pu = UP
        #s = UT - 25
        x = (self.cal_fx2 * pow(s,2)) + (self.cal_fx1 * s) + self.cal_fx0
        self._logger.debug('x = {0}'.format(x))
        y = (self.cal_fy2 * pow(s,2)) + (self.cal_fy1 * s) + self.cal_fy0
        self._logger.debug('y = {0}'.format(y))
        z = (pu - x) / y
        self._logger.debug('z = {0}'.format(z))
        # Pressure Calculations
        p = ((self.cal_fp2 * pow(z,2)) + (self.cal_fp1 * z) + self.cal_fp0) * 100
        self._logger.debug('Pressure {0} Pa'.format(p))
        return p

def main():

    # Can enable debug output by uncommenting:
    #import logging
    #logging.basicConfig(level=logging.DEBUG)

    bmp_driver = BMP085(busnum=1,mode=BMP085_ULTRAHIGHRES)
    dt = bmp_driver.prepare()
    print "delay", dt
    time.sleep(dt/1000)
    temperature,pressure=bmp_driver.get_values()
    print "Temperature:", temperature, "C", " -- ", temperature + 273.15,"K"
    print "Pressure:", pressure / 100.0, "hPa"
    print {"B12101":int(temperature*100.+27315.),"B10004":int(pressure/10.)}


if __name__ == '__main__':
    main()  # (this code was run as script)
