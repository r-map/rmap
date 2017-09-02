/*
  Analog Input
 Demonstrates analog input by reading an analog sensor on analog pin 0 and
 turning on and off a light emitting diode(LED)  connected to digital pin 13.
 The amount of time the LED will be on and off depends on
 the value obtained by analogRead().

 The circuit:
 * Potentiometer attached to analog input 0
 * center pin of the potentiometer to the analog pin
 * one side pin (either one) to ground
 * the other side pin to +5V
 * LED anode (long leg) attached to digital output 13
 * LED cathode (short leg) attached to ground

 * Note: because most Arduinos have a built-in LED attached
 to pin 13 on the board, the LED is optional.


 Created by David Cuartielles
 modified 30 Aug 2011
 By Tom Igoe

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/AnalogInput

 */

int sensorPin0 = 0;    // select the input pin for the potentiometer
int sensorPin1 = 1;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  // declare the ledPin as an OUTPUT:
  //pinMode(ledPin, OUTPUT);
   // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);  
  
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin0);
  sensorValue = analogRead(sensorPin0);
  Serial.print(F("value 0 "));
  Serial.println(sensorValue);
  sensorValue = analogRead(sensorPin1);
  sensorValue = analogRead(sensorPin1);
  Serial.print(F("value 1 "));
  Serial.println(sensorValue);
  // stop the program for 1000 milliseconds:
  delay(1000);
}
