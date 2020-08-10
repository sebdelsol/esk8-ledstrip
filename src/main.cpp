#define USE_BT
#define USE_OTA 
// #define USE_TELNET 

// #define DEBUG_RASTER
// #define DEBUG_LED_TOWIFI
// #define DEBUG_LED_INFO
// #define DEBUG_ACC

// ----------------------------------------------------
#include <Streaming.h>
#include <soc/rtc.h> // cpu freq

#include <Pins.h>
#include <Ledstrip.h>
#include <Motion.h>
#include <myWifi.h>
#include <Raster.h>

// -- Telnet Serial dbg
#ifdef USE_TELNET
  #include  <TelnetSpy.h>  
  TelnetSpy SerialAndTelnet;  // it redefines Serial
  #define   Serial  SerialAndTelnet 
#endif

// -- OTA
#ifdef USE_OTA
  #include  <OTA.h>
  OTA       Ota(Serial);
#endif

// -- Acc & Wifi
MOTION  Motion(Serial);
myWifi  MyWifi(Serial);

// -- BT
#ifdef USE_BT
  #include  <Bluetooth.h>
  #include  <Button.h>
  #include  <AllObjBT.h>
  Button    Button(BUTTON_PIN);
  BlueTooth BT(Serial);
  AllObjBT  AllObj(Serial);
#else
  #include  <AllObj.h>
  AllObj    AllObj(Serial);
#endif

#define RegisterAllObjs(...) RegisterOBJS(AllObj, __VA_ARGS__)

// -- Cfg
#include  <Cfg.h> // needs Motion & BT objs defined
CFG       Cfg;

// --- Strips
#define   AQUA          CRGB(0x00FFFF)
#define   AQUA_MENTHE   CRGB(0x7FFFD4)
#define   LUSH_LAVA     CRGB(0xFF4500)
#define   HUE_AQUA_BLUE 140

AllLedStrips  AllStrips(LED_MAX_MA, Serial);

LedStrip    <NBLEDS_MIDDLE, LEDM_PIN> StripM(Serial, "Mid Strip");
RunningFX   FireRun(LUSH_LAVA, 3);     
RunningFX   AquaRun(AQUA_MENTHE, -3);  
TwinkleFX   FireTwk(HUE_RED); 
TwinkleFX   AquaTwk(HUE_AQUA_BLUE);
PlasmaFX    Plasma;

LedStrip    <NBLEDS_TIPS, LEDR_PIN>  StripR(Serial, "Rear Strip");
DblCylonFX  CylonR(LUSH_LAVA); 
FireFX      FireRL;
FireFX      FireRR(true); // reverse
TwinkleFX   TwinkleR(CRGB::Red);
RunningFX   RunR(CRGB::Gold); 

LedStrip    <NBLEDS_TIPS, LEDF_PIN>  StripF(Serial, "Front Strip");
DblCylonFX  CylonF(AQUA);   
PacificaFX  Pacifica;
TwinkleFX   TwinkleF(HUE_AQUA_BLUE); 
RunningFX   RunF(CRGB::Gold);

// ----------------------------------------------------
void setup()
{
  // -- switch off all leds
  AllStrips.setBrightness(0);
  AllStrips.clearAndShow();

  // -- main inits
  Serial.begin(SERIAL_BAUD);

  Serial << endl << "---------" << endl;
  Serial << "ESP32 " << esp_get_idf_version() << endl;
  Serial << "CPU runs @ " << rtc_clk_cpu_freq_get() * 80 << "MHz" << endl;
  Serial << "Main runs on Core " << xPortGetCoreID() << endl;

  Cfg.init();
  Motion.init();

  // -- Strip inits
  AllStrips._RegisterSTRIPS(StripM, StripR, StripF); 
  AllStrips.init();
  
  StripM._RegisterFXS(FireRun,  FireTwk, AquaRun,  AquaTwk, Plasma);
  StripR._RegisterFXS(TwinkleR, FireRR,  FireRL,   RunR,    CylonR);
  StripF._RegisterFXS(TwinkleF, RunF,    Pacifica, CylonF);

  // -- Register AllObj
  AllObj.init();
  
  RegisterAllObjs("",        Motion,   Cfg);            
  RegisterAllObjs("mid.",    FireRun,  FireTwk, AquaRun,  AquaTwk,  Plasma);
  RegisterAllObjs("rear.",   TwinkleR, FireRR,  FireRL,   RunR,     CylonR);
  RegisterAllObjs("front.",  TwinkleF, RunF,    Pacifica, CylonF);

  AllObj.save(true); // save default
  AllObj.load(false, false); // load not default, do not send change to BT

  // -- BlueTooth
  #ifdef USE_BT
    BT.init();
    BT.start();
    Button.begin();
  #else   
    pinMode(LIGHT_PIN, OUTPUT); //blue led
    digitalWrite(LIGHT_PIN, LOW); // switch off blue led
    btStop(); // turnoff bt 
  #endif
  
  // -- Wifi
  #if defined(DEBUG_LED_TOWIFI) || defined(USE_OTA) || defined(USE_TELNET)
    MyWifi.start();
    #ifdef DEBUG_LED_TOWIFI
      MyWifi._AddStripsToWifi(StripM, StripR, StripF)
    #endif
  #else
    MyWifi.stop();
  #endif
}

