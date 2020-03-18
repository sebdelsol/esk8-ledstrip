#include <ledstrip.h>

// ----------------------------------------------------
void FX::init(int nLeds)
{
  mNLEDS = nLeds;
  mLeds = (CRGB *)malloc(nLeds * sizeof(CRGB));
  CLEAR_LED(mLeds, nLeds);
  specialInit(nLeds);

  REGISTER_VAR(FX,  "alpha", { self->setAlpha(arg0); },  self->getAlpha(), 0, 255)
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

bool FX::drawOn(CRGBSet dst, ulong time, ulong dt)
{
  if (mAlpha > 0) { // 0 is invisible
    update(time, dt);
    memcpy8(dst.leds, mLeds, mNLEDS * sizeof(CRGB));
    
    if (mAlpha < 255) // 255 = no fade
      dst.nscale8_video(mAlpha);
    
    return true;
  }
  return false;
}


// ----------------------------------------------------
FireFX::FireFX(const byte speed) : mSpeed(speed) 
{
  REGISTER_VAR_SIMPLE(FireFX,  "speed", self->mSpeed, 1, 255)
  mPal = HeatColors_p;
}

void FireFX::specialInit(int nLeds)
{
  mHeat = (byte*)malloc(nLeds*sizeof(byte));
  mNoise = (byte*)malloc(nLeds*sizeof(byte));
  mCentre = (nLeds / 2) - 1;
}

void FireFX::update(ulong time, ulong dt)
{
  // get one noise value out of a moving noise space
  uint16_t ctrl1 = inoise16(11 * time, 0, 0);
  uint16_t ctrl2 = inoise16(13 * time, 100000, 100000);
  uint16_t  ctrl = ((ctrl1 + ctrl2) / 2); // average of both to get a more unpredictable curve

  // here we define the impact of the wind, high factor = a lot of movement to the sides
  uint32_t x = 3 * ctrl * mSpeed;

  // this is the speed of the upstream itself, high factor = fast movement
  uint32_t y = 15 * time * mSpeed;

  // just for ever changing patterns we move through z as well
  uint32_t z = 3 * time * mSpeed;

  // ...and dynamically scale the complete heatmap for some changes in the size of the heatspots.
  // The speed of change is influenced by the factors in the calculation of ctrl1 & 2 above.
  // The divisor sets the impact of the size-scaling.
  uint32_t scale_x = ctrl1 / 2;
  uint32_t scale_y = ctrl2 / 2;

  // Calculate the noise array based on the control parameters.
	for (uint8_t j = 0; j < mNLEDS; j++) 
  {
    uint32_t joffset = scale_y * (j - mCentre);
    uint16_t data = ((inoise16(x, y + joffset, z)) + 1);
    mNoise[j] = data >> 8;
	}

  // Draw the first (lowest) line - seed the fire.
  // It could be random pixels or anything else as well.
  mLeds[mNLEDS-1] = ColorFromPalette( mPal, mNoise[0]);
  // and fill the lowest line of the heatmap, too
  mHeat[mNLEDS-1] = mNoise[0];

  // Copy the heatmap one line up for the scrolling.
  for (uint8_t y = 0; y < mNLEDS - 1; y++)
  {
    mHeat[y] = mHeat[y + 1];
  }

	float ratio = pow(1.4, dt/10.);
  
  // Scale the heatmap values down based on the independent scrolling noise array.
  for (uint8_t y = 0; y < mNLEDS - 1; y++)
  {
      // get data from the calculated noise field
      uint8_t dim = mNoise[y];

      // This number is critical
      // If it´s to low (like 1.1) the fire dosn´t go up far enough.
      // If it´s to high (like 3) the fire goes up too high.
      // It depends on the framerate which number is best.
      // If the number is not right you loose the uplifting fire clouds
      // which seperate themself while rising up.
      dim = dim / ratio;
      dim = 255 - dim;

      // here happens the scaling of the heatmap
      mHeat[y] = scale8(mHeat[y] , dim);
  }

  // Now just map the colors based on the heatmap.
  for (uint8_t y = 0; y < mNLEDS - 1; y++)
  {
      mLeds[y] = ColorFromPalette( mPal, mHeat[y]);
  }
}

// ----------------------------------------------------
PlasmaFX::PlasmaFX(const byte wavelenght, const byte period1, const byte period2) : mK(wavelenght), mP1(period1), mP2(period2) 
{
  REGISTER_VAR_SIMPLE(PlasmaFX,  "p1",    self->mP1, 1, 255)
  REGISTER_VAR_SIMPLE(PlasmaFX,  "p2",    self->mP2, 1, 255)
  REGISTER_VAR_SIMPLE(PlasmaFX,  "freq",  self->mK, 1, 255)
}

void PlasmaFX::update(ulong time, ulong dt)
{
  // cos16 & sin16(0 to 65535) => results in -32767 to 32767
  u_long t = (time * 66) >> 2;          // 65536/1000 => 2pi * time / 4
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
  REGISTER_VAR3(CylonFX, "color",   { self->mColor = CRGB(arg0, arg1, arg2); },        self->mColor.r,   self->mColor.g,   self->mColor.b, 0, 255)
  REGISTER_VAR(CylonFX,  "eyeSize", { self->setEyeSize(arg0*(self->mNLEDS-1)/255); },  self->mEyeSize*255/(self->mNLEDS-1), 1, 255)
  REGISTER_VAR(CylonFX,  "speed",   { self->mSpeed = arg0<<3; },                       self->mSpeed>>3, 0, 255)
}

int CylonFX::getPos(ulong time) 
{
  return   triwave8(time * mSpeed * 38 / 10000) << 8; // speed = 1<<3, 1.5 second period 
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

void CylonFX::update(ulong time, ulong dt)
{
  CLEAR_LED(mLeds, mNLEDS)
  showEye(getPos(time));
}

// ----------------------------------------------------
void DblCylonFX::update(ulong time, ulong dt)
{
  CLEAR_LED(mLeds, mNLEDS)
  int pos = getPos(time);
  showEye(pos);
  showEye(65535 - pos);
}

// ----------------------------------------------------
RunningFX::RunningFX(const CRGB color, const int width, const int speed) : mWidth(width), mSpeed(speed), mColor(color) 
{
  REGISTER_VAR_SIMPLE(RunningFX,  "speed", self->mSpeed, 0, 255)
}

void RunningFX::update(ulong time, ulong dt)
{
  fill_solid(mLeds, mNLEDS, mColor);

  u_long t = time * 66 * mSpeed; // 65536/1000 => 2pi * time 
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
  REGISTER_VAR_SIMPLE(TwinkleFX,  "hue", self->mHSV.h, 0, 255)
  REGISTER_VAR_SIMPLE(TwinkleFX,  "div", self->mDiv, 1, 255)
}

void TwinkleFX::update(ulong time, ulong dt)
{
  random16_set_seed(535);  // The randomizer needs to be re-set each time through the loop in order for the 'random' numbers to be the same each time through.

  for (int i = 0; i<mNLEDS; i++)
  {
    byte fader = sin8(time/random8(mDiv, mDiv<<1));       // The random number for each 'i' will be the same every time.
    byte hue = sin8(time/random8(mDiv, mDiv)) >> mHueDiv; // ditto
    mLeds[i] = CHSV(mHSV.h + hue, mHSV.s , fader);
  }

  random16_set_seed(time); // "restart" random for other FX
}

// ----------------------------------------------------
AllLedStrips::AllLedStrips(const int maxmA, Stream &serial) : mSerial(&serial)
{
  FastLED.setMaxPowerInVoltsAndMilliamps(5, maxmA);
  FastLED.countFPS();
  FastLED.setDither(BINARY_DITHER); //DISABLE_DITHER
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
  ulong t = millis();
  ulong dt = mLastT ? t - mLastT : 1; // to prevent possible /0
  mLastT = t;

  for (byte i=0; i < mNStrips; i++)
    mStrips[i]->update(t, dt);
}

void AllLedStrips::getInfo()
{
  *mSerial << "FPS " << FastLED.getFPS() << endl;
  for (byte i=0; i < mNStrips; i++)
    mStrips[i]->getInfo();
}
