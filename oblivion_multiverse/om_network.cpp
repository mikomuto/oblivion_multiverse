#include "om_network.h"

ENetAddress address;
ENetHost* client;
ENetPeer* peer;
ENetEvent event;
ENetPacket* packet;

bool isConnected = false;

void enetSyncHandler() {
	//check for pending event(incoming packet)
	enet_host_service(client, &event, 0);
	char hrIP[INET_ADDRSTRLEN];
	if (event.type != NULL) {
		//convert event host ip address to human readable
		inet_ntop(AF_INET, &(event.peer->address.host), hrIP, INET_ADDRSTRLEN);
	}
	switch (event.type)
	{
	case ENET_EVENT_TYPE_RECEIVE:
		//check packet flag
		int flag;
		{
			std::string tmpData(((char*)event.packet->data), event.packet->dataLength);
			std::istringstream is(tmpData);
			cereal::BinaryInputArchive Archive(is);
			Archive(flag);
		}
		switch (flag) {
		case 0: {
			_MESSAGE("received auth packet from %s:%u", hrIP, event.peer->address.port);
		}
		case 1: {
			_MESSAGE("A player position tracking packet of length %u was received from %s:%u:%u",
				event.packet->dataLength,
				hrIP,
				event.peer->address.port,
				event.channelID);
			//store position data
			{
				std::string tmpData(((char*)event.packet->data), event.packet->dataLength);
				updateActor(tmpData);
			}
			// Clean up the packet now that we're done using it
			enet_packet_destroy(event.packet);
			break;
		}
		}


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
		UInt32 cellID;
		if ((*g_thePlayer)->parentCell->worldSpace)
		{
			cellID = (*g_thePlayer)->parentCell->worldSpace->refID;
		}
		else
		{
			cellID = (*g_thePlayer)->parentCell->refID;
		}
		// send position using an unreliable packet
		std::ostringstream SData;
		{
			cereal::BinaryOutputArchive Archive(SData);
			Archive(OMPlayerPOS, (*g_thePlayer)->refID, cellID, (*g_thePlayer)->posX, (*g_thePlayer)->posY, (*g_thePlayer)->posZ, (*g_thePlayer)->rotX, (*g_thePlayer)->rotY, (*g_thePlayer)->rotZ);
		}
		std::string Out = SData.str();
		ENetPacket* packet = enet_packet_create(Out.c_str(), Out.size(), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
		// queue a packet for the peer over channel id 1
		enet_peer_send(peer, 1, packet);
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
				Archive(OMIdentity, SUPER_VERSION, MAIN_VERSION, SUB_VERSION, ServerPassword);
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