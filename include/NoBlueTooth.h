#pragma once

#include <Streaming.h>
#include <Pins.h>

void NoBT()
{
  pinMode(LIGHT_PIN, OUTPUT); //blue led
  digitalWrite(LIGHT_PIN, LOW); // switch off blue led
  btStop(); // turnoff bt 
  Serial << "No Bluetooth" << endl;
}