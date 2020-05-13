#pragma once

#include <bluetooth.h>
#include <myMpu6050.h>
#include <Pickle.h>

class PickleBT : public Pickle
{
public:
  PickleBT(Stream& dbgSerial);
  
  bool receiveUpdate(BlueTooth &BT);
  bool sendUpdate(BlueTooth &BT, myMPU6050& mMotion);
  void sendInits(BlueTooth &BT);
};
