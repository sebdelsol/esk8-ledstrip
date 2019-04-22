#include <ledstrip.h>

// ----------------------------------------------------
CRGB* FX::updateAndFade()
{
  if (mAlpha > 0) { // 0 is invisible
    update();
    if (mAlpha < 255) // 255 no fade
      // fadeToBlackBy(mLeds, mNLEDS, 255-mAlpha);
      nscale8_video(mLeds, mNLEDS, mAlpha);
    return mLeds;
  }
  return NULL;
}

void FX::answer(const MyCmd &cmd, byte arg1, byte arg2, byte arg3)
{
  *cmd.answer << cmd.fx << " " <<  cmd.what << " " << arg1 << " " << arg2 << " " << arg3 << endl;
  // Serial << "Send " << cmd.fx << " " <<  cmd.what << " " << arg1 << " " << arg2 << " " << arg3 << endl;
}

void FX::answer(const MyCmd &cmd, byte arg)
{
  *cmd.answer << cmd.fx << " " <<  cmd.what << " " << arg << endl;
  // Serial << "Send " << cmd.fx << " " <<  cmd.what << " " << arg << endl;
}

// ----------------------------------------------------
Fire::Fire(const bool reverse, const byte cooling, const byte sparkling) : mReverse(reverse), mCooling(cooling), mSparkling(sparkling)
{
  mPal = HeatColors_p;
}

void Fire::update()
{
  random16_add_entropy(random16());

  //  Cool down every cell a little
  for( byte i = 0; i < mNLEDS; i++)
    mHeat[i] = qsub8( mHeat[i],  random8(0, ((mCooling * 10) / mNLEDS) + 2));

  //  mHeat from each cell drifts 'up' and diffuses a little
  for( byte k = mNLEDS - 1; k >= 2; k--)
    mHeat[k] = (mHeat[k - 1] + mHeat[k - 2] + mHeat[k - 2] ) / 3;

  //  Randomly ignite new 'sparks' of mHeat near the bottom
  if( random8() < mSparkling ) {
    byte y = random8(7);
    mHeat[y] = qadd8( mHeat[y], random8(160, 255) );
  }

  //  Map from mHeat cells to LED colors
  for( byte j = 0; j < mNLEDS; j++) {
    byte colorindex = scale8( mHeat[j], 240); // scale down to 0-240 for best results with color palettes.
    CRGB color = ColorFromPalette(mPal, colorindex);

    byte i = mReverse ? (mNLEDS-1) - j : j;
    mLeds[i] = color;
  }
}

void Fire::setCmd(const MyCmd &cmd)
{
  switch(cmd.what) {
    case 'C': mCooling = cmd.arg[0]; break;
    case 'S': mSparkling = cmd.arg[0]; break;
    case 'R': mReverse = cmd.arg[0]; break;
    case 'B': setAlpha(cmd.arg[0]); break;
  }
}

void Fire::getCmd(const MyCmd &cmd)
{
  switch(cmd.what) {
    case 'C': answer(cmd, mCooling); break;
    case 'S': answer(cmd, mSparkling); break;
    case 'R': answer(cmd, mReverse); break;
    case 'B': answer(cmd, getAlpha()); break;
  }
}

// ----------------------------------------------------
Aqua::Aqua(const bool reverse, const byte cooling, const byte sparkling) : Fire(reverse, cooling, sparkling)
{
  mPal = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
}

// ----------------------------------------------------
Plasma::Plasma(const byte wavelenght, const byte period1, const byte period2) : mK(wavelenght), mP1(period1), mP2(period2)
{}

void Plasma::update()
{
  // cos16 & sin16(0 to 65535) => results in -32767 to 32767
  u_long t = (millis() * 66) >> 2; // 65536/1000 => time is 2*PI / sec
  int16_t _costp1 = cos16(t/mP1) >> 1;
  int16_t _sintp2 = sq(sin16(t/mP2) >> 1);
  int16_t _sint = sin16(t);

  int16_t x = -5215;
  int16_t step = 10430 / mNLEDS;  // 65536 / (2*pi) => x (-.5 to .5)

  for (byte i=0; i < mNLEDS; i++, x += step) {
    //  cx = x + .5 * cos(time/3.0); cy = .5 * sin(time/5.0);
    int16_t cx = x + _costp1;
    int16_t sqrxy = sqrt16((cx*cx + _sintp2) >> 16) << 8;

    // sin(time) + sin(.5 * (x*k + time)) * 2 + sin(sqrt(k*k*(cx*cx+cy*cy)+1) + time);
    int16_t v = _sint + (sin16((mK*x + t) >> 1) << 1) + sin16(mK*sqrxy + t);
    mLeds[i] = CHSV(v>>8, 0xf0, 0xff);
  }
}

