#pragma once

#include <BluetoothSerial.h>
#include <log.h>
#include <Pins.h>

#define BT_TERMINAL_NAME "Esk8"
#define AUTO_STOP_IF_NOTCONNECTED 30000 // duration before bluetooth autostop if not connected

class BlueTooth
{
  long mStartTime;
  bool mON = false;
  bool mConnected = false;

  BluetoothSerial mBTSerial;

public:
  void init(const bool on=true);
  void start(const bool on=true);
  void toggle();
  void onEvent(esp_spp_cb_event_t event, esp_spp_cb_param_t* param); 
  BluetoothSerial& getSerial() { return mBTSerial; };
  bool isReady();
};
