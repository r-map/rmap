/*! \mainpage STIMA Readme
\section introduction Introduction
software to collect weather data contributed by citizens; to make
these data available for weather services and homeland security; to
provide feedback to the data contributors so that they have the tools
to check and improve their data quality

http://rmap.cc

\section howto Howto

<!---
################################################################################
# Ethernet
################################################################################
-->

\subsection stima_ethernet STIMA over Ethernet:

\image html ethernet.jpg width=300px
\image latex ethernet.jpg

\subsubsection stima_ethernet_hardware Hardware

1) Stima I2C-Base @ 5V

2) Microduino Ethernet WIZ

3) Microduino RJ45

4) Stima core+1284 @ 5V

5) Stima I2C-RTC @ 5V

6) Stima FT232RL

7) Stima SD-Card

\subsubsection stima_ethernet_software Software

1) open sketch arduino/sketchbook/rmap/rmap/rmap.ino

2) in arduino/sketchbook/rmap/rmap/rmap-config.h set STIMA_MODULE_TYPE_REPORT_ETH or STIMA_MODULE_TYPE_SAMPLE_ETH in MODULE_VERSION define

3) open arduino/sketchbook/libraries/RmapConfig/sensors_config.h and set true or false
sensors's define and json's define in order to enable or disable relative sensor's driver and library

4) compile and upload firmware

5) short-circuit the two configure pins with a jumper and configure it!

<!---
################################################################################
# GSM/GPRS
################################################################################
-->

\subsection stima_gsm STIMA over GSM/GPRS:

\image html gsm.jpg width=300px
\image latex gsm.jpg

\subsubsection stima_gsm_hardware Hardware

1) Stima I2C-Base @ 5V

2) Stima SIM800C Power

3) Stima SIM800C Module

4) Stima core+1284 @ 5V

5) Stima I2C-RTC @ 5V

6) Stima FT232RL

7) Stima SD-Card

\subsubsection stima_gsm_software Software

1) open sketch arduino/sketchbook/rmap/rmap/rmap.ino

2) in arduino/sketchbook/rmap/rmap/rmap-config.h set STIMA_MODULE_TYPE_REPORT_GSM or STIMA_MODULE_TYPE_SAMPLE_GSM in MODULE_VERSION define

3) open arduino/sketchbook/libraries/RmapConfig/sensors_config.h and set true or false
sensors's define and json's define in order to enable or disable relative sensor's driver and library

4) compile and upload firmware

5) short-circuit the two configure pins with a jumper and configure it!

<!---
################################################################################
# Passive
################################################################################
-->

\subsection stima_passive STIMA Passive:

\image html passive.jpg width=300px
\image latex passive.jpg

\subsubsection stima_passive_hardware Hardware

1) Stima I2C-Base @ 5V / 3.3V

2) Stima core+1284 @ 5V / Stima core+644 @ 3.3V

3) Stima I2C-RTC @ 5V / 3.3V

4) Stima FT232RL

\subsubsection stima_passive_software Software

1) open sketch arduino/sketchbook/rmap/rmap/rmap.ino

2) in arduino/sketchbook/rmap/rmap/rmap-config.h set STIMA_MODULE_TYPE_PASSIVE in MODULE_VERSION define

3) open arduino/sketchbook/libraries/RmapConfig/sensors_config.h and set true or false
sensors's define and json's define in order to enable or disable relative sensor's driver and library

4) compile and upload firmware

5) short-circuit the two configure pins with a jumper and configure it!

<!---
################################################################################
# I2C-TH
################################################################################
-->

\subsection i2c-th STIMA I2C-TH:

\image html th.jpg width=300px
\image latex th.jpg

\subsubsection stima_i2c_th_hardware Hardware

1) Stima I2C-Base @ 3.3V

2) Stima core+644 @ 3.3V

3) Stima FT232RL

4) Stima SD-Card

\subsubsection stima_i2c_th_software Software

1) open sketch arduino/sketchbook/rmap/i2c-th/i2c-th.ino

2) open arduino/sketchbook/libraries/RmapConfig/sensors_config.h and set true or false
sensors's define and json's define in order to enable or disable relative sensor's driver and library

3) compile and upload firmware

<!---
################################################################################
# I2C-Rain
################################################################################
-->

\subsection i2c-rain STIMA I2C-Rain:

\image html rain.jpg width=300px
\image latex rain.jpg

\subsubsection stima_i2c_rain_hardware Hardware

1) Stima I2C-Base @ 3.3V

2) Stima core+644 @ 3.3V

3) Stima I2C-Digital

4) Stima FT232RL

5) Stima SD-Card

\subsubsection stima_i2c_rain_software Software

1) open sketch arduino/sketchbook/rmap/i2c-rain/i2c-rain.ino

2) open arduino/sketchbook/libraries/RmapConfig/sensors_config.h and set false
in all sensors's define and json's define

3) compile and upload firmware

<!---
################################################################################
# Station assembly
################################################################################
-->

\subsection station STIMA Meteo Station assembly

\image html station.jpg width=900px
\image latex station.jpg

1) Stima over Ethernet or Stima over GSM

--> connect with a cable at 5V hub port
--> in GSM/GPRS version: connect SMA antenna and insert a SIM card
--> in Ethernet version: connect ethernet/POE cable

2) Stima I2C-TH

--> connect with a cable at 3.3V hub port

3) Stima I2C-Rain

--> connect with a cable at 3.3V hub port

--> connect at tipping bucket rain on 2 external pins of Stima I2C-Digital

4) I2C sensor's:

--> connect with a cable at 3.3V or 5V hub port

5) I2C LCD Display

--> connect with a cable at 5V hub port

6) Stima I2C-HUB

Power up the station through one of the following ways:

1) USB power supply with USB type B connector

2) Plug a POE cable into RJ45 interface (only for ethernet version)

3) 5V DC power supply through hub input port

4) DigitecoPower through hub input port with capability of 12V battery backup, solar panel or 12-30V DC input source voltage

in that case, the pins on the DigitecoPower module are:

1) VCC_IN: 12-30V DC input source VCC (+)

2) GND_IN: 12-30V DC input source GND (-)

3) VCC_BAT: 12V DC input/otput battery backup VCC (+)

4) GND_BAT: 12V DC input/otput battery backup GND (-)

5) Status LED: green for battery charged, orange for medium charged battery, red for low battery

6) VCC_OUT: 5V DC output for input hub connector VCC (+)

7) SCL: I2C SCL for input hub connector

8) SDA: I2C SDA for input hub connector

9) GND_OUT: 5V DC output for input hub connector GND (-)

<!---
################################################################################
# Library
################################################################################
-->

\section library Project Library

For details, look at the specific library files.

\subsection rmapconfig RmapConfig

This library contains the definitions that are useful for configuring some default values.
Below is a list of the files contained therein.

debug_config.h: Enable or disable debug in sketch and library

ethernet_config.h: Ethernet configuration's parameters (IP, DHCP, delay, ecc..)

gsm_config.h: GSM configuration's parameters (APN, username, ecc..)

hardware_config.h: Hardware configuration's parameters (I2C bus clock, ecc..)

json_config.h: JSON configuration's parameters (buffer length)

lcd_config.h: LCD configuration's parameters (rows, columns, ecc..)

mqtt_config.h: MQTT configuration's parameters (topic length, buffers length, ecc..)

ntp_config.h: NTP configuration's parameters (timezone, server, ecc..)

sdcard_config.h: SDCARD configuration's parameters (name length, ecc..)

sensors_config.h: Enable or disable sensor driver sensors for specific sketch

\subsection rmap Rmap

This library contains generic utility features. Below is a list of the files contained therein.

debug.h debug.cpp: Debugging functions for print debug message on serial port or LCD

eeprom_utility.h eeprom_utility.cpp: EEPROM utility for write and read eeprom

i2c_utility.h i2c_utility.cpp: I2C utility for bus recovery

registers.h: General register's define

registers-th.h: I2C-TH register's define

registers-rain.h: I2C-Rain register's define

rmap_utility.h rmap_utility.cpp: RMAP useful functions

sdcard_utility.h sdcard_utility.cpp: SD-Card useful functions

stima_module.h: STIMA station's definition

typedef.h: Useful project typedef

\subsection sensordriver SensorDriver

This library is provided to read measurements from I2C sensors.

SensorDriverSensors.h: define list with sensor names in SensorDriver

SensorDriver.h SensorDriver.cpp: SensorDriver library files

\subsection hyt2x1 HYT2X1

This library implements functions for read and configure HYT271 and HYT221 sensors.

hyt2x1.h hyt2x1.cpp: HYT2X1 library files

\subsection ntp NTP

This library implements NTP functions for read time over NTP server with ethernet client or sim800 client.

ntp.h ntp.cpp: NTP library files

\subsection pcf8563 PCF8563

This library implements PCF8563 functions for communicate with pcf8563 real time clock.

pcf8563.h pcf8563.cpp: PCF8563 library files

\subsection sim800 SIM800

This library implements SIM800 functions for communicate with SIM800C/SIM800L GSM/GPRS module.

sim800.h sim800.cpp: SIM800 library files

sim800Client.h: SIM800 library interface for Arduino Client.

SIM800C is fully supported, SIM800L is partially supported (coming soon...)

\subsection implemented Implemented features

\subsubsection transport Transport

o) Serial: yes

o) Ethernet: partial (basic functions are present but need to interface with Ethernet Client)

o) MQTT: partial (subscribe functions are present but need to interface with rpc process function)

See ArduinoJsonRPC library

\subsubsection files SD-Card files

On the sdcard there is a file called mqtt_ptr.txt containing a binary data in uint32_t format
corresponding to the seconds passed since 00:00:00 01/01/1970 indicating the last data sent by MQTT.

The data is recorded on files (one file for each recording day) named in the format yyyy_mm_dd.txt
and each data recorded on sd card is MQTT_SENSOR_TOPIC_LENGTH + MQTT_MESSAGE_LENGTH bytes long (look at the mqtt_config.h file).

Each recorded data has the format of the type: TRANGE/LEVEL/VAR { “v”: VALUE, “t”: TIME}

\subsubsection sensordriversensors SensorDriver's sensors

o) ADT7420 (ADT)

o) HIH6100 (HIH)

o) HYT221 (HYT)

o) HYT271 (HYT)

o) DigitecoPower (DEP)

o) I2C-TH (STH, ITH, NTH, MTH, XTH)

o) I2C-Rain (TBS, TBR)

o) I2C-Wind (DW1)

other sensors can be easily integrated (see SensorDriver library).
