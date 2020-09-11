#pragma once

#include <log.h>
#include <SPIFFS.h>
#include <Variadic.h>
#include <ObjVar.h>
#include <Buf.h>
#include <iterator.h>

//------------------- Dbg
// #define DBG_SHOWFILES  // to see files in SPIFFS
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

static auto CFG_CURRENT      = "/config.cfg";
static auto CFG_DEFAULT      = "/config.def";

enum class CfgType : bool   { Default, Current };
enum class Decode : uint8_t { compact, verbose, undefined };
enum class FileMode : bool  { save, load };

//-------------------------------
class AllObj 
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
  bool        spiffsOK = false;
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
  void sendCmdForAllVars(Stream& stream, const char* cmdKeyword, TrackChange trackChange, Decode decode, MyVar::TestFunc testVar = nullptr);

public:
  void init();
  void save(CfgType cfgtype);
  void load(CfgType cfgtype, TrackChange trackChange = TrackChange::yes);
  
  bool addObj(OBJVar& obj, const char* name);
  OBJVar* getObjFromName(const char* name);
  ForEachMethodPairs(addObj);  // create a method addObjs(obj1, name1, obj2, name2, ...) that calls addObj(obj, name) for each pair
};

//-------------------------------
class CfgFile
{
  File  f;
  bool  isloading;

public:
  CfgFile(CfgType cfgtype, FileMode mode)
  {
    bool isdef = cfgtype == CfgType::Default;
    isloading = mode == FileMode::load; 
    
    const char* fname = isdef ? CFG_DEFAULT : CFG_CURRENT;
    f = SPIFFS.open(fname, isloading ? "r" : "w");

    if (f)
      _log << (isloading ? "Loading from " : "Saving to ") << fname << "...";
    else
      _log << "FAIL to " << (isloading ? "load from " : "save to ") << fname << endl;
  };

  ~CfgFile()
  {
    if (f)
    {
      f.close();
      _log << (isloading ? "loaded" : "saved") << endl;
    }
  }

  Stream& getStream() { return (Stream&)f; };
  bool    isOk()      { return f ? true : false; };
};
