#pragma once

#include <log.h>
#include <SPIFFS.h>
#include <Variadic.h>
#include <ObjVar.h>
#include <Buf.h>

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

#define FNAME_CURRENT     "/config.cfg"
#define FNAME_DEFAULT     "/config.def"

enum class CfgFile : bool { Default, Current };
enum class Decode : uint8_t { compact, verbose, undefined };

//-------------------------------
class AllObj 
{
  struct parsedCmd 
  {
    OBJVar*      obj;
    MyVar*       var;
    const char*  objName;
    const char*  varName;
  };

  HashName<ALLOBJ_MAXOBJ, OBJVar> mHash;
  OBJVar*     mOBJS[ALLOBJ_MAXOBJ];
  byte        mNOBJ = 0;
  byte        mID = 0;

  const char* cfg_fname = FNAME_CURRENT;
  const char* def_fname = FNAME_DEFAULT;
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
  File    getFile(CfgFile cfgfile, const char* mode);

protected:
  const char* mSetKeyword = ALLOBJ_SET;
  const char* mGetKeyword = ALLOBJ_GET;
  const char* mInitKeyword = ALLOBJ_INIT;

  void readCmd(Stream& stream, BUF& buf, TrackChange trackChange, Decode decode);
  void sendCmdForAllVars(const char* cmdKeyword, Stream& stream, TrackChange trackChange, Decode decode, OBJVar::ObjTestVarFunc testVar = nullptr);

public:
  void init();
  void save(CfgFile cfgfile);
  void load(CfgFile cfgfile, TrackChange trackChange = TrackChange::yes);
  
  bool addObj(OBJVar& obj, const char* name);
  ForEachMethodPairs(addObj);  // create a method addObjs(obj1, name1, obj2, name2, ...) that calls addObj(obj, name) for each pair
};