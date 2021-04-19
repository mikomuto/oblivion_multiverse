#include "oblivion_multiverse_server.h"

// Global Server Variables
bool bServerAlive = true;
ENetAddress address;
ENetHost* server;
ENetEvent event;
std::map<enet_uint32, bool> mapIsAuth;
std::map<enet_uint32, std::string> mapActors;

FILE* serverlog;
errno_t err;

void serverOutput(char* message)
{
	//output to console
	printf(message);
	//output to logfile
	if ((err = fopen_s(&serverlog, "oblivion_multiverse_server_log.txt", "a")) == 0)
	{
		fprintf(serverlog, "%s", message);
		fclose(serverlog);
	}
}

void syncActors() {
	char message[32] = "";
	while (bServerAlive) {
		//update approx 60 times per second
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
		//send actor updates to all auth clients
		for (auto& x : mapActors)
		{
			if (x.first == 0) {
				return;
			}
			//send to all connected peers
			//TODO: send only to nearby players
			ENetPacket* packet = enet_packet_create(x.second.c_str(), x.second.size(), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
			enet_host_broadcast(server, 1, packet);
			//enet_host_flush(server);
		}
	}
}

int main()
{
	//wipe log file
	if ((err = fopen_s(&serverlog, "oblivion_multiverse_server_log.txt", "w")) == 0)
	{
		fclose(serverlog);
	}

	//load ini
	OMLoadConfig();

	//initialize enet library
	if (enet_initialize() != 0)
	{
		char message[] = "ENet library initialization failed\n";
		serverOutput(message);
		return EXIT_FAILURE;
	}
	else {
		char message[] = "ENet library initialization complete\n";
		serverOutput(message);
	}

	/* Bind the server to the default localhost.     */
	/* A specific host address can be specified by   */
	/* enet_address_set_host (& address, "x.x.x.x"); */
	address.host = ENET_HOST_ANY;
	/* Bind the server to port */
	address.port = ServerPort;

	//create server
	server = enet_host_create(&address /* the address to bind the server host to */,
		MaxClients      /* allow up to x clients and/or outgoing connections */,
		2      /* allow up to 2 channels to be used, 0 and 1 */,
		0      /* assume any amount of incoming bandwidth */,
		0      /* assume any amount of outgoing bandwidth */);

	//check result
	if (server == NULL)
	{
		char message[] = "Server initialization failed\n";
		serverOutput(message);
		return EXIT_FAILURE;
	}

	char message[128]{};
	sprintf_s(message, "Oblivion Multiverse Server %i.%i.%i \"%s\"\n   %s\n--------------------------\n", SUPER_VERSION, MAIN_VERSION, SUB_VERSION, RELEASE_CODENAME, RELEASE_COMMENT);
	serverOutput(message);

	//start actor sync thread
	std::thread myThread(syncActors);

	while (bServerAlive)
	{
		//handle incoming packets
		while (enet_host_service(server, &event, 5000) > 0) {
			//convert event host ip address to human readable
			char hrIP[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(event.peer->address.host), hrIP, INET_ADDRSTRLEN);

			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT: {
				sprintf_s(message, "A new client connected from %s:%u\n", hrIP, event.peer->address.port);
				serverOutput(message);
				break;
			}
			case ENET_EVENT_TYPE_RECEIVE: {
				//check if authenticated
				std::map<enet_uint32, bool>::iterator itr = mapIsAuth.find(event.peer->address.host);
				if (itr == mapIsAuth.end()) {
					sprintf_s(message, "%s:%u is not authenticated\n", hrIP, event.peer->address.port);
					serverOutput(message);
				}
				int flag;
				int cSuperVersion;
				int cMainVersion;
				int cSubVersion;
				char cPassword[32] = "";
				//set packet flag
				{
					std::string tmpData(((char*)event.packet->data), event.packet->dataLength);
					std::istringstream is(tmpData);
					cereal::BinaryInputArchive Archive(is);
					Archive(flag);
				}
				switch (flag) {
				case 0: {
					sprintf_s(message, "received auth packet from %s:%u\n", hrIP, event.peer->address.port);
					serverOutput(message);
					{
						std::string tmpData(((char*)event.packet->data), event.packet->dataLength);
						std::istringstream is(tmpData);
						cereal::BinaryInputArchive Archive(is);
						Archive(flag, cSuperVersion, cMainVersion, cSubVersion, cPassword);
					}
					//sprintf_s(message, "cereal did it's thing\n");
					//serverOutput(message);
					if (cSuperVersion == SUPER_VERSION && cMainVersion == MAIN_VERSION && cSubVersion == SUB_VERSION) {
						//sprintf_s(message, "correct version\n");
						//serverOutput(message);
						if (strcmp(cPassword, ServerPassword) == 0) {
							//sprintf_s(message, "correct password\n");
							//serverOutput(message);
							sprintf_s(message, "%s:%u is now authenticated\n", hrIP, event.peer->address.port);
							serverOutput(message);
							//add host to auth map
							mapIsAuth.insert(std::pair<enet_uint32, bool>(event.peer->address.host, true));;
							// Clean up the packet now that we're done using it
							enet_packet_destroy(event.packet);
							break;
						}
						else {
							//TODO send client error message before disconnet
							sprintf_s(message, "%s:%u Wrong password\n", hrIP, event.peer->address.port);
							serverOutput(message);
							enet_peer_disconnect(event.peer, 0);
							// Clean up the packet now that we're done using it
							enet_packet_destroy(event.packet);
							break;
						}
					}
					else {
						//TODO send client error message before disconnet
						sprintf_s(message, "%s:%u Version mismatch\n", hrIP, event.peer->address.port);
						serverOutput(message);
						enet_peer_disconnect(event.peer, 0);
						// Clean up the packet now that we're done using it
						enet_packet_destroy(event.packet);
						break;
					}
				case 1: {
					/*sprintf_s(message, "A player position tracking packet of length %u was received from %s:%u:%u\n",
						event.packet->dataLength,
						hrIP,
						event.peer->address.port,
						event.channelID);
					serverOutput(message);*/
					//store position data
					{
						std::string tmpData(((char*)event.packet->data), event.packet->dataLength);
						mapActors.insert(std::pair<enet_uint32, std::string>(event.peer->address.host, tmpData));
					}
					// Clean up the packet now that we're done using it
					enet_packet_destroy(event.packet);
					break;
				}
				default: {
					sprintf_s(message, "A unkown packet of length %u containing %s was received from %s:%u:%u\n",
						event.packet->dataLength,
						event.packet->data,
						hrIP,
						event.peer->address.port,
						event.channelID);
					serverOutput(message);
					// Clean up the packet now that we're done using it
					enet_packet_destroy(event.packet);
					break;
				}
				}
				}
			}
			case ENET_EVENT_TYPE_DISCONNECT: {
				sprintf_s(message, "%s:%u:%u disconnected\n", hrIP, event.peer->address.port, event.channelID);
				serverOutput(message);
				/* Reset the peer's client information. */
				event.peer->data = NULL;

			}
			}
		}
	}
}

void serverStop() {
	char message[] = "Shutdown request received\n";
	serverOutput(message);
	bServerAlive = false;

	enet_host_destroy(server);
	enet_deinitialize();
}
