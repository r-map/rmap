#include "common.h"


// De-initialize/Reset SD Card
// https://github.com/greiman/SdFat/issues/351
void clearSD ()
{
  /*
  SPI.begin();
  SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
  SPI.transfer(0XFF);
  SPI.endTransaction();
  SPI.end()
  */
  
  byte sd = 0;
  digitalWrite(C3SS, LOW);
  while (sd != 0XFF)
  {
    SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
    sd = SPI.transfer(0XFF);
    SPI.endTransaction();
    //Serial.print("sd=");
    //Serial.println(sd);
  }
  digitalWrite(C3SS, HIGH);
}

// callback for data returned by sqlite
static int db_exec_callback(void* data, int argc, char **argv, char **azColName) {
  int i;
  for (i = 0; i<argc; i++){
    ((db_data_t *)data)->logger->notice(F("db SQL result %s = %s"), azColName[i], argv[i] ? argv[i] : "NULL");
  }
  return 0;
}

// execute one SQL sentence
int dbThread::db_exec( const char *sql) {
  char *zErrMsg = 0;
  data->logger->notice(F("db SQL exec: %s"),sql);
  long start = millis();
  int rc = sqlite3_exec(db, sql, db_exec_callback, data, &zErrMsg);
  if (rc != SQLITE_OK) {
    data->logger->notice(F("db SQL error: %s"), zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    data->logger->notice(F("db SQL operation done successfully"));
  }
  data->logger->notice(F("db SQL time taken: %l"),millis()-start);
  return rc;
}

// callback for data returned by sqlite obsolete sentence
static int db_obsolete_callback(void* result, int argc, char **argv, char **azColName) {
  time_t time=now() - SDRECOVERYTIME;
  //Serial.println(time);  
  //Serial.println(atoi(argv[0]));
  if (argv[0] != NULL ) {
    *(bool *)result = (atoi(argv[0]) < time);
  } else {
    *(bool *)result = false;
  }
  return 0;
}

// execute one obsolete SQL sentence
bool dbThread::db_obsolete(bool& obsolete) {
  char *zErrMsg = 0;
  char sql[] = "SELECT MAX(datetime) FROM messages";
  data->logger->notice(F("db SQL exec: %s"),sql);
  long start = millis();
  
  int rc = sqlite3_exec(db, sql, db_obsolete_callback, &obsolete, &zErrMsg);
  if (rc != SQLITE_OK) {
    data->logger->error(F("db SQL error: %s"), zErrMsg);
    sqlite3_free(zErrMsg);
    obsolete=false;
  } else {
    data->logger->notice(F("SQL time taken: %l"),millis()-start);
    data->logger->notice(F("db SQL operation done successfully"));
  }

  //  if (obsolete){
  //    rc=db_remove();

  //    data->logger->notice(F("DROP TABLE"));    
  //    rc = db_exec("DROP TABLE messages");

  //    if (rc == SQLITE_OK){
  //      rc = db_exec("CREATE TABLE IF NOT EXISTS messages ( sent INT NOT NULL, datetime INT NOT NULL, topic TEXT NOT NULL , payload TEXT NOT NULL, PRIMARY KEY(datetime,topic))");
  //    }

  //  }

  return rc == SQLITE_OK;
}

// setup the databse opened by sqlite
void dbThread::db_setup(){

  //sqlite3_hard_heap_limit64(SQLITE_MEMORY);

  // The user-version is an integer that is available to applications
  // to use however they want.
  if(db_exec("PRAGMA user_version = 1;") != SQLITE_OK) {
    data->logger->error(F("db pragma user_version"));       
  }
  
  //When temp_store is FILE (1) temporary tables and indices are
  //stored in a file.
  if(db_exec("PRAGMA temp_store = FILE;") != SQLITE_OK) {
    data->logger->error(F("db pragma temp_store"));
  }

  // When synchronous is FULL (2), the SQLite database engine will use
  // the xSync method of the VFS to ensure that all content is safely
  // written to the disk surface prior to continuing. This ensures
  // that an operating system crash or power failure will not corrupt
  // the database. FULL synchronous is very safe, but it is also
  // slower.
  if(db_exec("PRAGMA synchronous = FULL;") != SQLITE_OK) {
    data->logger->error(F("db pragma synchronous"));       
  }

  // When secure_delete is on, SQLite overwrites deleted content with
  // zeros.  Applications that wish to avoid leaving forensic traces
  // after content is deleted or updated should enable the
  // secure_delete
  if(db_exec("PRAGMA secure_delete = FALSE;") != SQLITE_OK) {
    data->logger->error(F("db pragma secure_delete"));       
  }

  // When the locking-mode is set to EXCLUSIVE, the database
  // connection never releases file-locks.
  if(db_exec("PRAGMA locking_mode = EXCLUSIVE;") != SQLITE_OK) {
    data->logger->error(F("db pragma locking_mode"));       
  }
  
  // The journal_size_limit pragma may be used to limit the size of
  // rollback-journal and WAL files left in the file-system after
  // transactions or checkpoints.
  if(db_exec("PRAGMA journal_size_limit = 1000000 ;") != SQLITE_OK) {
    data->logger->error(F("db pragma journal_size_limit"));       
  }

  // The TRUNCATE journaling mode commits transactions by truncating
  // the rollback journal to zero-length instead of deleting it. On
  // many systems, truncating a file is much faster than deleting the
  // file since the containing directory does not need to be changed.
  if(db_exec("PRAGMA journal_mode = DELETE;") != SQLITE_OK) {
    data->logger->error(F("db pragma journal_mode"));       
  }
}


// set data in specific datetime range as unset for recovery
// of messages for retrasmission

bool dbThread::data_purge(const bool flush=false, int nmessages=100){
  int rc;

  data->logger->notice(F("db Start purge"));       

  //The values emitted by the RETURNING clause are the values as seen
  //by the top-level DELETE, INSERT, or UPDATE statement and do not
  //reflect any subsequent value changes made by triggers. Thus, if
  //the database includes AFTER triggers that modifies some of the
  //values of each row inserted or updated, the RETURNING clause emits
  //the original values that are computed before those triggers run

  char sql[100];
  //char sql[] = "DELETE FROM messages WHERE datetime < strftime('%s',?)";
  //char sql[] = "DELETE FROM messages WHERE pk IN (SELECT pk FROM messages WHERE datetime < strftime('%s',?) LIMIT 100)";
  if (flush) {
    strcpy(sql, "SELECT sent,topic,payload FROM messages");
  } else {
    strcpy(sql, "DELETE FROM messages WHERE datetime < strftime('%s',?) RETURNING sent,topic,payload LIMIT ?");
  }
  sqlite3_stmt* stmt; // will point to prepared stamement object
  rc=sqlite3_prepare_v2(
			db,             // the handle to your (opened and ready) database
			sql,            // the sql statement, utf-8 encoded
			strlen(sql),    // max length of sql statement
			&stmt,          // this is an "out" parameter, the compiled statement goes here
			nullptr);       // pointer to the tail end of sql statement (when there are 
                                        // multiple statements inside the string; can be null)
  if(rc != SQLITE_OK) {
    data->logger->error(F("db purge prepare: %s"),sqlite3_errmsg(db));
  } else {

    if (!flush) {
      char dtstart[20];
      time_t time=now() - SDRECOVERYTIME;
      snprintf(dtstart,20,"%04u-%02u-%02uT%02u:%02u:%02u",
	       year(time), month(time), day(time),
	       hour(time), minute(time), second(time));

      data->logger->notice(F("db purge data in DB from: %s"),dtstart);

      sqlite3_bind_text(
			stmt,                  // previously compiled prepared statement object
			1,                     // parameter index, 1-based
			dtstart,               // the data
			-1,                    // length of data
			SQLITE_STATIC);        // this parameter is a little tricky - it's a pointer to the callback
                                               // function that frees the data after the call to this function.
                                               // It can be null if the data doesn't need to be freed, or like in this case,
                                               // special value SQLITE_STATIC (data will be freed automatically).

      sqlite3_bind_int(
		       stmt,                  // previously compiled prepared statement object
		       2,                     // parameter index, 1-based
		       (int)nmessages);       // the data

    }

    uint16_t recbuffer=0;
    while(1) {
      mqttMessage_t message;
      // fetch a row's status
      rc = sqlite3_step(stmt);
      if(rc == SQLITE_ROW) {
	memset(&message, 0, sizeof(message));
	message.sent = (uint8_t)sqlite3_column_int(stmt, 0);
	strcpy(message.topic, (const char*)sqlite3_column_text(stmt, 1));
	strcpy(message.payload, (const char*)sqlite3_column_text(stmt, 2));
	if (archiveFile) {
	  data->logger->trace(F("db try save message in archive %T:%s:%s"),message.sent,message.topic,message.payload);       
	  if (archiveFile.write((uint8_t *)&message,sizeof(mqttMessage_t)/sizeof(uint8_t))) {
	    data->logger->trace(F("db saved message in archive"));       
	    if (recbuffer > 1000){
	      archiveFile.flush();
	      data->logger->notice(F("db flush messages to archive file"));       
	      recbuffer=0;
	    }
	    recbuffer++;
	  } else {
	    data->logger->error(F("db save message in archive %T:%s:%s"),message.sent,message.topic,message.payload);
	  }
	} else {
	  data->logger->error(F("db skip message for archive %s:%s"),message.topic,message.payload);       
	}
	
      } else if(rc == SQLITE_DONE) {
	rc  = SQLITE_OK;
	break;
      } else {
	data->logger->error(F("db purge getting values from DB: %s"),sqlite3_errmsg(db));       
	break;
      }
    }
  }

  archiveFile.flush();
  sqlite3_finalize(stmt);

  if (flush) {
    if(!db_remove()) data->logger->error("db removing old DB");
  }
  
  data->logger->notice(F("db purge values in DB Ended"));        
  return rc  == SQLITE_OK;
}


// set data in specific datetime range as unset for recovery
// of messages for retrasmission
bool dbThread::data_set_recovery(void){
  int rc;

  data->logger->notice(F("db set recovery started: %s ; %s"), rpcrecovery.dtstart, rpcrecovery.dtend);       

  char sql[] = "UPDATE messages SET sent = false WHERE datetime BETWEEN  strftime('%s',?) and strftime('%s',?)";
  //char sql[] = "UPDATE messages SET sent = false WHERE datetime strftime('%s',?))";
    
  sqlite3_stmt* stmt; // will point to prepared stamement object
  rc=sqlite3_prepare_v2(
			db,             // the handle to your (opened and ready) database
			sql,            // the sql statement, utf-8 encoded
			strlen(sql),    // max length of sql statement
			&stmt,          // this is an "out" parameter, the compiled statement goes here
			nullptr);       // pointer to the tail end of sql statement (when there are 
                                        // multiple statements inside the string; can be null)
  if(rc != SQLITE_OK) {
    data->logger->error(F("db set recovery prepare"));
  } else {

    sqlite3_bind_text(
		      stmt,                  // previously compiled prepared statement object
		      1,                     // parameter index, 1-based
		      rpcrecovery.dtstart,   // the data
		      -1,                    // length of data
		      SQLITE_STATIC);        // this parameter is a little tricky - it's a pointer to the callback
                                             // function that frees the data after the call to this function.
                                             // It can be null if the data doesn't need to be freed, or like in this case,
                                             // special value SQLITE_STATIC (data will be freed automatically).

    sqlite3_bind_text(
		      stmt,                  // previously compiled prepared statement object
		      2,                     // parameter index, 1-based
		      rpcrecovery.dtend,     // the data
		      -1,                    // length of data
		      SQLITE_STATIC);        // this parameter is a little tricky - it's a pointer to the callback
                                             // function that frees the data after the call to this function.
                                             // It can be null if the data doesn't need to be freed, or like in this case,
                                             // special value SQLITE_STATIC (data will be freed automatically).    
    rc = sqlite3_step(stmt);
    
    if(rc == SQLITE_DONE) {
      rc  = SQLITE_OK;
    } else {
      data->logger->error(F("db set recovery updating values in DB: %s"),sqlite3_errmsg(db));       
    }
  }
  sqlite3_finalize(stmt);
  data->logger->notice(F("db End set values in DB"));        
  return rc  == SQLITE_OK;
}

// when activated scan the DB for unsent messages and schedule a burst
// of messages for retrasmission
bool dbThread::data_recovery(void){
  int rc;
  mqttMessage_t message;

  data->logger->notice(F("db recovery from DB started"));       

  //if(data->status->publish != ok)
  //  data->logger->warning(F("db recovery skip: publish task KO"));   
  //  return false;
  //}
  
  if (data->mqttqueue->NumSpacesLeft() < MQTT_QUEUE_SPACELEFT_RECOVERY){
    data->logger->warning(F("db recovery no space in mqtt queue"));   
    return true;
  }

  char dtstart[20];
  time_t time=now()- SDRECOVERYTIME;
  snprintf(dtstart,20,"%04u-%02u-%02uT%02u:%02u:%02u",
	   year(time), month(time), day(time),
	   hour(time), minute(time), second(time));
  
  // select DATA_BURST_RECOVERY unsent messages
  //char sql[] = "SELECT sent,topic,payload  FROM messages WHERE sent = 0 AND datetime > strftime('%s',?) ORDER BY datetime LIMIT ?";
  char sql[] = "SELECT sent,topic,payload  FROM messages WHERE sent = 0 AND datetime > strftime('%s',?) LIMIT ?";
    
  sqlite3_stmt* stmt; // will point to prepared stamement object
  rc=sqlite3_prepare_v2(
			db,             // the handle to your (opened and ready) database
			sql,            // the sql statement, utf-8 encoded
			strlen(sql),    // max length of sql statement
			&stmt,          // this is an "out" parameter, the compiled statement goes here
			nullptr);       // pointer to the tail end of sql statement (when there are 
                                        // multiple statements inside the string; can be null)
  if(rc != SQLITE_OK) {
    data->logger->error(F("db recovery prepare select data from DB: %s"),sqlite3_errmsg(db));   
  } else {


    sqlite3_bind_text(
		      stmt,                  // previously compiled prepared statement object
		      1,                     // parameter index, 1-based
		      dtstart,               // the data
		      -1,                    // length of data
		      SQLITE_STATIC);
    
    sqlite3_bind_int(
		     stmt,                  // previously compiled prepared statement object
		     2,                     // parameter index, 1-based
		     (int)DATA_BURST_RECOVERY);    // the data
    
    while(1) {
      // fetch a row's status
      rc = sqlite3_step(stmt);
      if(rc == SQLITE_ROW) {
	message.sent = (uint8_t)sqlite3_column_int(stmt, 0);
	strcpy(message.topic, (const char*)sqlite3_column_text(stmt, 1));
	strcpy(message.payload, (const char*)sqlite3_column_text(stmt, 2));
	
	data->logger->notice(F("db recovery queuing message for publish %s:%s"),message.topic,message.payload);       
	if(!data->mqttqueue->Enqueue(&message,pdMS_TO_TICKS(0))){
	  data->logger->warning(F("db recovery message for publish not queued"));
	}
      } else if(rc == SQLITE_DONE) {
	rc  = SQLITE_OK;
	break;
      } else {
	data->logger->error(F("db recovery getting values from DB: %s"),sqlite3_errmsg(db));       
	break;
      }
    }
    sqlite3_finalize(stmt);
  } 
  data->logger->notice(F("db End recovery from DB"));        
  return rc == SQLITE_OK;
}


bool dbThread::db_remove(){

  // close sqlite3 DB, SDcard
  data->logger->notice(F("db close DB"));       
  sqlite3_close_v2(db);

  data->logger->notice(F("db remove DB from SDcard"));       
  SD.remove("/stima.db");

  if (sqlite3_open("/sd/stima.db", &db)==SQLITE_OK){
    data->logger->notice(F("db DB begin OK"));       
    // create table
    db_exec("CREATE TABLE IF NOT EXISTS messages ( sent INT NOT NULL, datetime INT NOT NULL, topic TEXT NOT NULL , payload TEXT NOT NULL, PRIMARY KEY(datetime,topic))");

    // create index on datetime
    db_exec("CREATE INDEX ts ON messages(datetime)");

    // create index on sent
    db_exec("CREATE INDEX status ON messages(sent)");

    // setup for the DB
    db_setup();
    return true;
  } else {
    data->logger->error(F("db DB open: %s"),sqlite3_errmsg(db));       
    // close open things
    sqlite3_close_v2(db);
    SD.end();
    data->status->database=error;
    return false;
  }
}



// re/start SDcard and sqlite management
// https://www.reddit.com/r/esp32/comments/qilboy/is_there_a_way_to_reset_a_microsd_card_from/
// https://forum.arduino.cc/t/resolved-how-can-i-reboot-an-sd-card/349700
bool dbThread::db_restart(){

  // close sqlite3 DB, SDcard
  data->logger->error(F("db close DB and SDcard"));       
  sqlite3_close_v2(db);
  sqlite3_shutdown();
  SD.end();

  SPI.begin(C3SCK, C3MISO, C3MOSI, C3SS); //SCK, MISO, MOSI, SS
  SPI.setDataMode(SPI_MODE0);
  bool s = SD.begin(C3SS,SPI,SPICLOCK, "/sd",SDMAXFILE, false);
  if (!s){
    data->logger->error(F("db SD begin"));       
    SD.end();
    delay(1000);
    clearSD();
    delay(1000);
    s = SD.begin(C3SS,SPI,SPICLOCK, "/sd",SDMAXFILE, false);
  }

  data->logger->notice(F("db SD begin OK"));       

  // sqlite use static allocated memory
  if (sqlite3_config(SQLITE_CONFIG_HEAP, sqlite_memory, SQLITE_MEMORY, 32)!=SQLITE_OK){
    data->logger->error(F("db sqlite3_config: %s"),sqlite3_errmsg(db));
    data->status->database=error;
    return false;
  }

  //initialize sqlite
  if(sqlite3_initialize()!=SQLITE_OK){
    data->logger->error(F("db sqlite3_initialize"));
    data->status->database=error;
    return false;
  }

  if (sqlite3_open("/sd/stima.db", &db)==SQLITE_OK){
    data->logger->notice(F("db DB begin OK"));       
    // setup for the DB
    db_setup();
    return true;
  }else{
    data->logger->error(F("db DB open: %s"),sqlite3_errmsg(db));       
    // close open things
    sqlite3_close_v2(db);
    SD.end();
    data->status->database=error;
    return false;
  }
}


// insert or replace a record in DB
bool dbThread::doDb(const mqttMessage_t& message) {
  
  int rc;
  char sql[] = "INSERT OR REPLACE INTO messages VALUES (?, strftime('%s',?), ?, ?)";
  sqlite3_stmt* stmt; // will point to prepared stamement object

  data->logger->notice(F("db spaceleft in mqtt queue %d"),data->mqttqueue->NumSpacesLeft());   
  
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
      data->logger->error(F("db datetime missed in payload"));
      return true;
    }
  } else {
    data->logger->error(F("db failed to deserialize payload json %s"),jserror.c_str());
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
    data->status->database=error;
    data->logger->error(F("db inserting values in DB: %s"),sqlite3_errmsg(db));       

    sqlite3_finalize(stmt);
    sqlite_status=false;
    return true;   // go for retry
  }

  sqlite3_finalize(stmt);
  sqlite_status=true;
  data->status->database=ok;  
  data->logger->notice(F("db Data saved on SD %s:%s"),message.topic,message.payload);       

  mqttMessage_t tmpmsg;
  data->dbqueue->Dequeue(&tmpmsg, pdMS_TO_TICKS( 0 ));  // all done: dequeue the message
    
  return true;
}    


