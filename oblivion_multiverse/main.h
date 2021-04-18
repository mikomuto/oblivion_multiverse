#pragma once

#include "obse/PluginAPI.h"
#include "obse/CommandTable.h"
#include "obse/ParamInfos.h"
#include <obse/obse/GameObjects.h>

#include <enet/enet.h>
#include <cereal/archives/binary.hpp>

#include "om_network.h"
#include "om_structs.h"

#include <string>
#include <sstream>

#define SUPER_VERSION 0
#define MAIN_VERSION 0
#define SUB_VERSION 1
#define RELEASE_CODENAME "Alpha Release" // The Name . Can be empty
#define RELEASE_COMMENT "" // For betas and special builds only

//externals
extern IDebugLog gLog;

