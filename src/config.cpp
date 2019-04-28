#include <config.h>

AllConfig::AllConfig()
{
  Serial << "SPIFFS begin" << endl;
  mOk = SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED);
  if (!mOk)
    Serial << "SPIFFS Mount Failed" << endl;
}

bool AllConfig::isRegistered(const char* name)
{
  for (byte i=0; i < mNcfg; i++)
    if (strcmp(mCFG[i]->getName(), name)==0)
      return true;
  return false;
}

void AllConfig::cleanUnRegistered()
{
  #define ROOT "/"
  if (mOk){
    Serial << "Clean unregistered config" << endl;
    File root = SPIFFS.open(CFG_ROOT);
    File f = SPIFFS.open("/toto", "w");
    f.close();

    File file = root.openNextFile();
    while(file){
      const char* name = file.name();
      if (!isRegistered(name)){
        Serial << name << "...removed" << endl;
        SPIFFS.remove(name);
      }
      file = root.openNextFile();
    }
  }
}

bool AllConfig::RegisterCfg(BaseCfg &cfg)
{
  bool ok = mNcfg < MAXCFG;
  if (ok)
    mCFG[mNcfg++] = (BaseCfg*)&cfg;
  return ok;
}

void AllConfig::load()
{
  if (mOk)
    for (byte i=0; i < mNcfg; i++)
      mCFG[i]->load();
}

void AllConfig::save()
{
  if (mOk)
    for (byte i=0; i < mNcfg; i++)
      mCFG[i]->save();
}
