
/*
Copyright (C) 2015  Paolo Paruno <p.patruno@iperbole.bologna.it>
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
*/

#include "sim800Client.h"

//#include <inttypes.h>


#ifdef HARDWARESERIAL
  HardwareSerial *modem = &HARDWARESERIAL;
//modem = &HARDWARESERIAL;
#endif


// You must free the result if result is non-NULL.
char *str_replace( const char *orig, const char *rep, const char *with) {
    char *result; // the return string
    const char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep
    int len_with; // length of with
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    if (!orig)
        return NULL;
    if (!rep)
        rep = "";
    len_rep = strlen(rep);
    if (!with)
        with = "";
    len_with = strlen(with);

    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    tmp = result = (char*)malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}


// check presence of str2 in str1
bool found( const char * buf, const char * check ){
  //IF_SDEBUG(Serial.print(F("in:")));
  //IF_SDEBUG(Serial.println(buf));
  //IF_SDEBUG(Serial.print(F("search:")));
  if (!check) {
    //IF_SDEBUG(Serial.println(F("NONE")));
    return false;
  }
  //IF_SDEBUG(Serial.println(check));
  //return !strstr(buf-strlen(check), check);
  return strstr(buf, check);
}


SIM800::SIM800() {
}


// send to modem
void SIM800::send(const char *buf) {
  IF_SDEBUG(Serial.println(F("#sim800:SEND:")));
  IF_SDEBUG(Serial.println(buf));
  modem->print(buf);
  //Serial1.print(buf);
}

// receive until default timeout
byte SIM800::receive(char *buf) {
  return receive(buf, 5000);
}

// receive until timeout
byte SIM800::receive(char *buf, uint16_t timeout) {
  return SIM800::receive(buf, timeout, NULL, NULL);
}

// receive until timeout or string
bool SIM800::receive(char *buf, uint16_t timeout, char const *checkok, char const *checkerror) {
  byte count = 0;
  bool ok = false;
  bool error = false;
  unsigned long timeIsOut;
  char *rec;
  rec=buf;
  *rec = 0;

  timeIsOut = millis() + timeout;
  while (timeIsOut > millis() && count < (BUF_LENGTH - 1) && !ok && !error) {  
#ifdef ENABLEWDT
    wdt_reset();
#endif
    if (modem->available())
      //if (Serial1.available())
      {
	count++;
	*rec++ = modem->read();
	//*rec++ = Serial1.read();

	// if uncomment the line below the timeout will be inter-char timeout
	//timeIsOut = millis() + timeout;

	*rec = 0;           // terminate the string
	ok=found(buf,checkok);
	if (!ok){
	  error=found(buf,checkerror);
	}
      }
  }

  IF_SDEBUG(Serial.println(F("#sim800:RECEIVED:")));
  IF_SDEBUG(Serial.println(buf));

  if (checkok){
    if (ok){
      IF_SDEBUG(Serial.println(F("#sim800:->ok")));
    }else{
      IF_SDEBUG(Serial.println(F("#sim800:->not ok")));
    }
  }

  return ok;

}

// receive until timeout or number of bytes
bool SIM800::receivelen(char *buf, uint16_t timeout, unsigned int datalen) {
  byte count = 0;
  unsigned long timeIsOut;
  char *rec;
  rec=buf;
  *rec=0;

  timeIsOut = millis() + timeout;
  while (timeIsOut > millis() && count < datalen) {  
#ifdef ENABLEWDT
    wdt_reset();
#endif
    if (modem->available())
      //if (Serial1.available())
      {
	count++;
	*rec++ = modem->read();
	//*rec++ = Serial1.read();
	*rec = 0;           // terminate the string
      }
  }

  IF_SDEBUG(Serial.println(F("#sim800:RECEIVED:")));
  IF_SDEBUG(Serial.println(buf));

  if (count == datalen){
    IF_SDEBUG(Serial.println(F("#sim800:->ok")));
  }else{
    IF_SDEBUG(Serial.println(F("#sim800:->not ok")));
  }

  return count == datalen;
}

