#include <myWifi.h>

// ----------------------------------------------------
void myWifi::stop()
{
  WiFi.mode(WIFI_OFF);
  digitalWrite(BUILTIN_LED, HIGH); // led off

  mWantON = false;
  mON = false;
  _log << "Wifi stop" << endl;
}

void myWifi::start()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_ssid, WIFI_password);
  
  mBegunOn = millis();
  mON = false;
  mWantON = true;

  _log << "Wifi start" << endl;
}

// ----------------------------------------------------
void myWifi::addStrip(const BaseLedStrip &strip)
{
  mIsSocket = true;

  if (mNStrips < MAXSTRIP)
    mStrips[mNStrips++] = (BaseLedStrip*)&strip;
}

void myWifi::sendStripData()
{
    for (byte i=0; i < mNStrips; i++)
    {
      BaseLedStrip& strip = *mStrips[i];
      int length = strip.getRawLength();
      mClient.write(length); 
      mClient.write(i); 
      mClient.write(strip.getRawData(), length); 
    }
}

// --------------
void myWifi::socketInit()
{
  if (mIsSocket)
  {
    // OTA_NAME & OTA_PORT are shared by OTA and the webSocket server, check platformio build_flags
    mServer.begin();
    _log << "Socket server, answer @ " << OTA_NAME << ".local:" << OTA_PORT << endl;

    if (MDNS.begin(OTA_NAME))
      MDNS.enableArduino(OTA_PORT, false); // no auth
    else
      _log << "mDNS Error !" << endl;
  }
}

void myWifi::socketUpdate()
{
  if (mIsSocket) 
  {
    mIsClientConnected = mClient.connected();

    if (!mIsClientConnected)
      mClient = mServer.available();
    else
      sendStripData();

    if (mIsClientConnected != mWasClientConnected)
    {
      _log << "Socket client " << (mIsClientConnected ? "connected" : "disconnected") << endl;
      mWasClientConnected = mIsClientConnected;
    }
  }
}

// ----------------------------------------------------
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
          _log << "Wifi connected, answser @ " << WiFi.localIP() << endl;
          digitalWrite(BUILTIN_LED, LOW); // led on

          socketInit();
          mON = true;
        }
      }
      else 
        stop();
    }
    else
      socketUpdate();
  }
  return mON;
}
