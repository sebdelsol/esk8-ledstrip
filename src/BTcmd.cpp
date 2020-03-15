#include <BTcmd.h>

BTcmd::BTcmd(Stream &stream) : mStream(&stream), mLast(NULL)
{
  strcpy(mDelim, BTCMD_DELIM);
  clearBuffer();
}


OBJCmd* BTcmd::getObjFromName(char* name)
{
  for (byte i = 0; i < mNOBJ; i++) //look for the obj
    if (strcmp(name, mOBJ[i].name)==0)
      return mOBJ[i].obj;
  
  return NULL;
}


bool BTcmd::registerObj(const OBJCmd& obj, char* name)
{
  bool ok = mNOBJ < BTCMD_MAXOBJ-1;
  if (ok) {
    mOBJ[mNOBJ].obj = (OBJCmd*)&obj;
    
    char* str = (char *)malloc(strlen(name) + 1);
    strcpy(str, name);
    mOBJ[mNOBJ++].name = str;
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
  char *cmd = first();
  if (cmd!=NULL) {
    
    char *objName = next();
    if (objName!=NULL) {

      OBJCmd* objCmd = getObjFromName(objName);
      if(objCmd!=NULL) {

        char *what = next();
        if (what!=NULL){ 

          #define BTCMD_MAXARGS 3
          byte args[BTCMD_MAXARGS];
          byte nbArg;

          for (nbArg = 0; nbArg < BTCMD_MAXARGS; nbArg++) {
            char *a = next();
            if (a==NULL) break;
            args[nbArg] = atoi(a);
          }

          if (strcmp(cmd, "set")==0)
              objCmd->set(what, args, nbArg);
              
          else if (strcmp(cmd, "get")==0) {
            nbArg = objCmd->get(what, args);
            
            if (nbArg) { // answer
              *mStream << objName << " " << what;
              for (byte i=0; i < nbArg; i++)
                *mStream << " " << args[i];
              *mStream << endl;
            }
          }
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
      //Serial << mBuf << endl;
    }
  }
}
