/* -*- C++ -*- */
/********************
2018 Paolo Patruno - p.patruno@iperbole.bologna.it

IR receiver using IRremote library
*/

#ifndef RSITE_ARDUINO_MENU_IRREMOTE
  #define RSITE_ARDUINO_MENU_IRREMOTE


#include "../menuDefs.h"
#ifdef ARDUINO_ARCH_ESP8266
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#else
#include <IRremote.h>
#endif

// ==================== start of TUNEABLE PARAMETERS ====================

//#define DECODE_AC

// IR telecontrol CODE
#define DECODETYPE NEC
#define KEYPAD_0     0xFF48B7 // 0 Keypad Button
#define KEYPAD_1     0xFF906F // 1 Keypad Button
#define KEYPAD_2     0xFFB847 // 2 Keypad Button
#define KEYPAD_3     0xFFF807 // 3 Keypad Button
#define KEYPAD_4     0xFFB04F // 4 Keypad Button
#define KEYPAD_5     0xFF9867 // 5 Keypad Button
#define KEYPAD_6     0xFFD827 // 6 Keypad Button
#define KEYPAD_7     0xFF8877 // 7 Keypad Button
#define KEYPAD_8     0xFFA857 // 8 Keypad Button
#define KEYPAD_9     0xFFE817 // 9 Keypad Button
#define KEYPAD_MINUS 0xFF50AF // Vol- Keypad Button
#define KEYPAD_PLUS  0xFF7887 // Vol+ Keypad Button
#define KEYPAD_DOWN  0xFF40BF // CH- Keypad Button
#define KEYPAD_UP    0xFFA05F // CH+ Keypad Button
#define KEYPAD_OK    0xFF02FD // full screen Keypad Button
#define KEYPAD_POWERDOWN 0xFFB24D // powerdown Keypad Button
#define KEYPAD_REPEAT   UINT64_MAX // Key pressed for more time

// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
#define CAPTURE_BUFFER_SIZE 1024

// TIMEOUT is the Nr. of milli-Seconds of no-more-data before we consider a
// message ended.
// This parameter is an interesting trade-off. The longer the timeout, the more
// complex a message it can capture. e.g. Some device protocols will send
// multiple message packets in quick succession, like Air Conditioner remotes.
// Air Coniditioner protocols often have a considerable gap (20-40+ms) between
// packets.
// The downside of a large timeout value is a lot of less complex protocols
// send multiple messages when the remote's button is held down. The gap between
// them is often also around 20+ms. This can result in the raw data be 2-3+
// times larger than needed as it has captured 2-3+ messages in a single
// capture. Setting a low timeout value can resolve this.
// So, choosing the best TIMEOUT value for your use particular case is
// quite nuanced. Good luck and happy hunting.
// NOTE: Don't exceed MAX_TIMEOUT_MS. Typically 130ms.
#if DECODE_AC
#define TIMEOUT 50U  // Some A/C units have gaps in their protocols of ~40ms.
                     // e.g. Kelvinator
                     // A value this large may swallow repeats of some protocols
#else  // DECODE_AC
#define TIMEOUT 15U  // Suits most messages, while not swallowing many repeats.
#endif  // DECODE_AC
// Alternatives:
// #define TIMEOUT 90U  // Suits messages with big gaps like XMP-1 & some aircon
                        // units, but can accidentally swallow repeated messages
                        // in the rawData[] output.
// #define TIMEOUT MAX_TIMEOUT_MS  // This will set it to our currently allowed
                                   // maximum. Values this high are problematic
                                   // because it is roughly the typical boundary
                                   // where most messages repeat.
                                   // e.g. It will stop decoding a message and
                                   //   start sending it to serial at precisely
                                   //   the time when the next message is likely
                                   //   to be transmitted, and may miss it.

