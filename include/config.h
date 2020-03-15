
// ----------------------------------------------------
// gow it works:

// typedef struct {
//   int toto = 1;
//   byte tutu = 3;
//   int titi = 201;
// } TstCfg;

// AllConfig AllCFG;
// Config<TstCfg, 1> test("test");

// AllCFG.init();
  // AllCFG.RegisterCfg(test);
  // AllCFG.load();
  // // Serial << test.mData.toto << " " << test.mData.tutu << " " << test.mData.titi << endl;
  // // test.mData.toto = test.mData.toto+1;
  // AllCFG.save();
  // AllCFG.cleanUnRegistered();

#pragma once

#include <Streaming.h>
#include <SPIFFS.h>

#define CFG_ROOT "/"
#define CFG_EXT ".cfg"
#define MAXCFG 10

class BaseCfg
{
public:
  virtual void getDefault();
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
  void getDefault();
  void load();
  void save();
};

// ----------------------------------------------------
template <typename DATASTRUCT, int VER>
class Config : public BaseCfg
{
  char      *mFName;

public:
  DATASTRUCT*  mData;

  DATASTRUCT   mDefault;

  Config(const char* name, DATASTRUCT *data)
  {
    mFName = (char *)malloc(strlen(name) + strlen(CFG_ROOT) + strlen(CFG_EXT) + 1);
    sprintf(mFName, "%s%s%s", CFG_ROOT, name, CFG_EXT);
    mData = data;
    memcpy(&mDefault, data, sizeof(DATASTRUCT));
  };

  char* getName() { return mFName; };

  void getDefault() { memcpy(mData, &mDefault, sizeof(DATASTRUCT)); };

  bool load()
  {
    long start = millis();
    Serial << mFName << " loading...";

    File f = SPIFFS.open(mFName, "r");
    if (f) {
      if (byte ver = f.read()!=VER)
        Serial << "bad Version (" << ver << ") should be (" << VER << ")" << endl;

      else if (int s = f.size()!=sizeof(DATASTRUCT)+1)
        Serial << "wrong size (" << s << ") should be (" << (sizeof(mData)+1) << ")" << endl;

      else {
        f.read((byte *)mData, sizeof(DATASTRUCT));
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
      f.write((byte *)mData, sizeof(DATASTRUCT));
      f.close();
      Serial << "ok, took " << (millis() - start) << "ms" << endl;
      return true;
    }

    Serial << "Error Opening" << endl;
    return false;
  };
};
