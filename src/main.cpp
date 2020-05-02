#define USE_BT // see p latformio & use "board_build.partitions = huge_app.csv"
#define USE_OTA 
// #define USE_TELNET //needs USE_OTA to work

// #define DEBUG_LED_INFO
// #define DEBUG_LED_TOWIFI // use wifi
// #define DEBUG_ACC

// ----------------------------------------------------
#include <ledstrip.h>
#include <myMpu6050.h>
#include <myWifi.h>
#include <Streaming.h>
#include <soc/rtc.h> // get cpu freq

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

  #ifdef USE_TELNET
    #include <TelnetSpy.h>
    TelnetSpy SerialAndTelnet;
    #define Serial  SerialAndTelnet
  #endif
#endif

void handleOta()
{
  #ifdef USE_OTA
    Ota.update();
    #ifdef USE_TELNET
      SerialAndTelnet.handle();
    #endif
  #endif
}

// ----------------------------------------------------
myWifi    MyWifi;

// ----------------------------------------------------
#define   SERIAL_BAUD   115200  // ms
#define   LED_MAX_MA    800     // mA, please check OBJVar.bright to avoid reaching this value
#define   LED_TICK      15      // ms
#define   BT_TICK       15      // ms

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
DblCylonFX  CylonR(LUSH_LAVA); 
FireFX      FireRL;
FireFX      FireRR(true);
TwinkleFX   TwinkleR(CRGB::Red);
RunningFX   RunR(CRGB::Gold); 

LedStrip    <NBLEDS_TIPS, LEDF_PIN>  LedsF("LedF");
PacificaFX  Pacifica;
DblCylonFX  CylonF(AQUA);   
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

  // light probe ?
  #define MaxProbe 4095
  bool probe      = false;
  byte bright     = 128; // half brightness is enough to avoid reaching LED_MAX_MA
  int  minProbe   = 400;

  // pacifica ?
  byte pacifica   = 255;
  byte fire       = 0;

  // for rotation
  byte runSpeed    = 3;
  int  neutralWZ   = 3000;
  int  maxWZ       = 7000; 

  // for acc
  byte divAcc     = 2;
  int  smoothAcc  = 1600;
  byte thresAcc   = 30;
  
  // Cylons
  byte minEye     = 5;
  byte maxEye     = 10;

  // Fire
  int  minDim     = 4;
  int  maxDim     = 10;

  //twinkleR
  int minTwkR     = 20;

  #ifdef USE_BT
    CFG()
    {
      #define REGISTER_CFG(var, min, max) REGISTER_VAR_SIMPLE(CFG, #var, self->var, min, max)

      REGISTER_CMD(CFG,        "save",      {BT.save(false);} )       // save not default
      REGISTER_CMD(CFG,        "load",      {BT.load(false);} )       // load not default
      REGISTER_CMD(CFG,        "default",   {BT.load(true);}  )       // load default
      REGISTER_CMD_NOSHOW(CFG, "getInits",  {BT.sendInitsOverBT();} ) // answer with all vars init (min, max, value)
      REGISTER_CMD_NOSHOW(CFG, "getUpdate", {sendUpdate();} )         // answer with all updates

      REGISTER_CFG(ledR,       0, 1);
      REGISTER_CFG(ledF,       0, 1);
      REGISTER_CFG(led,        0, 1);

      REGISTER_CFG(probe,      0, 1);
      REGISTER_CFG(bright,     1, 255);
      REGISTER_CFG(minProbe,   1, MaxProbe);

      REGISTER_CFG(pacifica,   0, 255);
      REGISTER_CFG(fire,       0, 255);

      REGISTER_CFG(runSpeed,   0, 10);
      REGISTER_CFG(neutralWZ,  0, 32768);
      REGISTER_CFG(maxWZ,      0, 32768);

      REGISTER_CFG(divAcc,     1, 10);
      REGISTER_CFG(smoothAcc,  1, 32768);
      REGISTER_CFG(thresAcc,   0, 255);

      REGISTER_CFG(minEye,     1, (NBLEDS_TIPS>>1));
      REGISTER_CFG(maxEye,     1, (NBLEDS_TIPS>>1));
      REGISTER_CFG(minDim,     1, 10);
      REGISTER_CFG(maxDim,     1, 10);
      REGISTER_CFG(minTwkR,    0, 255);
    };
  #endif
};

