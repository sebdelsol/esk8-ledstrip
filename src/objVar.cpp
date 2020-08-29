#include <ObjVar.h>

// ----------------------------------------------------
bool OBJVar::registerVar(const char* name, SetFunc* set, GetFunc* get, int min, int max, bool show)
{
  bool ok = mNVAR < MAX_VAR;
  if (ok)
  {
    // check the name already exists
    if (getVarFromName(name))
    {
      Serial << ">> ERROR !! name already exists: " << name << endl; 
      return false;
    }

    MyVar* var = (MyVar* )malloc(sizeof(MyVar));
    assert (var!=nullptr);

    mHash.add(name, var);
    mVar[mNVAR++] = var;

    var->name = (char* )malloc(strlen(name) + 1);
    assert (var->name!=nullptr);
    strcpy(var->name, name);

    var->set =  set;
    var->get =  get;
    var->min =  min;
    var->max =  max;
    var->show = show;
  }
  else
    Serial << ">> ERROR !! Max var is reached " << MAX_VAR << endl; 
    
  return ok;
}

// ----------------------------------------------------
void OBJVar::getMinMax(MyVar* var, int* min, int* max)
{
  *min = var->min;
  *max = var->max;
}

void OBJVar::set(MyVar* var, int* toSet, byte n, bool change)
{
  (*var->set)(toSet, n);

  if (!change) // handled as nothing as changed... so
    get(var, var->last); //update value in var->last
}

byte OBJVar::get(MyVar* var, int* toGet)
{
  return (*var->get)(toGet);
}

// ----------------------------------------------------
bool OBJVar::hasVarChanged(byte i)
{
  MyVar *var = mVar[i]; 

  int cur[MAX_ARGS]; 
  byte n = get(var, cur); //update value in cur

  for (byte k=0; k < n; k++)
    if (cur[k] != var->last[k])
    {
      get(var, var->last); //update value in var->last
      return true;
    } 
  return false;
}