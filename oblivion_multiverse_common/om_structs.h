#pragma once

struct clientIdentity {
	char header = 'OM';
	int SUPER_VERSION;
	int MAIN_VERSION;
	int SUB_VERSION;
	char ServerPassword[32];

	// This method lets cereal know which data members to serialize
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(header, SUPER_VERSION, MAIN_VERSION, SUB_VERSION, ServerPassword); // serialize things by passing them to the archive
	}
};
