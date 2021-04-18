#pragma once

#include <cstdio>
#include <ctime>
#include <iomanip>
#include <enet/enet.h>
#include <cereal/archives/binary.hpp>

#include "om_config.h"

#define MAXCLIENTS 12
#define SUPER_VERSION 0
#define MAIN_VERSION 0
#define SUB_VERSION 1
#define RELEASE_CODENAME "Alpha Release" // The Name . Can be empty
#define RELEASE_COMMENT "" // For betas and special builds only
