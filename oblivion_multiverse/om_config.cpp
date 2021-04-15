#include "om_config.h"

char ServerPassword[32];
char ServerAddress[16];
unsigned short ServerPort = 0;
int MaxPlayers = 0;

void OMLoadConfig()
{
	//config file
	CSimpleIniA ini;
	SI_Error ie = ini.LoadFile("./Data/OBSE/Plugins/oblivion_multiverse.ini");

	if (!ie) {
		_MESSAGE("Loading ini file");
		// get all sections
		CSimpleIniA::TNamesDepend sections;
		ini.GetAllSections(sections);

		// get all keys in main section
		CSimpleIniA::TNamesDepend keys;
		ini.GetAllKeys("MAIN", keys);

		// get ip addr
		const char* ipValue = ini.GetValue("MAIN",
			"SERVER_ADDRESS", "127.0.0.1");
		strncpy_s(ServerAddress, ipValue, 16);

		// get port
		const char* pValue = ini.GetValue("MAIN",
			"PORT", "41805");
		ServerPort = atoi(pValue);

		// get password
		const char* pwdValue = ini.GetValue("MAIN",
			"PASSWORD", NULL);
		if (strlen(pwdValue) > 31) {
			strncpy_s(ServerPassword, pwdValue, 32);
			ServerPassword[31] = '\0';   /* null character manually added */
		}
		else {
			strncpy_s(ServerPassword, pwdValue, strlen(pwdValue));
		}

		// get max players to sync
		const char* maValue = ini.GetValue("MAIN",
			"MAX_PLAYERS", "12");
		MaxPlayers = atoi(maValue);
		// fix max actors if over 32
		if (MaxPlayers > 32) {
			MaxPlayers = 32;
		}
	}
	//if we don't have a config, make it
	else {
		_MESSAGE("Creating initial ini file");
		//initial IP address
		strncpy_s(ServerAddress, "127.0.0.1", 16);
		ini.SetValue("MAIN", "SERVER_ADDRESS", "127.0.0.1");

		//initial serverPort
		ServerPort = 41805;
		char buffer[33];
		_itoa_s(ServerPort, buffer, 10);
		ini.SetValue("MAIN", "PORT", buffer);

		//initial password
		ini.SetValue("MAIN", "PASSWORD", "");

		//initial max actors
		MaxPlayers = 12;
		char buffer_mc[3];
		_itoa_s(MaxPlayers, buffer_mc, 10);
		ini.SetValue("MAIN", "MAX_PLAYERS", buffer_mc);

		// write initial ini
		long rc = ini.SaveFile("./Data/OBSE/Plugins/oblivion_multiverse.ini");
	}
}

void OMUpdateConfig()
{
	//config file
	CSimpleIniA ini;
	SI_Error ie = ini.LoadFile("./Data/OBSE/Plugins/oblivion_multiverse.ini");

	if (!ie) {
		//update IP address
		ini.SetValue("MAIN", "SERVER_ADDRESS", ServerAddress);

		//update serverPort
		char buffer[33];
		_itoa_s(ServerPort, buffer, 10);
		ini.SetValue("MAIN", "PORT", buffer);

		//update password
		ini.SetValue("MAIN", "PASSWORD", ServerPassword);

		//update max players
		char buffer_mc[3];
		_itoa_s(MaxPlayers, buffer_mc, 10);
		ini.SetValue("MAIN", "MAX_PLAYERS", buffer_mc);

		// write ini
		long rc = ini.SaveFile("./Data/OBSE/Plugins/oblivion_multiverse.ini");
	}
}