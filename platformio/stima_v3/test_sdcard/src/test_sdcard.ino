#define SPI_DRIVER_SELECT 1
#define USE_SD_CRC 2
#define SDFAT_FILE_TYPE 1
#define USE_LONG_FILE_NAMES 1

#include <SPI.h>
#include <SdFat.h>

SdFat SD;
File test_file;

#define SDCARD_CHIP_SELECT_PIN 7
#define SPI_SPEED SD_SCK_MHZ(4)

void setup() {
  // Open serial communications
  Serial.begin(115200);
  SPI.begin();
  pinMode(SDCARD_CHIP_SELECT_PIN, OUTPUT);
  digitalWrite(SDCARD_CHIP_SELECT_PIN, HIGH);
  delay(10000);
  Serial.println("\nInitializing SD card...");
}

void loop(void) {

  if (SD.begin(SDCARD_CHIP_SELECT_PIN,SPI_SPEED)){
    Serial.print("The FAT type of the volume: ");
    Serial.println(SD.vol()->fatType());

    test_file=SD.open("test.txt", O_RDWR | O_CREAT | O_APPEND);
    if (test_file) {
      if (test_file.close()){
	Serial.println("OK");
      }else {
	Serial.println("error close");
      }
    }else{
      Serial.println("error open");
    }
  }else{
    Serial.println("error init");
  }


  
  if (SD.exists("test.txt")) {
    Serial.println(F("file test.txt exists\r\n") );   
    SD.remove("renamed.txt");
    if (SD.rename("test.txt", "renamed.txt")) {
      Serial.println(F("test.txt file renamed to renamed.txt file\r\n"));
    }
  } else {
    Serial.println(F("file test.txt do not exists\r\n"));   
  }	  

  SD.end();
  
  delay(5000);

}
