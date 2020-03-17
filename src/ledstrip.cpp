#include <ledstrip.h>

// ----------------------------------------------------
void FX::init(int nLeds)
{
  mNLEDS = nLeds;
  mLeds = (CRGB *)malloc(nLeds * sizeof(CRGB));
  CLEAR_LED(mLeds, nLeds);
  specialInit(nLeds);

  REGISTER_CMD(FX,  "alpha", { self->setAlpha(arg0); },  self->getAlpha(), 0, 255)
}

void FX::setAlpha(const byte alpha) 
{ 
  mAlpha = (alpha*alpha)>>8; 
  mLinearAlpha = alpha; 
}

byte FX::getAlpha() 
{ 
  return mLinearAlpha; 
}

bool FX::drawOn(CRGBSet dst)
{
  if (mAlpha > 0) { // 0 is invisible
    update();
    memcpy8(dst.leds, mLeds, mNLEDS * sizeof(CRGB));
    
    if (mAlpha < 255) // 255 = no fade
      dst.nscale8_video(mAlpha);
    
    return true;
  }
  return false;
}

// ----------------------------------------------------
PlasmaFX::PlasmaFX(const byte wavelenght, const byte period1, const byte period2) : mK(wavelenght), mP1(period1), mP2(period2) 
{
  REGISTER_CMD_SIMPLE(PlasmaFX,  "p1",    self->mP1, 1, 255)
  REGISTER_CMD_SIMPLE(PlasmaFX,  "p2",    self->mP2, 1, 255)
  REGISTER_CMD_SIMPLE(PlasmaFX,  "freq",  self->mK, 1, 255)
}

void PlasmaFX::update()
{
  // cos16 & sin16(0 to 65535) => results in -32767 to 32767
  u_long t = (millis() * 66) >> 2;          // 65536/1000 => 2pi * time / 4
  int16_t cos_tp1 = cos16(t/mP1) >> 1;      // .5 cos(time/mP1)
  int16_t sin_tp2 = sq(sin16(t/mP2)) >> 2;  // (.5 sin(time/mP2))^2
  int16_t sin_t = sin16(t);

  int16_t x = -5215;
  int16_t step = 10430 / mNLEDS;            // 10430 = 65536 / (2 pi) => x (-.5 to .5)

  for (byte i=0; i < mNLEDS; i++, x += step)
  {
    //  cx = x + .5 cos(time/mP1); cy = .5 sin(time/mP2);
    int16_t cx = x + cos_tp1;
    int16_t sqrxy = sqrt16((cx*cx + sin_tp2) >> 16) << 8;

    // sin(time) + 2 sin(.5 (x k + time)) + sin(sqrt(k^2(cx^2 + cy^2) + 1) + time);
    int16_t v = sin_t + (sin16((mK*x + t) >> 1) << 1) + sin16(mK*sqrxy + t);
    mLeds[i] = CHSV(v>>8, SATURATION, 0xff);
  }
}

// ----------------------------------------------------
CylonFX::CylonFX(const CRGB color, const int eyeSize, const int speed) : mEyeSize(eyeSize), mSpeed(speed), mColor(color) 
{
  REGISTER_CMD3(CylonFX, "color",   { self->mColor = CRGB(arg0, arg1, arg2); },        self->mColor.r,   self->mColor.g,   self->mColor.b, 0, 255)
  REGISTER_CMD(CylonFX,  "eyeSize", { self->setEyeSize(arg0*(self->mNLEDS-1)/255); },  self->mEyeSize*255/(self->mNLEDS-1), 1, 255)
  REGISTER_CMD(CylonFX,  "speed",   { self->mSpeed = arg0<<3; },                       self->mSpeed>>3, 0, 255)
}

int CylonFX::getPos() 
{
  return   triwave8(millis() * mSpeed * 38 / 10000) << 8; // speed = 1<<3, 1.5 second period 
}

