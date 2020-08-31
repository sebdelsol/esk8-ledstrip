#define USE_BT
#define USE_OTA 
// #define USE_TELNET 

// #define DEBUG_RASTER
// #define DEBUG_LED_TOWIFI
// #define DEBUG_LED_INFO

#define USE_WIFI (defined(DEBUG_LED_TOWIFI) || defined(USE_OTA) || defined(USE_TELNET))

// ----------------------------------------------------
// #include <Streaming.h>
#include <Pins.h>
#include <ledstrip.h>
#include <mpu.h>
#include <myWifi.h>
#include <Raster.h>

// -- Telnet Serial 
#ifdef USE_TELNET
  #include  <TelnetSpy.h>  
  TelnetSpy SerialAndTelnet;  
  #define   Serial  SerialAndTelnet // redefine Serial
#endif

// -- log, depends on Serial possible redifinition 
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

  #include  <Cfg.h>
  CFG       Cfg(AllObj, BT, Mpu);

#else
  #include  <NoBluetooth.h>
  #include  <AllObj.h>
  AllObj    AllObj;

  #include  <Cfg.h> 
  CFG       Cfg;
#endif

// --- Strips & Fxs
AllLedStrips  AllStrips;

LedStrip    <NBLEDS_MIDDLE, LEDM_PIN> StripM("Mid");
RunningFX   FireRun(LUSH_LAVA, 3);     
RunningFX   AquaRun(AQUA_MENTHE, -3);  
TwinkleFX   FireTwk(HUE_RED); 
TwinkleFX   AquaTwk(HUE_AQUA_BLUE);
PlasmaFX    Plasma;

LedStrip    <NBLEDS_TIPS, LEDR_PIN>  StripR("Rear");
DblCylonFX  CylonR(LUSH_LAVA); 
FireFX      FireRL;
FireFX      FireRR(true); // reverse
TwinkleFX   TwinkleR(CRGB::Red);
RunningFX   RunR(CRGB::Gold); 

