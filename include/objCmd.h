#pragma once
#include <Arduino.h.>

#define MAX_CMD 5

class OBJCmd
{
  typedef void (*setFunc)(void *obj, byte* toSet, byte n);
  typedef byte (*getFunc)(void *obj, byte* toGet);

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
  void set(char* name, byte* toSet, byte n);
  byte get(char* name, byte* toGet);
};

#define REGISTER_CMD(class, name, setCode, toGet0) \
    registerCmd(this, name, \
      [](void* obj, byte* toSet, byte n) { \
        if (n==1) { \
          class* self = (class *)obj; \
          byte arg0 = toSet[0]; \
          setCode; \
        } \
      }, \
      [](void* obj, byte* toGet) -> byte  { \
        class* self = (class *)obj; \
        toGet[0] = toGet0; \
        return 1; \
      } \
    );

#define REGISTER_CMD_SIMPLE(class, name, var) REGISTER_CMD(class, name, { var = arg0; }, var) 

#define REGISTER_CMD3(class, name, setCode, toGet0, toGet1, toGet2) \
    registerCmd(this, name, \
      [](void* obj, byte* toSet, byte n) { \
        if (n==3) { \
          class* self = (class *)obj; \
          byte arg0 = toSet[0]; \
          byte arg1 = toSet[1]; \
          byte arg2 = toSet[2]; \
          setCode; \
        } \
      }, \
      [](void* obj, byte* toGet) -> byte  { \
        class* self = (class *)obj; \
        toGet[0] = toGet0; \
        toGet[1] = toGet1; \
        toGet[2] = toGet2; \
        return 3; \
      } \
    );

