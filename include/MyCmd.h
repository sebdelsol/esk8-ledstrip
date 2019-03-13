#pragma once

#include <Arduino.h>

#define BTCMD_MAXARGS 3

typedef struct MyCmd{
  char *cmd;
  char fx;
  char what;
  byte arg[BTCMD_MAXARGS];
  byte nbArg;
  Stream* answer;
} MyCmd;
