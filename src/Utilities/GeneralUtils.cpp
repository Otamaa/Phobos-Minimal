#include "GeneralUtils.h"
#include "Debug.h"
#include <ScenarioClass.h>
#include <Conversions.h>
#include <VocClass.h>

#include <Utilities/Cast.h>

#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>
#include <TranslateFixedPoints.h>

#include <Locomotor/CLSIDs.h>

#include "EventClass.h"

#include <Ext/Rules/Body.h>

#include <SuperClass.h>

int GeneralUtils::GetColorFromColorAdd(int colorIndex)
{
	auto const& colorAdd = RulesClass::Instance->ColorAdd;
	const int colorValue = GetColorIndexForColorAdd(colorIndex);

	if (RulesExtData::Instance()->ColorAddUse8BitRGB)
		return colorAdd[colorValue].ToInit();

	return GetColorFromColorAdd(colorAdd[colorValue]);
}

bool GeneralUtils::IsValidString(const char* str)
{
	if (str == nullptr || strlen(str) == 0 || GameStrings::IsBlank(str))
		return false;

	if (IS_SAME_STR_(str , DEFAULT_STR) || IS_SAME_STR_(str, DEFAULT_STR2))
		return false;

	return true;
}

bool GeneralUtils::IsValidString(const wchar_t* str)
{
	return str != nullptr && wcslen(str) != 0 && !wcsstr(str, L"MISSING:");
}

void GeneralUtils::IntValidCheck(int* source, const char* section, const char* tag, int defaultValue, int min, int max)
{
	if (defaultValue < min) defaultValue = min;
	if (defaultValue > max) defaultValue = max;

	if (*source < min || *source>max) {
		//Debug::LogInfo("[Developer warning][%s]%s=%d is invalid! Reset to %d.", section, tag, *source, defaultValue);
		*source = defaultValue;
	}
}

void GeneralUtils::DoubleValidCheck(double* source, const char* section, const char* tag, double defaultValue, double min, double max)
{
	if (defaultValue < min) defaultValue = min;
	if (defaultValue > max) defaultValue = max;

	if (*source < min || *source>max) {
		//Debug::LogInfo("[Developer warning][%s]%s=%f is invalid! Reset to %f.", section, tag, *source, defaultValue);
		*source = defaultValue;
	}
}

#include <Misc/Ares/CSF.h>

const wchar_t* GeneralUtils::LoadStringOrDefault(const char* key, const wchar_t* defaultValue)
{
	if (!GeneralUtils::IsValidString(key))
		return defaultValue;

	return StringTable::FetchString(key);
}

const wchar_t* GeneralUtils::LoadStringUnlessMissing(const char* key, const wchar_t* defaultValue)
{
	if (!GeneralUtils::IsValidString(key))
		return defaultValue;

	auto pCSF = CSFLoader::FindOrAllocateDynamicStrings(key);

	if (pCSF->IsMissingValue) {
		wcscpy_s(pCSF->Text, std::size(pCSF->Text), defaultValue);
		pCSF->IsMissingValue = false; // enforce it to false, since we dont want to do this operation again
	}

	return pCSF->Text;
}

const wchar_t* GeneralUtils::LoadStringUnlessMissingNoChecks(const char* key, const wchar_t* defaultValue)
{
	auto pCSF = CSFLoader::FindOrAllocateDynamicStrings(key);

	if (pCSF->IsMissingValue) {
		wcscpy_s(pCSF->Text, std::size(pCSF->Text), defaultValue);
		pCSF->IsMissingValue = false; // enforce it to false, since we dont want to do this operation again
	}

	return pCSF->Text;
}

void GeneralUtils::AdjacentCellsInRange(std::vector<CellStruct>& nCells, short range)
{
	nCells.reserve((2 * range + 1) * (2 * range + 1));
	for (CellSpreadEnumerator it(range); it; ++it)
		nCells.emplace_back(*it);
}

const double GeneralUtils::GetWarheadVersusArmor(WarheadTypeClass* pWH, Armor const ArmorType)
{
	const auto& verses = WarheadTypeExtContainer::Instance.Find(pWH)->GetVerses(ArmorType);
	return verses.Verses;
	//return double(FakeWarheadTypeClass::ModifyDamage(100, pWH, ArmorType, 0)) / 100.0;
}

const bool GeneralUtils::ProduceBuilding(HouseClass* pOwner, int idxBuilding)
{
	if (auto pItem = ObjectTypeClass::FetchTechnoType(AbstractType::BuildingType, idxBuilding))
	{
		if (pOwner->CanBuild(pItem, true, true) == CanBuildResult::Buildable)
		{
			if (pItem->FindFactory(true, true, true, pOwner))
			{
				const auto pBuilding = static_cast<BuildingTypeClass*>(pItem);

				if (pOwner->GetPrimaryFactory(AbstractType::Building, false, pBuilding->BuildCat))
					return false;

				EventClass vEvent { pOwner->ArrayIndex , EventType::PRODUCE , pItem->WhatAmI(), pItem->GetArrayIndex(), pItem->Naval };
				EventClass::AddEvent(&vEvent);

				return true;
			}
		}
	}

	return false;
}

AnimTypeClass* GeneralUtils::SelectRandomAnimFromVector(std::vector<AnimTypeClass*>& vec, AnimTypeClass* fallback)
{
	if (vec.empty())
		return fallback;

	if (vec.size() == 1)
		return vec[0];

	return vec[ScenarioClass::Instance->Random.RandomFromMax(vec.size() - 1)];
}

