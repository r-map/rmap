
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
  File* logFile;
};

typedef enum {
  ARCHIVE_RECOVERY_NONE,
  ARCHIVE_RECOVERY_INIT,
  ARCHIVE_RECOVERY_READ_MESSAGE,
  ARCHIVE_RECOVERY_FILTER,
  ARCHIVE_RECOVERY_PUBLISH,
  ARCHIVE_RECOVERY_END,
} archive_recovery_state_t;

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
  archive_recovery_state_t getArchiveRecoveryState();
  
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
  bool recovery();
  bool archive_recovery();
  bool data_set_recovery();
  bool db_restart();
  void setEventArchiveRecovery();
  int db_count_delayed_messages(uint32_t& number);
  
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
  File archiveRecoveryFile;
  bool archiveRecoveryResend;
  bool archive_recovery_run;
  mqttMessage_t archive_recovery_message;
  time_t archive_recovery_start;
  time_t archive_recovery_stop;  
  archive_recovery_state_t archive_recovery_state;
};

// we need this global for SDcard restart
#if (ENABLE_SDCARD_LOGGING)   
/*!
\var logFile
\brief File for logging on SD-Card.
*/
extern File logFile;
#endif

#endif
