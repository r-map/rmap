/**
 * @author Ozk
 * @email hamza@hamzaozkan.com.tr
 * @create date 2022-06-21 22:42:40
 * @modify date 2022-06-21 22:42:40
 * @copydetails free licance
 * @desc Look at the nmea description
 *  @link https://www.sparkfun.com/datasheets/GPS/NMEA%20Reference%20Manual-Rev2.1-Dec07.pdf @endlink
 *  @link https://www.sparkfun.com/datasheets/GPS/Modules/D2523T%20V1.pdf @endlink
 * 
 * @copyright This library was written by @ozk, 
 * there is no harm in copying, distributing, changing it. 
 * See the description for the library's working guarantee and make sure you've done everything right. 
 * Any damage or liability arising from the use of this library belongs to the user.
 */
#ifndef GPS_H_
#define GPS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h> 
#include <Arduino.h>

#ifndef GPS_DEBUG
#define DEBUG_GPS 0
#else
#define DEBUG_GPS GPS_DEBUG
#endif
#define DEBUG_GPS_TEXT (false)

//#ifndef millis
//#define millis() ((uint32_t)(1))
//#endif
#define c2int(c) ((uint8_t)(c-'0'))

#define _GPS_MPH_PER_KNOT 1.15077945
#define _GPS_MPS_PER_KNOT 0.51444444
#define _GPS_KMPH_PER_KNOT 1.852
#define _GPS_MILES_PER_METER 0.00062137112
#define _GPS_KM_PER_METER 0.001
#define _GPS_FEET_PER_METER 3.2808399
#define _GPS_MAX_FIELD_SIZE 15

#define STC_PARSE_ERROR 136 
#define STC_ARRAY_SIZE 12
#define TTERM_ARR_SIZE 20

//S id
enum minmea_sentence_id {
    MINMEA_SENTENCE_TXT,
    MINMEA_SENTENCE_RMC,
    MINMEA_SENTENCE_GGA,
    MINMEA_SENTENCE_GLL,
    MINMEA_SENTENCE_VTG,  
    MINMEA_SENTENCE_GSA,
    MINMEA_SENTENCE_GSV,
    MINMEA_SENTENCE_MSS,
    MINMEA_SENTENCE_TRF,
    MINMEA_SENTENCE_STN,
    MINMEA_SENTENCE_XTE,
    MINMEA_SENTENCE_ZDA,
};

//parsing variable

struct minmea_time {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t microseconds;
};

typedef struct datetime_ {
    uint8_t msec;
    uint8_t sec;
    uint8_t min;
    uint8_t hours;
    uint8_t day;
    uint8_t mon;
    uint8_t year;
} datetime;

enum minmea_ident_text {
    MINMEA_TEXT_IDT_ERROR = 01,
    MINMEA_TEXT_IDT_WARNING = 02,
    MINMEA_TEXT_IDT_NOTICE = 03,
    MINMEA_TEXT_IDT_USER = 07
};

struct minmea_coordinate
{
    float latitude;
    char latdirect;
    float longitude;
    char longdirect;
};

struct minmea_sentence_txt
{
    uint8_t msgid;
    uint8_t msgrange;
    enum minmea_ident_text ident;
    char* msg;
};

struct minmea_sentence_rmc {
    struct minmea_coordinate dms;
    datetime time;
    uint8_t valid;
    float speed;
    float course;
    uint8_t variation;
    char vardirect;
};

struct minmea_sentence_gga {
    struct minmea_coordinate dms;
    struct minmea_time time;
    uint8_t fix_quality;
    uint8_t satellites_tracked;
    float hdop;
    float altitude; char altitude_units;
    float height; char height_units;
    float dgps_age;
};

struct minmea_sentence_gll {
    struct minmea_coordinate dms;
    struct minmea_time time;
    char status;
    char mode;
};

struct minmea_sentence_gsa {
    char mode;
    uint8_t fix_type;
    uint8_t sats[12];
    float pdop;
    float hdop;
    float vdop;
};

struct minmea_sat_info {
    uint8_t id;
    uint8_t elevation;
    uint8_t azimuth;
    uint8_t snr;
};

struct minmea_sentence_gsv {
    uint8_t total_msgs;
    uint8_t msg_nr;
    uint8_t total_sats;
    struct minmea_sat_info sats[4];
};

enum minmea_faa_mode {
    MINMEA_FAA_MODE_NOT_VALID = 'N',
    MINMEA_FAA_MODE_AUTONOMOUS = 'A',
    MINMEA_FAA_MODE_DIFFERENTIAL = 'D',
    MINMEA_FAA_MODE_ESTIMATED = 'E',
    MINMEA_FAA_MODE_MANUAL = 'M',
    MINMEA_FAA_MODE_SIMULATED = 'S',
    MINMEA_FAA_MODE_PRECISE = 'P',
};

