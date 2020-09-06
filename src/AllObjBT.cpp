#include <AllObjBT.h>

//----------------
void AllObjBT::sendInits(BlueTooth &BT)
{
  if(BT.isReadyToSend())
  {
    //for all vars, send an init cmd and output the result in BTSerial (a list of init of vars)
    sendCmdForAllVars(mInitKeyword, BT.getSerial(), TrackChange::undefined, Decode::undefined, &OBJVar::isVarShown); 
    BT.getSerial() << "initdone" << endl;
  }
}

//----------------
void AllObjBT::sendUpdate(BlueTooth &BT, MPU& mpu)
{
  if(BT.isReadyToSend())
  {
    //for all vars, send a get cmd and output the result in BTSerial (a list of set of changed vars)
    sendCmdForAllVars(mGetKeyword, BT.getSerial(), TrackChange::undefined, Decode::compact, &OBJVar::hasVarChanged); 

    SensorOutput& m = mpu.mOutput;
    if(m.updated)
      BT.getSerial() << JoinbySpace(ALLOBJ_MPU_CMD, m.axis.x, m.axis.y, m.axis.z, m.angle, m.accY, m.wZ) << endl;
  }
}

//----------------
void AllObjBT::receiveUpdate(BlueTooth &BT)
{
  if (BT.isReadyToReceive())
    // should receive a list of set cmd 
    // DO NOT track change or what's received would echoed in sendUpdate
    readCmd(BT.getSerial(), mBTbuf, TrackChange::no, Decode::undefined); // there's nothing to decode set cmd @the moment
}
