
#ifndef __LOLIN_I2C_BUTTON_H
#define __LOLIN_I2C_BUTTON_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "Wire.h"

#define PRODUCT_ID_I2C_BUTTON 0x01
#define DEFAULT_I2C_BUTTON_ADDRESS 0x31

#define OLED_I2C_ADDRESS_1 0x3C
#define OLED_I2C_ADDRESS_2 0x3D

enum I2C_CMD
{
  GET_SLAVE_STATUS = 0x01,
  RESET_SLAVE,
  CHANGE_I2C_ADDRESS,
  GET_KEY_VALUE,
  CHANGE_KEY_A_LONG_PRESS_TIME,
  CHANGE_KEY_B_LONG_PRESS_TIME,
  CHANGE_KEY_A_DOUBLE_PRESS_INTERVAL,
  CHANGE_KEY_B_DOUBLE_PRESS_INTERVAL
};

enum KEY_VALUE
{
    KEY_VALUE_NONE = 0x00,
    KEY_VALUE_SHORT_PRESS,
    KEY_VALUE_LONG_PRESS,
    KEY_VALUE_DOUBLE_PRESS,
    KEY_VALUE_HOLD
};

class I2C_BUTTON
{
  public:
	I2C_BUTTON(unsigned char address = DEFAULT_I2C_BUTTON_ADDRESS);
	unsigned char get(void);
	unsigned char reset(void);
	unsigned char changeAddress(unsigned char address);
  unsigned char getInfo(void);

	unsigned char BUTTON_A;
	unsigned char BUTTON_B;
  unsigned char VERSION=0;
  unsigned char PRODUCT_ID=0;

  private:
	unsigned char _address;
	unsigned char send_data[2] = {0};
	unsigned char get_data[2]={0};
	unsigned char sendData(unsigned char *data, unsigned char len);
};

#endif
