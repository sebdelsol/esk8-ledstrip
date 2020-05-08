#pragma once

#include <Streaming.h>
#include <objVar.h>
#include <SPIFFS.h>

#define FORMAT_SPIFFS_IF_FAILED true

#define BTCMD_BUFF_SIZE 127
#define BTCMD_MAXOBJ    20

#define BTCMD_1ST_ID  '!'
#define BTCMD_TERM    '\n' 
#define BTCMD_ALIVE    '~'
#define BTCMD_DELIM   " " // strtok_r needs a null-terminated string
#define BTCMD_SET     "set"
#define BTCMD_GET     "get"
#define BTCMD_INIT    "init"

#define FNAME_CURRENT "/config.cfg"
#define FNAME_DEFAULT "/config.def"

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

  void clear()
  {
    mBuf[0] = '\0';
    mBufPos = 0;
  };

  void append(char c)
  {
    if (mBufPos < BTCMD_BUFF_SIZE) {
      mBuf[mBufPos++] = c;  // Put character into buffer
      mBuf[mBufPos] = '\0';      // Null terminate
    }
  };
};

//-------------------------------
struct parsedCmd {
  OBJVar* obj;
  MyVar* var;
  const char* objName;
  const char* varName;
};

class BTcmd
{
  struct mRegisteredOBJ {
    char* name;
    OBJVar* obj;
  };

  mRegisteredOBJ mOBJ[BTCMD_MAXOBJ];
  byte mNOBJ = 0;
  byte mID = 0;

  Stream* mBTStream;
  Stream* mDbgSerial;
  
  bool spiffsOK = false;
  const char* cfg_fname = FNAME_CURRENT;
  const char* def_fname = FNAME_DEFAULT;

  BUF mBTbuf;
  BUF mTmpBuf;
  
  const char* mSetKeyword = BTCMD_SET;
  const char* mGetKeyword = BTCMD_GET;
  const char* mInitKeyword = BTCMD_INIT;
  
  bool isNumber(const char* txt);
  void dbgCmd(const char* cmd, const parsedCmd& parsed, int nbArg, int* args);

  void handleSetCmd(const parsedCmd& parsed, BUF& buf, bool change);
  void handleGetCmd(const parsedCmd& parsed, Stream* stream, bool compact);
  void handleInitCmd(const parsedCmd& parsed, Stream* stream);
  void handleCmd(Stream* stream, BUF& buf, bool change = true, bool compact = false);

  void readStream(Stream* stream, BUF& buf, bool change = true, bool compact = false);
  void emulateCmdForAllVars(const char* cmdKeyword, Stream *stream, OBJVar::ObjTestVarFunc testVar = NULL, bool change = true, bool compact = false);

  File getFile(bool isdefault, const char* mode);

public:

  BTcmd(Stream& btStream);
  
  bool registerObj(OBJVar& obj, const char* name);
  OBJVar* getObjFromName(const char* name); 

  void init(Stream& dbgSerial);
  void save(bool isdefault);
  void load(bool isdefault, bool change);
  void sendInitsOverBT();
  void sendUpdateOverBT();
  void readBTStream() { readStream(mBTStream, mBTbuf, false, true); };
};
