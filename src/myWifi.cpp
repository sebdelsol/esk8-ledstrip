#include <myWifi.h>

//------------------------------------------------------------
myWifi* CurrentMyWifi;

void CallbackWrapper(WStype_t type, uint8_t * payload, size_t length)
{
  CurrentMyWifi->onWSEvent(type, payload, length);
}

void myWifi::onWSEvent(WStype_t type, uint8_t * payload, size_t length)
{
  switch(type)
  {
    case WStype_DISCONNECTED:
      mWSConnected = false;
      _log << "Socket client Disconnected" << endl;
      break;
    case WStype_CONNECTED:
      mWSConnected = true;
      _log << "Socket client Connected" << endl;
      break;
  }
}

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
    if (maxTosend==0) tosend = (byte*) malloc(length);
    else if (length > maxTosend) tosend = (byte*) realloc(tosend, length);
    assert(tosend !=nullptr);
    maxTosend = length;
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
          _log << "Wifi Connected @ " << WiFi.localIP() << endl;
          digitalWrite(BUILTIN_LED, LOW); // led on
          mON = true;

          if (mIsSocket)
          {
            _log << "Socket client Started" << endl;
            CurrentMyWifi = this;
            webSocket.onEvent(CallbackWrapper);
            webSocket.begin(SOCK_ADDR, SOCK_PORT);
          }
        }
      }
      else 
        stop();
    }
    
    else
    {
      if (mIsSocket)
      {
        webSocket.loop();

        if (mWSConnected)
          for (byte i=0; i < mNStrips; i++)
          {
            tosend[0] = i; //row to display
            int length = mStrips[i]->getRawLength();
            memcpy(&tosend[1], mStrips[i]->getRawData(), length);

            webSocket.sendBIN(tosend, length + 1);
          }
      }
    }
  }

  return mON;
}
