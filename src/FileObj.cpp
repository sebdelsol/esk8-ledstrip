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
  return FileObjPtr( spiffsOK ? new FileObj(path, mode, mNVS) : nullptr ); 
}

//--------------------------------------------------------------------------
FileObj::FileObj(const char* path, FileMode mode, MyNvs& nvs) : path(path), isloading(mode == FileMode::load), mNVS(nvs)
{
  if (!isloading || SPIFFS.exists(path))
  {
    f = SPIFFS.open(path, isloading ? "r" : "w+"); // need to be readeable to compute crc
    if (f)
    {
      _log << (isloading ? "Loading from " : "Saving to ") << path << "...";
      return;
    }
  }
  _log << "FAIL to " << (isloading ? "load from " : "save to ") << path << endl;
};

//----------------
FileObj::~FileObj()
{
  if (f)
  {
    _log << (isloading ? "loaded" : "saved") << "...";

    if (mNVS.isOK())
    {
      f.flush();
      f.seek(0, SeekMode::SeekSet);
      handleCRC();
    }
    // f.close(); // already done in file destructor

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
  uint32_t oldcrc;
  uint32_t crc = getCRC();

  _log << ( isloading ? "check" : "set" ) << " crc...";

  if (isloading && mNVS.getuint32(path, oldcrc))
  {
    if (crc != oldcrc) 
    {
      _log << "BAD, delete the file";
      SPIFFS.remove(path); 
    }
    else      
      _log << "ok";
  }
  else
    _log << ( mNVS.setuint32(path, crc) ? "ok" : "failed" );
}

