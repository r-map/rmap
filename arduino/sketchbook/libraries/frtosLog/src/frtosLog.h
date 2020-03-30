/*
  Log library for FreeRTOS
  version 1.0.0

Copyright (C) 2020  Paolo Patruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef FRTOSLOGGING_H
#define FRTOSLOGGING_H
#include <ArduinoLog.h>
#ifdef ARDUINO_ARCH_AVR
  #include <ArduinoSTL.h>
  #include <Arduino_FreeRTOS.h>
#else 
  #ifdef ARDUINO_ARCH_STM32
    #include "STM32FreeRTOS.h"
  #else
    #include "FreeRTOS.h"
  #endif
#endif

#include <mutex.hpp>

using namespace cpp_freertos;

// *************************************************************************
//  Uncomment line below to fully disable logging, and reduce project size
// ************************************************************************
//#define DISABLE_LOGGING

#define FRTOSLOGGING_VERSION 1_0_0

/*!
 frtosLogging is a FreeRTOS wrapper to ArduinoLog helper class to 
 output informations over RS232.
*/

#if (LOG_LEVEL >= LOG_FATAL)
#define FRTOSLOGF(x, ...) ({frtosLog.fatal(x, ##__VA_ARGS__);})
#else
#define FRTOSLOGF(x, ...)
#endif

#if (LOG_LEVEL >= LOG_ERROR)
#define FRTOSLOGE(x, ...) ({frtosLog.error(x, ##__VA_ARGS__);})
#else
#define FRTOSLOGE(x, ...)
#endif

#if (LOG_LEVEL >= LOG_WARNING)
#define FRTOSLOGW(x, ...) ({frtosLog.warning(x, ##__VA_ARGS__);})
#else
#define FRTOSLOGW(x, ...)
#endif

#if (LOG_LEVEL >= LOG_INFO)
#define FRTOSLOGI(x, ...) ({frtosLog.info(x, ##__VA_ARGS__);})
#else
#define FRTOSLOGI(x, ...)
#endif

#if (LOG_LEVEL >= LOG_NOTICE)
#define FRTOSLOGN(x, ...) ({frtosLog.notice(x, ##__VA_ARGS__);})
#else
#define FRTOSLOGN(x, ...)
#endif

#if (LOG_LEVEL >= LOG_TRACE)
#define FRTOSLOGT(x, ...) ({frtosLog.trace(x, ##__VA_ARGS__);})
#else
#define FRTOSLOGT(x, ...)
#endif

#if (LOG_LEVEL >= LOG_VERBOSE)
#define FRTOSLOGV(x, ...) ({frtosLog.verbose(x, ##__VA_ARGS__);})
#define FRTOSLOGD(x, ...) ({frtosLog.verbose(x, ##__VA_ARGS__);})
#else
#define FRTOSLOGV(x, ...)
#define FRTOSLOGD(x, ...)
#endif

class frtosLogging {
private:
  Logging _logging;
  MutexStandard _semaphore;

public:
  /*!
   * default Constructor
   */
  frtosLogging()
    : _logging(),
      _semaphore() {}


    /**
    * Initializing, must be called as first. Note that if you use
    * this variant of Init, you need to initialize the baud rate
    * yourself, if printer happens to be a serial port.
    * \param level - logging levels <= this will be logged.
    * \param printer - place that logging output will be sent to.
    * \return void
    *
    */
  
  void begin(int level, Print *output, MutexStandard &semaphore, bool showLevel=true);
  
  /**
   * Sets a function to be called before each log command.
   */
  void setPrefix(printfunction);
  
  /**
   * Sets a function to be called after each log command.
   */
  void setSuffix(printfunction);
  
  /**
   * Output a fatal error message. Output message contains
   * F: followed by original message
   * Fatal error messages are printed out at
   * loglevels >= LOG_LEVEL_FATAL
   *
   * \param msg format string to output
   * \param ... any number of variables
   * \return void
   */
  template <class T, typename... Args> void fatal(T msg, Args... args){
#ifndef DISABLE_LOGGING
    LockGuard guard(_semaphore);
    _logging.fatal( msg, args...);
#endif
  }
  
  /**
   * Output an error message. Output message contains
   * E: followed by original message
   * Error messages are printed out at
   * loglevels >= LOG_LEVEL_ERROR
   *
   * \param msg format string to output
   * \param ... any number of variables
   * \return void
   */
  template <class T, typename... Args> void error(T msg, Args... args){
#ifndef DISABLE_LOGGING
    LockGuard guard(_semaphore);
    _logging.error( msg, args...);
#endif
  }
  /**
   * Output a warning message. Output message contains
   * W: followed by original message
   * Warning messages are printed out at
   * loglevels >= LOG_LEVEL_WARNING
   *
   * \param msg format string to output
   * \param ... any number of variables
   * \return void
   */
  
  template <class T, typename... Args> void warning(T msg, Args...args){
#ifndef DISABLE_LOGGING
    LockGuard guard(_semaphore);
    _logging.warning( msg, args...);
#endif
  }
    /**
     * Output a notice message. Output message contains
     * N: followed by original message
     * Notice messages are printed out at
     * loglevels >= LOG_LEVEL_NOTICE
     *
     * \param msg format string to output
     * \param ... any number of variables
     * \return void
     */
  
  template <class T, typename... Args> void notice(T msg, Args...args){
#ifndef DISABLE_LOGGING
    LockGuard guard(_semaphore);
    _logging.notice( msg, args...);
#endif
  }
  /**
   * Output a trace message. Output message contains
   * N: followed by original message
   * Trace messages are printed out at
   * loglevels >= LOG_LEVEL_TRACE
   *
   * \param msg format string to output
   * \param ... any number of variables
   * \return void
   */
  template <class T, typename... Args> void trace(T msg, Args... args){
#ifndef DISABLE_LOGGING
    LockGuard guard(_semaphore);
    _logging.trace( msg, args...);
#endif
  }
  
  /**
   * Output a verbose message. Output message contains
   * V: followed by original message
   * Debug messages are printed out at
   * loglevels >= LOG_LEVEL_VERBOSE
   *
   * \param msg format string to output
   * \param ... any number of variables
   * \return void
   */
  template <class T, typename... Args> void verbose(T msg, Args... args){
#ifndef DISABLE_LOGGING
    LockGuard guard(_semaphore);
    _logging.verbose( msg, args...);
#endif
  }

};

extern frtosLogging frtosLog;
#endif

