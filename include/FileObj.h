#pragma once

#include <SPIFFS.h>
#include <CRC32.h>
#include <myNVS.h>
#include <variadic.h>

//------------------- Dbg
#define DBG_SHOWFILES  // to see files in SPIFFS

static auto CFG_CURRENT = "/config.cfg";
static auto CFG_DEFAULT = "/config.def";

enum class CfgType :  bool { Default, Current };
enum class FileMode : bool { save, load };

//-------------------------------
class FileObj
{
  bool        isloading;
  MyNvs&      mNVS; // to load/save CRC
  File        f;
  
  uint32_t  getCRC();
  void      handleCRC();
  void      remove();

public:
  FileObj(const char* path, FileMode mode, MyNvs& nvs);
  ~FileObj();

  bool    ok() { return f ? true : false; };
  Stream& getStream() { return (Stream&)f; };
};

//-------------------------------
using FileObjPtr = std::unique_ptr<FileObj>;

class CfgFiles
{
  bool        spiffsOK = false;
  MyNvs       mNVS;

public:
  void       init();
  FileObjPtr getCfgFile(CfgType cfgtype, FileMode mode);
};

