#include "om_config.h"

void OMLoadConfig()
{
	//config file
	CSimpleIniA ini;
	SI_Error ie = ini.LoadFile("oblivion_multiverse.ini");

	if (!ie) {
		// get all sections
		CSimpleIniA::TNamesDepend sections;
		ini.GetAllSections(sections);

		// get all keys in main section
		CSimpleIniA::TNamesDepend keys;
		ini.GetAllKeys("MAIN", keys);

		// get port
		const char* pValue = ini.GetValue("MAIN",
			"PORT", "41805");
		ServerPort = atoi(pValue);

		// get max clients
		const char* mcValue = ini.GetValue("MAIN",
			"MAX_CLIENTS", "32");
		MaxClients = atoi(mcValue);

		// get password
		const char* pwdValue = ini.GetValue("MAIN",
			"PASSWORD", NULL);
		if (strlen(pwdValue) > 31) {
			strncpy(ServerPassword, pwdValue, 32);
			ServerPassword[31] = '\0';   /* null character manually added */
		}
		else {
			strncpy(ServerPassword, pwdValue, strlen(pwdValue));
		}
	}
	//if we don't have a config, make it
	else {
		//initial serverPort
		ServerPort = 41805;
		char buffer[33];
		snprintf(buffer, 12, "%d", ServerPort);
		ini.SetValue("MAIN", "PORT", buffer);

		//initial max clients
		MaxClients = 32;
		char buffer_mc[3];
		snprintf(buffer_mc, 12, "%d", MaxClients);
		ini.SetValue("MAIN", "MAX_CLIENTS", buffer_mc);

		//initial password
		ini.SetValue("MAIN", "PASSWORD", "");

		// write initial ini
		long rc = ini.SaveFile("oblivion_multiverse.ini");
	}
}

void OMUpdateConfig()
{
	//config file
	CSimpleIniA ini;
	SI_Error ie = ini.LoadFile("./oblivion_multiverse.ini");

	if (!ie) {
		//update serverPort
		char buffer[33];
		snprintf(buffer, 12, "%d", ServerPort);
		ini.SetValue("MAIN", "PORT", buffer);

		//update max clients
		char buffer_mc[3];
		snprintf(buffer_mc, 12, "%d", MaxClients);
		ini.SetValue("MAIN", "MAX_CLIENTS", buffer_mc);

		//update password
		ini.SetValue("MAIN", "PASSWORD", ServerPassword);

		// write ini
		long rc = ini.SaveFile("oblivion_multiverse.ini");
	}
}