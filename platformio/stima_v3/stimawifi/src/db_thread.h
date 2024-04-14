
#ifndef DB_THREAD_H_
#define DB_THREAD_H_

struct db_data_t {
  int id;
  frtosLogging* logger;
  Queue* dbqueue;
  Queue* mqttqueue;
  BinarySemaphore* recoverysemaphore;
  dbStatus_t* status;
};

bool doDb(sqlite3 *db_test, db_data_t& data, const mqttMessage_t& message);

using namespace cpp_freertos;

class dbThread : public Thread {
  
 public:
  /**
   *  Constructor to create a DB thread.
   *
   *  @param db_data data used by thread.
   */
  
  dbThread(db_data_t& db_data);
  ~dbThread();
  virtual void Cleanup();
  
 protected:
  virtual void Run();

 private:
  db_data_t data;
  sqlite3 *db;
  //SdFat SD;
};

#endif
