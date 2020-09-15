#pragma once

// #define ARDUHAL_LOG_LEVEL ARDUHAL_LOG_LEVEL_NONE

#include <WiFi.h>
#include <ESPmDNS.h>
#include <log.h>
#include <ledstrip.h>

// wificonfig.h needs to define :
// #define WIFI_ssid "******"
// #define WIFI_password "****"
#include <wificonfig.h>  

const long WIFI_TIMEOUT = 10 * 1000; // sec before stopping non connected wifi

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

  void socketInit();
  void socketUpdate();
  void sendStripData();
  bool justConnected();

public:
  void start();
  void stop(const char* reason);
 
  void addStrip(const BaseLedStrip &strip);
  ForEachMethod(addStrip); // create a method addStrips(...) that calls addStrip on all args

  bool update();
};

