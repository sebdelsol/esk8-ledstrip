#pragma once

#define NODEBUG_WEBSOCKETS

#include <ESP8266WiFi.h>
#include <Streaming.h>
#include <ledstrip.h>
#include <WebSocketsClient.h>

#include <wificonfig.h>
// #define WIFINAME "******"
// #define WIFIPASS "****"
// #define SOCK_ADDR "**.**.**.**"
// #define SOCK_PORT **

class myWifi
{
  bool mON = true;
  LedStrip* mLeds;
  WebSocketsClient webSocket;

public:
  void on();
  void off();
  void toggle() { mON ? off() : on(); };
  void addLeds(const LedStrip &leds);
  void update();
};
