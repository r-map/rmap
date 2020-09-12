/**********************************************************************
Copyright (C) 2014  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Paruno <p.patruno@iperbole.bologna.it>

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

 * derived work from:
 * I2CGPS  - Inteligent GPS and NAV module for MultiWii by EOSBandi
 * V2.2
 */


/*********************************************************************
 *
 * This program implements position and time functions
 * by serial port gps parsing exported to i2c interface.
 * 
**********************************************************************/

#define VERSION 01                                 //Software version for cross checking


#ifdef ARDUINO_ARCH_AVR
#include <avr/wdt.h>
#endif
#include "Wire.h"
#include "registers-gps.h"                         //Register definitions
#include "config.h"

#define REG_MAP_SIZE       sizeof(I2C_REGISTERS)   //size of register map
#define MAX_SENT_BYTES     0x0F                    //maximum amount of data that I could receive from a master device (register, plus 15 byte)


#if defined(INIT_MTK_GPS)

#define LAT  0
#define LON  1

static int32_t GPS_read[2];

 #define MTK_SET_BINARY          "$PGCMD,16,0,0,0,0,0*6A\r\n"
 #define MTK_SET_NMEA            "$PGCMD,16,1,1,1,1,1*6B\r\n"
 #define MTK_SET_NMEA_SENTENCES  "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"
 #define MTK_OUTPUT_4HZ          "$PMTK220,250*29\r\n"
 #define MTK_OUTPUT_5HZ          "$PMTK220,200*2C\r\n"
 #define MTK_OUTPUT_10HZ         "$PMTK220,100*2F\r\n"
 #define MTK_NAVTHRES_OFF        "$PMTK397,0*23\r\n" // Set Nav Threshold (the minimum speed the GPS must be moving to update the position) to 0 m/s  
 #define SBAS_ON                 "$PMTK313,1*2E\r\n"
 #define WAAS_ON                 "$PMTK301,2*2E\r\n"
 #define SBAS_TEST_MODE		 "$PMTK319,0*25\r\n"	//Enable test use of sbas satelite in test mode (usually PRN124 is in test mode)

#endif


typedef struct {
  uint8_t    sw_version;     // 0x00  Version of the I2C_GPS sw
  uint8_t    gps2dfix;
  uint8_t    gps3dfix;
  uint8_t    numsats;
} status_register_t;

typedef struct {
  long      lat;            //degree*10 000 000
  long      lon;            //degree*10 000 000
} gps_coordinates_t;

typedef struct {
  int16_t	year;
  int8_t	month;
  int8_t	day;
  int8_t	hour;
  int8_t	min;
  int8_t        sec;
} datetime_t;

typedef struct {

//Status registers
  status_register_t     status;                   // 0x01  status register

//GPS & navigation data
  gps_coordinates_t     gps_loc;                  // 0x04 current location (8 byte) lat,lon
  int16_t               altitude;                 // 0x0C gps altitude
  uint16_t	        ground_course;	          // 0x0E GPS ground course
  uint16_t              ground_speed;             // 0x10 ground speed from gps m/s*100
  uint32_t		time;	                  // 0x12 UTC Time from of gps_loc
  datetime_t            datetime;                 // 0x16 UTC Date and Time
} __attribute__((packed)) I2C_REGISTERS;

static I2C_REGISTERS   i2c_buffer1;
static I2C_REGISTERS   i2c_buffer2;

static I2C_REGISTERS*   i2c_dataset1;
static I2C_REGISTERS*   i2c_dataset2;
static I2C_REGISTERS*   i2c_datasettmp;

byte i2c_writabledataset[0xE0];  // unused !

static uint8_t         receivedCommands[MAX_SENT_BYTES];
static uint8_t         new_command;                        //new command received (!=0)


#ifdef DEBUGSOFTWARESERIAL
#include <SoftwareSerial.h>
SoftwareSerial dbgSerial(rxPin,txPin);
#endif


////////////////////////////////////////////////////////////////////////////////////
// Utilities
//

#if defined(NMEA)

/* The latitude or longitude is coded this way in NMEA frames
  dddmm.mmmm   coded as degrees + minutes + minute decimal
  This function converts this format in a unique unsigned long where 1 degree = 10 000 000
  I increased the precision here, even if we think that the gps is not precise enough, with 10e5 precision it has 76cm resolution
  with 10e7 it's around 1 cm now. Increasing it further is irrelevant, since even 1cm resolution is unrealistic, however increased 
  resolution also increased precision of nav calculations
*/

