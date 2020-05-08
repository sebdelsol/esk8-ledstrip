#include <BTcmd.h>

BTcmd::BTcmd(Stream& btStream) : mBTStream(&btStream)
{
  mBTbuf.clear();
}  

void BTcmd::init(Stream& dbgSerial)
{
  mDbgSerial = &dbgSerial;
  
  *mDbgSerial << "mount SPIFFS" << endl;
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
    mOBJ[mNOBJ].obj = (OBJVar* )&obj;
    
    char* str = (char* )malloc(strlen(name) + 1);
    assert (str!=NULL);
    strcpy(str, name);
    mOBJ[mNOBJ++].name = str;

    // create IDs
    byte nbVar = obj.getNbVar();
    for (byte i = 0; i < nbVar; i++)
    {
      MyVar* var = obj.getVar(i);
      
      obj.setID(var, BTCMD_1ST_ID + mID);
      mID += 1;
    }
  }
  else
    *mDbgSerial << "----------- !!!!!!!!!! Max obj is reached " << BTCMD_MAXOBJ << endl; 

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

void BTcmd::dbgCmd(const char* cmdKeyword, const parsedCmd& parsed, int nbArg, int* args)
{
  *mDbgSerial << cmdKeyword << " " << parsed.objName << " " << parsed.varName;
  for (byte i=0; i < nbArg; i++) *mDbgSerial << " " << args[i];
  *mDbgSerial << endl;
}

//--------------------------------------
void BTcmd::handleSetCmd(const parsedCmd& parsed, BUF& buf, bool change)
{
  int min, max;
  parsed.obj->getMinMax(parsed.var, &min, &max);

  int args[MAX_ARGS];
  byte nbArg = 0;

  for (; nbArg < MAX_ARGS; nbArg++) // get the args
  {
    const char* a = buf.next();
    if (a!=NULL && isNumber(a))
      args[nbArg] = constrain(strtol(a, NULL, 10), min, max);
    else 
      break;
  }

  parsed.obj->set(parsed.var, args, nbArg, change); //set the value from args
  // dbgCmd(mSetKeyword, parsed , nbArg, args);
}

void BTcmd::handleGetCmd(const parsedCmd& parsed, Stream* stream, bool compact)
{
  int args[MAX_ARGS];
  byte nbArg = parsed.obj->get(parsed.var, args); //get the value in args

  if (nbArg) 
  { 
    if (compact)
      *stream << parsed.obj->getID(parsed.var);
    else
      *stream << mSetKeyword << " " << parsed.objName << " " << parsed.varName;

    for (byte i=0; i < nbArg; i++)
      *stream << " " << args[i];
  
    *stream << endl;
    // dbgCmd(mGetKeyword, parsed, nbArg, args);
  }
}

void BTcmd::handleInitCmd(const parsedCmd& parsed, Stream* stream)
{
  *stream << mInitKeyword << " " << parsed.objName << " " << parsed.varName << " " << parsed.obj->getID(parsed.var); 

  int min, max;
  parsed.obj->getMinMax(parsed.var, &min, &max);
  *stream << " " << min << " " << max;

  int args[MAX_ARGS];
  byte nbArg = parsed.obj->get(parsed.var, args); 

  for (byte i=0; i < nbArg; i++)
    *stream << " " << args[i];
  
  *stream << endl;
  // dbgCmd(mInitKeyword, parsed, nbArg, args);
}

//--------------------------------------
void BTcmd::handleCmd(Stream* stream, BUF& buf, bool change, bool compact)
{
  const char* cmd = buf.first();
  if (cmd!=NULL)
  {
    if (strcmp(cmd, "U")==0) // shortcut for update
    {
      snprintf(buf.getBuf(), buf.getLen(), "%s Cfg getUpdate", mSetKeyword); // emulate a set cmd
      cmd = buf.first();
    }

    parsedCmd parsed;

    parsed.objName = buf.next();
    if (parsed.objName != NULL)
    {
      parsed.obj = getObjFromName(parsed.objName);
      if(parsed.obj != NULL)
      {
        parsed.varName = buf.next();
        if (parsed.varName != NULL)
        { 
          parsed.var = parsed.obj->getVarFromName(parsed.varName);
          if (parsed.var != NULL)
          {
            // SET cmd ?
            if (strcmp(cmd, mSetKeyword)==0)
              handleSetCmd(parsed, buf, change); //read in buf and set the parsed var value
            
            // GET cmd ?
            else if (strcmp(cmd, mGetKeyword)==0)
              handleGetCmd(parsed, stream, compact); //write to stream the parsed var value           
            
            // INIT cmd ?
            else if (strcmp(cmd, mInitKeyword)==0) //write to stream the parsed var init           
              handleInitCmd(parsed, stream);            
          }
        }
      }
    }
  }
}

void BTcmd::readStream(Stream* stream, BUF& buf, bool change, bool compact)
{
  while (stream->available() > 0) 
  {
    char c = stream->read();
    if (c != BTCMD_ALIVE)
    {
      if (c == BTCMD_TERM)
      {
        handleCmd(stream, buf, change, compact);
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

void BTcmd::emulateCmdForAllVars(const char* cmdKeyword, Stream *stream, OBJVar::ObjTestVarFunc testVar, bool change, bool compact)
{
  for (byte i = 0; i < mNOBJ; i++)
  {
    char* objName = mOBJ[i].name;
    OBJVar* obj = mOBJ[i].obj;
    
    byte nbVar = obj->getNbVar();
    for (byte j = 0; j < nbVar; j++)
    {
      if(testVar == NULL || (obj->*testVar)(j))
      {
        char* varName = obj->getVarName(j);

        snprintf(mTmpBuf.getBuf(), mTmpBuf.getLen(), "%s %s %s", cmdKeyword, objName, varName); // emulate a cmd
        handleCmd(stream, mTmpBuf, change, compact); // the result of the cmd is sent to the stream
      }
    }
  } 
}

//--------------------------------------
File BTcmd::getFile(bool isdefault, const char* mode)
{	
  const char* fname = isdefault ? def_fname : cfg_fname;
  File f = spiffsOK ? SPIFFS.open(fname, mode) : File();
  bool isLoad = strcmp(mode, "r")==0;

  if (f)
    *mDbgSerial << (isLoad ? "loading from " : "saving to ") << fname << endl;
  else
    *mDbgSerial << "FAIL to " << (isLoad ? "load from " : "save to ") << fname << endl;
  
  return f;
}

void BTcmd::load(bool isdefault, bool change)
{
  File f = getFile(isdefault, "r");
  if (f)
  {
    mTmpBuf.clear(); // might not be cleared by readStream
    readStream((Stream* )&f, mTmpBuf, change); // should be a succession of set cmd
    f.close();
  }
}

void BTcmd::save(bool isdefault)
{
  File f = getFile(isdefault, "w");
  if (f)
  {
    emulateCmdForAllVars(mGetKeyword, (Stream*)&f); //for all vars, emulate a get cmd and send the result to mBTStream
    f.close();
  }
}

void BTcmd::sendUpdateOverBT()
{
  emulateCmdForAllVars(mGetKeyword, mBTStream, &OBJVar::hasVarChanged, true, true); //for all vars, emulate a get cmd and send the result to mBTStream
}

void BTcmd::sendInitsOverBT()
{
  emulateCmdForAllVars(mInitKeyword, mBTStream, &OBJVar::isVarShown); //for all vars, emulate a init cmd and send the result to mBTStream
  *mBTStream << "initdone" << endl;
}
