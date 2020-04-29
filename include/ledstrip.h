#pragma once

#include <FastLED.h>
#include <myPins.h>
#include <Streaming.h>
#include <objVar.h>

#define COLOR_ORDER     GRB
#define CHIPSET         WS2812B
#define MAXFX           6
#define MAXSTRIP        3
#define SATURATION      0xff    // for HSV FX
#define CLEAR_LED(l, n) memset8(l, 0, n * sizeof(CRGB)); 

//--------------------------------------
class FX : public OBJVar
{
  byte mAlpha = 255; // visible
  byte mLinearAlpha = 255; // no Gamma applied 

protected:
  int mNLEDS = 0;
  CRGB *mLeds;

public:
  FX();
  void init(int nLeds);
  virtual void specialInit(int nLeds) {};

  void setAlpha(const byte alpha);
  byte getAlpha();

  virtual const char* getName()=0;
  virtual void update(ulong time, ulong dt)=0;
  bool drawOn(CRGBSet dst, ulong time, ulong dt);
};

#define SetFxNAME(name) const char* getName() {return name;};

//--------------------------------------
class FireFX : public FX
{
  bool mReverse;
  byte mSpeed;
  float mDimRatio;
  ushort *mHeat; 

protected:
  CRGBPalette16 mPal;

public:
  FireFX(const bool reverse = false, const byte speed = 27, const float dimRatio = 10.);
  void specialInit(int nLeds);
  void update(ulong time, ulong dt);
  SetFxNAME("Fire");
};

//---------
class AquaFX : public FireFX
{
public:
  AquaFX(const bool reverse = false, const byte speed = 27, const float dimRatio = 10.);
  SetFxNAME("Aqua");
};

//--------------------------------------
class PlasmaFX : public FX
{
  byte mK, mP1, mP2;

public:
  PlasmaFX(const byte wavelenght = 5, const byte period1 = 3, const byte period2 = 5);
  void update(ulong time, ulong dt);
  SetFxNAME("Plasma");
};

//---------
class CylonFX : public FX
{
protected:
  int mEyeSize, mSpeed;  
  CRGB mColor;

  void showEye(int p); 
  int  getPos(ulong time);

public:
  CylonFX(const CRGB color=0x0000FF, const int eyeSize = 3, const int speed = 3<<3);
  void setEyeSize(const int eyeSize) {mEyeSize = eyeSize;};
  void update(ulong time, ulong dt);
  SetFxNAME("Cylon");
};

//---------
class DblCylonFX : public CylonFX
{
public:
  using CylonFX::CylonFX;
  void update(ulong time, ulong dt);
  SetFxNAME("DbldCylon");
};

//---------
class RunningFX : public FX
{
protected:
  int mWidth, mSpeed;  
  CRGB mColor;

public:
  RunningFX(const CRGB color=0x0000FF, const int width = 5, const int speed = 2);
  void setSpeed(const int speed) {mSpeed = speed;};
  void update(ulong time, ulong dt);
  SetFxNAME("Running");
};

//---------
class TwinkleFX : public FX
{
  CHSV mHSV; CRGB mColor;
  byte mHueDiv;
  byte mDiv;

  void setHue(const CRGB color);
  void setHue(const byte hue);

public: 
  TwinkleFX(const byte hue=0, const byte hueDiv=5, const byte div=5);
  TwinkleFX(const CRGB color=0xff0000, const byte hueDiv=5, const byte div=5);
  void registerAllCmd();
  void update(ulong time, ulong dt);
  SetFxNAME("Twinkle");
};

//--------- // by Mark Kriegsman and Mary Corey March.
class PacificaFX : public FX 
{
  CRGBPalette16 mPal1, mPal2, mPal3;
  uint16_t mT1, mT2, mT3, mT4;
  byte mSpeed;

  void oneLayer(CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t waveangle);
  
public: 
  PacificaFX(const byte speed = 4);
  void update(ulong time, ulong dt);
  SetFxNAME("Pacifica");
};

//--------------------------------------
class BaseLedStrip
{
protected:
  Stream* mSerial;
public:
  virtual void getInfo(); 
  virtual void update(ulong time, ulong dt);  
  virtual byte* getData(int& n); // for myWifi
  void setSerial(Stream *serial) {mSerial = serial;};
};

//---------
class AllLedStrips
{
  BaseLedStrip *mStrips[MAXSTRIP];
  byte mNStrips = 0;
  ulong  mLastT = 0;
  Stream* mSerial;

public:
  AllLedStrips(const int maxmA, Stream &serial);
  void setBrightness(const byte scale) { FastLED.setBrightness(scale); };
  byte getBrightness() { return FastLED.getBrightness(); };
  void show() { FastLED.show(); };
  void clearAndShow();

  bool registerStrip(BaseLedStrip &strip);
  void getInfo();
  void update();
};

//--------------------------------------

template <int NLEDS, int LEDPIN>
class LedStrip : public BaseLedStrip
{
  CRGBArray<NLEDS> mBuffer; // tmp buffer for copying & fading of each fx
  CRGBArray<NLEDS> mDisplay; // target display

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
    if (ok)
    {
      mFX[mNFX++] = &fx;
      fx.init(NLEDS);
    }
    return ok;
  };

  void getInfo()
  {
    *mSerial << mName;
    for (byte i=0; i < mNFX; i++)
    {
      FX *fx = mFX[i];
      *mSerial << " - " << fx->getName() << "(" << fx->getAlpha() << ")";
    }
    *mSerial << "                  " << endl;
  };

  byte* getData(int& n)
  {
    // *mSerial << "getdata" << NLEDS << endl;
    n = NLEDS * sizeof(CRGB);
    return (byte *) mDisplay.leds;
  };

  // void applyGamma(CRGB* leds) // fast gamma = 2
  // {
  //   for (byte i = 0; i < NLEDS; i++)
  //   {
  //     leds[i].r = dim8_video(leds[i].r);
  //     leds[i].g = dim8_video(leds[i].g);
  //     leds[i].b = dim8_video(leds[i].b);
  //   }
  // };

  void update(ulong time, ulong dt)
  {
    byte i = 0; // fx count

    // 1st fx is drawn on mDisplay
    for (; i < mNFX; i++) 
      if (mFX[i]->drawOn(mDisplay, time, dt))
        break; // now we've to blend

    // some fx left to draw ?
    if (++i <= mNFX)
    { // draw on mBuffer & blend with mDisplay
      for (; i < mNFX; i++) 
        if (mFX[i]->drawOn(mBuffer, time, dt)) 
            mDisplay |= mBuffer; // get the max of each RGB component
      
      // applyGamma(mDisplay.leds);
    }
    // if no fx drawn, clear the ledstrip
    else 
      mController->clearLedData();
  };

};

