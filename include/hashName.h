#pragma once
#include <Streaming.h>

// linear Probe hash for [name] => Class*
// Class needs to store its name and implement a getname() method 
template <int N, class Class>
class HashName 
{
  static constexpr int getN(uint8_t n) { return n * 2; }; // bigger but less long collisions

  Class*      values[getN(N)];
  uint8_t     maxCol = 0;

  inline uint8_t hash(const char *name) // naive hash, works ok with N*2
  {
    uint8_t h = 0;
    while (*name) h += *name++;
    return h % getN(N);
  };

  inline uint8_t next(uint8_t i) { return (i+1) % getN(N); };

public:
  HashName() { memset( values, 0, getN(N) ); };
  
  void add(Class* value)
  {
    assert (value!=nullptr);
    const char *name = value->getname();
    assert (name!=nullptr);
    
    uint8_t col = 0;
    uint8_t i = hash(name);
    while(values[i] != nullptr) // something already there ?
    {
      i = next(i);
      col++;
    }
    maxCol = col > maxCol ? col : maxCol;

    if (col > 0)
      Serial << "! hash of [" << name << "] has " << col << " collisions" << endl;

    values[i] = value;
  };

  inline bool valueHasNotMyName(Class* v, const char* name) 
  {
    const char* vname = v->getname();
    return vname != nullptr && strcmp(vname, name) != 0 ? true : false;
  };

  Class* get(const char *name)
  {
    assert (name!=nullptr);
    
    uint8_t col = 0;
    uint8_t i = hash(name);
    while(values[i] != nullptr && valueHasNotMyName(values[i], name) ) 
    {
      if (++col > maxCol) return nullptr; // failed
      i = next(i);
    }
    
    // Serial << "got " << name << " stored@" << i << endl;
    return values[i];
  };
};
