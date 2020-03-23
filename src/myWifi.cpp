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

    if (mNStrips < MAXSTRIPS)
      mLeds[mNStrips++] = (BaseLedStrip*)&leds;
  }
}

void myWifi::update()
{
  if (mON)
  {
    webSocket.loop();

    for (byte i=0; i < mNStrips; i++)
    {
      int length;
      byte *data = mLeds[i]->getData(length);

      #define INFO_LEN 15
      char info[INFO_LEN];
      snprintf(info, INFO_LEN, "STRIP %d %d", i, length/3); 
      webSocket.sendTXT(info, strlen(info));

      webSocket.sendBIN(data, length);
    }
  }
}