void Plasma::setCmd(const MyCmd &cmd)
{
  switch(cmd.what) {
    case 'F': mK = cmd.arg[0]; break;
    case 'O': mP1 = cmd.arg[0]; break;
    case 'T': mP2 = cmd.arg[0]; break;
    case 'B': setAlpha(cmd.arg[0]); break;
  }
}

void Plasma::getCmd(const MyCmd &cmd)
{
  switch(cmd.what) {
    case 'F': answer(cmd, mK); break;
    case 'O': answer(cmd, mP1); break;
    case 'T': answer(cmd, mP2); break;
    case 'B': answer(cmd, getAlpha()); break;
  }
}

// ----------------------------------------------------
Cylon::Cylon(const byte r, const byte g, const byte b, const int eyeSize, const int speed) : mEyeSize(eyeSize), mSpeed(speed), mColor(CRGB(r, g, b))
{}

void Cylon::update(){

  memset8(mLeds, 0, mNLEDS * sizeof(CRGB)); // clear

  #define FRAC_SHIFT 4
  long pos16 = (ease16InOutQuad(mPos) * (mNLEDS - mEyeSize - 1)) >> (16-FRAC_SHIFT);
  int pos = pos16 >> FRAC_SHIFT;
  byte frac = (pos16 & 0x0F) << FRAC_SHIFT;

  mLeds[pos] = mColor % (255 - frac);
  for(byte j = 0; j < mEyeSize; j++)
    mLeds[++pos] = mColor;
  if (pos < mNLEDS-1)
    mLeds[++pos] = mColor % frac;

  mPos += mSpeed;
  if (mPos <= 0 || mPos >= 65535){ // rebound ?
    mPos = constrain(mPos, 0, 65535);
    mSpeed = - mSpeed;
  }
}

void Cylon::setCmd(const MyCmd &cmd)
{
  switch(cmd.what) {
    case 'C': if (cmd.nbArg==3) mColor = CRGB(cmd.arg[0], cmd.arg[1], cmd.arg[2]); break;
    case 'E': mEyeSize = cmd.arg[0]*(mNLEDS-1)/255; break;
    case 'S': mSpeed = cmd.arg[0]<<3; break;
    case 'B': setAlpha(cmd.arg[0]); break;
  }
}

void Cylon::getCmd(const MyCmd &cmd)
{
  switch(cmd.what) {
    case 'C': answer(cmd, mColor.r, mColor.g, mColor.b); break;
    case 'E': answer(cmd, mEyeSize*255/(mNLEDS-1)); break;
    case 'S': answer(cmd, mSpeed>>3); break;
    case 'B': answer(cmd, getAlpha()); break;
  }
}

// ----------------------------------------------------
void LedStrip::init(const int maxmA)
{
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(mLeds, mNLEDS); //.setCorrection(TypicalSMD5050); // = TypicalLEDStrip
  FastLED.setMaxPowerInVoltsAndMilliamps(5, maxmA);
  FastLED.countFPS();
}

bool LedStrip::registerFX(FX& fx)
{
  bool ok = mNFX < MAXFX-1;
  if (ok){
    mFX[mNFX++] = (FX*)&fx;
    fx.init(mNLEDS);
  }
  return ok;
}

void LedStrip::getInfo()
{
  Serial << "FPS " << FastLED.getFPS();
  for (byte i=0; i < mNFX; i++) {
    FX *fx = mFX[i];
    Serial << " - " << fx->getName() << "(" << fx->getAlpha() << ")";
  }
  Serial << "                  \n";
}

byte* LedStrip::getData(int& n)
{
  n = mNLEDS * sizeof(CRGB);
  return (byte*) mLeds;
}

void LedStrip::update()
{
  // copy then blend available fx
  bool first = true;

  for (byte i=0; i < mNFX; i++) {
    CRGB *src = mFX[i]->updateAndFade();

    if (src!=NULL) {
      if (first) { // copy first
        memcpy8(mLeds, src, mNLEDS * sizeof(CRGB));
        first = false;
      }
      else // then blend
        for (byte k=0; k < mNLEDS; k++)
          mLeds[k] += src[k];
    }
  }

  if(first) // clear if no FX shown
    FastLED.clear();
}

// ----------------------------------------------------
// Fire2::Fire2(const bool reverse) : mReverse(reverse)
// {
//   mPal = HeatColors_p;
// }
//
// void Fire2::update()
// {
//   for(int i = 0; i < NLEDS; i++) {
//     byte index = inoise8(i * mXscale, millis() * mSpeed * NLEDS / 255); // X location is constant, but we move along the Y at the rate of millis()
//     index = min((i * index) >>6, 255);
//     byte bright = i * 255 / NLEDS;
//
//     int j = mReverse ? (NLEDS-1) - i : i;
//     mLeds[j] = ColorFromPalette(mPal, index, bright, LINEARBLEND);
//   }
// }
//
// // ----------------------------------------------------
// Aqua2::Aqua2(const bool reverse) : Fire2(reverse)
// {
//   mPal = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
// }
