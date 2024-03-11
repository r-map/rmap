#ifndef TYPEDEF_H_
#define TYPEDEF_H_
#include <mqtt_config.h>
#include <mutex.hpp>

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

#define MAX_CONSTANTDATA_COUNT                       (5)


struct summarydata_t{
  float temperature;
  int humidity;
  int pm2;
  int pm10;
  int co2;
  summarydata_t() {
    temperature=NAN;
    humidity=-999;
    pm2=-999;
    pm10=-999;
    co2=-999;
  }
};

typedef struct {
   char btable[CONSTANTDATA_BTABLE_LENGTH];                 //!< table B code for constant station data
   char value[CONSTANTDATA_VALUE_LENGTH];                   //!< value of constant station data
} constantdata_t;


struct georef_t
{
  char lon[11];
  char lat[11];
  time_t timestamp;
  cpp_freertos::MutexStandard* mutex;
};
  
// sensor information
struct station_t
{
  char longitude[11];
  char latitude[11];
  char network[31];
  char ident[10];
  char server[41];
  char ntp_server[41];
  char mqtt_server[41];
  int  sampletime;
  char user[10];
  char password[31];
  char stationslug[31];
  char boardslug[31];
  char mqttrootpath[10];
  char mqttmaintpath[10];
  constantdata_t constantdata[MAX_CONSTANTDATA_COUNT];     //!< Constantdata buffer for storing constant station data parameter
  uint8_t constantdata_count;                              //!< configured constantdata number
  
  //define your default values here, if there are different values in config.json, they are overwritten.
  station_t() {
  longitude[0] = '\0';
  latitude[0] = '\0';
  network[0] = '\0';
  strcpy(server,"rmap.cc");
  strcpy(ntp_server, "europe.pool.ntp.org");
  strcpy(mqtt_server, "rmap.cc");
  sampletime = DEFAULT_SAMPLETIME;
  user[0] = '\0';
  password[0] = '\0';
  strcpy(stationslug, "stimawifi");
  strcpy(boardslug, "default");
  strcpy(mqttrootpath, "sample");
  strcpy(mqttmaintpath,"maint");
  constantdata_count=0;
  }
};

// sensor information
struct sensor_t
{
  char driver[SENSORDRIVER_DRIVER_LEN];     // driver name
  int node;                                 // RF24Nework node id
  char type[SENSORDRIVER_TYPE_LEN];         // sensor type name
  int address;                              // i2c address
  char timerange[SENSORDRIVER_META_LEN];    // timerange for mqtt pubblish
  char level [SENSORDRIVER_META_LEN];       // level for mqtt pubblish

  sensor_t() : address(-1) {
       driver[0]='\0';
       node = -1;
       type[0]='\0';
       timerange[0]='\0';
       level[0]='\0';
  }
};

struct mqttMessage_t
{
  char topic[MQTT_ROOT_TOPIC_LENGTH+MQTT_SENSOR_TOPIC_LENGTH];
  char payload[MQTT_MESSAGE_LENGTH];
};


//enum sensorNovalue_e { unknown, ok, error };
//enum sensorMeasure_e { unknown, ok, error };
enum status_e { unknown, ok, error };
struct measureStatus_t
{
  //sensorNovalue_e novalue;
  //sensorMeasure_e sensor;
  status_e novalue;
  status_e sensor;
  status_e geodef;
};

//enum mqttConnect_e { unknown, ok, error };
//enum mqttPublish_e { unknown, ok, error };
struct publishStatus_t
{
  //mqttConnect_e connect;
  //mqttPublish_e publish;
  status_e connect;
  status_e publish;
};

//enum udpReceive_e { unknown, ok, error };
struct udpStatus_t
{
  //udpreceive_e receive;
  status_e receive;
};

struct stimawifiStatus_t
{
  measureStatus_t measure;
  publishStatus_t publish;
  udpStatus_t udp;
};


#endif
