#include <ledserver.h>

void LedServer::addAllStrips(AllLedStrips &allStrip)
{
  mAllStrip = &allStrip;
}

// --------------
void LedServer::send()
{
  byte i = 0;
  byte bright = mAllStrip->getRawBrightness();
  for (auto strip : *mAllStrip)
  {
    int length = strip->getRawLength();
    mClient.write(length); 
    mClient.write(i++); 
    mClient.write(bright); 
    mClient.write(strip->getRawData(), length); 
  }
}

// --------------
void LedServer::begin()
{
  mServer.begin();

  // OTA_NAME & OTA_PORT are shared by OTA and the webSocket server, check platformio build_flags
  _log << "Socket server, answer @ " << OTA_NAME << ".local:" << OTA_PORT << endl;
  if (MDNS.begin(OTA_NAME))
    MDNS.enableArduino(OTA_PORT, false); // no auth
  else
    _log << "mDNS Error !" << endl;
  
  mHasBegun = true;
}

// --------------
bool LedServer::update()
{
  if (!mHasBegun)
    begin();
  
  mIsClientConnected = mClient.connected();

  if (!mIsClientConnected)
    mClient = mServer.accept(); // it disconnects an already connected client
  else
    send();

  if (mIsClientConnected != mWasClientConnected)
  {
    mWasClientConnected = mIsClientConnected;
    _log << "Socket client ";
    if (mIsClientConnected)
      _log << "connected @ " << mClient.remoteIP() << ":" << mClient.remotePort() << endl;
    else
      _log << "disconnected" << endl;
  }

  return mIsClientConnected;
}
