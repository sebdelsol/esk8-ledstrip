#pragma once
#include <Streaming.h>

#define MAX_VAR 25
#define MAX_ARGS 3

//--------------------------------- 
//setVarFunc functors class to hide lambda capture
class setVarFunc
{
public:
  virtual void operator()(int* toSet, byte n) = 0;
};

template <class Func>
class setVarFuncF : public setVarFunc
{
    Func func_;
public:
    setVarFuncF(Func func) : func_(func) {}
    inline void operator()(int* toSet, byte n) { func_(toSet, n); };
};

template <class Func>
setVarFuncF<Func>* newSetVarFunc(Func func) { return new setVarFuncF<Func>(func); }

//-------------- 
// getVarFunc functors class to hide lambda capture
class getVarFunc
{
public:
  virtual byte operator()(int* toSet) = 0;
};

template <class Func>
class getVarFuncF : public getVarFunc
{
    Func func_;
public:
    getVarFuncF(Func func) : func_(func) {}
    inline byte operator()(int* toSet) { return func_(toSet); };
};

template <class Func>
getVarFuncF<Func>* newGetVarFunc(Func func) { return new getVarFuncF<Func>(func); }

//---------------------------------
struct MyVar 
{
  char*       name;
  setVarFunc* set;
  getVarFunc* get;
  int         min;
  int         max;
  bool        show;
  byte        ID;
  int         last[MAX_ARGS];
};

//---------------------------------
class OBJVar
{
  MyVar* mVar[MAX_VAR];
  byte mNVAR = 0;

public:  
  bool registerVar(const char* name, setVarFunc* set, getVarFunc* get, int min = 0, int max = 0, bool show = true);
  MyVar* getVarFromName(const char* name);
  MyVar* getVar(byte i) { return mVar[i]; };

  void set(MyVar* var, int* toSet, byte n, bool change = false);
  byte get(MyVar* var, int* toGet);
  void getMinMax(MyVar* var, int* min, int* max);

  byte getID(MyVar* var) { return var->ID; };
  void setID(MyVar* var, byte id) const { var->ID = id; };

  byte getNbVar() { return mNVAR;};
  char* getVarName(byte i) { return mVar[i]->name; };

  // ------ ObjTestVarFunc 
  typedef bool (OBJVar::*ObjTestVarFunc)(byte i);
  bool isVarShown(byte i) { return mVar[i]->show; };
  bool hasVarChanged(byte i);
};

//---------------------------------
#define _REGISTER_CMD(name, doCode, show) \
  { \
    setVarFunc* setF = newSetVarFunc([this](int* toSet, byte n) { if (n==0) { doCode; } }); \
    getVarFunc* getF = newGetVarFunc([](int* toGet) -> byte { return 0; }); \
    registerVar(name, setF, getF, 0, 0, show); \
  }

#define REGISTER_CMD(name, doCode)         _REGISTER_CMD(name, doCode, true)
#define REGISTER_CMD_NOSHOW(name, doCode)  _REGISTER_CMD(name, doCode, false)

#define _REGISTER_VAR(name, setCode, toGet0, min, max, show) \
  { \
    setVarFunc* setF = newSetVarFunc([this](int* toSet, byte n) \
    { \
      if (n==1) \
      { \
        int arg0 = toSet[0]; \
        setCode; \
      } \
    }); \
    getVarFunc* getF = newGetVarFunc([this](int* toGet) -> byte \
    { \
      toGet[0] = toGet0; \
      return 1; \
    }); \
    registerVar(name, setF, getF, min, max, show); \
  }

#define REGISTER_VAR(name, setCode, toGet0, min, max)        _REGISTER_VAR(name, setCode, toGet0, min, max, true) 
#define REGISTER_VAR_NOSHOW(name, setCode, toGet0, min, max) _REGISTER_VAR(name, setCode, toGet0, min, max, false) 

#define REGISTER_VAR_SIMPLE(name, var, min, max)             _REGISTER_VAR(name, { var = arg0; }, var, min, max, true) 
#define REGISTER_VAR_SIMPLE_NOSHOW(name, var, min, max)      _REGISTER_VAR(name, { var = arg0; }, var, min, max, false) 

#define REGISTER_VAR_SIMPLE_NAME(var, min, max)             _REGISTER_VAR(#var, { var = arg0; }, var, min, max, true) 
#define REGISTER_VAR_SIMPLE_NAME_NOSHOW(var, min, max)      _REGISTER_VAR(#var, { var = arg0; }, var, min, max, false) 

#define REGISTER_VAR3(name, setCode, toGet0, toGet1, toGet2, min, max) \
  { \
    setVarFunc* setF = newSetVarFunc([this](int* toSet, byte n) \
    { \
      if (n==3) \
      { \
        int arg0 = toSet[0]; \
        int arg1 = toSet[1]; \
        int arg2 = toSet[2]; \
        setCode; \
      } \
    }); \
    getVarFunc* getF = newGetVarFunc([this](int* toGet) -> byte \
    { \
      toGet[0] = toGet0; \
      toGet[1] = toGet1; \
      toGet[2] = toGet2; \
      return 3; \
    }); \
    registerVar(name, setF, getF, min, max); \
  }