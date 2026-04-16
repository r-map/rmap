
#include <SPI.h>
#include "SD.h"
//#include <SdFat.h>
#include <pins_stima.h>

#define SDMAXFILE 5
#define SCK D5   
#define MISO D6  
#define MOSI D7
#define SS D8
#define SDCARD_CHIP_SELECT_PIN SS

//#define SPI_SPEED SD_SCK_MHZ(4)
#define SPI_SPEED 10000000

//SdFat SD;
File test_file;

void setup() {
  // Open serial communications
  Serial.begin(115200);
  delay(5000);
  
  SPI.begin(SCK, MISO, MOSI, SS); //SCK, MISO, MOSI, SS
}

void loop(void) {
  
  Serial.println("\nInitializing SD card...");
  if (SD.begin(SS,SPI,SPI_SPEED, "/sd",SDMAXFILE, false)){
    //if (SD.begin(SS,SPI_SPEED)){
    Serial.println   (F("Wiring is correct and a card is present."));
    Serial.println("mount OK: ");

    test_file=SD.open("/test.txt",FILE_WRITE);
    if (test_file) {
      test_file.close();
    }else{
      Serial.println("error open");
    }
  
    if (SD.exists("/test.txt")) {
      Serial.println(F("file test.txt exists") );   
      SD.remove("/renamed.txt");
      if (SD.rename("/test.txt", "/renamed.txt")) {
	Serial.println(F("test.txt file renamed to renamed.txt file"));
      }
    } else {
      Serial.println(F("file test.txt do not exists"));   
    }	  
    
  }else{
    Serial.println   (F("initialization failed. Things to check:"));
    Serial.println   (F("* is a card inserted?"));
    Serial.println   (F("* is your wiring correct?"));
    Serial.println   (F("* did you change the chipSelect pin to match your shield or module?"));
  }

  SD.end();
  
  delay(5000);

}
