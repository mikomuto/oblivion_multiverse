#include "main.h"

#if OBLIVION
#include "obse/GameAPI.h"
#else
#include "obse_editor/EditorAPI.h"
#endif

IDebugLog		gLog("./Data/OBSE/Plugins/oblivion_multiverse.log");

PluginHandle				g_pluginHandle = kPluginHandle_Invalid;


/**********************
* Command functions
**********************/

bool Cmd_OMServerConnect_Execute(COMMAND_ARGS)
{
	return serverConnect();
}


/**************************
* Command definitions
**************************/

static CommandInfo kOMServerConnectCommand =
{
	"OMServerConnect",
	"OMSC",
	0,
	"Connect to server",
	0,		// requires parent obj
	0,		// doesn't have params
	NULL,	// no param table
	Cmd_OMServerConnect_Execute
};


/*************************
	Messaging API
*************************/

OBSEMessagingInterface* g_msg;

void MessageHandler(OBSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case OBSEMessagingInterface::kMessage_LoadGame:
		_MESSAGE("OblivionMultiverse received LoadGame mesage");
		initializeClient();
		break;
	case OBSEMessagingInterface::kMessage_ExitGame:
		_MESSAGE("OblivionMultiverse received ExitGame message");
		discardClient();
		break;
	case OBSEMessagingInterface::kMessage_ExitToMainMenu:
		_MESSAGE("OblivionMultiverse received ExitToMainMenu message");
		discardClient();
		break;
	case OBSEMessagingInterface::kMessage_ExitGame_Console:
		_MESSAGE("OblivionMultiverse received quit game from console message");
		break;
	default:
		//_MESSAGE("OblivionMultiverse received unknown message");
		break;
	}
}

extern "C" {

	bool OBSEPlugin_Query(const OBSEInterface* obse, PluginInfo* info)
	{
		_MESSAGE("OBSE queried this plugin");

		// fill out the info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "oblivion_multiverse";
		info->version = 1;

		// version checks
		if (!obse->isEditor)
		{
			if (obse->obseVersion < OBSE_VERSION_INTEGER)
			{
				_ERROR("OBSE version too old (got %u expected at least %u)", obse->obseVersion, OBSE_VERSION_INTEGER);
				return false;
			}

#if OBLIVION
			if (obse->oblivionVersion != OBLIVION_VERSION)
			{
				_ERROR("incorrect Oblivion version (got %08X need %08X)", obse->oblivionVersion, OBLIVION_VERSION);
				return false;
			}
#endif

		}
		else
		{
			// no version checks needed for editor
		}

		// version checks pass

		return true;
	}

	bool OBSEPlugin_Load(const OBSEInterface* obse)
	{
		_MESSAGE("Plugin loading");

		g_pluginHandle = obse->GetPluginHandle();

		// register commands
		obse->SetOpcodeBase(0x2000); //TODO set release OpcodeBase
		obse->RegisterCommand(&kOMServerConnectCommand);

		//load ini
		OMLoadConfig;

		//initialize enet library
		if (enet_initialize() != 0)
		{
			_MESSAGE("ENet library initialization failed\n");
		}

		// register to receive messages from OBSE
		OBSEMessagingInterface* msgIntfc = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
		msgIntfc->RegisterListener(g_pluginHandle, "OBSE", MessageHandler);
		g_msg = msgIntfc;

		// get command table, if needed
		OBSECommandTableInterface* cmdIntfc = (OBSECommandTableInterface*)obse->QueryInterface(kInterface_CommandTable);
		if (cmdIntfc) {
#if 0	// enable the following for loads of log output
			for (const CommandInfo* cur = cmdIntfc->Start(); cur != cmdIntfc->End(); ++cur) {
				_MESSAGE("%s", cur->longName);
			}
#endif
		}
		else {
			_MESSAGE("Couldn't read command table");
		}
		return true;
	}

};
