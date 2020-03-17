#pragma once
#include <Arduino.h.>
#include <Streaming.h>

#define MAX_VAR 17

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
};

//---------------------------------
class OBJVar
{

  MyVar mVar[MAX_VAR];
  byte mNVAR = 0;

public:  
  bool registerVar(void *obj, const char *name, setVarFunc set, getVarFunc get, int min=0, int max=0);
  MyVar* getVarFromName(const char* name);

  void set(MyVar* cmd, int* toSet, byte n);
  byte get(MyVar* cmd, int* toGet);
  void getMinMax(MyVar* cmd, int* min, int* max);

  byte getNbVar() { return mNVAR;};
  char* getVarName(byte i) { return mVar[i].name;};
};

//---------------------------------
#define REGISTER_CMD_PURE(class, name, doCode) \
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
      } \
    );

#define REGISTER_CMD(class, name, setCode, toGet0, min, max) \
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
      min, max \
    );

#define REGISTER_CMD_SIMPLE(class, name, var, min, max) REGISTER_CMD(class, name, { var = arg0; }, var, min, max) 

#define REGISTER_CMD3(class, name, setCode, toGet0, toGet1, toGet2, min, max) \
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

