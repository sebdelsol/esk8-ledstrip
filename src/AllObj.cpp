#include <AllObj.h>

//--------------------------------------
void AllObj::init()
{
  _log << "Mount SPIFFS" << endl;
  spiffsOK = SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED);
  if (!spiffsOK)
    _log << "SPIFFS Mount Failed" << endl;
}

//--------------------------------------
bool AllObj::addObj(OBJVar& obj, const char* name)
{
  bool ok = mNOBJ < ALLOBJ_MAXOBJ;
  if (ok)
  {
    char* namecpy = strdup(name);
    assert (namecpy!=nullptr);

    mOBJ& mobj = mOBJS[mNOBJ++];
    mobj.name = namecpy;
    mobj.obj = &obj;
    mHash.add(&mobj);

    // create IDs
    byte nbVar = obj.getNbVar();
    for (byte i = 0; i < nbVar; i++)
    {
      MyVar* var = obj.getVar(i);
      
      obj.setID(*var, ALLOBJ_1ST_ID + mID);
      mID += 1;
    }
  }
  else
    _log << ">> ERROR !! Max obj is reached " << ALLOBJ_MAXOBJ << endl; 

  return ok;
}

//--------------------------------------
OBJVar* AllObj::getObjFromName(const char* name)
{ 
  mOBJ* mobj = mHash.get(name); 
  return mobj!=nullptr ? mobj->obj : nullptr; 
}

//--------------------------------------
bool AllObj::isNumber(const char* txt) 
{ 
  for (int i = 0; i < strlen(txt); i++) 
    if (!(isdigit(txt[i]) || txt[i]=='-')) 
      return false; 

  return true; 
} 

void AllObj::dbgCmd(const char* cmdKeyword, const parsedCmd& parsed, int nbArg, int* args)
{
  #ifdef DBG_CMD
    JoinbySpace(_log, cmdKeyword, parsed.objName, parsed.varName);
    
    for (byte i=0; i < nbArg; i++) 
      _log << " " << args[i];
    _log << endl;
  #endif
}

//--------------------------------------
void AllObj::handleSetCmd(const parsedCmd& parsed, BUF& buf, bool change)
{
  int min, max;
  parsed.obj->getMinMax(*parsed.var, min, max);

  int args[MAX_ARGS];
  byte nbArg = 0;

  for (; nbArg < MAX_ARGS; nbArg++) // get the args
  {
    const char* a = buf.next();
    if (a!=nullptr && isNumber(a))
      args[nbArg] = constrain(strtol(a, nullptr, 10), min, max);
    else 
      break;
  }

  parsed.obj->set(*parsed.var, args, nbArg, change); //set the value from args
  
  dbgCmd(mSetKeyword, parsed , nbArg, args);
}

//----------------
void AllObj::handleGetCmd(const parsedCmd& parsed, Stream& stream, bool compact)
{
  int args[MAX_ARGS];
  byte nbArg = parsed.obj->get(*parsed.var, args); //get the value in args

  if (nbArg) 
  { 
    if (compact)
      stream << parsed.obj->getID(*parsed.var);
    else
      JoinbySpace(stream, mSetKeyword, parsed.objName, parsed.varName);

    for (byte i=0; i < nbArg; i++)
      stream << " " << args[i];
    stream << endl;
    
    dbgCmd(mGetKeyword, parsed, nbArg, args);
  }
}

//----------------
void AllObj::handleInitCmd(const parsedCmd& parsed, Stream& stream)
{
  JoinbySpace(stream, mInitKeyword, parsed.objName, parsed.varName, parsed.obj->getID(*parsed.var)); 

  int min, max;
  parsed.obj->getMinMax(*parsed.var, min, max);
  stream << " " << min << " " << max;

  int args[MAX_ARGS];
  byte nbArg = parsed.obj->get(*parsed.var, args); 

  for (byte i=0; i < nbArg; i++)
    stream << " " << args[i];
  stream << endl;

  dbgCmd(mInitKeyword, parsed, nbArg, args);
}

//--------------------------------------
bool AllObj::getObjVar(parsedCmd& parsed, BUF& buf)
{
  parsed.objName = buf.next();
  if (parsed.objName != nullptr)
  {
    parsed.obj = getObjFromName(parsed.objName);
    if(parsed.obj != nullptr)
    {
      parsed.varName = buf.next();
      if (parsed.varName != nullptr)
      { 
        parsed.var = parsed.obj->getVarFromName(parsed.varName);
        if (parsed.var != nullptr)
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
  if (cmd!=nullptr)
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
    char* objName = mOBJS[i].name;
    OBJVar* obj = mOBJS[i].obj;
    
    byte nbVar = obj->getNbVar();
    for (byte j = 0; j < nbVar; j++)
    {
      if(testVar == nullptr || (obj->*testVar)(j))
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
    _log << (isLoading ? "Loading from " : "Saving to ") << fname << "...";
  else
    _log << "FAIL to " << (isLoading ? "load from " : "save to ") << fname;
  
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
    _log << "loaded";
  }
  _log << endl;
}

//----------------
void AllObj::save(bool isdefault)
{
  File f = getFile(isdefault, "w");
  if (f)
  {
    emulateCmdForAllVars(mGetKeyword, (Stream& )f); //for all vars, emulate a get cmd and send the result to file stream
    f.close();
    _log << "saved";
  }
  _log << endl;
}
