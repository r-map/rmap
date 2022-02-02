#ifndef RSITE_STREAM_FLOW
#define RSITE_STREAM_FLOW

#include <Stream.h>
//#include <avr/pgmspace.h>
#include <HardwareSerial.h>

//#define endl "\n\r"
//#define tab "\t"
//#define then "\0"
//#define dot "."
//#define dotl ".\n\r"
class fmt {
  public:
  virtual Stream& operator<<(Stream& o)=0;
};

class StreamFlush:public fmt {
  public:
  StreamFlush() {}
  virtual Stream& operator<<(Stream& o) {o.flush();return o;}
};

extern StreamFlush go;

class intFmt:public fmt {
  public:
  intFmt(int v):v(v) {}
  int v;
};

class hex:public intFmt {
  public:
  hex(int v):intFmt(v) {}
  Stream& operator<<(Stream& s) {s.print(v,HEX);return s;}
};

class bin:public intFmt {
  public:
  bin(int v):intFmt(v) {}
  Stream& operator<<(Stream& s) {s.print(v,BIN);return s;}
};

inline Stream& operator<<(Stream& s,hex v) {return v.operator<<(s);}
inline Stream& operator<<(Stream& s,bin v) {return v.operator<<(s);}

inline Stream& operator<<(Stream& s,char v) {s.print(v);return s;}
inline Stream& operator<<(Stream& s,int v) {s.print(v);return s;}
inline Stream& operator<<(Stream& s,long v) {s.print(v);return s;}
inline Stream& operator<<(Stream& s,unsigned int v) {s.print(v);return s;}
inline Stream& operator<<(Stream& s,unsigned long v) {s.print(v);return s;}
inline Stream& operator<<(Stream& s,double v) {s.print(v);return s;}
inline Stream& operator<<(Stream& s,const char* v) {s.print(v);return s;}
//inline Stream& operator<<(Stream& s,const char* const __attribute__((__progmem__)) v) {s.print(v);return s;}
inline Stream& operator<<(Stream& s,const __FlashStringHelper* v) {
  const char* const ptr=(const char* const)v;
  for(uint16_t n=0;n<0xffff;n++) {
    char ch=pgm_read_byte(ptr+n);
    if (!ch) break;
    s.print(ch);
  }
  //s.print(v);
  return s;
}
//inline Stream& operator<<(Stream& s,fmt& v) {return v.operator<<(s);}
//.. add some more members as needed

class endlObj:public fmt {
  public:Stream& operator<<(Stream& o) override {return o<<"\r\n";}
};
class tabObj:public fmt {
  public:Stream& operator<<(Stream& o) override {return o<<'\t';}
};
class thenObj:public fmt {
  public:Stream& operator<<(Stream& o) override {return o<<'\0';}
};
class dotObj:public fmt {
  public:Stream& operator<<(Stream& o) override {return o<<'.';}
};
class dotlObj:public fmt {
  public:Stream& operator<<(Stream& o) override {return o<<".\r\n";}
};
extern endlObj endl;
extern tabObj tab;
extern thenObj then;
extern dotObj dot;
extern dotlObj dotl;
inline Stream& operator<<(Stream &o,endlObj& v) {return v.operator<<(o);}
inline Stream& operator<<(Stream &o,tabObj& v) {return v.operator<<(o);}
inline Stream& operator<<(Stream &o,thenObj& v) {return v.operator<<(o);}
inline Stream& operator<<(Stream &o,dotObj& v) {return v.operator<<(o);}
inline Stream& operator<<(Stream &o,dotlObj& v) {return v.operator<<(o);}

template <int N>
class tabs {
	public:
  Stream& operator<<(Stream& o) {for(int n=0;n<N;n++) tab.operator<<(o);return o;}
};

template <int N>
inline Stream& operator<<(Stream& s,tabs<N>& v) {return v.operator<<(s);}
// template<> Stream& operator<<(Stream&,tabs<1> v) {return v.operator<<(s);}

#endif
