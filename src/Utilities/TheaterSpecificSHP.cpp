#include "TheaterSpecificSHP.h"
#include "INIParser.h"
#include "SHPUtils.h"

#include <Utilities/GeneralUtils.h>
#include <Utilities/SavegameDef.h>

bool TheaterSpecificSHP::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey) > 0)
	{
		auto pValue = parser.value();
		
		// Use the new utility function for better fallback handling
		if (auto const pImage = SHPUtils::LoadTheaterSHPWithFallback(pValue))
		{
			value = pImage;
			return true;
		}
		else
		{
			Debug::LogInfo("Failed to find theater-specific SHP file '{}' referenced by [{}]{}={}", pValue, pSection, pKey, pValue);
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