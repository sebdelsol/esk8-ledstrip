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

  mWantON = false;
  mON = false;
  *mSerial << "Wifi off" << endl;
}

void myWifi::on()
{
  WiFi.mode(WIFI_STA);
  // WiFi.forceSleepWake();
  WiFi.begin(WIFINAME, WIFIPASS);
  
  mBegunOn = millis();
  mON = false;
  mWantON = true;

  *mSerial << "Wifi Connecting" << endl;
}

void myWifi::addLeds(const BaseLedStrip &leds)
{
  mIsSocket = true;

  if (mNStrips < MAXSTRIPS)
    mLeds[mNStrips++] = (BaseLedStrip* )&leds;
}

bool myWifi::update()
{
  if(mWantON)
  {
    if(!mON)
    {
      if(millis() - mBegunOn < WIFI_TIMEOUT)
      {
        if(WiFi.status() == WL_CONNECTED)
        {
          *mSerial << "Wifi connected, IP address: " << WiFi.localIP() << endl;
          digitalWrite(BUILTIN_LED, LOW); // led on
          mON = true;

          if (mIsSocket)
          {
            *mSerial << "Socket client started" << endl;
            webSocket.begin(SOCK_ADDR, SOCK_PORT);
          }
        }
      }
      else 
        off();
    }
    
    else
    {
      if (mIsSocket)
      {
        webSocket.loop();

        for (byte i=0; i < mNStrips; i++)
        {
          int length;
          byte* data = mLeds[i]->getData(length);
          snprintf(mInfo, INFO_LEN, "STRIP %d %d", i, length/3); 

          webSocket.sendTXT(mInfo, strlen(mInfo));
          webSocket.sendBIN(data, length);
        }
      }
    }
  }

  return mON;
}
