#ifndef FILE_READER_H
#define FILE_READER_H

#include <FS.h>

#ifdef ESP32
#include <SPIFFS.h>
#endif

class FileReader
{
private:
  File mFile;
  unsigned int mCurrentLineNumber;
  unsigned int mCurrentPosition;

public:
  FileReader()
  {
    mCurrentLineNumber = 0;
    mCurrentPosition = 0;
  };

  ~FileReader()
  {
    mFile.close();
  };

  // Open the specified file and return the result.
  bool open(const String& path)
  {
    if (!SPIFFS.exists(path))
      return false;

    mFile = SPIFFS.open(path, "r");
    if (mFile)
      return true;
    else
      return false;
  }

  // Read a char and return the result. It also count the number of line and the position of the cursor.
  bool readChar(char &ch)
  {
    int val = mFile.read();

    if (val == -1)
      return false;
    else
    {
      ch = char(val);

      // Check if its a line ending to count number of lines
      if (ch == 10 || ch == 13)
      {
        mCurrentLineNumber++;
        mCurrentPosition = 0;
      }
      else
        mCurrentPosition++;

      return true;
    }
  }

  inline bool endOfLine() const
  {
    return mCurrentPosition == 0;
  }

  inline unsigned int getCurrentLine() const
  {
    return mCurrentLineNumber;
  }

  inline unsigned int getCurrentPosition() const
  {
    return mCurrentPosition;
  }
};

#endif
