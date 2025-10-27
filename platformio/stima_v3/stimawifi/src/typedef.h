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

/*!
\def MAX_CONSTANTDATA_COUNT
\brief Numero massimo di dati costanti di stazione (metadati).
*/
#define MAX_CONSTANTDATA_COUNT                       (5)


/*!
\def struct summarydata_t
\brief Dati di sintesi utili per la visualizzazione su display.
*/
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

/*!
\def struct constantdata_t
\brief Descrittori e valori per i dati costanti di stazione (metadati).
*/
typedef struct {
   char btable[CONSTANTDATA_BTABLE_LENGTH];                 //!< table B code for constant station data
   char value[CONSTANTDATA_VALUE_LENGTH];                   //!< value of constant station data
} constantdata_t;

/*!
\def struct georef_t
\brief Dati per la georeferenziazione.
*/
struct georef_t
{
  char lon[11];                                     //!< longitudine in sessadecimale 5 decimali rappresentazione intera
  char lat[11];                                     //!< latitudine in sessadecimale 5 decimali rappresentazione intera
  time_t timestamp;                                 //!< timestamp delle coordinate
  cpp_freertos::MutexStandard* mutex;               //!< mutex per l'accesso ai dati
};
  
/*!
\def struct station_t
\brief Metadati di stazione.
*/
struct station_t
{
  char longitude[11];                  //!< longitudine in sessadecimale 5 decimali rappresentazione intera
  char latitude[11];                   //!< latitudine in sessadecimale 5 decimali rappresentazione intera
  char network[31];                    //!< rete osservativa della stazione
  char ident[10];                      //!< identificativo per stazioni mobili
  char server[41];                     //!< server RMAP
  char ntp_server[41];                 //!< server NTP
  char mqtt_server[41];                //!< broker MQTT
  int  sampletime;                     //!< intervallo tra le misurazioni in secondi
  char user[10];                       //!< utente
  char password[31];                   //!< password
  char stationslug[31];                //!< nome sintetico della stazione
  char boardslug[31];                  //!< nome sintetico della board
  char mqttrootpath[10];               //!< radice del topic MQTT per i dati
  char mqttmaintpath[10];              //!< radice del topic MQTT per i dati amministrativi
  char mqttrpcpath[10];                //!< radice del topic MQTT per le RPC
  constantdata_t constantdata[MAX_CONSTANTDATA_COUNT];     //!< Constantdata buffer for storing constant station data parameter (metadati)
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
  strcpy(mqttrpcpath,"rpc");
  constantdata_count=0;
  }
};

