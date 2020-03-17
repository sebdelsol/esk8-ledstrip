#include <objCmd.h>

bool OBJCmd::registerCmd(void *obj, const char *name, setCmdFunc set, getCmdFunc get, int min, int max)
{
  bool ok = mNCMD < MAX_CMD-1;
  if (ok) {
    mCmd[mNCMD].obj = obj;
    mCmd[mNCMD].set = set;
    mCmd[mNCMD].get = get;
    mCmd[mNCMD].min = min;
    mCmd[mNCMD].max = max;
    
    char* str = (char *)malloc(strlen(name) + 1);
    strcpy(str, name);
    mCmd[mNCMD++].name = str;
  }
  return ok;
}


MyCmd* OBJCmd::getCmd(const char* name)
{
  for (byte i = 0; i < mNCMD; i++) //look for the cmd
    if (strcmp(name, mCmd[i].name)==0)
      return &mCmd[i];

  return NULL;
}


void OBJCmd::getMinMax(MyCmd* cmd, int* min, int* max)
{
  *min = cmd->min;
  *max = cmd->max;
}

void OBJCmd::set(MyCmd* cmd, int* toSet, byte n)
{
    // debug
    // Serial << "set " << name;
    // for (byte k=0; k < n; k++)
    //   Serial << " " << toSet[k];
    // Serial << endl;
    (*cmd->set)(cmd->obj, toSet, n);
}

byte OBJCmd::get(MyCmd* cmd, int* toGet)
{
  return (*cmd->get)(cmd->obj, toGet);
}