const char* GeneralUtils::GetLocomotionName(const CLSID& clsid)
{
	if (clsid == CLSIDs::Drive())
	{
		return "Drive";
	}
	else if (clsid == CLSIDs::Drive())
	{
		return "Fly";
	}
	else if (clsid == CLSIDs::Jumpjet())
	{
		return "Jumpjet";
	}
	else if (clsid == CLSIDs::DropPod())
	{
		return "DropPod";
	}
	else if (clsid == CLSIDs::Tunnel())
	{
		return "Tunnel";
	}
	else if (clsid == CLSIDs::Walk())
	{
		return "Walk";
	}
	else if (clsid == CLSIDs::Mech())
	{
		return "Mech";
	}
	else if (clsid == CLSIDs::Teleport())
	{
		return "Teleport";
	}
	else if (clsid == CLSIDs::Rocket())
	{
		return "Rocket";
	}
	else if (clsid == CLSIDs::Hover())
	{
		return "Hover";
	}
	else if (clsid == CLSIDs::Ship())
	{
		return "Ship";
	}

	return "<unknown>";
}

#include <New/Type/TheaterTypeClass.h>

bool GeneralUtils::ApplyTheaterSuffixToString(char* str)
{
	str = _strlwr(str);

	if (auto pSuffix = strstr(str, "~~~"))
	{
		std::string pTheater = TheaterTypeClass::Array.empty() ? Theater::Get(ScenarioClass::Instance->Theater)->Extension :
			TheaterTypeClass::FindFromTheaterType(ScenarioClass::Instance->Theater)->Extension.c_str();
		PhobosCRT::lowercase(pTheater);

		pSuffix[0] = pTheater[0];
		pSuffix[1] = pTheater[1];
		pSuffix[2] = pTheater[2];
		//Debug::LogInfo("Found designated string, Replacing [%s] to [%s] ", str, pSuffix);
		return true;
	}

	return false;
}

bool GeneralUtils::ApplyTheaterExtToString(std::string& flag)
{
	const auto nPos = flag.find("~");
	if (nPos != std::string::npos)
	{
		std::string pTheater = TheaterTypeClass::Array.empty() ? Theater::Get(ScenarioClass::Instance->Theater)->Letter:
			TheaterTypeClass::FindFromTheaterType(ScenarioClass::Instance->Theater)->Letter.c_str();

		PhobosCRT::lowercase(pTheater);

		flag.replace(nPos, 1, pTheater);
		return true;
	}

	return false;
}

std::string GeneralUtils::ApplyTheaterSuffixToString(const std::string& str)
{
	std::string buffer = str;

	const auto nPos = buffer.find("~~~");
	if (nPos != std::string::npos)
	{
		std::string pTheater = TheaterTypeClass::Array.empty() ? Theater::Get(ScenarioClass::Instance->Theater)->Extension
			: TheaterTypeClass::FindFromTheaterType(ScenarioClass::Instance->Theater)->Extension.c_str()
			;

		PhobosCRT::lowercase(pTheater);

		//only set the 3 characters without the terminator string
		buffer.replace(nPos, 3, pTheater);

		//Debug::LogInfo("Found designated string at [%d] Replacing [%s] to [%s] ",
		//	nPos, str.c_str(), buffer.c_str());

	}

	return buffer;
}

#pragma region Otamaa

AnimTypeClass* GeneralUtils::GetAnimFacingFromVector(TechnoClass* pFirer, const Iterator<AnimTypeClass*> iter)
{
	return iter.GetItemAtOrDefault(GeneralUtils::GetAnimIndexFromFacing(pFirer, iter.size()),nullptr);
}

const int GeneralUtils::GetAnimIndexFromFacing(TechnoClass* pFirer, int nVectorSize)
{
	if (!pFirer || nVectorSize <= 0)
		return -1;

	int idx = 0;
	int highest = Conversions::Int2Highest(nVectorSize);

	if (highest >= 3)
	{
		unsigned int offset = 1U << (highest - 3);
		idx = TranslateFixedPoint::Normal(16, highest, static_cast<WORD>(pFirer->GetRealFacing().Raw), offset);
	}

	return idx;
}

const int GeneralUtils::GetAnimIndexFromFacing(FootClass* pFoot, int nVectorSize)
{
	if (pFoot) {
		auto highest = Conversions::Int2Highest(nVectorSize);

		// 2^highest is the frame count, 3 means 8 frames
		if (highest >= 3)
		{
			auto offset = 1u << (highest - 3);
			return TranslateFixedPoint::Normal(16, highest, static_cast<WORD>(pFoot->GetRealFacing().GetValue<16>()), offset);
		}
	}

	return 0;
}

int GeneralUtils::GetLSAnimHeightFactor(AnimTypeClass* pType, CellClass* pCell, bool checklevel)
{
	int ImageHeignt = 0;
	if (auto pImage = pType->GetImage())
		ImageHeignt = pImage->Height;

	const auto Height = int(((double)ImageHeignt) / 2);
	const auto LSFactor = (Height - 0.5) * LightningStorm::CloudHeightFactor();

	return int(checklevel ? LSFactor + (double)Unsorted::LevelHeight * (double)pCell->Level : LSFactor);
}
#pragma endregion
