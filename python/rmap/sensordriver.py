#!/usr/bin/python
# GPL. (C) 2015 Paolo Patruno.

# This program is free software; you can redistribute it and/or modify 
# it under the terms of the GNU General Public License as published by 
# the Free Software Foundation; either version 2 of the License, or 
# (at your option) any later version. 
# 
# This program is distributed in the hope that it will be useful, 
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
# GNU General Public License for more details. 
# 
# You should have received a copy of the GNU General Public License 
# along with this program; if not, write to the Free Software 
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 


import json
import time
from utils import nint
from registerswind import *
from registersrain import *


try:
    from rmap_bmp085 import *
except:
    pass
    #print "warning bmp085 driver will not be included"
import jsonrpc

MAXDELAYTIME = 30000

# Encoder per la data
class JSONEncoder(json.JSONEncoder):
    def default(self, o): 
        if isinstance(o, datetime):
            return o.strftime("%Y-%m-%dT%H:%M:%S")
        else:
            return json.JSONEncoder.default(o)

# Funzione per fare il dump in JSON
def dumps(o):
    return json.dumps(o, cls=JSONEncoder)

def signInteger(value, bitcount):
    if value & (1<<(bitcount-1)):
        return value - (1<<bitcount)
    return value


class SensorDriver(object):
    
    def factory(driver=None,**kwargs):
        """
        driver factory
        """

        if driver == "TMP":
            return tmp(**kwargs)
        elif driver == "ADT":
            return Adt7420(**kwargs)
        elif driver == "JRPC":
            return jsonrpcproxy(**kwargs);
        # now we use a microcontroller module connected by serial to interface an RF24 rf module
        # we should develop a software driver to manage RF directly
        #elif driver == "RF24":
        #    return serialproxy(**kwargs);
        elif driver == "HIH":
            return hih6100(**kwargs);
        elif driver == "BMP":
            return bmp(**kwargs);
        elif driver == "DW1":
            return dw1(**kwargs);
        elif driver == "TBR":
            return tbr(**kwargs);
        else:
            return None;

    factory = staticmethod(factory)


class jsonrpcproxy(SensorDriver):

    def __init__(self,transport):

        self._timing=0
        # default for USB
        self.proxy=jsonrpc.ServerProxy( jsonrpc.JsonRpc20(),transport)


    def setup(self,driver="I2C",node=0,type="TMP",address=0x48):

        self.driver=driver
        self.node=node
        self.type=type
        self.address=address
        # not needed: the remote board do the setup at boot when well configured
        #self.proxy.setup(driver=self.driver,node=self.node,type=self.type,address=self.address)

    def prepare(self):

        self._timing=time.time()
        self._delay=None
        self._delay=self.proxy.prepare(driver=self.driver,node=self.node,type=self.type,address=self.address)["waittime"]
        return  self._delay

    def get(self):

        if self._delay is None: return None
        if (time.time() - self._timing) < (self._delay/1000.):
            return None
        if (time.time() - self._timing > MAXDELAYTIME/1000.):
            return None

        return self.proxy.getjson(driver=self.driver,node=self.node,type=self.type,address=self.address)


    def getJson(self):

        return dumps(self.get())



class tmp(SensorDriver):


    def __init__(self):
        self._delay=500
        self._timing=0

    def setup(self,i2cbus=1,address=0x48):

        import smbus

        resolution=0x60  # resolution is exadecimal value of the 2 bits for resolution

        self.i2cbus=i2cbus
        self.address=address
        self.resolution=resolution & 0x60 # take only resolution bits

        self.bus = smbus.SMBus(self.i2cbus)
        #set resolution and shutdown mode
        self.bus.write_word_data(self.address,0x01,self.resolution | 0x1)



    def prepare(self):

        # set one shot (OS) bit
        self.bus.write_word_data(self.address,0x01,self.resolution | 0x1 | 0x80)

        # sleep ? the time to take a single temperature conversion
        # time.sleep(.5)
        self._timing=time.time()
        return self._delay

    def get(self):

        if (time.time() - self._timing) < (self._delay/1000.):
            return None
        if (time.time() - self._timing > MAXDELAYTIME/1000.):
            return None

        d = self.bus.read_word_data(self.address,0x0)
        #print d
        d=[d >> i & 0xff for i in (8,0)]
        #print d
        count = ((d[1] << 4) | (d[0] >> 4)) & 0xFFF
        return {"B12101":nint(signInteger(count, 12)*6.25 + 27315.)}

    def getJson(self):

        #for btable,value in self.get().iteritems():
        return dumps(self.get())


