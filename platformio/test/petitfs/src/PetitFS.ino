// Petit FS test.   
// For minimum flash use edit pffconfig.h and only enable
// _USE_READ and either _FS_FAT16 or _FS_FAT32

#include "PetitFS.h"
#include "PetitSerial.h"

PetitSerial PS;
// Use PetitSerial instead of Serial.
#define Serial PS

// The SD chip select pin is currently defined as 10
// in pffArduino.h.  Edit pffArduino.h to change the CS pin.

FATFS fs;     /* File system object */

void test() {
  
  uint8_t buf[43];
  
  // Initialize SD and file system.
  if (pf_mount(&fs))   return -1;
  
  // Open test file.
  if (pf_open("FIRMWARE.HEX"))   return -1;
  
  // Dump test file to Serial.
  while (1) {
    UINT nr;
    if (pf_read(buf, sizeof(buf), &nr)) return -1;
    if (nr == 0) break;
    Serial.write(buf, nr);

    /*

    // init read variables
    uint8_t hexReadStep = WAIT_FOR_LINE_START;
    uint8_t stepBytesRemaining = 1;
    uint8_t lineWords = 0;
    uint16_t hexNumber;
    uint8_t* hexNumberByte = (uint8_t*)((void*)&hexNumber);

    uint16_t pageBaseAddress = 0x0000;
    uint16_t pageAddress = 0x0000;


    // interpret the byte
    if( hexReadStep == WAIT_FOR_LINE_START ) {

      if( buf[0] != ':' ) {
	stepBytesRemaining++; //prevent ending step 
      }
    } else {
      // build number
      *hexNumberByte <<= 4;
      if( buf[0] <= '9' ) {
	*hexNumberByte += (buf[0] - '0');
      } else {
	*hexNumberByte += buf[0] - 'A' + 0x0A;
      }
    }

    // byte interpreted
    stepBytesRemaining--;
    if( stepBytesRemaining == 2 ) { //if reading word, next byte
      hexNumberByte++;
    }

    // check if step is finished
    if( stepBytesRemaining == 0 ) {

      switch ( hexReadStep ) {
      case WAIT_FOR_LINE_START:
	
	// next read two byte of line size
	stepBytesRemaining = 2;  
	break;

      case READ_LINE_SIZE:

	lineWords = hexNumber/2;
	
	// next read 4 byte of address
	// the value is not used
	stepBytesRemaining = 4;
	break;

      case READ_ADDRESS:

	// next read 2 byt eof line type
	stepBytesRemaining = 2;
	break;

      case READ_LINE_TYPE:

	if( hexNumber == 0x01 ) {
	  // file end terminate flash
	  if( pageAddress != pageBaseAddress ) {
	    write_page(pageBaseAddress);
	  }
	  return 0;
	}

	// next read data word
	stepBytesRemaining = 4;
	break;
      default:  //hexReadStep == READ_DATA

	// if needed prepare flash page
	if( pageAddress == pageBaseAddress ) {
	  __boot_page_erase_short(pageBaseAddress);
	  boot_spm_busy_wait();
	}

	// write a new word
	__boot_page_fill_short(pageAddress, hexNumber);
	pageAddress += 2;
	lineWords--;
	
	// check if we need to change page
	if( pageAddress - pageBaseAddress >= SPM_PAGESIZE ) {
	  write_page(pageBaseAddress);
	  pageBaseAddress += SPM_PAGESIZE; //so pageBaseAddress == pageAddress
	}
      }
 
      // go to next step
      if( hexReadStep != READ_DATA ) {
	hexReadStep++;
      } else {
	if( ! lineWords ) {
	  hexReadStep = WAIT_FOR_LINE_START;
	  stepBytesRemaining = 1;
	} else { //next word
	  stepBytesRemaining = 4;
	}
      }

      // clear number
      hexNumber = 0;
      hexNumberByte = (uint8_t*)((void*)&hexNumber);

    }
    */
  }

  //must not reach this line
  return -1;

}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  test();
  Serial.println("\nDone!");
}
void loop() {}
