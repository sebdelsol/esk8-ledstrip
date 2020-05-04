#define USE_BT
#define USE_OTA 
// #define USE_TELNET 
// #define USE_PROBE
// #define USE_RASTER
// #define USE_CYLON

// #define DEBUG_LED_INFO
// #define DEBUG_LED_TOWIFI
// #define DEBUG_ACC


// ----------------------------------------------------
#include <ledstrip.h>
#include <myMpu6050.h>
#include <myWifi.h>
#include <Streaming.h>
#include <soc/rtc.h> // cpu freq

myWifi    MyWifi;

// ----------------------------------------------------
#ifdef USE_BT
  #include <bluetooth.h>
  #include <Button.h>

  BlueTooth BT;
  Button    Button(BUTTON_PIN);
#endif

// ----------------------------------------------------
#ifdef USE_OTA
  #include <OTA.h>
  OTA Ota;
#endif

#ifdef USE_TELNET
  #include <TelnetSpy.h>
  TelnetSpy SerialAndTelnet;
  #define Serial  SerialAndTelnet
#endif

// ----------------------------------------------------
#define   SERIAL_BAUD   115200  // ms
#define   LED_MAX_MA    800     // mA, please check OBJVar.bright to avoid reaching this value
#define   LED_TICK      15      // ms
#define   BT_TICK       30      // ms

#define   NBLEDS_MIDDLE 30
#define   NBLEDS_TIPS   36

#define   AQUA          CRGB(0x00FFFF)
#define   AQUA_MENTHE   CRGB(0x7FFFD4)
#define   LUSH_LAVA     CRGB(0xFF4500)
#define   HUE_AQUA_BLUE 140

// ----------------------------------------------------
AllLedStrips  AllLeds(LED_MAX_MA, Serial);

LedStrip    <NBLEDS_MIDDLE, LED_PIN> Leds("Led");
RunningFX   FireRun(LUSH_LAVA, 10, 3);     
RunningFX   AquaRun(AQUA_MENTHE, 10, -3);  
TwinkleFX   FireTwk(HUE_RED); 
TwinkleFX   AquaTwk(HUE_AQUA_BLUE);
PlasmaFX    Plasma;

LedStrip    <NBLEDS_TIPS, LEDR_PIN>  LedsR("LedR");
#ifdef USE_CYLON
  DblCylonFX  CylonR(LUSH_LAVA); 
#endif
FireFX      FireRL;
FireFX      FireRR(true);
TwinkleFX   TwinkleR(CRGB::Red);
RunningFX   RunR(CRGB::Gold); 

LedStrip    <NBLEDS_TIPS, LEDF_PIN>  LedsF("LedF");
#ifdef USE_CYLON
  DblCylonFX  CylonF(AQUA);   
#endif
PacificaFX  Pacifica;
TwinkleFX   TwinkleF(HUE_AQUA_BLUE); 
RunningFX   RunF(CRGB::Gold);

// ----------------------------------------------------
myMPU6050     Accel;
bool          GotAccel = false;
VectorInt16   AXIS, VACC;
int           ANGLE, WZ;

// ----------------------------------------------------
#ifdef USE_BT
  void sendUpdate()
  {
    if(BT.sendUpdate() && GotAccel)
      *BT.getBtSerial() << "A " << AXIS.x << " " << AXIS.y << " " << AXIS.z << " " << ANGLE << " " << VACC.y << " " << WZ << endl;
  }
#endif

// ----------------------------------------------------
class CFG : public OBJVar
{
public:
  
  // update ?
  bool ledR       = true;
  bool ledF       = true;
  bool led        = true;

  // brightness ?
  byte bright     = 128;  // half brightness is enough to avoid reaching LED_MAX_MA
  int fade        = 0;    // for the fade in
  #ifdef USE_PROBE
    #define MaxProbe 4095
    int  minProbe   = 400;
    bool probe      = false;
  #endif

  #ifdef USE_CYLON
    byte pacifica   = 255;
    byte fire       = 0;
  #endif

  // for rotation
  byte runSpeed    = 3;
  int  neutralWZ   = 3000;
  int  maxWZ       = 7000; 

