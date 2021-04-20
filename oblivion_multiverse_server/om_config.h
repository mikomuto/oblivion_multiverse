#pragma once

#include "simpleini-4.17/SimpleIni.h"

//externals
extern void OMLoadConfig();
extern void OMUpdateConfig();
extern int ServerPort;
extern int MaxClients;
extern char ServerPassword[32];

char ServerPassword[32];
int ServerPort = 0;
int MaxClients = 0;