void SIM800::cleanInput () {
  bool avail =false;
  bool wasavail;

  if ((avail=modem->available())){
    //if ((avail=Serial.available())){
    IF_SDEBUG(Serial.print(F("#sim800:SKIP:")));
  }

  wasavail=avail;

  while (avail) 
    {
      char inchar = modem->read();
      //char inchar = Serial1.read();
      IF_SDEBUG(Serial.print(inchar)); 
      avail=modem->available();
      //avail=Serial1.available();
  }

  if (wasavail) IF_SDEBUG(Serial.println("")); 

}

// defaul at command with OK/ERROR response
bool SIM800::ATcommand(const char *command, char *buf) {
  return SIM800::ATcommand(command, buf, OKSTR, ERRORSTR, 5000);
}

bool SIM800::ATcommand(const char *command, char *buf, char const *checkok, char const *checkerror, unsigned long  timeout) {
			
  bool status;

  delay(100);
  cleanInput();

  send(ATSTR);
  send(command);
  send("\r\n");
  status = receive(buf, timeout, checkok,checkerror);

  if (status)
    {
      // remove crlfl from head
      if (strncmp (buf,"\r\n",2) == 0)
	{
	  // shift to left
	  for (unsigned int i = 0;( i <= strlen(buf) - 2); ++i)
	    {
	      buf[i] = buf[i + 2];
	    }
	}
      
      // remove crlfl from tail
      if (strlen(buf) > 2)
	{
	  if (strncmp (buf+strlen(buf)-2,"\r\n",2) == 0)
	    {
	      buf[strlen(buf)-2] = 0;
	    }
	}
    }
  return status;
}



#ifndef HARDWARESERIAL

bool SIM800::init(HardwareSerial *modemPort, byte onOffPin, byte resetPin) {

  modem = modemPort;

#else

bool SIM800::init(byte onOffPin, byte resetPin) {

#endif


  this->onOffPin = onOffPin;
  pinMode(onOffPin, OUTPUT);
  this->resetPin = resetPin;
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, HIGH);

  switchOn();
  resetModem();

  bool status;

  if (!(status=init_autobaud())){
    status = init_fixbaud();
  }
  return status;
}


bool SIM800::init_onceautobaud() {
  char buf[BUF_LENGTH];

  cleanInput();
  if (ATcommand("", buf)){        // sync autobaud
    state |= STATE_INITIALIZED;
    return true;
  }
  return false;
}

bool SIM800::init_autobaud() {
  state = STATE_NONE;

  modem->begin(115200);
  //Serial1.begin(115200);

  IF_SDEBUG(Serial.println(F("#sim800:initializing modem autobaud ...")));

  // try autobaud

  byte i = 1;
  do{
    if(init_onceautobaud()){
      IF_SDEBUG(Serial.println(F("#sim800:inizialize done")));
      return true;
    }
    //It is recommended to wait 3 to 5 seconds before sending the first AT
    //character. Otherwise undefined characters might be returned. 
    // I do it the second time to be more fast on startup
#ifdef ENABLEWDT
    wdt_reset();
#endif
    delay(5000);
#ifdef ENABLEWDT
    wdt_reset();
#endif
  } while (i++ < 3 && !isInitialized());

#ifdef ENABLEWDT
  wdt_reset();
#endif
  return false;
}

  // this is required to reset modem to default configuration and autobaud mode
bool SIM800::init_fixbaud() {

  char buf[BUF_LENGTH];
  char command[BUF_LENGTH];

  IF_SDEBUG(Serial.println(F("#sim800:initializing modem fixbaud ...")));

  // try different fixed baud rate
  long int baudrate []={1200,2400,4800,9600,19200,38400,57600,115200};

  for (byte i=0; (i<(sizeof(baudrate) / sizeof(long int))); i++) {
#ifdef ENABLEWDT
    wdt_reset();
#endif
    cleanInput();
    IF_SDEBUG(Serial.print(F("#sim800:TRY BAUDRATE:")));
    IF_SDEBUG(Serial.println(baudrate[i]));
    modem->begin(baudrate[i]);
    //Serial1.begin(baudrate[i]);
    
    if (ATcommand("", buf)){
      IF_SDEBUG(Serial.println(F("#sim800:baudrate found")));
      sprintf(command,"+IPR=%ld",baudrate[i]);
      if (ATcommand(command, buf)){
	IF_SDEBUG(Serial.println(F("#sim800:inizialize done")));
	if (!ATcommand("&F", buf)) return false;
	// this is the default
	//if (!ATcommand("+IPR=0", buf)) return false;
	if (!ATcommand("&W", buf)) return false;    // save

	// here a reset is required !

	// TODO for now I take the baudrate of TA as my baudrate

	   // switch off the modem
	   // if (!ATcommand("+CPOWD=1", buf)) return false;

	   // this require hardware patch on microduino
	   // resetModem();

	   // redo everythings in autobaud
	   // return init_autobaud(modemPort, onOffPin,resetPin);
	//

	// for now I go forward, but the next boot it will be autobaud
	state |= STATE_INITIALIZED;
	return true;
      }
    }
  }
  state &= ~STATE_INITIALIZED;
  IF_SDEBUG(Serial.println(F("#sim800:inizialize failed")));
#ifdef ENABLEWDT
  wdt_reset();
#endif
  return false;
}