  // for acc
  byte divAcc     = 2;
  int  smoothAcc  = 1600;
  byte thresAcc   = 30;
  
  #ifdef USE_CYLON
    byte minEye     = 5;
    byte maxEye     = 10;
  #endif

  // Fire
  int  minDim     = 4;
  int  maxDim     = 10;

  //twinkleR
  int minTwkR     = 128;

  CFG()
  {
    #define REGISTER_CFG(var, min, max) REGISTER_VAR_SIMPLE(CFG, #var, self->var, min, max)

    #ifdef USE_BT
      REGISTER_CMD(CFG,        "save",      {BT.save(false);} )       // save not default
      REGISTER_CMD(CFG,        "load",      {BT.load(false);} )       // load not default
      REGISTER_CMD(CFG,        "default",   {BT.load(true);}  )       // load default
      REGISTER_CMD_NOSHOW(CFG, "getInits",  {BT.sendInitsOverBT();} ) // answer with all vars init (min, max, value)
      REGISTER_CMD_NOSHOW(CFG, "getUpdate", {sendUpdate();} )         // answer with all updates
    #endif

    REGISTER_CFG(ledR,       0, 1);
    REGISTER_CFG(ledF,       0, 1);
    REGISTER_CFG(led,        0, 1);

    REGISTER_CFG(bright,     1, 255);
    #ifdef USE_PROBE
      REGISTER_CFG(minProbe,   1, MaxProbe);
      REGISTER_CFG(probe,      0, 1);
    #endif

    #ifdef USE_CYLON
      REGISTER_CFG(pacifica,   0, 255);
      REGISTER_CFG(fire,       0, 255);
    #endif

    REGISTER_CFG(runSpeed,   0, 10);
    REGISTER_CFG(neutralWZ,  0, 32768);
    REGISTER_CFG(maxWZ,      0, 32768);

    REGISTER_CFG(divAcc,     1, 10);
    REGISTER_CFG(smoothAcc,  1, 32768);
    REGISTER_CFG(thresAcc,   0, 255);

    #ifdef USE_CYLON
      REGISTER_CFG(minEye,     1, (NBLEDS_TIPS>>1));
      REGISTER_CFG(maxEye,     1, (NBLEDS_TIPS>>1));
    #endif

    REGISTER_CFG(minDim,     1, 10);
    REGISTER_CFG(maxDim,     1, 10);
    REGISTER_CFG(minTwkR,    0, 255);
  };
};

CFG Cfg;

// ----------------------------------------------------
void setup()
{
  // switch off all leds 
  AllLeds.setBrightness(0);
  AllLeds.clearAndShow();

  // esp32 inits
  Serial.begin(SERIAL_BAUD);
  Serial << endl << "-------- START --------" << endl;
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);
  Serial << "Esp32 core " << esp_get_idf_version() << endl;
  Serial << "CPU freq " << rtc_clk_cpu_freq_get() * 80 << "MHz" << endl;

  // LEDS -----------------------------
  #define AddFX(l, fx) l.registerFX(fx)

  AllLeds.registerStrip(Leds); 
  AllLeds.registerStrip(LedsR); 
  AllLeds.registerStrip(LedsF); 

  AddFX(Leds, FireRun);   AddFX(Leds, FireTwk); AddFX(Leds, AquaRun);   AddFX(Leds, AquaTwk);   AddFX(Leds, Plasma);
  AddFX(LedsR, TwinkleR); AddFX(LedsR, FireRR); AddFX(LedsR, FireRL);   AddFX(LedsR, RunR);     
  AddFX(LedsF, TwinkleF); AddFX(LedsF, RunF);   AddFX(LedsF, Pacifica); 
  #ifdef USE_CYLON 
    AddFX(LedsR, CylonR); 
    AddFX(LedsF, CylonF);
  #endif

  // BlueTooth -----------------------------
  #ifdef USE_BT
    #define AddOBJ(o) BT.registerObj(o, #o);
    
    BT.init(Serial);

    AddOBJ(Accel);     AddOBJ(Cfg);            
    AddOBJ(TwinkleF);  AddOBJ(RunF);    AddOBJ(Pacifica); 
    AddOBJ(TwinkleR);  AddOBJ(FireRR);  AddOBJ(FireRL);   AddOBJ(RunR);      
    AddOBJ(FireRun);   AddOBJ(FireTwk); AddOBJ(AquaRun);  AddOBJ(AquaTwk);   AddOBJ(Plasma);
    #ifdef USE_CYLON 
      AddOBJ(CylonR);
      AddOBJ(CylonF);
    #endif

    BT.save(true); // save default
    BT.load(false, false); // load not default, do not send change to BT
    BT.start();

    Accel.begin(Serial);
    Button.begin();

  #else    
    Accel.begin(Serial, true); // calibrate

    pinMode(LIGHT_PIN, OUTPUT); //blue led
    digitalWrite(LIGHT_PIN, LOW); // switch off blue led
    btStop(); // turnoff bt 
  #endif

  // Wifi -----------------------------
  MyWifi.init(Serial);
  
  #if defined(DEBUG_LED_TOWIFI) || defined(USE_OTA) || defined(USE_TELNET)
    MyWifi.on();
  
    #ifdef DEBUG_LED_TOWIFI
      MyWifi.addLeds(Leds);   
      MyWifi.addLeds(LedsR);  
      MyWifi.addLeds(LedsF);
    #endif
  
  #else
    MyWifi.off();
  #endif
}

