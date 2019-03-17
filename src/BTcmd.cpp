#include <BTcmd.h>

BTcmd::BTcmd(Stream &stream) : mStream(&stream), mLast(NULL)
{
  strcpy(mDelim, BTCMD_DELIM);
  clearBuffer();
  mCmd.answer = &stream;
}

bool BTcmd::registerFX(const FX& fx, char desc)
{
  bool ok = mNFX < BTCMD_MAXFX-1;
  if (ok) {
    mFX[mNFX].fx = (FX*)&fx;
    mFX[mNFX++].desc = desc;
  }
  return ok;
}

void BTcmd::clearBuffer()
{
  mBuf[0] = '\0';
  mBufPos = 0;
}

void BTcmd::appendToBuffer(char c)
{
  if (mBufPos < BTCMD_BUFF_SIZE) {
    mBuf[mBufPos++] = c;  // Put character into buffer
    mBuf[mBufPos] = '\0';      // Null terminate
  }
}

void BTcmd::handleCmd()
{
  // cmd are [SET or GET] [FX one char desc] [what one char] [byte args]
  mCmd.cmd = first();
  if (mCmd.cmd!=NULL) {

    char *fxStr = next();
    if (fxStr!=NULL && strlen(fxStr)==1) { // only 1 char

      mCmd.fx = *fxStr;

      for (int i = 0; i < mNFX; i++) { //look for the fx
        if (mCmd.fx==mFX[i].desc) {

          char *whatStr = next();
          if (whatStr!=NULL && strlen(whatStr)==1){ // only 1 char

            mCmd.what = *whatStr;
            for (mCmd.nbArg = 0; mCmd.nbArg < BTCMD_MAXARGS; mCmd.nbArg++) {
              char *a = next();
              if (a==NULL) break;
              mCmd.arg[mCmd.nbArg] = atoi(a);
            }

            Serial << "Receive " << mCmd.cmd << " " << mCmd.fx << " " << mCmd.what << " ";
            for (byte i=0; i<mCmd.nbArg; i++)
              Serial << i << ":" << mCmd.arg[i] << " ";
            Serial << endl;

            if (strcmp(mCmd.cmd, "SET")==0){
              if (mCmd.nbArg)
                mFX[i].fx->setCmd(mCmd);
            }
            else if (strcmp(mCmd.cmd, "GET")==0)
              mFX[i].fx->getCmd(mCmd);
          }
          break;
        }
      }
    }
  }
}

void BTcmd::readStream()
{
  while (mStream->available() > 0) {

    char c = mStream->read();

    if (c == BTCMD_TERM) {
      handleCmd();
      clearBuffer();
    }
    else if (isprint(c)){
      appendToBuffer(c);
      // Serial << mBuf << endl;
    }
  }
}
