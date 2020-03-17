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

//--------------------------------------
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

//--------------------------------------
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
              objCmd->set(what, args, nbArg); //set the value from args

          else if (strcmp(cmd, mGetKeyword)==0) {
            nbArg = objCmd->get(what, args); //get the value in args

            if (nbArg) { // and answer on stream
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
      buf.clear();
    }
    else if (isprint(c)) {
      buf.append(c);
    }
  }
}

//--------------------------------------
File BTcmd::getFile(bool isdefault, const char *mode)
{	
  if (spiffsOK) {
    const char *fname = isdefault ? def_fname : cfg_fname;
    return SPIFFS.open(fname, mode);
  }
  return File();
}

void BTcmd::save(bool isdefault)
{
  File f = getFile(isdefault, "w");
  if (f) {
    for (byte i = 0; i < mNOBJ; i++) {

      char* objName = mOBJ[i].name;
      OBJCmd* obj = mOBJ[i].obj;
      byte nbCmd = obj->getNbCmd();

      for (byte j = 0; j < nbCmd; j++) {
        char* varName = obj->getCmdName(j);
        snprintf(mFilebuf.getBuf(), mFilebuf.getLen(), "%s %s %s", mGetKeyword, objName, varName); // emulate a get cmd
        handleCmd((Stream*)&f, mFilebuf); // set cmd stored in the file 
      }
    } 

    Serial << "saved to " << f.name() << endl;
    f.close();
  }
}

void BTcmd::load(bool isdefault)
{
  File f = getFile(isdefault, "r");
  if (f) {
    mFilebuf.clearBuffer(); // might not be cleared by readStream
    readStream((Stream*)&f, mFilebuf); // should be a succession of set cmd

    Serial << "loaded from " << f.name() << endl;
    f.close();
  }
}
