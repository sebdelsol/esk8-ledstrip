#include <objCmd.h>

bool OBJCmd::registerCmd(void *obj, char *name, setFunc set, getFunc get)
{
  bool ok = mNCMD < MAX_CMD-1;
  if (ok) {
    mCmd[mNCMD].obj = obj;
    mCmd[mNCMD].set = set;
    mCmd[mNCMD].get = get;
    
    char* str = (char *)malloc(strlen(name) + 1);
    strcpy(str, name);
    mCmd[mNCMD++].name = str;
  }
  return ok;
}

void OBJCmd::set(char* name, int* toSet, byte n)
{
  for (byte i = 0; i < mNCMD; i++) //look for the cmd
    if (strcmp(name, mCmd[i].name)==0) {

      // debug
      // Serial << "set " << name;
      // for (byte k=0; k < n; k++)
      //   Serial << " " << toSet[k];
      // Serial << endl;

      (*mCmd[i].set)(mCmd[i].obj, toSet, n);
    }
}

byte OBJCmd::get(char* name, int* toGet)
{
  for (byte i = 0; i < mNCMD; i++) //look for the cmd
    if (strcmp(name, mCmd[i].name)==0)
      return (*mCmd[i].get)(mCmd[i].obj, toGet);
  return 0;
}
