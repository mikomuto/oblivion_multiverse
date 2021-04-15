#include "om_network.h"

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
		_MESSAGE("ENet client host initialization failed");
		return false;
	}
	_MESSAGE("ENet client host initialized");
	return true;
}

void discardClient()
{
	enet_host_destroy(client);
}

bool serverConnect() {
	if (!isConnected) {
		_MESSAGE("Connecting to %s : %u", ServerAddress, ServerPort);
		/* Connect to some.server.net:1234. */
		enet_address_set_host(& address, ServerAddress);
		address.port = ServerPort;
		/* Initiate the connection, allocating the two channels 0 and 1. */
		peer = enet_host_connect(client, & address, 2, 0);

		if (peer == NULL)
		{
			_MESSAGE("No available peers for initiating an ENet connection");
			return false;
		}
		/* Wait up to 5 seconds for the connection attempt to succeed. */
		if (enet_host_service(client, &event, 5000) > 0 &&
			event.type == ENET_EVENT_TYPE_CONNECT)
		{
			// send identifier using a reliable packet on channel 0
			std::string tmpident = "OMIDENT";
			tmpident.append(std::to_string(SUPER_VERSION));
			tmpident.append(std::to_string(MAIN_VERSION));
			tmpident.append(std::to_string(SUB_VERSION));
			const char* omidentifier = tmpident.c_str();
			ENetPacket* packet = enet_packet_create(omidentifier, strlen(omidentifier) + 1, ENET_PACKET_FLAG_RELIABLE);
			// queue a packet for the peer over channel id 0
			enet_peer_send(peer, 0, packet);
			// flush to send
			enet_host_flush(client);

			isConnected = true;
			_MESSAGE("Connected to  %s : %u", ServerAddress, ServerPort);
			return true;
		}
		else
		{
			/* Either the 5 seconds are up or a disconnect event was */
			/* received. Reset the peer in the event the 5 seconds   */
			/* had run out without any significant event.            */
			enet_peer_reset(peer);
			_MESSAGE("Connection failed");
			return false;
		}
	}
}

bool serverDisconnect() {
	if (isConnected) {
		_MESSAGE("Disconnecting from server");
		if (peer != NULL) {
			enet_peer_disconnect(peer, 0);
			/* Allow up to 3 seconds for the disconnect to succeed
			 * and drop any packets received packets.
			 */
			while (enet_host_service(client, &event, 3000) > 0)
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_RECEIVE:
					enet_packet_destroy(event.packet);
					break;
				case ENET_EVENT_TYPE_DISCONNECT:
					_MESSAGE("Disconnected");
					isConnected = false;
					return true;
				}
			}
			/* We've arrived here, so the disconnect attempt didn't */
			/* succeed yet.  Force the connection down.             */
			enet_peer_reset(peer);
		}
	}
	return false;
}