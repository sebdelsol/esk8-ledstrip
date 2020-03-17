#pragma once
#include <Arduino.h.>
#include <Streaming.h>

#define MAX_CMD 17

typedef void (*setCmdFunc)(void *obj, int* toSet, byte n);
typedef byte (*getCmdFunc)(void *obj, int* toGet);

struct MyCmd {
  char*       name;
  void*       obj;
  setCmdFunc  set;
  getCmdFunc  get;
  int         min;
  int         max;
};

class OBJCmd
{

  MyCmd mCmd[MAX_CMD];
  byte mNCMD = 0;

public:  
  bool registerCmd(void *obj, const char *name, setCmdFunc set, getCmdFunc get, int min=0, int max=0);
  MyCmd* getCmd(const char* name);

  void set(MyCmd* cmd, int* toSet, byte n);
  byte get(MyCmd* cmd, int* toGet);
  void getMinMax(MyCmd* cmd, int* min, int* max);

  byte getNbCmd() { return mNCMD;};
  char* getCmdName(byte i) { return mCmd[i].name;};
};

#define REGISTER_CMD_PURE(class, name, doCode) \
    registerCmd(this, name, \
      [](void* obj, int* toSet, byte n) { \
        if (n==0) { \
          class* self = (class *)obj; \
          doCode; \
        } \
      }, \
      [](void* obj, int* toGet) -> byte  { \
        return 0; \
      } \
    );

#define REGISTER_CMD(class, name, setCode, toGet0, min, max) \
    registerCmd(this, name, \
      [](void* obj, int* toSet, byte n) { \
        if (n==1) { \
          class* self = (class *)obj; \
          int arg0 = toSet[0]; \
          setCode; \
        } \
      }, \
      [](void* obj, int* toGet) -> byte  { \
        class* self = (class *)obj; \
        toGet[0] = toGet0; \
        return 1; \
      }, \
      min, max \
    );

#define REGISTER_CMD_SIMPLE(class, name, var, min, max) REGISTER_CMD(class, name, { var = arg0; }, var, min, max) 
//#define REGISTER_CMD_SIMPLE(class, var) REGISTER_CMD(class, #var, { var = arg0; }, var) 

#define REGISTER_CMD3(class, name, setCode, toGet0, toGet1, toGet2, min, max) \
    registerCmd(this, name, \
      [](void* obj, int* toSet, byte n) { \
        if (n==3) { \
          class* self = (class *)obj; \
          int arg0 = toSet[0]; \
          int arg1 = toSet[1]; \
          int arg2 = toSet[2]; \
          setCode; \
        } \
      }, \
      [](void* obj, int* toGet) -> byte  { \
        class* self = (class *)obj; \
        toGet[0] = toGet0; \
        toGet[1] = toGet1; \
        toGet[2] = toGet2; \
        return 3; \
      }, \
      min, max \
    );