// Set the smallest sized "UNKNOWN" message packets we actually care about.
// This value helps reduce the false-positive detection rate of IR background
// noise as real messages. The chances of background IR noise getting detected
// as a message increases with the length of the TIMEOUT value. (See above)
// The downside of setting this message too large is you can miss some valid
// short messages for protocols that this library doesn't yet decode.
//
// Set higher if you get lots of random short UNKNOWN messages when nothing
// should be sending a message.
// Set lower if you are sure your setup is working, but it doesn't see messages
// from your device. (e.g. Other IR remotes work.)
// NOTE: Set this value very high to effectively turn off UNKNOWN detection.
#define MIN_UNKNOWN_SIZE 12
// ==================== end of TUNEABLE PARAMETERS ====================

#if DECODE_AC
#include <ir_Daikin.h>
#include <ir_Fujitsu.h>
#include <ir_Gree.h>
#include <ir_Haier.h>
#include <ir_Kelvinator.h>
#include <ir_Midea.h>
#include <ir_Toshiba.h>
#endif  // DECODE_AC

  namespace Menu {

    // An IR detector/demodulator is connected to GPIO pin
    // RECV_PIN D5
    template<uint16_t RECV_PIN>
    class irIn {
    public:
      volatile int pos=0;
      volatile bool ok=false;
      volatile bool escape=false;
      volatile int8_t ind=-1;
      // Use turn on the save buffer feature for more complete capture coverage.
      //IRrecv myirrecv= IRrecv(RECV_PIN, CAPTURE_BUFFER_SIZE, TIMEOUT, true);
      IRrecv myirrecv= IRrecv(RECV_PIN);
      decode_results results;  // to store the results
      short int lastkey=-1;
      
      void begin() {
#if DECODE_HASH
	// Ignore messages with less than minimum on or off pulses.
	myirrecv.setUnknownThreshold(MIN_UNKNOWN_SIZE);
#endif  // DECODE_HASH
	myirrecv.enableIRIn();  // Start the receiver
      }

      void process() {

	// Check if the IR code has been received.
	if (myirrecv.decode(&results)) {
	  // Display a crude timestamp.
	  uint32_t now = millis();

	  /*
	  if (results.overflow)
	    Serial.printf("WARNING: IR code is too big for buffer (>= %d). "
			  "This result shouldn't be trusted until this is resolved. "
			  "Edit & increase CAPTURE_BUFFER_SIZE.\n",
			  CAPTURE_BUFFER_SIZE);
	  // Display the basic output of what we found.
	  Serial.print(resultToHumanReadableBasic(&results));
	  dumpACInfo(&results);  // Display any extra A/C info if we have it.
	  yield();  // Feed the WDT as the text output can take a while to print.

	  // Display the library version the message was captured with.
	  Serial.print("Library   : v");
	  Serial.println(_IRREMOTEESP8266_VERSION_);
	  Serial.println();

	  // Output RAW timing info of the result.
	  Serial.println(resultToTimingInfo(&results));
	  yield();  // Feed the WDT (again)

	  // Output the results as source code
	  Serial.println(resultToSourceCode(&results));
	  Serial.println("");  // Blank line between entries
	  yield();  // Feed the WDT (again)
	  */

	  // Print Code in HEX
	  //serialPrintUint64(results.value, HEX);
	  //Serial.print(" : ");
	  //Serial.print(results.decode_type);
	  //Serial.print(" : ");
	  //Serial.println(NEC);

	  
	  if (results.decode_type == DECODETYPE) {
	    int8_t key=-1;
 
	    switch(results.value){
	    case KEYPAD_0: // 1 Keypad Button
	      key=0;
	      lastkey=key;
	      break;

	    case KEYPAD_1: // 1 Keypad Button
	      key=1;
	      lastkey=key;
	      break;

	    case KEYPAD_2: // 2 Keypad Button
	      key=2;
	      lastkey=key;
	      break;

	    case KEYPAD_3: // 3 Keypad Button
	      key=3;
	      lastkey=key;
	      break;

	    case KEYPAD_4: // 1 Keypad Button
	      key=4;
	      lastkey=key;
	      break;

	    case KEYPAD_5: // 2 Keypad Button
	      key=5;
	      lastkey=key;
	      break;

	    case KEYPAD_6: // 3 Keypad Button
	      key=6;
	      lastkey=key;
	      break;

	    case KEYPAD_7:
	      key=7;
	      lastkey=key;
	      break;
	      
	    case KEYPAD_8:
	      key=8;
	      lastkey=key;
	      break;

	    case KEYPAD_9:
	      key=9;
	      lastkey=key;
	      break;
	
	    case KEYPAD_MINUS:
	      key=10;
	      lastkey=key;
	      break;

	    case KEYPAD_PLUS:
	      key=11;
	      lastkey=key;
	      break;

	    case KEYPAD_DOWN:
	      key=12;
	      lastkey=key;
	      break;

	    case KEYPAD_UP:
	      key=13;
	      lastkey=key;
	      break;

	    case KEYPAD_OK:
	      key=14;
	      lastkey=key;
	      break;

	    case KEYPAD_POWERDOWN:
	      key=15;
	      lastkey=key;
	      break;
	      
	    case KEYPAD_REPEAT: // REPEAT code
	      key=lastkey;
	      break;
	      
	    default:
	      //LOGN(F("unknown key" CR));
	      lastkey=-1;
	      break;
	    }
	    //LOGN(F("key: %d  lastkey: %d" CR),key,lastkey);

	    if (key >= 0){
	      Serial.println(key);
	      switch(key){ 
	      case 10:
	      case 12: // minus Button	    
		pos+=1;
		break;
	      case 11:
	      case 13: // plus Button
		pos-=1;
		break;
	      case 14: // ok Button
		ok=true;
		break;
	      case 15: // escape Button
		escape=true;
		break;
	      default:
		ind=key;
		break;
	      }
	    }
	  }
	  myirrecv.resume();
	}
      }
    };

    //emulate a stream based on irIn movement returning +/- for every 'sensivity' steps
    //buffer not needer because we have an accumulator
    template<uint16_t RECV_PIN>
    class irInStream:public menuIn {
    public:
      irIn<RECV_PIN> &ir;//associated hardware irIn
      int oldPos=0;
      irInStream(irIn<RECV_PIN> &ir):ir(ir) {}

      int available(void) {
	return abs(ir.pos-oldPos)+ir.ok+ir.escape+(ir.ind>=0);
      }

      int peek(void) override {

	int d=ir.pos-oldPos;
	Serial.println("read quindi:");
	Serial.println(d);
	Serial.println(ir.ok);
	Serial.println(ir.escape);
	Serial.println(ir.ind);
	
        if (d<=-1)return options->navCodes[downCmd].ch;
        if (d>=1) return options->navCodes[upCmd].ch;
	if (ir.ok) return options->navCodes[enterCmd].ch;
	if (ir.escape) return options->navCodes[escCmd].ch;
	if (ir.ind >=0){
	  char ind [10];
	  itoa (ir.ind,ind,10);
	  Serial.println(ind);
	  return ind[0];
	}
	return -1;
      }

      int read() override {
        int d=ir.pos-oldPos;

	Serial.println("read quindi:");
	Serial.println(d);
	Serial.println(ir.ok);
	Serial.println(ir.escape);
	Serial.println(ir.ind);
	
        if (d<=-1) {
          oldPos-=1;
          return options->navCodes[downCmd].ch;
        }
        if (d>=1) {
          oldPos+=1;
          return options->navCodes[upCmd].ch;
        }
	if (ir.ok) {
	  ir.ok=false;
	  return options->navCodes[enterCmd].ch;
	}
	if (ir.escape){
	  ir.escape=false;
	  return options->navCodes[escCmd].ch;
	}

	if (ir.ind >=0){
	  char ind [10];
	  itoa (ir.ind,ind,10);
	  ir.ind=-1;
	  Serial.println(ind);
	  return ind[0];
	}
	return -1;
      }

      void flush() {
	oldPos=ir.pos;
	ir.ok=false;
	ir.escape=false;
	ir.ind=-1;
      }

      size_t write(uint8_t v) {oldPos=v;return 1;}

    };

  }//namespace Menu
#endif
