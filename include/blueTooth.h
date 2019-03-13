#pragma once

#include <Streaming.h>
#include <SoftwareSerial.h>
#include <BTcmd.h>
#include <myPins.h>

#define BT_BAUD 9600 //38400 //9600

class BlueTooth
{
  bool mON = false;
  BTcmd *mBTcmd;
  SoftwareSerial *mBTserial;

  void cmdAT(const char *fmt, ...);
  void start(const bool on);

public:
  // void addCMD(const char *cmd, void(*func)()) { mSCmd->addCommand(cmd, func); };
  // char* getArg() { return mSCmd->next(); };
  SoftwareSerial *getBtSerial() { return mBTserial; };
  void answer(const char *txt) { mBTserial->println(txt); };
  bool update() { if (mON) mBTcmd->readStream(); return mON; };
  bool registerFX(const FX& fx, char desc) { return mBTcmd->registerFX(fx, desc); };

  BlueTooth();
  void init();
  void toggle();
};
