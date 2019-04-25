#pragma once

#include <FastLED.h>
#include <myPins.h>
#include <Streaming.h>
#include <MyCmd.h>

#define COLOR_ORDER   GRB
#define CHIPSET       WS2812B
#define MAXFX         5
#define MAXSTRIP      3

//--------------------------------------
class FX
{
  byte mAlpha = 0; // not visible , call setAlpha

protected:
  int mNLEDS = 0;
  CRGB *mLeds;

public:
  void init(int nLeds) {mNLEDS = nLeds; mLeds = (CRGB *)malloc(nLeds * sizeof(CRGB)); specialInit(nLeds);};
  virtual void specialInit(int nLeds) {};

  void setAlpha(const byte alpha) { mAlpha = alpha; };
  byte getAlpha() { return mAlpha; };

  void answer(const MyCmd &cmd, byte arg1, byte arg2, byte arg3);
  void answer(const MyCmd &cmd, byte arg);

  virtual const char* getName();
  virtual void setCmd(const MyCmd &cmd);
  virtual void getCmd(const MyCmd &cmd);

  CRGB* updateAndFade();
  virtual void update();
};

//--------------------------------------
class FireFX : public FX
{
  byte mCooling, mSparkling;
  byte *mHeat;

protected:
  bool mReverse;
  CRGBPalette16 mPal;

public:
  FireFX(const bool reverse = false, const byte cooling = 75, const byte sparkling = 120);
  void update();

  void specialInit(int nLeds) {mHeat = (byte*)malloc(nLeds*sizeof(byte));};
  const char* getName() {return "Fire";};
  void setCmd(const MyCmd &cmd);
  void getCmd(const MyCmd &cmd);
};

//---------
class AquaFX : public FireFX
{
public:
  AquaFX(const bool reverse, const byte cooling = 75, const byte sparkling = 120);
  const char* getName() {return "Aqua";};
};

//---------
class PlasmaFX : public FX
{
  byte mK, mP1, mP2;

public:
  PlasmaFX(const byte wavelenght = 5, const byte period1 = 3, const byte period2 = 5);
  void update();

  const char* getName() {return "Plasma";};
  void setCmd(const MyCmd &cmd);
  void getCmd(const MyCmd &cmd);
};

//---------
class CylonFX : public FX
{
  int mPos = 0, mEyeSize, mSpeed;
  CRGB mColor;

public:
  CylonFX(const byte r=0x00, const byte g = 0x00, const byte b = 0xff, const int eyeSize = 3, const int speed = 2<<8);
  void update();

  const char* getName() {return "Cylon";};
  void setCmd(const MyCmd &cmd);
  void getCmd(const MyCmd &cmd);
};

//--------------------------------------
class BaseLedStrip
{
public:
  virtual void getInfo(); // for AllLedStrips
  virtual void update();  // for AllLedStrips
  virtual byte* getData(int& n); // for myWifi
};

//--------------------------------------
template <int NLEDS, int LEDPIN>
class LedStrip : public BaseLedStrip
{
  CRGB mLeds[NLEDS];
  CLEDController *mController;

  FX *mFX[MAXFX];
  byte mNFX = 0;

  String mName;

public:

  LedStrip(const String name = String()) : mName(name)
  {
    mController = &FastLED.addLeds<CHIPSET, LEDPIN, COLOR_ORDER>(mLeds, NLEDS); //.setCorrection(TypicalSMD5050); // = TypicalLEDStrip
  };

  bool registerFX(FX& fx)
  {
    bool ok = mNFX < MAXFX;
    if (ok){
      mFX[mNFX++] = (FX*)&fx;
      fx.init(NLEDS);
    }
    return ok;
  };

  void getInfo()
  {
    Serial << mName;
    for (byte i=0; i < mNFX; i++) {
      FX *fx = mFX[i];
      Serial << " - " << fx->getName() << "(" << fx->getAlpha() << ")";
    }
    Serial << "                  \n";
  };

  byte* getData(int& n)
  {
    Serial << "getdata" << NLEDS << endl;
    n = NLEDS * sizeof(CRGB);
    return (byte*) mLeds;
  };

  void update()
  {
    // copy then blend available fx
    bool shown = false;

    for (byte i=0; i < mNFX; i++) {
      CRGB *src = mFX[i]->updateAndFade();

      if (src!=NULL) {
        if (!shown) { // copy first
          memcpy8(mLeds, src, NLEDS * sizeof(CRGB));
          shown = true;
        }
        else // then blend
          for (byte k=0; k < NLEDS; k++)
            mLeds[k] += src[k];
      }
    }

    if(!shown) // clear if no FX shown
      mController->clearLedData();
  };
};

class AllLedStrips
{
  BaseLedStrip *mStrips[MAXSTRIP];
  byte mNStrips = 0;

public:
  AllLedStrips(const int maxmA = 2000);
  void setBrightness(const byte scale) { FastLED.setBrightness(scale); };
  byte getBrightness() { return FastLED.getBrightness(); };
  void show() { FastLED.show(); };

  bool registerStrip(BaseLedStrip &strip);
  void getInfo();
  void update();
};

//---------
// class Fire2 : public FX
// {
//   ulong mXscale = 20;  // How far apart they are in perlin noise
//   ulong mSpeed = 3;   // How fast they move
//
// protected:
//   bool mReverse = false;
//   CRGBPalette16 mPal;
//
// public:
//   Fire2(const bool reverse = false);
//   const char* getName() {return "Fire2";};
//   void update();
// };
//
// //---------
// class Aqua2 : public Fire2
// {
// public:
//   Aqua2(const bool reverse = false);
//   const char* getName() {return "Aqua2";};
// };