bool SIM800::setup() {
  char buf[BUF_LENGTH];

  if(!isInitialized()) return false;

  if (!ATcommand("&F", buf)) return false;    // reset
  if (!ATcommand("E0", buf)) return false;    // echo mode off

  //ATcommand("+CIPMUX=?", buf);
  //ATcommand("+CIPMUX?", buf);
  //ATcommand("+CIPMODE=?", buf);
  //ATcommand("+CIPMODE?", buf);
  //ATcommand("+CIPCCFG?", buf);

  // wait Network Registration
  unsigned short int ntryreg=0;
  while(ATcommand("+CREG?", buf)) {
    if (found(buf,"+CREG: 0,1")) break;
    if (ntryreg++ == 20) return false;
    IF_SDEBUG(Serial.println(F("#sim800 wait for Network Registration")));
    IF_SDEBUG(if(found(buf,"+CREG: 0,2")) Serial.println(F("Not registered, but MT is currently searching a new operator to register to")));
    IF_SDEBUG(if(found(buf,"+CREG: 0,3")) Serial.println(F("Registration denied")));
    IF_SDEBUG(if(found(buf,"+CREG: 0,4")) Serial.println(F("Unknown")));
    IF_SDEBUG(if(found(buf,"+CREG: 0,5")) Serial.println(F("Registered,roaming")));
    delay(1000);

#ifdef ENABLEWDT
    wdt_reset();
#endif
  }

  return true;
}

bool SIM800::startNetwork(const char *apn, const char *user, const char *pwd ) {
  char buf[BUF_LENGTH];
  char bufcommand[BUFCOMMAND_LENGTH];

  IF_SDEBUG(Serial.println(F("#sim800:start network ...")));

  if(!isInitialized()) return false;

  // restart network if already connected
  if (isRegistered()) stopNetwork();

  if (!ATcommand("+CIPMUX=0", buf)) return false; //IP Single Connection

  if (!isRegistered()){
    if (ATcommand("+SAPBR=3,1,\"Contype\",\"GPRS\"", buf)){
      sprintf(bufcommand,"+SAPBR=3,1,\"APN\",\"%s\"",apn );
      if (ATcommand(bufcommand, buf)){
	sprintf(bufcommand,"+SAPBR=3,1,\"USER\",\"%s\"",user );
	if (ATcommand(bufcommand, buf)){
	  sprintf(bufcommand,"+SAPBR=3,1,\"PWD\",\"%s\"",pwd );
	  if (ATcommand(bufcommand, buf)){
  // 85 sec timeout!
	    if(ATcommand("+SAPBR=1,1", buf, OKSTR, ERRORSTR, 85000)){
	      if(ATcommand("+SAPBR=2,1", buf)){
		state |= STATE_REGISTERED;
	      }
	    }
	  }
	}
      }
    }
  }

#ifdef DEBUG
  if(isRegistered() {
      Serial.println(F("start network done")));
    }else{
      Serial.println(F("start network failed")));
    }
#endif

  return isRegistered();

}

