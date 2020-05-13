#pragma once

#include <bluetooth.h>
#include <myMpu6050.h>
#include <AllObj.h>

class AllObjBT : public AllObj
{
public:
  AllObjBT(Stream& dbgSerial);
  
  bool receiveUpdate(BlueTooth &BT);
  bool sendUpdate(BlueTooth &BT, myMPU6050& mMotion);
  void sendInits(BlueTooth &BT);
};
