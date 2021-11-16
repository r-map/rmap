#define TIMEOUT 6000L
#define REPEATWAIT  5
#define ENABLE_SUBSCRIBE
#define ENABLE_QOS0
#define ENABLE_QOS1

#define ENABLE_QOS2
// in paho library QOS2 is disabled by default; enable it
#define MQTTCLIENT_QOS2 1 

#define MQTTCLIENT_QOS MQTT::QOS1
#define SERIAL_TRACE_LEVEL                (SERIAL_TRACE_LEVEL_DEBUG)
#define SIM800_ON_OFF_PIN     (5)
#define GSM_APN        ("ibox.tim.it")
#define GSM_USERNAME   ("")
#define GSM_PASSWORD   ("")
#define TOPIC "test/MQTTClient/sim800"
#define HOSTNAME "test.rmap.cc"
#define DATE_TIME_STRING_LENGTH (20)
#ifndef ARDUINO_ARCH_AVR
HardwareSerial Serial1(PB11, PB10);
#endif


#include <debug_config.h>
#include <sim800Client.h>
#include <Sim800IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>
#include <ArduinoLog.h>

unsigned int arrivedcount = 0;

void messageArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  
  Serial.print("Receive message ");
  Serial.print(--arrivedcount);
  Serial.print(" arrived: qos ");
  Serial.print(message.qos);
  Serial.print(", retained: ");
  Serial.print(message.retained);
  Serial.print(", dup: ");
  Serial.print(message.dup);
  Serial.print(", packetid: ");
  Serial.print(message.id);
  Serial.println(", Payload:");
  Serial.println((char*)message.payload);
}


sim800Client s800;
IPStack ipstack(s800);
MQTT::Client<IPStack, Countdown, 120, 2> client = MQTT::Client<IPStack, Countdown, 120, 2>(ipstack,TIMEOUT);

bool initmodem(void)
{
  LOGN("try to init sim800");
  if(s800.init(SIM800_ON_OFF_PIN)){
    s800.setTimeout(6000);
    LOGN("initialized sim800");
  }else{
    LOGN("error initializing sim800");
    return false;
  }
    
  while(true){
    int rc = s800.switchOn();
    if (rc == SIM800_OK) {
      delay(SIM800_WAIT_FOR_AUTOBAUD_DELAY_MS);
      LOGN("switch on OK");
      break;
    }else if (rc == SIM800_ERROR){
      LOGN("switch on failed");
      return false;
    }
  }

  while(true){
    int rc =s800.initAutobaud();
    if (rc == SIM800_OK) {
      delay(SIM800_WAIT_FOR_AUTOBAUD_DELAY_MS);
      LOGN("autobaud OK");
      break;
    }else if (rc == SIM800_ERROR){
      LOGN("autobaud failed");
      return false;
    }
  }

  while(true){
    int rc =s800.setup();
    if (rc == SIM800_OK) {
      LOGN("setup OK");
      break;
    }else if (rc == SIM800_ERROR){
      LOGN("setup failed");
      return false;
    }
  }
  

  while(true){
    int rc =s800.startConnection(GSM_APN,GSM_USERNAME,GSM_PASSWORD);
    if (rc == SIM800_OK) {
      LOGN("start connection OK");
      break;
    }else if (rc == SIM800_ERROR){
      LOGN("start connection failed");
      return false;
    }
  }
  return true;
}


bool connect(void)
{
  char* hostname = HOSTNAME;
  int port = 1883;

  Serial.print("Connecting to ");
  Serial.print(hostname);
  Serial.print(":");
  Serial.println(port);

  while(true){
    int rc = ipstack.connect(hostname, port);
    if (rc == 2){
      Serial.print("error rc from TCP connect is ");
      Serial.println(rc);
      return false;
    }else if (rc == 1){
      Serial.println("TCP connected");
      break;
    }
  }

  Serial.println("MQTT connecting");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"test-ID";
  int rc = client.connect(data);
  if (rc != 0)
  {
    Serial.print("error rc from MQTT connect is ");
    Serial.println(rc);
    return false;
  }else{
    Serial.println("MQTT connected");
  }
  
#ifdef ENABLE_SUBSCRIBE
  rc = client.subscribe(TOPIC, MQTTCLIENT_QOS, messageArrived);   
  if (rc != 0)
  {
    Serial.print("rc from MQTT subscribe is ");
    Serial.println(rc);
    return false;
  }else{
    Serial.println("MQTT subscribed");
  }
#endif

  return true;
}

