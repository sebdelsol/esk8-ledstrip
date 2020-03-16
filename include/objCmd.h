#pragma once
#include <Arduino.h.>
#include <Streaming.h>

#define MAX_CMD 17

class OBJCmd
{
  typedef void (*setFunc)(void *obj, int* toSet, byte n);
  typedef byte (*getFunc)(void *obj, int* toGet);

  struct Cmd {
    char*   name;
    void*   obj;
    setFunc set;
    getFunc get;
  };

  Cmd mCmd[MAX_CMD];
  byte mNCMD = 0;

public:  
  bool registerCmd(void *obj, char *name, setFunc set, getFunc get);
  void set(char* name, int* toSet, byte n);
  byte get(char* name, int* toGet);

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

#define REGISTER_CMD(class, name, setCode, toGet0) \
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
      } \
    );

#define REGISTER_CMD_SIMPLE(class, name, var) REGISTER_CMD(class, name, { var = arg0; }, var) 
//#define REGISTER_CMD_SIMPLE(class, var) REGISTER_CMD(class, #var, { var = arg0; }, var) 

#define REGISTER_CMD3(class, name, setCode, toGet0, toGet1, toGet2) \
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
      } \
    );

