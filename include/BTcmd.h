#pragma once

#include <Streaming.h>
#include <objCmd.h>
#include <SPIFFS.h>

#define FORMAT_SPIFFS_IF_FAILED true

#define BTCMD_BUFF_SIZE 127
#define BTCMD_MAXOBJ 15

#define BTCMD_TERM '\n'
#define BTCMD_DELIM " " // strtok_r needs a null-terminated string

#define BTCMD_SET "set"
#define BTCMD_GET "get"

#define BTCMD_MAXARGS 3

class BUFbase
{
  virtual char* getBuf();
  virtual int getLen();
  virtual void clearBuffer();
  virtual void appendToBuffer(char c);
}

template <int SIZE>
class BUF: BUFbase
{
 char  mBuf[SIZE + 1]; // Buffer of stored characters while waiting for terminator character
 int  mBufPos;         // Current position in the buffer
  
public:
 char* getBuf() { return mBuf; };
 int getLen() { return SIZE; };
  
 void clearBuffer()
 {
   mBuf[0] = '\0';
   mBufPos = 0;
 };

 void appendToBuffer(char c)
 {
  if (mBufPos < SIZE) {
    mBuf[mBufPos++] = c;  // Put character into buffer
    mBuf[mBufPos] = '\0';      // Null terminate
  }
 };
};

class BTcmd
{
  struct mRegisteredOBJ {
    char*   name;
    OBJCmd* obj;
  };

  mRegisteredOBJ mOBJ[BTCMD_MAXOBJ];
  byte mNOBJ = 0;

  Stream* mBTStream;
  
  bool    spiffsOK = false;
  const char* cfg_fname = "/config.cfg";
  const char* def_fname = "/config.def";

  //char  mBuf[BTCMD_BUFF_SIZE + 1]; // Buffer of stored characters while waiting for terminator character
  //byte  mBufPos;                   // Current position in the buffer
  BUF<BTCMD_BUFF_SIZE> mBTbuf;
  BUF<BTCMD_BUFF_SIZE> mFilebuf;
  
  char* mLast;    // for strtok_r
  char  mDelim[] = BTCMD_DELIM; // strtok_r needs a /0 terminated string
  
  const char* mSetKeyword = BTCMD_SET;
  const char* mGetKeyword = BTCMD_GET;

  void handleCmd(Stream* stream,BUFbase &buf);

  char* first(char* buf) { return strtok_r(buf, mDelim, &mLast); };
  char* next() { return strtok_r(NULL, mDelim, &mLast); };
  void readStream(Stream* stream,BUFbase &buf);
  File getFile(bool isdefault, const char *mode);
  
public:

  BTcmd(Stream &btStream);
  void initSPIFFS();
  bool registerObj(const OBJCmd& obj, char* name);
  void save(bool isdefault);
  void load(bool isdefault);

  OBJCmd* getObjFromName(char* name); 
  void readStream() {readStream(mBTStream, mBTbuf);};
};
