#ifndef _PMS_CONFIG_H_
#define _PMS_CONFIG_H_

////////////////////////////////////////////

// Use one of: 
// it depends on Serial Library (and serial pin connection)

#if not defined(PMS_SOFTSERIAL)
#define PMS_SOFTSERIAL
#endif

////////////////////////////////////////////

// Use PMS_DYNAMIC to be C++ strict
// Without PMS_DYNAMIC: 
//   con: Pmsx003 related object should be defined as global variable (C style): Pmsx003 pms;
//   con: It can not be initialized inside constructor. It is too early, serial ports and other pins will be redefined by Arduino bootloader
//   con: It has to be initialzed within setup() - see examples: uses of begin()
// With PMS_DYNAMIC:
//   pro: Pmsx003 related object instance should be created (C++ style) using _pms = new Pmsx003();
//   pro: It can be created within setup(), it is good time to initialze class instance in the constructor. Arduino board was initialzed here.
//   con: If you are not using heap: it uses heap, it adds meaningful code overhead - malloc, new() and memory mangement should be included. ( 0.5kb of program memory, a few memory bytes)

// #define PMS_DYNAMIC

////////////////////////////////////////////

// Undef to use modern min() template function instead of min() macro. I hate unnecessary macros.
// Works with Visual Studio, compiler errors with Arduino IDE.

// #define NOMINMAX

////////////////////////////////////////////

#if defined PMS_SOFTSERIAL
#include <SoftwareSerial.h>
#endif

////////////////////////////////////////////

#endif
