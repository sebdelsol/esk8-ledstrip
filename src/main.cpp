#include <ledstrip.h>
#include <myMpu6050.h>
#include <myWifi.h>
#include <Button.h>
#include <bluetooth.h>
#include <config.h>

#include <Streaming.h>
#include <soc/rtc.h> // get cpu freq

// ----------------------------------------------------
#define LED_MAX_MA 700//2000
#define LED_TICK 15
#define BT_TICK 15

#define SERIAL_BAUD 115200 //9600

#define MIN_LIGHT 400// 25
#define MAX_LIGHT 4095

// #define DEBUG_LED

#define TOFRAC(a) ((a)<<15)
#define FROMFRAC(a) ((a)>>15)
#define MYLERP(from, to, s) ((from<<8) + (s<<7) + (to-from)*s) >> 8 // s is a fract of 256
#define ALPHA_MULT(a, multfrac) FROMFRAC(((a) * (TOFRAC(256)-multfrac)) >> 8)

// ----------------------------------------------------
myWifi MyWifi;
BlueTooth BT;
Button Button(BUTTON_PIN);
myMPU6050 Accel;

AllLedStrips AllLeds(LED_MAX_MA);

LedStrip<30, LED_PIN> Leds("Led");
FireFX Fire(true); // reverse
AquaFX Aqua(false); // not reverse
PlasmaFX Plasma;
CylonFX Cylon;

LedStrip<30, LEDR_PIN> LedsR("LedR");
PulseFX PulseR;
// CylonFX CylonR1(0xff,0,0, 10, 4<<8);
// CylonFX CylonR2(0xff,0,0, 10, (-4)<<8);

LedStrip<15, LEDF_PIN> LedsF("LedF");
CylonFX CylonF;

typedef struct {
  int toto = 1;
  byte tutu = 3;
  int titi = 201;
} TstCfg;

AllConfig AllCFG;
Config<TstCfg, 1> test("test");

// ----------------------------------------------------
void setup()
{
  Serial.begin(SERIAL_BAUD);
  Serial << endl << "-------- START --------" << endl;

  // rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);
  Serial << "CPU freq " << rtc_clk_cpu_freq_get() * 80 << "MHz" << endl;
  Serial << "Esp32 core " << esp_get_idf_version() << endl;

  AllCFG.init();
  AllCFG.RegisterCfg(test);
  AllCFG.load();
  // Serial << test.mData.toto << " " << test.mData.tutu << " " << test.mData.titi << endl;
  // test.mData.toto = test.mData.toto+1;
  AllCFG.save();
  AllCFG.cleanUnRegistered();

  // pinMode(BUILTIN_LED, OUTPUT);
  // digitalWrite(BUILTIN_LED, LOW);
  // Serial << "led " << BUILTIN_LED << endl;

  PulseR.setAlpha(255);
  // CylonR1.setAlpha(255);
  // CylonR2.setAlpha(255);
  CylonF.setAlpha(255);

  AllLeds.registerStrip(Leds);
  Leds.registerFX(Fire);
  Leds.registerFX(Aqua);
  Leds.registerFX(Plasma);
  Leds.registerFX(Cylon);

  AllLeds.registerStrip(LedsR);
  LedsR.registerFX(PulseR);
  // LedsR.registerFX(CylonR1);
  // LedsR.registerFX(CylonR2);

  AllLeds.registerStrip(LedsF);
  LedsF.registerFX(CylonF);

  #ifdef DEBUG_LED
    MyWifi.on();
    MyWifi.addLeds(Leds);
  #else
    MyWifi.off();
  #endif

  BT.init();
  BT.registerFX(Fire, 'F');
  BT.registerFX(Aqua, 'A');
  BT.registerFX(Plasma, 'P');
  BT.registerFX(Cylon, 'C');

  // test 74AHCT125n
  // pinMode(LED_PIN, OUTPUT);
  // digitalWrite(LED_PIN, HIGH);

  Button.begin();
  Accel.begin();
}

// ----------------------------------------------------
void loop()
{
  long start = millis();

  int x, y, z, oneG;
  float *ypr;
  bool gotAccel = Accel.getXYZ(&ypr, x, y, z, oneG);
  if (gotAccel){
    int rx = int(ypr[0]*180/M_PI), ry = int(ypr[1]* 180/M_PI), rz = int(ypr[2]* 180/M_PI);
    // Serial << rx << " " << ry << " " << rz << " " << endl;
  }

  static long alphaBT = 255, alphaBTtarget = 0;
  static bool btOn = BT.update();

  EVERY_N_MILLISECONDS(BT_TICK) {

    if (Button.pressed()) {
        Serial << "button pressed " << endl;
        BT.toggle();
    }

    btOn = BT.update();
    alphaBTtarget = TOFRAC(btOn ? 255 : 0);
  }

  EVERY_N_MILLISECONDS(LED_TICK) {
    int light = analogRead(LDR_PIN); // read analog input pin 0
    byte bright = map(light, MIN_LIGHT, MAX_LIGHT, 255, 0); // to darker the light, the brighter the leds
    // Serial << light << " " << bright << endl;
    // AllLeds.setBrightness(bright);

    alphaBT = MYLERP(alphaBT, alphaBTtarget, 10); //20/256
    // Serial << alphaBT << " => " << alphaBTtarget << endl;

    if (!btOn){
      if (gotAccel){

        int fwd = constrain(x * 256 / (oneG/2), -255, 255);
        // Serial << FROMFRAC(alphaBT) << " " <<fwd << " .. " << x << " " << y << " " << z << " " << endl;

        Plasma.setAlpha(ALPHA_MULT(255-abs(fwd), alphaBT));          // plasma visible when fwd is ~0
        Aqua.setAlpha(ALPHA_MULT(max(fwd, 0), alphaBT)); // aqua visible when fwd is >> 0
        Fire.setAlpha(ALPHA_MULT(max(-fwd, 0), alphaBT)); // fire visible when fwd is << 0
        // Cylon.setAlpha(FROMFRAC(alphaBT)); // Cylon visible only when BT on;
      }
    }
    else{
      if (gotAccel){
        EVERY_N_MILLISECONDS(50) {
          // int rx = int(ypr[0]*180/M_PI), ry = int(ypr[1]* 180/M_PI), rz = int(ypr[2]* 180/M_PI);
          // *(BT.getBtSerial()) << "ANG A " << rx << " " << ry << " " << rz << endl;
          // Serial << rx << " " << ry << " " << rz << " " << endl;
        }
      }
    }

    AllLeds.update();
    #ifdef DEBUG_LED
      MyWifi.update();
    #endif
  }

  EVERY_N_MILLISECONDS(1000)
    AllLeds.getInfo();

  AllLeds.show(); // to be called as much as possible for Fastled brightness dithering

  // perf
  // Serial << "busy " << (millis() - start) << " / " << LED_TICK << "ms         \n";
}
