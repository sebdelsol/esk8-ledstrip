#pragma once

#include <log.h>
#include <Variadic.h>
#include <ObjVar.h>
#include <Buf.h>
#include <iterator.h>
#include <FileObj.h>

//------------------- Dbg
// #define DBG_CMD        // to see what's happening with send & received cmd

//-------------------------------
#define MAXOBJ        18

#define CMD_RESERVED  '!'
#define CMD_1ST_ID    (CMD_RESERVED + 1)
#define CMD_TERM      '\n' 
#define CMD_ALIVE     '~'

static auto CMD_SET          = "set";
static auto CMD_GET          = "get";
static auto CMD_INIT         = "init";
static auto CMD_INIT_DONE    = "initdone";
static auto CMD_UPDATE_SHORT = "U";

enum class Decode :   uint8_t { compact, verbose, undefined };

//-------------------------------
class AllObj : public CfgFiles
{
  struct parsedCmd 
  {
    OBJVar*      obj;
    MyVar*       var;
  };

  HashName<OBJVar, MAXOBJ> mHash;
  OBJVar*     mOBJS[MAXOBJ];
  byte        mNOBJ = 0;
  ArrayOfPtr_Iter(OBJVar, mOBJS, mNOBJ); 
  
  byte        mID = 0;
  BUF         mTmpBuf;

  void    dbgCmd(const char* cmdKeyword, const parsedCmd& parsed, int nbArg, int* args, bool line);
  void    dbgCmd(const char* cmdKeyword, const parsedCmd& parsed, int nbArg, int* args, int min, int max);

  bool    isNumber(const char* txt);
  void    setCmd(const parsedCmd& parsed, BUF& buf, TrackChange trackChange);
  void    getCmd(const parsedCmd& parsed, Stream& stream, Decode decode);
  void    initCmd(const parsedCmd& parsed, Stream& stream);
  bool    parseCmd(parsedCmd& parsed, BUF& buf);
  void    handleCmd(Stream& stream, BUF& buf, TrackChange trackChange, Decode decode);
  
protected:
  void readCmd(Stream& stream, BUF& buf, TrackChange trackChange, Decode decode);
  void sendCmdForAllVars(Stream& stream, const char* cmdKeyword, TrackChange trackChange, Decode decode, MyVar::TestFunc test = nullptr);

public:
  void save(CfgType cfgtype);
  void load(CfgType cfgtype, TrackChange trackChange = TrackChange::yes);
  
  bool addObj(OBJVar& obj, const char* name);
  OBJVar* getObjFromName(const char* name);
  ForEachMethodPairs(addObj);  // create a method addObjs(obj1, name1, obj2, name2, ...) that calls addObj(obj, name) for each pair
};

