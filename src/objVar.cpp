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

void OBJVar::getMinMax(MyVar* cmd, int* min, int* max)
{
  *min = cmd->min;
  *max = cmd->max;
}

void OBJVar::set(MyVar* cmd, int* toSet, byte n)
{
  (*cmd->set)(cmd->obj, toSet, n);
}

byte OBJVar::get(MyVar* cmd, int* toGet)
{
  return (*cmd->get)(cmd->obj, toGet);
}
