#pragma once

#define NODEBUG_WEBSOCKETS

#include <WiFi.h>
#include <ESPmDNS.h>
#include <log.h>
#include <ledstrip.h>

// wificonfig.h needs to define :
// #define WIFI_ssid "******"
// #define WIFI_password "****"
#include <wificonfig.h>  

#define WIFI_TIMEOUT 10000

// using namespace websockets;

class myWifi
{
  int  mBegunOn;
  bool mON = false;
  bool mWantON = false;

  WiFiServer mServer = WiFiServer(OTA_PORT);
  WiFiClient mClient;

  bool mIsSocket           = false;
  bool mIsClientConnected  = false; 
  bool mWasClientConnected = false;

  BaseLedStrip* mStrips[MAXSTRIP];
  byte          mNStrips = 0;

public:
  void start();
  void stop();
  
  void socketInit();
  void socketUpdate();

  void addStrip(const BaseLedStrip &strip);
  void sendStripData();
  ForEachMethod(addStrip); // create a method addStrips(...) that calls addStrip on all args

  bool update();
};

