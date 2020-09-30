#pragma once

#include <FastledCfg.h>
#include <FastLED.h>
#include <log.h>
#include <Pins.h>
#include <ObjVar.h>
#include <FX.h>
#include <AllObj.h>
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
  virtual void  showInfo(); 
  virtual void  update(ulong time, ulong dt);  
  virtual void  init();
  virtual void  addObjs(AllObj& allobj);
  virtual byte* getRawData(); // for myWifi
  virtual int   getRawLength(); // for myWifi
};

//---------
class AllLedStrips : public OBJVar
{
  BaseLedStrip* mStrips[MAXSTRIP];
  byte          mNStrips  = 0;

  ulong     mLastT      = 0;
  bool      mDither     = true;
  int       mMaxmA      = 800;
  byte      mBright     = 255; // half brightness (128) is enough & avoid reaching maxmA
  byte      mRawBright  = 0;   // with fade 
  int       mFade       = 0;   // for the fade in
  int       mFadeTime   = 1000; // ms
  int       mMinProbe   = 400;
  const int mMaxProbe   = 4095;
  bool      mProbe      = false;

  // time to show & fadein
  bool      mHasbegun   = false;
  long      mBeginTime; 

public:
  AllLedStrips();
  void init();

  ArrayOfPtr_Iter(BaseLedStrip, mStrips, mNStrips); 
  byte getRawBrightness() { return mRawBright; };
  void setBrightness(const byte bright);
  void setDither(const bool dither)     { FastLED.setDither(dither ? BINARY_DITHER : DISABLE_DITHER); };
  void setMaxmA(const int maxmA)        { FastLED.setMaxPowerInVoltsAndMilliamps(5, maxmA); };
  
  void show();

  bool addStrip(BaseLedStrip& strip);
  ForEachMethod(addStrip); // create a method addStrips(...) that calls addStrip on all args
  
  void addObjs(AllObj& allobj);

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
  CLEDController*  mController;
  const char*      mName;

  FX*              mFX[MAXFX];
  byte             mNFX = 0;
  ArrayOfPtr_Iter(FX, mFX, mNFX); 

public:

  LedStrip(const char* name) : mName(name) {};

  void init() // better for startup, no blinking, fastled is initialized before with 0 brightness
  {
    mController = &FastLED.addLeds<CHIPSET, LEDPIN, COLOR_ORDER>(mDisplay, NLEDS);
    mController->setCorrection(TypicalSMD5050); // = TypicalLEDStrip
    FastLED.clear(true); // clear all to avoid blinking leds startup 
  };

  bool addFX(FX& fx, const char* name)
  {
    bool ok = mNFX < MAXFX;
    if (ok)
    {
      mFX[mNFX++] = &fx;

      char* fxname = (char *)malloc(strlen(mName) + strlen(name) + 2);
      assert(fxname != nullptr);
      sprintf(fxname, "%s.%s", mName, name); // fxname is strip.fx
      fx.setName(fxname);

      fx.init(NLEDS);
    }
    else
      _log << ">> ERROR !! Max FX is reached " << MAXFX << endl; 

    return ok;
  };

  ForEachMethodPairs(addFX); // create a method addFXs(fx1, name1, fx2, name2, ...) that calls addFX(fx, name) for each pair

  void addObjs(AllObj& allobj)
  {
    for (auto fx : *this) allobj.addObj(*fx, fx->getName());
  };

  void showInfo()
  {
    _log << _WIDTH(NLEDS,3) << " leds";
    for (auto fx : *this) _log << " - " << _WIDTH(fx->getName(), 16) << " " << _WIDTH(fx->getAlpha(), 3);
    _log << endl;
  };

  int   getRawLength() { return NLEDS * sizeof(CRGB); };
  byte* getRawData()   { return (byte* ) mDisplay.leds; };

  void update(ulong t, ulong dt)
  {
    bool first = true;

    for (auto fx : *this) 
      if (fx->drawOn(first ? mDisplay : mBuffer, t, dt)) // first drawn on mDisplay, then on mBuffer
      {
        if (first) first = false;
        else nblend(mDisplay.leds, mBuffer.leds, NLEDS, 128); //average of the 2
      } 

    // if no fx drawn, clear the ledstrip
    if (first) mController->clearLedData();
  };
};


