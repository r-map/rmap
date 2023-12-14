/**
 * @author Ozk
 * @email hamza@hamzaozkan.com.tr
 * @create date 2022-06-21 22:42:40
 * @modify date 2022-06-21 22:42:40
 * @copydetails free licance
 * @desc Look at the nmea description
 *  @link https://www.sparkfun.com/datasheets/GPS/NMEA%20Reference%20Manual-Rev2.1-Dec07.pdf @endlink
 *  @link https://www.sparkfun.com/datasheets/GPS/Modules/D2523T%20V1.pdf @endlink
 *  @link https://gpsd.gitlab.io/gpsd/NMEA.html @endlink
 * 
 * @copyright This library was written by @ozk, 
 * there is no harm in copying, distributing, changing it. 
 * See the description for the library's working guarantee and make sure you've done everything right. 
 * Any damage or liability arising from the use of this library belongs to the user.
 */
#include "ozgps.h"

char OZGPS::nimea_terms[30][20];
char OZGPS::term[20];//term

// The optional checksum is an XOR of all bytes between "$" and "*".
int8_t OZGPS::init(struct MGPS *mpgs){
    p_mgps = mpgs;
    is_init = true;
    return is_init;
}

char buffer[100];

int8_t OZGPS::set_filter(uint16_t filter){
    //**bit2hex(12bit:4096); {"TXT","RMC","GGA","GLL","VTG","GSA","GSV","MSS","TRF","STN","XTE","ZDA"}; 
    //WARNING!(no limit 0xfff or 0)
    filter_stc = filter;
    return 1;
}

int8_t OZGPS::set_error(int8_t flag, const char *err){
    error_flag = flag;
    if(DEBUG_GPS){
        sprintf(buffer, "GPS flag: %d, error: %s\n", flag, err);
        printf(buffer);
    }
    error = buffer;
    return false;
}

int8_t OZGPS::sentence_check(){
    uint8_t i=0;
    while(i < STC_ARRAY_SIZE){
        if(
            stctype[i][0] == term[2] && 
            stctype[i][1] == term[3] && 
            stctype[i][2] == term[4] && 
            (filter_stc>>i & 1) )//sentence filter add
        {
            ++sentence_fix_count;
            sentence_finded = true;// founded sentenced
            return i;
        }
        i++;
    }

    return -1;
}

uint8_t OZGPS::get_integer(const char *tterm, long int *fint){
    uint8_t f = 0, 
            k = c2int(tterm[TTERM_ARR_SIZE-1]); 
    bool ferr = true;
    while(f < k){
         ferr &= isdigit(tterm[f]);
         if(!ferr)break;
         f++;
    }

    *fint = atol(tterm); 
    return ferr ? true : set_error(STC_PARSE_ERROR, "get_float parsing");
}

uint8_t OZGPS::get_float(const char *tterm, float *flnum){
    
    uint8_t f = 0, k = 0; 
    bool ferr = true;

    while(f < c2int(tterm[TTERM_ARR_SIZE-1])){
        ferr &= isdigit(tterm[f]);
        if(!ferr && tterm[f]!='.'){
            break;
        }
        ferr = 1;
        f++;
    }

    if(ferr){
        *flnum = strtof(tterm, NULL);
    }

    return ferr ? true : set_error(STC_PARSE_ERROR, "get_float parsing");
}

int OZGPS::fromHex(char a){
  if (a >= 'A' && a <= 'F')
    return a - 'A' + 10;
  else if (a >= 'a' && a <= 'f')
    return a - 'a' + 10;
  else
    return a - '0';
}

uint8_t OZGPS::convert_to_deg(const char *tterm, float *deg, char dir){

    float pd  = 0.0;

    if(!get_float(tterm, &pd)){
        return false;
    }

	*deg = floor(pd / 100);				
	*deg +=  ((pd - *deg * 100) / 60.0);

	//everything should be N/E, so flip S,W
	if (dir=='S' || dir == 'W'){
		*deg *= -1.0;
	}

	return 1;
}

uint8_t OZGPS::convert_speed(float *var, char unit, float speed){

    if(unit=='N'){
        *var *= _GPS_KMPH_PER_KNOT;
        return 1;
    }

    *var = speed;
    return true;
}