class Adt7420(SensorDriver):


    def __init__(self):
        self._delay=250
        self._timing=0

    #default address is with A1 and A0 with pullup resistor
    def setup(self,i2cbus=1,address=0x4b):

        import smbus

        self.i2cbus=i2cbus
        self.address=address

        self.bus = smbus.SMBus(self.i2cbus)
        self.bus.write_byte_data(self.address,0x03)
        self.bus.write_byte_data(self.address,0x20)

    def prepare(self):

        self.bus.write_byte_data(self.address,0x03)
        self.bus.write_byte_data(self.address,0x20)

        # the time to take a single temperature conversion
        self._timing=time.time()
        return self._delay

    def get(self):

        if (time.time() - self._timing) < (self._delay/1000.):
            return None
        if (time.time() - self._timing > MAXDELAYTIME/1000.):
            return None


            ####  TODO
            #  this is python code
            #
            #  //it's a 13bit int, using two's compliment for negative
            #  int TemperatureSum = ((MSB << 8) | LSB) >> 3 & 0xFFF; 
            #  //int TemperatureSum = ((MSB << 8) | LSB) >> 3 ; 
            #
            #  if (TemperatureSum & 0x800)
            #  {
            #    TemperatureSum=TemperatureSum - 0x1000;
            #  }
            #
            #  if (lenvalues >= 1)  values[0] = (int)(TemperatureSum*6.25 + 27315.) ;
            #  _timing=0;
            ####################################

            #        d = self.bus.read_word_data(self.address,0x0)
            #        #print d
            #
            #        d=[d >> i & 0xff for i in (8,0)]
            #        #print d
            #        count = ((d[1] << 4) | (d[0] >> 4)) & 0xFFF
            #        return {"B12101":nint(signInteger(count, 12)*6.25 + 27315.)}

            return {"B12101":None}

    def getJson(self):

        #for btable,value in self.get().iteritems():
        return dumps(self.get())



class hih6100(SensorDriver):

    def __init__(self):

        # Update rate:  Start-up time (power-on to data ready)
        # measurements are taken only when the application requests them
        # Typ.  50 ms.
        # Max.  60 ms.
        self._delay=60

        self._timing=0

    def setup(self,i2cbus=1,address=0x27):

        import smbus

        self.i2cbus=i2cbus
        self.address=address

        self.bus = smbus.SMBus(self.i2cbus)

    def prepare(self):

        #Making a Measurement Request
        #By default, the digital output humidity sensor performs humidity
        #measurement and temperature measurement conversions
        #whenever it receives a Measurement Request (MR) command;
        #otherwise, the digital output humidity sensor is always powered
        #down. The results are stored after each measurement in output
        #registers to be read using a Data Fetch (DF) command

        #Note: technically the chip speaks i2c and SMBus doesn't actually support
        #the block read that's needed. As such, it actually performs two
        #measurements per read request and the status code is always wrong."""
        # Temporary storage array for HIH6130 data

        self.bus.write_quick(self.address)

        # time.sleep(.06)
        self._timing=time.time()
        return self._delay

    def get(self):

        if (time.time() - self._timing) < (self._delay/1000.):
            return None
        if (time.time() - self._timing > MAXDELAYTIME/1000.):
            return None

        # This, technically, sends an incorrect command. This issues an additional
        # measurement request, which causes the sensor to make another reading. As
        # the write is built into this, there is no delay and thus the result is
        # considered stale. The result it returns, however, is from moments ago so
        # it's fine.

        # This is our: i2cget -y 1 0x27 4 w
        val = self.bus.read_i2c_block_data(self.address,0,4)

        status = (val[0] & 0xc0) >> 6
        humid = float(((val[0] & 0x3f) << 8) + val[1])/0x3ffe*100.
        temp =  float((val[2] << 6)  + ((val[3] & 0xfc) >> 2))/0x3ffe*165.-40.
    
        #good status will be 00 but it's 1

        return {"B12101":nint(temp*100.+27315.),"B13003":nint(humid)}

    def getJson(self):

        #for btable,value in self.get().iteritems():
        return dumps(self.get())

#    def __str__(self):
#        return "HIH"


