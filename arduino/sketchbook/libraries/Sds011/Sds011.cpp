#include "Sds011.h"
#include "config.h"

using namespace sds011;

Sds011::Sds011(Stream &out) : _out(out)
{
  _out.setTimeout(10000);
}

String Sds011::firmware_version(void)
{
    bool ok;

    IF_SDEBUG(Serial.println(F("Sds011 CMD firmware version")));
    _send_cmd(CMD_FIRMWARE, NULL, 0);

    ok = _read_response();
    if (!ok) {
        return "";
    }

    return String(_buf[3])+"_"+String(_buf[4])+"_"+String(_buf[5]);
}

bool Sds011::set_mode(Report_mode mode)
{
    uint8_t data[] = {0x1, mode};
    IF_SDEBUG(Serial.print(F("Sds011 CMD set mode: ")));
    IF_SDEBUG(Serial.println(mode));
    _send_cmd(CMD_MODE, data, 2);

    if (!_read_response()) {
        IF_SDEBUG(Serial.println(F("Sds011 read response failed")));
        return false;
    }

    /*
    byte expected[]= 
    int n=memcmp ( _buff, expected, sizeof(expected) )
    */
    delay(2000);
    return true;
}

bool Sds011::set_sleep(bool sleep)
{
    uint8_t data[] = {0x1, !sleep};
    IF_SDEBUG(Serial.print(F("Sds011 CMD set sleep: ")));    
    IF_SDEBUG(Serial.println(sleep));    
    _send_cmd(CMD_SLEEP, data, 2);
    
    if (!_read_response()) {
        IF_SDEBUG(Serial.println(F("Sds011 read response failed")));
        return false;
    }

    /*
    byte expected[]= 
    int n=memcmp ( _buff, expected, sizeof(expected) )
    */
    if (sleep==sds011::WORK){
      IF_SDEBUG(Serial.println(F("Sds011 wake up: you have to wait for 30s")));
      //delay(30000);
    }else{
      delay(2000);
    }
    return true;

}

bool Sds011::query_data(int *pm25, int *pm10)
{
    bool ok;
    IF_SDEBUG(Serial.println(F("Sds011 CMD query data")));
    _send_cmd(CMD_QUERY_DATA, NULL, 0);

    ok =_read_response();
    if (!ok) {
        IF_SDEBUG(Serial.println(F("Sds011 read response failed")));
        return false;
    }

    *pm25 = _buf[2] | _buf[3]<<8;
    *pm10 = _buf[4] | _buf[5]<<8;

    return true;
}

bool Sds011::query_data_auto(int *pm25, int *pm10, int n)
{
    int pm25_table[n];
    int pm10_table[n];
    int ok;

    IF_SDEBUG(Serial.println(F("Sds011 query data auto")));
    
    for (int i = 0; i<n; i++) {
        ok = query_data(&pm25_table[i], &pm10_table[i]);
        if (!ok){
	  //i--;          // here you can manage a retry
	  return false;   // or fail
        }

	//recommended query interval of not less than 3 seconds
	if (i < (n-1)) delay(3000);
    }

    _filter_data(n, pm25_table, pm10_table, pm25, pm10);

    return true;
}

bool Sds011::crc_ok(void)
{
    uint8_t crc = 0 ;
    for (int i=2; i<8; i++) {
        crc+=_buf[i];
    }
    //IF_SDEBUG(Serial.println(crc==_buf[8]));
    return crc==_buf[8];
}

void Sds011::_send_cmd(enum Command cmd, uint8_t *data, uint8_t len)
{
    uint8_t i, crc;

    _buf[0] = 0xAA;
    _buf[1] = 0xB4;
    _buf[2] = cmd;
    _buf[15] = 0xff;
    _buf[16] = 0xff;
    _buf[18] = 0xAB;

    crc = cmd + _buf[15] + _buf[16];

    for (i=0; i<12; i++) {
        if (i<len) {
            _buf[3+i] = data[i];
        } else {
            _buf[3+i] = 0;
        }
        crc += _buf[3+i];
    }

    _buf[17] = crc;

    //_out.flush();

    while (_out.read() >= 0 ){
      IF_SDEBUG(Serial.println(F("Sds011 skip byte")));
    }
    
    for (i = 0; i < 19; i++) {
        _out.write(_buf[i]);
    }
    //_out.flush();
}


bool Sds011::_read_response(void)
{

  IF_SDEBUG(Serial.println(F("Sds011 read_response")));
 
  unsigned short int nbytes = _out.readBytes(_buf, 10);

  IF_SDEBUG(Serial.print(F("Sds011 read:")));
  IF_SDEBUG(Serial.println(nbytes));
  for (short unsigned int i =0 ; i<nbytes ; i++) {
    IF_SDEBUG(Serial.print(_buf[i],HEX));
    IF_SDEBUG(Serial.print(F(",")));
  }
  IF_SDEBUG(Serial.println(F("<")));
  
  if (nbytes < 10) {
    IF_SDEBUG(Serial.println(F("Sds011 timeout")));
    return false;
  }

  if (!crc_ok()) {
    IF_SDEBUG(Serial.println(F("Sds011 wrong crc")));
    return false;
  }

  IF_SDEBUG(Serial.println(F("Sds011 crc ok")));  
  return true;
}


void Sds011::_filter_data(int n, int *pm25_table, int *pm10_table, int *pm25, int *pm10)
{
    int pm25_min, pm25_max, pm10_min, pm10_max, pm25_sum, pm10_sum;

    pm10_sum = pm10_min = pm10_max = pm10_table[0];
    pm25_sum = pm25_min = pm25_max = pm25_table[0];

    for (int i=1; i<n; i++) {
        if (pm10_table[i] < pm10_min) {
            pm10_min = pm10_table[i];
        }
        if (pm10_table[i] > pm10_max) {
            pm10_max = pm10_table[i];
        }
        if (pm25_table[i] < pm25_min) {
            pm25_min = pm25_table[i];
        }
        if (pm25_table[i] > pm25_max) {
            pm25_max = pm25_table[i];
        }
        pm10_sum += pm10_table[i];
        pm25_sum += pm25_table[i];
    }

    if (n > 2) {
        *pm10 = (pm10_sum - pm10_max - pm10_min)/(n-2);
        *pm25 = (pm25_sum - pm25_max - pm25_min)/(n-2);
    } else if (n > 1) {
        *pm10 = (pm10_sum - pm10_min)/(n-1);
        *pm25 = (pm25_sum - pm25_min)/(n-1);
    } else {
        *pm10 = pm10_sum/n;
        *pm25 = pm25_sum/n;
    }
}