// return false if wrong response or already disconnected
bool SIM800::stopNetwork() {
  char buf[BUF_LENGTH];
  bool status=false;

  IF_SDEBUG(Serial.println(F("#sim800:stop network ...")));

  //if (isRegistered()){
  // force stop ever!!
  if (status=ATcommand("+SAPBR=0,1", buf , OKSTR, ERRORSTR, 65000)){
    state &= ~STATE_REGISTERED;
  }
  //}
  
  IF_SDEBUG(Serial.println(F("#sim800:stop network done")));
  return status;
}



bool SIM800::httpGET(const char* server, int port, const char* path, char* result, int resultlength)
{
  char bufcommand[BUFCOMMAND_LENGTH];
  char buf[BUF_LENGTH];
  char* newpath;

  IF_SDEBUG(Serial.println(F("#sim800:start httpget ...")));

  if(!isInitialized()) return false;
  if(!isRegistered()) return false;

  if (isHttpInitialized()){
    ATcommand("+HTTPTERM", buf);
    state &= ~STATE_HTTPINITIALIZED;
  }

  if(!ATcommand("+HTTPINIT", buf)){
    ATcommand("+HTTPTERM", buf);
    if(!ATcommand("+HTTPINIT", buf)) return false;
    state |= STATE_HTTPINITIALIZED;
  }

  if(!ATcommand("+HTTPPARA=\"CID\",1", buf)) return false;
  newpath = str_replace(path,"\"", "%22");
  sprintf(bufcommand, "+HTTPPARA=\"URL\",\"%s%s\"",server,newpath );
  free(newpath);
  if(!ATcommand(bufcommand, buf)) return false;
  if(!ATcommand("+HTTPACTION=0", buf)) return false;
  //receive(buf,5000,"\n",NULL);             // here we receive some spourious \r\n; do not wait for it
  receive(buf,20000,"+HTTPACTION",NULL);     // timeout for response 20 sec
  receive(buf,5000,"\n",NULL);
  int method;
  int status;
  int datalen;
  int token_count = sscanf(buf,":%i,%i,%i",&method,&status,&datalen);
  if ( token_count == 3 ){
    IF_SDEBUG(Serial.print(F("#sim800:method: ")));
    IF_SDEBUG(Serial.println(method));
    IF_SDEBUG(Serial.print(F("#sim800:status: ")));
    IF_SDEBUG(Serial.println(status));
    IF_SDEBUG(Serial.print(F("#sim800:datalen: ")));
    IF_SDEBUG(Serial.println(datalen));
    }else{
    IF_SDEBUG(Serial.println(F("#sim800:ERROR httpaction")));
    IF_SDEBUG(Serial.print(F("#sim800:token count: ")));
    IF_SDEBUG(Serial.println(token_count));
    IF_SDEBUG(Serial.println(buf));
    return false;
  }
  if (status != 200) return false;
  send("AT+HTTPREAD\r\n");
  if(!receive(buf,5000,"\r\n",NULL)) return false;  // get +HTTPREAD: n
  if(!receive(buf,5000,"\r\n",NULL)) return false;   // gel null line

  if (datalen+1 > resultlength){
    IF_SDEBUG(Serial.println(F("#sim800:ERROR no buffer space for http response")));
    return false;
  }
  if(!receivelen(result,5000,datalen)) return false;
  if(!receive(buf,5000,"OK\r\n",NULL)) return false;  
  if(!ATcommand("+HTTPTERM", buf)) return false;
  state &= ~STATE_HTTPINITIALIZED;

  return true;
}



bool SIM800::GetMyIP(char*ip) {
  char buf[BUF_LENGTH];
  //char ip[16];
  bool retstatus;

  if(!isInitialized()) return false;
  if(!isRegistered()) return false;

  IF_SDEBUG(Serial.println(F("#sim800:checking network ...")));
  if ((retstatus=ATcommand("+SAPBR=2,1", buf))){
    if (found(buf,"+SAPBR: 1,1")) {

      int token_count = sscanf(buf,"+SAPBR: 1,1,%s",ip);
      if ( token_count == 1 ){
	IF_SDEBUG(Serial.print(F("#sim800:IP: ")));
	IF_SDEBUG(Serial.println(ip));
      }else{
	IF_SDEBUG(Serial.println(F("#sim800:ERROR getting IP")));
	IF_SDEBUG(Serial.print(F("#sim800:token count: ")));
	IF_SDEBUG(Serial.println(token_count));
	IF_SDEBUG(Serial.println(F("#sim800:")));
	IF_SDEBUG(Serial.println(buf));
	retstatus=false;
	strcpy(ip,"0.0.0.0");
      }
    }else {
      retstatus=false;
      strcpy(ip,"0.0.0.0");
    }
  }else{
    retstatus=false;
    strcpy(ip,"0.0.0.0");
  }
  
  return retstatus;
}




