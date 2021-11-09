#define TIMEOUT 6000L
#define MQTTCLIENT_QOS MQTT::QOS1
#define SIM800_ON_OFF_PIN     (5)
#define GSM_APN        ("ibox.tim.it")
#define GSM_USERNAME   ("")
#define GSM_PASSWORD   ("")
#define TOPICCOM "test/ident/com"
#define TOPICRES "test/ident/res"
#define HOSTNAME "test.rmap.cc"
#ifndef ARDUINO_ARCH_AVR
HardwareSerial Serial1(PB11, PB10);
#endif

#include <sim800Client.h>
#include <Sim800IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>

sim800Client s800;
IPStack ipstack(s800);
MQTT::Client<IPStack, Countdown, 120, 1> client = MQTT::Client<IPStack, Countdown, 120, 1>(ipstack,TIMEOUT);
bool send_response=false;

bool publish(const char* topic, const char* payload)
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

  Serial.println("Publish");  
  Serial.println("Send QoS 1 message:");

  Serial.println(payload);
  message.qos = MQTT::QOS1;  
  message.payload = payload;
  message.payloadlen = strlen(payload);
                                       
  rc = client.publish(topic, message);
  if (rc != 0)
  {
    Serial.print("rc from MQTT pubblish is ");
    Serial.println(rc);
    errorcount++;
  }

  return true;
}

void messageArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  
  Serial.print("Receive message ");
  Serial.print(" arrived: qos ");
  Serial.print(message.qos);
  Serial.print(", retained: ");
  Serial.print(message.retained);
  Serial.print(", dup: ");
  Serial.print(message.dup);
  Serial.print(", packetid: ");
  Serial.print(message.id);
  Serial.print(", Payload:");
  Serial.println((char*)message.payload);

  send_response=true;
}

bool initmodem(void)
{
  Serial.println("try to init sim800");
  if(s800.init(SIM800_ON_OFF_PIN)){
    Serial.println("initialized sim800");
  }else{
    Serial.println("error initializing sim800");
    return false;
  }
    
  while(true){
    int rc = s800.switchOn();
    if (rc == SIM800_OK) {
      delay(SIM800_WAIT_FOR_AUTOBAUD_DELAY_MS);
      Serial.println("switch on OK");
      break;
    }else if (rc == SIM800_ERROR){
      Serial.println("switch on failed");
      return false;
    }
  }

  while(true){
    int rc =s800.initAutobaud();
    if (rc == SIM800_OK) {
      delay(SIM800_WAIT_FOR_AUTOBAUD_DELAY_MS);
      Serial.println("autobaud OK");
      break;
    }else if (rc == SIM800_ERROR){
      Serial.println("autobaud failed");
      return false;
    }
  }

  while(true){
    int rc =s800.setup();
    if (rc == SIM800_OK) {
      Serial.println("setup OK");
      break;
    }else if (rc == SIM800_ERROR){
      Serial.println("setup failed");
      return false;
    }
  }
  

  while(true){
    int rc =s800.startConnection(GSM_APN,GSM_USERNAME,GSM_PASSWORD);
    if (rc == SIM800_OK) {
      Serial.println("start connection OK");
      break;
    }else if (rc == SIM800_ERROR){
      Serial.println("start connection failed");
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
  data.MQTTVersion = 4;
  data.clientID.cstring = (char*)"test-ID";
  //data.username.cstring = (char*)"ident";
  //data.password.cstring = (char*)"password";
  data.cleansession = true;
  
  int rc = client.connect(data);
  if (rc != 0)
  {
    Serial.print("error rc from MQTT connect is ");
    Serial.println(rc);
    return false;
  }else{
    Serial.println("MQTT connected");
  }

  rc = client.subscribe(TOPICCOM, MQTTCLIENT_QOS, messageArrived);   
  if (rc != 0)
  {
    Serial.print("rc from MQTT subscribe is ");
    Serial.println(rc);
    return false;
  }else{
    Serial.println("MQTT subscribed");
  }

  return true;
}


void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("MQTT Hello example");

  while(!initmodem()){
    delay(5000);
  }


  /*
  Serial.println("Disconnect");
  client.disconnect();
  ipstack.disconnect();
  delay(30000);
  */
  
}

void loop()
{

  while (!client.isConnected()){
    Serial.println("Connect");
    connect();
    delay(1000);
  }
  
  if(send_response){
    const char* payload="{\"jsonrpc\":\"2.0\",\"id\":0,\"result\":{\"state\":\"done\"}}";
    publish(TOPICRES,payload);
    send_response=false;
  }
  
  client.yield(1000LU);

  uint32_t pub_time=millis() / 10000UL;
  static uint32_t last_publish=0;
  if( pub_time > last_publish){
    last_publish=pub_time;
    Serial.println(pub_time);
    publish("test/ident/test","prova 1");
    publish("test/ident/test","prova 2");
    publish(TOPICCOM,"{\"jsonrpc\":\"2.0\",\"id\":0,\"params\":{}}");
    publish("test/ident/test","prova 3");

  }
}
