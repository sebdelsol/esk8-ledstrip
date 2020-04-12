
#define USE_BT // see p latformio & use "board_build.partitions = huge_app.csv"
#define USE_OTA 
// #define USE_TELNET //needs USE_OTA to work
// #define USE_LIGHTPROBE

// #define DEBUG_LED_INFO
// #define DEBUG_LED_TOWIFI // use wifi
// #define DEBUG_ACC

// ----------------------------------------------------
#include <ledstrip.h>
#include <myMpu6050.h>
#include <myWifi.h>
#include <Streaming.h>
#include <soc/rtc.h> // get cpu freq

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
#define   SERIAL_BAUD   115200  // ms
#define   LED_MAX_MA    800     // mA, please check OBJVar.bright to avoid reaching this value
#define   LED_TICK      15      // ms
#define   BT_TICK       15      // ms

// ----------------------------------------------------
myWifi    MyWifi;

// ----------------------------------------------------
AllLedStrips  AllLeds(LED_MAX_MA, Serial);

#define   AQUA_MENTHE   CRGB(0x7FFFD4)
#define   LUSH_LAVA     CRGB(0xFF4500)
#define   HUE_AQUA_BLUE 140

#define   NBLEDS_MIDDLE 30
#define   NBLEDS_TIPS   36

LedStrip    <NBLEDS_MIDDLE, LED_PIN> Leds("Led");
RunningFX   Fire(LUSH_LAVA, 10, 3);     // FireFX   Fire;     // AquaFX   Fire(true);     
RunningFX   Aqua(AQUA_MENTHE, 10, -3);  
TwinkleFX   FireTwk(HUE_RED); 
TwinkleFX   AquaTwk(HUE_AQUA_BLUE);
PlasmaFX    Plasma;

LedStrip    <NBLEDS_TIPS, LEDR_PIN>  LedsR("LedR");
DblCylonFX  CylonR(LUSH_LAVA); 
TwinkleFX   TwinkleR(LUSH_LAVA);
RunningFX   RunR(CRGB::Gold); 

LedStrip    <NBLEDS_TIPS, LEDF_PIN>  LedsF("LedF");
DblCylonFX  CylonF(AQUA_MENTHE);
TwinkleFX   TwinkleF(HUE_AQUA_BLUE); 
RunningFX   RunF(CRGB::Gold);

// ----------------------------------------------------
myMPU6050     Accel;
bool          GotAccel = false;
VectorInt16   DIR, UP, VACC;
int           WZ;

// ----------------------------------------------------
#ifdef USE_BT
  void sendUpdate()
  {
    if(BT.sendUpdate() && GotAccel)
      *(BT.getBtSerial()) << "acc " << DIR.x << " " << DIR.y << " " << DIR.z << " " << UP.x << " " << UP.y << " " << UP.z << " " << VACC.x << " " << VACC.y << " " << VACC.z << endl;
  }
#endif

// ----------------------------------------------------
class CFG : public OBJVar
{
public:
  #ifndef USE_LIGHTPROBE
    byte bright     = 64; // 1/4 brightness is enough to avoid reaching LED_MAX_MA
  #endif
  
  // update ?
  bool ledR       = true;
  bool ledF       = true;
  bool led        = true;

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

      #ifndef USE_LIGHTPROBE
        REGISTER_CFG(bright,   1, 255);
      #endif

      REGISTER_CFG(runSpeed,   0, 10);
      REGISTER_CFG(neutralWZ,  0, 32768);
      REGISTER_CFG(maxWZ,      0, 32768);

      REGISTER_CFG(divAcc,     1, 10);
      REGISTER_CFG(smoothAcc,  1, 32768);
      REGISTER_CFG(thresAcc,   0, 255);

      REGISTER_CFG(minEye,     1, (NBLEDS_TIPS>>1));
      REGISTER_CFG(maxEye,     1, (NBLEDS_TIPS>>1));
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
  #define Register3FX(l, f1, f2, f3)          AllLeds.registerStrip(l);   l.registerFX(f1); l.registerFX(f2); l.registerFX(f3);
  #define Register5FX(l, f1, f2, f3, f4, f5)  Register3FX(l, f1, f2, f3); l.registerFX(f4); l.registerFX(f5);
  
  Register5FX(Leds,   Fire,       FireTwk,    Aqua,   AquaTwk,    Plasma);
  Register3FX(LedsR,  TwinkleR,   CylonR,     RunR);
  Register3FX(LedsF,  TwinkleF,   CylonF,     RunF);
  AllLeds.clearAndShow();
  
  // Wifi -----------------------------
  MyWifi.init(Serial);
  #if defined(DEBUG_LED_TOWIFI) || defined(USE_OTA)
    MyWifi.on();
    #ifdef DEBUG_LED_TOWIFI
      MyWifi.addLeds(Leds);   MyWifi.addLeds(LedsR);  MyWifi.addLeds(LedsF);
    #endif
    #ifdef USE_OTA
      Ota.begin();
    #endif
  #else
    MyWifi.off();
  #endif

