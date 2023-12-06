# Nmea, GPS NMEA 0183 parser library
 
[![C/C++ 
CI](https://github.com/kosma/minmea/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/hmz06967/OZGPS_NMEA/actions/workflows/c-cpp.yml)

A library that parses NMEA gps record from Uart or any source ``"gps.encode(char c);"`` can parse sequentially with a single character.

## Supported sentences

* ``TXT``
* ``RMC``
* ``GGA``
* ``GLL``
* ``VTG``
* ``GSA``
* ``GSV``
* ``MSS``
* ``TRF``
* ``STN``
* ``XTE``
* ``ZDA``

Read the description in the library to add more options. !You can do this!
on NMEA is at https://gpsd.gitlab.io/gpsd/NMEA.html

## Start
``#include "ozgps.h" ``

## Set Filter NMEA Sentence

Becareful!!

``uint16_t filter = 0x37;``

``gps.set_filter(filter);" ``

By specifying a filter, you can remove the sentence that you do not want parsed in NMEA. The program first determines the data type and sees if it can be bypassed.

``{"TXT","RMC","GGA","GLL","VTG","GSA","GSV","MSS","TRF","STN","XTE","ZDA"};``

``b: 1     1     1     0     1     1     0     0     0     0     0     0 ->> 0x37``   

``//** bit2hex(12bit:4096);  ``

Output sentences: 

``0x37-->> "TXT","RMC","GGA","VTG","GSA"``

skipped over other sentences and not parsed


## Data type for sentence

The data format is like this, there are more data formats in the library. ``"NMEA"`` retains the type of format it supports. For example, in float data type, NMEA data is the same as ``"struct"`` structure.

* ``struct minmea_sentence_txt txt``

```c

struct minmea_sentence_txt
{
    uint8_t msgid;
    uint8_t msgrange;
    enum minmea_ident_text ident;
    char* msg;
};

```

## Coordinate format

NMEA uses ``DDMM.MMMM`` format which, honestly, is not good in the internet era. Internally, minmea stores it as a fractional number for practical uses, the value should be probably converted to the ``DD.DDDDD`` floating point format using the following.

* ``MGPS mgps;``
* ``gps.init(&mgps);``
* ``...``
* ``... gps.encode(c);``
* ``float latitude = mgps.rmc.dms.latitude; //return 51.115680``
* ``-->> 51.115680``

```c
struct minmea_coordinate
{
    float latitude;
    char latdirect;
    float longitude;
    char longdirect;
};
```

## Example

```c
#include "../src/ozgps.h"

int main(){
    //init gps
    uint8_t gpsflag;
    MGPS mgps;
    gps.init(&mgps);
    uint32_t t = 0;

    const char* console_text = "$GPTXT,01,01,02,ANTSTATUS=INIT*25\r\n$GPRMC,,V,,,,,,,,,,N*53\r\n$GPVTG,,,,,,,,,N*30\r\n$GPGGA,,,,,,0,00,99.99,,,,,,*48\r\n$GPGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*30\r\n$GPGLL,,,,,,V,N*64\r\n$GPXTE,A,A,0.67,L,N*6F\r\n$GPXTE,A,A,0.67,L,N*6f\r\n$GPGGA,123204.00,5106.94086,N,01701.51680,E,1,06,3.86,127.9,M,40.5,M,,*51\r\n$GPGSA,A,3,02,08,09,05,04,26,,,,,,,4.92,3.86,3.05*00\r\n$GPGSV,4,1,13,02,28,259,33,04,12,212,27,05,34,305,30,07,79,138,*7F\r\n$GPGSV,4,2,13,08,51,203,30,09,45,215,28,10,69,197,19,13,47,081,*76\r\n$GPGSV,4,3,13,16,20,040,17,26,08,271,30,28,01,168,18,33,24,219,27*74\r\n$GPGSV,4,4,13,39,31,170,27*40\r\n$GPGLL,5106.94086,N,01701.51680,E,123204.00,A,A*63\r\n$GPRMC,123205.00,A,5106.94085,N,01701.51689,E,0.016,,280214,,,A*7B\r\n$GPVTG,,T,,M,0.016,N,0.030,K,A*27\r\n$GPGST,024603.00,3.2,6.6,4.7,47.3,5.8,5.6,22.0*58\r\n$GPZDA,160012.71,11,03,2004,-1,00*7D\r\nGNGBS,170556.00,3.0,2.9,8.3,,,,*5C\r\n";

    for (size_t i = 0; i < strlen(console_text); i++)
    {
        gpsflag = gps.encode(console_text[i]);
        if(gps.valid){
            t++;
            printf("parser_ok: %d\n", t);
        }
    }

    printf("latitude: %f", mgps.rmc.dms.latitude);
    printf("gps_error: %d", gpsflag);

}

```

## Running unit tests

Building and running the tests requires the following:

* CMake
* Check Framework (https://libcheck.github.io/check/).
* Clang Static Analyzer (https://clang-analyzer.llvm.org/).

If you have both in your ``$PATH``, running the tests should be as simple as:

```
mkdir build
cd build
cmake ../
make
make test
//or
g++ -o example *.cpp .\example
```

## Licensing

Minmea is open source software; see ``COPYING`` for amusement. Email me if the
license bothers you and I'll happily re-license under anything else under the sun.
