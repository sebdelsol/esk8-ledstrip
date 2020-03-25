#include <myWifi.h>

void myWifi::init(Stream &serial)
{
  mSerial = &serial;
}

void myWifi::off()
{
  WiFi.mode(WIFI_OFF);
  // WiFi.forceSleepBegin();
  digitalWrite(BUILTIN_LED, HIGH); // led off

  mON = false;
  *mSerial << "Wifi off" << endl;
}

void myWifi::on(int count)
{
  WiFi.mode(WIFI_STA);
  // WiFi.forceSleepWake();
  WiFi.begin(WIFINAME, WIFIPASS);

  *mSerial << "Wifi Connecting";
  while(WiFi.status() != WL_CONNECTED && count-- > 0)
  {
    delay(500);
    digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
    *mSerial << ".";
  }

  if(WiFi.status() == WL_CONNECTED)
  {
    *mSerial << "Connected, IP address: " << WiFi.localIP() << endl;
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
      *mSerial << "Socket client started" << endl;
      webSocket.begin(SOCK_ADDR, SOCK_PORT);
      mSocketBegun = true;
    }

    if (mNStrips < MAXSTRIPS)
      mLeds[mNStrips++] = (BaseLedStrip*)&leds;
  }
}

void myWifi::update()
{
  if (mON && mSocketBegun)
  {
    webSocket.loop();

    for (byte i=0; i < mNStrips; i++)
    {
      int length;
      byte *data = mLeds[i]->getData(length);
      snprintf(mInfo, INFO_LEN, "STRIP %d %d", i, length/3); 

      webSocket.sendTXT(mInfo, strlen(mInfo));
      webSocket.sendBIN(data, length);
    }
  }
}
