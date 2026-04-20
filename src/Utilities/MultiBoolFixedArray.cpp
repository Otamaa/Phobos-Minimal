#include "MultiBoolFixedArray.h"
#include "Savegame.h"

#include "EnumFunctions.h"
#include "Parser.h"
#include "INIParser.h"

void MultiBoolFixedArray<(size_t)PhobosAbilityType::count>::Read(
	INI_EX& parser,
	const char* const pSection,
	const char* const pKey,
	std::array<const char*, (size_t)PhobosAbilityType::count>& nKeysArray)
{
	if (parser.ReadString(pSection, pKey) > 0)
	{
		Reset();
		char* context = nullptr;

		for (char* cur = strtok_s(parser.value(), Phobos::readDelims, &context);
			cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			for (size_t i = 0; i < (size_t)PhobosAbilityType::count; ++i)
			{
				if (IS_SAME_STR_(cur, nKeysArray[i]))
				{
					Set(i, true);
					break;
				}
			}
		}
	}
}

bool MultiBoolFixedArray<(size_t)PhobosAbilityType::count>::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	Reset();

	for (size_t i = 0; i < (size_t)PhobosAbilityType::count; ++i)
	{
		bool value = false;
		if (!Stm.Process(value, RegisterForChange))
			return false;
		Set(i, value);
	}

	return true;
}

bool MultiBoolFixedArray<(size_t)PhobosAbilityType::count>::Save(PhobosStreamWriter& Stm) const
{
	for (size_t i = 0; i < (size_t)PhobosAbilityType::count; ++i)
	{
		bool value = Get(i);
		if (!Stm.Process(value))
			return false;
	}

	return true;
}