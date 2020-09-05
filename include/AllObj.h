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

#define FORMAT_SPIFFS_IF_FAILED true

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

  struct mOBJ
  {
    char*        name;
    OBJVar*      obj;
  };

  HashName<ALLOBJ_MAXOBJ, mOBJ> mHash;
  mOBJ  mOBJS[ALLOBJ_MAXOBJ];
  byte  mNOBJ = 0;
  byte  mID = 0;

  const char* cfg_fname = FNAME_CURRENT;
  const char* def_fname = FNAME_DEFAULT;
  bool        spiffsOK = false;
  BUF         mTmpBuf;
  
  void    dbgCmd(const char* cmdKeyword, const parsedCmd& parsed, int nbArg, int* args, bool  line);
  void    dbgCmd(const char* cmdKeyword, const parsedCmd& parsed, int nbArg, int* args, int min, int max);

  bool    isNumber(const char* txt);
  OBJVar* getObjFromName(const char* name);

  void    handleSetCmd(const parsedCmd& parsed, BUF& buf, bool change);
  void    handleGetCmd(const parsedCmd& parsed, Stream& stream, bool compact);
  void    handleInitCmd(const parsedCmd& parsed, Stream& stream);
  bool    parseCmd(parsedCmd& parsed, BUF& buf);
  void    handleCmd(Stream& stream, BUF& buf, bool change = true, bool compact = false);
  File    getFile(bool isdefault, const char* mode);

protected:
  const char* mSetKeyword = ALLOBJ_SET;
  const char* mGetKeyword = ALLOBJ_GET;
  const char* mInitKeyword = ALLOBJ_INIT;

  void readAndHandleCmd(Stream& stream, BUF& buf, bool change = true, bool compact = false);
  void emulateCmdForAllVars(const char* cmdKeyword, Stream& stream, OBJVar::ObjTestVarFunc testVar = nullptr, bool change = true, bool compact = false);

public:
  void init();
  void save(bool isdefault);
  void load(bool isdefault, bool change = true);
  bool addObj(OBJVar& obj, const char* name);
};

// macro to add several objs to allobj 
#define _addObj(allobj, obj)  allobj.addObj(obj, #obj);
#define AddObjs(allobj, ...)  ForEachMacro(_addObj, allobj, __VA_ARGS__)
