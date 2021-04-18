#pragma once

#include "include/simpleini-4.17/SimpleIni.h"

#include <thread>

//externals
extern void OMLoadConfig();
extern void OMUpdateConfig();
extern char ServerAddress[16];
extern unsigned short ServerPort;
extern char ServerPassword[32];
extern int MaxPlayers;
