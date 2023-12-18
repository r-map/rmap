#include "stimawifi.h"

#ifndef UDP_THREAD_H_
#define UDP_THREAD_H_


struct udp_data_t {
  int id;
  frtosLogging logger;
};


using namespace cpp_freertos;

class udpThread : public Thread {
  
 public:
  /**
   *  Constructor to create a UDP thread.
   *
   *  @param udp_data data used by thread.
   */
  
  udpThread(udp_data_t &udp_data);
  
 protected:
  virtual void Run();

 private:
  udp_data_t data;

};

#endif
