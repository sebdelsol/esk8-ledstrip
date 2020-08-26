#pragma once
#include <Streaming.h>

#define MAX_VAR 25
#define MAX_ARGS 3

//--------------------------------- 
// virtual functors class to hide lambda capture
template <class Ret, class...Args>
struct _Functor 
{ 
  virtual Ret operator()(Args... args) = 0; 
};

// actual implementation 
template <class Func, class Ret, class...Args>
class _FunctorImplement : public _Functor<Ret, Args...>
{
  Func func;
public:
  _FunctorImplement(Func func) : func(func) {};
  inline Ret operator()(Args... args) { return func(args...); };
};

// define newSetFunc & SetFunc of type : void SetFunc(int*, byte)
typedef _Functor<void, int*, byte> SetFunc; 
template <class Func> using SetFuncImplement = _FunctorImplement<Func, void, int*, byte>;  
template <class Func> inline SetFuncImplement<Func>* newSetFunc(Func func) { return new SetFuncImplement<Func>(func); }

// define newGetFunc & GetFunc of type : byte GetFunc(int*)
typedef _Functor<byte, int*> GetFunc; 
template <class Func> using GetFuncImplement = _FunctorImplement<Func, byte, int*>; 
template <class Func> inline GetFuncImplement<Func>* newGetFunc(Func func) { return new GetFuncImplement<Func>(func); }

//---------------------------------
struct MyVar 
{
  SetFunc*  set;
  GetFunc*  get;
  char*     name;
  int       min;
  int       max;
  bool      show;
  byte      ID;
  int       last[MAX_ARGS];
};

//---------------------------------
class OBJVar
{
  MyVar* mVar[MAX_VAR];
  byte mNVAR = 0;

public:  
  bool registerVar(const char* name, SetFunc* set, GetFunc* get, int min = 0, int max = 0, bool show = true);
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
    SetFunc* setF = newSetFunc([this](int* toSet, byte n) { if (n==0) { doCode; } }); \
    GetFunc* getF = newGetFunc([](int* toGet) -> byte { return 0; }); \
    registerVar(name, setF, getF, 0, 0, show); \
  }

#define REGISTER_CMD(name, doCode)         _REGISTER_CMD(name, doCode, true)
#define REGISTER_CMD_NOSHOW(name, doCode)  _REGISTER_CMD(name, doCode, false)

#define _REGISTER_VAR(name, setCode, toGet0, min, max, show) \
  { \
    SetFunc* setF = newSetFunc([this](int* toSet, byte n) \
    { \
      if (n==1) \
      { \
        int arg0 = toSet[0]; \
        setCode; \
      } \
    }); \
    GetFunc* getF = newGetFunc([this](int* toGet) -> byte \
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
    SetFunc* setF = newSetFunc([this](int* toSet, byte n) \
    { \
      if (n==3) \
      { \
        int arg0 = toSet[0]; \
        int arg1 = toSet[1]; \
        int arg2 = toSet[2]; \
        setCode; \
      } \
    }); \
    GetFunc* getF = newGetFunc([this](int* toGet) -> byte \
    { \
      toGet[0] = toGet0; \
      toGet[1] = toGet1; \
      toGet[2] = toGet2; \
      return 3; \
    }); \
    registerVar(name, setF, getF, min, max); \
  }