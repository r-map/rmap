/*
    Restore I2C address to DEFAULT_I2C_BUTTON_ADDRESS (0x31)
*/

#include <Wire.h>
#include <LOLIN_I2C_BUTTON.h>

void setup()
{
    byte error, address;

    Wire.begin();

    Serial.begin(115200);

    Serial.println("Scanning...");
    for (address = 1; address < 127; address++) //find the I2C BUTTON device
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            I2C_BUTTON button(address);
            if (button.getInfo() == 0)
            {

                if (button.PRODUCT_ID == PRODUCT_ID_I2C_BUTTON)
                {
                    Serial.print("I2C BUTTON found at address 0x");
                    if (address < 16)
                        Serial.print("0");
                    Serial.println(address, HEX);

                    Serial.print("Firmware Version: ");
                    Serial.println(button.VERSION);

                    if (address == DEFAULT_I2C_BUTTON_ADDRESS)
                    {
                        Serial.println("Already default address");
                    }
                    else
                    {
                        Serial.print("Try to restore default address 0x");
                        Serial.println(DEFAULT_I2C_BUTTON_ADDRESS, HEX);

                        button.changeAddress(DEFAULT_I2C_BUTTON_ADDRESS);
                    }
                }
            }
        }
    }
}

void loop()
{
}