using namespace cpp_freertos;

dbThread::dbThread(db_data_t* db_data)
  : Thread{"DB", 5500, 1}
    ,data{db_data}
{
  //data->logger->notice("Create Thread %s %d", GetName().c_str(), data->id);
  data->status->database=unknown;
  sqlite_status=false;
  //Start();
};

dbThread::~dbThread()
{
}
  
void dbThread::Cleanup()
{
  data->logger->notice("Delete Thread %s %d", GetName().c_str(), data->id);
  data->status->database=unknown;
  sqlite3_close(db);
  archiveFile.close();
  delete this;
}

void dbThread::Run() {
  data->logger->notice("Starting Thread %s %d", GetName().c_str(), data->id);

  if (SD.cardType() == CARD_NONE) {
    SPI.begin(C3SCK, C3MISO, C3MOSI, C3SS); //SCK, MISO, MOSI, SS
    SPI.setDataMode(SPI_MODE0);
    //bool begin(uint8_t ssPin=SS, SPIClass &spi=SPI, uint32_t frequency=SPICLOCK, const char * mountpoint="/sd", uint8_t max_files=5, bool format_if_empty=false)
    if (SD.begin(C3SS,SPI,SPICLOCK, "/sd",SDMAXFILE, true)){
      data->logger->notice(F("db SD mount OK"));
    }else{
      data->logger->error(F("db SD mount"));
      //data->status->database=error;
      return;     // without SD card terminate the thread
    }
  }

  // open and write info.dat file
  File infoFile = SD.open("/info.dat", FILE_WRITE);
  if (!infoFile){
    data->logger->error(F("db failed to open info file for writing"));
  }

  infoFile.print(data->station->user);
  infoFile.print("/");
  infoFile.print(data->station->stationslug);
  infoFile.print("/");
  infoFile.println(data->station->boardslug);
  infoFile.println(MAJOR_VERSION);
  infoFile.println(MINOR_VERSION);
  infoFile.println("");
  infoFile.println(MQTT_ROOT_TOPIC_LENGTH+MQTT_SENSOR_TOPIC_LENGTH);
  infoFile.println(MQTT_MESSAGE_LENGTH);
  infoFile.println(1);
  infoFile.close();
  
  // open archive on sd card
  archiveFile = SD.open(SDCARD_ARCHIVE_FILE_NAME, FILE_APPEND);
  if (!archiveFile){
    data->logger->error(F("db failed to open archive file for appending"));
  }
  
  /*
  data->logger->notice(F("largest free block %l"),heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
  void* sqlite_memory= malloc(SQLITE_MEMORY);
  if (sqlite_memory == 0){
    data->logger->error(F("sqlite3 memory malloc"));
    data->status->database=error;
    return;    
  }
  */

  // sqlite use static allocated memory
  if (sqlite3_config(SQLITE_CONFIG_HEAP, sqlite_memory, SQLITE_MEMORY, 32)!=SQLITE_OK){
    data->logger->error(F("db sqlite3_config: %s"),sqlite3_errmsg(db));
    data->status->database=error;
    return;
  }

  //initialize sqlite
  if(sqlite3_initialize()!=SQLITE_OK){
    data->logger->error(F("db sqlite3_initialize"));
    data->status->database=error;
    return;
  }

  /*   if use LittleFS

  if (LittleFS.exists("/stima.db")) {
    LittleFS.remove("/stima.db");
  }
    
  if (sqlite3_open("/littlefs/stima.db", &db)){
    data->logger->error(F("DB open"));
    return;
  }
  */

  // open DB
  if ((sqlite3_open("/sd/stima.db", &db)!=SQLITE_OK)){
    data->logger->error(F("db DB open"));
    data->status->database=error;
    return;
  }

  // create table
  if(db_exec("CREATE TABLE IF NOT EXISTS messages ( sent INT NOT NULL, datetime INT NOT NULL, topic TEXT NOT NULL , payload TEXT NOT NULL, PRIMARY KEY(datetime,topic))"
	     ) != SQLITE_OK) {
    //sqlite3_close(db);
    //SD.end();
    //return;
  }

  // create index on datetime
  if(db_exec("CREATE INDEX ts ON messages(datetime)") != SQLITE_OK) {
    //sqlite3_close(db);
    //SD.end();
    //return;
  }

  // create index on sent
  if(db_exec("CREATE INDEX status ON messages(sent)") != SQLITE_OK) {
    //sqlite3_close(db);
    //SD.end();
    //return;
  }

  // setup for the DB
  db_setup();

  /*
  //int rc = db_exec(db, "SELECT datetime(datetime,'unixepoch'),topic,payload  FROM messages",data);
  int rc = db_exec(db, "SELECT sent,topic,payload  FROM messages WHERE sent = 0 ORDERED BY datetime",data);
  if (rc != SQLITE_OK) {
    data->logger->error(F("select all to be sent in DB"));   
  }
  */
 
  int rc;
  mqttMessage_t message;

  data->logger->notice(F("db Total number of record in DB"));       
  rc = db_exec("SELECT COUNT(*) FROM messages");
  if (rc != SQLITE_OK) {
    data->logger->error(F("db select count"));   
  }

  data->logger->notice(F("db Total number of record in DB to send"));       
  rc = db_exec("SELECT COUNT(*) FROM messages WHERE sent = 0");
  if (rc != SQLITE_OK) {
    data->logger->error(F("db select count to sent"));
  }

  /*
  // remove old DB
  bool obsolete;
  if (!db_obsolete(obsolete)) data->logger->error(F("db cheking obsolete DB"));

  if (obsolete){
    if (!db_remove()) data->logger->error(F("db remove obsoltete DB"));
  }
  */

  /*
  // migrate all old data from DB to archive
  bool obsolete;
  if (!db_obsolete(obsolete)) data->logger->error(F("db cheking obsolete DB"));
  while (obsolete){
    bool end;
    if (!data_purge()) data->logger->error(F("db purge DB"));
    if (!db_obsolete(obsolete)) data->logger->error(F("db cheking obsolete DB"));
  }
  */
  
  // migrate all old data from DB to archive
  data->logger->notice(F("db cheking obsolete DB"));
  bool obsolete;
  if (!db_obsolete(obsolete)) data->logger->error(F("db cheking obsolete DB"));
  if (obsolete){
    data->logger->notice(F("db flush data from obsolete DB to archive"));
    if (!data_purge(true)) data->logger->error(F("db purge DB"));
  }
  
  data->status->database=ok;
  sqlite_status=true;
  
  //  uint8_t basePriority = GetPriority();
  //  SetPriority(basePriority-1);
  //  SetPriority(basePriority);
  
  for(;;){
      
    while (data->dbqueue->Peek(&message, pdMS_TO_TICKS( 1000 ))){  // peek one message
      if (!doDb(message)) return;                                 // return and terminate task if fatal error
      if (!sqlite_status) sqlite_status = db_restart();           // try to restart SD card and sqlite
      if (!sqlite_status) ESP.restart();                          // SD do not restart; REBOOT
    }

    
    // free memory
    //if(db_exec("PRAGMA shrink_memory;") != SQLITE_OK) {
    if(sqlite3_db_release_memory(db)) {
      data->logger->error(F("db release memory"));       
    }

    // check queue for rpc recovey
    if(data->recoveryqueue->Dequeue(&rpcrecovery, pdMS_TO_TICKS( 0 ))) data_set_recovery();

    while (data->dbqueue->Peek(&message, pdMS_TO_TICKS( 1000 ))){  // peek one message
      if (!doDb(message)) return;                                 // return and terminate task if fatal error
      if (!sqlite_status) sqlite_status = db_restart();           // try to restart SD card and sqlite
      if (!sqlite_status) ESP.restart();                          // SD do not restart; REBOOT
    }

    // check semaphore for data recovey
    if(data->recoverysemaphore->Take(0)){
      if (timeStatus() == timeSet) {
	if (!data_purge()) data->logger->error(F("db purge DB"));
      }
      if(!data_recovery()) data->logger->error(F("db recovery DB"));
    }

    // checks for heap and stack
    //data->logger->notice(F("HEAP: %l"),esp_get_minimum_free_heap_size());
    if( esp_get_minimum_free_heap_size() < HEAP_MIN_WARNING)data->logger->error(F("HEAP: %l"),esp_get_minimum_free_heap_size());
    //data->logger->notice("stack db: %d",uxTaskGetStackHighWaterMark(NULL));
    if ( uxTaskGetStackHighWaterMark(NULL) < STACK_MIN_WARNING ) data->logger->error(F("db stack"));
    
  }
};  
