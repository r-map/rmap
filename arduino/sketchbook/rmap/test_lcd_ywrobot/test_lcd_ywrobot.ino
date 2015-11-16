/* FILE:    ARD_LCD_HCARDU0023_I2C_Hello_World_Example.pde
   DATE:    11/07/12
   VERSION: 0.1

This is a simple example of how to use the Hobby Components I2C LCD module 
(HCARDU0023). To use this module you will require the appropriate library 
which can be downloaded from the following location:

http://forum.hobbycomponents.com/arduino_shields_modules
 
This code also demonstrates the correct pin assignment for the LCD. When you 
run this program you should see a greeting message appear on the display. 


DEVICE PINOUT (SPI Interface):

PIN 1: GND
PIN 2: +5V
PIN 3: SDA - Connect to Arduino analogue PIN 4
PIN 4: SCL - Connect to Arduino analogue PIN 5


You may copy, alter and reuse this code in any way you like but please leave 
reference to hobbycomponents.com in your comments if you redistribute this code. */


/* Include the SPI/IIC Library */
#include <Wire.h>
#include <YwrobotLiquidCrystal_I2C.h>


/* Initialise the LiquidCrystal library. The default address is 0x27 and this is a 16x2 line display */
LiquidCrystal_I2C lcd(0x27,20,4);


void setup() 
{
  /* Initialise the LCD */
  lcd.init();
  lcd.init();
}

/* Main program loop */
void loop() 
{
  /* Make sure the backlight is turned on */
  lcd.backlight();

  /* Output the test message to the LCD */
  lcd.setCursor(0,0); 
  lcd.print("R-map project");
  lcd.setCursor(0,1); 
  lcd.print("**HELLO WORLD**");
  lcd.setCursor(0,2); 
  lcd.print("12345678901234567890");
  lcd.setCursor(0,3); 
  lcd.print("**     test ok    **");

  
  /* Do nothing */
  while(1);
}

