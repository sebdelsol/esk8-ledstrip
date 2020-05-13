#pragma once

#ifdef DEBUG_RASTER
  #define RASTER_BEGIN(nb) \
    struct Raster \
    { \
      const __FlashStringHelper* name; \
      long time; \
    } _rasters[nb]; \
    int _rasterMax = nb; \
    int _rasterCount = 0; \
    long _startTime = micros(); \

  #define RASTER(txt) \
    if (_rasterCount < _rasterMax) \
    { \
      _rasters[_rasterCount].time = micros(); \
      _rasters[_rasterCount++].name = F(txt); \
    }  \
    else \
      Serial << ">> ERROR !! Max Raster reached "  << _rasterCount << endl;

  #define RASTER_END \
    static long _lastEndTime;  \
    long _endTime = micros(); \
    Serial << "LOOP "  << (_endTime - _lastEndTime) << "µs"; \
    _lastEndTime = _endTime; \
    Serial << " \t TOTAL " << (_endTime - _startTime) << "µs"; \
    for(byte i=0; i < _rasterCount; i++) \
      Serial << " \t - " << _rasters[i].name << " " << (_rasters[i].time - (i==0 ? _startTime : _rasters[i-1].time)) << "µs  "; \
    Serial << endl;

#else
  #define RASTER_BEGIN
  #define RASTER(txt)
  #define RASTER_END
#endif
