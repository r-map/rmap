/*
 Basic MQTT example 
 
  - connects to an MQTT server
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic"
*/

#define GSMAPN "ibox.tim.it"
#define GSMUSER ""
#define GSMPASSWORD ""

#include <sim800bClient.h>
#include <PubSubClient.h>
#include <TimeAlarms.h>

sim800Client s800;

// Update these with values suitable for your network.
//byte server[] = { 192, 168, 1, 199 };
char server[] = "rmap.cc";

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  char mypl[48];
  Serial.println(length);
  memcpy(mypl,payload,length);
  mypl[length]=char(0);
  Serial.print("receive: ");
  Serial.print(topic);
  Serial.print("->");
  Serial.println(mypl);
}


PubSubClient client(server, 1883, callback, s800);


// utility function to debug
void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

// utility function to debug
void digitalClockDisplay(time_t t ){
  // digital clock display of the time

  Serial.print(F("#"));

  if (t == 0UL){
    Serial.println(F("NULL"));
  }
  else{	  
    printDigits(hour(t));
    Serial.print(":");
    printDigits(minute(t));
    Serial.print(":");
    printDigits(second(t));
    Serial.print(" ");
    printDigits(day(t));
    Serial.print(" ");
    printDigits(month(t));
    Serial.print(" ");
    Serial.print(year(t)); 
    Serial.println(); 
  }
}

time_t scantime(const char *buf)
{
  tmElements_t tm;
  int token_count = sscanf(buf,"%02hhd/%02hhd/%02hhd,%02hhd:%02hhd:%02hhd+00\n",&tm.Year,&tm.Month,&tm.Day,&tm.Hour,&tm.Minute,&tm.Second);
  //tm.Wday
  if (token_count == 6){
    tm.Year = y2kYearToTm(tm.Year);
    return(makeTime(tm));
  }
  return 0UL;
}

char mainbuf[200];


time_t ResyncGSMRTC() {

  time_t t=0UL;

  if (s800.isRegistered()) {
    wdt_reset();
    // compose URL
    for (int i = 0; ((i < 3) & (t == 0UL)); i++){ 
      strcpy (mainbuf, "/http2mqtt/?time=t");
      Serial.print(F("#GSM send get:"));
      Serial.println(mainbuf);
      if ((s800.httpGET("rmap.cc", 80,mainbuf, mainbuf, sizeof(mainbuf)))){
	//Print the results.
	Serial.println(F("#GSM Data received:"));
	Serial.println(mainbuf);
	t=scantime(mainbuf);
	digitalClockDisplay(t);
      }
    }
  }
  return t;
}

// this task resync GSM and system clock with httpget
// when GSMGPRSRTCBOOT is enabled the system clock is not synked with RTC
// so this do that manually
time_t periodicResyncGSMRTC() {

  time_t tt;
  time_t t;

  //disconn clean
  client.publish("test/out","disconn");
  client.disconnect();
  s800.TCPstop();
  
  s800.stopNetwork();

  // get first guess time from sim800 RTC
  t = s800.RTCget();
  digitalClockDisplay(t);

  for (int i = 0; ((i < 10) &  !s800.checkNetwork()); i++) {
    s800.startNetwork(GSMAPN, GSMUSER, GSMPASSWORD);
  }

  if ((tt=ResyncGSMRTC()) != 0UL){
    t=tt;
    s800.RTCset(t);
    digitalClockDisplay(t);
}

  s800.stopNetwork();
  s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD);
  for (int i = 0; ((i < 10) & !client.connect("arduinoClient")); i++) {
    Serial.println("connect failed");
    s800.stopNetwork();
    s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD);
  }
	 
  Serial.println("connected");
  client.publish("test/out","conn");
  client.subscribe("test/in");

  return t;

}

time_t periodicResyncGSMRTC() {

  time_t tt;
  time_t t;

  //disconn clean
  client.publish("test/out","disconn");
  client.disconnect();
  s800.TCPstop();
  
  s800.stopNetwork();

  // get first guess time from sim800 RTC
  t = s800.RTCget();
  digitalClockDisplay(t);

  for (int i = 0; ((i < 10) &  !s800.checkNetwork()); i++) {
    s800.startNetwork(GSMAPN, GSMUSER, GSMPASSWORD);
  }

  if ((tt=ResyncGSMRTC()) != 0UL){
    t=tt;
    s800.RTCset(t);
    digitalClockDisplay(t);
}

  s800.stopNetwork();
  s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD);
  for (int i = 0; ((i < 10) & !client.connect("arduinoClient")); i++) {
    Serial.println("connect failed");
    s800.stopNetwork();
    s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD);
  }
	 
  Serial.println("connected");
  client.publish("test/out","conn");
  client.subscribe("test/in");

  return t;

}


void pub()
{
  Serial.print("send: ");
  Serial.print("test/out");
  Serial.print("->");
  Serial.println("bye bye");
  client.publish("test/out","bye bye");
}

void setup()
{

  Serial.begin(9600);
  Serial.println("SIM800 Shield testing.");

  //Ethernet.begin(mac, ip);

  for (int i=0; i<10; i++){
    delay(5000);
    Serial.println("try to init sim800");

#ifdef HARDWARESERIAL
    if (s800.init( 7, 6)) break;
#else
    if (s800.init(&Serial1 , 7, 6)) break;
#endif

  }

  Serial.println("try to setup sim800");
  s800.setup();
  s800.stop();
  s800.TCPstop();
  s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD);

  while (!client.connect("arduinoClient")) {
    Serial.println("connect failed");
    delay(1000);
  }

  Serial.println("connected");
  client.publish("test/out","conn");
  client.subscribe("test/in");
  setSyncProvider(periodicResyncGSMRTC);
  Alarm.timerRepeat(10, pub);             // timer
}

void loop()
{
    client.loop();
    Alarm.delay(1000); 
    //Returns the status of time sync
    int status=timeStatus();
    if (status == timeNotSet) Serial.println("Time's clock has not been set.  The time & date are unknown.");
    if (status == timeSet) Serial.println(" Time's clock has been set.");
    if (status == timeNeedsSync) Serial.println("Time's clock is set, but the sync has failed, so it may not be accurate");
    digitalClockDisplay(now());
    
}