LedStrip    <NBLEDS_TIPS, LEDF_PIN>  StripF("Front");
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

  // -- log
  Serial.begin(SERIAL_BAUD);
  _log << "\n---------\n- START -\n---------\n";
  _log << "ESP32 " << esp_get_idf_version() << endl;
  _log << "CPU runs @ " << getCpuFrequencyMhz() << "MHz" << endl;
  _log << "Main runs on Core " << xPortGetCoreID() << endl;

  // -- main inits
  Cfg.init();
  Mpu.init();

  // -- Strip inits & register FXs
  AllStrips.init(LED_MAX_MA, LED_DITHERING);
  AllStrips.addStrips(StripM, StripR, StripF); 
  
  StripM.addFXs(FireRun,  FireTwk, AquaRun,  AquaTwk, Plasma);
  StripR.addFXs(TwinkleR, FireRR,  FireRL,   RunR,    CylonR);
  StripF.addFXs(TwinkleF, RunF,    Pacifica, CylonF);

  // -- Register AllObj
  AllObj.init();

  #define _addObj(cat, obj)  AllObj.addObj(obj, cat#obj);
  #define AddObjs(cat, ...)  ForEachMacro(_addObj, cat, __VA_ARGS__)

  AddObjs("",        Mpu,      Cfg);            
  AddObjs("mid.",    FireRun,  FireTwk, AquaRun,  AquaTwk,  Plasma);
  AddObjs("rear.",   TwinkleR, FireRR,  FireRL,   RunR,     CylonR);
  AddObjs("front.",  TwinkleF, RunF,    Pacifica, CylonF);

  AllObj.save(true); // save default
  AllObj.load(false, false); // load not default, do not send change to BT

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
  EVERY_N_MILLISECONDS(WIFI_TICK)
  {
    if(MyWifi.update())
    {
      #ifdef USE_TELNET
        SerialAndTelnet.handle();
      #endif

      #ifdef USE_OTA
        if (!MyWifi.isWSConnected()) // concurrency issue with WSockets
          Ota.update();
      #endif
    }
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
    if (Button.pressed())
      BT.toggle();

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
    // -- Master brightness
    if(Cfg.probe)
    {
      int light = analogRead(LDR_PIN);
      Cfg.bright = map(light, Cfg.minProbe, MAX_probe, 255, 0); // the darker the light, the brighter the leds
    }
    Cfg.fade = lerp16by16(Cfg.fade,  65535,  650);
    byte bright = (Cfg.bright * ((Cfg.fade >> 8) + 1)) >> 8; 
    AllStrips.setBrightness(bright);
    Raster.add("Light probe");

    // led setup modified by MPU
    SensorOutput& m = Mpu.mOutput;
    if (m.updated)
    {
      // -- Gyro
      int runSpeed =  ((m.wZ > 0) - (m.wZ < 0)) * Cfg.runSpeed;
      int WZ = abs(m.wZ);
      byte alpha = WZ > Cfg.neutralWZ ? min((WZ - Cfg.neutralWZ) * 255 / Cfg.maxWZ, 255) : 0;
      byte invAlpha = 255 - alpha;

      // -- Acc
      #define MAXACC 256
      int acc = constrain(m.accY / Cfg.divAcc, -MAXACC, MAXACC) << 8;

      // -- Front strip
      int fwd = constrain(acc, 0, 65535);
      Cfg.FWD = fwd > Cfg.FWD ? fwd : lerp16by16(Cfg.FWD, fwd, Cfg.smoothAcc);

      int alphaF = constrain((Cfg.FWD - (Cfg.thresAcc << 8))/(MAXACC - Cfg.thresAcc), 0, 255);
      int eyeF = Cfg.minEye + (((Cfg.maxEye - Cfg.minEye) * alphaF) >> 8);

      if (Cfg.stripFront)
      { 
        RunF.setSpeed(runSpeed);
        RunF.setAlpha(alpha);
        CylonF.setEyeSize(eyeF);
        CylonF.setAlphaMul(255 - Cfg.pacifica, invAlpha); 
        Pacifica.setAlphaMul(Cfg.pacifica, invAlpha); 
        TwinkleF.setAlphaMul(alphaF, invAlpha); 
      }

      // -- Rear Strip
      int rwd = constrain(-acc, 0, 65535);
      Cfg.RWD = rwd > Cfg.RWD ? rwd : lerp16by16(Cfg.RWD, rwd, Cfg.smoothAcc);

      int alphaR = constrain((Cfg.RWD - (Cfg.thresAcc << 8))/(MAXACC - Cfg.thresAcc), 0, 255);
      int eyeR = Cfg.minEye + (((Cfg.maxEye - Cfg.minEye) * alphaR) >> 8);
      int dim = Cfg.minDim + (((Cfg.maxDim - Cfg.minDim) * alphaR) >> 8);

      if (Cfg.stripRear)
      { 
        RunR.setSpeed(runSpeed);
        RunR.setAlpha(alpha);
        CylonR.setEyeSize(eyeR);
        CylonR.setAlphaMul(255 - Cfg.fire, invAlpha); 
        FireRR.setAlphaMul(Cfg.fire, invAlpha); 
        FireRL.setAlphaMul(Cfg.fire, invAlpha);
        FireRR.setDimRatio(dim); 
        FireRL.setDimRatio(dim); 
        TwinkleR.setAlphaMul(max(Cfg.minTwkR, alphaR), invAlpha); 
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
  
      Raster.add("Leds setup");
    }

    // -- update ... and showing if dithering is off
    AllStrips.update();
    if (!LED_DITHERING) AllStrips.show(); 
    
    Raster.add("Leds update");
  }

  // -- Leds dithering
  if (LED_DITHERING)
  {
    AllStrips.show(); // to be called as much as possible for Fastled brightness dithering
    Raster.add("Leds dither"); 
  }

  #ifdef DEBUG_LED_INFO
    EVERY_N_SECONDS(1) AllStrips.showInfo();
  #endif
}

// --------
void loop()
{
  Raster.begin();
  loopMpu();
  loopWifi();
  loopBT();
  loopLeds();
  Raster.end();
}
