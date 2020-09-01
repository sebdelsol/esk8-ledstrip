#pragma once

#include <FastledCfg.h>
#include <FastLED.h>
#include <log.h>
#include <Pins.h>
#include <ObjVar.h>
#include <FX.h>
#include <Variadic.h>

#define COLOR_ORDER     GRB
#define CHIPSET         WS2812B
#define MAXFX           5
#define MAXSTRIP        3

//------------------- FastLED.show() run in a task
// #define FASTLED_CORE  1
// #define FASTLED_PRIO  (configMAX_PRIORITIES - 1)
// #define FASTLED_WAIT  100 //ms
// #define FASTLED_STACK 2048

//--------------------------------------
class BaseLedStrip
{
public:
  virtual void showInfo(); 
  virtual void update(ulong time, ulong dt);  
  virtual void init();
  virtual byte* getData(int& n); // for myWifi
};

//---------
class AllLedStrips : public OBJVar
{
  BaseLedStrip* mStrips[MAXSTRIP];
  byte      mNStrips  = 0;
  ulong     mLastT    = 0;

  bool      mDither   = true;
  int       mMaxmA    = 800;
  byte      mBright   = 255;  // half brightness (128) is enough & avoid reaching maxmA
  int       mFade     = 0;    // for the fade in
  int       mMinProbe = 400;
  const int mMaxProbe = 4095;
  bool      mProbe    = false;

public:
  AllLedStrips();
  void init();

  void setBrightness(const byte bright);
  void setDither(const bool dither)     { FastLED.setDither(dither ? BINARY_DITHER : DISABLE_DITHER); };
  void setMaxmA(const int maxmA)        { FastLED.setMaxPowerInVoltsAndMilliamps(5, maxmA); };
  
  void switchOff();
  void show();

  bool addStrip(BaseLedStrip& strip);
  ForEachMethod(addStrip); // create method addStrips(...) that calls addStrip on all args

  void showInfo();
  void update();
  bool doDither();
};

//--------------------------------------
template <int NLEDS, int LEDPIN>
class LedStrip : public BaseLedStrip
{
  CRGBArray<NLEDS> mBuffer; // tmp buffer for copying & fading of each fx
  CRGBArray<NLEDS> mDisplay; // target display

  CLEDController* mController;
  char* mName;

  FX* mFX[MAXFX];
  byte mNFX = 0;

public:

  LedStrip(const char* name="")
  {
    mName = (char* )malloc(strlen(name) + 1);
    assert (mName!=nullptr);
    sprintf(mName, "%s", name);
  };

  void init() // better for startup, no blinking, fastled is initialized before with 0 brightness
  {
    mController = &FastLED.addLeds<CHIPSET, LEDPIN, COLOR_ORDER>(mDisplay, NLEDS);
    mController->setCorrection(TypicalSMD5050); // = TypicalLEDStrip
  };

  bool addFX(FX& fx)
  {
    bool ok = mNFX < MAXFX;
    if (ok)
    {
      mFX[mNFX++] = &fx;
      fx.init(NLEDS);
    }
    else
      _log << ">> ERROR !! Max FX is reached " << MAXFX << endl; 

    return ok;
  };

  ForEachMethod(addFX);   // create method AddFxs(...) that calls addFX on all args

  void showInfo()
  {
    _log << mName << "(" << NLEDS << ") ";
    for (byte i=0; i < mNFX; i++)
    {
      FX* fx = mFX[i];
      _log << " - " << fx->getName() << "(" << fx->getAlpha() << ")";
    }
    _log << "                  " << endl;
  };

  byte* getData(int& n)
  {
    n = NLEDS * sizeof(CRGB);
    return (byte* ) mDisplay.leds;
  };

  void update(ulong t, ulong dt)
  {
    byte i = 0; // fx count

    // 1st fx is drawn on mDisplay
    for (; i < mNFX; i++) 
      if (mFX[i]->drawOn(mDisplay, t, dt))
        break; // now we've to blend

    // something drawn ?
    if (++i <= mNFX)
    { 
      // some fx left to draw ? draw on mBuffer & blend with mDisplay
      for (; i < mNFX; i++) 
        if (mFX[i]->drawOn(mBuffer, t, dt)) 
            mDisplay |= mBuffer; // get the max of each RGB component
    }
    // if no fx drawn, clear the ledstrip
    else 
      mController->clearLedData();
  };
};