bool SIM800::getIMEI(char*imei) {
  char buf[BUF_LENGTH];
  bool retstatus;

  if(!isInitialized()) return false;
  //  if(!isRegistered()) return false;

  IF_SDEBUG(Serial.println(F("#sim800:get IMEI")));
  if ((retstatus=ATcommand("+GSN", buf))){
    int token_count = sscanf(buf,"%s\r\n",imei);
    if ( token_count == 1 ){
      IF_SDEBUG(Serial.print(F("#sim800:IMEI: ")));
      IF_SDEBUG(Serial.println(imei));
    }else{
      IF_SDEBUG(Serial.println(F("#sim800:ERROR getting IMEI")));
      IF_SDEBUG(Serial.print(F("#sim800:token count: ")));
      IF_SDEBUG(Serial.println(token_count));
      IF_SDEBUG(Serial.println(buf));
      retstatus=false;
      strcpy(imei,"000000000000000");
    }
  }else {
    retstatus=false;
    strcpy(imei,"000000000000000");
  }
  
  return retstatus;
}


bool SIM800::getSignalQualityReport(int*rssi,int*ber) {
  /*
    returns received signal strength indication <rssi>
    and channel bit error rate <ber> 
  */
  
  char buf[BUF_LENGTH];
  bool retstatus;

  if(!isInitialized()) return false;
  //  if(!isRegistered()) return false;

  IF_SDEBUG(Serial.println(F("#sim800:get Signal Quality Report")));
  if ((retstatus=ATcommand("+CSQ", buf))){
    int token_count = sscanf(buf,"+CSQ: %i,%i\r\n",rssi,ber);
    if ( token_count == 2 ){
      IF_SDEBUG(Serial.print(F("#sim800:rssi: ")));
      IF_SDEBUG(Serial.println(*rssi));
      IF_SDEBUG(Serial.print(F("#sim800:ber: ")));
      IF_SDEBUG(Serial.println(*ber));
    }else{
      IF_SDEBUG(Serial.println(F("#sim800:ERROR getting Signal Quality Report")));
      IF_SDEBUG(Serial.print(F("#sim800:token count: ")));
      IF_SDEBUG(Serial.println(token_count));
      IF_SDEBUG(Serial.println(buf));
      retstatus=false;
      *rssi=99;
      *ber=99;
    }
  }else {
    retstatus=false;
    *rssi=99;
    *ber=99;
  }
  
  return retstatus;
}

bool SIM800::checkNetwork() {
  char buf[BUF_LENGTH];
  bool retstatus;

  if(!isInitialized()) return false;
  if(!isRegistered()) return false;

  IF_SDEBUG(Serial.println(F("#sim800:checking network ...")));
  if ((retstatus=ATcommand("+SAPBR=2,1", buf))){
    // here the secon number=1 means all is OK
    if (found(buf,"+SAPBR: 1,1")) {
      state |= STATE_REGISTERED;
    }else{
      IF_SDEBUG(Serial.println(F("#sim800:ERROR network status")));
      IF_SDEBUG(Serial.println(buf));
      state &= ~STATE_REGISTERED;
      retstatus=false;
    }
  
  }else {
    state &= ~STATE_REGISTERED;
  }

  IF_SDEBUG(Serial.println(F("#sim800:cheking network done")));
  return retstatus;
}

// todo to be used
/*
 * Reads a token from the given string. Token is seperated by the 
 * given delimiter.
 */
/*

char *SIM800::readToken(char *str, char *buf, char delimiter) {
  uint8_t c = 0;
  while (true) {
    c = *str++;
    if ((c == delimiter) || (c == '\0')) {
      break;
    }
    else if (c != ' ') {
      *buf++ = c;
    }
  }
  *buf = '\0';
  return str;
}
*/

bool SIM800::isOn() {
  return (state & STATE_ON);
}

