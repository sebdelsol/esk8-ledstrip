#include <myWifi.h>

void myWifi::off()
{
  WiFi.mode(WIFI_OFF);
  // WiFi.forceSleepBegin();

  mON = false;
  Serial << "Wifi off" << endl;
}

void myWifi::on()
{
  Serial << "before Connecting";
  WiFi.mode(WIFI_STA);
  // WiFi.forceSleepWake();
  WiFi.begin(WIFINAME, WIFIPASS);

  Serial << "Connecting";
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    // digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
    Serial << ".";
  }
  Serial << endl << "Connected, IP address: " << WiFi.localIP() << endl;
  // digitalWrite(BUILTIN_LED, LOW); // led on
  webSocket.begin(SOCK_ADDR, SOCK_PORT);
  mON = true;
}

void myWifi::addLeds(const BaseLedStrip &leds)
{
  mLeds = (BaseLedStrip*)&leds;
}

void myWifi::update()
{
  if (mON) {
    webSocket.loop();

    if (mLeds) {
      int length;
      byte *data = mLeds->getData(length);
      webSocket.sendBIN(data, length);
    }
  }
}
