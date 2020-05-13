#pragma once

#include <Streaming.h>
#include <BluetoothSerial.h>
#include <Buf.h>
#include <Pins.h>

#define BT_TERMINAL_NAME "Esk8"
#define AUTO_STOP_IF_NOTCONNECTED 30000 // duration before bluetooth autostop after started if not connected

class BlueTooth
{
  long mStartTime;
  bool mON = false;
  bool mConnected = false;

  Stream& mDbgSerial;

public:
  BluetoothSerial mBTSerial;
  BUF mBTbuf;

  void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t* param); //for trampoline

  bool isReadyToReceive();
  bool isReadyToSend();

  BlueTooth(Stream& serial);
  void init();
  void start(const bool on=true);
  void toggle();

  bool receiveUpdate();
  bool sendUpdate();
  void sendInits();
};
