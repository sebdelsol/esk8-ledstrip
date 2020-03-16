#pragma once

#include <Streaming.h>
#include <objCmd.h>
#include <SPIFFS.h>

#define FORMAT_SPIFFS_IF_FAILED true

#define BTCMD_BUFF_SIZE 127
#define BTCMD_MAXOBJ 15

#define BTCMD_TERM '\n'
#define BTCMD_DELIM " " // strtok_r needs a null-terminated string

#define BTCMD_MAXARGS 3

class BTcmd
{
  struct mRegisteredOBJ {
    char*   name;
    OBJCmd* obj;
  };

  mRegisteredOBJ mOBJ[BTCMD_MAXOBJ];
  byte mNOBJ = 0;

  Stream* mStream;
  
  bool    spiffsOK = false;
  const char* cfg_fname = "/config.cfg";
  const char* def_fname = "/config.def";

  char  mBuf[BTCMD_BUFF_SIZE + 1]; // Buffer of stored characters while waiting for terminator character
  byte  mBufPos;                   // Current position in the buffer
  char* mLast;    // for strtok_r
  char  mDelim[2]; // strtok_r needs a /0 terminated string

  void clearBuffer();
  void appendToBuffer(char c);
  void handleCmd(char* buf, bool autoSet = false);

  char* first(char* buf) { return strtok_r(buf, mDelim, &mLast); };
  char* next() { return strtok_r(NULL, mDelim, &mLast); };

public:

  BTcmd(Stream &stream);
  void initSPIFFS();
  bool registerObj(const OBJCmd& obj, char* name);
  void save(bool isdefault);
  void load(bool isdefault);

  OBJCmd* getObjFromName(char* name);

  void readStream();
};
