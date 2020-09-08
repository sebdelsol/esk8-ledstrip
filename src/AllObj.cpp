#include <AllObj.h>

//--------------------------------------
void AllObj::init()
{
  _log << "Mount SPIFFS" << endl;
  spiffsOK = SPIFFS.begin(true); // format if failed
  if (!spiffsOK)
    _log << "SPIFFS Mount Failed" << endl;

#ifdef DBG_SHOWFILES
  else
  {
    File root = SPIFFS.open("/");
    while(File file = root.openNextFile())
      _log << "FILE: \"" << file.name() << "\" - " << file.size() << " B" << endl;
  }
#endif
}

//--------------------------------------
bool AllObj::addObj(OBJVar& obj, const char* name)
{
  bool ok = mNOBJ < MAXOBJ;
  if (ok)
  {
    const char* objname = strdup(name);
    assert(objname != nullptr); 
    obj.setName(objname);

    mOBJS[mNOBJ++] = &obj;
    mHash.add(&obj);

    // create absolute IDs
    for(MyVar* var : obj)
      var->setID(CMD_1ST_ID + mID++);
  }
  else
    _log << ">> ERROR !! Max obj is reached " << MAXOBJ << endl; 

  return ok;
}

OBJVar* AllObj::getObjFromName(const char* name) 
{ 
  return name != nullptr ? mHash.get(name) : nullptr; 
};
  
//--------------------------------------
bool AllObj::isNumber(const char* txt) 
{ 
  for (int i = 0; i < strlen(txt); i++) 
    if (!(isdigit(txt[i]) || txt[i]=='-')) 
      return false; 

  return true; 
} 

void AllObj::dbgCmd(const char* cmdKeyword, const parsedCmd& parsed, int nbArg, int* args, bool line = true)
{
  #ifdef DBG_CMD
    _log << SpaceIt(cmdKeyword, parsed.obj->getName(), parsed.var->getName());
    
    for (byte i=0; i < nbArg; i++) 
      _log << " " << args[i];
    if (line) _log << endl;
  #endif
}

void AllObj::dbgCmd(const char* cmdKeyword, const parsedCmd& parsed, int nbArg, int* args, int min, int max)
{
  #ifdef DBG_CMD
    dbgCmd(cmdKeyword, parsed, nbArg, args, false);
    _log << " [" << min << "-" << max << "]" << endl;
  #endif
}

//--------------------------------------
// get the var args from the cmd
void AllObj::setCmd(const parsedCmd& parsed, BUF& buf, TrackChange trackChange)
{
  assert(trackChange != TrackChange::undefined);
  
  int min, max;
  parsed.var->getRange(min, max);

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

  parsed.var->set(args, nbArg, trackChange); //set the value from args
  
  dbgCmd(mSetKeyword, parsed , nbArg, args);
}

//----------------
// write the var with it args to the stream as a set cmd
void AllObj::getCmd(const parsedCmd& parsed, Stream& stream, Decode decode)
{
  assert(decode != Decode::undefined);

  int args[MAX_ARGS];
  byte nbArg = parsed.var->get(args); //get the value in args

  // remove pure cmd (no args)
  if (nbArg) 
  { 
    if (decode == Decode::compact)
      stream << parsed.var->getID();
    else
      stream << SpaceIt(mSetKeyword, parsed.obj->getName(), parsed.var->getName());

    for (byte i=0; i < nbArg; i++)
      stream << " " << args[i];
    stream << endl;
    
    dbgCmd(mGetKeyword, parsed, nbArg, args);
  }
}

//----------------
// write the var with it args + min/max to the stream as a int cmd
void AllObj::initCmd(const parsedCmd& parsed, Stream& stream)
{
  stream << SpaceIt(mInitKeyword, parsed.obj->getName(), parsed.var->getName(), parsed.var->getID()); 

  int min, max;
  parsed.var->getRange(min, max);
  stream << " " << min << " " << max;

  int args[MAX_ARGS];
  byte nbArg = parsed.var->get(args); 

  for (byte i=0; i < nbArg; i++)
    stream << " " << args[i];
  stream << endl;

  dbgCmd(mInitKeyword, parsed, nbArg, args, min, max);
}

//--------------------------------------
bool AllObj::parseCmd(parsedCmd& parsed, BUF& buf)
{
  const char* objname = buf.next();
  parsed.obj = getObjFromName(objname);
  
  if(parsed.obj != nullptr)
  {
    const char* varname = buf.next();
    parsed.var = parsed.obj->getVarFromName(varname); 
    
    if (parsed.var != nullptr)
      return true;
  }
  return false;
}

//----------------
void AllObj::handleCmd(Stream& stream, BUF& buf, TrackChange trackChange, Decode decode)
{
  const char* cmd = buf.first();
  if (cmd!=nullptr)
  {
    // shortcut for update ?
    if (strcmp(cmd, mUpdateShortcut)==0) 
    {
      snprintf(buf.getBuf(), buf.getLen(), "%s Cfg getUpdate", mSetKeyword); // emulate a set cmd
      cmd = buf.first();
    }

    parsedCmd parsed;
    
    if (parseCmd(parsed, buf))
    {
      // SET cmd ?
      if (strcmp(cmd, mSetKeyword)==0)
        setCmd(parsed, buf, trackChange); //read in buf and set the parsed var values
      
      // GET cmd ?
      else if (strcmp(cmd, mGetKeyword)==0)
        getCmd(parsed, stream, decode); //write to stream the parsed var values           
      
      // INIT cmd ?
      else if (strcmp(cmd, mInitKeyword)==0) //write to stream the parsed var inits           
        initCmd(parsed, stream);            
    }
  }
}

//----------------
void AllObj::readCmd(Stream& stream, BUF& buf, TrackChange trackChange, Decode decode)
{
  while (stream.available() > 0) 
  {
    char c = stream.read();
    if (c != CMD_ALIVE)
    {
      if (c == CMD_TERM)
      {
        handleCmd(stream, buf, trackChange, decode);
        buf.clear();
      }
      else if (isprint(c))
        buf.append(c);
    }
  }
}

//----------------
void AllObj::sendCmdForAllVars(const char* cmdKeyword, Stream& stream, TrackChange trackChange, Decode decode, MyVar::TestFunc testVar)
{
  for (OBJVar* obj : *this)
  {
    const char* objName = obj->getName();
    for(MyVar* var : *obj)
    {
      if(testVar == nullptr || (var->*testVar)())
      {
        // write a cmd in mTmpBuf
        snprintf(mTmpBuf.getBuf(), mTmpBuf.getLen(), "%s %s %s", cmdKeyword, objName, var->getName()); 
        // send the result of the cmd to the stream
        handleCmd(stream, mTmpBuf, trackChange, decode); 
      }
    }
  }
  mTmpBuf.clear(); 
}

//----------------
void AllObj::load(CfgType cfgtype, TrackChange trackChange)
{
  if (spiffsOK)
  {
    CfgFile f = CfgFile(cfgtype, FileMode::load);
    if (f.isOk())
      // should be a succession of set cmd
      readCmd(f.getStream(), mTmpBuf, trackChange, Decode::undefined); 
  }
}

//----------------
void AllObj::save(CfgType cfgtype)
{
  if (spiffsOK)
  {
    CfgFile f = CfgFile(cfgtype, FileMode::save);
    if (f.isOk())
      // for all vars, send a get cmd & output the result in the file stream
      sendCmdForAllVars(mGetKeyword, f.getStream(), TrackChange::undefined, Decode::verbose); 
  }
}
