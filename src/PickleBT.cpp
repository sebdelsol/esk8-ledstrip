#include <PickleBT.h>

PickleBT::PickleBT(Stream& dbgSerial) : Pickle(dbgSerial) {}  

void PickleBT::sendInits(BlueTooth &BT)
{
  if(BT.isReadyToSend())
  {
    emulateCmdForAllVars(mInitKeyword, BT.mBTSerial, &OBJVar::isVarShown); //for all vars, emulate a init cmd and send the result to mBTSerial
    BT.mBTSerial << "initdone" << endl;
  }
}

//----------------
bool PickleBT::sendUpdate(BlueTooth &BT, myMPU6050& mMotion)
{
  if(BT.isReadyToSend())
  {
    emulateCmdForAllVars(mGetKeyword, BT.mBTSerial, &OBJVar::hasVarChanged, true, true); //for all vars, emulate a get cmd and send the result to mBTSerial

    if(mMotion.updated)
    {
      SensorOutput& m = mMotion.mOutput;
      BT.mBTSerial << PICKLE_MOTION_CMD << " " << m.axis.x << " " << m.axis.y << " " << m.axis.z << " " << m.angle << " " << m.accY << " " << m.wZ << endl;
    }
  }
}

bool PickleBT::receiveUpdate(BlueTooth &BT)
{
  if (BT.isReadyToReceive())
    readCmdFromStream(BT.mBTSerial, BT.mBTbuf, false, true);
}
