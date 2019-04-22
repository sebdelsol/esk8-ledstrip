
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
  pinMode(GREEN_PIN, OUTPUT);
  start(on);
}

// Power management
void BlueTooth::start(const bool on)
{
  Serial << (on ? "Start" : "Stop") << " BT" << endl;
  mON = on;
  if (mON){
    btStart();
    BTSerial.begin(BT_TERMINAL_NAME);
  }else{
    btStop();
    BTSerial.end();
  }
  digitalWrite(GREEN_PIN, mON ? HIGH : LOW);
  // Serial << (on ? "Start" : "Stop") << " BT done" << endl;
}

void BlueTooth::toggle()
{
  start(mON ? false : true);
}