void CylonFX::showEye(int p)
{
  #define FRAC_SHIFT 4
  long pos16 = (ease16InOutQuad(p) * (mNLEDS - mEyeSize - 1)) >> (16-FRAC_SHIFT);
  int pos = pos16 >> FRAC_SHIFT;
  byte frac = (pos16 & 0x0F) << FRAC_SHIFT;

  mLeds[pos] = mColor % (255 - frac);
  for(byte j = 0; j < mEyeSize; j++)
    mLeds[++pos] = mColor;
  if (pos < mNLEDS-1)
    mLeds[++pos] = mColor % frac;
}

void CylonFX::update()
{
  CLEAR_LED(mLeds, mNLEDS)
  showEye(getPos());
}

// ----------------------------------------------------
void DblCylonFX::update()
{
  CLEAR_LED(mLeds, mNLEDS)
  int pos = getPos();
  showEye(pos);
  showEye(65535 - pos);
}

// ----------------------------------------------------
RunningFX::RunningFX(const CRGB color, const int width, const int speed) : mWidth(width), mSpeed(speed), mColor(color) 
{
  REGISTER_CMD_SIMPLE(RunningFX,  "speed", self->mSpeed, 0, 255)
}

void RunningFX::update()
{
  fill_solid(mLeds, mNLEDS, mColor);

  u_long t = millis() * 66 * mSpeed; // 65536/1000 => 2pi * time 
  u_long x = 0; 
  u_long dx = 32768 / mWidth;
  for (byte i=0; i < mNLEDS; i++, x += dx)
  {
    int _sin = sin16(x + t);
    mLeds[i].nscale8(_sin > 0 ? _sin>>8 : 0);
  }
}

// ----------------------------------------------------
TwinkleFX::TwinkleFX(const byte hue, const byte hueDiv, const byte div) : mHueDiv(hueDiv), mDiv(div) 
{
  mHSV = CHSV(hue, 0xff, 0xff);
  registerAllCmd();
}

TwinkleFX::TwinkleFX(const CRGB color, const byte hueDiv, const byte div) : mHueDiv(hueDiv), mDiv(div)
{
  mHSV = rgb2hsv_approximate(color);
  registerAllCmd();
}

void TwinkleFX::registerAllCmd()
{
  REGISTER_CMD_SIMPLE(TwinkleFX,  "hue", self->mHSV.h, 0, 255)
  REGISTER_CMD_SIMPLE(TwinkleFX,  "div", self->mDiv, 1, 255)
}

void TwinkleFX::update()
{
  random16_set_seed(535);  // The randomizer needs to be re-set each time through the loop in order for the 'random' numbers to be the same each time through.

  int t = millis();
  for (int i = 0; i<mNLEDS; i++)
  {
    byte fader = sin8(t/random8(mDiv, mDiv<<1));       // The random number for each 'i' will be the same every time.
    byte hue = sin8(t/random8(mDiv, mDiv)) >> mHueDiv; // ditto
    mLeds[i] = CHSV(mHSV.h + hue, mHSV.s , fader);
  }

  random16_set_seed(millis()); // "restart" random for other FX
}

// ----------------------------------------------------
AllLedStrips::AllLedStrips(const int maxmA, Stream &serial) : mSerial(&serial)
{
  FastLED.setMaxPowerInVoltsAndMilliamps(5, maxmA);
  FastLED.countFPS();
  FastLED.setDither(BINARY_DITHER);
}

bool AllLedStrips::registerStrip(BaseLedStrip &strip)
{
  bool ok = mNStrips < MAXSTRIP;
  if (ok)
  {
    mStrips[mNStrips++] = &strip;
    strip.setSerial(mSerial);
  }
  return ok;
}

void AllLedStrips::update()
{
  for (byte i=0; i < mNStrips; i++)
    mStrips[i]->update();
}

void AllLedStrips::getInfo()
{
  *mSerial << "FPS " << FastLED.getFPS() << endl;
  for (byte i=0; i < mNStrips; i++)
    mStrips[i]->getInfo();
}
