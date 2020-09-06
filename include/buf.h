#pragma once

#define BUFF_SIZE   128
#define BUFF_DELIM  " " // strtok_r needs a null-terminated string

class BUF
{
  char        mBuf[BUFF_SIZE];     // Buffer of stored characters while waiting for terminator character
  int         mBufPos;             // Current position in the buffer
  char*       mLast;               // for strtok_r
  const char* mDelim = BUFF_DELIM; // strtok_r needs a /0 terminated string
  
public:
  char*       getBuf() { return mBuf; };
  int         getLen() { return BUFF_SIZE-1; };
  const char* first()  { return strtok_r(mBuf, mDelim, &mLast); };
  const char* next()   { return strtok_r(nullptr, mDelim, &mLast); };

  void clear()
  {
    mBuf[0] = '\0';
    mBufPos = 0;
  };

  void append(char c)
  {
    if (mBufPos < BUFF_SIZE-1)
    {
      mBuf[mBufPos++] = c;  // Put character into buffer
      mBuf[mBufPos] = '\0'; // Null terminate
    }
  };
};
