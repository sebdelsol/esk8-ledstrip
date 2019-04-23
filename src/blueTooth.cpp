
#include <bluetooth.h>
#include <stdarg.h>

BluetoothSerial BTSerial;
BTcmd BTcmd(BTSerial);

bool Connected = false;

void BTcallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    Serial << "Client Connected" << endl;
    Connected = true;
  }
  else if(event == ESP_SPP_CLOSE_EVT){
    Serial << "Client DisConnected" << endl;
    Connected = false;
  }
}

// inits
BlueTooth::BlueTooth()
{
  mBTserial = &BTSerial;
  mBTcmd = &BTcmd;
}

void BlueTooth::init(bool on)
{
  Serial << "Init BT" << endl;
  BTSerial.register_callback(BTcallback);
  pinMode(GREEN_PIN, OUTPUT);
  start(on);
}

// Power management
void BlueTooth::start(const bool on)
{
  Serial << (on ? "Start" : "Stop") << " BT" << endl;
  digitalWrite(GREEN_PIN, on ? HIGH : LOW);
  if (on){
    mON = BTSerial.begin(BT_TERMINAL_NAME);
    digitalWrite(GREEN_PIN, mON ? HIGH : LOW);
  }
  else{
    mON = false;
    BTSerial.end();
  }
  Connected = false;
  // Serial << (mON ? "Start" : "Stop") << " BT done" << endl;
}

void BlueTooth::toggle()
{
  start(mON ? false : true);
}

bool BlueTooth::update()
{
  if (mON & Connected) mBTcmd->readStream();
  return mON;
}
