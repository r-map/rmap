// StreamUtils - github.com/bblanchon/ArduinoStreamUtils
// Copyright Benoit Blanchon 2019-2021
// MIT License

#pragma once

#include <Stream.h>
#include <WString.h>

#include "../Configuration.hpp"
#include "../Polyfills.hpp"

namespace StreamUtils {

class StringStream : public Stream {
 public:
  StringStream() {}

  explicit StringStream(String str) : _str(Polyfills::move(str)) {}

  size_t write(const uint8_t* p, size_t n) override {
    for (size_t i = 0; i < n; i++) {
      uint8_t c = p[i];
      if (c == 0)
        return i;
      write(c);
    }
    return n;
  }

  size_t write(uint8_t c) override {
    if (c == 0)
      return 0;
    _str += static_cast<char>(c);
    return 1;
  }

  const String& str() const {
    return _str;
  }

  void str(String str) {
    _str = Polyfills::move(str);
  }

  int available() override {
    return _str.length();
  }

  int read() override {
    if (_str.length() == 0)
      return -1;
    char c = _str[0];
    _str.remove(0, 1);
    return c;
  }

#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
  size_t readBytes(char* buffer, size_t length) override {
    if (length > _str.length())
      length = _str.length();
    _str.toCharArray(buffer, length);
    _str.remove(0, length);
    return length;
  }
#endif

  int peek() override {
    return _str.length() > 0 ? _str[0] : -1;
  }

  void flush() override {}

 private:
  String _str;
};
}  // namespace StreamUtils