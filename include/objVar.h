#pragma once

#include <HashName.h>
#include <iterator.h>

#define MAX_VAR  16
#define MAX_ARGS 3

//--------------------------------- 
// abstract functor class to hide a lambda capture
template <class Ret, class...Args>
struct Fctor
{ 
  virtual Ret operator()(Args...) = 0; 
};

// actual functor class implementation 
template <class F, class Ret, class...Args>
struct FctorI : Fctor<Ret, Args...>
{
  const F f;
  FctorI(const F& f) : f(f) {};
  inline Ret operator()(Args... args) { return f(args...); };
};

#define NewFunctor(ftr, ...)                              /* __VA_ARGS__ gives Ret & Args...                                       */ \
using ftr = Fctor<__VA_ARGS__>;                           /* ftr     is a Fctor<Ret, Args...>                                      */ \
template <class F> using ftr##I = FctorI<F, __VA_ARGS__>; /* ftrI<F> is a FctorI<F, Ret, Args...> implementing Fctor<Ret, Args...> */ \
template <class F> ftr##I<F>* new##ftr(const F& f) { return new ftr##I<F>(f); } // newftr<F>(f) returns a ftrI<F>* that stores Ret f(Args...){} 

// actual use
using SetArgs = const int (&)[MAX_ARGS];  // ref on int[MAX_ARGS] non mutable
using GetArgs = int (&)[MAX_ARGS];        // ref on int[MAX_ARGS] mutable 

NewFunctor(SetFunc, void, SetArgs, byte)  // newSetFunc(f) returns a SetFunc* that stores void f(SetArg, byte){}
NewFunctor(GetFunc, byte, GetArgs)        // newGetFunc(f) returns a GetFunc* that stores byte f(GetArgs){} 

//---------------------------------
enum class TrackChange : uint8_t { yes, no, undefined };

class MyVar 
{
  SetFunc*  mSetF;
  GetFunc*  mGetF;
  char*     mName;
  int       mMin, mMax;
  bool      mShow;
  byte      mID;
  int       mLast[MAX_ARGS];

public:
  MyVar(const char* name, SetFunc* set, GetFunc* get, int min, int max, bool show);
  inline const char* getName() { return mName; };
  
  void   getRange(int& min, int& max);
  void   set(SetArgs toSet, byte n, TrackChange trackChange);
  byte   get(GetArgs toGet);

  byte   getID()        { return mID; };
  void   setID(byte id) { mID = id; };

  using  TestFunc = bool (MyVar::*)();
  bool   isShown()      { return mShow; };
  bool   hasChanged();
};

//---------------------------------
class OBJVar
{
  HashName<MyVar, MAX_VAR> mHash;
  MyVar*      mVar[MAX_VAR];
  byte        mNVAR = 0;
  const char* mName;

public:  
  void setName(const char* name) { mName = name; };
  inline const char* getName()   { return mName; };

  bool   addVar(const char* name, SetFunc* set, GetFunc* get, int min = 0, int max = 0, bool show = true);
  MyVar* getVarFromName(const char* name) { return mHash.get(name); };
  ArrayIterator(MyVar, mVar, mNVAR);
};

//---------------------------------
#define _Stor0(args)                                                        return 0 
#define _Stor1(args, _0)         args[0] = _0;                              return 1
#define _Stor3(args, _0, _1, _2) args[0] = _0; args[1] = _1; args[2] = _2;  return 3

#define _AddVar(N, name, min, max, show, set, ...)                                              \
{ /* __VA_ARGS__ gives optional get expressions */                                              \
  SetFunc* setF = newSetFunc( [this](SetArgs args, byte n) { if (n==N) { set; } });             \
  GetFunc* getF = newGetFunc( [this](GetArgs args) -> byte { _Stor##N(args, ##__VA_ARGS__); }); \
  addVar(name, setF, getF, min, max, show);                                                     \
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
