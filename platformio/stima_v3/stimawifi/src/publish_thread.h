#include <Countdown.h>
#include <WifiIPStack.h>
#include <MQTTClient.h>

/*!
\def MQTT_TIMEOUT_MS
\brief Timeout in milliseconds for mqtt stack.
*/
#define MQTT_TIMEOUT_MS                (6000)

/*!
\def IP_STACK_TIMEOUT_MS
\brief IPStack timeout.
*/
#define IP_STACK_TIMEOUT_MS            (MQTT_TIMEOUT_MS)

/*!
\def MQTT_PACKET_SIZE
\brief Length in bytes for max mqtt packet size.
*/
#define MQTT_PACKET_SIZE               (220)

# define MQTT_SERVER_PORT (1883)


/*!
\def CONSTANTDATA_BTABLE_LENGTH
\brief Maximum lenght of btable code plus terminator that describe one constant data.
*/
#define CONSTANTDATA_BTABLE_LENGTH                    (7)

/*!
\def CONSTANTDATA_VALUE_LENGTH
\brief Maximum lenght of value plus terminator for one constant data.
*/
#define CONSTANTDATA_VALUE_LENGTH                    (33)

#ifndef PUBLISH_THREAD_H_
#define PUBLISH_THREAD_H_


struct publish_data_t {
  int id;
  frtosLogging* logger;
  Queue* mqttqueue;
  publishStatus_t* status;
  station_t* station;
  WiFiClient* mqttClient;
};


using namespace cpp_freertos;

class publishThread : public Thread {

public:
  publishThread(publish_data_t &publish_data);
  ~publishThread();
  virtual void Cleanup();
  
 protected:  
  virtual void Run();
    
 private:
  publish_data_t data;
  IPStack ipstack;
  MQTT::Client<IPStack, Countdown, MQTT_PACKET_SIZE, 1 > mqttclient;
};

#endif