CFG Cfg;

// ----------------------------------------------------
void setup()
{
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

  AddFX(Leds, FireRun);   AddFX(Leds, FireTwk); AddFX(Leds, AquaRun); AddFX(Leds, AquaTwk);   AddFX(Leds, Plasma);
  AddFX(LedsR, TwinkleR); AddFX(LedsR, CylonR); AddFX(LedsR, FireRR); AddFX(LedsR, FireRL);   AddFX(LedsR, RunR);
  AddFX(LedsF, TwinkleF); AddFX(LedsF, CylonF); AddFX(LedsF, RunF);   AddFX(LedsF, Pacifica);

  AllLeds.clearAndShow();
  
  // Wifi -----------------------------
  MyWifi.init(Serial);
  
  #if defined(DEBUG_LED_TOWIFI) || defined(USE_OTA)
    MyWifi.on();
  
    #ifdef DEBUG_LED_TOWIFI
      MyWifi.addLeds(Leds);   
      MyWifi.addLeds(LedsR);  
      MyWifi.addLeds(LedsF);
    #endif
  
    #ifdef USE_OTA
      Ota.begin();
    #endif
  
  #else
    MyWifi.off();
  #endif

  // BlueTooth -----------------------------
  #ifdef USE_BT
    #define AddOBJ(o) BT.registerObj(o, #o);
    
    BT.init(Serial);

    AddOBJ(Cfg);
    AddOBJ(TwinkleF);  AddOBJ(CylonF);  AddOBJ(RunF);    AddOBJ(Pacifica);
    AddOBJ(TwinkleR);  AddOBJ(CylonR);  AddOBJ(FireRR);  AddOBJ(FireRL);    AddOBJ(RunR);
    AddOBJ(FireRun);   AddOBJ(FireTwk); AddOBJ(AquaRun); AddOBJ(AquaTwk);   AddOBJ(Plasma);

    BT.save(true); // save default
    BT.load(false, false); // load not default, do not send change to BT
    BT.start();

    Button.begin();

  #else    
    pinMode(LIGHT_PIN, OUTPUT); //blue led
    digitalWrite(LIGHT_PIN, LOW); // switch off blue led
    btStop(); // turnoff bt 
  #endif

  // accel -----------------------------
  Accel.begin(Serial, &handleOta);
}

// ----------------------------------------------------
void loop()
{
  handleOta();

  GotAccel = Accel.getMotion(AXIS, ANGLE, VACC, WZ);

  #ifdef USE_BT
    if (Button.pressed())
      BT.toggle();
    
    EVERY_N_MILLISECONDS(BT_TICK)
      BT.update();
  #endif

  EVERY_N_MILLISECONDS(LED_TICK)
  {
    
    // Master brightness
    if(Cfg.probe)
    {
      int light = analogRead(LDR_PIN);
      Cfg.bright = map(light, Cfg.minProbe, MaxProbe, 255, 0); // to darker the light, the brighter the leds
    }
    AllLeds.setBrightness(Cfg.bright);

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

      //------
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
        Serial << "[areal  " << VACC.x << "\t" << VACC.y << "\t" << VACC.z << "]\t";
        Serial << "[fwd " << fwd << "\trwd " << rwd << "\tACC " << acc << "]\t";
        Serial << "[alpha " << alpha << "\tinv " << invAlpha << "]\t";
        Serial << "[eyeR " << eyeR << "\teyeF " << eyeF << "\talphaR " << alphaR << "\talphaF " << alphaF << "]" << endl; //"       \r";//endl;
      #endif
    }

    AllLeds.update();

    #ifdef DEBUG_LED_TOWIFI
      MyWifi.update();
    #endif
  }

  #ifdef DEBUG_LED_INFO
    EVERY_N_MILLISECONDS(1000)
      AllLeds.getInfo();
  #endif

  AllLeds.show(); // to be called as much as possible for Fastled brightness dithering
}
