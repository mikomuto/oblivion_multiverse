#include "oblivion_multiverse_server.h"


// Global Server Variables
bool bServerAlive = true;
ENetAddress address;
ENetHost* server;
ENetEvent event;

FILE* serverlog;
errno_t err;

void serverOutput(char* message)
{
	//output to console
	printf(message);
	//output to logfile
	if ((err = fopen_s(&serverlog, "oblivion_multiverse_server_log.txt", "a")) != 0)
	{

	}
	else {
		fprintf(serverlog, "%s", message);
		fclose(serverlog);
	}
}

int main()
{
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
	/* Bind the server to port 41805. */
	address.port = 41805;

	//create server
	server = enet_host_create(& address /* the address to bind the server host to */,
		32      /* allow up to 32 clients and/or outgoing connections */,
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

	while (bServerAlive)
	{
		while (enet_host_service(server, &event, 10000) > 0) {
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				sprintf_s(message, "A new client connected from %x:%u.\n", event.peer->address.host, event.peer->address.port);
				serverOutput(message);
				/* Store any relevant client information here. */
				//event.peer->data = "Client information";
				puts("Connection succeeded.");
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				sprintf_s(message, "A packet of length %u containing %s was received from %s on channel %u.\n",
					event.packet->dataLength,
					event.packet->data,
					event.peer->data,
					event.channelID);
				serverOutput(message);
				/* Clean up the packet now that we're done using it. */
				enet_packet_destroy(event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				sprintf_s(message, "%s disconnected\n", event.peer->data);
				serverOutput(message);
				/* Reset the peer's client information. */
				event.peer->data = NULL;
			}
		}
	}

	enet_host_destroy(server);
	enet_deinitialize();
}

void serverStop() {

	char message[] = "Shutdown request received\n";
	serverOutput(message);
}
