#pragma once

#include <log.h>
#include <SPIFFS.h>
#include <Variadic.h>
#include <ObjVar.h>
#include <Buf.h>
#include <iterator.h>

//------------------- Dbg
// #define DBG_SHOWFILES   // to see files in SPIFFS
// #define DBG_CMD        // to see what's happening with send & received cmd

//-------------------------------
#define ALLOBJ_MAXOBJ     18

#define ALLOBJ_RESERVED   '!'
#define ALLOBJ_1ST_ID      (ALLOBJ_RESERVED + 1)
#define ALLOBJ_TERM        '\n' 
#define ALLOBJ_ALIVE       '~'

#define ALLOBJ_SET        "set"
#define ALLOBJ_GET        "get"
#define ALLOBJ_INIT       "init"

#define CFG_CURRENT       "/config.cfg"
#define CFG_DEFAULT       "/config.def"

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

  HashName<ALLOBJ_MAXOBJ, OBJVar> mHash;
  OBJVar*     mOBJS[ALLOBJ_MAXOBJ];
  byte        mNOBJ = 0;
  byte        mID = 0;

  bool        spiffsOK = false;
  BUF         mTmpBuf;
  
  void    dbgCmd(const char* cmdKeyword, const parsedCmd& parsed, int nbArg, int* args, bool  line);
  void    dbgCmd(const char* cmdKeyword, const parsedCmd& parsed, int nbArg, int* args, int min, int max);

  bool    isNumber(const char* txt);

  void    setCmd(const parsedCmd& parsed, BUF& buf, TrackChange trackChange);
  void    getCmd(const parsedCmd& parsed, Stream& stream, Decode decode);
  void    initCmd(const parsedCmd& parsed, Stream& stream);
  bool    parseCmd(parsedCmd& parsed, BUF& buf);
  void    handleCmd(Stream& stream, BUF& buf, TrackChange trackChange, Decode decode);

protected:
  const char* mSetKeyword = ALLOBJ_SET;
  const char* mGetKeyword = ALLOBJ_GET;
  const char* mInitKeyword = ALLOBJ_INIT;

  void readCmd(Stream& stream, BUF& buf, TrackChange trackChange, Decode decode);
  void sendCmdForAllVars(const char* cmdKeyword, Stream& stream, TrackChange trackChange, Decode decode, MyVar::TestFunc testVar = nullptr);
  ArrayIterator(OBJVar, mOBJS, mNOBJ); // not really needed :)

public:
  void init();
  void save(CfgType cfgtype);
  void load(CfgType cfgtype, TrackChange trackChange = TrackChange::yes);
  
  bool addObj(OBJVar& obj, const char* name);
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
