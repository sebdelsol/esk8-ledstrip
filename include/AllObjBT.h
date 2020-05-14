#pragma once

#include <Bluetooth.h>
#include <Motion.h>
#include <AllObj.h>
#include <Buf.h>

#define ALLOBJ_MOTION_CMD ALLOBJ_RESERVED

class AllObjBT : public AllObj
{
  BUF mBTbuf;

public:
  AllObjBT(Stream& dbgSerial);
  
  bool receiveUpdate(BlueTooth &BT);
  bool sendUpdate(BlueTooth &BT, MOTION& motion);
  void sendInits(BlueTooth &BT);
};