try:
    class bmp(SensorDriver):

        def __init__(self):

            #TODO
            # Update rate:  Start-up time (power-on to data ready)
            # from 4.5 tp 25.5 ms.  

            self._delay=0

            self._timing=0

        def setup(self,i2cbus=1,address=0x77,mode=BMP085_ULTRAHIGHRES):

            self.i2cbus=i2cbus
            self.address=address

            # You can also optionally change the BMP085 mode to one of BMP085_ULTRALOWPOWER, 
            # BMP085_STANDARD, BMP085_HIGHRES, or BMP085_ULTRAHIGHRES.  See the BMP085
            # datasheet for more details on the meanings of each mode (accuracy and power
            # consumption are primarily the differences).  The default mode is STANDARD.
            #sensor = BMP085.BMP085(mode=BMP085.BMP085_ULTRAHIGHRES)

            self.sensor = BMP085(busnum=self.i2cbus,mode=mode)

        def prepare(self):

            self._timing=time.time()

            self._delay=self.sensor.prepare()
            return self._delay

        def get(self):

            if (time.time() - self._timing) < (self._delay/1000.):
                return None
            if (time.time() - self._timing > MAXDELAYTIME/1000.):
                return None

            temperature,pressure=self.sensor.get_values()

            return {"B12101":nint(temperature*100.+27315.),"B10004":nint(pressure/10.)}

        def getJson(self):

            #for btable,value in self.get().iteritems():
            return dumps(self.get())

except:
    pass



class dw1(SensorDriver):

    def __init__(self):
        self._delay=2500
        self._timing=0

    def setup(self,i2cbus=1,address=I2C_WIND_ADDRESS):

        import smbus

        self.i2cbus=i2cbus
        self.address=address

        self.bus = smbus.SMBus(self.i2cbus)


        oneshot=True;
        self.bus.write_word_data(self.address,I2C_WIND_ONESHOT,oneshot)
        time.sleep(.001)

        self.bus.write_word_data(self.address,I2C_WIND_COMMAND,I2C_WIND_COMMAND_ONESHOT_STOP)

    def prepare(self):

        self.bus.write_word_data(self.address,0x01,I2C_WIND_COMMAND,I2C_WIND_COMMAND_ONESHOT_START)

        self._timing=time.time()
        return self._delay

    def get(self):

        if (time.time() - self._timing) < (self._delay/1000.):
            return None
        if (time.time() - self._timing > MAXDELAYTIME/1000.):
            return None

        dd = self.bus.read_word_data(self.address,I2C_WIND_DD)
        #print dd

        ff = self.bus.read_word_data(self.address,I2C_WIND_FF)
        #print ff

        return {"B11001":dd,"B11002":ff}

    def getJson(self):

        #for btable,value in self.get().iteritems():
        return dumps(self.get())



class tbr(SensorDriver):

    def __init__(self):
        self._delay=500
        self._timing=0

    def setup(self,i2cbus=1,address=I2C_RAIN_ADDRESS):

        import smbus

        self.i2cbus=i2cbus
        self.address=address

        self.bus = smbus.SMBus(self.i2cbus)


        oneshot=True;
        self.bus.write_word_data(self.address,I2C_RAIN_ONESHOT,oneshot)
        time.sleep(.001)

        self.bus.write_word_data(self.address,I2C_RAIN_COMMAND,I2C_RAIN_COMMAND_ONESHOT_START)

    def prepare(self):

        self.bus.write_word_data(self.address,0x01,I2C_RAIN_COMMAND,I2C_RAIN_COMMAND_ONESHOT_STARTSTOP)

        self._timing=time.time()
        return self._delay

    def get(self):

        if (time.time() - self._timing) < (self._delay/1000.):
            return None
        if (time.time() - self._timing > MAXDELAYTIME/1000.):
            return None

        r = self.bus.read_word_data(self.address,I2C_RAIN_TIPS)
        #print r

        return {"B13011":r}

    def getJson(self):

        #for btable,value in self.get().iteritems():
        return dumps(self.get())



def main():

    drivers=[]

#    drivers.append(SensorDriver.factory("TMP"))
#    drivers.append(SensorDriver.factory("HIH"))
#    drivers.append(SensorDriver.factory("BMP"))
    drivers.append(SensorDriver.factory("DW1"))
    drivers.append(SensorDriver.factory("TBR"))


#    transport=jsonrpc.TransportSERIAL(port='/dev/ttyACM0',baudrate=9600,sleep=2,logfunc=jsonrpc.log_dummy)

    # bluetooth over serial on linux host
    # transport=jsonrpc.TransportSERIAL(logfunc=jsonrpc.log_file("logrpc.txt"),port='/dev/rfcomm0',baudrate=115200,sleep=0)
    #  )

#    drivers.append(SensorDriver.factory("JRPC",transport=transport))

    time.sleep(5)


    for driver in drivers:
        driver.setup()
        #driver.setup(driver="TMP",node=1,type="TMP",address=0x48):

    while True:

        dt=0
        for driver in drivers:
            dt=max(driver.prepare(),dt)
            
        print "sleep:",dt
        time.sleep(dt/1000.)

        for driver in drivers:
            print driver.getJson()

        time.sleep(1)


if __name__ == '__main__':
    main()  # (this code was run as script)
