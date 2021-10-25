/*
BUTTON A:
    PRESS: number-1
    DOUBLE PRESS: number-2
    HOLD: number-10

BUTTON B:
    PRESS: number+1
    DOUBLE PRESS: number+2
    HOLD: number+10

HOLD BUTTON A & B:
    number=0
*/

#include <Wire.h>
#include <LOLIN_I2C_BUTTON.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

I2C_BUTTON button(DEFAULT_I2C_BUTTON_ADDRESS); //I2C Address 0x31

// I2C_BUTTON button; //I2C address 0x31
// I2C_BUTTON button(your_address); //using customize I2C address


byte number = 0;

void setup()
{
    Serial.begin(115200);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println(number);
    display.println();
    display.print("-   +");

    display.display();
}

void loop()
{

    if (button.get() == 0)// Button press
    {

        display.clearDisplay();
        display.setCursor(0, 0);

        if ((button.BUTTON_A == KEY_VALUE_HOLD) && (button.BUTTON_B == KEY_VALUE_HOLD))// hold button A&B
        {
            number = 0;
        }
        else
        {

            switch (button.BUTTON_B)
            {
            case KEY_VALUE_NONE:
                break;

            case KEY_VALUE_SHORT_PRESS:
                number++;
                break;

            case KEY_VALUE_DOUBLE_PRESS:
                number += 2;
                break;

            case KEY_VALUE_HOLD:
                number += 10;
                break;

            case KEY_VALUE_LONG_PRESS:

                break;
            }

            switch (button.BUTTON_A)
            {
            case KEY_VALUE_NONE:
                break;

            case KEY_VALUE_SHORT_PRESS:
                number--;
                break;

            case KEY_VALUE_DOUBLE_PRESS:
                number -= 2;
                break;

            case KEY_VALUE_HOLD:
                number -= 10;
                break;

            case KEY_VALUE_LONG_PRESS:

                break;
            }
        }

        display.println(number);
        display.println();
        display.print("-   +");

        display.display();
    }

    // delay(10);
}
