#include <BTcmd.h>

BTcmd::BTcmd(Stream &btStream) : mBTStream(&btStream)
{
  mBTbuf.clear();
}  

void BTcmd::init(Stream &dbgSerial)
{
  mDbgSerial = &dbgSerial;
  initSPIFFS();
}

void BTcmd::initSPIFFS()
{
  *mDbgSerial << "SPIFFS begin" << endl;
  spiffsOK = SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED);
  if (!spiffsOK)
    *mDbgSerial << "SPIFFS Mount Failed" << endl;
}

//--------------------------------------
OBJVar* BTcmd::getObjFromName(const char* name)
{
  for (byte i = 0; i < mNOBJ; i++) //look for the obj
    if (strcmp(name, mOBJ[i].name)==0)
      return mOBJ[i].obj;
  
  return NULL;
}

bool BTcmd::registerObj(OBJVar& obj, const char* name)
{
  bool ok = mNOBJ < BTCMD_MAXOBJ;
  if (ok)
  {
    mOBJ[mNOBJ].obj = (OBJVar*)&obj;
    
    char* str = (char *)malloc(strlen(name) + 1);
    strcpy(str, name);
    mOBJ[mNOBJ++].name = str;

    // create IDs
    byte nbVar = obj.getNbVar();
    for (byte i = 0; i < nbVar; i++)
    {
      MyVar* var = obj.getVar(i);
      
      // do not use BTCMD_ALIVE as id set chararacter
      #define FIRST_VAR '!'
      if (FIRST_VAR + mID == BTCMD_ALIVE) mID += 1;

      obj.setID(var, FIRST_VAR+ mID);
      // *mDbgSerial << mID << " - " << (char)(FIRST_VAR+ mID) << endl;
      mID += 1;
    }
  }
  return ok;
}

//--------------------------------------
bool BTcmd::isNumber(const char* txt) 
{ 
  for (int i = 0; i < strlen(txt); i++) 
    if (!(isdigit(txt[i]) || txt[i]=='-')) 
      return false; 

  return true; 
} 

void BTcmd::dbgCmd(const char *cmd, const char *objName, const char *varName, int nbArg, int *args)
{
  *mDbgSerial << cmd << " " << objName << " " << varName;
  for (byte i=0; i < nbArg; i++) *mDbgSerial << " " << args[i];
  *mDbgSerial << endl;
}

void BTcmd::handleCmd(Stream* stream, BUF& buf, bool change, bool useShortCut)
{
  const char *cmd = buf.first();
  if (cmd!=NULL)
  {
    if (strcmp(cmd, "U")==0) // shortcut
    {
      snprintf(buf.getBuf(), buf.getLen(), "%s Cfg getUpdate", mSetKeyword); // emulate a set cmd
      cmd = buf.first();
    }

    const char *objName = buf.next();
    if (objName!=NULL)
    {
      OBJVar* obj = getObjFromName(objName);
      if(obj!=NULL)
      {
        const char *varName = buf.next();
        if (varName!=NULL)
        { 
          MyVar* var = obj->getVarFromName(varName);
          if (var)
          {
            int min=0, max=0;
            int args[MAX_ARGS];
            byte nbArg;

            // SET cmd ?
            if (strcmp(cmd, mSetKeyword)==0)
            {
              obj->getMinMax(var, &min, &max);

              for (nbArg = 0; nbArg < MAX_ARGS; nbArg++) // get the args
              {
                const char *a = buf.next();
                if (a!=NULL && isNumber(a))
                  args[nbArg] = constrain(strtol(a, NULL, 10), min, max);
                else 
                  break;
              }

              obj->set(var, args, nbArg, change); //set the value from args
              // dbgCmd(mSetKeyword, objName, varName, nbArg, args);
            }
            
            // GET cmd ?
            else if (strcmp(cmd, mGetKeyword)==0)
            {
              // answer on stream
              nbArg = obj->get(var, args); //get the value in args
              if (nbArg) 
              { 
                if (useShortCut)
                  *stream << obj->getID(var);
                else
                  *stream << mSetKeyword << " " << objName << " " << varName;

                for (byte i=0; i < nbArg; i++)
                  *stream << " " << args[i];
              
                *stream << endl;
                // dbgCmd(mGetKeyword, objName, varName, nbArg, args);
              }
            }
            
            // INIT cmd ?
            else if (strcmp(cmd, mInitKeyword)==0)
            {
              // answer on stream
              *stream << mInitKeyword << " " << objName << " " << varName << " " << obj->getID(var); 

              obj->getMinMax(var, &min, &max);
              *stream << " " << min << " " << max;

              nbArg = obj->get(var, args); 
              for (byte i=0; i < nbArg; i++)
                *stream << " " << args[i];
              
              *stream << endl;
              // dbgCmd(mInitKeyword, objName, varName, nbArg, args);
            }

          }
        }
      }
    }
  }
}

