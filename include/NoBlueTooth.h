#pragma once

#include <Streaming.h>
#include <Pins.h>

void NoBT()
{
  pinMode(BLUE_PIN, OUTPUT); //blue led
  digitalWrite(BLUE_PIN, LOW); // switch off blue led
  btStop(); // turnoff bt 
  _log << "No Bluetooth" << endl;
}