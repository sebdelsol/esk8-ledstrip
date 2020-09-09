#define USE_BT
#define USE_OTA 
// #define USE_TELNET 

// #define DEBUG_RASTER
// #define DEBUG_LED_TOWIFI
// #define DEBUG_LED_INFO

#define USE_WIFI (defined(DEBUG_LED_TOWIFI) || defined(USE_OTA) || defined(USE_TELNET))

// ----------------------------------------------------
#include <ledstrip.h>
#include <mpu.h>
#include <myWifi.h>
#include <Raster.h>

// -- Telnet Serial 
#ifdef USE_TELNET
  #include  <TelnetSpy.h>  
  TelnetSpy Telnet;  // output on Serial and Telnet
  #define   Serial  Telnet // redefine Serial
#endif

// -- log, depends on Serial possible redefinition 
#include <log.h>
Stream& _log = Serial;

// -- OTA
#ifdef USE_OTA
  #include  <OTA.h>
  OTA       Ota;
#endif

// -- Acc & Wifi
myWifi  MyWifi;
MPU     Mpu;

// -- BT & Cfg
#ifdef USE_BT
  #include  <Bluetooth.h>
  #include  <Button.h>
  #include  <AllObjBT.h>
  Button    Button(BUTTON_PIN);
  BlueTooth BT;
  AllObjBT  AllObj;
  #define   CfgArgs (AllObj, BT, Mpu)

#else
  #include  <NoBluetooth.h>
  #include  <AllObj.h>
  AllObj    AllObj;
  #define   CfgArgs
#endif

#include  <Cfg.h> 
CFG       Cfg CfgArgs;
Tweaks    Twk;

// --- Strips & Fxs
AllLedStrips AllStrips;

LedStrip    <NBLEDS_MIDDLE, LEDM_PIN> StripM("mid");
RunningFX   FireRun(LUSH_LAVA, 3);     
RunningFX   AquaRun(AQUA_MENTHE, -3);  
TwinkleFX   FireTwk(HUE_RED); 
TwinkleFX   AquaTwk(HUE_AQUA_BLUE);
PlasmaFX    Plasma;

LedStrip    <NBLEDS_TIPS, LEDR_PIN>  StripR("rear");
DblCylonFX  CylonR(LUSH_LAVA); 
FireFX      FireL;
FireFX      FireR(true); // reverse
TwinkleFX   TwinkleR(CRGB::Red);
RunningFX   RunR(CRGB::Gold); 

LedStrip    <NBLEDS_TIPS, LEDF_PIN>  StripF("front");
DblCylonFX  CylonF(AQUA);   
PacificaFX  Pacifica;
TwinkleFX   TwinkleF(HUE_AQUA_BLUE); 
RunningFX   RunF(CRGB::Gold);

// ----------------------------------------------------
void setup()
{
  // -- log
  Serial.begin(SERIAL_BAUD);
  _log << "\n---------\n- START -\n---------\n";
  _log << "ESP32 " << esp_get_idf_version() << endl;
  _log << "CPU runs @ " << getCpuFrequencyMhz() << "MHz" << endl;
  _log << "Main runs on Core " << xPortGetCoreID() << endl;

  // -- main inits
  AllStrips.init();
  AllObj.init();
  Cfg.init();
  Twk.init();
  Mpu.init();

  // -- register Strips & FXs
  AllStrips.addStrips(StripM, StripR, StripF); 

  StripM.addFXs( NameIt(FireRun,  FireTwk, AquaRun, AquaTwk, Plasma) );
  StripR.addFXs( NameIt(TwinkleR, RunR,    CylonR,  FireR,   FireL) );
  StripF.addFXs( NameIt(TwinkleF, RunF,    CylonF,  Pacifica) );

  // -- Register AllObj
  AllObj.addObjs( NameIt(Cfg, Mpu, AllStrips, Twk) );            
  AllStrips.addObjs(AllObj);

  AllObj.save(CfgType::Default);        
  AllObj.load(CfgType::Current, TrackChange::no);  // inits' cmd will send the right values to BT

  // -- BlueTooth
  #ifdef USE_BT
    BT.init(); // and start
    Button.begin();
  #else   
    NoBT();
  #endif
  
  // -- Wifi
  #if USE_WIFI
    MyWifi.start();
    #ifdef DEBUG_LED_TOWIFI
      MyWifi.addStrips(StripM, StripR, StripF);
    #endif
  #else
    MyWifi.stop();
  #endif
}

// ----------------------------------------------------
Raster Raster;

