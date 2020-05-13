#include <AllObjBT.h>

AllObjBT::AllObjBT(Stream& dbgSerial) : AllObj(dbgSerial) {}  

void AllObjBT::sendInits(BlueTooth &BT)
{
  if(BT.isReadyToSend())
  {
    emulateCmdForAllVars(mInitKeyword, BT.mBTSerial, &OBJVar::isVarShown); //for all vars, emulate a init cmd and send the result to mBTSerial
    BT.mBTSerial << "initdone" << endl;
  }
}

//----------------
bool AllObjBT::sendUpdate(BlueTooth &BT, myMPU6050& motion)
{
  if(BT.isReadyToSend())
  {
    emulateCmdForAllVars(mGetKeyword, BT.mBTSerial, &OBJVar::hasVarChanged, true, true); //for all vars, emulate a get cmd and send the result to mBTSerial

    SensorOutput& m = motion.mOutput;
    if(m.updated)
      BT.mBTSerial << ALLOBJ_MOTION_CMD << " " << m.axis.x << " " << m.axis.y << " " << m.axis.z << " " << m.angle << " " << m.accY << " " << m.wZ << endl;
  }
}

bool AllObjBT::receiveUpdate(BlueTooth &BT)
{
  if (BT.isReadyToReceive())
    readCmdFromStream(BT.mBTSerial, BT.mBTbuf, false, true);
}
