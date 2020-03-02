
#define USE_OTA
// #define USE_TELNET //needs USE_OTA to work
// #define USE_BT // see platformio & use "board_build.partitions = huge_app.csv"
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

#ifdef USE_OTA
  #include <OTA.h>
  OTA Ota;

  #ifdef USE_TELNET
    #include <TelnetSpy.h>
    TelnetSpy SerialAndTelnet;
    #define Serial  SerialAndTelnet
  #endif
#endif

// ----------------------------------------------------
#define LED_MAX_MA 800
#define LED_TICK 15
#define BT_TICK 15
#define SERIAL_BAUD 115200 

// ----------------------------------------------------
myWifi MyWifi;
Button Button(BUTTON_PIN);
myMPU6050 Accel;
AllLedStrips AllLeds(LED_MAX_MA, Serial);

// ----------------------------------------------------
#define NBLEDS_MIDDLE 30
#define NBLEDS_TIP 36

#define AQUA_MENTHE   0x7FFFD4
#define LUSH_LAVA     0xFF4500
#define PHANTOM_BLUE  0x191970
#define YELLOW        0xffff00

LedStrip<NBLEDS_MIDDLE, LED_PIN> Leds("Led");
FireFX Fire(true, 75, 120); 
TwinkleFX Fire2(0);
AquaFX Aqua(false, 75, 120); // not reverse
TwinkleFX Aqua2(CRGB(AQUA_MENTHE));
PlasmaFX Plasma;

LedStrip<NBLEDS_TIP, LEDR_PIN> LedsR("LedR");
TwinkleFX TwinkleR(CRGB(LUSH_LAVA)); //15 //orange
CylonFX CylonR1(LUSH_LAVA, 7, 3<<8); 
CylonFX CylonR2(LUSH_LAVA, 7, -3<<8);
RunningFX RunR(YELLOW, 5, 2); //RunningFX(color,width,speed);

LedStrip<NBLEDS_TIP, LEDF_PIN> LedsF("LedF");
TwinkleFX TwinkleF(140); // aqua
CylonFX CylonF1(AQUA_MENTHE,  3, 3<<8);
CylonFX CylonF2(AQUA_MENTHE,  3, -3<<8);
RunningFX RunF(YELLOW, 5, 2); //RunningFX(color,width,speed);

// ----------------------------------------------------
// typedef struct {
//   int toto = 1;
//   byte tutu = 3;
//   int titi = 201;
// } TstCfg;

// AllConfig AllCFG;
// Config<TstCfg, 1> test("test");

// ----------------------------------------------------
void setup()
{
  Serial.begin(SERIAL_BAUD);
  Serial << endl << "-------- START --------" << endl;

  // rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);
  Serial << "Esp32 core " << esp_get_idf_version() << endl;
  Serial << "CPU freq " << rtc_clk_cpu_freq_get() * 80 << "MHz" << endl;

  // AllCFG.init();
  // AllCFG.RegisterCfg(test);
  // AllCFG.load();
  // // Serial << test.mData.toto << " " << test.mData.tutu << " " << test.mData.titi << endl;
  // // test.mData.toto = test.mData.toto+1;
  // AllCFG.save();
  // AllCFG.cleanUnRegistered();

  AllLeds.registerStrip(Leds);
  Leds.registerFX(Fire);
  Leds.registerFX(Fire2);
  Leds.registerFX(Aqua);
  Leds.registerFX(Aqua2);
  Leds.registerFX(Plasma);

  AllLeds.registerStrip(LedsR);
  LedsR.registerFX(TwinkleR);
  LedsR.registerFX(CylonR1);
  LedsR.registerFX(CylonR2);
  LedsR.registerFX(RunR);

  AllLeds.registerStrip(LedsF);
  LedsF.registerFX(TwinkleF);
  LedsF.registerFX(CylonF1);
  LedsF.registerFX(CylonF2);
  LedsF.registerFX(RunF);

  // switch off blue led
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, LOW);

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
    BT.init();
    BT.registerFX(Fire, 'F');
    BT.registerFX(Aqua, 'A');
    BT.registerFX(Plasma, 'P');
    BT.registerFX(Cylon, 'C');
  #else    
    btStop(); // turnoff bt 
  #endif

  Button.begin();
  Accel.begin(Serial);
}

