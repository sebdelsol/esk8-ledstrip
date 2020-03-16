#include <BTcmd.h>

BTcmd::BTcmd(Stream &stream) : mStream(&stream), mLast(NULL)
{
  strcpy(mDelim, BTCMD_DELIM);
  clearBuffer();
}  

void BTcmd::initSPIFFS()
{
  Serial << "SPIFFS begin" << endl;
  spiffsOK = SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED);
  if (!spiffsOK)
    Serial << "SPIFFS Mount Failed" << endl;
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

void BTcmd::handleCmd(char* buf, bool autoSet)
{
  const char *cmd = autoSet ? "set" : first(buf);
  if (cmd!=NULL) {
    
    char *objName = autoSet ? first(buf): next();
    if (objName!=NULL) {

      OBJCmd* objCmd = getObjFromName(objName);
      if(objCmd!=NULL) {

        char *what = next();
        if (what!=NULL){ 

          int args[BTCMD_MAXARGS];
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
      handleCmd(mBuf);
      clearBuffer();
    }
    else if (isprint(c)){
      appendToBuffer(c);
      //Serial << mBuf << endl;
    }
  }
}

//--------------------------------------
void BTcmd::save(bool isdefault)
{
  if (spiffsOK) {
    const char *fname = isdefault ? def_fname : cfg_fname;
    File f = SPIFFS.open(fname, "w");
    if (f) {

      int args[BTCMD_MAXARGS];
      byte nbArg;

      for (byte i = 0; i < mNOBJ; i++) {

        char* objName = mOBJ[i].name;
        OBJCmd* obj = mOBJ[i].obj;

        byte nbCmd = obj->getNbCmd();
        for (byte j = 0; j < nbCmd; j++) {

          char* varName = obj->getCmdName(j);

          nbArg = obj->get(varName, args);
          if (nbArg) {
            f.print(objName); 
            f.print(" "); 
            f.print(varName);
            
            for (byte k=0; k < nbArg; k++) {
              f.print(" "); 
              f.print(args[k]);
            }
            f.println();
          }
        }
      } 
      f.close();
      Serial << "saved to " << fname << endl;
    }
  }
}

void BTcmd::load(bool isdefault)
{
  if (spiffsOK) {
    const char *fname = isdefault ? def_fname : cfg_fname;
    File f = SPIFFS.open(fname, "r");
    if (f) {
      while (f.available()){ // !!!!!!!!!!! readStringUntil uses String !!!!!!!!!
        char *buf = (char *)f.readStringUntil('\n').c_str(); 
        handleCmd(buf, true); // autoset
      }
      f.close();
      Serial << "loaded " << fname << endl;
    }
  }
}
