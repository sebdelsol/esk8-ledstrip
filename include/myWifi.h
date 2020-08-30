#pragma once

#define NODEBUG_WEBSOCKETS

#include <WiFi.h>//<ESP8266WiFi.h>
#include <Streaming.h>
#include <WebSocketsClient.h>

#include <ledstrip.h>
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
  int  mBegunOn;
  bool mON = false;
  bool mWantON = false;

  bool mIsSocket = false;
  bool mWSConnected = false; 
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
  
  void onWSEvent(WStype_t type, uint8_t * payload, size_t length);
  bool isWSConnected() { return mIsSocket && mWSConnected; };
  
  void addStrip(const BaseLedStrip &leds);
  ForEachMethod(addStrip); // create method addStrips(...) that calls addStrip on all args

  bool update();
};