#define DIGIT_TO_VAL(_x)	(_x - '0')
uint32_t GPS_coord_to_degrees(char* s)
{
	char *p, *q;
	uint8_t deg = 0, min = 0;
	unsigned int frac_min = 0;

	// scan for decimal point or end of field
	for (p = s; isdigit(*p); p++)
		;
	q = s;

	// convert degrees
	while ((p - q) > 2) {
		if (deg)
			deg *= 10;
		deg += DIGIT_TO_VAL(*q++);
	}
	// convert minutes
	while (p > q) {
		if (min)
			min *= 10;
		min += DIGIT_TO_VAL(*q++);
	}
	// convert fractional minutes
	// expect up to four digits, result is in
	// ten-thousandths of a minute
	if (*p == '.') {
		q = p + 1;
		for (int i = 0; i < 4; i++) {
			frac_min *= 10;
			if (isdigit(*q))
				frac_min += *q++ - '0';
		}
	}
	return deg * 10000000UL + (min * 1000000UL + frac_min*100UL) / 6;
}

/* This is am expandable implementation of a GPS frame decoding
   This should work with most of modern GPS devices configured to output NMEA frames.
   It assumes there are some NMEA GGA, GSA and RMC frames to decode on the serial bus
   Using the following data :
   GGA
     - time
     - latitude
     - longitude
     - GPS fix 
     - GPS num sat (5 is enough to be +/- reliable)
     - GPS alt
   GSA
     - 3D fix (it could be left out since we have a 3D fix if we have more than 4 sats  
   RMC
     - GPS speed over ground, it will be handy for wind compensation (future)  
     
*/

#define NO_FRAME    0
#define GPGGA_FRAME 1
#define GPGSA_FRAME 2
#define GPRMC_FRAME 3

bool GPS_NMEA_newFrame(char c) {

  uint8_t frameOK = 0;
  static uint8_t param = 0, offset = 0, parity = 0;
  static char string[15];
  static uint8_t checksum_param, gps_frame = NO_FRAME;

  switch (c) {
    case '$': param = 0; offset = 0; parity = 0; 
              break;
    case ',':
    case '*':  string[offset] = 0;
                if (param == 0) { //frame identification
                  gps_frame = NO_FRAME;  
                  if (string[0] == 'G' && string[1] == 'P' && string[2] == 'G' && string[3] == 'G' && string[4] == 'A') gps_frame = GPGGA_FRAME;
                  if (string[0] == 'G' && string[1] == 'P' && string[2] == 'G' && string[3] == 'S' && string[4] == 'A') gps_frame = GPGSA_FRAME;
                  if (string[0] == 'G' && string[1] == 'P' && string[2] == 'R' && string[3] == 'M' && string[4] == 'C') gps_frame = GPRMC_FRAME;
                }
                
                switch (gps_frame)
                {
                  //************* GPGGA FRAME parsing
                  case GPGGA_FRAME: 
                    switch (param)
                     {
                      case 1: i2c_dataset1->time = (atof(string)*1000);      //up to .000 s precision not needed really but the data is there anyway
                              break;
                      //case 2: i2c_dataset1->gps_loc.lat = GPS_coord_to_degrees(string);
                      case 2: GPS_read[LAT] = GPS_coord_to_degrees(string);
                              break;
                      //case 3: if (string[0] == 'S') i2c_dataset1->gps_loc.lat = -i2c_dataset1->gps_loc.lat;
                      case 3: if (string[0] == 'S') GPS_read[LAT] = -GPS_read[LAT];
                              break;
                      //case 4: i2c_dataset1->gps_loc.lon = GPS_coord_to_degrees(string);
                      case 4: GPS_read[LON] = GPS_coord_to_degrees(string);
                              break;
                      //case 5: if (string[0] == 'W') i2c_dataset1->gps_loc.lon = -i2c_dataset1->gps_loc.lon;
                      case 5: if (string[0] == 'W') GPS_read[LON] = -GPS_read[LON];
                              break;
                      case 6: i2c_dataset1->status.gps2dfix = string[0]  > '0';
                              break;
                      case 7: i2c_dataset1->status.numsats = atoi(string);
                              break;
                      case 9: i2c_dataset1->altitude = atoi(string);
                              break;
                     }
                   break;         
                   //************* GPGSA FRAME parsing
                   case GPGSA_FRAME:
                     switch (param)
                     {
                      case 2: i2c_dataset1->status.gps3dfix = string[0] == '3';
                      break;
                     }
                   break;
                   //************* GPGSA FRAME parsing
                   case GPRMC_FRAME:
                     switch(param)
                     {
                       case 7: i2c_dataset1->ground_speed = (atof(string)*0.5144444)*10;      //convert to m/s*100
                               break; 
	               case 8: i2c_dataset1->ground_course = (atof(string)*10);				//Convert to degrees *10 (.1 precision)
							   break;
                     }
                   
                   break;                   
                }
            
                param++; offset = 0;
                if (c == '*') checksum_param=1;
                else parity ^= c;
                break;
     case '\r':
     case '\n':  
                if (checksum_param) { //parity checksum
                  uint8_t checksum = 16 * ((string[0]>='A') ? string[0] - 'A'+10: string[0] - '0') + ((string[1]>='A') ? string[1] - 'A'+10: string[1]-'0');
                  if (checksum == parity) frameOK = 1;
                }
                checksum_param=0;
                break;
     default:
             if (offset < 15) string[offset++] = c;
             if (!checksum_param) parity ^= c;
             
  }
  return frameOK && (gps_frame == GPGGA_FRAME);
}
#endif 

