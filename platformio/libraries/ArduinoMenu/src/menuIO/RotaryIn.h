/* -*- C++ -*- */
/********************
Sept. 2014 Rui Azevedo - ruihfazevedo(@rrob@)gmail.com
Dece. 2018 Paolo Patruno - p.patruno@iperbole.bologna.it

Rotary incremental encoder using Rotary library
*/

#ifndef RSITE_ARDUINO_MENU_ROTARY_ENCODER
  #define RSITE_ARDUINO_MENU_ROTARY_ENCODER

  #include <Rotary.h>
  #include "../menuDefs.h"

  namespace Menu {

    template<uint8_t pinA,uint8_t pinB>
    class encoderIn {
    public:
      volatile int pos=0;
      Rotary r = Rotary(pinA, pinB);
      //int pinA,pinB;
      //encoderIn<pinA,pinB>(int a,int b):pinA(a),pinB(b) {}
      void begin(bool pullup=true) {
	r.begin(pullup);
      }

      void process() {
	unsigned char result = r.process();
	if (result) {
	  if (result == DIR_CW ){
	    pos+=1;
	  }else{
	    pos-=1;
	  }
	}
      }
    };

    //emulate a stream based on encoderIn movement returning +/- for every 'sensivity' steps
    //buffer not needer because we have an accumulator
    template<uint8_t pinA,uint8_t pinB>
    class encoderInStream:public menuIn {
    public:
      encoderIn<pinA,pinB> &enc;//associated hardware encoderIn
      int oldPos=0;
      encoderInStream(encoderIn<pinA,pinB> &enc):enc(enc) {}

      int available(void) {
	return abs(enc.pos-oldPos);
      }

      int peek(void) override {
        int d=enc.pos-oldPos;
        if (d<=-1)return options->navCodes[downCmd].ch;
        if (d>=1) return options->navCodes[upCmd].ch;
        return -1;
      }

      int read() override {
        int d=enc.pos-oldPos;
        if (d<=-1) {
          oldPos-=1;
          return options->navCodes[downCmd].ch;
        }
        if (d>=1) {
          oldPos+=1;
          return options->navCodes[upCmd].ch;
        }
        return -1;
      }

      void flush() {oldPos=enc.pos;}

      size_t write(uint8_t v) {oldPos=v;return 1;}

    };

  }//namespace Menu
#endif
