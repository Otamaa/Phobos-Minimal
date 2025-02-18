#include "TheaterSpecificSHP.h"
#include "INIParser.h"

#include <Utilities/GeneralUtils.h>
#include <Utilities/SavegameDef.h>

bool TheaterSpecificSHP::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey) > 0)
	{
		auto pValue = parser.value();
		GeneralUtils::ApplyTheaterSuffixToString(pValue);

		std::string Result = pValue;
		if (!strstr(pValue, ".shp"))
			Result += ".shp";

		if (auto const pImage = FileSystem::LoadSHPFile(Result.c_str()))
		{
			value = pImage;
			return true;
		}
		else
		{
			Debug::LogInfo("Failed to find file {} referenced by [{}]{}={}", Result.c_str(), pSection, pKey, pValue);
		}
	}

	return false;
}

bool TheaterSpecificSHP::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Savegame::ReadPhobosStream(Stm, this->value, RegisterForChange);
}

bool TheaterSpecificSHP::Save(PhobosStreamWriter& Stm) const
{
	return Savegame::WritePhobosStream(Stm, this->value);
}

TheaterSpecificSHP::~TheaterSpecificSHP()
{
	GameDelete<true, true>(std::exchange(value, nullptr));
}