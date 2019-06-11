import smbus
import time
from utils import nint

def signInteger(value, bitcount):
    if value & (1<<(bitcount-1)):
        return value - (1<<bitcount)
    return value


class tmp(object):


    def __init__(self,i2cbus=1,address=0x4c,resolution=0x60):
        """
        resolution is exadecimal value of the 2 bits for resolution
        """

        self.i2cbus=i2cbus
        self.address=address
        self.resolution=resolution & 0x60 # take only resolution bits

        self.bus = smbus.SMBus(self.i2cbus)
        #set resolution and shutdown mode
        self.bus.write_word_data(self.address,0x01,self.resolution | 0x1)


    def getvalues(self):

        # set one shot (OS) bit
        self.bus.write_word_data(self.address,0x01,self.resolution | 0x1 | 0x80)
        # sleep ? the time to take a single temperature conversion
        time.sleep(.5)
        d = self.bus.read_word_data(self.address,0x0)
        #print d
        d=[d >> i & 0xff for i in (8,0)]
        #print d
        count = ((d[1] << 4) | (d[0] >> 4)) & 0xFFF
        return {"B12101":nint(signInteger(count, 12)*6.25 + 27315.)}


def main():

    import time

    t1=tmp(address=0x4c)
    t2=tmp(address=0x4f)

    while True:
        print t1.get_temp()
        print t2.get_temp()
        time.sleep(1)



if __name__ == '__main__':
    main()  # (this code was run as script)
