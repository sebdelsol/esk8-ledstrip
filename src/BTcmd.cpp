#include <BTcmd.h>

BTcmd::BTcmd(Stream &btStream) : mBTStream(&btStream), mLast(NULL)
{
  mBTbuf.clearBuffer();
  
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
/*
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
*/

void BTcmd::handleCmd(Stream* stream,BUFbase &buf)
{
  const char *cmd = first(buf.getBuf());
  if (cmd!=NULL) {
    
    char *objName = next();
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

          if (strcmp(cmd, mSetKeyword)==0)
              objCmd->set(what, args, nbArg);

          else if (strcmp(cmd, mGetKeyword)==0) {
            nbArg = objCmd->get(what, args);
            
            if (nbArg) { // answer
              *stream << objName << " " << what;
              for (byte i=0; i < nbArg; i++)
                *stream << " " << args[i];
              *stream << endl;
            }
          }
        }
      }
    }
  }
}

void BTcmd::readStream(Stream* stream, BUFbase &buf)
{
  while (stream->available() > 0) {

    char c = stream->read();

    if (c == BTCMD_TERM) {
      handleCmd(stream, buf.getBuf());
      buf.clearBuffer();
    }
    else if (isprint(c)){
      buf.appendToBuffer(c);
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
// f << mSetKeyword << " " << readstream(f);
          nbArg = obj->get(varName, args);
          if (nbArg) {
            f.print(mSetKeyword);
            f.print(" ");
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
      mFilebuf.clearBuffer(); // might not be cleared by readStream
      readStream(f, mFilebuf);
       
      /*
      while (f.available()) {
        char *buf = (char *)f.readStringUntil('\n').c_str(); 
        handleCmd(buf, true); // NO IT USES STRTOK that changes the string and c_str is const char* !
      }*/
      f.close();
      Serial << "loaded " << fname << endl;
    }
  }
}