bool SIM800::isInitialized() {
  return (state & STATE_INITIALIZED);
}

bool SIM800::isRegistered() {
  return (state & STATE_REGISTERED);
}
bool SIM800::isHttpInitialized() {
  return (state & STATE_HTTPINITIALIZED);
}


void SIM800::switchOn() {
  IF_SDEBUG(Serial.println(F("#sim800:switching on")));
  if (!isOn()) {
    switchModem();
    state |= STATE_ON;
  }
  IF_SDEBUG(Serial.println(F("#sim800:done")));
}

void SIM800::switchOff() {
  IF_SDEBUG(Serial.println(F("#sim800:switching off")));
  if (isOn()) {
    switchModem();
    state &= ~STATE_ON;
  }
  IF_SDEBUG(Serial.println(F("#sim800:done")));
}

void SIM800::switchModem() {
  digitalWrite(onOffPin, HIGH);
  delay(1500);
  digitalWrite(onOffPin, LOW);
  delay(2000);
#ifdef ENABLEWDT
  wdt_reset();
#endif
}

void SIM800::resetModem() {
  digitalWrite(resetPin, LOW);
  delay(300);
  digitalWrite(resetPin, HIGH);
  delay(3000);
#ifdef ENABLEWDT
  wdt_reset();
#endif

}

time_t SIM800::RTCget()   // Aquire data from buffer and convert to time_t
{
  tmElements_t tm;
  
  if (RTCread(tm) != 0){
    return 0UL;
  }
  return(makeTime(tm));
}

uint8_t  SIM800::RTCset(time_t t)
{
  tmElements_t tm;

  if (t!= 0UL){
    breakTime(t, tm);
    return RTCwrite(tm);
  }
  return 1;
}

uint8_t SIM800::RTCread( tmElements_t &tm){

  char buf[BUF_LENGTH];

  if (ATcommand("+CCLK?",buf)){

    //"yy/MM/dd,hh:mm:ss±zz"
    //int token_count = sscanf(buf,"+CCLK: \"%02hu/%02hu/%02hu,%02hu:%02hu:%02hu+00\"\r\n",&tm.Year,&tm.Month,&tm.Day,&tm.Hour,&tm.Minute,&tm.Second);
    //int token_count = sscanf(buf,"+CCLK: \"%02SCNd8/%02SCNd8/%02SCNd8,%02SCNd8:%02SCNd8:%02SCNd8+00\"\r\n",&tm.Year,&tm.Month,&tm.Day,&tm.Hour,&tm.Minute,&tm.Second);
    //int token_count = sscanf(buf,"+CCLK: \"%02"SCNd8"/%02"SCNd8"/%02"SCNd8",%02"SCNd8":%02"SCNd8":%02"SCNd8"+00\"\r\n",&tm.Year,&tm.Month,&tm.Day,&tm.Hour,&tm.Minute,&tm.Second);

    int token_count = sscanf(buf,"+CCLK: \"%02hhd/%02hhd/%02hhd,%02hhd:%02hhd:%02hhd+00\"\r\n",&tm.Year,&tm.Month,&tm.Day,&tm.Hour,&tm.Minute,&tm.Second);

    //tm.Wday
    if (token_count == 6){
      if (tm.Year == 1) return 1;         // never set
      tm.Year = y2kYearToTm(tm.Year);
      return 0;
    }
  }
  return 1;                            // error in atcommand
}

uint8_t SIM800::RTCwrite(tmElements_t &tm){

  //ATcommand("+CCLK=\"yy/MM/dd,hh:mm:ss±zz\"",buf);
  //ATcommand("+CCLK=\"15/04/27,16:12:00+00\"",buf);

  char buf[BUF_LENGTH];
  char command[BUF_LENGTH];

  sprintf(command,"+CCLK=\"%02i/%02i/%02i,%02i:%02i:%02i+00\"",tmYearToY2k(tm.Year),tm.Month,tm.Day,tm.Hour,tm.Minute,tm.Second);
  //tm.Wday

  if (ATcommand(command,buf)){
    return 0;
  }
  return 1;

}


