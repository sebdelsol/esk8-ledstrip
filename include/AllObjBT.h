#pragma once

#include <Bluetooth.h>
#include <Motion.h>
#include <AllObj.h>

class AllObjBT : public AllObj
{
public:
  AllObjBT(Stream& dbgSerial);
  
  bool receiveUpdate(BlueTooth &BT);
  bool sendUpdate(BlueTooth &BT, MOTION& motion);
  void sendInits(BlueTooth &BT);
};
