#include "om_actors.h""

std::map<TESObjectREFR*, UInt32> mapSpawndActors;

void runscript(Actor* tmpActor, Script* tmpScript) {
	typedef OBSEArrayVarInterface::Element	OBSEElement;
	// call the function
	OBSEElement funcResult;
	if (g_scriptIntfc->CallFunction(tmpScript, tmpActor, NULL, &funcResult, 0)) {
		if (funcResult.GetType() == funcResult.kType_String) {
			_MESSAGE("runscript returned string %s", funcResult.String());
		}
		else {
			_MESSAGE("runscript did not return a string");
		}
	}
	else {
		_MESSAGE("runscript could not call function script");
	}
}

bool updateActor(std::string tmpData) {
	int flag;
	UInt32 refID;
	UInt32 cellID;
	float posX, posY, posZ, rotX, rotY, rotZ;

	std::istringstream is(tmpData);
	cereal::BinaryInputArchive Archive(is);
	Archive(flag, refID, cellID, posX, posY, posZ, rotX, rotY, rotZ);

	//get actor from baseFormID
	Actor* tmpActor = NULL;
	//tmpActor = reinterpret_cast<Actor*> (LookupFormByID(refID));
	tmpActor = (Actor*)LookupFormByID(refID);
	if (!tmpActor->IsActor()) {
		_MESSAGE("tmpActor refID %08x not found", refID);
		return false;
	}
	_MESSAGE("tmpActor refID: %08x", refID);
	_MESSAGE("tmpActor Name: %s", tmpActor->GetName());

	std::multimap<TESObjectREFR*, UInt32>::iterator it = mapSpawndActors.find(tmpActor);
	if (it == mapSpawndActors.end()) {
		//actor not found. spawn and add
		mapSpawndActors.insert(std::pair<TESObjectREFR*, UInt32>(tmpActor, refID));
	}

	//convert UInt32 to string
	char string[9];
	sprintf(string, "%08x", refID);

	std::string ScriptString;
	ScriptString = "player.placeatme ";
	ScriptString += string;
	Script* tmpScript = (Script*)ScriptString.c_str();
	_MESSAGE("Injected script: \"%s\"", ScriptString.c_str());
	runscript(tmpActor, tmpScript);

	//std::string ScriptString;
	//ScriptString = tmpActor->GetEditorName();
	//ScriptString += ".MoveTo";
	//ScriptString += posX;
	//ScriptString += ",";
	//ScriptString += posY;
	//ScriptString += ",";
	//ScriptString += posZ;
	//ScriptString += ",";
	//ScriptString += rotZ;
	//ScriptString += ",";
	//ScriptString += cellID;
	//Script* tmpScript = (Script*)ScriptString.c_str();
	//_MESSAGE("Injected script: \" %s! \"", ScriptString.c_str());
	//runscript(tmpActor, tmpScript);

}
