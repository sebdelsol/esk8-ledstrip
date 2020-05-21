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

// ----------------------------------------------------
#ifdef USE_TELNET
  #include  <TelnetSpy.h>  
  TelnetSpy SerialAndTelnet;  // define first sinc it redefine Serial
  #define   Serial  SerialAndTelnet 
#endif

// --
#ifdef USE_OTA
  #include  <OTA.h>
  OTA       Ota(Serial);
#endif

// --
MOTION  Motion(Serial);
myWifi  MyWifi(Serial);

// --
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

// --
#include  <Cfg.h> // needs Motion obje & BT obj defined
CFG       Cfg;

// ----------------------------------------------------
#define   AQUA          CRGB(0x00FFFF)
#define   AQUA_MENTHE   CRGB(0x7FFFD4)
#define   LUSH_LAVA     CRGB(0xFF4500)
#define   HUE_AQUA_BLUE 140

AllLedStrips  AllLeds(LED_MAX_MA, Serial);

LedStrip    <NBLEDS_MIDDLE, LED_PIN> Leds(Serial, "Led");
RunningFX   FireRun(LUSH_LAVA, 3);     
RunningFX   AquaRun(AQUA_MENTHE, -3);  
TwinkleFX   FireTwk(HUE_RED); 
TwinkleFX   AquaTwk(HUE_AQUA_BLUE);
PlasmaFX    Plasma;

LedStrip    <NBLEDS_TIPS, LEDR_PIN>  LedsR(Serial, "LedR");
DblCylonFX  CylonR(LUSH_LAVA); 
FireFX      FireRL;
FireFX      FireRR(true);
TwinkleFX   TwinkleR(CRGB::Red);
RunningFX   RunR(CRGB::Gold); 

LedStrip    <NBLEDS_TIPS, LEDF_PIN>  LedsF(Serial, "LedF");
DblCylonFX  CylonF(AQUA);   
PacificaFX  Pacifica;
TwinkleFX   TwinkleF(HUE_AQUA_BLUE); 
RunningFX   RunF(CRGB::Gold);

// ----------------------------------------------------
void setup()
{
  // -- switch off all leds
  AllLeds.setBrightness(0);
  AllLeds.clearAndShow();

  // -- main inits
  Serial.begin(SERIAL_BAUD);

  Serial << endl << "---------" << endl;
  Serial << "ESP32 " << esp_get_idf_version() << endl;
  Serial << "CPU runs @ " << rtc_clk_cpu_freq_get() * 80 << "MHz" << endl;
  Serial << "Main runs on Core " << xPortGetCoreID() << endl;

  Cfg.init();
  Motion.init();

  // -- Leds inits
  #define AddFX(l, fx) l.registerFX(fx)

  AllLeds.registerStrip(Leds); 
  AllLeds.registerStrip(LedsR); 
  AllLeds.registerStrip(LedsF); 
  AllLeds.init();

  AddFX(Leds, FireRun);   AddFX(Leds, FireTwk); AddFX(Leds, AquaRun);   AddFX(Leds, AquaTwk);   AddFX(Leds, Plasma);
  AddFX(LedsR, TwinkleR); AddFX(LedsR, FireRR); AddFX(LedsR, FireRL);   AddFX(LedsR, RunR);     AddFX(LedsR, CylonR);
  AddFX(LedsF, TwinkleF); AddFX(LedsF, RunF);   AddFX(LedsF, Pacifica); AddFX(LedsF, CylonF);

  // -- Register AllObj
  #define AddOBJ(o) AllObj.registerObj(o, #o);
  
  AllObj.init();
  AddOBJ(Motion);    AddOBJ(Cfg);            
  AddOBJ(TwinkleF);  AddOBJ(RunF);    AddOBJ(Pacifica); 
  AddOBJ(TwinkleR);  AddOBJ(FireRR);  AddOBJ(FireRL);   AddOBJ(RunR);      AddOBJ(CylonR);
  AddOBJ(FireRun);   AddOBJ(FireTwk); AddOBJ(AquaRun);  AddOBJ(AquaTwk);   AddOBJ(Plasma);  AddOBJ(CylonF);

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
      MyWifi.addLeds(Leds);   
      MyWifi.addLeds(LedsR);  
      MyWifi.addLeds(LedsF);
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
    // -- Update Motion sensor
    Motion.update(); 
    RASTER("Motion");

    // -- Master brightness
    if(Cfg.probe)
    {
      int light = analogRead(LDR_PIN);
      Cfg.bright = map(light, Cfg.minProbe, MaxProbe, 255, 0); // the darker the light, the brighter the leds
    }
    Cfg.fade = lerp16by16(Cfg.fade,  65535,  650);
    byte bright = (Cfg.bright * ((Cfg.fade >> 8) + 1)) >> 8; 
    AllLeds.setBrightness(bright);
    RASTER("probe");

    // -- handle Motion Sensor
    SensorOutput& m = Motion.mOutput;
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

      // -- Fwd leds
      static int FWD = 0;
      int fwd = constrain(acc, 0, 65535);
      FWD = fwd > FWD ? fwd : lerp16by16(FWD, fwd, Cfg.smoothAcc);

      int alphaF = constrain((FWD - (Cfg.thresAcc << 8))/(MAXACC - Cfg.thresAcc), 0, 255);
      int eyeF = Cfg.minEye + (((Cfg.maxEye - Cfg.minEye) * alphaF) >> 8);

      if (Cfg.ledF)
      { 
        RunF.setSpeed(runSpeed);
        RunF.setAlpha(alpha);
        CylonF.setEyeSize(eyeF);
        CylonF.setAlpha(MulAlpha(255 - Cfg.pacifica, invAlpha)); 
        Pacifica.setAlpha(MulAlpha(Cfg.pacifica, invAlpha)); 
        TwinkleF.setAlpha(MulAlpha(alphaF, invAlpha)); 
      }

      // -- Rwd Led
      static int RWD = 0;
      int rwd = constrain(-acc, 0, 65535);
      RWD = rwd > RWD ? rwd : lerp16by16(RWD, rwd, Cfg.smoothAcc);

      int alphaR = constrain((RWD - (Cfg.thresAcc << 8))/(MAXACC - Cfg.thresAcc), 0, 255);
      int eyeR = Cfg.minEye + (((Cfg.maxEye - Cfg.minEye) * alphaR) >> 8);
      int dim = Cfg.minDim + (((Cfg.maxDim - Cfg.minDim) * alphaR) >> 8);

      if (Cfg.ledR)
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

      // -- Central Leds
      if (Cfg.led)
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
    }
    RASTER("led setup");

    // -- Leds actual drawing
    AllLeds.update();
    RASTER("Led update");

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
      RASTER("wifi");
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
    RASTER("BT");
  #endif

  // -- Leds dithering
  #ifdef DEBUG_LED_INFO
    EVERY_N_SECONDS(1)
      AllLeds.getInfo();
  #endif

  AllLeds.show(); // to be called as much as possible for Fastled brightness dithering
  RASTER("led show"); 

  RASTER_END;
}