#if defined(UBLOX)

struct ubx_header {
  uint8_t       preamble1;
  uint8_t       preamble2;
  uint8_t       msg_class;
  uint8_t       msg_id;
  uint16_t      length;
};

struct ubx_nav_timeutc {
  uint32_t	itow;
  uint32_t	tacc;
  int32_t	nano;
  datetime_t    datetime;
  uint8_t       valid;
};

struct ubx_nav_posllh {
  uint32_t	time;				// GPS msToW
  int32_t	longitude;
  int32_t	latitude;
  int32_t	altitude_ellipsoid;
  int32_t	altitude_msl;
  uint32_t	horizontal_accuracy;
  uint32_t	vertical_accuracy;
};

struct ubx_nav_status {
  uint32_t	time;				// GPS msToW
  uint8_t	fix_type;
  uint8_t	fix_status;
  uint8_t	differential_status;
  uint8_t	res;
  uint32_t	time_to_first_fix;
  uint32_t	uptime;				// milliseconds
};

struct ubx_nav_solution {
  uint32_t	time;
  int32_t	time_nsec;
  int16_t	week;
  uint8_t	fix_type;
  uint8_t	fix_status;
  int32_t	ecef_x;
  int32_t	ecef_y;
  int32_t	ecef_z;
  uint32_t	position_accuracy_3d;
  int32_t	ecef_x_velocity;
  int32_t	ecef_y_velocity;
  int32_t	ecef_z_velocity;
  uint32_t	speed_accuracy;
  uint16_t	position_DOP;
  uint8_t	res;
  uint8_t	satellites;
  uint32_t	res2;
};

struct ubx_nav_velned {
  uint32_t	time;				// GPS msToW
  int32_t	ned_north;
  int32_t	ned_east;
  int32_t	ned_down;
  uint32_t	speed_3d;
  uint32_t	speed_2d;
  int32_t	heading_2d;
  uint32_t	speed_accuracy;
  uint32_t	heading_accuracy;
};

enum ubs_protocol_bytes {
  PREAMBLE1    = 0xb5,
  PREAMBLE2    = 0x62,
  CLASS_NAV    = 0x01,
  CLASS_ACK    = 0x05,
  CLASS_CFG    = 0x06,
  MSG_ACK_NACK = 0x00,
  MSG_ACK_ACK  = 0x01,
  MSG_TIMEUTC  = 0x21,
  MSG_POSLLH   = 0x2,
  MSG_STATUS   = 0x3,
  MSG_SOL      = 0x6,
  MSG_VELNED   = 0x12,
  MSG_CFG_PRT  = 0x00,
  MSG_CFG_RATE = 0x08,
  MSG_CFG_SET_RATE = 0x01,
  MSG_CFG_NAV_SETTINGS = 0x24
};

enum ubs_nav_fix_type {
  FIX_NONE = 0,
  FIX_DEAD_RECKONING = 1,
  FIX_2D = 2,
  FIX_3D = 3,
  FIX_GPS_DEAD_RECKONING = 4,
  FIX_TIME = 5
};
enum ubx_nav_status_bits {
  NAV_STATUS_FIX_VALID = 1
};

// Packet checksum accumulators
static uint8_t		_ck_a;
static uint8_t		_ck_b;

// State machine state
static uint8_t		_step=0;
static uint8_t		_msg_id;
static uint16_t	_payload_length;
static uint16_t	_payload_counter;

static bool        next_fix;

static uint8_t     _class;

// do we have new position information?
static bool		_new_position=false;

// do we have new speed information?
static bool		_new_speed=false;


// Receive buffer
static union {
  ubx_nav_posllh		posllh;
  ubx_nav_timeutc         timeutc;
  ubx_nav_status		status;
  ubx_nav_solution	solution;
  ubx_nav_velned		velned;
  uint8_t	bytes[1];                   // this size have to be fixed !
} _buffer;



