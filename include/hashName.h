#pragma once

#include <Streaming.h>

// linear Probe hash for [name] => Class* obj
// Class needs to store its name and implement a getname() method 
template <int N, class Class>
class HashName 
{
  static constexpr int getN(uint8_t n) { return n * 2; }; // bigger but less long collisions

  Class*      objs[getN(N)];
  uint8_t     maxCol = 0;

  inline uint8_t hash(const char *name) // naive hash, works ok with N*2
  {
    uint8_t h = 0;
    while (*name) h += *name++;
    return h % getN(N);
  };

  inline uint8_t next(uint8_t i) { return (i+1) % getN(N); };

public:
  HashName() { memset( objs, 0, getN(N) ); };
  
  void add(Class* obj)
  {
    assert (obj!=nullptr);
    const char *name = obj->getname();
    assert (name!=nullptr);
    
    uint8_t col = 0;
    uint8_t i = hash(name);
    while(objs[i] != nullptr) 
    { // linear probe looking for an empty slot
      i = next(i);
      col++;
    }
    maxCol = col > maxCol ? col : maxCol;

    if (col > 0)
      Serial << "! hash of [" << name << "] has " << col << " collisions" << endl;

    objs[i] = obj;
  };

  inline bool objHasNotMyName(Class* obj, const char* name) 
  {
    const char* oname = obj->getname();
    return oname != nullptr && strcmp(oname, name) != 0 ? true : false;
  };

  Class* get(const char *name)
  {
    assert (name!=nullptr);
    
    uint8_t col = 0;
    uint8_t i = hash(name);
    while(objs[i] != nullptr && objHasNotMyName(objs[i], name) ) 
    { // linear probe looking for an obj named name
      if (++col > maxCol) return nullptr; // failed
      i = next(i);
    }
    
    return objs[i];
  };
};
