/* -*- C++ -*- */

//for using standard arduino hd44780_I2Cexp driver

#ifndef RSITE_ARDUINO_MENU_HD44780_I2CEXPOUT
  #define RSITE_ARDUINO_MENU_HD44780_I2CEXPOUT
  #include "../menuDefs.h"
  #include <hd44780.h>
  #include <hd44780ioClass/hd44780_I2Cexp.h> // i2c LCD i/o class header

  namespace Menu {

    class liquidCrystalOut:public cursorOut {
      public:
        hd44780_I2Cexp& device;
        inline liquidCrystalOut(hd44780_I2Cexp& o,idx_t *t,panelsList &p,menuOut::styles s=minimalRedraw)
          :cursorOut(t,p,s),device(o) {}
        size_t write(uint8_t ch) override {return device.write(ch);}
        void clear() override {
          device.clear();
          panels.reset();
        }
        void setCursor(idx_t x,idx_t y,idx_t panelNr=0) override {
          const panel p=panels[panelNr];
          device.setCursor(p.x+x,p.y+y);
        }
        idx_t startCursor(navRoot& root,idx_t x,idx_t y,bool charEdit,idx_t panelNr=0) override {return 0;}
        idx_t endCursor(navRoot& root,idx_t x,idx_t y,bool charEdit,idx_t panelNr=0) override {return 0;}
        idx_t editCursor(navRoot& root,idx_t x,idx_t y,bool editing,bool charEdit,idx_t panelNr=0) override {
          //text editor cursor
          device.noBlink();
          device.noCursor();
          if (editing) {
            device.setCursor(x, y);
            if (charEdit) device.cursor();
            else device.blink();
          }
          return 0;
        }
    };

  }//namespace Menu
#endif