/*
void _update_checksum(uint8_t *data, uint8_t len, uint8_t &ck_a, uint8_t &ck_b)
{
	while (len--) {
		ck_a += *data;
		ck_b += ck_a;
		data++;
	}
}
*/

bool UBLOX_parse_gps(void)
{
  //IF_SDEBUG(dbgSerial.print(F("msg_id: "));dbgSerial.println(_msg_id,HEX));

  // we can use _class too
  
  if (_class == CLASS_NAV) {
    
    switch (_msg_id) {

    case MSG_TIMEUTC:
      i2c_dataset1->datetime=_buffer.timeutc.datetime;
      IF_SDEBUG(dbgSerial.print(F("seconds:"));dbgSerial.println(i2c_dataset1->datetime.sec));
      break;
      
    case MSG_POSLLH:
      
      i2c_dataset1->time	              = _buffer.posllh.time;
      i2c_dataset1->gps_loc.lat         = _buffer.posllh.latitude;
      i2c_dataset1->gps_loc.lon         = _buffer.posllh.longitude;
      
      i2c_dataset1->altitude  	      = _buffer.posllh.altitude_msl / 10 /100;      //alt in m
      i2c_dataset1->status.gps3dfix     = next_fix;
      _new_position = true;
      break;

    case MSG_STATUS:
      next_fix	= (_buffer.status.fix_status & NAV_STATUS_FIX_VALID) && (_buffer.status.fix_type == FIX_3D);
      if (!next_fix) i2c_dataset1->status.gps3dfix = false;
      break;

    case MSG_SOL:
      next_fix	= (_buffer.solution.fix_status & NAV_STATUS_FIX_VALID) && (_buffer.solution.fix_type == FIX_3D);
      if (!next_fix) i2c_dataset1->status.gps3dfix = false;
      i2c_dataset1->status.numsats	= _buffer.solution.satellites;
      break;

    case MSG_VELNED:
      //speed_3d	= _buffer.velned.speed_3d;				// cm/s
      i2c_dataset1->ground_speed = _buffer.velned.speed_2d;				// cm/s
      i2c_dataset1->ground_course = (uint16_t)(_buffer.velned.heading_2d / 10000);	// Heading 2D deg * 100000 rescaled to deg * 10
      _new_speed = true;
      break;
    default:
      return false;
    }

    // we only return true when we get new position and speed data
    // this ensures we don't use stale data
    if (_new_position && _new_speed) {
      _new_speed = _new_position = false;
      return true;
    }
    return false;
  }
  return false;
}


bool GPS_UBLOX_newFrame(uint8_t data)
{
  //IF_SDEBUG(dbgSerial.print(data,HEX));

  bool parsed = false;
  
  switch(_step) {
    
  case 1:
    if (PREAMBLE2 == data) {
      _step++;
      break;
    }
    _step = 0;
  case 0:
    if(PREAMBLE1 == data) _step++;
    break;
    
  case 2:
    _step++;
    _class = data;
    _ck_b = _ck_a = data;			// reset the checksum accumulators
    break;
  case 3:
    _step++;
    _ck_b += (_ck_a += data);			// checksum byte
    _msg_id = data;
    break;
  case 4:
    _step++;
    _ck_b += (_ck_a += data);			// checksum byte
    _payload_length = data;				// payload length low byte
    break;
  case 5:
    _step++;
    _ck_b += (_ck_a += data);			// checksum byte
    
    _payload_length += (uint16_t)(data<<8);
    if (_payload_length > 512) {
      _payload_length = 0;
      _step = 0;
    }
    _payload_counter = 0;				// prepare to receive payload
    break;
  case 6:
    _ck_b += (_ck_a += data);			// checksum byte
    if (_payload_counter < sizeof(_buffer)) {
      _buffer.bytes[_payload_counter] = data;
    }
    if (++_payload_counter == _payload_length)
      _step++;
    break;
  case 7:
    _step++;
    if (_ck_a != data){				// bad checksum
      IF_SDEBUG(dbgSerial.println(F("bad crc A")));
      _step = 0;
    }
    break;
  case 8:
    _step = 0;
    if (_ck_b != data) { 			// bad checksum
      IF_SDEBUG(dbgSerial.println(F("bad crc B")));
      break;
    }
    if (UBLOX_parse_gps())  { parsed = true; }
    //IF_SDEBUG(dbgSerial.print(F("parsed:")));
    //IF_SDEBUG(dbgSerial.println(parsed));
  } //end switch

  return parsed;
}


