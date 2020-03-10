
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
#define LED_MAX_MA  800 // mA
#define LED_TICK    15  // ms
#define BT_TICK     15  // ms
#define SERIAL_BAUD 115200 

// ----------------------------------------------------
myWifi MyWifi;
Button Button(BUTTON_PIN);
myMPU6050 Accel;
AllLedStrips AllLeds(LED_MAX_MA, Serial);

// ----------------------------------------------------
#define NBLEDS_MIDDLE 30
#define NBLEDS_TIP    36

#define AQUA_MENTHE   0x7FFFD4
#define AQUA          0x00FFFF
#define LUSH_LAVA     0xFF4500
#define PHANTOM_BLUE  0x191970
#define YELLOW        0xffff00

LedStrip<NBLEDS_MIDDLE, LED_PIN> Leds("Led");
RunningFX Fire(LUSH_LAVA, 10, 3); //width, speed // FireFX Fire(true); //reverse
RunningFX Aqua(AQUA_MENTHE, 10, -3); // AquaFX Aqua(false); 
TwinkleFX FireTwk(0); // red
TwinkleFX AquaTwk(140);//CRGB(AQUA_MENTHE));
PlasmaFX Plasma;

LedStrip<NBLEDS_TIP, LEDR_PIN> LedsR("LedR");
TwinkleFX TwinkleR(CRGB(LUSH_LAVA));
DblCylonFX CylonR(LUSH_LAVA); 
RunningFX RunR(YELLOW, 5); //width

LedStrip<NBLEDS_TIP, LEDF_PIN> LedsF("LedF");
TwinkleFX TwinkleF(140); // aqua
DblCylonFX CylonF(AQUA_MENTHE);
RunningFX RunF(YELLOW, 5); // width

// ----------------------------------------------------
void setup()
{
  Serial.begin(SERIAL_BAUD);
  Serial << endl << "-------- START --------" << endl;

  // rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);
  Serial << "Esp32 core " << esp_get_idf_version() << endl;
  Serial << "CPU freq " << rtc_clk_cpu_freq_get() * 80 << "MHz" << endl;

  AllLeds.registerStrip(Leds);
  Leds.registerFX(Fire);
  Leds.registerFX(FireTwk);
  Leds.registerFX(Aqua);
  Leds.registerFX(AquaTwk);
  Leds.registerFX(Plasma);

  AllLeds.registerStrip(LedsR);
  LedsR.registerFX(TwinkleR);
  LedsR.registerFX(CylonR);
  LedsR.registerFX(RunR);

  AllLeds.registerStrip(LedsF);
  LedsF.registerFX(TwinkleF);
  LedsF.registerFX(CylonF);
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
      #define MIN_LIGHT 400
      #define MAX_LIGHT 4095
      int light = analogRead(LDR_PIN);
      byte bright = map(light, MIN_LIGHT, MAX_LIGHT, 255, 0); // to darker the light, the brighter the leds
      Serial << light << " " << bright << endl;
      AllLeds.setBrightness(bright);
    #endif

    if (gotAccel) {

      #ifdef USE_BT
        if (btOn) {
          EVERY_N_MILLISECONDS(50) {
            int rx = int(ypr[0]*180/M_PI), ry = int(ypr[1]* 180/M_PI), rz = int(ypr[2]* 180/M_PI);
            *(BT.getBtSerial()) << "ANG A " << rx << " " << ry << " " << rz << endl;
          } 
        } 
        else {
      #endif
          //----------------------
          int runSpeed =  ((wz>0) - (wz<0)) * 3;
          RunR.setSpeed(runSpeed);
          RunF.setSpeed(runSpeed);

          //------
          #define NeutralZ  3000 
          #define maxZ      7000 
          wz = abs(wz);
          int alpha = wz > NeutralZ ? min((wz-NeutralZ) * 255 / maxZ, 255) : 0;
          RunR.setAlpha(alpha);
          RunF.setAlpha(alpha);

          int invAlpha = 255 - alpha;

          //----------------------
          #define SMOOTH_ACC  1600//3200 //.05
          #define THRES_ACC   30
          #define MAX_ACC     256
          #define MINeye      5
          #define MAXeye      7

          int acc = constrain(y / 2, -MAX_ACC, MAX_ACC) << 8;

          //------
          static int FWD = 0;
          int fwd = constrain(acc, 0, 65535);
          FWD = fwd > FWD ? fwd : lerp16by16(FWD, fwd, SMOOTH_ACC);

          int alphaF = constrain((FWD - (THRES_ACC<<8))/(MAX_ACC-THRES_ACC), 0, 255);
          int eyeF = MINeye + (((MAXeye-MINeye) * alphaF) >>8);

          CylonF.setEyeSize(eyeF);
          CylonF.setAlpha(invAlpha);
          TwinkleF.setAlpha((alphaF * (invAlpha + 1))>>8);

          //------
          static int RWD = 0;
          int rwd = constrain(-acc, 0, 65535);
          RWD = rwd > RWD ? rwd : lerp16by16(RWD, rwd, SMOOTH_ACC);

          int alphaR = constrain((RWD - (THRES_ACC<<8))/(MAX_ACC-THRES_ACC), 0, 255);
          int eyeR = MINeye + (((MAXeye-MINeye) * alphaR) >>8);

          CylonR.setEyeSize(eyeR);
          CylonR.setAlpha(invAlpha);
          TwinkleR.setAlpha((alphaR * (invAlpha + 1))>>8);

          //----------------------
          Aqua.setAlpha(alphaF);
          AquaTwk.setAlpha(alphaF);
          Fire.setAlpha(alphaR);
          FireTwk.setAlpha(alphaR);
          
          //------
          int alphaP = max(0, 255 - max(alphaR, alphaF));
          Plasma.setAlpha(alphaP);

          #ifdef DEBG_SERIAL
            Serial << "[areal  " << x << "\t" << y << "\t" << z << "]\t";
            Serial << "[fwd " << fwd << "\trwd " << rwd << "\tACC " << acc << "]\t";
            Serial << "[alpha " << alpha << "\tinv " << invAlpha << "]\t";
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
