/*
 * This program will format SD/SDHC/SDXC cards.
 * Warning all data will be deleted!
 *
 * This program attempts to match the format
 * generated by SDFormatter available here:
 *
 * http://www.sdcard.org/consumers/formatter/
 *
 * For very small cards this program uses FAT16
 * and the above SDFormatter uses FAT12.
 */
#include "SdFat.h"
#include "sdios.h"

/*
  Set DISABLE_CS_PIN to disable a second SPI device.
  For example, with the Ethernet shield, set DISABLE_CS_PIN
  to 10 to disable the Ethernet controller.
*/
const int8_t DISABLE_CS_PIN = -1;
/*
  Change the value of SD_CS_PIN if you are using SPI
  and your hardware does not use the default value, SS.
  Common values are:
  Arduino Ethernet shield: pin 4
  Sparkfun SD shield: pin 8
  Adafruit SD shields and modules: pin 10
*/

// SDCARD_SS_PIN is defined for the built-in SD on some boards.

// microduino
#define SDCARD_SS_PIN 7

#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif  ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_SDIO_CLASS
//==============================================================================
// Serial output stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
uint32_t cardSectorCount = 0;
uint8_t  sectorBuffer[512];
//------------------------------------------------------------------------------
// SdCardFactory constructs and initializes the appropriate card.
SdCardFactory cardFactory;
// Pointer to generic SD card.
SdCard* m_card = nullptr;


SdFs sd;
cid_t m_cid;
csd_t m_csd;
uint32_t m_eraseSize;
uint32_t m_ocr;


//------------------------------------------------------------------------------
#define sdError(msg) {cout << F("error: ") << F(msg) << endl; sdErrorHalt();}
//------------------------------------------------------------------------------
void sdErrorHalt() {
  if (!m_card) {
    cout << F("Invalid SD_CONFIG") << endl;
  } else if (m_card->errorCode()) {
    if (m_card->errorCode() == SD_CARD_ERROR_CMD0) {
      cout << F("No card, wrong chip select pin, or wiring error?") << endl;
    }
    cout << F("SD errorCode: ") << hex << showbase;
    printSdErrorSymbol(&Serial, m_card->errorCode());
    cout << F(" = ") << int(m_card->errorCode()) << endl;
    cout << F("SD errorData = ") << int(m_card->errorData()) << endl;
  }
  while (true) {}
}
//------------------------------------------------------------------------------
void clearSerialInput() {
  uint32_t m = micros();
  do {
    if (Serial.read() >= 0) {
      m = micros();
    }
  } while (micros() - m < 10000);
}
//------------------------------------------------------------------------------
// flash erase all data
uint32_t const ERASE_SIZE = 262144L;
void eraseCard() {
  cout << endl << F("Erasing\n");
  uint32_t firstBlock = 0;
  uint32_t lastBlock;
  uint16_t n = 0;

  do {
    lastBlock = firstBlock + ERASE_SIZE - 1;
    if (lastBlock >= cardSectorCount) {
      lastBlock = cardSectorCount - 1;
    }
    if (!m_card->erase(firstBlock, lastBlock)) {
      sdError("erase failed");
    }
    cout << '.';
    if ((n++)%64 == 63) {
      cout << endl;
    }
    firstBlock += ERASE_SIZE;
  } while (firstBlock < cardSectorCount);
  cout << endl;

  if (!m_card->readSector(0, sectorBuffer)) {
    sdError("readBlock");
  }
  cout << hex << showbase << setfill('0') << internal;
  cout << F("All data set to ") << setw(4) << int(sectorBuffer[0]) << endl;
  cout << dec << noshowbase << setfill(' ') << right;
  cout << F("Erase done\n");
}
//------------------------------------------------------------------------------
void formatCard() {
  //ExFatFormatter exFatFormatter;
  FatFormatter fatFormatter;

  /*
  // Format exFAT if larger than 32GB.
  bool rtn = cardSectorCount > 67108864 ?
    exFatFormatter.format(m_card, sectorBuffer, &Serial) :
    fatFormatter.format(m_card, sectorBuffer, &Serial);
  */
  
  bool rtn = fatFormatter.format(m_card, sectorBuffer, &Serial);

  if (!rtn) {
    sdErrorHalt();
  }
  cout << F("Run the SdInfo example for format details.") << endl;
}
//------------------------------------------------------------------------------
void printConfig(SdSpiConfig config) {
  if (DISABLE_CS_PIN < 0) {
    cout << F(
           "\nAssuming the SD is the only SPI device.\n"
           "Edit DISABLE_CS_PIN to disable an SPI device.\n");
  } else {
    cout << F("\nDisabling SPI device on pin ");
    cout << int(DISABLE_CS_PIN) << endl;
    pinMode(DISABLE_CS_PIN, OUTPUT);
    digitalWrite(DISABLE_CS_PIN, HIGH);
  }
  cout << F("\nAssuming the SD chip select pin is: ") << int(config.csPin);
  cout << F("\nEdit SD_CS_PIN to change the SD chip select pin.\n");
}
//------------------------------------------------------------------------------
bool cidDmp() {
  cout << F("\nManufacturer ID: ");
  cout << uppercase << showbase << hex << int(m_cid.mid) << dec << endl;
  cout << F("OEM ID: ") << m_cid.oid[0] << m_cid.oid[1] << endl;
  cout << F("Product: ");
  for (uint8_t i = 0; i < 5; i++) {
    cout << m_cid.pnm[i];
  }
  cout << F("\nVersion: ");
  cout << int(m_cid.prv_n) << '.' << int(m_cid.prv_m) << endl;
  cout << F("Serial number: ") << hex << m_cid.psn << dec << endl;
  cout << F("Manufacturing date: ");
  cout << int(m_cid.mdt_month) << '/';
  cout << (2000 + 16*m_cid.mdt_year_high + m_cid.mdt_year_low) << endl;
  cout << endl;
  return true;
}
//------------------------------------------------------------------------------
bool csdDmp() {
  bool eraseSingleBlock;
  if (m_csd.v1.csd_ver == 0) {
    eraseSingleBlock = m_csd.v1.erase_blk_en;
    m_eraseSize = (m_csd.v1.sector_size_high << 1) | m_csd.v1.sector_size_low;
  } else if (m_csd.v2.csd_ver == 1) {
    eraseSingleBlock = m_csd.v2.erase_blk_en;
    m_eraseSize = (m_csd.v2.sector_size_high << 1) | m_csd.v2.sector_size_low;
  } else {
    cout << F("m_csd version error\n");
    return false;
  }
  m_eraseSize++;
  cout << F("cardSize: ") << 0.000512 * sdCardCapacity(&m_csd);
  cout << F(" MB (MB = 1,000,000 bytes)\n");

  cout << F("flashEraseSize: ") << int(m_eraseSize) << F(" blocks\n");
  cout << F("eraseSingleBlock: ");
  if (eraseSingleBlock) {
    cout << F("true\n");
  } else {
    cout << F("false\n");
  }
  return true;
}