#endif //UBLOX

#if defined(MTK_BINARY16) || defined(MTK_BINARY19)
    struct diyd_mtk_msg {
        int32_t		latitude;
        int32_t		longitude;
        int32_t		altitude;
        int32_t		ground_speed;
        int32_t		ground_course;
        uint8_t		satellites;
        uint8_t		fix_type;
        uint32_t	utc_date;
        uint32_t	utc_time;
        uint16_t	hdop;
    };
// #pragma pack(pop)
    enum diyd_mtk_fix_type {
       FIX_NONE = 1,
	   FIX_2D = 2,
	   FIX_3D = 3,
	   FIX_2D_SBAS = 6,
	   FIX_3D_SBAS = 7 
    };

#if defined(MTK_BINARY16)
    enum diyd_mtk_protocol_bytes {
        PREAMBLE1 = 0xd0,
        PREAMBLE2 = 0xdd,
    };
#endif

#if defined(MTK_BINARY19)
    enum diyd_mtk_protocol_bytes {
        PREAMBLE1 = 0xd1,
        PREAMBLE2 = 0xdd,
    };
#endif

// Packet checksum accumulators
uint8_t 	_ck_a;
uint8_t 	_ck_b;

// State machine state
uint8_t 	_step;
uint8_t		_payload_counter;

// Time from UNIX Epoch offset
long		_time_offset;
bool		_offset_calculated;

// Receive buffer
union {
  diyd_mtk_msg	msg;
  uint8_t			bytes[1];                    // this size have to be fixed !
} _buffer;

inline long _swapl(const void *bytes)
{
    const uint8_t	*b = (const uint8_t *)bytes;
    union {
        long	v;
        uint8_t b[4];
    } u;

    u.b[0] = b[3];
    u.b[1] = b[2];
    u.b[2] = b[1];
    u.b[3] = b[0];

    return(u.v);
}

bool GPS_MTK_newFrame(uint8_t data)
{
       bool parsed = false;

restart:
        switch(_step) {

            // Message preamble, class, ID detection
            //
            // If we fail to match any of the expected bytes, we
            // reset the state machine and re-consider the failed
            // byte as the first byte of the preamble.  This
            // improves our chances of recovering from a mismatch
            // and makes it less likely that we will be fooled by
            // the preamble appearing as data in some other message.
            //
        case 0:
            if(PREAMBLE1 == data)
                _step++;
            break;
        case 1:
            if (PREAMBLE2 == data) {
                _step++;
                break;
            }
            _step = 0;
            goto restart;
        case 2:
            if (sizeof(_buffer) == data) {
                _step++;
                _ck_b = _ck_a = data;				// reset the checksum accumulators
                _payload_counter = 0;
            } else {
                _step = 0;							// reset and wait for a message of the right class
                goto restart;
            }
            break;

            // Receive message data
            //
        case 3:
            _buffer.bytes[_payload_counter++] = data;
            _ck_b += (_ck_a += data);
            if (_payload_counter == sizeof(_buffer))
                _step++;
            break;

            // Checksum and message processing
            //
        case 4:
            _step++;
            if (_ck_a != data) {
                _step = 0;
            }
            break;
        case 5:
            _step = 0;
            if (_ck_b != data) {
                break;
            }

            i2c_dataset1->status.gps3dfix 			= ((_buffer.msg.fix_type == FIX_3D) || (_buffer.msg.fix_type == FIX_3D_SBAS));
#if defined(MTK_BINARY16)
            GPS_read[LAT]		= _buffer.msg.latitude * 10;	// XXX doc says *10e7 but device says otherwise
            GPS_read[LON]		= _buffer.msg.longitude * 10;	// XXX doc says *10e7 but device says otherwise
#endif
#if defined(MTK_BINARY19)
            GPS_read[LAT]		= _buffer.msg.latitude;	// XXX doc says *10e7 but device says otherwise
            GPS_read[LON]		= _buffer.msg.longitude;	// XXX doc says *10e7 but device says otherwise
#endif
			i2c_dataset1->altitude		= _buffer.msg.altitude /100;
            i2c_dataset1->ground_speed	                = _buffer.msg.ground_speed;
            i2c_dataset1->ground_course	        = _buffer.msg.ground_course;
            i2c_dataset1->status.numsats		        = _buffer.msg.satellites;
            //GPS_hdop			= _buffer.msg.hdop;
            parsed = true;
            //GPS_Present = 1;
        }
    return parsed;
}
#endif //MTK



