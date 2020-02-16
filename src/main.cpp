#include <ledstrip.h>
#include <myMpu6050.h>
#include <myWifi.h>
// #include <Button.h>
// #include <bluetooth.h>
// #include <config.h>

#include <Streaming.h>
#include <soc/rtc.h> // get cpu freq

// ----------------------------------------------------
#define LED_MAX_MA 800//2000
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
// BlueTooth BT;
// Button Button(BUTTON_PIN);
myMPU6050 Accel;

AllLedStrips AllLeds(LED_MAX_MA);

LedStrip<30, LED_PIN> Leds("Led");
FireFX Fire(false, 75, 120); // (const bool reverse = false, const byte cooling = 75, const byte sparkling = 120);
TwinkleFX Twinkle;
// AquaFX Aqua(false, 40, 100); // not reverse
// PlasmaFX Plasma;

LedStrip<36, LEDR_PIN> LedsR("LedR");
TwinkleFX TwinkleR(15);
CylonFX CylonR1(0xff,0x10,0, 7, 4<<8);
CylonFX CylonR2(0xff,0x10,0, 7, -4<<8);

LedStrip<36, LEDF_PIN> LedsF("LedF");
TwinkleFX TwinkleF(140);
CylonFX CylonF1(0xc0,0xc0,0xff,  3, 4<<8);
CylonFX CylonF2(0xc0,0xc0,0xff,  3, -4<<8);

// typedef struct {
//   int toto = 1;
//   byte tutu = 3;
//   int titi = 201;
// } TstCfg;

// AllConfig AllCFG;
// Config<TstCfg, 1> test("test");

// ----------------------------------------------------

// #define USE_OTA
#ifdef USE_OTA

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>  

void OTAbegin() {
  MyWifi.on();

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
#endif

// ----------------------------------------------------
void setup()
{
  Serial.begin(SERIAL_BAUD);
  Serial << endl << "-------- START --------" << endl;

  // rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);
  Serial << "CPU freq " << rtc_clk_cpu_freq_get() * 80 << "MHz" << endl;
  Serial << "Esp32 core " << esp_get_idf_version() << endl;

  // AllCFG.init();
  // AllCFG.RegisterCfg(test);
  // AllCFG.load();
  // // Serial << test.mData.toto << " " << test.mData.tutu << " " << test.mData.titi << endl;
  // // test.mData.toto = test.mData.toto+1;
  // AllCFG.save();
  // AllCFG.cleanUnRegistered();

  AllLeds.registerStrip(Leds);
  Leds.registerFX(Fire);
  // Leds.registerFX(Aqua);
  Leds.registerFX(Twinkle);
  // Leds.registerFX(Plasma);

  AllLeds.registerStrip(LedsR);
  LedsR.registerFX(TwinkleR);
  TwinkleR.setAlpha(10);
  LedsR.registerFX(CylonR1);
  LedsR.registerFX(CylonR2);

  AllLeds.registerStrip(LedsF);
  LedsF.registerFX(TwinkleF);
  LedsF.registerFX(CylonF1);
  LedsF.registerFX(CylonF2);

  #ifdef USE_OTA
    OTAbegin();
  #else
    #ifdef DEBUG_LED
      MyWifi.on();
      MyWifi.addLeds(Leds);
    #else
      MyWifi.off();
      btStop(); // turnoff bt too
    #endif
  #endif
  // BT.init();
  // BT.registerFX(Fire, 'F');
  // BT.registerFX(Aqua, 'A');
  // BT.registerFX(Plasma, 'P');
  // BT.registerFX(Cylon, 'C');

  // Button.begin();
  Accel.begin();
}

// ----------------------------------------------------
void loop()
{
  #ifdef USE_OTA
    ArduinoOTA.handle();
  #endif

  long start = millis();

  int x, y, z, oneG;
  float *ypr;
  bool gotAccel = Accel.getXYZ(&ypr, x, y, z, oneG);

  // static long alphaBT = 255, alphaBTtarget = 0;
  // static bool btOn = BT.update();

  // EVERY_N_MILLISECONDS(BT_TICK) {

  //   if (Button.pressed()) {
  //       Serial << "button pressed " << endl;
  //       BT.toggle();
  //   }

  //   btOn = BT.update();
  //   alphaBTtarget = TOFRAC(btOn ? 255 : 0);
  // }

  EVERY_N_MILLISECONDS(LED_TICK) {
    // int light = analogRead(LDR_PIN);
    // byte bright = map(light, MIN_LIGHT, MAX_LIGHT, 255, 0); // to darker the light, the brighter the leds
    // Serial << light << " " << bright << endl;
    // AllLeds.setBrightness(bright);

    // alphaBT = MYLERP(alphaBT, alphaBTtarget, 10); //20/256
    // Serial << alphaBT << " => " << alphaBTtarget << endl;
    // int rx = int(ypr[0]*180/M_PI), ry = int(ypr[1]* 180/M_PI), rz = int(ypr[2]* 180/M_PI);
    // Serial << rx << " " << ry << " " << rz << " " << endl;

    // if (!btOn){
      if (gotAccel){

        int fwd = constrain(-y * 256 / (oneG/8), -255, 255);
        // Serial << eyesize << "   " << fwd << " .. " << x << " " << y << " " << z << " " << endl;

        int eyesizeR = map(max(-fwd, 0), 0, 256, 5, 10);
        int twinkR = map(max(-fwd, 0), 0, 256, 5, 128);
        CylonR1.setEyeSize(eyesizeR);
        CylonR2.setEyeSize(eyesizeR);
        TwinkleR.setAlpha(twinkR);

        int eyesizeF = map(max(fwd, 0), 0, 256, 2, 7);
        int twinkF = map(max(fwd, 0), 0, 256, 5, 128);
        CylonF1.setEyeSize(eyesizeF);
        CylonF2.setEyeSize(eyesizeF);
        TwinkleF.setAlpha(twinkF);

        // PulseR.setAlpha(max(-fwd, 0)); // fire visible when fwd is << 0
        // Plasma.setAlpha(ALPHA_MULT(255-abs(fwd), alphaBT));          // plasma visible when fwd is ~0
        // Aqua.setAlpha(ALPHA_MULT(max(fwd, 0), alphaBT)); // aqua visible when fwd is >> 0
        // Fire.setAlpha(ALPHA_MULT(max(-fwd, 0), alphaBT)); // fire visible when fwd is << 0
        // Cylon.setAlpha(FROMFRAC(alphaBT)); // Cylon visible only when BT on;
      }
    // }
    // else{
    //   if (gotAccel){
    //     EVERY_N_MILLISECONDS(50) {
    //       // int rx = int(ypr[0]*180/M_PI), ry = int(ypr[1]* 180/M_PI), rz = int(ypr[2]* 180/M_PI);
    //       // *(BT.getBtSerial()) << "ANG A " << rx << " " << ry << " " << rz << endl;
    //     }
    //   }
    // }

    AllLeds.update();

    #ifdef DEBUG_LED
      MyWifi.update();
    #endif
  }

  EVERY_N_MILLISECONDS(1000)
    AllLeds.getInfo();

  AllLeds.show(); // to be called as much as possible for Fastled brightness dithering

}
