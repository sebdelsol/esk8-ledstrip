#pragma once

#include <Streaming.h>
#include <BluetoothSerial.h>
#include <BTcmd.h>
#include <myPins.h>

//#define BT_BAUD 9600 //38400 //9600

class BlueTooth
{
  bool mON = false;
  BTcmd *mBTcmd;
  BluetoothSerial *mBTserial;

  void start(const bool on);

public:
  BluetoothSerial *getBtSerial() { return mBTserial; };
  void answer(const char *txt) { mBTserial->println(txt); };
  bool update() { if (mON) mBTcmd->readStream(); return mON; };
  bool registerFX(const FX& fx, char desc) { return mBTcmd->registerFX(fx, desc); };

  BlueTooth();
  void init(bool on=false);
  void toggle();
};
