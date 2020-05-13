#include <AllObj.h>

AllObj::AllObj(Stream& dbgSerial) : mDbgSerial(dbgSerial) {}  

void AllObj::init()
{
  mDbgSerial << "mount SPIFFS" << endl;
  spiffsOK = SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED);
  if (!spiffsOK)
    mDbgSerial << "SPIFFS Mount Failed" << endl;
}

//--------------------------------------
OBJVar* AllObj::getObjFromName(const char* name)
{
  for (byte i = 0; i < mNOBJ; i++) //look for the obj
    if (strcmp(name, mOBJ[i].name)==0)
      return mOBJ[i].obj;
  
  return NULL;
}

//----------------
bool AllObj::registerObj(OBJVar& obj, const char* name)
{
  bool ok = mNOBJ < ALLOBJ_MAXOBJ;
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
      
      obj.setID(var, ALLOBJ_1ST_ID + mID);
      mID += 1;
    }
  }
  else
    mDbgSerial << ">> ERROR !! Max obj is reached " << ALLOBJ_MAXOBJ << endl; 

  return ok;
}

//--------------------------------------
bool AllObj::isNumber(const char* txt) 
{ 
  for (int i = 0; i < strlen(txt); i++) 
    if (!(isdigit(txt[i]) || txt[i]=='-')) 
      return false; 

  return true; 
} 

#ifdef DBG_CMD
  void AllObj::dbgCmd(const char* cmdKeyword, const parsedCmd& parsed, int nbArg, int* args)
  {
    mDbgSerial << cmdKeyword << " " << parsed.objName << " " << parsed.varName;
    for (byte i=0; i < nbArg; i++) 
      mDbgSerial << " " << args[i];
    mDbgSerial << endl;
  }
#endif

//--------------------------------------
void AllObj::handleSetCmd(const parsedCmd& parsed, BUF& buf, bool change)
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
  #ifdef DBG_CMD
    dbgCmd(mSetKeyword, parsed , nbArg, args);
  #endif
}

//----------------
void AllObj::handleGetCmd(const parsedCmd& parsed, Stream& stream, bool compact)
{
  int args[MAX_ARGS];
  byte nbArg = parsed.obj->get(parsed.var, args); //get the value in args

  if (nbArg) 
  { 
    if (compact)
      stream << parsed.obj->getID(parsed.var);
    else
      stream << mSetKeyword << " " << parsed.objName << " " << parsed.varName;

    for (byte i=0; i < nbArg; i++)
      stream << " " << args[i];
  
    stream << endl;
    #ifdef DBG_CMD
      dbgCmd(mGetKeyword, parsed, nbArg, args);
    #endif
  }
}

//----------------
void AllObj::handleInitCmd(const parsedCmd& parsed, Stream& stream)
{
  stream << mInitKeyword << " " << parsed.objName << " " << parsed.varName << " " << parsed.obj->getID(parsed.var); 

  int min, max;
  parsed.obj->getMinMax(parsed.var, &min, &max);
  stream << " " << min << " " << max;

  int args[MAX_ARGS];
  byte nbArg = parsed.obj->get(parsed.var, args); 

  for (byte i=0; i < nbArg; i++)
    stream << " " << args[i];
  
  stream << endl;
  #ifdef DBG_CMD
    dbgCmd(mInitKeyword, parsed, nbArg, args);
  #endif
}

//--------------------------------------
bool AllObj::getObjVar(parsedCmd& parsed, BUF& buf)
{
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
          return true;
      }
    }
  }
  return false;
}

//----------------
void AllObj::handleCmd(Stream& stream, BUF& buf, bool change, bool compact)
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
    
    if (getObjVar(parsed, buf))
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

//----------------
void AllObj::readCmdFromStream(Stream& stream, BUF& buf, bool change, bool compact)
{
  while (stream.available() > 0) 
  {
    char c = stream.read();
    if (c != ALLOBJ_ALIVE)
    {
      if (c == ALLOBJ_TERM)
      {
        handleCmd(stream, buf, change, compact);
        buf.clear();
      }
      else if (isprint(c))
        buf.append(c);
    }
  }
}

//----------------
void AllObj::emulateCmdForAllVars(const char* cmdKeyword, Stream& stream, OBJVar::ObjTestVarFunc testVar, bool change, bool compact)
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
File AllObj::getFile(bool isdefault, const char* mode)
{	
  const char* fname = isdefault ? def_fname : cfg_fname;
  File f = spiffsOK ? SPIFFS.open(fname, mode) : File();
  bool isLoading = strcmp(mode, "r")==0;

  if (f)
    mDbgSerial << (isLoading ? "loading from " : "saving to ") << fname << endl;
  else
    mDbgSerial << "FAIL to " << (isLoading ? "load from " : "save to ") << fname << endl;
  
  return f;
}

//----------------
void AllObj::load(bool isdefault, bool change)
{
  File f = getFile(isdefault, "r");
  if (f)
  {
    mTmpBuf.clear(); // might not be cleared by readCmdFromStream
    readCmdFromStream((Stream& )f, mTmpBuf, change); // should be a succession of set cmd
    f.close();
  }
}

//----------------
void AllObj::save(bool isdefault)
{
  File f = getFile(isdefault, "w");
  if (f)
  {
    emulateCmdForAllVars(mGetKeyword, (Stream& )f); //for all vars, emulate a get cmd and send the result to file stream
    f.close();
  }
}
