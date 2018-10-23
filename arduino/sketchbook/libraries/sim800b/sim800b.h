
#ifndef sim800_h
#define sim800_h

#include "Arduino.h"
#include <Time.h>
//#include <inttypes.h>
#include "IPAddress.h"

// activate debug on serial port
//#define DEBUGONSERIAL
#define ENABLEWDT 1
// if define HARDWARESERIAL is set then it will be static (required by Time library)
#define HARDWARESERIAL Serial1

#define BUF_LENGTH 100
#define BUFCOMMAND_LENGTH 120

#define STATE_NONE 0
#define STATE_ON 1
#define STATE_INITIALIZED 2
#define STATE_REGISTERED 4
#define STATE_HTTPINITIALIZED 8

#define ATSTR  "AT"
#define OKSTR  "OK\r\n"
#define ERRORSTR  "ERROR\r\n"


class SIM800 {

 public:
  SIM800();

#ifdef HARDWARESERIAL
  bool init(byte onOffPin, byte resetPin);
#else
  bool init(HardwareSerial *modemPort, byte onOffPin, byte resetPin);
#endif
  bool init_onceautobaud();
  bool init_autobaud();
  bool init_fixbaud();
  bool setup();

  bool startNetwork(const char *apn, const char *user, const char *pwd );
  bool stopNetwork();

  bool checkNetwork();
  bool GetMyIP(char*ip); // ip no less 15 char + terminator
  bool getIMEI(char *imei); // imei no less 15 char + terminator
  bool getSignalQualityReport(int*rssi,int*ber);

  bool httpGET(const char* server, int port, const char* path, char* result, int resultlength);

  bool isOn();
  bool isInitialized();
  bool isRegistered();
  bool isHttpInitialized();

  void switchOn();
  void switchOff();
  void resetModem();


#ifdef HARDWARESERIAL
  static void send(const char *buf);
  static void cleanInput();
  static byte receive(char *buf);
  static byte receive(char *buf, uint16_t timeout);
  static bool receive(char *buf, uint16_t timeout, char const *checkok, char const *checkerror);
  static bool receivelen(char *buf, uint16_t timeout, unsigned int datalen);
  static bool ATcommand(const char *command, char *buf);
  static bool ATcommand(const char *command, char *buf, char const *checkok, char const *checkerror, unsigned long  timeout);
  static time_t  RTCget();
  static uint8_t RTCread(tmElements_t &tm);
#else
  void send(const char *buf);
  void cleanInput();
  byte receive(char *buf);
  byte receive(char *buf, uint16_t timeout);
  bool receive(char *buf, uint16_t timeout, char const *checkok, char const *checkerror);
  bool receivelen(char *buf, uint16_t timeout, unsigned int datalen);
  bool ATcommand(const char *command, char *buf);
  bool ATcommand(const char *command, char *buf, char const *checkok, char const *checkerror, unsigned long  timeout);
  time_t  RTCget();
  uint8_t RTCread(tmElements_t &tm);
#endif

  bool TCPstart(const char *apn, const char *user, const char *pwd );
  bool TCPconnect(const char* server, int port);
  bool TCPGetMyIP(char*ip);
  bool TCPstop();

  uint8_t RTCset(time_t t);
  uint8_t RTCwrite(tmElements_t &tm);

  byte state;

 private:
  void switchModem();

  byte onOffPin;
  byte resetPin;

#ifndef HARDWARESERIAL
  HardwareSerial *modem;
#endif

};

#ifdef DEBUGONSERIAL
#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif

#ifdef ENABLEWDT
#include <avr/wdt.h>
#endif

#endif
