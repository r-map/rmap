#ifndef Client_h
#define Client_h

//#define EthernetClient sim800Client

#include "sim800.h"

class sim800Client : public SIM800 {

 public:
  sim800Client();
  //
  // etherclient compatibility
  //

  int connect(IPAddress ip, int port);
  int connect(const char *host, int port);
  uint8_t connected();
  int available();
  int read();
  //int read(uint8_t *buf, size_t size);
  size_t write(uint8_t);
  size_t write(const uint8_t *buf, size_t size);
  void flush();
  void stop();

  bool transparentescape();
  bool transparent();

};


/*
class Client : HardwareSerial {
 public:
  //Client() : HardWareserial(const HardwareSerial& );
  Client(const int baudrate);

  void connect(char *server, int port);

};

*/
#endif