// ----------------------------------------------------
void loop()
{
  RASTER_BEGIN(20);

  EVERY_N_MILLISECONDS(LED_TICK)
  {
    // -- Master brightness
    if(Cfg.probe)
    {
      int light = analogRead(LDR_PIN);
      Cfg.bright = map(light, Cfg.minProbe, MaxProbe, 255, 0); // the darker the light, the brighter the leds
    }
    Cfg.fade = lerp16by16(Cfg.fade,  65535,  650);
    byte bright = (Cfg.bright * ((Cfg.fade >> 8) + 1)) >> 8; 
    AllStrips.setBrightness(bright);
    RASTER("Light probe");

    // -- handle Motion Sensor
    Motion.update(); 
    SensorOutput& m = Motion.mOutput;
    RASTER("Motion");

    if (m.updated)
    {
      #define MulAlpha(a, b) (((a) * ((b) + 1)) >> 8)

      // -- Gyro
      int runSpeed =  ((m.wZ > 0) - (m.wZ < 0)) * Cfg.runSpeed;
      int _WZ = abs(m.wZ);
      byte alpha = _WZ > Cfg.neutralWZ ? min((_WZ - Cfg.neutralWZ) * 255 / Cfg.maxWZ, 255) : 0;
      byte invAlpha = 255 - alpha;

      // -- Acc
      #define MAXACC 256
      int acc = constrain(m.accY / Cfg.divAcc, -MAXACC, MAXACC) << 8;

      // -- Front strip
      static int FWD = 0;
      int fwd = constrain(acc, 0, 65535);
      FWD = fwd > FWD ? fwd : lerp16by16(FWD, fwd, Cfg.smoothAcc);

      int alphaF = constrain((FWD - (Cfg.thresAcc << 8))/(MAXACC - Cfg.thresAcc), 0, 255);
      int eyeF = Cfg.minEye + (((Cfg.maxEye - Cfg.minEye) * alphaF) >> 8);

      if (Cfg.stripFront)
      { 
        RunF.setSpeed(runSpeed);
        RunF.setAlpha(alpha);
        CylonF.setEyeSize(eyeF);
        CylonF.setAlpha(MulAlpha(255 - Cfg.pacifica, invAlpha)); 
        Pacifica.setAlpha(MulAlpha(Cfg.pacifica, invAlpha)); 
        TwinkleF.setAlpha(MulAlpha(alphaF, invAlpha)); 
      }

      // -- Rear Strip
      static int RWD = 0;
      int rwd = constrain(-acc, 0, 65535);
      RWD = rwd > RWD ? rwd : lerp16by16(RWD, rwd, Cfg.smoothAcc);

      int alphaR = constrain((RWD - (Cfg.thresAcc << 8))/(MAXACC - Cfg.thresAcc), 0, 255);
      int eyeR = Cfg.minEye + (((Cfg.maxEye - Cfg.minEye) * alphaR) >> 8);
      int dim = Cfg.minDim + (((Cfg.maxDim - Cfg.minDim) * alphaR) >> 8);

      if (Cfg.stripRear)
      { 
        RunR.setSpeed(runSpeed);
        RunR.setAlpha(alpha);
        CylonR.setEyeSize(eyeR);
        CylonR.setAlpha(MulAlpha(255 - Cfg.fire, invAlpha)); 
        FireRR.setAlpha(MulAlpha(Cfg.fire, invAlpha)); 
        FireRL.setAlpha(MulAlpha(Cfg.fire, invAlpha));
        FireRR.setDimRatio(dim); 
        FireRL.setDimRatio(dim); 
        TwinkleR.setAlpha(MulAlpha(max(Cfg.minTwkR, alphaR), invAlpha)); 
      }

      // -- Middle Strip
      if (Cfg.stripMid)
      {
        AquaRun.setAlpha(alphaF);
        AquaTwk.setAlpha(alphaF);
        FireRun.setAlpha(alphaR);
        FireTwk.setAlpha(alphaR);
        Plasma.setAlpha(max(0, 255 - max(alphaR, alphaF)));
      }

      #ifdef DEBUG_ACC
        Serial << "[areal  " << m.accX << "\t"      << m.accY   << "\t"        << m.accZ << "]\t";
        Serial << "[fwd "    << fwd    << "\trwd "  << rwd      << "\tACC "    << acc << "]\t";
        Serial << "[alpha "  << alpha  << "\tinv "  << invAlpha << "]\t";
        Serial << "[eyeR "   << eyeR   << "\teyeF " << eyeF     << "\talphaR " << alphaR << "\talphaF " << alphaF << "]" << endl;
      #endif
  
      RASTER("Leds setup");
    }

    // -- Leds actual drawing
    AllStrips.update();
    RASTER("Leds update");

    // -- wifi Update
    #if defined(DEBUG_LED_TOWIFI) || defined(USE_OTA) || defined(USE_TELNET)
      if(MyWifi.update())
      {
        #ifdef USE_TELNET
          SerialAndTelnet.handle();
        #endif

        #ifdef USE_OTA
          Ota.update();
        #endif
      }
      RASTER("Wifi");
    #endif
  }

  // -- Bluetooth Update
  #ifdef USE_BT
    EVERY_N_MILLISECONDS(BT_TICK)
    {
      if (Button.pressed())
        BT.toggle();

      AllObj.receiveUpdate(BT);
    }
    RASTER("BlueTooth");
  #endif

  #ifdef DEBUG_LED_INFO
    EVERY_N_SECONDS(1)
      AllStrips.getInfo();
  #endif

  // -- Leds dithering
  AllStrips.show(); // to be called as much as possible for Fastled brightness dithering
  RASTER("Leds show"); 

  RASTER_END;
}