// --------------
inline void loopMpu()
{
  EVERY_N_MILLISECONDS(MPU_TICK) Mpu.update(); 
  Raster.add("MPU");
}

// ------------
inline void loopWifi()
{
#if USE_WIFI
  EVERY_N_MILLISECONDS(WIFI_TICK) if(MyWifi.update())
  {
    #ifdef USE_TELNET
      Telnet.handle();
    #endif

    #ifdef USE_OTA
      // concurrency issue with WSockets
      if (!MyWifi.isWSConnected()) Ota.update();
    #endif
  }
  Raster.add("Wifi");
#endif
}

// ----------
inline void loopBT()
{
#ifdef USE_BT
  EVERY_N_MILLISECONDS(BT_TICK)
  {
    if (Button.pressed()) BT.toggle();
    AllObj.receiveUpdate(BT);
  }
  Raster.add("BlueTooth");
#endif
}

// ------------
inline void loopLeds()
{
  EVERY_N_MILLISECONDS(LED_TICK)
  {
    // led setup modified by MPU
    SensorOutput& m = Mpu.mOutput;
    if (m.updated)
    {
      // -- Gyro
      int runSpeed =  ((m.wZ > 0) - (m.wZ < 0)) * Twk.runSpeed;
      int WZ = abs(m.wZ);
      byte alpha = WZ > Twk.neutralWZ ? min((WZ - Twk.neutralWZ) * 255 / Twk.maxWZ, 255) : 0;
      byte invAlpha = 255 - alpha;

      // -- Acc
      int acc = constrain(m.accY / Twk.divAcc, -256, 256) << 8;

      // -- Front strip
      int fwd = constrain(acc, 0, 65535);
      Twk.FWD = fwd > Twk.FWD ? fwd : lerp16by16(Twk.FWD, fwd, Twk.smoothAcc);
      int alphaF = constrain((Twk.FWD - (Twk.thresAcc << 8)) / (256 - Twk.thresAcc), 0, 255);

      if (Twk.stripFront)
      { 
        int eyeF = Twk.minEye + (((Twk.maxEye - Twk.minEye) * alphaF) >> 8);
        RunF.setSpeed(runSpeed);
        RunF.setAlpha(alpha);
        CylonF.setEyeSize(eyeF);
        CylonF.setAlphaMul(255 - Twk.pacifica, invAlpha); 
        Pacifica.setAlphaMul(Twk.pacifica, invAlpha); 
        TwinkleF.setAlphaMul(alphaF, invAlpha); 
      }

      // -- Rear Strip
      int rwd = constrain(-acc, 0, 65535);
      Twk.RWD = rwd > Twk.RWD ? rwd : lerp16by16(Twk.RWD, rwd, Twk.smoothAcc);
      int alphaR = constrain((Twk.RWD - (Twk.thresAcc << 8)) / (256 - Twk.thresAcc), 0, 255);

      if (Twk.stripRear)
      { 
        int eyeR = Twk.minEye + (((Twk.maxEye - Twk.minEye) * alphaR) >> 8);
        int dim = Twk.minDim + (((Twk.maxDim - Twk.minDim) * alphaR) >> 8);
        RunR.setSpeed(runSpeed);
        RunR.setAlpha(alpha);
        CylonR.setEyeSize(eyeR);
        CylonR.setAlphaMul(255 - Twk.fire, invAlpha); 
        FireR.setAlphaMul(Twk.fire, invAlpha); 
        FireL.setAlphaMul(Twk.fire, invAlpha);
        FireR.setDimRatio(dim); 
        FireL.setDimRatio(dim); 
        TwinkleR.setAlphaMul(max(Twk.minTwkR, alphaR), invAlpha); 
      }

      // -- Middle Strip
      if (Twk.stripMid)
      {
        AquaRun.setAlpha(alphaF);
        AquaTwk.setAlpha(alphaF);
        FireRun.setAlpha(alphaR);
        FireTwk.setAlpha(alphaR);
        Plasma.setAlpha(max(0, 255 - max(alphaR, alphaF)));
      }
  
      Raster.add("Leds setup");
    }

    // -- update
    AllStrips.update();
    Raster.add("Leds update");
  }

  // -- Leds dithering
  if (AllStrips.doDither())
    Raster.add("Leds dither"); 

  #ifdef DEBUG_LED_INFO
    EVERY_N_SECONDS(1) AllStrips.showInfo();
  #endif
}

// --------
void loop()
{
  Raster.begin();

  loopWifi();
  loopBT();

  loopMpu();
  loopLeds();

  Raster.end();
}
