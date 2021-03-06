#include <FileObj.h>

//--------------------------------------------------------------------------
inline bool logTst(bool tst, const char* task, const char* ok, const char* failed)
{
  _log << "..." << task << "..." << (tst ? ok : failed);
  return tst;
}

//--------------------------------------------------------------------------
void CfgFiles::init()
{
  mNVS.begin("storage");
  spiffsOK = logTst(SPIFFS.begin(true), "Mount FS", "ok", "failed");
  _log << endl;

  if (spiffsOK)
  {
    File root = SPIFFS.open("/");
    while(File file = root.openNextFile())
      _log << "File " << file.name() << " - " << file.size() << " Bytes" << endl;
  }
}

//-----------------
FileObjPtr CfgFiles::getCfgFile(CfgType cfgtype, FileMode mode) 
{
  const char* path = cfgtype == CfgType::Default ? CFG_DEFAULT : CFG_CURRENT;
  return FileObjPtr( spiffsOK ? new FileObj(path, mode, mNVS) : nullptr ); 
}

//--------------------------------------------------------------------------
FileObj::FileObj(const char* path, FileMode mode, MyNvs& nvs) : isloading(mode == FileMode::load), mNVS(nvs)
{
  if (!isloading || SPIFFS.exists(path)) // might not exists if saving
  {
    f = SPIFFS.open(path, isloading ? "r" : "w+"); // need to be readable to compute crc
    if (f)
    {
      _log << (isloading ? "Loading " : "Saving  ") << path << "...";
      return;
    }
  }

  mNVS.erase(path);
  _log << "FAIL to " << (isloading ? "load " : "save ") << path << endl;
};

//----------------
FileObj::~FileObj()
{
  if (f)
  {
    _log << (isloading ? "loaded" : "saved ");
    if (mNVS.isOK()) handleCRC();
    _log << endl;
    // no need to close the file, it's already done in the file destructor
  }
}

//----------------
void FileObj::remove()
{
  char* path = strdup(f.name());
  if (path != nullptr)
  {
    f.close(); // close the file before removing it
    
    if (logTst(SPIFFS.remove(path), "delete file", "ok", "failed"))
      mNVS.erase(path);

    free(path);
  }
}

//----------------
uint32_t FileObj::getCRC()
{
  f.flush();
  f.seek(0, SeekMode::SeekSet);
  
  CRC32 crc;
  for (size_t i = 0; i < f.size(); i++) crc.update(f.read()); 
  return crc.finalize();
}

//----------------
void FileObj::handleCRC()
{
  uint32_t crc = getCRC();

  if (isloading)
  {
    uint32_t oldcrc;    
    if (logTst(mNVS.getuint(f.name(), oldcrc), "crc", "found", "doesn't exist"))
    {
      if (logTst(crc != oldcrc, "check", "BAD", "ok")) remove();
      return;
    }
  }

  logTst(mNVS.setuint(f.name(), crc), "set crc", "ok", "failed");
}