// ----------------------------------------------------
#ifdef USE_RASTER
  #define RASTER_BEGIN(nb) \
    struct Raster \
    { \
      const __FlashStringHelper* name; \
      long time; \
    } _rasters[nb]; \
    int _rasterMax = nb; \
    int _rasterCount = 0; \
    long _startTime = micros(); \

  #define RASTER(txt) \
    if (_rasterCount < _rasterMax) \
    { \
      _rasters[_rasterCount].time = micros(); \
      _rasters[_rasterCount++].name = F(txt); \
    } 

  #define RASTER_END \
    long _endTime = micros(); \
    Serial << "TOTAL " << (_endTime - _startTime) << "ms"; \
    for(byte i=0; i < _rasterCount; i++) \
      Serial << " \t - " << _rasters[i].name << " " << (_rasters[i].time - (i==0 ? _startTime : _rasters[i-1].time)) << "ms  "; \
    Serial << endl;

#else
  #define RASTER_BEGIN(nb)
  #define RASTER(txt)
  #define RASTER_END
#endif

// ----------------------------------------------------
void loop()
{
  RASTER_BEGIN(20);

  EVERY_N_MILLISECONDS(10) // MPU6050 is refreshed every 10ms
  {
    GotAccel = Accel.getMotion(AXIS, ANGLE, VACC, WZ);
    RASTER("accel");
  }

  #ifdef USE_BT
    EVERY_N_MILLISECONDS(BT_TICK)
    {
      if (Button.pressed())
      {
        Serial << "button pressed" << endl;
        BT.toggle();
      }
      BT.update();
    }
    RASTER("BT");
  #endif

  EVERY_N_MILLISECONDS(LED_TICK)
  {
    // Master brightness
    #ifdef USE_PROBE
      if(Cfg.probe)
      {
        int light = analogRead(LDR_PIN);
        Cfg.bright = map(light, Cfg.minProbe, MaxProbe, 255, 0); // the darker the light, the brighter the leds
      }
    #endif
    Cfg.fade = lerp16by16(Cfg.fade,  65535,  650);
    byte bright = (Cfg.bright * ((Cfg.fade >> 8) + 1)) >> 8; 
    AllLeds.setBrightness(bright);

    // handle motion
    if (GotAccel)
    {
      #define MulAlpha(a, b) (((a) * ((b) + 1)) >> 8)

      int runSpeed =  ((WZ>0) - (WZ<0)) * Cfg.runSpeed;

      //------
      int _WZ = abs(WZ);
      byte alpha = _WZ > Cfg.neutralWZ ? min((_WZ - Cfg.neutralWZ) * 255 / Cfg.maxWZ, 255) : 0;
      byte invAlpha = 255 - alpha;

      //----------------------
      #define MAXACC 256
      int acc = constrain(VACC.y / Cfg.divAcc, -MAXACC, MAXACC) << 8;

      //------
      static int FWD = 0;
      int fwd = constrain(acc, 0, 65535);
      FWD = fwd > FWD ? fwd : lerp16by16(FWD, fwd, Cfg.smoothAcc);

      int alphaF = constrain((FWD - (Cfg.thresAcc << 8))/(MAXACC - Cfg.thresAcc), 0, 255);
      #ifdef USE_CYLON
        int eyeF = Cfg.minEye + (((Cfg.maxEye - Cfg.minEye) * alphaF) >> 8);
      #endif

      if (Cfg.ledF)
      { 
        RunF.setSpeed(runSpeed);
        RunF.setAlpha(alpha);
        #ifdef USE_CYLON
          CylonF.setEyeSize(eyeF);
          CylonF.setAlpha(MulAlpha(255 - Cfg.pacifica, invAlpha)); 
          Pacifica.setAlpha(MulAlpha(Cfg.pacifica, invAlpha)); 
          TwinkleF.setAlpha(MulAlpha(alphaF, invAlpha)); 
        #else
          Pacifica.setAlpha(invAlpha); 
          TwinkleF.setAlpha(MulAlpha(alphaF, invAlpha)); 
        #endif
      }

      //------
      static int RWD = 0;
      int rwd = constrain(-acc, 0, 65535);
      RWD = rwd > RWD ? rwd : lerp16by16(RWD, rwd, Cfg.smoothAcc);

      int alphaR = constrain((RWD - (Cfg.thresAcc << 8))/(MAXACC - Cfg.thresAcc), 0, 255);
      #ifdef USE_CYLON
        int eyeR = Cfg.minEye + (((Cfg.maxEye - Cfg.minEye) * alphaR) >> 8);
      #endif
      int dim = Cfg.minDim + (((Cfg.maxDim - Cfg.minDim) * alphaR) >> 8);

      if (Cfg.ledR)
      { 
        RunR.setSpeed(runSpeed);
        RunR.setAlpha(alpha);
        #ifdef USE_CYLON
          CylonR.setEyeSize(eyeR);
          CylonR.setAlpha(MulAlpha(255 - Cfg.fire, invAlpha)); 
          FireRR.setAlpha(MulAlpha(Cfg.fire, invAlpha)); 
          FireRL.setAlpha(MulAlpha(Cfg.fire, invAlpha));
        #else
          FireRR.setAlpha(invAlpha); 
          FireRL.setAlpha(invAlpha);
        #endif
        FireRR.setDimRatio(dim); 
        FireRL.setDimRatio(dim); 
        TwinkleR.setAlpha(MulAlpha(max(Cfg.minTwkR, alphaR), invAlpha)); 
      }

      //----------------------
      if (Cfg.led)
      {
        AquaRun.setAlpha(alphaF);
        AquaTwk.setAlpha(alphaF);
        FireRun.setAlpha(alphaR);
        FireTwk.setAlpha(alphaR);
        Plasma.setAlpha(max(0, 255 - max(alphaR, alphaF)));
      }

      #ifdef DEBUG_ACC
        Serial << "[areal  " << VACC.x << "\t"      << VACC.y   << "\t"        << VACC.z << "]\t";
        Serial << "[fwd "    << fwd    << "\trwd "  << rwd      << "\tACC "    << acc << "]\t";
        Serial << "[alpha "  << alpha  << "\tinv "  << invAlpha << "]\t";
        Serial << "[eyeR "   << eyeR   << "\teyeF " << eyeF     << "\talphaR " << alphaR << "\talphaF " << alphaF << "]" << endl;
      #endif
    }
    RASTER("led setup");

    AllLeds.update();
    RASTER("Led update");

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

  #ifdef DEBUG_LED_INFO
    EVERY_N_MILLISECONDS(1000)
      AllLeds.getInfo();
  #endif

  AllLeds.show(); // to be called as much as possible for Fastled brightness dithering
  RASTER("led show");
  RASTER_END;
}
