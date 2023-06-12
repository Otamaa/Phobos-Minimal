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
//#include "ChemLauncher.h"
//#include "MultiLauncher.h"

std::array<std::unique_ptr<NewSWType>, (size_t)AresNewSuperType::count> NewSWType::Array;

void NewSWType::Register(std::unique_ptr<NewSWType> pType , AresNewSuperType nType)
{
	pType->TypeIndex = nType;
	Array[size_t(nType)] = (std::move(pType));
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
static bool NewSWTypeInited = false;

void NewSWType::Init()
{
	if (NewSWTypeInited)
		return;

	NewSWTypeInited = true;
#define RegSW(name ,type) Register(std::make_unique<name>(), type);

	RegSW(SW_SonarPulse , AresNewSuperType::SonarPulse)
	RegSW(SW_UnitDelivery, AresNewSuperType::UnitDelivery)
	RegSW(SW_GenericWarhead, AresNewSuperType::GenericWarhead)
	RegSW(SW_Firewall, AresNewSuperType::Firestorm)
	RegSW(SW_Protect, AresNewSuperType::Protect)
	RegSW(SW_Reveal, AresNewSuperType::Reveal)
	RegSW(SW_ParaDrop, AresNewSuperType::ParaDrop)
	RegSW(SW_SpyPlane, AresNewSuperType::SpyPlane)
	RegSW(SW_ChronoSphere, AresNewSuperType::ChronoSphere)
	RegSW(SW_ChronoWarp, AresNewSuperType::ChronoWarp)
	RegSW(SW_GeneticMutator, AresNewSuperType::GeneticMutator)
	RegSW(SW_PsychicDominator, AresNewSuperType::PsychicDominator)
	RegSW(SW_LightningStorm, AresNewSuperType::LightningStorm)
	RegSW(SW_NuclearMissile, AresNewSuperType::NuclearMissile)
	RegSW(SW_HunterSeeker, AresNewSuperType::HunterSeeker)
	RegSW(SW_DropPod, AresNewSuperType::DropPod)
	RegSW(SW_EMPulse, AresNewSuperType::EMPulse)
	RegSW(SW_Battery, AresNewSuperType::Battery)
	//RegSW(SW_EMPField, AresNewSuperType::EMPField)
	//RegSW(SW_IonCannon, AresNewSuperType::IonCannon)
	//RegSW(SW_ChemLauncher, AresNewSuperType::ChemLauncher)
	//RegSW(SW_MultiLauncher, AresNewSuperType::MultiLauncher)
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
		return SuperWeaponType((int)SuperWeaponType::count + (int)(*It)->TypeIndex);

	return SuperWeaponType::Invalid;
}

NewSWType* NewSWType::GetNewSWType(const SWTypeExt::ExtData* pData)
{
	SuperWeaponType iDx = pData->HandledType != SuperWeaponType::Invalid ? pData->HandledType : pData->OwnerObject()->Type;
	return iDx >= SuperWeaponType::count ? NewSWType::GetNthItem(iDx) : nullptr;
}

SuperWeaponType NewSWType::FindFromTypeID(const char* pType)
{
	auto It = std::find_if(Array.begin(), Array.end(), [pType](const std::unique_ptr<NewSWType>& item) {
		const auto& pIDs = item->GetTypeString();

		if (!pIDs.empty()) {
			for (auto& Id : pIDs) {
				if (!strcmp(Id, pType)) {
					return true;
				}
			}
		}

		return false;
	});

	if (It != Array.end()) {
		return static_cast<SuperWeaponType>(
			(size_t)SuperWeaponType::count + (size_t)(*It)->TypeIndex);
	}

	return SuperWeaponType::Invalid;
}