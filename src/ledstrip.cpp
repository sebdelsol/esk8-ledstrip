#include <ledstrip.h>

// ----------------------------------------------------
void FX::init(int nLeds)
{
  mNLEDS = nLeds;
  mLeds = (CRGB *)malloc(nLeds * sizeof(CRGB));
  CLEAR_LED(mLeds, nLeds);
  specialInit(nLeds);
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

void FX::answer(const MyCmd &cmd, byte arg1, byte arg2, byte arg3)
{
  *cmd.answer << cmd.fx << " " <<  cmd.what << " " << arg1 << " " << arg2 << " " << arg3 << endl;
}

void FX::answer(const MyCmd &cmd, byte arg)
{
  *cmd.answer << cmd.fx << " " <<  cmd.what << " " << arg << endl;
}

// ----------------------------------------------------
PlasmaFX::PlasmaFX(const byte wavelenght, const byte period1, const byte period2) : mK(wavelenght), mP1(period1), mP2(period2) {}

void PlasmaFX::update()
{
  // cos16 & sin16(0 to 65535) => results in -32767 to 32767
  u_long t = (millis() * 66) >> 2;          // 65536/1000 => 2pi * time / 4
  int16_t cos_tp1 = cos16(t/mP1) >> 1;      // .5 cos(time/mP1)
  int16_t sin_tp2 = sq(sin16(t/mP2)) >> 2;  // (.5 sin(time/mP2))^2
  int16_t sin_t = sin16(t);

  int16_t x = -5215;
  int16_t step = 10430 / mNLEDS;            // 10430 = 65536 / (2 pi) => x (-.5 to .5)

  for (byte i=0; i < mNLEDS; i++, x += step) {
    //  cx = x + .5 cos(time/mP1); cy = .5 sin(time/mP2);
    int16_t cx = x + cos_tp1;
    int16_t sqrxy = sqrt16((cx*cx + sin_tp2) >> 16) << 8;

    // sin(time) + 2 sin(.5 (x k + time)) + sin(sqrt(k^2(cx^2 + cy^2) + 1) + time);
    int16_t v = sin_t + (sin16((mK*x + t) >> 1) << 1) + sin16(mK*sqrxy + t);
    mLeds[i] = CHSV(v>>8, SATURATION, 0xff);
  }
}

void PlasmaFX::setCmd(const MyCmd &cmd)
{
  switch(cmd.what) {
    case 'F': mK = cmd.arg[0]; break;
    case 'O': mP1 = cmd.arg[0]; break;
    case 'T': mP2 = cmd.arg[0]; break;
    case 'B': setAlpha(cmd.arg[0]); break;
  }
}

void PlasmaFX::getCmd(const MyCmd &cmd)
{
  switch(cmd.what) {
    case 'F': answer(cmd, mK); break;
    case 'O': answer(cmd, mP1); break;
    case 'T': answer(cmd, mP2); break;
    case 'B': answer(cmd, getAlpha()); break;
  }
}

// ----------------------------------------------------
CylonFX::CylonFX(const CRGB color, const int eyeSize, const int speed) : mEyeSize(eyeSize), mSpeed(speed), mColor(color) {}

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

void CylonFX::setCmd(const MyCmd &cmd)
{
  switch(cmd.what) {
    case 'C': if (cmd.nbArg==3) mColor = CRGB(cmd.arg[0], cmd.arg[1], cmd.arg[2]); break;
    case 'E': mEyeSize = cmd.arg[0]*(mNLEDS-1)/255; break;
    case 'S': mSpeed = cmd.arg[0]<<3; break;
    case 'B': setAlpha(cmd.arg[0]); break;
  }
}

void CylonFX::getCmd(const MyCmd &cmd)
{
  switch(cmd.what) {
    case 'C': answer(cmd, mColor.r, mColor.g, mColor.b); break;
    case 'E': answer(cmd, mEyeSize*255/(mNLEDS-1)); break;
    case 'S': answer(cmd, mSpeed>>3); break;
    case 'B': answer(cmd, getAlpha()); break;
  }
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
RunningFX::RunningFX(const CRGB color, const int width, const int speed) : mWidth(width), mSpeed(speed), mColor(color) {}

void RunningFX::update()
{
  fill_solid(mLeds, mNLEDS, mColor);

  u_long t = millis() * 66 * mSpeed; // 65536/1000 => 2pi * time 
  u_long x = 0; 
  u_long dx = 32768 / mWidth;
  for (byte i=0; i < mNLEDS; i++, x += dx) {
    int _sin = sin16(x + t);
    mLeds[i].nscale8(_sin > 0 ? _sin>>8 : 0);
  }
}

// ----------------------------------------------------
TwinkleFX::TwinkleFX(const byte hue, const byte hueDiv, const byte div) : mHueDiv(hueDiv), mDiv(div) 
{
  mHSV = CHSV(hue, 0xff, 0xff);
}

TwinkleFX::TwinkleFX(const CRGB color, const byte hueDiv, const byte div) : mHueDiv(hueDiv), mDiv(div)
{
  mHSV = rgb2hsv_approximate(color);
}

void TwinkleFX::update()
{
  random16_set_seed(535);  // The randomizer needs to be re-set each time through the loop in order for the 'random' numbers to be the same each time through.

  int t = millis();
  for (int i = 0; i<mNLEDS; i++) {
    byte fader = sin8(t/random8(mDiv, mDiv<<1));       // The random number for each 'i' will be the same every time.
    byte hue = sin8(t/random8(mDiv, mDiv)) >> mHueDiv; // ditto
    mLeds[i] = CHSV(mHSV.h + hue, mHSV.s , fader);
  }

  random16_set_seed(millis()); // "restart" random for other FX
}

void TwinkleFX::setCmd(const MyCmd &cmd)
{
  switch(cmd.what) {
    case 'H': mHSV.h = cmd.arg[0]; break;
    case 'D': mDiv = cmd.arg[0]; break;
    case 'B': setAlpha(cmd.arg[0]); break;
  }
}

void TwinkleFX::getCmd(const MyCmd &cmd)
{
  switch(cmd.what) {
    case 'H': answer(cmd, mHSV.h); break;
    case 'D': answer(cmd, mDiv); break;
    case 'B': answer(cmd, getAlpha()); break;
  }
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
  if (ok){
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
