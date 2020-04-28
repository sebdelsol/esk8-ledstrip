#include <objVar.h>

bool OBJVar::registerVar(void *obj, const char *name, setVarFunc set, getVarFunc get, int min, int max, bool show)
{
  bool ok = mNVAR < MAX_VAR-1;
  if (ok)
  {
    mVar[mNVAR].obj = obj;
    mVar[mNVAR].set = set;
    mVar[mNVAR].get = get;
    mVar[mNVAR].min = min;
    mVar[mNVAR].max = max;
    mVar[mNVAR].show = show;
    
    char* str = (char *)malloc(strlen(name) + 1);
    strcpy(str, name);
    mVar[mNVAR++].name = str;
  }
  return ok;
}

MyVar* OBJVar::getVarFromName(const char* name)
{
  for (byte i = 0; i < mNVAR; i++) //look for the cmd
    if (strcmp(name, mVar[i].name)==0)
      return &mVar[i];

  return NULL;
}

void OBJVar::getMinMax(MyVar* var, int* min, int* max)
{
  *min = var->min;
  *max = var->max;
}

void OBJVar::set(MyVar* var, int* toSet, byte n, bool change)
{
  (*var->set)(var->obj, toSet, n);

  if (!change) // handled as nothing as changed... so
    get(var, var->last); //update value in var->last
}

byte OBJVar::get(MyVar* var, int* toGet)
{
  return (*var->get)(var->obj, toGet);
}

bool OBJVar::hasVarChanged(byte i)
{
  MyVar *var = &mVar[i]; 

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