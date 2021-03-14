#pragma once

#include "obse/PluginAPI.h"
#include "obse/CommandTable.h"

#include <enet/enet.h>

#include "om_network.h"

#include <string>

#define MAXCLIENTS 12
#define SUPER_VERSION 0
#define MAIN_VERSION 0
#define SUB_VERSION 1
#define RELEASE_CODENAME "Alpha Release" // The Name . Can be empty
#define RELEASE_COMMENT "" // For betas and special builds only

//externals
extern IDebugLog gLog;