  // BlueTooth -----------------------------
  pinMode(LIGHT_PIN, OUTPUT); //blue led
  #ifdef USE_BT
    Button.begin();

    #define BT_REGISTER_OBJ(o) BT.registerObj(o, #o);
    #define BT_REGISTER_3OBJ(o1, o2, o3) BT_REGISTER_OBJ(o1); BT_REGISTER_OBJ(o2); BT_REGISTER_OBJ(o3);
    #define BT_REGISTER_5OBJ(o1, o2, o3, o4, o5) BT_REGISTER_3OBJ(o1, o2, o3); BT_REGISTER_OBJ(o4); BT_REGISTER_OBJ(o5);
    
    BT.init(Serial);
    BT_REGISTER_OBJ(Cfg);
    BT_REGISTER_3OBJ(TwinkleR,  CylonR,     RunR);
    BT_REGISTER_3OBJ(TwinkleF,  CylonF,     RunF);
    BT_REGISTER_5OBJ(Fire,      FireTwk,    Aqua,   AquaTwk,    Plasma);

    BT.save(true); // save default
    BT.load(false, false); // load not default, do not send change to BT
    BT.start();
  #else    
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

  GotAccel = Accel.getMotion(DIR, UP, VACC, WZ);

  #ifdef USE_BT
    if (Button.pressed())
      BT.toggle();
    
    EVERY_N_MILLISECONDS(BT_TICK)
      BT.update();
  #endif

  EVERY_N_MILLISECONDS(LED_TICK)
  {
    #ifdef USE_LIGHTPROBE
      #define MIN_LIGHT 400
      #define MAX_LIGHT 4095
      int light = analogRead(LDR_PIN);
      byte bright = map(light, MIN_LIGHT, MAX_LIGHT, 255, 0); // to darker the light, the brighter the leds
      Serial << light << " " << bright << endl;
      AllLeds.setBrightness(bright);
    #else
      AllLeds.setBrightness(Cfg.bright);
    #endif

    if (GotAccel)
    {
      int runSpeed =  ((WZ>0) - (WZ<0)) * Cfg.runSpeed;

      //------
      int _WZ = abs(WZ);
      byte alpha = _WZ > Cfg.neutralWZ ? min((_WZ-Cfg.neutralWZ) * 255 / Cfg.maxWZ, 255) : 0;
      byte invAlpha = 255 - alpha;

      //----------------------
      #define MAXACC 256
      int acc = constrain(VACC.y / Cfg.divAcc, -MAXACC,MAXACC) << 8;

      //------
      static int FWD = 0;
      int fwd = constrain(acc, 0, 65535);
      FWD = fwd > FWD ? fwd : lerp16by16(FWD, fwd, Cfg.smoothAcc);

      int alphaF = constrain((FWD - (Cfg.thresAcc<<8))/(MAXACC - Cfg.thresAcc), 0, 255);
      int eyeF = Cfg.minEye + (((Cfg.maxEye - Cfg.minEye) * alphaF) >>8);

      if (Cfg.ledF)
      { 
        RunF.setSpeed(runSpeed);
        RunF.setAlpha(alpha);
        CylonF.setEyeSize(eyeF);
        CylonF.setAlpha(invAlpha);
        TwinkleF.setAlpha((alphaF * (invAlpha + 1))>>8);
      }

      //------
      static int RWD = 0;
      int rwd = constrain(-acc, 0, 65535);
      RWD = rwd > RWD ? rwd : lerp16by16(RWD, rwd, Cfg.smoothAcc);

      int alphaR = constrain((RWD - (Cfg.thresAcc<<8))/(MAXACC - Cfg.thresAcc), 0, 255);
      int eyeR = Cfg.minEye + (((Cfg.maxEye - Cfg.minEye) * alphaR) >>8);

      if (Cfg.ledR)
      { 
        RunR.setSpeed(runSpeed);
        RunR.setAlpha(alpha);
        CylonR.setEyeSize(eyeR);
        CylonR.setAlpha(invAlpha);
        TwinkleR.setAlpha((alphaR * (invAlpha + 1))>>8);
      }

      //----------------------
      int alphaP = max(0, 255 - max(alphaR, alphaF));
      if (Cfg.led)
      {
        Aqua.setAlpha(alphaF);
        AquaTwk.setAlpha(alphaF);
        Fire.setAlpha(alphaR);
        FireTwk.setAlpha(alphaR);
        Plasma.setAlpha(alphaP);
      }

      #ifdef DEBUG_ACC
        Serial << "[areal  " << x << "\t" << y << "\t" << z << "]\t";
        Serial << "[fwd " << fwd << "\trwd " << rwd << "\tACC " << acc << "]\t";
        Serial << "[alpha " << alpha << "\tinv " << invAlpha << "]\t";
        Serial << "[eyeR " << eyeR << "\teyeF " << eyeF << "\talphaR " << alphaR << "\talphaF " << alphaF << "\talphaP " << alphaP << "]" << endl; //"       \r";//endl;
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
