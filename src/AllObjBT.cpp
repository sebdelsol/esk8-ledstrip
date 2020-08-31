#include <AllObjBT.h>

//----------------
void AllObjBT::sendInits(BlueTooth &BT)
{
  if(BT.isReadyToSend())
  {
    emulateCmdForAllVars(mInitKeyword, BT.getSerial(), &OBJVar::isVarShown); //for all vars, emulate a init cmd and send the result to BTSerial
    BT.getSerial() << "initdone" << endl;
  }
}

//----------------
void AllObjBT::sendUpdate(BlueTooth &BT, MPU& mpu)
{
  if(BT.isReadyToSend())
  {
    emulateCmdForAllVars(mGetKeyword, BT.getSerial(), &OBJVar::hasVarChanged, true, true); //for all vars, emulate a get cmd and send the result to BTSerial

    SensorOutput& m = mpu.mOutput;
    if(m.updated)
      JoinbySpace(BT.getSerial(), ALLOBJ_MPU_CMD, m.axis.x, m.axis.y, m.axis.z, m.angle, m.accY, m.wZ) << endl;
  }
}

//----------------
void AllObjBT::receiveUpdate(BlueTooth &BT)
{
  if (BT.isReadyToReceive())
    readCmdFromStream(BT.getSerial(), mBTbuf, false, true);
}
