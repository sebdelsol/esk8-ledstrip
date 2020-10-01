#define USE_BT
#define USE_OTA 
// #define USE_TELNET 
// #define USE_LEDSERVER

// #define DEBUG_RASTER
// #define DEBUG_LED_INFO

// ----------------------------------------------------
#include <ledstrip.h>
#include <mpu.h>
#include <Raster.h>
#include <myWifi.h>

#define USE_WIFI (defined(USE_LEDSERVER) || defined(USE_OTA) || defined(USE_TELNET))

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

// -- LedServer
#ifdef USE_LEDSERVER
  #include  <ledserver.h>
  LedServer LedServer;
#endif

// -- Acc & Wifi
myWifi  MyWifi;
MPU     Mpu;

// -- BT & Cfg
#ifdef USE_BT
  #include  <Bluetooth.h>
  #include  <Button.h>
  #include  <AllObjBT.h>
  Button    Button(BUTTON_PIN, LOW);
  AllObjBT  AllObj;
  BlueTooth BT;
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
  _log << _FMT("ESP32 %", esp_get_idf_version()) << endl;
  _log << _FMT("Main runs on Core % @ % MHz", xPortGetCoreID(), getCpuFrequencyMhz()) << endl;

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
    BT.init(true); // and start
  #else   
    NoBT();
  #endif
  
  // -- Wifi
  #if USE_WIFI
    MyWifi.start();

    #ifdef USE_LEDSERVER
      LedServer.addAllStrips(AllStrips);
    #endif

    #ifdef USE_TELNET
      Telnet.setWelcomeMsg("");
    #endif

  #else
    MyWifi.stop();
  #endif
}

// ----------------------------------------------------
Raster Raster; // use only string literals for .add()

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
    bool isClient = false;

    #ifdef USE_TELNET
      Telnet.handle();
      isClient |= Telnet.isClientConnected();
    #endif

    #ifdef USE_LEDSERVER
      isClient |= LedServer.update();
    #endif

    #ifdef USE_OTA
      // ota crash when other socket clients are connected
      if (!isClient) Ota.update();
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
    if (Button.debounce()) BT.toggle();
    
    AllObj.receiveUpdate(BT);
  }
  Raster.add("BlueTooth");
#endif
}

// ------------
inline byte max0(int acc) { return acc >=0 ? acc : 0; }
inline byte normalized(int acc, byte _min, byte _max) { return _min + (((_max - _min) * acc) >> 8); }

inline void loopLeds()
{
  EVERY_N_MILLISECONDS(LED_TICK)
  {
    // led setup modified by MPU
    SensorOutput& m = Mpu.mOutput;
    if (m.updated)
    {
      // -- gyro
      int8_t runSpeed =  ((m.w > 0) - (m.w < 0)) * Twk.runSpeed;
      byte rot = abs(m.w);
      byte invrot = 255 - rot;

      // -- acc
      byte fwd = max0(m.acc);
      byte rwd = max0(-m.acc);

      // -- front strip
      if (Twk.stripFront)
      { 
        RunF.setSpeed(runSpeed);
        RunF.setAlpha(rot);
        CylonF.setEyeSize(normalized(fwd, Twk.minEye, Twk.maxEye));
        CylonF.setAlphaMul(255 - Twk.pacifica, invrot); 
        Pacifica.setAlphaMul(Twk.pacifica, invrot); 
        TwinkleF.setAlphaMul(fwd, invrot); 
      }

      // -- rear Strip
      if (Twk.stripRear)
      { 
        RunR.setSpeed(runSpeed);
        RunR.setAlpha(rot);
        CylonR.setEyeSize(normalized(rwd, Twk.minEye, Twk.maxEye));
        CylonR.setAlphaMul(255 - Twk.fire, invrot); 
        FireR.setAlphaMul(Twk.fire, invrot); 
        FireL.setAlphaMul(Twk.fire, invrot);
        byte dim = normalized(rwd, Twk.minDim, Twk.maxDim);
        FireR.setDimRatio(dim); 
        FireL.setDimRatio(dim); 
        TwinkleR.setAlphaMul(max(Twk.minTwkR, rwd), invrot); 
      }

      // -- mid Strip
      if (Twk.stripMid)
      {
        AquaRun.setAlpha(fwd);
        AquaTwk.setAlpha(fwd);
        FireRun.setAlpha(rwd);
        FireTwk.setAlpha(rwd);
        Plasma.setAlpha(max(0, 255 - max(rwd, fwd)));
      }
  
      Raster.add("Leds setup");
    }

    // -- update
    AllStrips.update();
    Raster.add("Leds update");
  }

  // -- Leds dithering
  if (AllStrips.doDither()) Raster.add("Leds dither"); 

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
