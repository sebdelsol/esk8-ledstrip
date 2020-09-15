#include <myWifi.h>

// ----------------------------------------------------
void myWifi::stop(const char* reason = nullptr)
{
  WiFi.mode(WIFI_OFF);
  digitalWrite(BUILTIN_LED, HIGH); // led off

  mWantON = false;
  mON = false;
  
  _log << "Wifi stopped";
  if (reason!=nullptr)
    _log << " because of " << reason;
  _log << endl;
}

void myWifi::start()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_ssid, WIFI_password);
  
  mBegunOn = millis();
  mON = false;
  mWantON = true;

  _log << "Wifi started" << endl;
}

bool myWifi::justConnected()
{
  if(millis() - mBegunOn < WIFI_TIMEOUT)
  {
    if(WiFi.status() == WL_CONNECTED)
    {
      _log << "Wifi connected, answer @ " << WiFi.localIP() << endl;
      digitalWrite(BUILTIN_LED, LOW); // led on
      mON = true;
      return true;
    }
  }
  else 
    stop("timeout");

  return false;
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
      if (!mClient) return;
      int length = mStrips[i]->getRawLength();
      mClient.write(length); 
      mClient.write(i); 
      mClient.write(mStrips[i]->getRawData(), length); 
    }
}

// --------------
void myWifi::socketInit()
{
  if (mIsSocket)
  {
    mServer.begin();

    // OTA_NAME & OTA_PORT are shared by OTA and the webSocket server, check platformio build_flags
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
      mClient = mServer.accept(); // it disconnects an already connected client
    else
      sendStripData();

    if (mIsClientConnected != mWasClientConnected)
    {
      mWasClientConnected = mIsClientConnected;
      _log << "Socket client ";
      if (mIsClientConnected)
        _log << "connected @ " << mClient.remoteIP() << ":" << mClient.remotePort() << endl;
      else
        _log << "disconnected" << endl;
    }
  }
}

// --------------
bool myWifi::update()
{
  if(mWantON)
  {
    if(!mON)
    {
      if (justConnected())
          socketInit();
    }
    else
      socketUpdate();
  }
  return mON;
}
