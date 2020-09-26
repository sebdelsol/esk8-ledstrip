#pragma once

#include <BluetoothSerial.h>
#include <Pins.h>
#include <log.h>

const char* const BT_SERVER_NAME = "Esk8";
const long BT_TIMEOUT            = 30 * 1000; // sec before stopping non connected BT

class BlueTooth
{
  BluetoothSerial mBTSerial;
  bool mON = false;

  static long mStartTime;
  static bool mConnected;
  static void onEvent(esp_spp_cb_event_t event, esp_spp_cb_param_t* param); 

public:
  void init(const bool on=true);
  void start(const bool on=true);
  void toggle();
  BluetoothSerial& getSerial() { return mBTSerial; };
  bool isReady();
};
