#pragma once
#include <Arduino.h.>
#include <Streaming.h>

#define MAX_VAR 25
#define MAX_ARGS 3

//---------------------------------
typedef void (*setVarFunc)(void *obj, int* toSet, byte n);
typedef byte (*getVarFunc)(void *obj, int* toGet);

struct MyVar {
  char*       name;
  void*       obj;
  setVarFunc  set;
  getVarFunc  get;
  int         min;
  int         max;
  bool        show;
  byte        ID;
  int         last[MAX_ARGS];
};

//---------------------------------
class OBJVar
{
  MyVar *mVar[MAX_VAR];
  byte mNVAR = 0;

public:  
  bool registerVar(void *obj, const char *name, setVarFunc set, getVarFunc get, int min=0, int max=0, bool show=true);
  MyVar* getVarFromName(const char* name);
  MyVar* getVar(byte i) { return mVar[i]; };

  void set(MyVar* var, int* toSet, byte n, bool change = false);
  byte get(MyVar* var, int* toGet);
  void getMinMax(MyVar* var, int* min, int* max);

  byte getID(MyVar* var) { return var->ID; };
  void setID(MyVar* var, byte id) const { var->ID = id; };

  byte getNbVar() { return mNVAR;};
  char* getVarName(byte i) { return mVar[i]->name; };
  bool isVarShown(byte i) { return mVar[i]->show; };
  bool hasVarChanged(byte i);
};

//---------------------------------
#define _REGISTER_CMD(class, name, doCode, show) \
    registerVar(this, name, \
      [](void* obj, int* toSet, byte n) \
      { \
        if (n==0) \
        { \
          class* self = (class *)obj; \
          doCode; \
        } \
      }, \
      [](void* obj, int* toGet) -> byte { \
        return 0; \
      }, \
      0, 0, show \
    );

#define REGISTER_CMD(class, name, doCode)         _REGISTER_CMD(class, name, doCode, true)
#define REGISTER_CMD_NOSHOW(class, name, doCode)  _REGISTER_CMD(class, name, doCode, false)

#define _REGISTER_VAR(class, name, setCode, toGet0, min, max, show) \
    registerVar(this, name, \
      [](void* obj, int* toSet, byte n) \
      { \
        if (n==1) \
        { \
          class* self = (class *)obj; \
          int arg0 = toSet[0]; \
          setCode; \
        } \
      }, \
      [](void* obj, int* toGet) -> byte \
      { \
        class* self = (class *)obj; \
        toGet[0] = toGet0; \
        return 1; \
      }, \
      min, max, show \
    );

#define REGISTER_VAR(class, name, setCode, toGet0, min, max)        _REGISTER_VAR(class, name, setCode, toGet0, min, max, true) 
#define REGISTER_VAR_NOSHOW(class, name, setCode, toGet0, min, max) _REGISTER_VAR(class, name, setCode, toGet0, min, max, false) 

#define REGISTER_VAR_SIMPLE(class, name, var, min, max)             _REGISTER_VAR(class, name, { var = arg0; }, var, min, max, true) 
#define REGISTER_VAR_SIMPLE_NOSHOW(class, name, var, min, max)      _REGISTER_VAR(class, name, { var = arg0; }, var, min, max, false) 

#define REGISTER_VAR3(class, name, setCode, toGet0, toGet1, toGet2, min, max) \
    registerVar(this, name, \
      [](void* obj, int* toSet, byte n) \
      { \
        if (n==3) \
        { \
          class* self = (class *)obj; \
          int arg0 = toSet[0]; \
          int arg1 = toSet[1]; \
          int arg2 = toSet[2]; \
          setCode; \
        } \
      }, \
      [](void* obj, int* toGet) -> byte  \
      { \
        class* self = (class *)obj; \
        toGet[0] = toGet0; \
        toGet[1] = toGet1; \
        toGet[2] = toGet2; \
        return 3; \
      }, \
      min, max \
    );

