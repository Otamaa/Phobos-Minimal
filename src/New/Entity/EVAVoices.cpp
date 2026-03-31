#include "EVAVoices.h"

#include <Phobos.Defines.h>
#include <GameStrings.h>

std::vector<std::string> EVAVoices::Types;

int EVAVoices::FindIndexById(const char* type)
{
	//Debug::LogInfo("[Phobos] Find EVAVoices Index by ID [%s]", type);
	// the default values
	if (IS_SAME_STR_(type, GameStrings::Allied()))
	{
		return 0;
	}
	else if (IS_SAME_STR_(type, GameStrings::Russian()))
	{
		return 1;
	}
	else if (IS_SAME_STR_(type, GameStrings::Yuri()))
	{
		return 2;
	}

	// find all others
	for (size_t i = 0; i < Types.size(); ++i)
	{
		if (IS_SAME_STR_(type, Types[i].c_str()))
		{
			return static_cast<int>(i + 3);
		}
	}

	// not found
	return -1;
}

// adds the EVA type only if it doesn't exist
void EVAVoices::RegisterType(const char* type)
{
	int index = EVAVoices::FindIndexById(type);

	if (index < 0)
	{
		char* str = _strdup(type);
		Types.emplace_back(str);
		free(str);
	}
}
