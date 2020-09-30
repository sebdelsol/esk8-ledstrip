#include <AllObjBT.h>

//----------------
void AllObjBT::sendInits(BlueTooth &BT)
{
  if(BT.isReady())
  {
    BluetoothSerial& BTserial = BT.getSerial();

    // for all vars, send an init cmd and output the result in BTSerial (a list of init of vars)
    sendCmdForAllVars(BTserial, CMD_INIT, TrackChange::undefined, Decode::undefined, &MyVar::isShown); 

    // end of inits
    BTserial << CMD_INIT_DONE << endl;
  }
}

//----------------
void AllObjBT::sendUpdate(BlueTooth &BT, MPU& mpu)
{
  if(BT.isReady())
  {
    BluetoothSerial& BTserial = BT.getSerial();

    // for all vars, send a get cmd and output the result in BTSerial (a list of set of changed vars)
    sendCmdForAllVars(BTserial, CMD_GET, TrackChange::undefined, Decode::compact, &MyVar::hasChanged); 

    // send mpu update 
    SensorOutput& m = mpu.mOutput;
    if(m.updated)
      BTserial << SpaceIt(CMD_MPU_UPDATE, m.axis.x, m.axis.y, m.axis.z, m.angle, m.acc, m.w) << endl;
  }
}

//----------------
void AllObjBT::receiveUpdate(BlueTooth &BT)
{
  if (BT.isReady())
  {
    // should receive a list of set cmd 
    // DO NOT track change or what's received would echoed in sendUpdate
    readCmd(BT.getSerial(), mBTbuf, TrackChange::no, Decode::undefined); // there's nothing to decode set cmd @the moment
  }
}
