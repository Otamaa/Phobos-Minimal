#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

#include <ScenarioClass.h>

//Static init
HouseExt::ExtContainer HouseExt::ExtMap;

void HouseExt::ExtData::InitializeConstants() {

}

void HouseExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(HouseAirFactory,reinterpret_cast<BuildingClass*>(ptr));
	AnnounceInvalidPointer(Factory_BuildingType,ptr);
	AnnounceInvalidPointer(Factory_InfantryType, ptr);
	AnnounceInvalidPointer(Factory_VehicleType, ptr);
	AnnounceInvalidPointer(Factory_NavyType, ptr);
	AnnounceInvalidPointer(Factory_AircraftType, ptr);
}

int HouseExt::ActiveHarvesterCount(HouseClass* pThis)
{
	if (!pThis || !pThis->IsPlayer()) return 0;

	int result =
	std::count_if(TechnoClass::Array->begin(), TechnoClass::Array->end(), [pThis](TechnoClass* techno)
	{
		if (const auto pTechnoExt = TechnoTypeExt::ExtMap.Find(techno->GetTechnoType()))
			if (pTechnoExt->IsCountedAsHarvester() && techno->Owner == pThis)
					return TechnoExt::IsHarvesting(techno);

		return false;
	});

	return result;
}

int HouseExt::TotalHarvesterCount(HouseClass* pThis)
{
	if (!pThis || !pThis->IsPlayer() || pThis->Defeated) return 0;

	int result = 0;

	std::for_each(TechnoTypeClass::Array->begin(), TechnoTypeClass::Array->end(), [&result,pThis](TechnoTypeClass* techno)
	{
		if (const auto pTechnoExt = TechnoTypeExt::ExtMap.Find(techno))	{
			if (pTechnoExt->IsCountedAsHarvester())	{
				result += pThis->CountOwnedAndPresent(techno);
			}
		}
	});

	return result;
}

int HouseExt::CountOwnedLimbo(HouseClass* pThis, BuildingTypeClass const* const pItem)
{
	const auto pHouseExt = HouseExt::ExtMap.Find(pThis);
	return pHouseExt->OwnedLimboBuildingTypes.GetItemCount(pItem->ArrayIndex);
}

HouseClass* HouseExt::FindCivilianSide() {
	return HouseClass::FindBySideIndex(RulesExt::Global()->CivilianSideIndex);
}

HouseClass* HouseExt::FindSpecial() {
	return HouseClass::FindByCountryIndex(RulesExt::Global()->SpecialCountryIndex);
}

HouseClass* HouseExt::FindNeutral(){
	return  HouseClass::FindByCountryIndex(RulesExt::Global()->NeutralCountryIndex);
}

void HouseExt::ForceOnlyTargetHouseEnemy(HouseClass* pThis, int mode = -1)
{
	const auto pHouseExt = HouseExt::ExtMap.Find(pThis);

	if (!pHouseExt)
		return;

	if (mode < 0 || mode > 2)
		mode = -1;

	enum { ForceFalse = 0, ForceTrue = 1, ForceRandom = 2, UseDefault = -1 };

	pHouseExt->ForceOnlyTargetHouseEnemyMode = mode;

	switch (mode)
	{
	case ForceFalse:
		pHouseExt->ForceOnlyTargetHouseEnemy = false;
		break;

	case ForceTrue:
		pHouseExt->ForceOnlyTargetHouseEnemy = true;
		break;

	case ForceRandom:
		pHouseExt->ForceOnlyTargetHouseEnemy = ScenarioClass::Instance->Random.RandomBool();
		break;

	default:
		pHouseExt->ForceOnlyTargetHouseEnemy = false;
		break;
	}
}

// Ares
HouseClass* HouseExt::GetHouseKind(OwnerHouseKind const& kind, bool const allowRandom, HouseClass* const pDefault, HouseClass* const pInvoker, HouseClass* const pVictim)
{
	switch (kind) {
	case OwnerHouseKind::Invoker:
	case OwnerHouseKind::Killer:
		return pInvoker ? pInvoker : pDefault;
	case OwnerHouseKind::Victim:
		return pVictim ? pVictim : pDefault;
	case OwnerHouseKind::Civilian:
		return HouseExt::FindCivilianSide();// HouseClass::FindCivilianSide();
	case OwnerHouseKind::Special:
		return HouseExt::FindSpecial();//  HouseClass::FindSpecial();
	case OwnerHouseKind::Neutral:
		return HouseExt::FindNeutral();//  HouseClass::FindNeutral();
	case OwnerHouseKind::Random:
		if (allowRandom) {
			auto& Random = ScenarioClass::Instance->Random;
			return HouseClass::Array->GetItem(
				Random.RandomFromMax(HouseClass::Array->Count - 1));
		}
		else {
			return pDefault;
		}
	case OwnerHouseKind::Default:
	default:
		return pDefault;
	}
}

HouseClass* HouseExt::GetSlaveHouse(SlaveReturnTo const& kind, HouseClass* const pKiller, HouseClass* const pVictim)
{
	switch (kind)
	{
	case SlaveReturnTo::Killer:
		return pKiller;
	case SlaveReturnTo::Master:
		return pVictim;
	case SlaveReturnTo::Civilian:
		return HouseExt::FindCivilianSide();
	case SlaveReturnTo::Special:
		return HouseExt::FindSpecial();
	case SlaveReturnTo::Neutral:
		return HouseExt::FindNeutral();
	case SlaveReturnTo::Random:
			auto& Random = ScenarioClass::Instance->Random;
			return HouseClass::Array->GetItem(
				Random.RandomFromMax(HouseClass::Array->Count - 1));
	}

	return pKiller;
}
// =============================
// load / save

template <typename T>
void HouseExt::ExtData::Serialize(T& Stm)
{
	Stm
	    .Process(this->BuildingCounter)
		.Process(this->OwnedLimboBuildingTypes)
		.Process(this->Building_BuildSpeedBonusCounter)
		.Process(this->HouseAirFactory)
		.Process(this->ForceOnlyTargetHouseEnemy)
		.Process(this->ForceOnlyTargetHouseEnemyMode)
		//.Process(this->RandomNumber)
		.Process(this->Factory_BuildingType)
		.Process(this->Factory_InfantryType)
		.Process(this->Factory_VehicleType)
		.Process(this->Factory_NavyType)
		.Process(this->Factory_AircraftType)
		;
}

void HouseExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<HouseClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void HouseExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<HouseClass>::Serialize(Stm);
	this->Serialize(Stm);
}

bool HouseExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool HouseExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

HouseExt::ExtContainer::ExtContainer() : Container("HouseClass") {}
HouseExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4F6532, HouseClass_CTOR, 0x5)
{
	GET(HouseClass*, pItem, EAX);
#ifdef ENABLE_NEWHOOKS
	HouseExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	HouseExt::ExtMap.FindOrAllocate(pItem);
#endif
	return 0;
}

DEFINE_HOOK(0x4F7371, HouseClass_DTOR, 0x6)
{
	GET(HouseClass*, pItem, ESI);
	HouseExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x504080, HouseClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x503040, HouseClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(HouseClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	HouseExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x504069, HouseClass_Load_Suffix, 0x7)
{
	HouseExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x5046DE, HouseClass_Save_Suffix, 0x7)
{
	HouseExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x50114D, HouseClass_InitFromINI, 0x5)
{
	GET(HouseClass* const, pThis, EBX);
	GET(CCINIClass* const, pINI, ESI);

	HouseExt::ExtMap.LoadFromINI(pThis, pINI);

	return 0;
}