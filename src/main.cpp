#include <ledstrip.h>
// #include <adxl345.h>
#include <myMpu6050.h>
#include <ESP8266WiFi.h>
#include <myWifi.h>
#include <Button.h>
#include <bluetooth.h>
#include <Streaming.h>

// ----------------------------------------------------
#define LED_MAX_MA 700//2000
#define LED_TICK 15
#define BT_TICK 100

#define CPU_FREQ 80
#define SERIAL_BAUD 115200 //9600

#define MIN_LIGHT 25
#define MAX_LIGHT 800

// #define DEBUG_LED

#define TOFRAC(a) ((a)<<15)
#define FROMFRAC(a) ((a)>>15)
#define MYLERP(from, to, s) ((from<<8) + (s<<7) + (to-from)*s) >> 8 // s is a fract of 256
#define ALPHA_MULT(a, multfrac) FROMFRAC(((a) * (TOFRAC(256)-multfrac)) >> 8)

// ----------------------------------------------------
myWifi MyWifi;
BlueTooth BT;
Button Button(BUTTON_PIN);
// myADXL345 Accel;
myMPU6050 Accel;

LedStrip Leds;
Fire Fire(true); // reverse
Aqua Aqua(false); // not reverse
Plasma Plasma;
Cylon Cylon;

// ----------------------------------------------------
void setup()
{
  system_update_cpu_freq(CPU_FREQ);
  Serial.begin(SERIAL_BAUD);
  Serial << endl;

  // !!!! BUILTIN_LED D4 !!!!!
  // pinMode(BUILTIN_LED, OUTPUT);
  // digitalWrite(BUILTIN_LED, LOW);

  Accel.begin();
  Button.begin();

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
  BT.toggle();

  // test 74AHCT125n
  // pinMode(LED_PIN, OUTPUT);
  // digitalWrite(LED_PIN, HIGH);

  delay(100);   //slow start for reducing amps inflow
}

// ----------------------------------------------------
void loop()
{
  long start = millis();

  static long alphaBT = 0, alphaBTtarget = 0;
  static bool btOn = true;

  alphaBTtarget = TOFRAC(BT.update() ? 255 : 0);
  EVERY_N_MILLISECONDS(BT_TICK) {

    btOn = BT.update();
    alphaBTtarget = TOFRAC(btOn ? 255 : 0);

    if (Button.pressed()) {
        Serial << "button pressed " << endl;
        BT.toggle();
    }
  }

  EVERY_N_MILLISECONDS(LED_TICK) {
    // int light = analogRead(ANALOG_PIN); // read analog input pin 0
    // byte bright = map(light, MIN_LIGHT, MAX_LIGHT, 255, 0); // to darker the light, the brighter the leds
    // Leds.setBrightness(bright);

    if (!btOn){
      int x, y, z, oneG;
      if (Accel.getXYZ(x, y, z, oneG)){

        // int fwd = y * max(0, -z); // forward acceleration on the horizontal plane
        int fwd = x;
        fwd = constrain(fwd * 256 / (oneG/2), -255, 255);
        Serial << FROMFRAC(alphaBT) << " " <<fwd << " .. " << x << " " << y << " " << z << " " << endl;

        Plasma.setAlpha(ALPHA_MULT(255-abs(fwd), alphaBT));          // plasma visible when fwd is ~0
        Aqua.setAlpha(ALPHA_MULT(max(fwd, 0), alphaBT)); // aqua visible when fwd is >> 0
        Fire.setAlpha(ALPHA_MULT(max(-fwd, 0), alphaBT)); // fire visible when fwd is << 0
        Cylon.setAlpha(FROMFRAC(alphaBT)); // Cylon visible only when BT on;

        alphaBT = MYLERP(alphaBT, alphaBTtarget, 10); //20/256
      }
    }
    else{
      int x, y, z, oneG;
      if (Accel.getXYZ(x, y, z, oneG)){
        // Serial << x << " " << y << " " << z << " " << endl;
      }

      Aqua.setAlpha(0);
      Fire.setAlpha(0);
      Plasma.setAlpha(0);
      Cylon.setAlpha(40);
    }


    Leds.update();
    #ifdef DEBUG_LED
      MyWifi.update();
    #endif
    Leds.show(); // to be called as much as possible for Fastled brightness dithering BUT some flickering issue with the Bluetooth....
  }

  EVERY_N_MILLISECONDS(1000)
    Leds.getInfo();

  // Leds.show(); // to be called as much as possible for Fastled brightness dithering

  // perf
  // Serial << "busy " << (millis() - start) << " / " << LED_TICK << "ms         \r";
}
