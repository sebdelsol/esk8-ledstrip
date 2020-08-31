#include <AllObjBT.h>

//----------------
void AllObjBT::sendInits(BlueTooth &BT)
{
  if(BT.isReadyToSend())
  {
    //for all vars, emulate a init cmd and send the result to BTSerial (init vars with min max)
    emulateCmdForAllVars(mInitKeyword, BT.getSerial(), &OBJVar::isVarShown); 
    BT.getSerial() << "initdone" << endl;
  }
}

//----------------
void AllObjBT::sendUpdate(BlueTooth &BT, MPU& mpu)
{
  if(BT.isReadyToSend())
  {
    //for all vars, emulate a get cmd and send the result to BTSerial (changed vars)
    emulateCmdForAllVars(mGetKeyword, BT.getSerial(), &OBJVar::hasVarChanged, true, true); 

    SensorOutput& m = mpu.mOutput;
    if(m.updated)
      JoinbySpace(BT.getSerial(), ALLOBJ_MPU_CMD, m.axis.x, m.axis.y, m.axis.z, m.angle, m.accY, m.wZ) << endl;
  }
}

//----------------
void AllObjBT::receiveUpdate(BlueTooth &BT)
{
  if (BT.isReadyToReceive())
    readAndHandleCmd(BT.getSerial(), mBTbuf, false, true);
}
