/*
    Display button value on OLED
*/

#include <Wire.h>
#include <LOLIN_I2C_BUTTON.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//0x31
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

I2C_BUTTON button(DEFAULT_I2C_BUTTON_ADDRESS); //I2C Address 0x31

// I2C_BUTTON button; //I2C address 0x31
// I2C_BUTTON button(your_address); //using customize I2C address

String keyString[] = {"None", "Press", "Long", "Double", "Hold"};

void setup()
{
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

void loop()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.setTextColor(WHITE);

    if (button.get() == 0) // Button press
    {
        if (button.BUTTON_A)
        {
            display.println("A: ");
            display.println(keyString[button.BUTTON_A]);
        }

        if (button.BUTTON_B)
        {
            display.println("B: ");
            display.println(keyString[button.BUTTON_B]);
        }
        display.display();
    }

    delay(500);
}