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
  {
    byte i = mNStrips++;
    mStrips[i] = (BaseLedStrip*)&strip;

    int length = mStrips[i]->getRawLength() + 1;
    if (length > maxPayloadLength)
    {
      payload = (byte*) (maxPayloadLength==0 ? malloc(length) : realloc(payload, length));
      maxPayloadLength = length;
    }
    assert(payload !=nullptr);
  }
}

void myWifi::sendStripData()
{
  for (byte i=0; i < mNStrips; i++)
  {
    int length = mStrips[i]->getRawLength();
    assert(length + 1 <= maxPayloadLength);

    payload[0] = i; //row to display
    memcpy(&payload[1], mStrips[i]->getRawData(), length);
    mServer.sendBIN(0, payload, length + 1);
  }
}

// --------------
void myWifi::socketInit()
{
  if (mIsSocket)
  {
    mServer.begin();
    _log << "Socket server, answer @ " << OTA_NAME << ".local:" << OTA_PORT;

    // OTA_NAME & OTA_PORT are shared by OTA and the webSocket server, check platformio build_flags
    bool mdns = MDNS.begin(OTA_NAME); 
    if (mdns) MDNS.enableArduino(OTA_PORT, false); // no auth
    _log << (mdns ? "" : " - mDNS Error") << endl;
  }
}

void myWifi::socketUpdate()
{
  if (mIsSocket)
  {
    mServer.loop();
    mIsClientConnected = mServer.connectedClients() > 0;

    if (mIsClientConnected != mWasClientConnected)
      _log << "Socket client " << (mIsClientConnected ? "connected" : "disconnected") << endl;
    mWasClientConnected = mIsClientConnected;

    if (mIsClientConnected) sendStripData();
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
          mON = true;

          socketInit();
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
