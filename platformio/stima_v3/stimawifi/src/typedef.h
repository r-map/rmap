#ifndef TYPEDEF_H_
#define TYPEDEF_H_

// sensor information
struct station_t
{
  char longitude[11];
  char latitude[11];
  char network[31];
  char server[41];
  char ntp_server[41];
  char mqtt_server[41];
  int  sampletime;
  char user[10];
  char password[31];
  char slug[31];
  char mqttrootpath[10];
  char mqttmaintpath[10];

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
  strcpy(slug, "stimawifi");
  strcpy(mqttrootpath, "sample");
  strcpy(mqttmaintpath,"maint");
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
  char topic[100];
  char payload[100];
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