//////////////////////////////////////////////////////////////////////////////////////
// I2C handlers
// Handler for requesting data
//
void requestEvent()
{
  Wire.write(((uint8_t *)i2c_dataset2)+receivedCommands[0],32);
  //Write up to 32 byte, since master is responsible for reading and sending NACK
  //32 byte limit is in the Wire library, we have to live with it unless writing our own wire library

  //Wire.write((uint8_t *)&i2c_dataset+4,32);
}

//Handler for receiving data
void receiveEvent(int bytesReceived)
{
     uint8_t  *ptr;
     for (int a = 0; a < bytesReceived; a++) {
          if (a < MAX_SENT_BYTES) {
               receivedCommands[a] = Wire.read();
          } else {
               Wire.read();  // if we receive more data then allowed just throw it away
          }
     }

     if (bytesReceived == 2){
       if (receivedCommands[0] == I2C_GPS_COMMAND) { new_command = receivedCommands[1]; return; }  //Just one byte, ignore all others
     }

     if (bytesReceived == 1){
       //read address for a given register
       //Addressing over the reg_map fallback to first byte
       if(bytesReceived == 1 && ( (receivedCommands[0] < 0) || (receivedCommands[0] >= REG_MAP_SIZE))) {
	 receivedCommands[0]=0;
       }
       return;
     }
     //More than 1 byte was received, so there is definitely some data to write into a register
     //Check for writeable registers and discard data is it's not writeable
     // TODO not atomic!
     if ((receivedCommands[0]>=I2C_GPS_MAP_WRITABLE) && (receivedCommands[0]<REG_MAP_SIZE)) {    //Writeable registers
       ptr = (uint8_t *)i2c_writabledataset+receivedCommands[0];
       for (int a = 1; a < bytesReceived; a++) { *ptr++ = receivedCommands[a]; }
     }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GPS initialisation
//

uint32_t init_speed[5] = {9600,19200,38400,57600,115200};

#if defined(UBLOX)
   const uint8_t UBLOX_INIT[] PROGMEM = {                          // PROGMEM array must be outside any function !!!

     // disable all default NMEA messages (rate=0)
    0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x05, 0x00, 0xFF, 0x19,                   // GxVTG  
    0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x03, 0x00, 0xFD, 0x15,		        // GxGSV
    0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x01, 0x00, 0xFB, 0x11,		        // GxGLL
    0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x00, 0x00, 0xFA, 0x0F,		        // GxGGA
    0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x02, 0x00, 0xFC, 0x13,		        // GXGSA
    0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0xF0, 0x04, 0x00, 0xFE, 0x17,		        // GxRMC

    // enable required messages (rate=1)
    0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0x01, 0x21, 0x01, 0x2D, 0x85,                  //set TIMEUTC MSG rate
    0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0x01, 0x02, 0x01, 0x0E, 0x47,                  //set POSLLH MSG rate
    0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0x01, 0x03, 0x01, 0x0F, 0x49,                  //set STATUS MSG rate
    0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0x01, 0x06, 0x01, 0x12, 0x4F,                  //set SOL MSG rate
    0xB5, 0x62, 0x06, 0x01, 0x03, 0x00, 0x01, 0x12, 0x01, 0x1E, 0x67,                  //set VELNED MSG rate

    // optional configuration
    //0xB5, 0x62, 0x06, 0x16, 0x08, 0x00, 0x03, 0x07, 0x03, 0x00, 0x51, 0x08, 0x00, 0x00, 0x8A, 0x41, //set WAAS to EGNOS       //0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xC8, 0x00, 0x01, 0x00, 0x01, 0x00, 0xDE, 0x6A //set rate to 5Hz

};
#endif

  void GPS_SerialInit() {

    IF_SDEBUG(dbgSerial.println(F("start GPS serial init")));
  
  gpsSerial.begin(GPS_SERIAL_SPEED);  
  delay(1000);
  
#if defined(UBLOX)
	//Set speed

      for(uint8_t i=0;i<5;i++){
	IF_SDEBUG(dbgSerial.print(F("set speed: ")));
	IF_SDEBUG(dbgSerial.println(init_speed[i]));
        gpsSerial.begin(init_speed[i]);          // switch UART speed for sending SET BAUDRATE command (NMEA mode)

        #if (GPS_SERIAL_SPEED==19200)
          gpsSerial.print(F("$PUBX,41,1,0003,0001,19200,0*23\r\n"));     // 19200 baud - minimal speed for 5Hz update rate
	  IF_SDEBUG(dbgSerial.println(F("$PUBX,41,1,0003,0001,19200,0*23\r\n")));
        #endif  
        #if (GPS_SERIAL_SPEED==38400)
          gpsSerial.print(F("$PUBX,41,1,0003,0001,38400,0*26\r\n"));     // 38400 baud
	  IF_SDEBUG(dbgSerial.println(F("$PUBX,41,1,0003,0001,38400,0*26\r\n")));
        #endif  
        #if (GPS_SERIAL_SPEED==57600)
          gpsSerial.print(F("$PUBX,41,1,0003,0001,57600,0*2D\r\n"));     // 57600 baud
	  IF_SDEBUG(dbgSerial.println(F("$PUBX,41,1,0003,0001,57600,0*2D\r\n")));
        #endif  
        #if (GPS_SERIAL_SPEED==115200)
          gpsSerial.print(F("$PUBX,41,1,0003,0001,115200,0*1E\r\n"));    // 115200 baud
	  IF_SDEBUG(dbgSerial.println(F("$PUBX,41,1,0003,0001,115200,0*1E\r\n")));
        #endif  
        delay(300);		//Wait for init 
      }
      delay(200);


      IF_SDEBUG(dbgSerial.println(F("start UBLOX init")));

      gpsSerial.begin(GPS_SERIAL_SPEED);  
      for(uint8_t i=0; i<sizeof(UBLOX_INIT); i++) {                        // send configuration data in UBX protocol

	if (pgm_read_byte(UBLOX_INIT+i) == 0xB5 ) {IF_SDEBUG(dbgSerial.print(F("\r\n")));} 
	IF_SDEBUG(dbgSerial.print(pgm_read_byte(UBLOX_INIT+i),HEX));
	IF_SDEBUG(dbgSerial.print(F("-")));

        gpsSerial.write(pgm_read_byte(UBLOX_INIT+i));
        delay(10); //simulating a 38400baud pace (or less), otherwise commands are not accepted by the device.
	//I found this delay essential for the V1 CN-06 GPS (The one without EEPROM)
	//Also essential is the bridging of pins 13- and 14 on the NEO-6M module 

      }
      IF_SDEBUG(dbgSerial.println(F("\r\nend UBLOX init")));


#elif defined(INIT_MTK_GPS)                            // MTK GPS setup
      for(uint8_t i=0;i<5;i++){
        gpsSerial.begin(init_speed[i]);					   // switch UART speed for sending SET BAUDRATE command
        #if (GPS_SERIAL_SPEED==19200)
          gpsSerial.write("$PMTK251,19200*22\r\n");     // 19200 baud - minimal speed for 5Hz update rate
        #endif  
        #if (GPS_SERIAL_SPEED==38400)
          gpsSerial.write("$PMTK251,38400*27\r\n");     // 38400 baud
        #endif  
        #if (GPS_SERIAL_SPEED==57600)
          gpsSerial.write("$PMTK251,57600*2C\r\n");     // 57600 baud
        #endif  
        #if (GPS_SERIAL_SPEED==115200)
          gpsSerial.write("$PMTK251,115200*1F\r\n");    // 115200 baud
        #endif  
        delay(300);
      }
      // at this point we have GPS working at selected (via #define GPS_BAUD) baudrate
      gpsSerial.begin(GPS_SERIAL_SPEED);

	  gpsSerial.write(MTK_NAVTHRES_OFF);
	  delay(100);
      gpsSerial.write(SBAS_ON);
	  delay(100);
      gpsSerial.write(WAAS_ON);
	  delay(100);
      gpsSerial.write(SBAS_TEST_MODE);
	  delay(100);
      gpsSerial.write(MTK_OUTPUT_5HZ);           // 5 Hz update rate
	  delay(100);

      #if defined(NMEA)
        gpsSerial.write(MTK_SET_NMEA_SENTENCES); // only GGA and RMC sentence
      #endif     
      #if defined(MTK_BINARY19) || defined(MTK_BINARY16)
		gpsSerial.write(MTK_SET_BINARY);
      #endif



#endif
		IF_SDEBUG(dbgSerial.println(F("end GPS serial init")));
  }


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup
//
void setup() {

#ifdef ARDUINO_ARCH_AVR  
  /*
  Nel caso di un chip in standalone senza bootloader, la prima
  istruzione che è bene mettere nel setup() è sempre la disattivazione
  del Watchdog stesso: il Watchdog, infatti, resta attivo dopo il
  reset e, se non disabilitato, esso può provare il reset perpetuo del
  microcontrollore
  */
  wdt_disable();

  // enable watchdog with timeout to 8s
  wdt_enable(WDTO_8S);
#endif

#ifdef DEBUGONSERIAL
  dbgSerial.begin(9600);
  delay(1000);
  dbgSerial.println(F("sync"));
  delay(1000);
  dbgSerial.println(F("I2C_GPS_NAV_v2_2 started"));
#endif

  // inizialize double buffer
  i2c_dataset1=&i2c_buffer1;
  i2c_dataset2=&i2c_buffer2;

  //Init GPS
  GPS_SerialInit();

  uint8_t *ptr;
  //Init to 0 i2c_dataset1;
  ptr = (uint8_t *)i2c_dataset1;
  for (uint8_t i=0;i<REG_MAP_SIZE;i++) { *ptr = 0; ptr++;}

  //Init to 0 i2c_dataset1;
  ptr = (uint8_t *)i2c_dataset2;
  for (uint8_t i=0;i<REG_MAP_SIZE;i++) { *ptr = 0; ptr++;}

  IF_SDEBUG(dbgSerial.println(F("i2c_dataset set to zero")));

  //Set up default parameters
  i2c_dataset1->status.sw_version          = VERSION;

  //Start I2C communication routines
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);          // Set up event handlers
  Wire.onReceive(receiveEvent);

  IF_SDEBUG(dbgSerial.println(F("end setup")));

}

