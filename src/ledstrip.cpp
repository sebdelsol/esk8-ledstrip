#include <ledstrip.h>

// ----------------------------------------------------
#ifdef FASTLED_SHOW_CORE

  static TaskHandle_t FastLEDshowTaskHandle = 0;
  static TaskHandle_t userTaskHandle = 0;

  // Call this function instead of FastLED.show(). It signals core 0 to issue a show,
  // then waits for a notification that it is done.
  void TriggerFastLEDShow()
  {
    if (userTaskHandle == 0)
    {
      // Store the handle of the current task, so that the show task can notify it when it's done
      userTaskHandle = xTaskGetCurrentTaskHandle();

      // Trigger the show task
      xTaskNotifyGive(FastLEDshowTaskHandle);

      // Wait to be notified that it's done
      const TickType_t xMaxBlockTime = pdMS_TO_TICKS(5);
      ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
      userTaskHandle = 0;
    }
  }

  // This function runs on core 0 and just waits for requests to call FastLED.show()
  void FastLEDshowTask(void *pvParameters)
  {
    for (;;) // forever
    {
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Wait for the trigger
      FastLED.show(); //Do the show (synchronously)
      xTaskNotifyGive(userTaskHandle); // Notify the calling task
    }
  }
#endif

// ----------------------------------------------------
AllLedStrips::AllLedStrips(const int maxmA, Stream &serial) : mSerial(&serial)
{
  FastLED.setMaxPowerInVoltsAndMilliamps(5, maxmA);
  FastLED.countFPS();
  FastLED.setDither(BINARY_DITHER); //DISABLE_DITHER
  // FastLED.setTemperature(UncorrectedTemperature);    
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
    Serial << "Fastled run on task on Core " << FASTLED_SHOW_CORE << " with Prio " << FASTLED_TASK_PRIO << endl;
  #else 
    Serial << "Fastled run on main Core " << endl;
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
