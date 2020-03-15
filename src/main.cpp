
// #define USE_OTA
// #define USE_TELNET //needs USE_OTA to work
#define USE_BT // see platformio & use "board_build.partitions = huge_app.csv"
// #define DEBUG_LED
// #define DEBG_SERIAL
// #define USE_LIGHTPROBE

// ----------------------------------------------------
// #include <config.h>
#include <ledstrip.h>
#include <myMpu6050.h>
#include <myWifi.h>
#include <Button.h>
#include <Streaming.h>
#include <soc/rtc.h> // get cpu freq

#ifdef USE_BT
  #include <bluetooth.h>
  BlueTooth BT;
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
#define   LED_MAX_MA  1500 // mA
#define   LED_TICK    7  // ms
#define   BT_TICK     15  // ms
#define   SERIAL_BAUD 115200 

// ----------------------------------------------------
myWifi        MyWifi;
Button        Button(BUTTON_PIN);
myMPU6050     Accel;
AllLedStrips  AllLeds(LED_MAX_MA, Serial);

// ----------------------------------------------------
#define     NBLEDS_MIDDLE 30
#define     NBLEDS_TIPS   36

#define     AQUA_MENTHE   CRGB(0x7FFFD4)
#define     LUSH_LAVA     CRGB(0xFF4500)
#define     HUE_AQUA_BLUE 140

LedStrip    <NBLEDS_MIDDLE, LED_PIN>  Leds("Led");
RunningFX   Fire(LUSH_LAVA, 10, 3);     
RunningFX   Aqua(AQUA_MENTHE, 10, -3);  
TwinkleFX   FireTwk(HUE_RED); 
TwinkleFX   AquaTwk(HUE_AQUA_BLUE);
PlasmaFX    Plasma;

LedStrip    <NBLEDS_TIPS, LEDR_PIN>   LedsR("LedR");
DblCylonFX  CylonR(LUSH_LAVA); 
TwinkleFX   TwinkleR(LUSH_LAVA);
RunningFX   RunR(CRGB::Gold); 

LedStrip    <NBLEDS_TIPS, LEDF_PIN>   LedsF("LedF");
DblCylonFX  CylonF(AQUA_MENTHE);
TwinkleFX   TwinkleF(HUE_AQUA_BLUE); 
RunningFX   RunF(CRGB::Gold);

// ----------------------------------------------------
class CFG : public OBJCmd
{
public:
  bool ledR = true;
  bool ledF = true;
  bool led = true;
  CFG();
};

CFG::CFG()
{
  REGISTER_CMD(CFG,  "ledR",  { self->ledR = arg0; }, self->ledR)
  REGISTER_CMD(CFG,  "ledF",  { self->ledF = arg0; }, self->ledF)
  REGISTER_CMD(CFG,  "led",   { self->led = arg0; },  self->led)
}

CFG Cfg;

// ----------------------------------------------------
void setup()
{
  Serial.begin(SERIAL_BAUD);
  Serial << endl << "-------- START --------" << endl;

  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);
  Serial << "Esp32 core " << esp_get_idf_version() << endl;
  Serial << "CPU freq " << rtc_clk_cpu_freq_get() * 80 << "MHz" << endl;

  #define Register3FX(l, f1, f2, f3)          AllLeds.registerStrip(l);   l.registerFX(f1); l.registerFX(f2); l.registerFX(f3);
  #define Register5FX(l, f1, f2, f3, f4, f5)  Register3FX(l, f1, f2, f3); l.registerFX(f4); l.registerFX(f5);
  
  Register5FX(Leds,   Fire,       FireTwk,    Aqua,   AquaTwk,    Plasma);
  Register3FX(LedsR,  TwinkleR,   CylonR,     RunR);
  Register3FX(LedsF,  TwinkleF,   CylonF,     RunF);

  #ifdef USE_OTA
    Ota.begin();
  #else
    #ifdef DEBUG_LED
      MyWifi.on();
      MyWifi.addLeds(Leds);
    #else
      MyWifi.off();
    #endif
  #endif

  #ifdef USE_BT
    #define BT_REGISTER_OBJ(o) BT.registerObj(o, #o);
    #define BT_REGISTER_3OBJ(o1, o2, o3) BT_REGISTER_OBJ(o1); BT_REGISTER_OBJ(o2); BT_REGISTER_OBJ(o3);
    #define BT_REGISTER_5OBJ(o1, o2, o3, o4, o5) BT_REGISTER_3OBJ(o1, o2, o3); BT_REGISTER_OBJ(o4); BT_REGISTER_OBJ(o5);
    
    BT.init(true);
    BT_REGISTER_5OBJ(Fire,      FireTwk,    Aqua,   AquaTwk,    Plasma);
    BT_REGISTER_3OBJ(TwinkleR,  CylonR,     RunR);
    BT_REGISTER_3OBJ(TwinkleF,  CylonF,     RunF);
    BT_REGISTER_OBJ(Cfg);
  #else    
    // switch off blue led
    pinMode(LIGHT_PIN, OUTPUT);
    digitalWrite(LIGHT_PIN, LOW);
    btStop(); // turnoff bt 
  #endif

  Button.begin();
  Accel.begin(Serial, &handleOta);
}

