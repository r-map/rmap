
#ifndef DB_THREAD_H_
#define DB_THREAD_H_

struct db_data_t {
  int id;
  frtosLogging* logger;
  Queue* dbqueue;
  Queue* mqttqueue;
  BinarySemaphore* recoverysemaphore;
  BinaryQueue* recoveryqueue;
  dbStatus_t* status;
  station_t* station;
};


using namespace cpp_freertos;

class dbThread : public Thread {
  
 public:
  /**
   *  Constructor to create a DB thread.
   *
   *  @param db_data data used by thread.
   */
  
  dbThread(db_data_t* db_data);
  ~dbThread();
  virtual void Cleanup();
  
 protected:
  virtual void Run();

 private:
  bool doDb(const mqttMessage_t& );
  int db_exec( const char*);
  bool db_obsolete(bool& obsolete);
  bool db_remove();
  void db_setup();
  bool data_purge(const bool flush, int messages);
  bool data_recovery();
  bool data_set_recovery();
  bool db_restart();
  db_data_t* data;
  sqlite3 *db;
  //SdFat SD;
  uint8_t sqlite_memory[SQLITE_MEMORY];   // allocated memory used by sqlite
  bool sqlite_status;
  rpcRecovery_t rpcrecovery;
  /*!
    \var archiveFile
    \brief File for archive on SD-Card.
  */
  File archiveFile;

};

#endif
