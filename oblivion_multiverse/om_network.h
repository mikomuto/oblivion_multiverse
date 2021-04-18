#pragma once

#include "main.h"
#include "om_config.h"


extern bool initializeClient();
extern void discardClient();
extern bool serverConnect();
extern void incomingPacketHandler();
extern void sendPlayerPOS();
extern bool serverDisconnect();

enum OMPacketType
{
	OMIdentity = 0,		//Send to synchronise versions , 
	OMPlayerPOS			//update player position on server
};