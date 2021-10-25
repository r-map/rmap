#ifndef _BUFFER_STREAM_H_INCLUDED_
#define _BUFFER_STREAM_H_INCLUDED_

#include <Stream.h>
#include <ByteBuffer.h>

class BufferStream : public Stream
{
public:
 BufferStream( ByteBuffer &bytebuffer) :  buffer(bytebuffer) { }
  
  // Stream methods
  virtual void setTimeout( unsigned long time) {}
  virtual int available()    { return buffer.getSize();      }
  virtual int read()         { return buffer.getFromBack(); }
  virtual int   peek() { return buffer.peek(0); }
  virtual void flush() {buffer.clear(); };
  // Print methods
  virtual size_t write(uint8_t c) { buffer.putInFront(c); return 1; };
  //virtual String readString()

  virtual size_t readBytes(char* outchar, size_t len)
  {
    size_t ind=0;
    while (buffer.getSize() > 0 && ind <  (len)){
      outchar[ind++]=(char)buffer.getFromBack();
    }
    return ind;
  }
  
  virtual String readString()
  {
    String outstring=String("");
    while (buffer.getSize() > 0){
      outstring.concat((char)buffer.getFromBack());
    }
    return outstring;
  }
    /*     TO BE DONE
      size_t write(String string)
      size_t write(uint8_t *buffer, size_t len)0
      find()
      findUntil()
      readBytesUntil()
      readStringUntil()
      parseInt()
      parseFloat()
    */

private:
  ByteBuffer &buffer;
};

#endif // _BUFFER_STREAM_H_INCLUDED_

