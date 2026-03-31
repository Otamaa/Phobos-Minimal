#pragma once

#include <vector>
#include <string>

class EVAVoices
{
public:
	static int FindIndexById(const char* type);

	// adds the EVA type only if it doesn't exist
	static void RegisterType(const char* type);

	static std::vector<std::string> Types;
};
