#include <AllObjBT.h>

//----------------
AllObjBT::AllObjBT(Stream& dbgSerial) : AllObj(dbgSerial) 
{
  mBTbuf.clear();
}  

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
bool AllObjBT::sendUpdate(BlueTooth &BT, MOTION& motion)
{
  if(BT.isReadyToSend())
  {
    emulateCmdForAllVars(mGetKeyword, BT.getSerial(), &OBJVar::hasVarChanged, true, true); //for all vars, emulate a get cmd and send the result to BTSerial

    SensorOutput& m = motion.mOutput;
    if(m.updated)
      BT.getSerial() << JoinbySpace(ALLOBJ_MOTION_CMD, m.axis.x, m.axis.y, m.axis.z, m.angle, m.accY, m.wZ) << endl;
  }
}

//----------------
bool AllObjBT::receiveUpdate(BlueTooth &BT)
{
  if (BT.isReadyToReceive())
    readCmdFromStream(BT.getSerial(), mBTbuf, false, true);
}