/*!
\def struct sensor_t
\brief Metadati dei sensori.
*/
struct sensor_t
{
  char driver[SENSORDRIVER_DRIVER_LEN];     // driver name
  int node;                                 // RF24Nework node id (radio driver)
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

/*!
\def struct mqttMessage_t
\brief dati per la pubblicazione MQTT (topic e payload).
*/
struct mqttMessage_t
{
  uint8_t sent;
  char topic[MQTT_ROOT_TOPIC_LENGTH+MQTT_SENSOR_TOPIC_LENGTH];
  char payload[MQTT_MESSAGE_LENGTH];
};

/*!
\def enum status_e
\brief Le varie operazioni vengono monitore e desscitte tramite uno stato
\ queste sono le varie condizioni possibili
*/
enum status_e { unknown, ok, error };

/*!
\def struct measureStatus_t
\brief Stati relativi al thread di misura dei dati.
*/
struct measureStatus_t
{
  status_e novalue;    //!< stato delle misurazioni
  status_e sensor;     //!< stato dei sensori
  status_e geodef;     //!< stato dei dati di georefenzazione per la pubblicazione
  status_e memory_collision;     //!< check collisione stack e heap
  status_e no_heap_memory;       //!< no memory for allocation in heap
};

/*!
\def struct publishStatus_t
\brief Stati relativi al thread di pubblicazione dei dati.
*/
struct publishStatus_t
{
  status_e connect;    //!< stato della connessione MQTT (errore dopo ripetuti tentativi)
  status_e publish;    //!< stato della pubblicazione dati MQTT (errore con la coda di oubblicazione piena)
  status_e memory_collision;     //!< check collisione stack e heap
  status_e no_heap_memory;       //!< no memory for allocation in heap
};

/*!
\def struct udpStatus_t
\brief Stati relativi al thread di ricezione dei dati UDP per la georefenziazione.
*/
struct udpStatus_t
{
  status_e receive;    //!< stato della ricezione dei dati per georeferenziazione  
  status_e memory_collision;     //!< check collisione stack e heap
  status_e no_heap_memory;       //!< no memory for allocation in heap
};

/*!
\def struct gpsStatus_t
\brief Stati relativi al thread di ricezione dei dati GPS (porta seriale) per la georefenziazione.
*/
struct gpsStatus_t
{
  status_e receive;    //!< stato della ricezione dei dati per georeferenziazione  
  status_e memory_collision;     //!< check collisione stack e heap
  status_e no_heap_memory;       //!< no memory for allocation in heap
};

/*!
\def struct dbStatus_t
\brief Stati relativi al thread di gestione del DataBase.
*/
struct dbStatus_t
{
  status_e sdcard;      //!< stato di funzionamento del'SD card
  status_e database;    //!< stato di funzionamento del DataBase
  status_e archive;     //!< stato di funzionamento dell'archivio su SDcard
  status_e memory_collision;     //!< check collisione stack e heap
  status_e no_heap_memory;       //!< no memory for allocation in heap
};

/*!
\def struct summaryStatus_t
\brief Sommario degli errori di tutta la stazione.
*/
struct summaryStatus_t
{
  bool err_reboot;        //!< si è verificato un reboot non programmato (non dovuto a RPC, riavvio periodico, aggiornamento firmware)
  bool err_power_on;      //!< alla stazione è stata tolta l'alimentazione e poi rialimentata
  bool err_georef;        //!< se la stazione è mobile errore dal GPS o nella comunicazione UDP
  bool err_sdcard;        //!< errore sulla SDcard
  bool err_db;            //!< errore del DB sqlite (dati recenti)
  bool err_archive;       //!< errore sull'archivio (dati storici)
  bool err_mqtt_publish;  //!< la coda di pubblicazione dei dati MQTT si è riempita e i dati inviati direttamente al DB
  bool err_mqtt_connect;  //!< dopo numerosi tentativi non si è riusciti a connettersi al broker MQTT
  bool err_geodef;        //!< se la stazione è mobile non è stato possibile georeferenziare i dati
  bool err_sensor;        //!< errore lettura dati dai sensori
  bool err_novalue;       //!< errore almeno un sensore ha riportato almeno un dato mancante
  bool err_rtc;           //!< errore nell'ottenere data e ora dall'RTC
  bool err_memory;        //!< errore nella gestione della memoria
  bool err_rssi;          //!< errore segnale WiFi (RSSI < 50)
};

/*!
\def struct stimawifiStatus_t
\brief Stati relativi all'intera stazione.
*/
struct stimawifiStatus_t
{
  measureStatus_t measure;     //!< Stati relativi al thread di misura
  publishStatus_t publish;     //!< Stati relativi al thread di pubblicazione
  udpStatus_t udp;             //!< Stati relativi al thread di ricezione UDP dei dati di georeferenziazione
  gpsStatus_t gps;             //!< Stati relativi al thread di ricezione GPS (porta seriale) dei dati di georeferenziazione
  dbStatus_t db;               //!< Stati relativi al thread di gestione del DataBase
  status_e rtc;                //!< Stato dell'RTC
  summaryStatus_t summary;     //!< Sommario dello stato stazione che evidenzia eventuali errori nel periodo  
};


/*!
\def struct rpcRecovery_t
\brief params for recovery rpc
*/
struct rpcRecovery_t
{
  char dtstart[20];   //"YYYY-MM-GGTHH:MM:SS";
  char dtend[20];     //"YYYY-MM-GGTHH:MM:SS";
};

#endif
