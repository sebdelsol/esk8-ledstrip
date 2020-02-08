#pragma once

#include <FastLED.h>
#include <myPins.h>
#include <Streaming.h>
#include <MyCmd.h>

#define COLOR_ORDER   GRB
#define CHIPSET       WS2812B
#define MAXFX         5
#define MAXSTRIP      3
#define SATURATION    0xff    // for HSV FX

//--------------------------------------
class FX
{
  byte mAlpha = 0; // not visible , call setAlpha

protected:
  int mNLEDS = 0;
  CRGB *mLeds;

public:
  void init(int nLeds);
  virtual void specialInit(int nLeds) {};

  void setAlpha(const byte alpha) { mAlpha = alpha; };
  byte getAlpha() { return mAlpha; };

  void answer(const MyCmd &cmd, byte arg1, byte arg2, byte arg3);
  void answer(const MyCmd &cmd, byte arg);

  virtual const char* getName();
  virtual void setCmd(const MyCmd &cmd);
  virtual void getCmd(const MyCmd &cmd);

  virtual void update();
  bool updateAndScale(CRGB *dst);
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
  void specialInit(int nLeds);
  void update();

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
protected:
  int mPos = 0, mEyeSize, mSpeed;
  CRGB mColor;

public:
  CylonFX(const byte r=0x00, const byte g = 0x00, const byte b = 0xff, const int eyeSize = 3, const int speed = 2<<8);
  void specialInit(int nLeds);
  void update();

  const char* getName() {return "Cylon";};
  void setCmd(const MyCmd &cmd);
  void getCmd(const MyCmd &cmd);
};

//---------
class PulseFX : public FX
{
  byte mHue;
  long mFreq;
  byte mWavelength;

public:
  PulseFX(const byte hue=0, const long frac8=64, const byte w=10);
  void update();

  const char* getName() {return "Pulse";};
  void setCmd(const MyCmd &cmd);
  void getCmd(const MyCmd &cmd);
};

//---------
class TwinkleFX : public FX
{
  byte mHue;
  byte mDiv;

public:
  TwinkleFX(const byte hue=0, const byte d=5);
  void update();

  const char* getName() {return "Pulse";};
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

//--------------------------------------
template <int NLEDS, int LEDPIN>
class LedStrip : public BaseLedStrip
{
  CRGB mDisplay[NLEDS];
  CLEDController *mController;
  char *mName;

  FX *mFX[MAXFX];
  byte mNFX = 0;

public:

  LedStrip(const char* name="")
  {
    mName = (char *)malloc(strlen(name) + 1);
    sprintf(mName, "%s", name);

    mController = &FastLED.addLeds<CHIPSET, LEDPIN, COLOR_ORDER>(mDisplay, NLEDS);
    mController->setCorrection(TypicalSMD5050); // = TypicalLEDStrip
  };

  bool registerFX(FX& fx)
  {
    bool ok = mNFX < MAXFX;
    if (ok) {
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
    Serial << "                  " << endl;
  };

  byte* getData(int& n)
  {
    Serial << "getdata" << NLEDS << endl;
    n = NLEDS * sizeof(CRGB);
    return (byte *) mDisplay;
  };

  void update()
  {
    byte i = 0; // led count

    // direct copy in mDisplay
    for (; i < mNFX; i++) 
      if (mFX[i]->updateAndScale(mDisplay))
        break;

    // copy in tmp then blend
    if (i < mNFX) 
    {
      CRGB tmp[NLEDS];
  
      for (; i < mNFX; i++)
        if (mFX[i]->updateAndScale(tmp))
            for (byte k=0; k < NLEDS; k++)
              mDisplay[k] |= tmp[k]; // = max(mDisplay[k], tmp[k])
    }
    // nothing shown, clear leds
    else 
      mController->clearLedData();
  };

};
