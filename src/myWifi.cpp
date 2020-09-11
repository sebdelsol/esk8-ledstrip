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
          _log << "Wifi Connected @ " << WiFi.localIP() << endl;
          digitalWrite(BUILTIN_LED, LOW); // led on
          mON = true;

          if (mIsSocket)
          {
            mWSConnected = webSocket.connect(SOCK_ADDR, SOCK_PORT, "/");
            _log << "Socket client " << (mWSConnected ? "started" : "not started - run debugLedstrip.py & reboot the esp32") << endl;
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
        mWSConnected = mWSConnected ? webSocket.available() : false;

        if (mWSConnected)
        {
          for (byte i=0; i < mNStrips; i++)
          {
            int length = mStrips[i]->getRawLength();
            assert(length + 1 <= maxPayloadLength);

            payload[0] = i; //row to display
            memcpy(&payload[1], mStrips[i]->getRawData(), length);
            webSocket.sendBinary((const char*)payload, length + 1);
          }
        }
        // else
        // {
        //   EVERY_N_SECONDS(1)
        //   {
        //     _log << "try reconnect" << endl;
        //     // mWSConnected = webSocket.connect(SOCK_ADDR, SOCK_PORT, "/");          
        //     _log << "try reconnect done" << endl;
        //   }
        // }
      }
    }
  }

  return mON;
}
