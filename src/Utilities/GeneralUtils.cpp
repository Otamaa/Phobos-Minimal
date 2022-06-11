#include "GeneralUtils.h"
#include "Debug.h"
#include <ScenarioClass.h>
#include <Conversions.h>

#include <Ext/Techno/Body.h>

bool GeneralUtils::IsValidString(const char* str)
{
	return str != nullptr
		&& CRT::strlen(str) != 0
		&& !INIClass::IsBlank(str)
		&& CRT::strcmpi(DEFAULT_STR, str)
		&& CRT::strcmpi(DEFAULT_STR2, str)
		;
}

void GeneralUtils::IntValidCheck(int* source, const char* section, const char* tag, int defaultValue, int min, int max)
{
	if (*source < min || *source>max)
	{
		Debug::Log("[Developer warning][%s]%s=%d is invalid! Reset to %d.\n", section, tag, *source, defaultValue);
		*source = defaultValue;
	}
}

void GeneralUtils::DoubleValidCheck(double* source, const char* section, const char* tag, double defaultValue, double min, double max)
{
	if (*source < min || *source>max)
	{
		Debug::Log("[Developer warning][%s]%s=%f is invalid! Reset to %f.\n", section, tag, *source, defaultValue);
		*source = defaultValue;
	}
}

const wchar_t* GeneralUtils::LoadStringOrDefault(const char* key, const wchar_t* defaultValue)
{
	if (GeneralUtils::IsValidString(key))
		return StringTable::LoadString(key);
	else
		return defaultValue;
}

const wchar_t* GeneralUtils::LoadStringUnlessMissing(const char* key, const wchar_t* defaultValue)
{
	return wcsstr(LoadStringOrDefault(key, defaultValue), L"MISSING:") ? defaultValue : LoadStringOrDefault(key, defaultValue);
}

std::vector<CellStruct> GeneralUtils::AdjacentCellsInRange(unsigned int range)
{
	std::vector<CellStruct> result;

	for (CellSpreadEnumerator it(range); it; ++it)
		result.push_back(*it);

	return result;
}

const double GeneralUtils::GetWarheadVersusArmor(WarheadTypeClass* pWH, Armor const ArmorType)
{
	return double(MapClass::GetTotalDamage(100, pWH, ArmorType, 0)) / 100.0;
}

#pragma region Otamaa
const int GeneralUtils::GetAnimIndexFromFacing(FootClass* pFoot, int nVectorSize)
{
	int index = 0;
	if (pFoot)
	{
		auto highest = Conversions::Int2Highest(nVectorSize);

		// 2^highest is the frame count, 3 means 8 frames
		if (highest >= 3)
		{
			auto offset = 1u << (highest - 3);
			index = TranslateFixedPoint(16, highest, static_cast<WORD>(pFoot->GetRealFacing().current().value()), offset);
		}
	}

	return index;
}
#pragma endregion
