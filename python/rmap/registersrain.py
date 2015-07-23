#////////////////////////////////////////////////////////////
#// I2C WIND registers
#////////////////////////////////////////////////////////////

I2C_RAIN_ADDRESS   =     0x22

#// all bit to 1 => 0xFFFF or 65535 for int 
MISSINTVALUE = 0xFFFF

#// offset to write signed int in unsigned int
OFFSET = 32768


I2C_RAIN_COMMAND            =   0xFF
I2C_RAIN_COMMAND_START      =     1
I2C_RAIN_COMMAND_STOP       =     2
I2C_RAIN_COMMAND_STARTSTOP  =     3

I2C_RAIN_VERSION            =   0x00      #// Version
I2C_RAIN_TIPS               =   0x01      #// TIPS

I2C_RAIN_MAP_WRITABLE       =   0x1F
I2C_RAIN_ONESHOT            =   0x00      #// saple mode (bool)


#////////////////////////////////////////////////////////////
#// End register definition 
#////////////////////////////////////////////////////////////
