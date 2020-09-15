#pragma once

#include <WiFi.h>
#include <ESPmDNS.h>
#include <ledstrip.h>
#include <log.h>

class LedServer
{
  WiFiServer mServer = WiFiServer(OTA_PORT);
  WiFiClient mClient;

  bool mIsClientConnected  = false; 
  bool mWasClientConnected = false;

  BaseLedStrip* mStrips[MAXSTRIP];
  byte          mNStrips = 0;

  bool mHasBegun = false;
  void begin();
  void send();

public:
  void addStrip(const BaseLedStrip &strip);
  ForEachMethod(addStrip); // create a method addStrips(...) that calls addStrip on all args

  bool update();
};
