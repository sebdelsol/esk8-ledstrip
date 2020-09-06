#include <ObjVar.h>

// ----------------------------------------------------
bool OBJVar::addVar(const char* name, SetFunc* set, GetFunc* get, int min, int max, bool show)
{
  bool ok = mNVAR < MAX_VAR;
  if (ok)
  {
    MyVar* var = new MyVar;
    assert (var!=nullptr);

    var->name = strdup(name);
    assert (var->name!=nullptr);

    var->set =  set;
    var->get =  get;
    var->min =  min;
    var->max =  max;
    var->show = show;

    mVar[mNVAR++] = var;
    mHash.add(var);
  }
  else
    _log << ">> ERROR !! Max var is reached " << MAX_VAR << endl; 
    
  return ok;
}

// ----------------------------------------------------
void OBJVar::getMinMax(const MyVar& var, int& min, int& max)
{
  min = var.min;
  max = var.max;
}

void OBJVar::set(MyVar& var, SetArgs toSet, byte n, TrackChange trackChange)
{
  (*var.set)(toSet, n); // toSet is not modified

  if (trackChange == TrackChange::no) // handled as nothing as changed... so
    get(var, var.last); // save new values in var.last
}

byte OBJVar::get(MyVar& var, GetArgs toGet)
{
  return (*var.get)(toGet); // toGet is modified
}

// ----------------------------------------------------
bool OBJVar::hasVarChanged(byte i)
{
  MyVar& var = *mVar[i]; 

  int cur[MAX_ARGS]; 
  byte n = get(var, cur); // get values in cur

  for (byte j=0; j < n; j++)
    if (cur[j] != var.last[j])
    {
      get(var, var.last); // save new values in var->last
      return true;
    } 
  return false;
}