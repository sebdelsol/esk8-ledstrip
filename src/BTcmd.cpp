#include <BTcmd.h>

BTcmd::BTcmd(Stream &btStream) : mBTStream(&btStream)
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

OBJCmd* BTcmd::getObjFromName(const char* name)
{
  for (byte i = 0; i < mNOBJ; i++) //look for the obj
    if (strcmp(name, mOBJ[i].name)==0)
      return mOBJ[i].obj;
  
  return NULL;
}

bool BTcmd::registerObj(const OBJCmd& obj, const char* name)
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

void BTcmd::handleCmd(Stream* stream, BUF& buf)
{
  const char *cmd = buf.first();
  if (cmd!=NULL) {
    
    const char *objName = buf.next();
    if (objName!=NULL) {

      OBJCmd* objCmd = getObjFromName(objName);
      if(objCmd!=NULL) {

        const char *what = buf.next();
        if (what!=NULL){ 

          int args[BTCMD_MAXARGS];
          byte nbArg;

          for (nbArg = 0; nbArg < BTCMD_MAXARGS; nbArg++) {
            const char *a = buf.next();
            if (a==NULL) break;
            args[nbArg] = atoi(a);
          }

          if (strcmp(cmd, mSetKeyword)==0)
              objCmd->set(what, args, nbArg);

          else if (strcmp(cmd, mGetKeyword)==0) {
            nbArg = objCmd->get(what, args);
            
            if (nbArg) { // answer
              *stream << mSetKeyword << " " << objName << " " << what;
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

void BTcmd::readStream(Stream* stream, BUF& buf)
{
  while (stream->available() > 0) {

    char c = stream->read();

    if (c == BTCMD_TERM) {
      handleCmd(stream, buf);
      buf.clearBuffer();
    }
    else if (isprint(c)){
      buf.appendToBuffer(c);
      //Serial << mBuf << endl;
    }
  }
}

//--------------------------------------
const char* BTcmd::getFileName(bool isdefault)
{
  return spiffsOK ? (isdefault ? def_fname : cfg_fname) : NULL;
}

void BTcmd::save(bool isdefault)
{
  const char *fname = getFileName(isdefault);
  if (fname!=NULL) {
    File f = SPIFFS.open(fname, "w");
    if (f) {
      for (byte i = 0; i < mNOBJ; i++) {

        char* objName = mOBJ[i].name;
        OBJCmd* obj = mOBJ[i].obj;
        byte nbCmd = obj->getNbCmd();

        for (byte j = 0; j < nbCmd; j++) {
          char* varName = obj->getCmdName(j);
          snprintf(mFilebuf.getBuf(), mFilebuf.getLen(), "%s %s %s\n", mGetKeyword, objName, varName); // emulate a get cmd
          handleCmd((Stream*)&f, mFilebuf); // store it in the file 
        }
      } 
      f.close();
      Serial << "saved to " << fname << endl;
    }
  }
}

void BTcmd::load(bool isdefault)
{
  const char *fname = getFileName(isdefault);
  if (fname!=NULL) {
    File f = SPIFFS.open(fname, "r");
    if (f) {
      mFilebuf.clearBuffer(); // might not be cleared by readStream
      readStream((Stream*)&f, mFilebuf); // should be a succession of set cmd
      f.close();
      Serial << "loaded " << fname << endl;
    }
  }
}
