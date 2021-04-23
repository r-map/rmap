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

  delay(5000);

}
