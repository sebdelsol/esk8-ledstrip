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

  AllLedStrips* mAllStrip;

  bool mHasBegun = false;
  void begin();
  void send();

public:
  void addAllStrips(AllLedStrips &allStrip);
  bool update();
};