/*
user should use the command group AT+CSTT, AT+CIICR and AT+CIFSR to start the
task and activate the wireless connection. Lastly, user can establish TCP connection between
SIM800 series and the server by AT command (AT+CIPSTART=”TCP”,”IP Address of server”,
“port number of server”). If the connection is established successfully, response “CONNECT OK”
will come up from the module.
User can close the TCP connection with “AT+CIPCLOSE”
*/

bool SIM800::TCPstart(const char *apn, const char *user, const char *pwd ) {
  char buf[BUF_LENGTH];
  char bufcommand[BUFCOMMAND_LENGTH];

  IF_SDEBUG(Serial.println(F("#sim800:start TCP ...")));

  // restart network if already connected
  //if (isRegistered()) TCPstop();

  IF_SDEBUG(Serial.print(F("#sim800:")));
  IF_SDEBUG(ATcommand("+CPIN?", buf));
  IF_SDEBUG(Serial.print(F("#sim800:")));
  IF_SDEBUG(ATcommand("+CSQ", buf));
  IF_SDEBUG(Serial.print(F("#sim800:")));
  IF_SDEBUG(ATcommand("+CREG?", buf));
  IF_SDEBUG(Serial.print(F("#sim800:")));
  IF_SDEBUG(ATcommand("+CGATT?", buf));

  // If not attached to GPRS attach to GPRS
  if(!ATcommand("+CGATT?", buf, "+CGATT: 1", "+CGATT: 0", 5000)) {
    if(!ATcommand("+CGATT=1", buf, OKSTR, ERRORSTR, 10000)) return false;
  }
  
  //  if (!ATcommand("+CIPMUX=0", buf)) return false; //IP Single Connection
  if (!ATcommand("+CIPMODE=1", buf)) return false; //IP transparent mode
  if (!ATcommand("+CIPCCFG=8,2,1024,1,0,1460,50", buf)) return false; // fixed the second parameter minimum is 2
  //For Sim900 /800 
  //+CIPCCFG: (NmRetry:3-8),(WaitTm:2-10),(SendSz:1-1460),(esc:0,1) ,(Rxmode:0,1), (RxSize:50-1460),(Rxtimer:20-1000)
  /*
    2.2.2 SIM800 Series_TCPIP_Application Note_V1.00 
    How to Configure Transparent Mode
    To enable transparent mode, the command AT+CIPMODE should be set to 1. In transparent
    mode, the command AT+CIPCCFG is used for configuring transfer mode, which has 7 parameters
    NmRetry, WaitTm, SendSz, Esc, Rxmode, RxSize, Rxtimer.

    NmRetry:    Number of retries to be made for an IP packet.
    WaitTm:     Number of 200ms intervals to wait for serial input before sending the packet
    SendSz:     Size in bytes of data block to be received from serial port before sending.
    Esc:        Whether turn on the escape sequence, default is TRUE.
    Rxmode:     Whether to set time interval during output data from serial port.
    RxSize:     Output data length for each time, default value is 1460.
    Rxtimer:    Time interval (ms) to wait for serial port to output data again. Default value: 50ms 
  */

  sprintf(bufcommand,"+CSTT=\"%s\",\"%s\",\"%s\"",apn,user,pwd );
  //sprintf(bufcommand,"+CSTT=\"%s\"",apn );
  if (!ATcommand(bufcommand, buf)) return false;
  if (!ATcommand("+CIICR", buf)) return false;

  if (TCPGetMyIP(buf)) {
    state |= STATE_REGISTERED;
    IF_SDEBUG(Serial.println(F("#sim800:start TCP done")));
    return true;
  }

  return false;

}


bool SIM800::TCPstop()
{
  char buf[BUF_LENGTH];

  state &= ~STATE_REGISTERED;

  if (!ATcommand("+CIPSHUT", buf,"SHUT OK", ERRORSTR, 65000)) return false;

  // If attached to GPRS detach GPRS
  if(!ATcommand("+CGATT?", buf, "+CGATT: 0", "+CGATT: 1", 5000)) {
    if(!ATcommand("+CGATT=0", buf)) return false;
  }

  return true;
}

