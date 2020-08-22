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
  #include  <NoBluetooth.h>
  #include  <AllObj.h>
  AllObj    AllObj(Serial);
#endif

// -- Cfg
#include  <Cfg.h> // needs Motion & BT objs defined
CFG       Cfg;

// --- Strips & Fxs
AllLedStrips  AllStrips(LED_MAX_MA, Serial);

LedStrip    <NBLEDS_MIDDLE, LEDM_PIN> StripM(Serial, "Mid");
RunningFX   FireRun(LUSH_LAVA, 3);     
RunningFX   AquaRun(AQUA_MENTHE, -3);  
TwinkleFX   FireTwk(HUE_RED); 
TwinkleFX   AquaTwk(HUE_AQUA_BLUE);
PlasmaFX    Plasma;

LedStrip    <NBLEDS_TIPS, LEDR_PIN>  StripR(Serial, "Rear");
DblCylonFX  CylonR(LUSH_LAVA); 
FireFX      FireRL;
FireFX      FireRR(true); // reverse
TwinkleFX   TwinkleR(CRGB::Red);
RunningFX   RunR(CRGB::Gold); 

LedStrip    <NBLEDS_TIPS, LEDF_PIN>  StripF(Serial, "Front");
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
  Serial << "\n---------\n- START -\n---------\n";
  Serial << "ESP32 " << esp_get_idf_version() << endl;
  Serial << "CPU runs @ " << rtc_clk_cpu_freq_get() * 80 << "MHz" << endl;
  Serial << "Main runs on Core " << xPortGetCoreID() << endl;

  // -- main inits
  Cfg.init();
  Motion.init();

  // -- Strip inits & register FXs
  AllStrips.addStrips(StripM, StripR, StripF); 
  AllStrips.init();
  
  StripM.addFXs(FireRun,  FireTwk, AquaRun,  AquaTwk, Plasma);
  StripR.addFXs(TwinkleR, FireRR,  FireRL,   RunR,    CylonR);
  StripF.addFXs(TwinkleF, RunF,    Pacifica, CylonF);

  // -- Register AllObj
  #define _addObj(cat, obj)  AllObj.addObj(obj, cat#obj);
  #define AddObjs(cat, ...)  ForEachMacro(_addObj, cat, __VA_ARGS__)

  AllObj.init();

  AddObjs("",        Motion,   Cfg);            
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
  #if defined(DEBUG_LED_TOWIFI) || defined(USE_OTA) || defined(USE_TELNET)
    MyWifi.start();
    #ifdef DEBUG_LED_TOWIFI
      MyWifi.addStrips(StripM, StripR, StripF)
    #endif
  #else
    MyWifi.stop();
  #endif
}

// ----------------------------------------------------
void loop()
{
  RASTER_BEGIN;

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