void BTcmd::readStream(Stream* stream, BUF& buf, bool change, bool useShortCut)
{
  while (stream->available() > 0) 
  {
    char c = stream->read();
    if (c != BTCMD_ALIVE)
    {
      if (c == BTCMD_TERM)
      {
        handleCmd(stream, buf, change, useShortCut);
        buf.clear();
      }
      else if (isprint(c))
      {
        buf.append(c);
        // *mDbgSerial << "Buf >>>" << buf.getBuf() << "<<<" << endl;
      }
    }
  }
}

//--------------------------------------
File BTcmd::getFile(bool isdefault, const char *mode)
{	
  if (spiffsOK)
  {
    const char *fname = isdefault ? def_fname : cfg_fname;
    return SPIFFS.open(fname, mode);
  }
  return File();
}

void BTcmd::save(bool isdefault)
{
  File f = getFile(isdefault, "w");
  if (f)
  {
    for (byte i = 0; i < mNOBJ; i++)
    {
      char* objName = mOBJ[i].name;
      OBJVar* obj = mOBJ[i].obj;
      byte nbVar = obj->getNbVar();
      for (byte j = 0; j < nbVar; j++)
      {
        char* varName = obj->getVarName(j);
        snprintf(mFilebuf.getBuf(), mFilebuf.getLen(), "%s %s %s", mGetKeyword, objName, varName); // emulate a get cmd
        handleCmd((Stream*)&f, mFilebuf); // set cmd stored in the file 
      }
    } 

    *mDbgSerial << "saved to " << f.name() << endl;
    f.close();
  }
  else    
    *mDbgSerial << "FAIL save" << endl;
}

void BTcmd::load(bool isdefault, bool change)
{
  File f = getFile(isdefault, "r");
  if (f)
  {
    mFilebuf.clear(); // might not be cleared by readStream
    readStream((Stream*)&f, mFilebuf, change); // should be a succession of set cmd

    *mDbgSerial << "loaded from " << f.name() << endl;
    f.close();
  }
  else    
    *mDbgSerial << "FAIL load" << endl;

}

void BTcmd::sendUpdateOverBT()
{
  for (byte i = 0; i < mNOBJ; i++)
  {
    char* objName = mOBJ[i].name;
    OBJVar* obj = mOBJ[i].obj;
    byte nbVar = obj->getNbVar();
    for (byte j = 0; j < nbVar; j++)
    {
      if (obj->hasVarChanged(j))
      {
        char* varName = obj->getVarName(j);
        snprintf(mFilebuf.getBuf(), mFilebuf.getLen(), "%s %s %s", mGetKeyword, objName, varName); // emulate a Get cmd
        handleCmd(mBTStream, mFilebuf, true, true); // answer with a Set cmd on BT 
      }
    }
  } 
}

void BTcmd::sendInitsOverBT()
{
  for (byte i = 0; i < mNOBJ; i++)
  {
    char* objName = mOBJ[i].name;
    OBJVar* obj = mOBJ[i].obj;
    byte nbVar = obj->getNbVar();
    for (byte j = 0; j < nbVar; j++)
    {
      if(obj->isVarShown(j))
      {
        char* varName = obj->getVarName(j);
        snprintf(mFilebuf.getBuf(), mFilebuf.getLen(), "%s %s %s", mInitKeyword, objName, varName); // emulate a Init cmd
        handleCmd(mBTStream, mFilebuf); // answer with a Init cmd on BT
      }
    }
  } 
  *mBTStream << "initdone" << endl;
}
