#include "GeneralUtils.h"
#include "Debug.h"
#include <ScenarioClass.h>
#include <Conversions.h>
#include <Networking.h>
#include <NetworkEvents.h>
#include <VocClass.h>

#include <Utilities/Cast.h>

#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>
#include <TranslateFixedPoints.h>

#include <Locomotor/CLSIDs.h>

bool GeneralUtils::IsValidString(const char* str)
{
	if (str == nullptr || strlen(str) == 0 || GameStrings::IsBlank(str))
		return false;

	if (IS_SAME_STR_(str , DEFAULT_STR) || IS_SAME_STR_(str, DEFAULT_STR2))
		return false;

	return true;
}

void GeneralUtils::IntValidCheck(int* source, const char* section, const char* tag, int defaultValue, int min, int max)
{
	if (*source < min || *source>max)
	{
		//Debug::Log("[Developer warning][%s]%s=%d is invalid! Reset to %d.\n", section, tag, *source, defaultValue);
		*source = defaultValue;
	}
}

void GeneralUtils::DoubleValidCheck(double* source, const char* section, const char* tag, double defaultValue, double min, double max)
{
	if (*source < min || *source>max)
	{
		//Debug::Log("[Developer warning][%s]%s=%f is invalid! Reset to %f.\n", section, tag, *source, defaultValue);
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

void GeneralUtils::AdjacentCellsInRange(std::vector<CellStruct>& nCells, size_t range)
{
	nCells.clear();

	for (CellSpreadEnumerator it(range); it; ++it)
		nCells.push_back(*it);
}

const double GeneralUtils::GetWarheadVersusArmor(WarheadTypeClass* pWH, Armor const ArmorType)
{
	const auto& verses = WarheadTypeExt::ExtMap.Find(pWH)->GetVerses(ArmorType);
	return verses.Verses;
}

const bool GeneralUtils::ProduceBuilding(HouseClass* pOwner, int idxBuilding)
{
	if (auto pItem = ObjectTypeClass::FetchTechnoType(AbstractType::BuildingType, idxBuilding))
	{
		if (pOwner->CanBuild(pItem, true, true) == CanBuildResult::Buildable)
		{
			if (pItem->FindFactory(true, true, true, pOwner))
			{
				const auto pBuilding = type_cast<BuildingTypeClass*>(pItem);

				if (pOwner->GetPrimaryFactory(AbstractType::Building, false, pBuilding->BuildCat))
					return false;

				NetworkEvent vEvent {};

				VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, Panning::Center, 1.0);
				vEvent.FillEvent_ProduceAbandonSuspend(
					pOwner->ArrayIndex, NetworkEventType::Produce, pItem->WhatAmI(), pItem->GetArrayIndex(), pItem->Naval
				);

				Networking::AddEvent(&vEvent);

				return true;
			}
		}
	}

	return false;
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

	if (auto pSuffix = CRT::strstr(str, "~~~"))
	{
		std::string pTheater =
			TheaterTypeClass::FindFromTheaterType(ScenarioClass::Instance->Theater)->Extension.c_str();
		pTheater = GeneralUtils::lowercase(pTheater);
		pSuffix[0] = pTheater[0];
		pSuffix[1] = pTheater[1];
		pSuffix[2] = pTheater[2];
		//Debug::Log("Found designated string, Replacing [%s] to [%s] \n", str, pSuffix);
		return true;
	}

	return false;
}

bool GeneralUtils::ApplyTheaterExtToString(std::string& flag)
{
	const auto nPos = flag.find("~");
	if (nPos != std::string::npos)
	{
		std::string pTheater =
			TheaterTypeClass::FindFromTheaterType(ScenarioClass::Instance->Theater)->Letter.c_str();
		pTheater = GeneralUtils::lowercase(pTheater);

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
		std::string pTheater =
			TheaterTypeClass::FindFromTheaterType(ScenarioClass::Instance->Theater)->Extension.c_str();
		pTheater = GeneralUtils::lowercase(pTheater);

		//only set the 3 characters without the terminator string
		buffer.replace(nPos, 3, pTheater);

		//Debug::Log("Found designated string at [%d] Replacing [%s] to [%s] \n",
		//	nPos, str.c_str(), buffer.c_str());

	}

	return buffer;
}

#pragma region Otamaa
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

	return int(checklevel ? LSFactor + Unsorted::LevelHeight * pCell->Level : LSFactor);
}
#pragma endregion
