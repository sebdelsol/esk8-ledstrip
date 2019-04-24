#include <ledstrip.h>
#include <myMpu6050.h>
#include <WiFi.h>
#include <myWifi.h>
#include <Button.h>
#include <bluetooth.h>
#include <Streaming.h>

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

LedStrip Leds(30);
Fire Fire(true); // reverse
Aqua Aqua(false); // not reverse
Plasma Plasma;
Cylon Cylon;

// ----------------------------------------------------
void setup()
{
  //system_update_cpu_freq(CPU_FREQ);
  Serial.begin(SERIAL_BAUD);
  Serial << "-------- START --------" << endl;

  //Serial << esp_get_idf_version() << endl;

  // pinMode(BUILTIN_LED, OUTPUT);
  // digitalWrite(BUILTIN_LED, LOW);
  // Serial << "led " << BUILTIN_LED << endl;

  Leds.init(LED_MAX_MA);
  Leds.registerFX(Fire);
  Leds.registerFX(Aqua);
  Leds.registerFX(Plasma);
  Leds.registerFX(Cylon);

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

  static long alphaBT = 255, alphaBTtarget = 0;
  static bool btOn = BT.update();

  int x, y, z, oneG;
  float *ypr;
  bool gotAccel = Accel.getXYZ(&ypr, x, y, z, oneG);
  if (gotAccel){
    int rx = int(ypr[0]*180/M_PI), ry = int(ypr[1]* 180/M_PI), rz = int(ypr[2]* 180/M_PI);
    // Serial << rx << " " << ry << " " << rz << " " << endl;
  }

  EVERY_N_MILLISECONDS(BT_TICK) {

    if (Button.pressed()) {
        Serial << "button pressed " << endl;
        BT.toggle();
    }

    btOn = BT.update();
    alphaBTtarget = TOFRAC(btOn ? 255 : 0);
  }

  EVERY_N_MILLISECONDS(LED_TICK) {
    int light = analogRead(ANALOG_PIN); // read analog input pin 0
    byte bright = map(light, MIN_LIGHT, MAX_LIGHT, 255, 0); // to darker the light, the brighter the leds
    // Serial << light << " " << bright << endl;
    // Leds.setBrightness(bright);

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

    Leds.update();
    #ifdef DEBUG_LED
      MyWifi.update();
    #endif
  }

  // EVERY_N_MILLISECONDS(1000)
  //   Leds.getInfo();

  Leds.show(); // to be called as much as possible for Fastled brightness dithering

  // perf
  // Serial << "busy " << (millis() - start) << " / " << LED_TICK << "ms         \n";
}
