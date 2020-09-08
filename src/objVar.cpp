#include <ObjVar.h>

// ------------------------------
MyVar::MyVar(const char* name, SetFunc* set, GetFunc* get, int min, int max, bool show)
{
  mName = strdup(name);
  assert (mName!=nullptr);

  mSetF = set;
  mGetF = get;
  mMin  = min;
  mMax  = max;
  mShow = show;
}

//----------------
void MyVar::getRange(int& min, int& max)
{
  min = mMin;
  max = mMax;
}

//----------------
void MyVar::set(SetArgs toSet, byte n, TrackChange trackChange)
{
  (*mSetF)(toSet, n); // toSet is read

  if (trackChange == TrackChange::no) // handled as nothing as changed... so
    get(mLast); // write new values in mLast
}

//----------------
byte MyVar::get(GetArgs toGet)
{
  return (*mGetF)(toGet); // write in toGet
}

//----------------
bool MyVar::hasChanged()
{
  int cur[MAX_ARGS]; 
  byte n = get(cur); // write current values in cur

  for (byte j=0; j < n; j++)
    if (cur[j] != mLast[j])
    {
      get(mLast); // write new values in mLast
      return true;
    } 
  return false;
}

// ----------------------------------------------------
bool OBJVar::addVar(const char* name, SetFunc* set, GetFunc* get, int min, int max, bool show)
{
  bool ok = mNVAR < MAX_VAR;
  if (ok)
  {
    MyVar* var = new MyVar(name, set, get, min, max, show);
    assert (var!=nullptr);

    mVar[mNVAR++] = var;
    mHash.add(var);
  }
  else
    _log << ">> ERROR !! Max var is reached " << MAX_VAR << endl; 
    
  return ok;
}

MyVar* OBJVar::getVarFromName(const char* name) 
{ 
  return name != nullptr ? mHash.get(name) : nullptr; 
};
