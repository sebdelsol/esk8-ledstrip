#pragma once

#include <FastledCfg.h>
#include <FastLED.h>
#include <Streaming.h>
#include <ObjVar.h>

#define ClearLeds(l, n) memset8(l, 0, n * sizeof(CRGB)); 
#define maxCOLOR (2 << 24)

//--------------------------------------
class FX : public OBJVar
{
  byte mAlpha; 
  byte mLinearAlpha; 

protected:
  int mNLEDS = 0;
  CRGB* mLeds;

public:
  void init(int nLeds);
  void setAlpha(const byte alpha);
  void setAlphaMul(const byte a1, const byte a2);
  byte getAlpha();
  bool drawOn(CRGBSet dst, ulong time, ulong dt);
  
  virtual void update(ulong time, ulong dt)=0;
  virtual void initFX()=0;
};

//--------------------------------------
class FireFX : public FX
{
  bool mReverse;
  byte mSpeed;
  int mDimRatio;
  ushort* mHeat; 

protected:
  CRGBPalette16 mPal;

public:
  FireFX(const bool reverse = false);
  void setDimRatio(const int dimRatio) { mDimRatio = dimRatio; };
  void initFX();
  void update(ulong time, ulong dt);
};

//---------
class AquaFX : public FireFX
{
public:
  AquaFX(const bool reverse = false);
};

//--------------------------------------
class PlasmaFX : public FX
{
  byte mK, mP1, mP2;

public:
  void initFX();
  void update(ulong time, ulong dt);
};

//---------
class CylonFX : public FX
{
protected:
  CRGB mColor;
  int mEyeSize, mSpeed;  

  void showEye(int p); 
  int  getPos(ulong time);

public:
  CylonFX(const CRGB color=0x0000FF);
  void setEyeSize(const int eyeSize) {mEyeSize = eyeSize;};
  void initFX();
  void update(ulong time, ulong dt);
};

//---------
class DblCylonFX : public CylonFX
{
public:
  using CylonFX::CylonFX;
  void update(ulong time, ulong dt);
};

//---------
class RunningFX : public FX
{
protected:
  int mWidth, mSpeed;  
  CRGB mColor;

public:
  RunningFX(const CRGB color=0x0000FF, const int speed = 3);
  void setSpeed(const int speed) {mSpeed = speed;};
  void initFX();
  void update(ulong time, ulong dt);
};

//---------
class TwinkleFX : public FX
{
  CHSV mHSV; CRGB mColor;
  byte mDiv;

  void setHue(const CRGB color);
  void setHue(const byte hue);

public: 
  TwinkleFX(const byte hue=0);
  TwinkleFX(const CRGB color=0xff0000);
  void initFX();
  void update(ulong time, ulong dt);
};

//--------- // by Mark Kriegsman and Mary Corey March.
class PacificaFX : public FX 
{
  CRGBPalette16 mPal1, mPal2, mPal3;
  uint16_t mT1, mT2, mT3, mT4;
  byte mSpeed;

  void oneLayer(CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t waveangle);
  
public: 
  PacificaFX();
  void initFX();
  void update(ulong time, ulong dt);
};
