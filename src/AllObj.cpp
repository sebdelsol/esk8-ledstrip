
#include <AllObj.h>

//--------------------------------------
bool AllObj::addObj(OBJVar& obj, const char* name)
{
  bool ok = mNOBJ < MAXOBJ;
  if (ok)
  {
    assert(name != nullptr); 
    assert(strchr(name, ' ')==nullptr); // no space !
    
    obj.setName(name); // string literals or already allocated

    mOBJS[mNOBJ++] = &obj;
    mHash.add(&obj);

    // create absolute IDs
    for (auto var : obj)
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
    // if (!(isxdigit(txt[i]) || txt[i]=='-')) 
      return false; 

  return true; 
} 

void AllObj::dbgCmd(const char* cmdKeyword, const parsedCmd& parsed, int nbArg, int* args, bool line = true)
{
  #ifdef DBG_CMD
    _log << SpaceIt(cmdKeyword, parsed.obj->getName(), parsed.var->getName());
    
    for (byte i=0; i < nbArg; i++) 
      // _log << " " << _HEX(args[i]);
      _log << " " << args[i]);
    if (line) _log << endl;
  #endif
}

void AllObj::dbgCmd(const char* cmdKeyword, const parsedCmd& parsed, int nbArg, int* args, int min, int max)
{
  #ifdef DBG_CMD
    dbgCmd(cmdKeyword, parsed, nbArg, args, false);
    // _log << " [" << _HEX(min) << "-" << _HEX(max) << "]" << endl;
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

  Args args;
  byte nbArg = 0;

  for (; nbArg < MAX_ARGS; nbArg++) // get the args
  {
    const char* a = buf.next();
    if (a!=nullptr && isNumber(a))
      // args[nbArg] = constrain(strtol(a, nullptr, 16), min, max);
      args[nbArg] = constrain(strtol(a, nullptr, 10), min, max);
    else 
      break;
  }

  parsed.var->set(args, nbArg, trackChange); //set the value from args
  
  dbgCmd(CMD_SET, parsed , nbArg, args);
}

//----------------
// write the var with it args to the stream as a set cmd
void AllObj::getCmd(const parsedCmd& parsed, Stream& stream, Decode decode)
{
  assert(decode != Decode::undefined);

  Args args;
  byte nbArg = parsed.var->get(args); //get the value in args

  // remove pure cmd (no args)
  if (nbArg) 
  { 
    if (decode == Decode::compact)
      stream << parsed.var->getID();
    else
      stream << SpaceIt(CMD_SET, parsed.obj->getName(), parsed.var->getName());

    for (byte i=0; i < nbArg; i++)
      stream << " " << args[i];
      // stream << " " << _HEX(args[i]);
    stream << endl;
    
    dbgCmd(CMD_GET, parsed, nbArg, args);
  }
}

//----------------
// write the var with it args + min/max to the stream as a int cmd
void AllObj::initCmd(const parsedCmd& parsed, Stream& stream)
{
  stream << SpaceIt(CMD_INIT, parsed.obj->getName(), parsed.var->getName(), parsed.var->getID()); 

  int min, max;
  parsed.var->getRange(min, max);
  // stream << " " << _HEX(min) << " " << _HEX(max);
  stream << " " << min << " " << max;

  Args args;
  byte nbArg = parsed.var->get(args); 

  for (byte i=0; i < nbArg; i++)
    // stream << " " << _HEX(args[i]);
    stream << " " << args[i];
  stream << endl;

  dbgCmd(CMD_INIT, parsed, nbArg, args, min, max);
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
    if (strcmp(cmd, CMD_UPDATE_SHORT)==0) 
    {
      snprintf(buf.getBuf(), buf.getLen(), "%s Cfg getUpdate", CMD_SET); // emulate a set cmd
      cmd = buf.first();
    }

    parsedCmd parsed;
    
    if (parseCmd(parsed, buf))
    {
      // SET cmd ?
      if (strcmp(cmd, CMD_SET)==0)
        setCmd(parsed, buf, trackChange); //read in buf and set the parsed var values
      
      // GET cmd ?
      else if (strcmp(cmd, CMD_GET)==0)
        getCmd(parsed, stream, decode); //write to stream the parsed var values           
      
      // INIT cmd ?
      else if (strcmp(cmd, CMD_INIT)==0) //write to stream the parsed var inits           
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
void AllObj::CmdAllVars(Stream& stream, const char* cmdKeyword, TrackChange trackChange, Decode decode, MyVar::TestFunc test)
{
  for (auto obj : *this)
  {
    const char* objName = obj->getName();
    for (auto var : *obj)
    {
      if(test == nullptr || (var->*test)())
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
  FileObjPtr cfg = getCfgFile(cfgtype, FileMode::load);
  if (cfg && cfg->ok())
    // should be a succession of set cmd
    readCmd(cfg->getStream(), mTmpBuf, trackChange, Decode::undefined); 
}

//----------------
void AllObj::save(CfgType cfgtype)
{
  FileObjPtr cfg = getCfgFile(cfgtype, FileMode::save);
  if (cfg && cfg->ok())
    // for all vars, send a get cmd & output the result in the file stream
    CmdAllVars(cfg->getStream(), CMD_GET, TrackChange::undefined, Decode::verbose); 
}