void OZGPS::set_txtident(long ptr){
    if(ptr==MINMEA_TEXT_IDT_ERROR){
        p_mgps->txt.ident = MINMEA_TEXT_IDT_ERROR;
    }else if(ptr==MINMEA_TEXT_IDT_WARNING){
        p_mgps->txt.ident = MINMEA_TEXT_IDT_WARNING;
    }else if(ptr==MINMEA_TEXT_IDT_NOTICE){
        p_mgps->txt.ident = MINMEA_TEXT_IDT_NOTICE;
    }else if(ptr==MINMEA_TEXT_IDT_USER){
        p_mgps->txt.ident = MINMEA_TEXT_IDT_USER;
    }
}

//parse coordinat
uint8_t OZGPS::get_coordinate(const char *tterm, float *dms, char dir){
    //lat: ddmm.mmmm 
    //long: dddmm.mmmm
    bool ferr = convert_to_deg(tterm, dms, dir);
    return ferr ? ferr : set_error(STC_PARSE_ERROR, "get_coordinate parsing");
} 

//parse date
uint8_t OZGPS::get_datetime(const char *tterm, uint8_t *v0, uint8_t *v1, uint8_t *v2, uint8_t *v3){
    //"ddmmyy" | "hhmmss.sss" | "hhmmss"
    uint8_t f = 0, 
        k = c2int(tterm[TTERM_ARR_SIZE-1]), 
        l=100;
    
    while(f < k){
        if (tterm[f] != '.' && !isdigit((unsigned char) tterm[f]))
            return set_error(STC_PARSE_ERROR, "get_datetime parsing");
        f++;
    }
    
    if(tterm[6] == '.' && k>7){
        uint32_t time = 100*(float)atof(tterm);//12320401
        *v0 = time/1000000;//12
        *v1 = (time/10000) % 100;//32
        *v2 = (time /100) % 100;//4
        *v3 = time % 100;//1
    }else{
        uint32_t time = (uint32_t)atol(tterm);//123204
        *v0 = time/10000;//32
        *v1 = (time/100) % 100;
        *v2 = time % 100;
    }

    return true;
}

inline bool OZGPS::minmea_isfield(char c) {
    return isprint((unsigned char) c) && c != ',' && c != '*';
}

