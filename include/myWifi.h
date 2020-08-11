#pragma once

#define NODEBUG_WEBSOCKETS

#include <WiFi.h>//<ESP8266WiFi.h>
#include <Streaming.h>
#include <Ledstrip.h>
#include <WebSocketsClient.h>

#include <wificonfig.h>  
// wificonfig.h needs to define :
// #define WIFINAME "******"
// #define WIFIPASS "****"
// #define SOCK_ADDR "**.**.**.**"
// #define SOCK_PORT **

#define MAXSTRIPS 3
#define WIFI_TIMEOUT 10000

class myWifi
{
  int mBegunOn;
  bool mON = false;
  bool mWantON = false;

  bool mIsSocket = false;
  WebSocketsClient webSocket;

  BaseLedStrip* mLeds[MAXSTRIPS];
  byte mNStrips = 0;

  #define INFO_LEN 15
  char mInfo[INFO_LEN];

  Stream& mSerial;

public:
  myWifi(Stream &serial) : mSerial(serial) {};
  void start();
  void stop();
  void toggle() { mON ? stop() : start(); };
  
  void addStrip(const BaseLedStrip &leds);
  _MAP(addStrips, addStrip); // create method addStrips(...) that calls addStrip on all args

  bool update();
};

