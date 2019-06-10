#/////////////////////////////////////////////////////////////////
#// I2C WIND registers
#//////////////////////////////////////////////////////////////////

I2C_WIND_ADDRESS      =  0x21                      #//7 bit address 0x40 write, 0x41 read

#// all bit to 1 => 0xFFFF or 65535 for int 
MISSINTVALUE = 0xFFFF

#// offset to write signed int in unsigned int
OFFSET = 32768


I2C_WIND_COMMAND               = 0xFF
I2C_WIND_COMMAND_ONESHOT_START =   1
I2C_WIND_COMMAND_ONESHOT_STOP  =   2
I2C_WIND_COMMAND_TEST          =   3

I2C_WIND_VERSION          =     0x00      #// Version
I2C_WIND_DD               =     0x01      #// DD
I2C_WIND_FF               =     0x03      #// FF
I2C_WIND_U                =     0x05      #// u component
I2C_WIND_V                =     0x07      #// v component
I2C_WIND_MEANU            =     0x09      #// 10 min mean U
I2C_WIND_MEANV            =     0x0B      #// 10 min mean V
I2C_WIND_PEAKGUSTU        =     0x0D      #// peak gust U
I2C_WIND_PEAKGUSTV        =     0x0F      #// peak gust V
I2C_WIND_LONGGUSTU        =     0x11      #// long (60s) gust U
I2C_WIND_LONGGUSTV        =     0x13      #// long (60s) gust V
I2C_WIND_MEANFF           =     0x15      #// mean FF
I2C_WIND_SIGMA            =     0x17      #// sigma FF
I2C_WIND_SECTOR1          =     0x17      #// frequency on sector 1
I2C_WIND_SECTOR2          =     0x17      #// frequency on sector 2
I2C_WIND_SECTOR3          =     0x17      #// frequency on sector 3
I2C_WIND_SECTOR4          =     0x17      #// frequency on sector 4
I2C_WIND_SECTOR5          =     0x17      #// frequency on sector 5
I2C_WIND_SECTOR6          =     0x17      #// frequency on sector 6
I2C_WIND_SECTOR7          =     0x17      #// frequency on sector 7
I2C_WIND_SECTOR8          =     0x17      #// frequency on sector 8
I2C_WIND_SECTORCALM       =     0x19      #// frequency of calm wind

I2C_WIND_MAP_WRITABLE     =     0x1F
I2C_WIND_ONESHOT          =     0x00      #// saple mode (bool)


#//////////////////////////////////////////
#// End register definition 
#//////////////////////////////////////////
