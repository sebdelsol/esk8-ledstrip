#include <ledstrip.h>

// ----------------------------------------------------
#ifdef FASTLED_SHOW_CORE
  static TaskHandle_t FastLEDshowTaskHandle = 0;
  static TaskHandle_t userTaskHandle = 0;

  // Trigger FastLED.show()
  void TriggerFastLEDShow()
  {
    if (userTaskHandle == 0)
    {
      // the show task can notify it when it's done
      userTaskHandle = xTaskGetCurrentTaskHandle();

      xTaskNotifyGive(FastLEDshowTaskHandle); // trigger fastled show task
      ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(5)); // Wait to be notified that it's done (max 5ms)
      userTaskHandle = 0;
    }
  }

  void FastLEDshowTask(void* pvParameters)
  {
    for (;;) // forever
    {
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Wait for the trigger
      FastLED.show(); 
      xTaskNotifyGive(userTaskHandle); // Notify the calling task
    }
  }
#endif

// ----------------------------------------------------
AllLedStrips::AllLedStrips(const int maxmA, Stream& serial) : mSerial(&serial)
{
  FastLED.setMaxPowerInVoltsAndMilliamps(5, maxmA);
  FastLED.countFPS();
  FastLED.setDither(BINARY_DITHER); //DISABLE_DITHER
}

bool AllLedStrips::registerStrip(BaseLedStrip &strip)
{
  bool ok = mNStrips < MAXSTRIP;
  if (ok)
  {
    mStrips[mNStrips++] = &strip;
    strip.setSerial(mSerial);
  }
  else
    *mSerial << "----------- !!!!!!!!!! Max LedStrips is reached " << MAXSTRIP << endl; 

  return ok;
}

void AllLedStrips::init() 
{
  for (byte i=0; i < mNStrips; i++)
    mStrips[i]->init();

  #ifdef FASTLED_SHOW_CORE
    xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, NULL, FASTLED_TASK_PRIO, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);  
    Serial << "Fastled runs on task on Core " << FASTLED_SHOW_CORE << " with Prio " << FASTLED_TASK_PRIO << endl;
  #else 
    Serial << "Fastled runs on main Core " << endl;
  #endif
}

void AllLedStrips::clearAndShow() 
{ 
  for(byte i=0; i < 3; i++)
    FastLED.clear(true);
}

void AllLedStrips::update()
{
  ulong t = GET_MILLIS();
  ulong dt = mLastT ? t - mLastT : 1; // to prevent possible /0
  mLastT += dt;

  for (byte i=0; i < mNStrips; i++)
    mStrips[i]->update(t, dt);
}

void AllLedStrips::getInfo()
{
  *mSerial << "FPS " << FastLED.getFPS() << endl;
  for (byte i=0; i < mNStrips; i++)
    mStrips[i]->getInfo();
}

void AllLedStrips::show() 
{ 
  #ifdef FASTLED_SHOW_CORE
    TriggerFastLEDShow();
  #else
    FastLED.show(); 
  #endif
}
