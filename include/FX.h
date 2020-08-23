#pragma once

#include <FastledCfg.h>
#include <FastLED.h>

#include <Streaming.h>
#include <ObjVar.h>

#define SetFxNAME(name) const char* getName() {return name;};
#define ClearLeds(l, n) memset8(l, 0, n * sizeof(CRGB)); 

//--------------------------------------
class FX : public OBJVar
{
  byte mAlpha = 255; // visible
  byte mLinearAlpha = 255; // no Gamma applied 

protected:
  int mNLEDS = 0;
  CRGB* mLeds;

public:
  void init(int nLeds);
  void setAlpha(const byte alpha);
  void setAlphaMul(const byte a1, const byte a2);
  byte getAlpha();
  bool drawOn(CRGBSet dst, ulong time, ulong dt);
  
  virtual void specialInit(int nLeds) {};
  virtual const char* getName()=0;
  virtual void update(ulong time, ulong dt)=0;
  virtual void registerVars()=0;
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
  FireFX(const bool reverse = false, const byte speed = 27, const int dimRatio = 4);
  void setDimRatio(const int dimRatio) { mDimRatio = dimRatio; };
  void specialInit(int nLeds);
  void registerVars();
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
  void registerVars();
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
  void registerVars();
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
  RunningFX(const CRGB color=0x0000FF, const int speed = 2, const int width = 10);
  void setSpeed(const int speed) {mSpeed = speed;};
  void registerVars();
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
  void registerVars();
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
  void registerVars();
  void update(ulong time, ulong dt);
  SetFxNAME("Pacifica");
};
