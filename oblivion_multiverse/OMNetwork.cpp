#include "OMNetwork.h"

ENetAddress address;
ENetHost* client;
ENetPeer* peer;
ENetEvent event;
ENetPacket* packet;

bool isConnected = false;

bool initializeClient()
{
	//initialize client
	client = enet_host_create(NULL /* create a client host */,
		1 /* only allow 1 outgoing connection */,
		2 /* allow up 2 channels to be used, 0 and 1 */,
		0 /* assume any amount of incoming bandwidth */,
		0 /* assume any amount of outgoing bandwidth */);
	if (client == NULL)
	{
		_MESSAGE("ENet client host initialization failed\n");
		return false;
	}
	_MESSAGE("ENet client host initialized\n");
	return true;
}

void discardClient()
{
	enet_host_destroy(client);
}

bool serverConnect() {
	if (!isConnected) {
		/* Connect to some.server.net:1234. */
		enet_address_set_host(&address, "localhost");
		address.port = 41805;
		/* Initiate the connection, allocating the two channels 0 and 1. */
		peer = enet_host_connect(client, &address, 2, 0);

		if (peer == NULL)
		{
			_MESSAGE("No available peers for initiating an ENet connection\n");
			return false;
		}
		/* Wait up to 5 seconds for the connection attempt to succeed. */
		if (enet_host_service(client, &event, 1000) > 0 &&
			event.type == ENET_EVENT_TYPE_CONNECT)
		{
			puts("Connection to some.server.net:1234 succeeded.");
			isConnected = true;
			_MESSAGE("Connected to server\n");
			return true;
		}
		else
		{
			/* Either the 5 seconds are up or a disconnect event was */
			/* received. Reset the peer in the event the 5 seconds   */
			/* had run out without any significant event.            */
			enet_peer_reset(peer);
			_MESSAGE("Connection failed\n");
			return false;
		}
	}
}
