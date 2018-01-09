/*
    _   ___ ___  _   _ ___ _  _  ___  _    ___   ___ 
   /_\ | _ \   \| | | |_ _| \| |/ _ \| |  / _ \ / __|
  / _ \|   / |) | |_| || || .` | (_) | |_| (_) | (_ |
 /_/ \_\_|_\___/ \___/|___|_|\_|\___/|____\___/ \___|
                                                     
  Log library for Arduino
  version 1.0.0
  https://github.com/thijse/Arduino-Log

Licensed under the MIT License <http://opensource.org/licenses/MIT>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ArduinoLog.h"

void Logging::begin(int level, Print* logOutput, bool showLevel){
    _level     = constrain(level,LOG_LEVEL_SILENT,LOG_LEVEL_VERBOSE);
	_showLevel = showLevel;
    _logOutput = logOutput;
}

void Logging::setPrefix(printfunction f){
    _prefix = f;
}

void Logging::setSuffix(printfunction f){
    _suffix = f;
}

void Logging::print(const __FlashStringHelper *format, va_list args) {
#ifndef DISABLE_LOGGING	  	
  PGM_P p = reinterpret_cast<PGM_P>(format);
  char c = pgm_read_byte(p++);
  for(;c != 0; c = pgm_read_byte(p++)){
    if (c == '%') {
      c = pgm_read_byte(p++);
      printFormat(c, &args);
    } else {
      _logOutput->print(c);
    }
  }
#endif	  
}

void Logging::print(const char *format, va_list args) {
#ifndef DISABLE_LOGGING	  	
  for (; *format != 0; ++format) {
    if (*format == '%') {
      ++format;
      printFormat(*format, &args);
    } else {
      _logOutput->print(*format);
    }
  }
#endif	  
}

void Logging::printFormat(const char format, va_list *args) {
#ifndef DISABLE_LOGGING	  	
  if (format == '\0') return;

  if (format == '%') {
    _logOutput->print(format);
    return;
  }

  if( format == 's' ) {
    register char *s = (char *)va_arg( *args, int );
    _logOutput->print(s);
    return;
  }

  if( format == 'd' || format == 'i') {
    _logOutput->print(va_arg( *args, int ),DEC);
    return;
  }

  if( format == 'D' || format == 'F') {
    _logOutput->print(va_arg( *args, double ));
    return;
  }

  if( format == 'x' ) {
    _logOutput->print(va_arg( *args, int ),HEX);
    return;
  }

  if( format == 'X' ) {
    _logOutput->print("0x");
    _logOutput->print(va_arg( *args, int ),HEX);
    return;
  }

  if( format == 'b' ) {
    _logOutput->print(va_arg( *args, int ),BIN);
    return;
  }

  if( format == 'B' ) {
    _logOutput->print("0b");
    _logOutput->print(va_arg( *args, int ),BIN);
    return;
  }

  if( format == 'l' ) {
    _logOutput->print(va_arg( *args, long ),DEC);
    return;
  }

  if( format == 'c' ) {
    _logOutput->print((char) va_arg( *args, int ));
    return;
  }

  if( format == 't' ) {
    if (va_arg( *args, int ) == 1) {
      _logOutput->print("T");
    }
    else {
      _logOutput->print("F");
    }
    return;
  }

  if( format == 'T' ) {
    if (va_arg( *args, int ) == 1) {
      _logOutput->print(F("true"));
    }
    else {
      _logOutput->print(F("false"));
    }
    return;
  }
#endif	  
}
 
Logging Log = Logging();

