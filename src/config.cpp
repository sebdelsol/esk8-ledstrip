#include <config.h>

void AllConfig::init()
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
  if (mOk){
    Serial << "Clean unregistered config" << endl;
    File root = SPIFFS.open(CFG_ROOT);

    while( File f=root.openNextFile() ) {
      const char* name = f.name();
      if (!isRegistered(name)){
        Serial << name << "...removed" << endl;
        SPIFFS.remove(name);
      }
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
