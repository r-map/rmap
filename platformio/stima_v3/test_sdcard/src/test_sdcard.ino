//#define SPI_DRIVER_SELECT 1
//#define USE_SD_CRC 2
//#define SDFAT_FILE_TYPE 1
//#define USE_LONG_FILE_NAMES 1

#include <SPI.h>
//#include <SdFat.h>

#include "FS.h"
#include "SD.h"

//SdFat SD;

#define C3SCK 1    //viola
#define C3MISO 0   // grigio
#define C3MOSI 4   //bianco
#define C3SS 6     //blu


//#define SCK D5
//#define MISO D6
//#define MOSI D7
//#define SS D4


File test_file;

//SPIClass * vspi = NULL;

//#define SPI_SPEED SD_SCK_MHZ(4)

void setup() {
  // Open serial communications
  Serial.begin(115200);

  delay(5000);
  
  //vspi = new SPIClass(HSPI);
  //vspi->begin(C3SCK, C3MISO, C3MOSI, C3SS); //SCK, MISO, MOSI, SS

  SPI.begin(C3SCK, C3MISO, C3MOSI, C3SS); //SCK, MISO, MOSI, SS
  //SPI.begin(SS);
  //pinMode(C3SS, OUTPUT);
  //digitalWrite(C3SS, LOW);

  Serial.println("\nInitializing SD card...");
}

void loop(void) {

  //if (SD.begin(SDCARD_CHIP_SELECT_PIN,SPI_SPEED)){
  //if (SD.begin(C3SS,*vspi)){

  //SD.begin() SD.begin(uint8_t ssPin=SS, SPIClass &spi=SPI, uint32_t frequency=4000000, const char * mountpoint=”/sd”, uint8_t max_files=5)
    
  if (SD.begin(C3SS,SPI)){
    //if (SD.begin(C3SS,100000)){
    Serial.println("mount OK: ");
 
  }else{
    Serial.println("ERROR mount");    
  }

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
