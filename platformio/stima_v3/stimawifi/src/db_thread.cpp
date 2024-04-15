#include "common.h"

//uint8_t sqlite_memory[SQLITE_MEMORY];

static int callback(void* data, int argc, char **argv, char **azColName) {
  int i;
  for (i = 0; i<argc; i++){
    ((db_data_t *)data)->logger->notice(F("SQL result %s = %s"), azColName[i], argv[i] ? argv[i] : "NULL");
  }
  return 0;
}

int db_exec(sqlite3 *db, const char *sql,db_data_t& data) {
  char *zErrMsg = 0;
  data.logger->notice(F("SQL exec: %s"),sql);
  long start = micros();
  int rc = sqlite3_exec(db, sql, callback, &data, &zErrMsg);
  if (rc != SQLITE_OK) {
    data.logger->notice(F("SQL error: %s"), zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    data.logger->notice(F("SQL operation done successfully"));
  }
  data.logger->notice(F("SQL time taken: %l"),micros()-start);
  return rc;
}

void db_setup(sqlite3 *db, db_data_t& data){

  //sqlite3_hard_heap_limit64(SQLITE_MEMORY);

  if(db_exec(db,
	     "PRAGMA user_version = 1;"
	     ,data) != SQLITE_OK) {
    data.logger->error(F("pragma user_version"));       
  }
  
  if(db_exec(db,
	     "PRAGMA temp_store = FILE;"
	     ,data) != SQLITE_OK) {
    data.logger->error(F("pragma temp_store"));       
  }
  
  if(db_exec(db,
	     "PRAGMA synchronous = FULL;"
	     ,data) != SQLITE_OK) {
    data.logger->error(F("pragma synchronous"));       
  }

  if(db_exec(db,
	     "PRAGMA secure_delete = FALSE;"
	     ,data) != SQLITE_OK) {
    data.logger->error(F("pragma secure_delete"));       
  }

  if(db_exec(db,
	     "PRAGMA locking_mode = EXCLUSIVE;"
	     ,data) != SQLITE_OK) {
    data.logger->error(F("pragma locking_mode"));       
  }

  if(db_exec(db,
	     "PRAGMA journal_size_limit = 10000 ;"
	     ,data) != SQLITE_OK) {
    data.logger->error(F("pragma journal_size_limit"));       
  }

  if(db_exec(db,
	     "PRAGMA journal_mode = TRUNCATE;"
	     ,data) != SQLITE_OK) {
    data.logger->error(F("pragma journal_mode"));       
  }
}

void data_recovery(sqlite3 *db, db_data_t& data){
  int rc;
  mqttMessage_t message;

  data.logger->notice(F("Start recovery from DB"));       

  if (data.mqttqueue->NumSpacesLeft() < MQTT_QUEUE_SPACELEFT_RECOVERY){
    data.logger->warning(F("no space in publish queue while recovery data from DB"));   
    return;
  }

  char sql[] = "SELECT sent,topic,payload  FROM messages WHERE sent = 0 ORDER BY datetime LIMIT ?";
    
  sqlite3_stmt* stmt; // will point to prepared stamement object
  rc=sqlite3_prepare_v2(
			db,             // the handle to your (opened and ready) database
			sql,            // the sql statement, utf-8 encoded
			strlen(sql),    // max length of sql statement
			&stmt,          // this is an "out" parameter, the compiled statement goes here
			nullptr);       // pointer to the tail end of sql statement (when there are 
                                        // multiple statements inside the string; can be null)
  if(rc) {
    data.logger->error(F("select recovery data from DB"));   
  } else {


  sqlite3_bind_int(
		   stmt,                  // previously compiled prepared statement object
		   1,                     // parameter index, 1-based
		   (int)MQTT_QUEUE_BURST_RECOVERY);    // the data
    
    while(1) {
      // fetch a row's status
      //CriticalSection::SuspendScheduler();
      rc = sqlite3_step(stmt);      
      //CriticalSection::ResumeScheduler();
      
      if(rc == SQLITE_ROW) {
	message.sent = (uint8_t)sqlite3_column_int(stmt, 0);
	strcpy(message.topic, (const char*)sqlite3_column_text(stmt, 1));
	strcpy(message.payload, (const char*)sqlite3_column_text(stmt, 2));
	
	data.logger->notice(F("Message recovery enqueue for publish %s:%s"),message.topic,message.payload);       
	if(!data.mqttqueue->Enqueue(&message,pdMS_TO_TICKS(0))){
	  data.logger->error(F("Message recovery recovery for publish : %s ; %s"),  message.topic, message.payload);
	}
      } else if(rc == SQLITE_DONE) {
	break;
      } else {
	data.logger->error(F("getting values from DB: %s"),sqlite3_errmsg(db));       
	break;
      }
    }
    sqlite3_finalize(stmt);
  }
}

bool doDb(sqlite3 *db, db_data_t& data, const mqttMessage_t& message) {

  int rc;
  char sql[] = "INSERT OR REPLACE INTO messages VALUES (?, strftime('%s',?), ?, ?)";
  sqlite3_stmt* stmt; // will point to prepared stamement object

  data.logger->notice(F("spaceleft in db queue %d"),data.mqttqueue->NumSpacesLeft());   
  
  sqlite3_prepare_v2(
		     db,             // the handle to your (opened and ready) database
		     sql,            // the sql statement, utf-8 encoded
		     strlen(sql),    // max length of sql statement
		     &stmt,          // this is an "out" parameter, the compiled statement goes here
		     nullptr);       // pointer to the tail end of sql statement (when there are 
                                     // multiple statements inside the string; can be null)
  
  char dt[20];   //"YYYY-MM-GGTHH:MM:SS";
  char tmppayload[MQTT_MESSAGE_LENGTH];

  strncpy(tmppayload, message.payload, MQTT_MESSAGE_LENGTH);

  StaticJsonDocument<32> doc;
  DeserializationError jserror = deserializeJson(doc,tmppayload,MQTT_MESSAGE_LENGTH);

  if (!jserror) {
    if (doc.containsKey("t")){
      strncpy(dt, doc["t"],20);
    }else{
      data.logger->error(F("datetime missed in payload"));      
      return true;
    }
  } else {
    data.logger->error(F("failed to deserialize payload json %s"),jserror.c_str());
    return true;
  }
  
  sqlite3_bind_int(
		   stmt,                  // previously compiled prepared statement object
		   1,                     // parameter index, 1-based
		   (int)message.sent);    // the data
  
  sqlite3_bind_text(
		    stmt,                  // previously compiled prepared statement object
		    2,                     // parameter index, 1-based
		    dt,                    // the data
		    -1,                    // length of data
		    SQLITE_STATIC);        // this parameter is a little tricky - it's a pointer to the callback
                                           // function that frees the data after the call to this function.
                                           // It can be null if the data doesn't need to be freed, or like in this case,
                                           // special value SQLITE_STATIC (data will be freed automatically).
					  
  sqlite3_bind_text(stmt, 3, message.topic,   -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 4, message.payload, -1, SQLITE_STATIC);

  //CriticalSection::SuspendScheduler();
  rc = sqlite3_step(stmt); // you'll want to check the return value, read on...
  //CriticalSection::ResumeScheduler();
  
  if (rc != SQLITE_DONE) {
    data.status->database=error;
    data.logger->error(F("insert values in DB: %s"),sqlite3_errmsg(db));       
    // close SDcard
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    SD.end();

    //restart SDcard
    SPI.begin(C3SCK, C3MISO, C3MOSI, C3SS); //SCK, MISO, MOSI, SS
    SPI.setDataMode(SPI_MODE0);
    if (SD.begin(C3SS,SPI,400000, "/sd",MAXFILE, false)){
      if (sqlite3_open("/sd/stima.db", &db)!=SQLITE_OK){
	// close open things
	sqlite3_close(db);
	SD.end();
	data.logger->error(F("DB open"));
	data.status->database=error;
	return false;      //terminate
      }else{
	db_setup(db,data);
      }
    }else{
      SD.end();
      data.logger->error(F("SD card open"));      
      data.status->database=error;
      return false;       //terminate
    }
    // go for retry
    return true;
  }
  
  sqlite3_finalize(stmt);

  data.status->database=ok;  
  data.logger->notice(F("Data saved on SD %s:%s"),message.topic,message.payload);       

  mqttMessage_t tmpmsg;
  data.dbqueue->Dequeue(&tmpmsg, pdMS_TO_TICKS( 0 ));  // all done: dequeue the message
    
  return true;
}    


using namespace cpp_freertos;

dbThread::dbThread(db_data_t& db_data)
  : Thread{"DB", 5000, 3}
    ,data{db_data}
{
  //data->logger->notice("Create Thread %s %d", GetName().c_str(), data->id);
  data.status->database=unknown;
  //Start();
};

dbThread::~dbThread()
{
}
  
void dbThread::Cleanup()
{
  data.logger->notice("Delete Thread %s %d", GetName().c_str(), data.id);
  data.status->database=unknown;
  sqlite3_close(db);  
  delete this;
}

void dbThread::Run() {
  data.logger->notice("Starting Thread %s %d", GetName().c_str(), data.id);

  SPI.begin(C3SCK, C3MISO, C3MOSI, C3SS); //SCK, MISO, MOSI, SS
  SPI.setDataMode(SPI_MODE0);
  //bool begin(uint8_t ssPin=SS, SPIClass &spi=SPI, uint32_t frequency=4000000, const char * mountpoint="/sd", uint8_t max_files=5, bool format_if_empty=false)
  if (SD.begin(C3SS,SPI,4000000, "/sd",MAXFILE, true)){
    data.logger->notice(F("SD mount OK"));
  }else{
    data.logger->error(F("SD mount"));
    //data.status->database=error;
    return;
  }

  data.logger->notice(F("largest free block %l"),heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
  void* sqlite_memory= malloc(SQLITE_MEMORY);
  if (sqlite_memory == 0){
    data.logger->error(F("sqlite3 memory malloc"));
    data.status->database=error;
    return;    
  }
  
  if (sqlite3_config(SQLITE_CONFIG_HEAP, sqlite_memory, SQLITE_MEMORY, 32)!=SQLITE_OK){
    data.logger->error(F("sqlite3_config: %s"),sqlite3_errmsg(db));
    data.status->database=error;
    return;
  }

  if(sqlite3_initialize()!=SQLITE_OK){
    data.logger->error(F("sqlite3_initialize"));
    data.status->database=error;
    return;
  }

  /*
  if (LittleFS.exists("/stima.db")) {
    LittleFS.remove("/stima.db");
  }
    
  if (sqlite3_open("/littlefs/stima.db", &db)){
    data.logger->error(F("DB open"));
    return;
  }
  */

  if (!(sqlite3_open("/sd/stima.db", &db)==SQLITE_OK)){
    data.logger->error(F("DB open"));
    data.status->database=error;
    return;
  }
  
  if(db_exec(db,
	     "CREATE TABLE IF NOT EXISTS messages ( sent INT NOT NULL, datetime INT NOT NULL, topic TEXT NOT NULL , payload TEXT NOT NULL, PRIMARY KEY(datetime,topic))"
	     ,data) != SQLITE_OK) {
    //sqlite3_close(db);
    //SD.end();
    //return;
  }

  if(db_exec(db, "CREATE INDEX ts ON messages(datetime)",data) != SQLITE_OK) {
    //sqlite3_close(db);
    //SD.end();
    //return;
  }
  
  if(db_exec(db, "CREATE INDEX status ON messages(sent)",data) != SQLITE_OK) {
    //sqlite3_close(db);
    //SD.end();
    //return;
  }

  db_setup(db,data);

  /*
  //int rc = db_exec(db, "SELECT datetime(datetime,'unixepoch'),topic,payload  FROM messages",data);
  int rc = db_exec(db, "SELECT sent,topic,payload  FROM messages WHERE sent = 0 ORDERED BY datetime",data);
  if (rc != SQLITE_OK) {
    data.logger->error(F("select all to be sent in DB"));   
  }
  */
 
  int rc;
  mqttMessage_t message;

  data.logger->notice(F("Total number of record in DB"));       
  rc = db_exec(db, "SELECT COUNT(*) FROM messages",data);
  if (rc != SQLITE_OK) {
    data.logger->error(F("select all in DB"));   
  }

  data.logger->notice(F("Total number of record in DB to send"));       
  rc = db_exec(db, "SELECT COUNT(*) FROM messages WHERE sent = 0",data);
  if (rc != SQLITE_OK) {
    data.logger->error(F("select to send in DB"));
  }
    
  data.status->database=ok;
  
  for(;;){

    while (data.dbqueue->Peek(&message, pdMS_TO_TICKS( 1000 ))){
      if (!doDb(db,data,message)) return;

      if(db_exec(db, "PRAGMA shrink_memory;",data) != SQLITE_OK) {
	data.logger->error(F("pragma shrink_memory"));       
      }
    }

    if(data.recoverysemaphore->Take(0)) data_recovery(db, data);
    
    //data.logger->error("stack db: %d",uxTaskGetStackHighWaterMark(NULL));
    if ( uxTaskGetStackHighWaterMark(NULL) < 100 ) data.logger->error(F("stack db"));
  }
};  
