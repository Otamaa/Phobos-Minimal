#include "TheaterSpecificSHP.h"
#include "INIParser.h"

#include <Utilities/GeneralUtils.h>
#include <Utilities/SavegameDef.h>
#include <Utilities/TemplateDef.h>

bool TheaterSpecificSHP::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		auto pValue = parser.value();
		GeneralUtils::ApplyTheaterSuffixToString(pValue);
		std::string Result = _strlwr(pValue);

		if (Result.size() < 4 || !std::equal(Result.end() - 4, Result.end(), ".shp",
			[](char input, char expected) { return input == expected; }))
			Result += ".shp";

		if (auto const pImage = FileSystem::LoadSHPFile(Result.c_str())) {
			value = pImage;
			return true;
		} else {
			Debug::Log("Failed to find file %s referenced by [%s]%s=%s\n", Result.c_str(), pSection, pKey, pValue);
		}
	}

	return false;
}

bool TheaterSpecificSHP::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm.Process(this->value, RegisterForChange);
}

bool TheaterSpecificSHP::Save(PhobosStreamWriter& Stm) const
{
	return Stm.Process(this->value);
}

TheaterSpecificSHP::~TheaterSpecificSHP()
{
	GameDelete<true, true>(std::exchange(value, nullptr));
}