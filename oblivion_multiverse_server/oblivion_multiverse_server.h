#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <WS2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include <cstdio>
#include <ctime>
#include <iomanip>
#include <thread>

//external
#include <enet/enet.h>
#include <cereal/archives/binary.hpp>

//internal
#include "om_config.h"

#define MAXCLIENTS 12
#define SUPER_VERSION 0
#define MAIN_VERSION 1
#define SUB_VERSION 2
#define RELEASE_CODENAME "Alpha Release" // The Name . Can be empty
#define RELEASE_COMMENT "" // For betas and special builds only
