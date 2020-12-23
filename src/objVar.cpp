#include <ObjVar.h>

// ------------------------------
MyVar::MyVar(byte n, const char* name, SetFunc* set, GetFunc* get, int def, int min, int max, bool show) 
: mName(name), mSetF(set), mGetF(get), mMin(min), mMax(max), mShow(show) 
{
  assert(name!=nullptr);
  assert(strchr(name, ' ')==nullptr); // no space !

  if (n==1)
  {
    Args defaults;
    defaults[0] = def;
    this->set(defaults, 1, TrackChange::no);
  }
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
  Args cur; 
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
bool OBJVar::addVar(byte n, const char* name, SetFunc* set, GetFunc* get, int def, int min, int max, bool show)
{
  bool ok = mNVAR < MAX_VAR;
  if (ok)
  {
    MyVar* var = new MyVar(n, name, set, get, def, min, max, show);
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