/******************************************************************************************************************/
/******************************************************* Main loop ************************************************/
/******************************************************************************************************************/
void loop() {

  static uint8_t _command;
  static uint32_t _watchdog_timer = 0;

  while (gpsSerial.available()) {

#if defined(NMEA)
    if (GPS_NMEA_newFrame(gpsSerial.read())) {
#endif 
#if defined(UBLOX)
    if (GPS_UBLOX_newFrame(gpsSerial.read())) {
#endif
#if defined(MTK_BINARY16) || defined(MTK_BINARY19)
    if (GPS_MTK_newFrame(gpsSerial.read())) {
#endif
 
#ifdef ARDUINO_ARCH_AVR
  wdt_reset();
#endif
 
      // disable interrupts for atomic operation
      noInterrupts();
      //exchange double buffer
      i2c_datasettmp=i2c_dataset1;
      i2c_dataset1=i2c_dataset2;
      i2c_dataset2=i2c_datasettmp;
      interrupts();
      // new data published

      /*      
	      byte val=gpsSerial.read();
	
	      if (val == 0xB5 ) {IF_SDEBUG(dbgSerial.print(F("\r\n")));} 
	      IF_SDEBUG(dbgSerial.print(val,HEX));
	      IF_SDEBUG(dbgSerial.print(F("-")));
	      }
      */
      /*
	IF_SDEBUG(dbgSerial.println(i2c_dataset1->gps_loc.lat));
	IF_SDEBUG(dbgSerial.println(i2c_dataset1->gps_loc.lon));
	
	IF_SDEBUG(dbgSerial.print(i2c_dataset1->datetime.year));
	IF_SDEBUG(dbgSerial.print(i2c_dataset1->datetime.month));
	IF_SDEBUG(dbgSerial.print(i2c_dataset1->datetime.day));
	IF_SDEBUG(dbgSerial.print(i2c_dataset1->datetime.hour));
	IF_SDEBUG(dbgSerial.print(i2c_dataset1->datetime.min));
	IF_SDEBUG(dbgSerial.println(i2c_dataset1->datetime.sec));
	
      */


    }
  }


  //check watchdog timer, after 1200ms without valid packet, assume that gps communication is lost.
  if (_watchdog_timer != 0){
    
    if (_watchdog_timer+1200 < millis()) {
      i2c_dataset1->status.gps2dfix = 0;
      i2c_dataset1->status.gps3dfix = 0;
      i2c_dataset1->status.numsats = 0;
      i2c_dataset1->gps_loc.lat = 0;
      i2c_dataset1->gps_loc.lon = 0;
      
      i2c_dataset1->datetime.year = 0;
      i2c_dataset1->datetime.month = 0;
      i2c_dataset1->datetime.day = 0;
      i2c_dataset1->datetime.hour = 0;
      i2c_dataset1->datetime.min = 0;
      i2c_dataset1->datetime.sec = 0;
      
      i2c_dataset1->gps_loc.lat = 0;
      i2c_dataset1->gps_loc.lon = 0;
	
      _watchdog_timer = 0;
      
    }
  }

  //Check for new incoming command on I2C
  if (new_command!=0) {
    _command = new_command;                                                   //save command byte for processing
    new_command = 0;                                                          //clear it
    //_command = _command & 0x0F;                                               //empty 4MSB bits   
    switch (_command) {
    case I2C_GPS_COMMAND_TEST1:
      break;         
    case I2C_GPS_COMMAND_TEST2:
      break;          
    case I2C_GPS_COMMAND_TEST3:
      break;
    } //switch  
  }
}