// ----------------------------------------------------
void loop()
{
  #ifdef USE_OTA
    Ota.update();
    #ifdef USE_TELNET
      SerialAndTelnet.handle();
    #endif
  #endif

  int x, y, z, oneG;
  float *ypr, wz;
  bool gotAccel = Accel.getXYZ(&ypr, wz, x, y, z, oneG);

  #ifdef USE_BT
    static bool btOn = BT.update();

    EVERY_N_MILLISECONDS(BT_TICK) {

      if (Button.pressed()) {
          Serial << "button pressed " << endl;
          // digitalWrite(LIGHT_PIN, !digitalRead(LIGHT_PIN));
          BT.toggle();
      }

      btOn = BT.update();
    }
  #endif

  EVERY_N_MILLISECONDS(LED_TICK) {

    #ifdef USE_LIGHTPROBE
      #define MIN_LIGHT 400// 25
      #define MAX_LIGHT 4095
      int light = analogRead(LDR_PIN);
      byte bright = map(light, MIN_LIGHT, MAX_LIGHT, 255, 0); // to darker the light, the brighter the leds
      Serial << light << " " << bright << endl;
      AllLeds.setBrightness(bright);
    #endif

    if (gotAccel){

      #ifdef USE_BT
        if (btOn) {
          EVERY_N_MILLISECONDS(50) {
            int rx = int(ypr[0]*180/M_PI), ry = int(ypr[1]* 180/M_PI), rz = int(ypr[2]* 180/M_PI);
            *(BT.getBtSerial()) << "ANG A " << rx << " " << ry << " " << rz << endl;
          } 
        } 
        else {
      #endif
          #define SMOOTH_ACC 3200 // 6500 //.05
          #define THRES_ACC 20
          #define MAX_ACC 255

          #define NeutralZ .025
          #define maxZ .2
          int runSpeed = wz > 0 ? 3 : -3;
          wz = wz >0 ? wz : -wz;
          int runAlpha = wz > NeutralZ ? min(int((wz-NeutralZ) * 255 / maxZ), 255) : 0;
          int invAlpha = 255 - runAlpha;

          int acc = constrain(y /2, -MAX_ACC, MAX_ACC) << 8;

          static int FWD = 0;
          int fwd = max(acc, 0);
          FWD = fwd > FWD ? fwd : lerp16by16(FWD, fwd, SMOOTH_ACC);
          fwd = FWD >> 8;

          static int RWD = 0;
          int rwd = max(-acc, 0);
          RWD = rwd > RWD ? rwd : lerp16by16(RWD, rwd, SMOOTH_ACC);
          rwd = RWD >> 8;

          int eyeR = map(rwd, THRES_ACC, MAX_ACC, 2, 10);
          int alphaR = max(0, int(map(rwd, THRES_ACC, MAX_ACC, 0, 255)));
          CylonR1.setEyeSize(eyeR);
          CylonR2.setEyeSize(eyeR);
          CylonR1.setAlpha(invAlpha);
          CylonR2.setAlpha(invAlpha);
          TwinkleR.setAlpha((alphaR * invAlpha)>>8);
          RunR.setAlpha(runAlpha);
          RunR.setSpeed(runSpeed);

          int eyeF = map(fwd, THRES_ACC, MAX_ACC, 2, 10);
          int alphaF = max(0, int(map(fwd, THRES_ACC, MAX_ACC, 0, 255)));
          CylonF1.setEyeSize(eyeF);
          CylonF2.setEyeSize(eyeF);
          CylonF1.setAlpha(invAlpha);
          CylonF2.setAlpha(invAlpha);
          TwinkleF.setAlpha((alphaF * invAlpha)>>8);
          RunF.setAlpha(runAlpha);
          RunF.setSpeed(runSpeed);

          Aqua.setAlpha(alphaF);
          Aqua2.setAlpha(alphaF);
          Fire.setAlpha(alphaR);
          Fire2.setAlpha(alphaR);
          int alphaP = max(0, 255 - max(alphaR, alphaF));
          Plasma.setAlpha(alphaP);

          #ifdef DEBG_SERIAL
            Serial << "[areal  " << x << "\t" << y << "\t" << z << "]\t";
            Serial << "[fwd " << fwd << "\trwd " << rwd << "]\t"; //"\t ACC " << acc << "]\t";
            Serial << "[eyeR " << eyeR << "\teyeF " << eyeF << "\talphaR " << alphaR << "\talphaF " << alphaF << "\talphaP " << alphaP << "]" << endl; //"       \r";//endl;
          #endif

      #ifdef USE_BT
        }
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