//------------------------------------------------------------------------------
void errorPrint() {
  if (sd.sdErrorCode()) {
    cout << F("SD errorCode: ") << hex << showbase;
    printSdErrorSymbol(&Serial, sd.sdErrorCode());
    cout << F(" = ") << int(sd.sdErrorCode()) << endl;
    cout << F("SD errorData = ") << int(sd.sdErrorData()) << endl;
  }
}
//------------------------------------------------------------------------------
bool mbrDmp() {
  MbrSector_t mbr;
  bool valid = true;
  if (!sd.card()->readSector(0, (uint8_t*)&mbr)) {
    cout << F("\nread MBR failed.\n");
    errorPrint();
    return false;
  }
  cout << F("\nSD Partition Table\n");
  cout << F("part,boot,bgnCHS[3],type,endCHS[3],start,length\n");
  for (uint8_t ip = 1; ip < 5; ip++) {
    MbrPart_t *pt = &mbr.part[ip - 1];
    if ((pt->boot != 0 && pt->boot != 0X80) ||
        getLe32(pt->relativeSectors) > sdCardCapacity(&m_csd)) {
      valid = false;
    }
    cout << int(ip) << ',' << uppercase << showbase << hex;
    cout << int(pt->boot) << ',';
    for (int i = 0; i < 3; i++ ) {
      cout << int(pt->beginCHS[i]) << ',';
    }
    cout << int(pt->type) << ',';
    for (int i = 0; i < 3; i++ ) {
      cout << int(pt->endCHS[i]) << ',';
    }
    cout << dec << getLe32(pt->relativeSectors) << ',';
    cout << getLe32(pt->totalSectors) << endl;
  }
  if (!valid) {
    cout << F("\nMBR not valid, assuming Super Floppy format.\n");
  }
  return true;
}
//------------------------------------------------------------------------------
void dmpVol() {
  cout << F("\nScanning FAT, please wait.\n");
  uint32_t freeClusterCount = sd.freeClusterCount();
  if (sd.fatType() <= 32) {
    cout << F("\nVolume is FAT") << int(sd.fatType()) << endl;
  } else {
    cout << F("\nVolume is exFAT\n");
  }
  cout << F("sectorsPerCluster: ") << sd.sectorsPerCluster() << endl;
  cout << F("clusterCount:      ") << sd.clusterCount() << endl;
  cout << F("freeClusterCount:  ") << freeClusterCount << endl;
  cout << F("fatStartSector:    ") << sd.fatStartSector() << endl;
  cout << F("dataStartSector:   ") << sd.dataStartSector() << endl;
  if (sd.dataStartSector() % m_eraseSize) {
    cout << F("Data area is not aligned on flash erase boundary!\n");
    cout << F("Download and use formatter from www.sdcard.org!\n");
  }
}
//------------------------------------------------------------------------------
void printCardType() {

  cout << F("\nCard type: ");

  switch (sd.card()->type()) {
    case SD_CARD_TYPE_SD1:
      cout << F("SD1\n");
      break;

    case SD_CARD_TYPE_SD2:
      cout << F("SD2\n");
      break;

    case SD_CARD_TYPE_SDHC:
      if (sdCardCapacity(&m_csd) < 70000000) {
        cout << F("SDHC\n");
      } else {
        cout << F("SDXC\n");
      }
      break;

    default:
      cout << F("Unknown\n");
  }
}
//------------------------------------------------------------------------------
void printConfig(SdioConfig config) {
  (void)config;
  cout << F("Assuming an SDIO interface.\n");
}
//------------------------------------------------------------------------------
void setup() {
  char c;
  Serial.begin(9600);
  // Wait for USB Serial
  while (!Serial) {
    yield();
  }

  cout << F("SdFat version: ") << SD_FAT_VERSION_STR << endl;
  printConfig(SD_CONFIG);
  cout << F("\nType any character to start\n");
  while (!Serial.available()) {
    yield();
  }
  // Discard any extra characters.
  clearSerialInput();

  cout << F(
         "\n"
         "This program can erase and/or format SD/SDHC/SDXC cards.\n"
         "\n"
         "Erase uses the card's fast flash erase command.\n"
         "Flash erase sets all data to 0X00 for most cards\n"
         "and 0XFF for a few vendor's cards.\n"
         "\n"
         "Cards up to 2 GiB (GiB = 2^30 bytes) will be formated FAT16.\n"
         "Cards larger than 2 GiB and up to 32 GiB will be formatted\n"
         //"FAT32. Cards larger than 32 GiB will be formatted exFAT.\n"
         "\n"
         "Warning, all data on the card will be erased.\n"
         "Enter 'Y' to continue: ");
  while (!Serial.available()) {
    yield();
  }
  c = Serial.read();
  cout << c << endl;
  if (c != 'Y') {
    cout << F("Quiting, you did not enter 'Y'.\n");
    return;
  }
  // Read any existing Serial data.
  clearSerialInput();

  // Select and initialize proper card driver.
  m_card = cardFactory.newCard(SD_CONFIG);
  if (!m_card || m_card->errorCode()) {
    sdError("card init failed.");
    return;
  }

  cardSectorCount = m_card->sectorCount();
  if (!cardSectorCount) {
    sdError("Get sector count failed.");
    return;
  }

  cout << F("\nCard size: ") << cardSectorCount*5.12e-7;
  cout << F(" GB (GB = 1E9 bytes)\n");
  cout << F("Card size: ") << cardSectorCount/2097152.0;
  cout << F(" GiB (GiB = 2^30 bytes)\n");

  cout << F("Card will be formated ");
  //if (cardSectorCount > 67108864) {
  //  cout << F("exFAT\n");
  //} else
  if (cardSectorCount > 4194304) {
    cout << F("FAT32\n");
  } else {
    cout << F("FAT16\n");
  }
  cout << F(
         "\n"
         "Options are:\n"
         "E - erase the card and skip formatting.\n"
         "F - erase and then format the card. (recommended)\n"
         "Q - quick format the card without erase.\n"
         "\n"
         "Enter option: ");

  while (!Serial.available()) {
    yield();
  }
  c = Serial.read();
  cout << c << endl;
  if (!strchr("EFQ", c)) {
    cout << F("Quiting, invalid option entered.") << endl;
    return;
  }
  if (c == 'E' || c == 'F') {
    eraseCard();
  }
  if (c == 'F' || c == 'Q') {
    formatCard();
  }
}
void loop() {
  // Read any existing Serial data.
  clearSerialInput();

  // F stores strings in flash to save RAM
  cout << F("\ntype any character to start\n");
  while (!Serial.available()) {
    yield();
  }
  uint32_t t = millis();
  if (!sd.cardBegin(SD_CONFIG)) {
    cout << F(
           "\nSD initialization failed.\n"
           "Do not reformat the card!\n"
           "Is the card correctly inserted?\n"
           "Is there a wiring/soldering problem?\n");
    if (isSpi(SD_CONFIG)) {
      cout << F(
           "Is SD_CS_PIN set to the correct value?\n"
           "Does another SPI device need to be disabled?\n"
           );
    }
    errorPrint();
    return;
  }
  t = millis() - t;
  cout << F("init time: ") << t << " ms" << endl;

  if (!sd.card()->readCID(&m_cid) ||
      !sd.card()->readCSD(&m_csd) ||
      !sd.card()->readOCR(&m_ocr)) {
    cout << F("readInfo failed\n");
    errorPrint();
    return;
  }
  printCardType();
  cidDmp();
  csdDmp();
  cout << F("\nOCR: ") << uppercase << showbase;
  cout << hex << m_ocr << dec << endl;
  if (!mbrDmp()) {
    return;
  }
  if (!sd.volumeBegin()) {
    cout << F("\nvolumeBegin failed. Is the card formatted?\n");
    errorPrint();
    return;
  }
  dmpVol();
}
