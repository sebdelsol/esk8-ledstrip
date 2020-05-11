#pragma once

#include <Streaming.h>
#include <BluetoothSerial.h>
#include <myPins.h>

#define BT_TERMINAL_NAME "Esk8"
#define AUTO_STOP_IF_NOTCONNECTED 30000 // duration before bluetooth autostop after started if not connected

class BlueTooth
{
  long mStartTime;
  bool mON = false;
  bool mConnected = false;

protected:
  Stream* mDbgSerial;
  BluetoothSerial mBTSerial;

public:
  void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t* param); //for trampoline

  bool isReadyToReceive();
  bool isReadyToSend();

  void initBT(Stream& serial);
  void start(const bool on=true);
  void toggle();
};
