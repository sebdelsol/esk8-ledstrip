#include <FileObj.h>

//--------------------------------------------------------------------------
void CfgFiles::init()
{
  mNVS.begin("storage");

  _log << "Mount SPIFFS...";
  spiffsOK = SPIFFS.begin(true); // format if failed
  _log << (spiffsOK ? "ok" : "failed") << endl;

#ifdef DBG_SHOWFILES
  if (spiffsOK)
  {
    File root = SPIFFS.open("/");
    while(File file = root.openNextFile())
      _log << "File \"" << file.name() << "\" - " << file.size() << " Bytes" << endl;
  }
#endif
}

//-----------------
FileObjPtr CfgFiles::getCfgFile(CfgType cfgtype, FileMode mode) 
{
  const char* path = cfgtype == CfgType::Default ? CFG_DEFAULT : CFG_CURRENT;
  return FileObjPtr(spiffsOK ? new FileObj(path, mode, mNVS) : nullptr ); 
}

//--------------------------------------------------------------------------
FileObj::FileObj(const char* path, FileMode mode, MyNvs& nvs) : path(path), isloading(mode == FileMode::load), mNVS(nvs)
{
  f = SPIFFS.open(path, isloading ? "r" : "w");
  
  // Spiffs always gives a working file, need to check the size if saving
  isOK = f && (!isloading || f.size()>0) ? true : false; 
  
  if (isOK)
    _log << (isloading ? "Loading from " : "Saving to ") << path << "...";
  else
    _log << "FAIL to " << (isloading ? "load from " : "save to ") << path << endl;
};

//----------------
FileObj::~FileObj()
{
  if (isOK)
  {
    f.close(); 

    _log << (isloading ? "loaded" : "saved") << "...";

    if (mNVS.isOK()) handleCRC();

    _log << endl;
  }
}

//----------------
uint32_t FileObj::getCRC()
{
  CRC32 crc;
  
  for (size_t i = 0; i < f.size(); i++) 
    crc.update(f.read());
  
  return crc.finalize();
}

//----------------
void FileObj::handleCRC()
{
  f = SPIFFS.open(path, "r");
  if (f)
  {
    uint32_t oldcrc;
    uint32_t crc = getCRC();

    if (isloading && mNVS.getuint32(path, oldcrc))
    {
      bool corrupted = crc != oldcrc;
      _log << "check crc..." << (corrupted ? "BAD, delete the file" : "ok");
      if (corrupted) SPIFFS.remove(path); // remove the file !
    }
    else
      _log << "set crc..." << ( mNVS.setuint32(path, crc) ? "ok" : "failed" );

    f.close();
  }
}

