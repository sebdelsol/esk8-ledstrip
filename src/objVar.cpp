#include <ObjVar.h>

// ----------------------------------------------------
bool OBJVar::addVar(const char* name, SetFunc* set, GetFunc* get, int min, int max, bool show)
{
  bool ok = mNVAR < MAX_VAR;
  if (ok)
  {
    // check the name already exists
    if (getVarFromName(name))
    {
      _log << ">> ERROR !! name already exists: " << name << endl; 
      return false;
    }

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

void OBJVar::set(MyVar& var, SetArgs toSet, byte n, bool change)
{
  (*var.set)(toSet, n); // toSet is not modified

  if (!change) // handled as nothing as changed... so
    get(var, var.last); //update value in var.last
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
  byte n = get(var, cur); //update value in cur

  for (byte k=0; k < n; k++)
    if (cur[k] != var.last[k])
    {
      get(var, var.last); //update value in var->last
      return true;
    } 
  return false;
}