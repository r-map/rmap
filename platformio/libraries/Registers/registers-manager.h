///////////////////////////////////////////////////////////////////////////////////////////////////
// I2C MANAGER registers
///////////////////////////////////////////////////////////////////////////////////////////////////

#define I2C_MANAGER_DEFAULTADDRESS         6                      //7 bit address 0x40 write, 0x41 read

// all bit to 1 => 0xFFFF or 65535 for int 
#define MISSINTVALUE 0xFFFF

// offset to write signed int in unsigned int
#define OFFSET 32768

// registers
#define I2C_MANAGER_COMMAND                         0xFF
#define I2C_MANAGER_NOCOMMAND                       0X00
#define I2C_MANAGER_COMMAND_SAVE                    0X01
#define I2C_MANAGER_COMMAND_BUTTON1_SHORTPRESSED    0x02
#define I2C_MANAGER_COMMAND_BUTTON1_LONGPRESSED     0x03
//#define I2C_MANAGER_COMMAND_BUTTON2_SHORTPRESSED    0x04
//#define I2C_MANAGER_COMMAND_BUTTON2_LONGPRESSED     0x05
#define I2C_MANAGER_COMMAND_ENCODER_RIGHT           0x06      // button 1 last command
#define I2C_MANAGER_COMMAND_ENCODER_LEFT            0x07      // button 1 last command

#define I2C_MANAGER_VERSION                         0x00      // Version		   
#define I2C_MANAGER_ENCODER_COUNT                   0x01      // encoder counter
//#define I2C_MANAGER_BUTTON1                         0x02      // button 1 last command
//#define I2C_MANAGER_BUTTON2                         0x03      // button 2 last command

#define I2C_MANAGER_MAP_WRITABLE                    0x1F
#define I2C_MANAGER_ADDRESS                         0x20      // i2c bus address (short unsigned int)

///////////////////////////////////////////////////////////////////////////////////////////////////
// End register definition 
///////////////////////////////////////////////////////////////////////////////////////////////////
