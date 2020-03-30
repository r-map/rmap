#ifndef _STRING_STREAM_H_INCLUDED_
#define _STRING_STREAM_H_INCLUDED_

#include <Stream.h>

class StringStream : public Stream
{
public:
  StringStream( String &s) : string(s), position(0) { }
  
  // Stream methods
  virtual int available() { return string.length() - position; }
  virtual int read() { return position < string.length() ? string[position++] : -1; }
  virtual String readString()
  {
    if (position < string.length()) {
      String outstring="";
      while (position < string.length()){
	outstring +=string[position++];
      }
      return outstring;
    }else{
      return "";
    }
  }
  virtual int peek() { return position < string.length() ? string[position] : -1; }
  virtual void flush() {string =""; position=0 ;};
  // Print methods
  virtual size_t write(uint8_t c) { string += (char)c; return 1; };
private:
  String &string;
  unsigned int length;
  unsigned int position;
};

#endif // _STRING_STREAM_H_INCLUDED_
