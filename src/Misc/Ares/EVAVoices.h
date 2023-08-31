#pragma once

#include <VoxClass.h>

#include <vector>

struct VoxFile
{
	char Name[9];

	//need to define a == operator so it can be used in array classes
	bool operator == (const VoxFile &other) const
	{
		return !CRT::strcmp(this->Name, other.Name);
	}
};

class VoxClass2 : public VoxClass
{
public:
	VoxClass2(char* pName) : VoxClass(pName) {}

	~VoxClass2() = default;

	std::vector<VoxFile> Voices;
};

class EVAVoices
{
public:
	static int FindIndexById(const char* type);

	// adds the EVA type only if it doesn't exist
	static void RegisterType(const char* type);

	static std::vector<const char*> Types;
};
