#include <myWifi.h>

void myWifi::off()
{
  WiFi.mode(WIFI_OFF);
  // WiFi.forceSleepBegin();
  digitalWrite(BUILTIN_LED, HIGH); // led off

  mON = false;
  Serial << "Wifi off" << endl;
}

void myWifi::on(int count)
{
  WiFi.mode(WIFI_STA);
  // WiFi.forceSleepWake();
  WiFi.begin(WIFINAME, WIFIPASS);

  Serial << "Wifi Connecting";
  while(WiFi.status() != WL_CONNECTED && count-- > 0)
  {
    delay(500);
    digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
    Serial << ".";
  }

  if(WiFi.status() == WL_CONNECTED)
  {
    Serial << "Connected, IP address: " << WiFi.localIP() << endl;
    digitalWrite(BUILTIN_LED, LOW); // led on
    mON = true;
  }
  else off();
}

void myWifi::addLeds(const BaseLedStrip &leds)
{
  if (mON)
  {
    if (!mSocketBegun)
    {
      webSocket.begin(SOCK_ADDR, SOCK_PORT);
      mSocketBegun = true;
    }
    mLeds = (BaseLedStrip*)&leds;
  }
}

void myWifi::update()
{
  if (mON)
  {
    webSocket.loop();

    if (mLeds)
    {
      int length;
      byte *data = mLeds->getData(length);
      webSocket.sendBIN(data, length);
    }
  }
}
