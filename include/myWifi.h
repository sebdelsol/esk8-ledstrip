#pragma once

#define NODEBUG_WEBSOCKETS

#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <log.h>
#include <ledstrip.h>
#include <wificonfig.h>  
// wificonfig.h needs to define :
// #define WIFINAME "******"
// #define WIFIPASS "****"

#define MAXSTRIPS    3
#define WIFI_TIMEOUT 10000

#define SOCK_PORT    80      
#define MDNSNAME    "esp32"

// see debugLedstrip.py to use the same type, it'll give '_leds._tcp.local.'
#define MDNSTYPE    "leds" 
#define MDNSPROT    "tcp"

class myWifi
{
  int  mBegunOn;
  bool mON = false;
  bool mWantON = false;

  WebSocketsServer mServer = WebSocketsServer(SOCK_PORT);

  bool mIsSocket           = false;
  bool mIsClientConnected  = false; 
  bool mWasClientConnected = false;

  BaseLedStrip* mStrips[MAXSTRIPS];
  byte          mNStrips = 0;

  byte*         payload;
  int           maxPayloadLength = 0;

public:
  void start();
  void stop();
  
  bool isClientConnected() { return mIsSocket && mIsClientConnected; };
  
  void addStrip(const BaseLedStrip &strip);
  ForEachMethod(addStrip); // create a method addStrips(...) that calls addStrip on all args

  bool update();
};

