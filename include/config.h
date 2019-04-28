#pragma once

#include <Streaming.h>
#include <SPIFFS.h>

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
  void init();
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
    mFName = (char *)malloc(strlen(name) + strlen(CFG_ROOT) + strlen(CFG_EXT) + 1);
    sprintf(mFName, "%s%s%s", CFG_ROOT, name, CFG_EXT);
  };

  char* getName() { return mFName; };

  bool load()
  {
    long start = millis();
    Serial << mFName << " loading...";

    File f = SPIFFS.open(mFName, "r");
    if (f) {
      if (byte ver = f.read()!=VER)
        Serial << "bad Version (" << ver << ") should be (" << VER << ")" << endl;

      else if (int s = f.size()!=sizeof(mData)+1)
        Serial << "wrong size (" << s << ") should be (" << (sizeof(mData)+1) << ")" << endl;

      else {
        f.read((byte *)&mData, sizeof(mData));
        Serial << "ok, took " << (millis() - start) << "ms" << endl;
      }

      f.close();
      return true;
    }

    Serial << "Error Opening" << endl;
    return false;
  };

  bool save()
  {
    long start = millis();
    Serial << mFName << " saving...";

    File f = SPIFFS.open(mFName, "w");
    if (f) {
      f.write(VER);
      f.write((byte *)&mData, sizeof(mData));
      f.close();
      Serial << "ok, took " << (millis() - start) << "ms" << endl;
      return true;
    }

    Serial << "Error Opening" << endl;
    return false;
  };
};
