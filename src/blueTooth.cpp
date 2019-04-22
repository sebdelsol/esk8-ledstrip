
#include <bluetooth.h>
#include <stdarg.h>

BluetoothSerial BTSerial;
BTcmd BTcmd(BTSerial);

// inits
BlueTooth::BlueTooth()
{
  mBTserial = &BTSerial;
  mBTcmd = &BTcmd;
}

void BlueTooth::init(bool on)
{
  Serial << "Init BT" << endl;
  start(on);
}

// Power management
void BlueTooth::start(const bool on)
{
  Serial << (on ? "Start" : "Stop") << " BT" << endl;
  mON = on;
  if (mON){
    btStart();
    BTSerial.begin("Esk8_2");
  }else{
    btStop();
    BTSerial.end();
  }
  // Serial << (on ? "Start" : "Stop") << " BT done" << endl;
}

void BlueTooth::toggle()
{
  start(mON ? false : true);
}