/*! 
    @arg type: parse edilecek nmea türünün id'si 
*/
uint8_t OZGPS::sentence_parse_run(uint8_t type){
    bool ferr = 1;

    if(!(filter_stc >> type & 1)){//sentence filter work here
        error_flag = 16;
        return false;
    }

    if(checksum != parity){
        fail_checksum_count++;
        return false;
    } 
    
    error_flag = 0;
    ++pass_checksum_count;
    if(DEBUG_GPS)printf("GPS PASS -->> ok: %ld fail: %ld\n", pass_checksum_count, fail_checksum_count);

    // {"TXT","RMC","GGA","GLL","VTG","GSA","GSV","MSS","TRF","STN","XTE","ZDA"};
    switch (type){
        case MINMEA_SENTENCE_TXT: {
            //$GPTXT,01,01,02,ANTSTATUS=INIT*25
            long ptr;
            ferr &= get_integer(nimea_terms[1],  &ptr);
            p_mgps->txt.msgid = ptr;
            ferr &= get_integer(nimea_terms[2],  &ptr);
            p_mgps->txt.msgrange = ptr;
            ferr &= get_integer(nimea_terms[3],  &ptr);
                set_txtident(ptr);
            p_mgps->txt.msg = (char *)nimea_terms[4];
            p_mgps->emittime = millis();
            valid = true;

        } break;
        case MINMEA_SENTENCE_RMC: {//trust
            //$GPRMC,161229.487,A,3723.2475,N,12158.3416,W,0.13,309.62,120598, ,*10
            p_mgps->rmc.valid = (nimea_terms[2][0]=='A');
            if(p_mgps->rmc.valid){//validate message
                ferr = get_datetime(nimea_terms[1], &p_mgps->rmc.time.hours, &p_mgps->rmc.time.min, &p_mgps->rmc.time.sec, &p_mgps->rmc.time.msec);
                p_mgps->rmc.valid = true;
                p_mgps->rmc.dms.latdirect =  nimea_terms[4][0];
                p_mgps->rmc.dms.longdirect = nimea_terms[6][0];
                ferr &= get_coordinate(nimea_terms[3], &p_mgps->rmc.dms.latitude, p_mgps->rmc.dms.latdirect);
                ferr &= get_coordinate(nimea_terms[5], &p_mgps->rmc.dms.longitude,  p_mgps->rmc.dms.longdirect);
                ferr &= get_float(nimea_terms[7], &p_mgps->rmc.speed);
                ferr &= get_float(nimea_terms[8], &p_mgps->rmc.course);
                ferr &= get_datetime(nimea_terms[9], &p_mgps->rmc.time.day, &p_mgps->rmc.time.mon, &p_mgps->rmc.time.year, &p_mgps->rmc.time.msec);
                
                p_mgps->rmc.variation = nimea_terms[10][0];
                p_mgps->rmc.vardirect = nimea_terms[11][0]; 
                p_mgps->rmc.speed *= _GPS_KMPH_PER_KNOT;//convert speed, knots to km/h 
                p_mgps->emittime = millis();
                //printf("%d",p_mgps->rmc.time.hours) ;    
                valid = true;        
            }
            
        } break;
        case MINMEA_SENTENCE_GGA:{//trust
            //$GPGGA,123204.00,5106.94086,N,01701.51680,E,1,06,3.86,127.9,M,40.5,M,,*51
            p_mgps->gga.fix_quality =  c2int(nimea_terms[6][0]);
            
            if(p_mgps->gga.fix_quality==1){//validate message
                ferr = get_datetime(nimea_terms[1], &p_mgps->gga.time.hours, &p_mgps->gga.time.minutes, &p_mgps->gga.time.seconds, &p_mgps->gga.time.microseconds);
                p_mgps->gga.dms.latdirect =  nimea_terms[3][0];
                p_mgps->gga.dms.longdirect = nimea_terms[5][0];
                ferr &= get_coordinate(nimea_terms[2], &p_mgps->gga.dms.latitude, p_mgps->gga.dms.latdirect);
                ferr &= get_coordinate(nimea_terms[4], &p_mgps->gga.dms.longitude,  p_mgps->gga.dms.longdirect);
                long ptr = p_mgps->gga.satellites_tracked;
                ferr &= get_integer(nimea_terms[7],  &ptr);
                p_mgps->gga.satellites_tracked = ptr;
                ferr &= get_float(nimea_terms[8], &p_mgps->gga.hdop);
                ferr &= get_float(nimea_terms[9], &p_mgps->gga.altitude);
                
                p_mgps->gga.altitude_units = nimea_terms[10][0];

                ferr &= get_float(nimea_terms[11], &p_mgps->gga.height);
                p_mgps->gga.height_units = nimea_terms[12][0];
                p_mgps->gga.dgps_age =  c2int(nimea_terms[13][0]);
                p_mgps->emittime = millis();
                //printf("%d",p_mgps->gga.time.hours) ;
                valid = true;
            }
        } break;
        case MINMEA_SENTENCE_GLL:{//trust
            //$GPGLL,5106.94086,N,01701.51680,E,123204.00,A,A*63
            p_mgps->gll.status = (nimea_terms[6][0]=='A');
            p_mgps->gll.mode = (nimea_terms[7][0]);
            if(p_mgps->gll.status){
                ferr = get_datetime(nimea_terms[5], &p_mgps->gll.time.hours, &p_mgps->gll.time.minutes, &p_mgps->gll.time.seconds, &p_mgps->gll.time.microseconds);
                p_mgps->gll.dms.latdirect =  nimea_terms[2][0];
                p_mgps->gll.dms.longdirect = nimea_terms[4][0];
                ferr &= get_coordinate(nimea_terms[1], &p_mgps->gll.dms.latitude, p_mgps->gll.dms.latdirect);
                ferr &= get_coordinate(nimea_terms[3], &p_mgps->gll.dms.longitude,  p_mgps->gll.dms.longdirect);    
                p_mgps->emittime = millis();   
                valid = true;         
            }
        } break;
        case MINMEA_SENTENCE_VTG:{//trust
            p_mgps->vtg.faa_mode =  (enum minmea_faa_mode)nimea_terms[9][0];    
            if(p_mgps->vtg.faa_mode!='N'){            
                ferr = get_float(nimea_terms[1], &p_mgps->vtg.true_track_degrees);
                ferr &= get_float(nimea_terms[5], &p_mgps->vtg.speed_knots);
                ferr &= get_float(nimea_terms[7], &p_mgps->vtg.speed_kph);
                ferr &= get_float(nimea_terms[3], &p_mgps->vtg.magnetic_track_degrees);
                p_mgps->emittime = millis();       
                valid = true;         
            }

        } break;
        case MINMEA_SENTENCE_GSA:{//trust
            p_mgps->gsa.mode = nimea_terms[1][0];
            p_mgps->gsa.fix_type = c2int(nimea_terms[2][0]);
            if(p_mgps->gsa.fix_type>1){
                for (size_t i = 0; i < 12; i++)
                {
                    p_mgps->gsa.sats[i] =atoi(nimea_terms[3+i]);
                }
                ferr &= get_float(nimea_terms[16], &p_mgps->gsa.pdop);
                ferr &= get_float(nimea_terms[17], &p_mgps->gsa.hdop);
                ferr &= get_float(nimea_terms[18], &p_mgps->gsa.vdop);  
                p_mgps->emittime = millis();     
                valid = true;         
            }


        } break;
        case MINMEA_SENTENCE_GSV:{
            uint8_t msg_num = c2int(nimea_terms[2][0]);
            p_mgps->gsv[msg_num].msg_nr = msg_num;
            p_mgps->gsv[msg_num].total_msgs = c2int(nimea_terms[1][0]);
            p_mgps->gsv[msg_num].total_sats = atoi(nimea_terms[3]);
            long ptr;
            ferr = get_integer(nimea_terms[4], &ptr);
            p_mgps->gsv[msg_num].sats->id = ptr;
            ferr &= get_integer(nimea_terms[5], &ptr);
            p_mgps->gsv[msg_num].sats->elevation = ptr;
            ferr &= get_integer(nimea_terms[6], &ptr); 
            p_mgps->gsv[msg_num].sats->azimuth = ptr;
            ferr &= get_integer(nimea_terms[7], &ptr);
            p_mgps->gsv[msg_num].sats->snr = ptr;
            p_mgps->emittime = millis();
            valid = true;

        } break;
        case MINMEA_SENTENCE_MSS:{
            //$--MSS,55,27,318.0,100,1,*57
            //--MSS,ss,snr,freq,brate,ch
        } break;
        case MINMEA_SENTENCE_TRF:{

        } break;
        case MINMEA_SENTENCE_STN:{
            //$--STN,x.x
            //-STN, id
        } break;
        case MINMEA_SENTENCE_XTE:{
            //$--XTE,A,A,x.x,a,N
            //https://gpsd.gitlab.io/gpsd/NMEA.html
            
        } break;
        case MINMEA_SENTENCE_ZDA:{
            //$GPZDA,181813,14,10,2003,00,00*4F
            //(utc)time,day,mon,year,lzhour,lzmin
        } break;
    }

    p_mgps->perror = !ferr;
    return ferr;
}

