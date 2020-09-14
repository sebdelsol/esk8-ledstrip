#pragma once

struct Raster
{
#ifdef DEBUG_RASTER
  static const int max = 20;

  struct                                                                                                               
  {                                                                                                                           
    const char* name;                                                                                          
    long        time;                                                                                                                
  } r[max];                                                                                                     

  int  n;                                                                                                       
  long start;                                                                                                 
  long lastEnd;  
  char fnumber[6];                                   

  inline void begin()
  {
    n = 0;                                                                                                           
    start = micros();
  };       

  // to be called only with string literal with static storage !!  
  inline void add(const char* name)
  {
    if (n < max)                                                                                              
    {                                                                                                                           
      r[n].time = micros();                                                                                   
      r[n++].name = name;                                                                                   
    }                                                                                                                           
    else                                                                                                                        
      _log << ">> ERROR !! Max Raster reached "  << n << endl;
  };

  inline char* printnb(long n)
  {
    snprintf(fnumber, 5, "%4d", n);
    return fnumber;
  };

  inline void end()
  {
    long end = micros();                                                                                                   
    _log << "LOOP "  << printnb(end - lastEnd) << "µs";                                                                    
    lastEnd = end;

    _log << "   TOTAL " << printnb(end - start) << "µs";                                                                  
    for (byte i=0; i < n; i++)
      _log << "  -  " << r[i].name << " " << printnb(r[i].time - (i==0 ? start : r[i-1].time)) << "µs"; 
    
    _log << "    - free Heap " << ESP.getFreeHeap() << "B" << endl;
  };

#else
  inline void begin(){};       
  inline void add(const char*){};
  inline void end(){};
#endif
};
