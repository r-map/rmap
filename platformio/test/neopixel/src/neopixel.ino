#include <Adafruit_NeoPixel.h>
 
#define PIN 47 // S3
//#define PIN 7  // C3
 
// Quando configuriamo la libreria NeoPixel, gli diciamo quanti pixel e quale pin usare per inviare segnali.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);
 
void setup()
{
  pixels.begin();            //INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();            // Turn OFF all pixels ASAP
  pixels.setBrightness(125); // Set BRIGHTNESS (max = 255)
}
 
void loop()
{
 
  pixels.setPixelColor(0, pixels.Color(255, 0, 0)); //Colore rosso
  pixels.show();
  delay(1000);
  pixels.setPixelColor(0, pixels.Color(0, 255, 0)); //Colore verde
  pixels.show();
  delay(1000);
  pixels.setPixelColor(0, pixels.Color(0, 0, 255)); //Colore blu
  pixels.show();
  delay(1000);
}
