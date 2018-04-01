/*

  "Hello World" version for U8x8 API

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

*/

#include <U8g2lib.h>
#include <Wire.h>

// wemos:
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0); // hardware

void setup(void)
{
  Serial.begin(115200);
  Serial.println();

  Serial.println("Started");

  Wire.begin();
  
  u8g2.begin();
  u8g2.setFont(u8g2_font_5x7_tf);
  //u8g2.setFontMode(0); // enable transparent mode, which is faster
  
}

void loop(void) {

  Serial.println("loop");
  /*
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.drawStr(0,10,"Hello World!");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display
  delay(1000); 
  */

  /*
  u8g2.firstPage();

  do {
    u8g2.drawStr(0,20,"Hello World!");
  } while ( u8g2.nextPage() );

  delay(1000);
  */

  int offset=64;			// current offset for the scrolling text
  int width; // pixel width of the scrolling text
  const char *text = "Testo veramente lungo lunghissimo e lo allungo ma allora ? Ma dai che funziona benissimo senza problemi di sorta ! E vai !!!!!!!"; // scroll this text from right to left
  width = u8g2.getStrWidth(text); // calculate the pixel width of the text  
  
  while (true){
    //u8g2.firstPage();
    //do {
      u8g2.clearBuffer();
      // draw the scrolling text at current offset
      Serial.print(offset);
      Serial.println(width);
      u8g2.drawStr(offset, 10, text);			// draw the scolling text
      //u8g2.drawStr(0,40,"Hello World!");		// draw the fixed text
      u8g2.setCursor(0, 40);
      u8g2.print(F("Hello World!"));		// draw the fixed text

      u8g2.sendBuffer();
      //} while ( u8g2.nextPage() );
  
    offset-=1;						// scroll by one pixel
    if ( offset < -width )	
      offset = 64;					// start over again

    delay(10); // do some small delay
  }
  
}