uint8_t OZGPS::nmea_parser(){
    ++term_count;
    if(sentence_finded && term_offset > 0){//karakter var.

        for(int i=0;i<term_offset;i++){
            nimea_terms[term_count-1][i] = term[i];
            if(DEBUG_GPS_TEXT){
                if(i<19 && nimea_terms[term_count-1][i]=='G' && nimea_terms[term_count-1][i+1]=='P'){
                    printf("\n");
                }
                printf("%c", nimea_terms[term_count-1][i]);
            }
        }

        nimea_terms[term_count-1][TTERM_ARR_SIZE-1] = term_offset+'0';
        nimea_terms[term_count-1][term_offset] = '?';

        if(DEBUG_GPS_TEXT)printf(",");

        return true;
    }

    return false;
}

uint8_t OZGPS::encode(char c){

    ++encoed_char_count;
    valid = false;
    
    if(!is_init || c == '\r' || c == '\n'){
        return false;
    }   

    if(is_checksum){
        if(checksum ==-2){
            checksum = 16 * fromHex(c);
        }else{
            checksum += fromHex(c);
            is_checksum = false;
            return sentence_parse_run(sentence_type);
        }
        return false;
    }

    if(c == ','){
        nmea_parser();
        parity ^= c;//, dahil et
        term_offset = 0;
    }
    else if(c == '*'){
        nmea_parser();
        //printf(" T:%d \n",term_count);
        term_count = 0;
        checksum = -2;
        is_checksum = true;
        sentence_finded = false;
    }
    else if(c == '$'){
        parity = 0;//sıfırla
        term_offset = 0;
        term_count = 0;
        sentence_finded = false;
    }
    else if(minmea_isfield(c)){

        term[term_offset++] = c;
        parity ^= c;

        if(!sentence_finded && term_offset > 4){ //sentence bul
            sentence_type = sentence_check();
            //term_offset = 0;
            return false;
        }

    }

    return false;
}