frtosLog - C++ Log library for Arduino devices and FreeRTOS
This is a wrapper to ArduinoLog with semaphore management for FreeRTOS
used from c++ wrapper freeRTOS_addon
=======================================================================

*An minimalistic Logging framework  for Arduino-compatible embedded systems.*

frtosLog is a minimalistic framework to help the programmer output
log statements to an output of choice, fashioned after extensive
logging libraries such as log4cpp ,log4j and log4net. In case of
problems with an application, it is helpful to enable logging so that
the problem can be located. ArduinoLog is designed so that log
statements can remain in the code with minimal performance cost. In
order to facilitate this the loglevel can be adjusted, and (if your
code is completely tested) all logging code can be compiled out.

## Features

* Different log levels (Error, Info, Warn, Debug, Verbose )
* Supports multiple variables
* Supports formatted strings 
* Supports formatted strings from flash memory
* Fixed memory allocation (zero malloc)
* manage the output stream with a MuxStandard semaphore for use by FreeRTOS
* GPL3 License

## Tested for 

* All Arduino boards (Uno, Due, Mini, Micro, Yun...)
* ESP8266

ArduinoLog is provided Copyright © 2020 under GPL3 License.
