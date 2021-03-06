#pragma once

#include <Bluetooth.h>
#include <mpu.h>
#include <AllObj.h>
#include <Buf.h>

#define CMD_MPU_UPDATE CMD_RESERVED

class AllObjBT : public AllObj
{
  BUF mBTbuf;

public:
  AllObjBT() { mBTbuf.clear(); };
  
  void receiveUpdate(BlueTooth& BT);
  void sendUpdate(BlueTooth& BT, MPU& mpu);
  void sendInits(BlueTooth& BT);
};
