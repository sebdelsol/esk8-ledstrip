#pragma once

#include <Streaming.h>
#include <SPIFFS.h>

#define FORMAT_SPIFFS_IF_FAILED true

#define CFG_ROOT "/"
#define CFG_EXT ".cfg"
#define MAXCFG 10

class BaseCfg
{
public:
  virtual bool load();
  virtual bool save();
  virtual char* getName();
};

// ----------------------------------------------------
class AllConfig
{
  bool mOk;
  BaseCfg *mCFG[MAXCFG];
  byte mNcfg = 0;

  bool isRegistered(const char* name);

public:
  AllConfig();
  bool RegisterCfg(BaseCfg &cfg);
  void cleanUnRegistered();
  void load();
  void save();
};

// AllConfig AllCFG;

// ----------------------------------------------------
template <typename DATASTRUCT, int VER>
class Config : public BaseCfg
{
  char      *mFName;

public:
  DATASTRUCT  mData;

  Config(const char* name)
  {
    mFName = (char *)malloc(strlen(name) + strlen(CFG_ROOT)+ strlen(CFG_EXT) + 1);
    sprintf(mFName, "%s%s%s", CFG_ROOT, name, CFG_EXT);
    // AllConfig.RegisterCfg(&this);
  };

  char* getName() { return mFName; };

  bool load()
  {
    Serial << mFName << " loading...";

    File f = SPIFFS.open(mFName, "r");
    if (f)
    {
      uint8_t ver = f.read();
      if (ver!=VER)
        Serial << "bad Version (" << ver << ") should be (" << VER << ")" << endl;
      else if (f.size()!=sizeof(mData)+1)
        Serial << "wrong size (" << f.size() << ") should be (" << (sizeof(mData)+1) << ")" << endl;
      else {
        f.read((uint8_t *)&mData, sizeof(mData));
        Serial << "ok" << endl;
      }

      f.close();
      return true;
    }

    Serial << "Error Opening" << endl;
    return false;
  };

  bool save()
  {
    Serial << mFName << " saving...";

    File f = SPIFFS.open(mFName, "w");
    if (f)
    {
      f.write(VER);
      f.write((uint8_t *)&mData, sizeof(mData));
      f.close();
      Serial << "ok" << endl;
      return true;
    }

    Serial << "Error Opening" << endl;
    return false;
  };
};
