#include <ledserver.h>

void LedServer::addAllStrips(AllLedStrips& allStrip)
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
  if (MDNS.begin(OTA_NAME))
  {
    MDNS.enableArduino(OTA_PORT, false); // no auth
    _log << "Socket server, answer @ " << OTA_NAME << ".local:" << OTA_PORT << endl;
  }
  
  mHasBegun = true;
}

// --------------
bool LedServer::update()
{
  if (!mHasBegun) begin();
  
  mConnected = mClient.connected();

  if (!mConnected)
    mClient = mServer.accept(); // it disconnects an already connected client
  else
    send();

  if (mConnected != mWasConnected)
  {
    mWasConnected = mConnected;
    _log << "Socket client " << (mConnected ? "connected" : "disconnected") << endl;
  }

  return mConnected;
}
