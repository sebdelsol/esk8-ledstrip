#pragma once

#define NODEBUG_WEBSOCKETS

#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <log.h>
#include <ledstrip.h>
#include <wificonfig.h>  
// wificonfig.h needs to define :
// #define WIFINAME "******"
// #define WIFIPASS "****"
// #define SOCK_ADDR "**.**.**.**"
// #define SOCK_PORT **

#define MAXSTRIPS 3
#define WIFI_TIMEOUT 10000

using namespace websockets;

class myWifi
{
  int  mBegunOn;
  bool mON = false;
  bool mWantON = false;

  WebsocketsClient webSocket;
  bool mIsSocket = false;
  bool mWSConnected = false; 

  BaseLedStrip* mStrips[MAXSTRIPS];
  byte          mNStrips = 0;

  byte*         payload;
  int           maxPayloadLength = 0;

public:
  void start();
  void stop();
  
  bool isWSConnected() { return mIsSocket && mWSConnected; };
  
  void addStrip(const BaseLedStrip &strip);
  ForEachMethod(addStrip); // create a method addStrips(...) that calls addStrip on all args

  bool update();
};

