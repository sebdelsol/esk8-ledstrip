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
    BluetoothSerial& BTSerial = BT.getSerial();

    emulateCmdForAllVars(mInitKeyword, BTSerial, &OBJVar::isVarShown); //for all vars, emulate a init cmd and send the result to BTSerial
    BTSerial << "initdone" << endl;
  }
}

//----------------
bool AllObjBT::sendUpdate(BlueTooth &BT, MOTION& motion)
{
  if(BT.isReadyToSend())
  {
    BluetoothSerial& BTSerial = BT.getSerial();
    SensorOutput& m = motion.mOutput;

    emulateCmdForAllVars(mGetKeyword, BTSerial, &OBJVar::hasVarChanged, true, true); //for all vars, emulate a get cmd and send the result to BTSerial
    if(m.updated)
      BTSerial << ALLOBJ_MOTION_CMD << " " << m.axis.x << " " << m.axis.y << " " << m.axis.z << " " << m.angle << " " << m.accY << " " << m.wZ << endl;
  }
}

//----------------
bool AllObjBT::receiveUpdate(BlueTooth &BT)
{
  if (BT.isReadyToReceive())
  {
    BluetoothSerial& BTSerial = BT.getSerial();
    readCmdFromStream(BTSerial, mBTbuf, false, true);
  }
}
