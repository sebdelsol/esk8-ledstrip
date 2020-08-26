#pragma once
#include <Streaming.h>

#define MAX_VAR 25
#define MAX_ARGS 3

//--------------------------------- 
// virtual functors class to hide lambda capture
template <class Ret, class...Args>
struct _Functor { virtual Ret operator()(Args... args) = 0; };

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
#define _Store0(args, dummy)         	
#define _Store1(args, arg0)             args[0] = arg0;
#define _Store3(args, arg0, arg1, arg2) args[0] = arg0; args[1] = arg1; args[2] = arg2;

#define _AddVar(nArg, name, min, max, show, setCode, ...)                                            \
{                                                                                                          \
  SetFunc* setF = newSetFunc([this](int* args, byte n) { if (n==nArg) { setCode; } });                     \
  GetFunc* getF = newGetFunc([this](int* args) -> byte { _Store##nArg(args, __VA_ARGS__); return nArg; }); \
  registerVar(name, setF, getF, min, max, show);                                                           \
}

#define AddCmd(name, cmdCode)                           _AddVar(0, name, 0,   0,   true,  cmdCode)
#define AddCmdHid(name, cmdCode)                        _AddVar(0, name, 0,   0,   false, cmdCode)

#define AddVarCode(name, setCode, getExpr, min, max)    _AddVar(1, name, min, max, true,  setCode,       getExpr) 
#define AddVarName(name, var, min, max)                 _AddVar(1, name, min, max, true,  var = args[0], var) 
#define AddVarNameHid(name, var, min, max)              _AddVar(1, name, min, max, false, var = args[0], var) 
#define AddVar(var, min, max)                           _AddVar(1, #var, min, max, true,  var = args[0], var) 
#define AddVarHid(var, min, max)                        _AddVar(1, #var, min, max, false, var = args[0], var) 

#define AddVarCode3(name, setCode, getExpr0, getExpr1, getExpr2, min, max) _AddVar(3, name, min, max, true,  setCode,       getExpr0, getExpr1, getExpr2) 
