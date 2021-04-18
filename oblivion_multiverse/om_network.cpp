#include "om_network.h"

ENetAddress address;
ENetHost* client;
ENetPeer* peer;
ENetEvent event;
ENetPacket* packet;

bool isConnected = false;

void incomingPacketHandler() {
	//check for pending event(incoming packet)
	enet_host_service(client, &event, 0);
		switch (event.type)
		{
		case ENET_EVENT_TYPE_RECEIVE:
			_MESSAGE("A packet of length %u containing %s was received from %s on channel %u.\n",
				event.packet->dataLength,
				event.packet->data,
				event.peer->data,
				event.channelID);
			/* Clean up the packet now that we're done using it. */
			enet_packet_destroy(event.packet);
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			_MESSAGE("Server forced disconnect");
			isConnected = false;
			/* Reset the peer's client information. */
			event.peer->data = NULL;
		}
}

void sendPlayerPOS() {
	if (isConnected) {
		_MESSAGE("connected and running sendPlayerPOS");
		UInt32 cellID;
		if ((*g_thePlayer)->parentCell->worldSpace)
		{
			cellID = (*g_thePlayer)->parentCell->worldSpace->refID;
		}
		else
		{
			cellID = (*g_thePlayer)->parentCell->refID;
		}
		// send position using a unreliable packet on channel 1
		std::ostringstream SData;
		{
			cereal::BinaryOutputArchive Archive(SData);
			Archive(OMPlayerPOS, cellID, (*g_thePlayer)->posX, (*g_thePlayer)->posY, (*g_thePlayer)->posZ, (*g_thePlayer)->rotX, (*g_thePlayer)->rotY, (*g_thePlayer)->rotZ);
		}
		std::string Out = SData.str();
		ENetPacket* packet = enet_packet_create(Out.c_str(), Out.size(), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
		// queue a packet for the peer over channel id 1
		enet_peer_send(peer, 1, packet);
		// flush to send
		enet_host_flush(client);
	}
}

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
		_MESSAGE("Connecting to %s:%u", ServerAddress, ServerPort);
		//Connect to server
		enet_address_set_host(&address, ServerAddress);
		address.port = ServerPort;
		/* Initiate the connection, allocating the two channels 0 and 1. */
		peer = enet_host_connect(client, &address, 2, 0);

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
			std::ostringstream SData;
			{
				cereal::BinaryOutputArchive Archive(SData);
				Archive(OMIdentity,SUPER_VERSION, MAIN_VERSION, SUB_VERSION, ServerPassword);
			}
			std::string Out = SData.str();
			ENetPacket* packet = enet_packet_create(Out.c_str(), Out.size(), ENET_PACKET_FLAG_RELIABLE);
			// queue a packet for the peer over channel id 0
			enet_peer_send(peer, 0, packet);
			// flush to send
			enet_host_flush(client);

			isConnected = true;
			_MESSAGE("Connected");
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
			enet_peer_disconnect_later(peer, 0);
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