// ----------------------------------------------------
void loop()
{
  handleOta();

  int wz, x, y, z, oneG;
  float *ypr;
  bool gotAccel = Accel.getXYZ(&ypr, wz, x, y, z, oneG);

  #ifdef USE_BT

    EVERY_N_MILLISECONDS(BT_TICK) {

      if (Button.pressed()) {
          Serial << "button pressed " << endl;
          // digitalWrite(LIGHT_PIN, !digitalRead(LIGHT_PIN));
          BT.toggle();
      }

      if (BT.update()) {
        if (gotAccel) {
          // int rx = int(ypr[0]*180/M_PI), ry = int(ypr[1]* 180/M_PI), rz = int(ypr[2]* 180/M_PI);
          // *(BT.getBtSerial()) << "ANG A " << rx << " " << ry << " " << rz << endl;
        }
      } 
    }
  #endif

  EVERY_N_MILLISECONDS(LED_TICK) {

    #ifdef USE_LIGHTPROBE
      #define MIN_LIGHT 400
      #define MAX_LIGHT 4095
      int light = analogRead(LDR_PIN);
      byte bright = map(light, MIN_LIGHT, MAX_LIGHT, 255, 0); // to darker the light, the brighter the leds
      Serial << light << " " << bright << endl;
      AllLeds.setBrightness(bright);
    #endif

    if (gotAccel) {

      int runSpeed =  ((wz>0) - (wz<0)) * 3;

      //------
      #define NeutralZ  3000 
      #define maxZ      7000 
      wz = abs(wz);
      int alpha = wz > NeutralZ ? min((wz-NeutralZ) * 255 / maxZ, 255) : 0;
      int invAlpha = 255 - alpha;

      //----------------------
      #define SMOOTH_ACC  1600//3200 //.05
      #define THRES_ACC   30
      #define MAX_ACC     256
      #define MINeye      5
      #define MAXeye      10

      int acc = constrain(y / 2, -MAX_ACC, MAX_ACC) << 8;

      //------
      static int FWD = 0;
      int fwd = constrain(acc, 0, 65535);
      FWD = fwd > FWD ? fwd : lerp16by16(FWD, fwd, SMOOTH_ACC);

      int alphaF = constrain((FWD - (THRES_ACC<<8))/(MAX_ACC-THRES_ACC), 0, 255);
      int eyeF = MINeye + (((MAXeye-MINeye) * alphaF) >>8);

      if (Cfg.ledF) { 
        RunF.setSpeed(runSpeed);
        RunF.setAlpha(alpha);
        CylonF.setEyeSize(eyeF);
        CylonF.setAlpha(invAlpha);
        TwinkleF.setAlpha((alphaF * (invAlpha + 1))>>8);
      }

      //------
      static int RWD = 0;
      int rwd = constrain(-acc, 0, 65535);
      RWD = rwd > RWD ? rwd : lerp16by16(RWD, rwd, SMOOTH_ACC);

      int alphaR = constrain((RWD - (THRES_ACC<<8))/(MAX_ACC-THRES_ACC), 0, 255);
      int eyeR = MINeye + (((MAXeye-MINeye) * alphaR) >>8);

      if (Cfg.ledR) { 
        RunR.setSpeed(runSpeed);
        RunR.setAlpha(alpha);
        CylonR.setEyeSize(eyeR);
        CylonR.setAlpha(invAlpha);
        TwinkleR.setAlpha((alphaR * (invAlpha + 1))>>8);
      }

      //----------------------
      int alphaP = max(0, 255 - max(alphaR, alphaF));
      if (Cfg.led) {
        Aqua.setAlpha(alphaF);
        AquaTwk.setAlpha(alphaF);
        Fire.setAlpha(alphaR);
        FireTwk.setAlpha(alphaR);
        Plasma.setAlpha(alphaP);
      }

      #ifdef DEBG_SERIAL
        Serial << "[areal  " << x << "\t" << y << "\t" << z << "]\t";
        Serial << "[fwd " << fwd << "\trwd " << rwd << "\tACC " << acc << "]\t";
        Serial << "[alpha " << alpha << "\tinv " << invAlpha << "]\t";
        Serial << "[eyeR " << eyeR << "\teyeF " << eyeF << "\talphaR " << alphaR << "\talphaF " << alphaF << "\talphaP " << alphaP << "]" << endl; //"       \r";//endl;
      #endif
    }

    AllLeds.update();

    #ifdef DEBUG_LED
      MyWifi.update();
    #endif
  }

  #ifdef DEBG_SERIAL
    EVERY_N_MILLISECONDS(1000)
      AllLeds.getInfo();
  #endif

  AllLeds.show(); // to be called as much as possible for Fastled brightness dithering
}
