#include "LOLIN_I2C_BUTTON.h"

/* 
	Init
*/
I2C_BUTTON::I2C_BUTTON(uint8_t address)
{
	Wire.begin();
	_address = address;
}

/*
	Get last button status  
	0 - None
	1 - Press
	2 - Long Press
	3 - Double Press
	4 - Hold
*/
unsigned char I2C_BUTTON::get()
{
	send_data[0] = GET_KEY_VALUE;
	unsigned char result = sendData(send_data, 1);

	if (result == 0)
	{
		BUTTON_A = (get_data[0] >> 4);
		BUTTON_B = (get_data[0] & 0x0f);
	}
	else
	{
		BUTTON_A = 0;
		BUTTON_B = 0;
	}

	return result;
}

/*
	Reset Device.
*/
unsigned char I2C_BUTTON::reset()
{
	send_data[0] = RESET_SLAVE;
	unsigned char result = sendData(send_data, 1);

	return result;
}

/*
	Change Device I2C address
	address: when address=0, address>=127, address=0x3C or address=0x3D, will change address to default I2C address 0x31
*/
unsigned char I2C_BUTTON::changeAddress(unsigned char address)
{
	send_data[0] = CHANGE_I2C_ADDRESS;
	send_data[1] = address;
	unsigned char result = sendData(send_data, 2);

	return result;
}

/*
	Get PRODUCT_ID and Firmwave VERSION
*/
unsigned char I2C_BUTTON::getInfo(void)
{
	send_data[0] = GET_SLAVE_STATUS;
	unsigned char result = sendData(send_data, 1);
	if (result == 0)
	{
		PRODUCT_ID = get_data[0];
		VERSION = get_data[1];
	}
	else
	{
		PRODUCT_ID = 0;
		VERSION = 0;
	}

	return result;
}

/*
	Send and Get I2C Data
*/
unsigned char I2C_BUTTON::sendData(unsigned char *data, unsigned char len)
{
	unsigned char i;

	if ((_address == 0) || (_address >= 127) || (_address == OLED_I2C_ADDRESS_1) || (_address == OLED_I2C_ADDRESS_2))
	{
		return 1;
	}
	else
	{

		Wire.beginTransmission(_address);
		for (i = 0; i < len; i++)
			Wire.write(data[i]);
		Wire.endTransmission();
		delay(50);

		if (data[0] == GET_SLAVE_STATUS)
			Wire.requestFrom(_address, 2);
		else
			Wire.requestFrom(_address, 1);

		i = 0;

		while (Wire.available())
		{
			get_data[i] = Wire.read();
			i++;
		}

		return 0;
	}
}