struct minmea_sentence_vtg {
    float true_track_degrees;
    float magnetic_track_degrees;
    float speed_knots;
    float speed_kph;
    enum minmea_faa_mode faa_mode;
};

struct minmea_sentence_zda {
    datetime time;
    int hour_offset;
    int minute_offset;
};

struct MGPS
{
    uint32_t emittime;
    uint8_t pps;
    uint8_t perror;
    uint8_t tmode;
    struct minmea_sentence_txt txt;
    struct minmea_sentence_rmc rmc;
    struct minmea_sentence_gga gga;
    struct minmea_sentence_gll gll;
    struct minmea_sentence_vtg vtg;
    struct minmea_sentence_gsa gsa;
    struct minmea_sentence_gsv gsv[4];
    //struct minmea_sentence_mss mss;
    //struct minmea_sentence_stn stn;
    //struct minmea_sentence_trf trf;
    //struct minmea_sentence_xte xte;
    struct minmea_sentence_zda zda;
};

/*
custom type <NOT USED>
typedef struct GPS_ {
    uint8_t 	satSize;
    uint32_t  	timeOfWeekmS;
    uint16_t	   utcYear;
    uint8_t		utcMonth;
    uint8_t		utcDay;
    uint8_t		utcHour;
    uint8_t		utcMinute;
    uint8_t		utcSecond;
    uint8_t		validity;
    uint32_t	   timeAccuracyNs;
    int32_t		nanoSeconds;
    uint8_t		fixType;
    uint8_t		flags1;
    uint8_t		flags2;
    uint8_t		numSV;
    int32_t 	   lonDeg7;
    int32_t 	   latDeg7;
    int32_t		heightEllipsoidmm;
    int32_t 	   heightMSLmm;
    uint32_t	   horzAccuracymm;
    uint32_t	   vertAccuracymm;
    int32_t		velNorthmmps;
    int32_t		velEastmmps;
    int32_t		velDownmmps;
    int32_t		groundSpeedmmps;
    int32_t		headingMotionDeg5;
    uint32_t	   speedAccuracymmps;
    uint32_t	   headingAccuracyDeg5;
    uint16_t	   posDOP;
    uint8_t		reserved;
    int32_t		headingDeg5;
    int16_t		magneticDeclinationDeg2;
    uint16_t	   declinationAccuracyDeg2;
    uint8_t	   ckA;
    uint8_t     ckB;
} NGPS;*/


class OZGPS{
    
    const char* stctype[STC_ARRAY_SIZE] = {"TXT","RMC","GGA","GLL","VTG","GSA","GSV","MSS","TRF","STN","XTE","ZDA"};
    uint16_t filter_stc = 0xfff; //WARNING!(no limit 0xfff or 0) filter_Array in stc ~[1 1 1 0 0 1 0 0 0 0 0]

    // parsing state variables
    uint8_t parity;
    uint8_t is_checksum;

    //parse variable
    static char nimea_terms[30][20];
    static char term[20];//term
    uint8_t term_count = 0;
    uint8_t term_offset = 0;
    uint8_t nimea_terms_count = 0;

    //sentence variable
    int8_t sentence_type;
    uint8_t sentence_finded;
    uint8_t sentence_has_fix;
    int8_t checksum;

  public:
    //start init variable
    struct MGPS *p_mgps;
    
    //status
    uint8_t valid;
    uint8_t is_init = 0;
    int8_t error_flag = -1;
    const char * error; 

    // statistics
    uint32_t encoed_char_count;
    uint32_t sentence_fix_count;
    uint32_t fail_checksum_count;
    uint32_t pass_checksum_count;

    //bool run(struct MGPS *gps_data);
    //uint8_t set_baud(uint16_t baudrate);
    int8_t init(struct MGPS *mpgs);
    int8_t set_filter(uint16_t filter);
    uint8_t encode(char c);
    
    static inline bool minmea_isfield(char c);

  private:
    int8_t sentence_check(void);
    uint8_t sentence_parse_run(uint8_t type);
    uint8_t nmea_parser(void);

    int fromHex(char a);
    uint8_t convert_to_deg(const char *tterm, float *deg, char dir);
    uint8_t convert_speed(float *var, char unit, float speed);

    uint8_t get_integer(const char *tterm, long *fint);
    uint8_t get_float(const char *tterm, float *flnum);
    uint8_t get_coordinate(const char *tterm, float *dms, char dir);
    uint8_t get_datetime(const char *tterm, uint8_t *v0, uint8_t *v1, uint8_t *v2, uint8_t *v3);
    
    void set_txtident(long ptr);

    int8_t set_error(int8_t flag, const char *err);

};

#endif