bool publish(void)
{ 

  static long unsigned int sentcount = 0;
  static long unsigned int errorcount = 0;

  MQTT::Message message;
  char* clientid="testID";
  
  int rc;
  char buf[100];
  message.retained = false;
  message.dup = false;
  message.payload = (void*)buf;

  LOGN("Publish");
  
#ifdef ENABLE_QOS0
  strcpy(buf, "Hello World! QoS 0 message");
  LOGN("Send QoS 0 message: %s",buf);
  message.qos = MQTT::QOS0;
  message.payloadlen = strlen(buf)+1;
  rc = client.publish(TOPIC, message);
  if (rc != 0)
  {
    LOGN("rc from MQTT pubblish is %d",rc);
    errorcount++;
  }else{
    arrivedcount++;
    sentcount++;
  }

  #ifdef ENABLE_SUBSCRIBE
  int i=0;
  while (arrivedcount > 0 && i++ < REPEATWAIT)
    {
      LOGN("Waiting for QoS 0 message");
      client.yield(TIMEOUT);
    }

  if (arrivedcount > 0 ){
    LOGN("message lost, Q0S 0: %d",arrivedcount);
  }
  arrivedcount=0;
  
#endif
#endif

#ifdef ENABLE_QOS1 
  strcpy(buf, "Hello World! QoS 1 message");
  LOGN("Send QoS 1 message: %s",buf);
  message.qos = MQTT::QOS1;
  message.payloadlen = strlen(buf)+1;
  rc = client.publish(TOPIC, message);
  if (rc != 0)
  {
    LOGN("rc from MQTT pubblish is %d",rc);
    errorcount++;
  }else{
    arrivedcount ++;
    sentcount++;
  }
#ifdef ENABLE_SUBSCRIBE

  int ii=0;
  while (arrivedcount > 0 && ii++ < REPEATWAIT)
    {
      LOGN("Waiting for QoS 1 message");
      client.yield(TIMEOUT);
    }

  if (arrivedcount > 0 ){
      LOGN("message lost, Q0S 1: %d",arrivedcount);
  }

  arrivedcount=0;
  
#endif
#endif
  
#ifdef ENABLE_QOS2
  strcpy(buf, "Hello World! QoS 2 message");
  LOGN("Send QoS 2 message: %s",buf);
  message.qos = MQTT::QOS2;
  message.payloadlen = strlen(buf)+1;
  rc = client.publish(TOPIC, message);
  if (rc != 0)
  {
    LOGN("rc from MQTT pubblish is %d",rc);
    errorcount++;
  }else{
    arrivedcount ++;
    sentcount++;
  }

#ifdef ENABLE_SUBSCRIBE

  int iii=0;
  while (arrivedcount > 0 && iii++ < REPEATWAIT)
    {
      LOGN("Waiting for QoS 2 message");
      client.yield(TIMEOUT);  
    }

  if (arrivedcount > 0 ){
    LOGN("message lost, Q0S 2: %d",arrivedcount);
  }
  
#endif
#endif
  
  LOGN("Total sent: %d Total error: %d",sentcount,errorcount);
 
  return true;
}


void logPrefix(Print* _logOutput) {
  char dt[DATE_TIME_STRING_LENGTH];
  snprintf(dt, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u", year(), month(), day(), hour(), minute(), second());  
  _logOutput->print("#");
  _logOutput->print(dt);
  _logOutput->print(" ");
}

void logSuffix(Print* _logOutput) {
  _logOutput->print('\n');
  //_logOutput->flush();  // we use this to flush every log message
}



void setup()
{
  Serial.begin(115200);
  Log.begin(LOG_LEVEL,&Serial);
  Log.setPrefix(logPrefix); // Uncomment to get timestamps as prefix
  Log.setSuffix(logSuffix); // Uncomment to get newline as suffix
  delay(2000);
  LOGN("MQTT test");

  while(!initmodem()){
    delay(5000);
  }

  while (!client.isConnected()){
    connect();
    delay(1000);
  }

  
}


void loop()
{ 
  if (publish()){
    LOGN("publish OK");
  }
}
