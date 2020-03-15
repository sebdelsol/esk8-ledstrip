#pragma once

#include <Streaming.h>
#include <BluetoothSerial.h>
#include <BTcmd.h>
#include <myPins.h>

//#define BT_BAUD 9600 //38400 //9600
#define BT_TERMINAL_NAME "Esk8"

class BlueTooth
{
  bool mON = false;
  BTcmd *mBTcmd;
  BluetoothSerial *mBTserial;

  void start(const bool on);

public:
  BluetoothSerial *getBtSerial() { return mBTserial; };
  // void answer(const char *txt) { mBTserial->println(txt); };
  bool update();
  bool registerObj(const OBJCmd& obj, char* name) { return mBTcmd->registerObj(obj, name); };

  BlueTooth();
  void init(bool on=false);
  void toggle();
};
