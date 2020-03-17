#pragma once

#include <Streaming.h>
#include <objCmd.h>
#include <SPIFFS.h>

#define FORMAT_SPIFFS_IF_FAILED true

#define BTCMD_BUFF_SIZE    127
#define BTCMD_MAXOBJ  15

#define BTCMD_TERM    '\n' 
#define BTCMD_DELIM   " " // strtok_r needs a null-terminated string
#define BTCMD_SET     "set"
#define BTCMD_GET     "get"

#define FNAME_CURRENT "/config.cfg"
#define FNAME_DEFAULT "/config.def"

#define BTCMD_MAXARGS 3

//----------------
class BUF
{
  char mBuf[BTCMD_BUFF_SIZE + 1];  // Buffer of stored characters while waiting for terminator character
  int mBufPos;                // Current position in the buffer
  char* mLast;                // for strtok_r
  const char* mDelim = BTCMD_DELIM; // strtok_r needs a /0 terminated string
  
public:
  char* getBuf() { return mBuf; };
  int getLen() { return BTCMD_BUFF_SIZE; };

  const char* first() { return strtok_r(mBuf, mDelim, &mLast); };
  const char* next() { return strtok_r(NULL, mDelim, &mLast); };

  void clearBuffer()
  {
    mBuf[0] = '\0';
    mBufPos = 0;
  };

  void appendToBuffer(char c)
  {
    if (mBufPos < BTCMD_BUFF_SIZE) {
      mBuf[mBufPos++] = c;  // Put character into buffer
      mBuf[mBufPos] = '\0';      // Null terminate
    }
  };
};

//-------------------------------
class BTcmd
{
  struct mRegisteredOBJ {
    char* name;
    OBJCmd* obj;
  };

  mRegisteredOBJ mOBJ[BTCMD_MAXOBJ];
  byte mNOBJ = 0;

  Stream* mBTStream;
  
  bool spiffsOK = false;
  const char* cfg_fname = FNAME_CURRENT;
  const char* def_fname = FNAME_DEFAULT;

  BUF mBTbuf;
  BUF mFilebuf;
  
  const char* mSetKeyword = BTCMD_SET;
  const char* mGetKeyword = BTCMD_GET;

  void handleCmd(Stream* stream, BUF& buf);
  void readStream(Stream* stream, BUF& buf);
  File getFile(bool isdefault, const char *mode);
  
public:

  BTcmd(Stream &btStream);
  
  bool registerObj(const OBJCmd& obj, const char* name);
  OBJCmd* getObjFromName(const char* name); 

  void initSPIFFS();
  void save(bool isdefault);
  void load(bool isdefault);

  void readBTStream() { readStream(mBTStream, mBTbuf); };
};