bool SIM800::TCPGetMyIP(char*ip) {
  char buf[BUF_LENGTH];
  //char ip[16];
  bool retstatus;

  IF_SDEBUG(Serial.println(F("#sim800:checking network ...")));
  //if ((retstatus=ATcommand("+CIFSR", buf))){
  send("AT+CIFSR\r\n");
  retstatus=receive(buf,5000,"\r\n",NULL);

  if((retstatus=receive(buf,5000,"\r\n",NULL))){
    int token_count = sscanf(buf,"%s\r\n",ip);
    if ( (token_count == 1) & !(strcmp(ip,"ERROR")==0)){
      IF_SDEBUG(Serial.print(F("#sim800:IP: ")));
      IF_SDEBUG(Serial.println(ip));
    }else{
      IF_SDEBUG(Serial.println(F("#sim800:ERROR getting IP")));
      IF_SDEBUG(Serial.print(F("#sim800:token count: ")));
      IF_SDEBUG(Serial.println(token_count));
      IF_SDEBUG(Serial.println(buf));
      retstatus=false;
      strcpy(ip,"0.0.0.0");
    }
  }else {
    retstatus=false;
    strcpy(ip,"0.0.0.0");
  }
  
  return retstatus;
}


bool SIM800::TCPconnect(const char* server, int port)
{
  char bufcommand[BUFCOMMAND_LENGTH];
  char buf[BUF_LENGTH];

  if (isRegistered()){

    sprintf(bufcommand, "+CIPSTART=\"TCP\",\"%s\",\"%i\"",server,port );
    if(!ATcommand(bufcommand, buf)) return false;
    receive(buf,150000,"\r\n",NULL);
    receive(buf,10000,"\r\n",NULL);
    if (found(buf,"CONNECT")){
      state |= STATE_HTTPINITIALIZED;

      return true;
    }
  }
  return false;
}


  //
  // etherclient compatibility
  //

sim800Client::sim800Client() {
}

int sim800Client::connect(IPAddress ip, int port)
{
  char server[16];
  sprintf(server, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  return connect(server,port);
}

int sim800Client::connect(const char *server, int port)
{
  return TCPconnect(server,port);
}

uint8_t sim800Client::connected()
{
  return isHttpInitialized();
}

int sim800Client::available()
{
#ifdef ENABLEWDT
  wdt_reset();
#endif
  return modem->available();
}

int sim800Client::read()
{
  byte buf;
  buf = modem->read();
  //Serial.print("----> read:");
  //Serial.println(buf,HEX);
  return buf;
}

byte sim800Client::readBytes(char *buf, size_t size)
{
  byte rc =  modem->readBytes(buf,size);
  //Serial.print("----> readBytes:");
  //for (size_t i=0; i<rc; i++) {
  //  Serial.print((byte)buf[i],HEX);
  //  Serial.print(":");
  //}
  //Serial.println("");
  return rc;
}

void sim800Client::setTimeout(unsigned long timeout)
{
  modem->setTimeout(timeout);
}

size_t sim800Client::write(uint8_t buf)
{
  //Serial.print("---> write:");
  //Serial.println(buf,HEX);
  return modem->write(buf);
}
size_t sim800Client::write(const uint8_t *buf, size_t size)
{
  //Serial.print("---> write:");
  //for (size_t i=0; i<size; i++){
  //  Serial.print(buf[i],HEX);
  //  Serial.print(":");
  //}
  //Serial.println("");

  return modem->write(buf,size);
}

bool sim800Client::transparentescape()
{
  char buf[BUF_LENGTH];
  // escape sequence
  IF_SDEBUG(Serial.println(F("#sim800:send escape sequence")));
  delay(1000);
  modem->write("+");
  modem->write("+");
  modem->write("+");
  delay(1000);
#ifdef ENABLEWDT
  wdt_reset();
#endif
  return ATcommand("", buf);

}


bool sim800Client::transparent()
{
  char buf[BUF_LENGTH];

  IF_SDEBUG(Serial.println(F("#sim800:going to transparent mode")));
  return ATcommand("O", buf, "CONNECT", ERRORSTR, 5000);

}


void sim800Client::flush()
{
  return;
}

void sim800Client::stop()
{
  char buf[BUF_LENGTH];

  IF_SDEBUG(Serial.println(F("#sim800:stop")));
  state &= ~STATE_HTTPINITIALIZED;
  transparentescape();
  ATcommand("+CIPCLOSE=0", buf);

  return;
}
