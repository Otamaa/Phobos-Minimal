#include "NewSWType.h"

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>

#include "SonarPulse.h"
#include "UnitDelivery.h"
#include "GenericWarhead.h"
#include "Firewall.h"
#include "Protect.h"
#include "Reveal.h"
#include "ParaDrop.h"
#include "SpyPlane.h"
#include "ChronoSphere.h"
#include "ChronoWarp.h"
#include "GeneticMutator.h"
#include "Dominator.h"
#include "LightningStorm.h"
#include "NuclearMissile.h"
#include "HunterSeeker.h"
#include "DropPod.h"
#include "EMPulse.h"
#include "Battery.h"
#include "EMPField.h"
#include "IonCannon.h"
#include "ChemLauncher.h"
#include "MultiLauncher.h"

//std::array<const char* const, size_t(AresNewSuperType::count)> NewSWType::AresNewSuperType_ToStrings
//{{
//	{ "SonarPulse" },
//	{ "UnitDelivery" },
//	{ "GenericWarhead" },
//	{ "Firestorm" },
//	{ "Protect" },
//	{ "Reveal" },
//	{ "ParaDrop" },
//	{ "SpyPlane" },
//	{ "ChronoSphere" },
//	{ "ChronoWarp" },
//	{ "GeneticMutator" },
//	{ "PsychicDominator" },
//	{ "LightningStorm" },
//	{ "NuclearMissile" },
//	{ "HunterSeeker" },
//	{ "DropPod" },
//	{ "EMPulse" },
//	{ "Battery" }
//	}
//};

std::vector<std::unique_ptr<NewSWType>> NewSWType::Array;

void NewSWType::Register(std::unique_ptr<NewSWType> pType)
{
	pType->TypeIndex = (AresNewSuperType)Array.size();
	Array.emplace_back(std::move(pType));
}

bool NewSWType::CanFireAt(TargetingData const& data, CellStruct const& cell, bool manual) const
{
	//
	return true;
}

SWRange NewSWType::GetRange(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Range;
}

bool NewSWType::IsLaunchSite(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	if (pBuilding->IsAlive && pBuilding->Health && !pBuilding->InLimbo && pBuilding->IsPowerOnline()) {
		return BuildingExt::ExtMap.Find(pBuilding)->HasSuperWeapon(pSWType->OwnerObject()->ArrayIndex, true);
	}

	return false;
}

std::pair<double, double> NewSWType::GetLaunchSiteRange(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	return { pSWType->SW_RangeMinimum.Get(), pSWType->SW_RangeMaximum.Get() };
}

void NewSWType::Init()
{
	if (!Array.empty())
		return;

#define RegSW(name) Register(std::make_unique<name>());

	RegSW(SW_SonarPulse)
	RegSW(SW_UnitDelivery)
	RegSW(SW_GenericWarhead)
	RegSW(SW_Firewall)
	RegSW(SW_Protect)
	RegSW(SW_Reveal)
	RegSW(SW_ParaDrop)
	RegSW(SW_SpyPlane)
	RegSW(SW_ChronoSphere)
	RegSW(SW_ChronoWarp)
	RegSW(SW_GeneticMutator)
	RegSW(SW_PsychicDominator)
	RegSW(SW_LightningStorm)
	RegSW(SW_NuclearMissile)
	RegSW(SW_HunterSeeker)
	RegSW(SW_DropPod)
	RegSW(SW_EMPulse)
	RegSW(SW_Battery)
	RegSW(SW_EMPField)
	RegSW(SW_IonCannon)
	RegSW(SW_ChemLauncher)
	RegSW(SW_MultiLauncher)
#undef RegSW
}

bool NewSWType::IsOriginalType(SuperWeaponType nType)
{
	return nType < SuperWeaponType::count;
}

bool NewSWType::LoadGlobals(PhobosStreamReader& Stm)
{
	for (const auto& ptr : Array) {
		Stm.RegisterChange(ptr.get());
	}

	return Stm.Success();
}

bool NewSWType::SaveGlobals(PhobosStreamWriter& Stm)
{
	for (const auto& ptr : Array) {
		Stm.Save(ptr.get());
	}

	return Stm.Success();
}

NewSWType* NewSWType::GetNthItem(SuperWeaponType i)
{
	return Array[static_cast<size_t>(i) - (size_t)SuperWeaponType::count].get();
}

SuperWeaponType NewSWType::GetHandledType(SuperWeaponType nType)
{
	auto It = std::find_if(Array.begin(), Array.end(), [&](const auto& Item) {
		return Item->HandleThisType(nType);
	});

	if (It != Array.end())
		return SuperWeaponType((int)SuperWeaponType::count + (int)std::distance(Array.begin(), It));

	return SuperWeaponType::Invalid;
}

NewSWType* NewSWType::GetNewSWType(const SWTypeExt::ExtData* pData)
{
	SuperWeaponType iDx = pData->HandledType != SuperWeaponType::Invalid ? pData->HandledType : pData->OwnerObject()->Type;
	return iDx >= SuperWeaponType::count ? NewSWType::GetNthItem(iDx) : nullptr;
}

SuperWeaponType NewSWType::FindFromTypeID(const char* pType)
{
	auto it = std::find_if(Array.begin(), Array.end(), [pType](const std::unique_ptr<NewSWType>& item)
	{
		const char* pID = item->GetTypeString();
		return pID && !strcmp(pID, pType);
	});

	if (it != Array.end())
	{
		return static_cast<SuperWeaponType>(
			(size_t)SuperWeaponType::count + std::distance(Array.begin(), it));
	}

	return SuperWeaponType::Invalid;
}