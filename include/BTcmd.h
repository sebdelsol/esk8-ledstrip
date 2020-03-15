#pragma once

#include <Streaming.h>
// #include <MyCmd.h>
#include <objCmd.h>

//#include <ledstrip.h>

// SET FX_alias what [args>0]
// GET FX_alias what

#define BTCMD_BUFF_SIZE 127
#define BTCMD_MAXOBJ 15

#define BTCMD_TERM '\n'
#define BTCMD_DELIM " " // strtok_r needs a null-terminated string

class BTcmd
{
  struct mRegisteredOBJ {
    char*   name;
    OBJCmd* obj;
  };

  mRegisteredOBJ mOBJ[BTCMD_MAXOBJ];
  byte mNOBJ = 0;

  Stream* mStream;

  char  mBuf[BTCMD_BUFF_SIZE + 1]; // Buffer of stored characters while waiting for terminator character
  byte  mBufPos;                   // Current position in the buffer
  char* mLast;    // for strtok_r
  char  mDelim[2]; // strtok_r needs a /0 terminated string

  void clearBuffer();
  void appendToBuffer(char c);
  void handleCmd();

  char* first() { return strtok_r(mBuf, mDelim, &mLast); };
  char* next() { return strtok_r(NULL, mDelim, &mLast); };

public:

  BTcmd(Stream &stream);
  bool registerObj(const OBJCmd& obj, char* name);
  OBJCmd* getObjCmdFromName(char* name);

  void readStream();
};
