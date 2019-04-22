
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

void BlueTooth::init()
{
  Serial << "Init BT" << endl;
  BTSerial.begin("Esk8_2");
  start(false);
  Serial << "Init BT done" << endl;
}

// Power management
void BlueTooth::start(const bool on)
{
  Serial << (on ? "Start" : "Stop") << " BT" << endl;
  mON = on;
  mON ? btStart() : btStop();
  Serial << (on ? "Start" : "Stop") << " BT done" << endl;
}

void BlueTooth::toggle()
{
  start(mON ? false : true);
}
