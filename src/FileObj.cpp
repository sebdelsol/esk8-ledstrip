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
FileObj::FileObj(const char* path, FileMode mode, MyNvs& nvs) : isloading(mode == FileMode::load), mNVS(nvs)
{
  if (!isloading || SPIFFS.exists(path)) // might not exists if saving
  {
    f = SPIFFS.open(path, isloading ? "r" : "w+"); // need to be readeable to compute crc
    if (f)
    {
      _log << (isloading ? "Loading from " : "Saving to ") << path << "...";
      return;
    }
  }

  mNVS.erase(path);
  _log << "FAIL to " << (isloading ? "load from " : "save to ") << path << endl;
};

//----------------
FileObj::~FileObj()
{
  if (f)
  {
    _log << (isloading ? "loaded" : "saved") << "...";

    if (mNVS.isOK()) handleCRC();

    _log << endl;

    // no need to manually close the file
    // it's already done in file destructor
  }
}

//----------------
void FileObj::remove()
{
  char* path = strdup(f.name());

  _log << "delete the file...";

  f.close(); // close the file before removing it
  
  bool deleted = path != nullptr && SPIFFS.remove(path);
  
  if (deleted)
  {
    mNVS.erase(path);
    free(path);
  }

  _log << ( deleted ? "ok" : "failed" );
}

//----------------
uint32_t FileObj::getCRC()
{
  CRC32 crc;
  
  f.flush();
  f.seek(0, SeekMode::SeekSet);
  
  for (size_t i = 0; i < f.size(); i++) 
    crc.update(f.read());
  
  return crc.finalize();
}

//----------------
void FileObj::handleCRC()
{
  uint32_t crc = getCRC();

  _log << ( isloading ? "check" : "set" ) << " crc...";

  if (isloading)
  {
    uint32_t oldcrc;
    
    if (mNVS.getuint(f.name(), oldcrc))
    {
      bool corrupted = crc != oldcrc;
    
      _log << ( corrupted ? "BAD..." : "ok" );
      if (corrupted) remove();
    
      return;
    }

    _log << "no crc...set...";
  }

  _log << ( mNVS.setuint(f.name(), crc) ? "ok" : "failed" );
}

