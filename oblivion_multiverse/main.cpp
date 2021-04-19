#include "main.h"

#if OBLIVION
#include "obse/GameAPI.h"
#else
#include "obse_editor/EditorAPI.h"
#endif

IDebugLog gLog("./Data/OBSE/Plugins/oblivion_multiverse.log");
PluginHandle g_pluginHandle = kPluginHandle_Invalid;
OBSEScriptInterface* g_scriptIntfc = NULL;
std::map<TESObjectREFR*, UInt32> mapTrackedActors;


/**********************
* Command functions
**********************/

bool Cmd_OMClientTick_Execute(COMMAND_ARGS)
{
	sendPlayerPOS();
	enetSyncHandler();
	return true;
}

bool Cmd_OMServerConnect_Execute(COMMAND_ARGS)
{

	return serverConnect();
}

bool Cmd_OMTrackActor_Execute(COMMAND_ARGS)
{
	if (!thisObj)
	{
		_MESSAGE("OMTrackActor: No actor reference given");
		return true;
	}
	if (thisObj->IsActor())
	{
		std::multimap<TESObjectREFR*, UInt32>::iterator it = mapTrackedActors.find(thisObj);
		if (it == mapTrackedActors.end()) {
			//actor not found. add
			Actor* ActorBuf = (Actor*)thisObj;
			mapTrackedActors[thisObj] = ActorBuf->refID;
			_MESSAGE("OMTrackActor: Now tracking actor");
			return true;
		}
		else {
			// conflict here
			_MESSAGE("OMTrackActor: Actor already tracked");
			return false;
		}
		//mapTrackedActors.erase(it);

	}
	return true;
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

static CommandInfo kOMClientTickCommand =
{
	"OMClientTick",
	"OMCT",
	0,
	"Perform tick",
	0,		// requires parent obj
	0,		// doesn't have params
	NULL,	// no param table
	Cmd_OMClientTick_Execute
};

static CommandInfo kOMTrackActorCommand =
{
	"OMTrackActor",
	"OMTA",
	0,
	"Track an actor",
	0,		// requires parent obj
	0,		// doesn't have params
	NULL,	// no param table
	Cmd_OMTrackActor_Execute
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
		serverDisconnect();
		discardClient();
		break;
	case OBSEMessagingInterface::kMessage_ExitToMainMenu:
		_MESSAGE("OblivionMultiverse received ExitToMainMenu message");
		serverDisconnect();
		discardClient();
		break;
	case OBSEMessagingInterface::kMessage_ExitGame_Console:
		_MESSAGE("OblivionMultiverse received quit game from console message");
		serverDisconnect();
		discardClient();
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
			g_scriptIntfc = (OBSEScriptInterface*)obse->QueryInterface(kInterface_Script);
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
		obse->RegisterCommand(&kOMClientTickCommand);
		obse->RegisterCommand(&kOMTrackActorCommand);

		//load ini
		OMLoadConfig();

		//initialize enet library
		if (enet_initialize() != 0)
		{
			_MESSAGE("ENet library initialization failed");
		}

		if (!obse->isEditor)
		{
			// register to use string var interface
			// this allows plugin commands to support '%z' format specifier in format string arguments
			OBSEStringVarInterface* g_Str = (OBSEStringVarInterface*)obse->QueryInterface(kInterface_StringVar);
			g_Str->Register(g_Str);
		}

		// register to receive messages from OBSE
		OBSEMessagingInterface* msgIntfc = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
		msgIntfc->RegisterListener(g_pluginHandle, "OBSE", MessageHandler);
		g_msg = msgIntfc;

		return true;
	}

};
