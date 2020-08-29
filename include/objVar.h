#pragma once
#include <Streaming.h>

#include <HashName.h>

#define MAX_VAR  25
#define MAX_ARGS 3

//--------------------------------- 
// abstract functor class to hide a lambda capture
template <class Ret, class...Args>
struct Fctor
{ 
  virtual Ret operator()(Args... args) = 0; 
};

// actual functor class implementation 
template <class F, class Ret, class...Args>
struct FctorI : Fctor<Ret, Args...>
{
  F f;
  FctorI(F f) : f(f) {};
  inline Ret operator()(Args... args) { return f(args...); };
};

#define NewFunctor(ftr, ...)                                                    /* __VA_ARGS__ gives Ret & Args...                                       */ \
using ftr = Fctor<__VA_ARGS__>;                                                 /* ftr     is a Fctor<Ret, Args...>                                      */ \                      
template <class F> using ftr##I = FctorI<F, __VA_ARGS__>;                       /* ftrI<F> is a FctorI<F, Ret, Args...> implementing Fctor<Ret, Args...> */ \
template <class F> inline ftr##I<F>* new##ftr(F f) { return new ftr##I<F>(f); } // newftr<F>(f) returns a ftrI<F>* that stores Ret f(Args...){} 

NewFunctor(SetFunc, void, int*, byte)  // newSetFunc(f) returns a SetFunc* that stores void f(int*, byte){}
NewFunctor(GetFunc, byte, int*)        // newGetFunc(f) returns a GetFunc* that stores byte f(int*){} 

//---------------------------------
struct MyVar 
{
  SetFunc*  set;
  GetFunc*  get;
  char*     name;
  int       min, max;
  bool      show;
  byte      ID;
  int       last[MAX_ARGS];
};

//---------------------------------
class OBJVar
{
  MyVar* mVar[MAX_VAR];
  byte   mNVAR = 0;

  HashName<MAX_VAR, MyVar> mHash;

public:  
  bool   registerVar(const char* name, SetFunc* set, GetFunc* get, int min = 0, int max = 0, bool show = true);
  byte   getNbVar()                       { return mNVAR;};
  char*  getVarName(byte i)               { return mVar[i]->name; };
  MyVar* getVar(byte i)                   { return mVar[i]; };
  MyVar* getVarFromName(const char* name) { return mHash.get(name); };


  void set(MyVar* var, int* toSet, byte n, bool change = false);
  byte get(MyVar* var, int* toGet);
  void getMinMax(MyVar* var, int* min, int* max);

  byte getID(MyVar* var) { return var->ID; };
  void setID(MyVar* var, byte id) const { var->ID = id; };

  // ------ ObjTestVarFunc 
  using ObjTestVarFunc = bool (OBJVar::*)(byte i);
  
  bool isVarShown(byte i) { return mVar[i]->show; };
  bool hasVarChanged(byte i);
};

//---------------------------------
#define _Stor0(args)                                                        return 0 
#define _Stor1(args, _0)         args[0] = _0;                              return 1
#define _Stor3(args, _0, _1, _2) args[0] = _0; args[1] = _1; args[2] = _2;  return 3

#define _AddVar(N, name, min, max, show, set, ...) /* __VA_ARGS__ gives optional get expressions */ \
{                                                                                                   \
  SetFunc* setF = newSetFunc([this](int* args, byte n) { if (n==N) { set; }             });         \
  GetFunc* getF = newGetFunc([this](int* args) -> byte { _Stor##N(args, ##__VA_ARGS__); });         \
  registerVar(name, setF, getF, min, max, show);                                                    \
}

#define AddCmd(name, cmd)                                  _AddVar(0, name, 0,   0,   true,  cmd)
#define AddCmdHid(name, cmd)                               _AddVar(0, name, 0,   0,   false, cmd)
#define AddVarCode(name, set, get, min, max)               _AddVar(1, name, min, max, true,  set,           get) 
#define AddVarName(name, var, min, max)                    _AddVar(1, name, min, max, true,  var = args[0], var) 
#define AddVarNameHid(name, var, min, max)                 _AddVar(1, name, min, max, false, var = args[0], var) 
#define AddVar(var, min, max)                              _AddVar(1, #var, min, max, true,  var = args[0], var) 
#define AddVarHid(var, min, max)                           _AddVar(1, #var, min, max, false, var = args[0], var) 
#define AddVarCode3(name, set, get0, get1, get2, min, max) _AddVar(3, name, min, max, true,  set,           get0, get1, get2) 

#define AddBool(var)              AddVar(var, 0, 1)
#define AddBoolName(name, var)    AddVarName(name, var, 0, 1)
#define AddBoolNameHid(name, var) AddVarNameHid(name, var, 0, 1)
