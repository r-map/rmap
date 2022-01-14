// Petit FS test.   
// For minimum flash use edit pffconfig.h and only enable
// _USE_READ and either _FS_FAT16 or _FS_FAT32

#include "PetitFS.h"
#include "kk_ihex_read.h"

/*
#include "PetitSerial.h"
PetitSerial PS;
// Use PetitSerial instead of Serial.
#define Serial PS
*/

// The SD chip select pin is currently defined as 10
// in pffArduino.h.  Edit pffArduino.h to change the CS pin.

FATFS fs;     /* File system object */


ihex_bool_t ihex_data_read (struct ihex_state *ihex,
                            ihex_record_type_t type,
                            ihex_bool_t checksum_error) {
    if (type == IHEX_DATA_RECORD) {
        unsigned long address = (unsigned long) IHEX_LINEAR_ADDRESS(ihex);
        Serial.println(address);
        Serial.write(ihex->data, ihex->length);
    } else if (type == IHEX_END_OF_FILE_RECORD) {
      Serial.println("\nEnd");
    }
    return true;
}

void ihex_read() {
  
  uint8_t buf[43];
  
  // Initialize SD and file system.
  if (pf_mount(&fs))   return -1;
  
  // Open test file.
  if (pf_open("FIRMWARE.HEX"))   return -1;

  struct ihex_state ihex;
  ihex_begin_read(&ihex);

  
  // Dump test file to Serial.
  while (1) {
    UINT nr;
    if (pf_read(buf, sizeof(buf), &nr)) return -1;
    if (nr == 0) break;
    Serial.print("\nread: ");
    Serial.println(nr);

    ihex_read_bytes(&ihex, buf, nr);
    
  }

  ihex_end_read(&ihex);

}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  delay(3000);
  Serial.println("\nStart");
  ihex_read();
  Serial.println("\nDone!");
}
void loop() {}
