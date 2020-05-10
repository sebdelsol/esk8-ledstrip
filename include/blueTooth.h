#pragma once

#include <Streaming.h>
#include <BluetoothSerial.h>
#include <myMpu6050.h>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// in BluetoothSerial.cpp
// #define TX_QUEUE_SIZE 128
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include <BTcmd.h>
#include <myPins.h>

//#define BT_BAUD 9600 //38400 //9600
#define BT_TERMINAL_NAME "Esk8"
#define AUTO_STOP_IF_NOTCONNECTED 30000 // duration before bluetooth autostop after started if not connected

class BlueTooth
{
  long mStartTime;
  bool mON = false;
  BTcmd* mBTcmd;
  BluetoothSerial* mBTserial;
  Stream* mDbgSerial;

  myMPU6050& mMotion; // for sendUpdate

public:
  bool update();
  bool registerObj(OBJVar& obj, char* name) { return mBTcmd->registerObj(obj, name); };
  void save(bool isdefault) { mBTcmd->save(isdefault); };
  void load(bool isdefault, bool change = true) { mBTcmd->load(isdefault, change); };
  void sendInitsOverBT() { mBTcmd->sendInitsOverBT(); };
  void sendUpdate();

  BlueTooth(myMPU6050& motion);
  void init(Stream& serial);
  void start(const bool on=true);
  void toggle();
};
