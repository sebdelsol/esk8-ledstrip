#pragma once

#define NODEBUG_WEBSOCKETS

#include <WiFi.h>//<ESP8266WiFi.h>
#include <Streaming.h>
#include <ledstrip.h>
#include <WebSocketsClient.h>

#include <wificonfig.h>
// wificonfig.h needs thos defines :
// #define WIFINAME "******"
// #define WIFIPASS "****"
// #define SOCK_ADDR "**.**.**.**"
// #define SOCK_PORT **

#define MAXSTRIPS 3

class myWifi
{
  bool mON = true;
  bool mSocketBegun = false;
  WebSocketsClient webSocket;

  // BaseLedStrip* mLeds;
  BaseLedStrip* mLeds[MAXSTRIPS];
  byte mNStrips = 0;

public:
  void on(int count=15);
  void off();
  void toggle() { mON ? off() : on(); };
  void addLeds(const BaseLedStrip &leds);
  void update();
};
