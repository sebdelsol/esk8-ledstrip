#include <bluetooth.h>
#include <rom/rtc.h>

BluetoothSerial BTSerial;
BTcmd BTcmd(BTSerial);

bool Connected = false;
Stream* DbgSerialForCB;

void BTcallback(esp_spp_cb_event_t event, esp_spp_cb_param_t* param)
{
  if(event == ESP_SPP_SRV_OPEN_EVT)
  {
    *DbgSerialForCB << "BT Client Connected @ ";
    for (int i = 0; i < 6; i++)
      *DbgSerialForCB << _HEX(param->srv_open.rem_bda[i]) << (i < 5 ? ":" : "");
    *DbgSerialForCB << endl;

    Connected = true;
  }
  else if(event == ESP_SPP_CLOSE_EVT)
  {
    *DbgSerialForCB << "BT Client DisConnected" << endl;
    Connected = false;
  }
}

// inits
BlueTooth::BlueTooth(myMPU6050& motion) : mMotion(motion)
{
  mBTserial = &BTSerial;
  mBTcmd = &BTcmd;
}

static __NOINIT_ATTR bool WasOn; // HACK

void BlueTooth::init(Stream& dbgSerial)
{
  mDbgSerial = &dbgSerial;
  DbgSerialForCB = &dbgSerial;

  BTSerial.register_callback(BTcallback);
  mBTcmd->init(dbgSerial);  

  pinMode(LIGHT_PIN, OUTPUT); //blue led

  if (rtc_get_reset_reason(0) == 12 && WasOn) // HACK, SW reset is proly crash, so auto restart BT if it was on
  {
    *mDbgSerial << "Reset detected, relaunch BT " << rtc_get_reset_reason(0) << endl;
    start(true);
  }
}

// Power management
void BlueTooth::start(const bool on)
{
  if (mON != on)
  {
    *mDbgSerial << "BT " << (on ? "Starting" : "Stopping") << endl;
    Connected = false;
  
    if (on)
    {
      mON = BTSerial.begin(BT_TERMINAL_NAME);
      if (mON) mStartTime = millis();
    }
    else 
    {
      mON = false;
      BTSerial.end();
    }

    WasOn = mON; //HACK
    *mDbgSerial << "BT has " << (mON ? "Started" : "Stopped") << endl;
    digitalWrite(LIGHT_PIN, mON ? HIGH : LOW);
  }
}

void BlueTooth::toggle()
{
  start(mON ? false : true);
}

bool BlueTooth::update()
{
  if (mON)
  {
    if (Connected)
    {
      mBTcmd->readBTStream();
      return true;
    }
    else if(millis() - mStartTime > AUTO_STOP_IF_NOTCONNECTED)
      start(false);
  }
  return false;
}

void BlueTooth::sendUpdate()
{
  if (mON && Connected)
  {
    mBTcmd->sendUpdateOverBT();

    if(mMotion.updated)
    {
      #define MOTION_UPDATE_CMD '!' // see BTCMD_1ST_ID

      SensorOutput& m = mMotion.mOutput;
      *mBTserial << MOTION_UPDATE_CMD << " " << m.axis.x << " " << m.axis.y << " " << m.axis.z << " " << m.angle << " " << m.accY << " " << m.wZ << endl;
    }
  }
}
