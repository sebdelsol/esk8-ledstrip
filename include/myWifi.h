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

class myWifi
{
  bool mON = true;
  bool mSocketBegun = false;
  BaseLedStrip* mLeds;
  WebSocketsClient webSocket;

public:
  void on(int count=15);
  void off();
  void toggle() { mON ? off() : on(); };
  void addLeds(const BaseLedStrip &leds);
  void update();
};
