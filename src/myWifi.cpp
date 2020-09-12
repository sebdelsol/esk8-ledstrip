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
  WiFi.begin(WIFINAME, WIFIPASS);
  
  mBegunOn = millis();
  mON = false;
  mWantON = true;

  _log << "Wifi start" << endl;
}

// ----------------------------------------------------
void myWifi::addStrip(const BaseLedStrip &strip)
{
  mIsSocket = true;

  if (mNStrips < MAXSTRIPS)
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
          _log << "Wifi Connected, answser @ " << WiFi.localIP() << endl;
          digitalWrite(BUILTIN_LED, LOW); // led on
          mON = true;

          if (mIsSocket)
          {
            bool mdns = MDNS.begin(MDNSNAME); 
            if (mdns) MDNS.addService(MDNSTYPE, MDNSPROT, SOCK_PORT);
            _log << (mdns ? "mDNS responder started" : "Error setting up MDNS responder!");
            _log << " for " << MDNSNAME << "." << MDNSTYPE << "." << MDNSPROT << ".local." << endl;

            mServer.begin();
            _log << "Socket server started" << endl;
          }
        }
      }
      else 
        stop();
    }
    
    else if (mIsSocket)
    {
      mServer.loop();
      mIsClientConnected = mServer.connectedClients() > 0;

      if (mIsClientConnected)
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

      if (mIsClientConnected != mWasClientConnected)
        _log << "Socket client " << (mIsClientConnected ? "connected" : "disconnected") << endl;
      mWasClientConnected = mIsClientConnected;
    }
  }

  return mON;
}
