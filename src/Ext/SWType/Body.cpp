#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/Macro.h>
#include <SuperWeaponTypeClass.h>
#include <StringTable.h>

bool SWTypeExt::Handled = false;
SuperClass* SWTypeExt::TempSuper = nullptr;
SuperClass* SWTypeExt::LauchData = nullptr;

void SWTypeExt::ExtData::Initialize()
{
	LimboDelivery_Types.reserve(5);
	LimboDelivery_IDs.reserve(5);
	LimboDelivery_RollChances.reserve(5);
	LimboKill_IDs.reserve(5);
	SW_Next.reserve(5);
	LimboDelivery_RandomWeightsData.reserve(100);
	SW_Next_RandomWeightsData.reserve(100);
	SW_Inhibitors.reserve(10);
	SW_Designators.reserve(10);
	SW_AuxBuildings.reserve(10);
	SW_NegBuildings.reserve(10);
}

void SWTypeExt::ExtData::LoadFromRulesFile(CCINIClass* pINI)
{ 
	INI_EX exINI(pINI);
}

void SWTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->Get();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	if (pINI == CCINIClass::INI_Rules)
		this->LoadFromRulesFile(pINI);

	INI_EX exINI(pINI);
	this->Money_Amount.Read(exINI, pSection, "Money.Amount");
	this->UIDescription.Read(exINI, pSection, "UIDescription");
	this->CameoPriority.Read(exINI, pSection, "CameoPriority");
	this->LimboDelivery_Types.Read(exINI, pSection, "LimboDelivery.Types");
	this->LimboDelivery_IDs.Read(exINI, pSection, "LimboDelivery.IDs");
	this->LimboDelivery_RollChances.Read(exINI, pSection, "LimboDelivery.RollChances");

	char tempBuffer[0x30];
	for (size_t i = 0; ; ++i)
	{
		ValueableVector<int> weights;
		IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "LimboDelivery.RandomWeights%d", i);
		weights.Read(exINI, pSection, tempBuffer);

		if (!weights.size())
			break;

		this->LimboDelivery_RandomWeightsData.push_back(weights);
	}

	ValueableVector<int> weights;
	weights.Read(exINI, pSection, "LimboDelivery.RandomWeights");
	if (weights.size())
		this->LimboDelivery_RandomWeightsData[0] = weights;

	this->LimboKill_Affected.Read(exINI, pSection, "LimboKill.Affected");
	this->LimboKill_IDs.Read(exINI, pSection, "LimboKill.IDs");
	// inhibitor related
	this->SW_Inhibitors.Read(exINI, pSection, "SW.Inhibitors");
	this->SW_AnyInhibitor.Read(exINI, pSection, "SW.AnyInhibitor");
	this->SW_Designators.Read(exINI, pSection, "SW.Designators");
	this->SW_AnyDesignator.Read(exINI, pSection, "SW.AnyDesignator");

	this->SW_RangeMinimum.Read(exINI, pSection, "SW.RangeMinimum");
	this->SW_RangeMaximum.Read(exINI, pSection, "SW.RangeMaximum");
	this->SW_RequiredHouses = pINI->ReadHouseTypesList(pSection, "SW.RequiredHouses", this->SW_RequiredHouses);
	this->SW_ForbiddenHouses = pINI->ReadHouseTypesList(pSection, "SW.ForbiddenHouses", this->SW_ForbiddenHouses);
	this->SW_AuxBuildings.Read(exINI, pSection, "SW.AuxBuildings");
	this->SW_NegBuildings.Read(exINI, pSection, "SW.NegBuildings");
	this->SW_InitialReady.Read(exINI, pSection, "SW.InitialReady");
	this->Detonate_Warhead.Read(exINI, pSection, "Detonate.Warhead" , true);
	this->Detonate_Weapon.Read(exINI, pSection, "Detonate.Weapon", true);
	this->Detonate_Damage.Read(exINI, pSection, "Detonate.Damage");
	this->Detonate_AtFirer.Read(exINI, pSection, "Detonate.AtFirer");

	#pragma region Otamaa
	this->GClock_Shape.Read(exINI, pSection, "GClock.Shape");
	this->GClock_Transculency.Read(exINI, pSection, "GClock.Transculency");
	this->GClock_Palette.Read(exINI, pSection, "GClock.Palette");

	// code disabled , unfinished
	this->ChargeTimer.Read(exINI, pSection, "Timer.ChargeMode");
	this->ChargeTimer_Backwards.Read(exINI, pSection, "Timer.ChargeModeBackwards");
	//
	#pragma endregion

	this->SW_Priority.Read(exINI, pSection, "SW.Priority");
	this->SW_Damage.Read(exINI, pSection, "SW.Damage");

	this->CursorType.Read(exINI, pSection, "Cursor");
	this->NoCursorType.Read(exINI, pSection, "NoCursor");
	this->SW_Range.Read(exINI, pSection, "SW.Range");
	this->Message_ColorScheme.Read(exINI, pSection, "Message.ColorScheme");

}
// =============================
// load / save

void SWTypeExt::LimboDeliver(BuildingTypeClass* pType, HouseClass* pOwner, int ID)
{
	BuildingExt::LimboDeliver(pType, pOwner, ID);
}

// Universal handler of the rolls-weights system
void SWTypeExt::WeightedRollsHandler(std::vector<int>& nResult , Valueable<double>& RandomBuffer, const ValueableVector<float>& rolls, const ValueableVector<ValueableVector<int>>& weights, size_t size)
{
	bool rollOnce = false;
	size_t rollsSize = rolls.size();
	size_t weightsSize = weights.size();
	int index = 0;
	std::vector<int> indices;

	// if no RollChances are supplied, do only one roll
	if (rollsSize == 0)
	{
		rollsSize = 1;
		rollOnce = true;
	}

	for (size_t i = 0; i < rollsSize; i++)
	{
		RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();
		if (!rollOnce && RandomBuffer > abs(rolls[i]))
			continue;

		// If there are more rolls than weight lists, use the last weight list
		size_t j = MinImpl(i, weightsSize - 1);
		index = GeneralUtils::ChooseOneWeighted(RandomBuffer, weights[j]);

		// If modder provides more weights than there are objects and we hit one of these, ignore it
		// otherwise add
		if (size_t(index) < size)
			nResult.push_back(index);
	}
}

void SWTypeExt::ExtData::WeightedRollsHandler(std::vector<int>& nResult, std::vector<float>* rolls, std::vector<std::vector<int>>* weights, size_t size)
{
	bool rollOnce = false;
	size_t rollsSize = rolls->size();
	size_t weightsSize = weights->size();
	int index = 0 ;

	// if no RollChances are supplied, do only one roll
	if (rollsSize == 0)
	{
		rollsSize = 1;
		rollOnce = true;
	}

	for (size_t i = 0; i < rollsSize; i++)
	{
		this->RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();
		if (!rollOnce && this->RandomBuffer > abs((*rolls)[i]))
			continue;

		// If there are more rolls than weight lists, use the last weight list
		size_t j = MinImpl(i , weightsSize - 1);
		index = GeneralUtils::ChooseOneWeighted(this->RandomBuffer, (*weights)[j]);

		// If modder provides more weights than there are objects and we hit one of these, ignore it
		// otherwise add
		if (size_t(index) < size)
			nResult.push_back(index);
	}
}

void SWTypeExt::ExtData::ApplyLimboDelivery(HouseClass* pHouse)
{
	// random mode
	if (this->LimboDelivery_RandomWeightsData.size())
	{
		int id = -1;
		size_t idsSize = this->LimboDelivery_IDs.size();
		std::vector<int> results;
		this->WeightedRollsHandler(results ,&this->LimboDelivery_RollChances, &this->LimboDelivery_RandomWeightsData, this->LimboDelivery_Types.size());

		for (const size_t& result : results)
		{
			if (result < idsSize)
				id = this->LimboDelivery_IDs[result];

			LimboDeliver(this->LimboDelivery_Types[result], pHouse, id);
		}
	}
	// no randomness mode
	else
	{
		int id = -1;
		size_t ids = this->LimboDelivery_IDs.size();

		for (size_t i = 0; i < this->LimboDelivery_Types.size(); i++)
		{
			if (i < ids)
				id = this->LimboDelivery_IDs[i];

			LimboDeliver(this->LimboDelivery_Types[i], pHouse, id);
		}
	}
}

void SWTypeExt::ExtData::ApplyLimboKill(HouseClass* pHouse)
{
	if (this->LimboKill_IDs.empty())
		return;

	for (HouseClass* pTargetHouse : *HouseClass::Array())
	{
		if (pTargetHouse->Type->MultiplayPassive)
			continue;

		BuildingExt::ApplyLimboKill(this->LimboKill_IDs, this->LimboKill_Affected, pTargetHouse, pHouse);
	}
}

void SWTypeExt::ExtData::ApplyDetonation(HouseClass* pHouse, const CellStruct& cell)
{
	if (!this->Detonate_Weapon.isset() && !this->Detonate_Warhead.isset())
		return;

	const auto pCell = MapClass::Instance->GetCellAt(cell);
	BuildingClass* const pFirer = *(std::find_if(pHouse->Buildings.begin(), pHouse->Buildings.end(),
		[&](BuildingClass* const pBld) { return this->IsLaunchSiteEligible(cell, pBld, false); }));

	CoordStruct nDest = CoordStruct::Empty;
	AbstractClass* pTarget = nullptr;

	if (this->Detonate_AtFirer)
	{
		if (!pFirer)
			return;

		pTarget = pFirer;
		nDest = pFirer->GetCenterCoords();
	}
	else
	{
		pTarget = pCell;
		nDest = pCell->GetCoords();
	}

	if (!MapClass::Instance->IsWithinUsableArea(nDest))
		Debug::Log("SW [%s] Lauch Outside Usable Map Area ! \n", this->Get()->ID);

	if (const auto pWeapon = this->Detonate_Weapon.Get())
		WeaponTypeExt::DetonateAt(pWeapon, nDest, pFirer, this->Detonate_Damage.Get(pWeapon->Damage));
	else
		WarheadTypeExt::DetonateAt(this->Detonate_Warhead.Get(), pTarget, nDest, pFirer, this->Detonate_Damage.Get(this->SW_Damage.Get(0)));
}

// SW.Next proper launching mechanic
void SWTypeExt::Launch(HouseClass* pHouse, SWTypeExt::ExtData* pLauncherTypeExt, int pLaunchedType, const CellStruct& cell)
{
	const auto pSuper = pHouse->Supers.GetItemOrDefault(pLaunchedType);

	if (!pSuper)
		return;

	const auto pSuperTypeExt = SWTypeExt::ExtMap.Find(pSuper->Type);
	if (!pLauncherTypeExt->SW_Next_RealLaunch ||
		(pSuperTypeExt && pSuper->IsCharged && pHouse->CanTransactMoney(pSuperTypeExt->Money_Amount)))
	{

		if (pLauncherTypeExt->SW_Next_IgnoreInhibitors || !pSuperTypeExt->HasInhibitor(pHouse, cell)
			&& (pLauncherTypeExt->SW_Next_IgnoreDesignators || pSuperTypeExt->HasDesignator(pHouse, cell)))
		{
			// Forcibly fire
			pSuper->Launch(cell, true);
			if (pLauncherTypeExt->SW_Next_RealLaunch)
				pSuper->Reset();
		}

	}
}

void SWTypeExt::ExtData::ApplySWNext(SuperClass* pSW, const CellStruct& cell)
{
	// random mode
	if (this->SW_Next_RandomWeightsData.size())
	{
		std::vector<int> results;
		this->WeightedRollsHandler(results, &this->SW_Next_RollChances, &this->SW_Next_RandomWeightsData, this->SW_Next.size());
		for (const int& result : results)
		{
			SWTypeExt::Launch(pSW->Owner, this, this->SW_Next[result], cell);
		}
	}
	// no randomness mode
	else
	{
		for (const auto& pSWType : this->SW_Next)
			SWTypeExt::Launch(pSW->Owner, this, pSWType, cell);
	}
}

void SWTypeExt::ExtData::FireSuperWeapon(SuperClass* pSW, HouseClass* pHouse, const CellStruct* const pCell, bool IsCurrentPlayer)
{
	if (!pHouse)
	{
		Debug::Log("SW[%x] Trying To execute %s with nullptr HouseOwner ! \n", pSW, "FireSuperWeapon");
		return;
	}

	if (!this->LimboDelivery_Types.empty())
		ApplyLimboDelivery(pHouse);

	if (!this->LimboKill_IDs.empty())
		ApplyLimboKill(pHouse);

	if (this->Detonate_Warhead.isset() || this->Detonate_Weapon.isset())
		this->ApplyDetonation(pSW->Owner, *pCell);

	if (!this->SW_Next.empty())
		this->ApplySWNext(pSW, *pCell);
}

//Ares 0.A helpers
bool SWTypeExt::ExtData::IsInhibitor(HouseClass* pOwner, TechnoClass* pTechno)
{
	if (pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated)
	{
		if (!pOwner->IsAlliedWith(pTechno))
		{
			if (auto pBld = abstract_cast<BuildingClass*>(pTechno))
			{
				if (!pBld->IsPowerOnline())
					return false;
			}

			return SW_AnyInhibitor
				|| SW_Inhibitors.Contains(pTechno->GetTechnoType());
		}
	}
	return false;
}

bool SWTypeExt::ExtData::IsInhibitorEligible(HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno)
{
	if (IsInhibitor(pOwner, pTechno))
	{
		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

		// get the inhibitor's center
		auto center = pTechno->GetCenterCoords();

		// has to be closer than the inhibitor range (which defaults to Sight)
		return Coords.DistanceFrom(CellClass::Coord2Cell(center)) <= pExt->InhibitorRange.Get(pType->Sight);
	}

	return false;
}

bool SWTypeExt::ExtData::HasInhibitor(HouseClass* pOwner, const CellStruct& Coords)
{
	// does not allow inhibitors
	if (SW_Inhibitors.empty() && !SW_AnyInhibitor)
		return false;

	// a single inhibitor in range suffices
	return std::any_of(TechnoClass::Array->begin(), TechnoClass::Array->end(), [=, &Coords](TechnoClass* pTechno)
						{ return IsInhibitorEligible(pOwner, Coords, pTechno); }
	);
}

// Designators check
bool SWTypeExt::ExtData::IsDesignator(HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->Owner == pOwner && pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated)
		return this->SW_AnyDesignator || this->SW_Designators.Contains(pTechno->GetTechnoType());

	return false;
}

bool SWTypeExt::ExtData::IsDesignatorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const
{
	if (this->IsDesignator(pOwner, pTechno))
	{
		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

		// get the designator's center
		auto center = pTechno->GetCenterCoords();

		// has to be closer than the designator range (which defaults to Sight)
		return coords.DistanceFrom(CellClass::Coord2Cell(center)) <= pExt->DesignatorRange.Get(pType->Sight);
	}

	return false;
}

bool SWTypeExt::ExtData::HasDesignator(HouseClass* pOwner, const CellStruct& coords) const
{
	// does not require designators
	if (this->SW_Designators.empty() && !this->SW_AnyDesignator)
		return true;

	// a single designator in range suffices
	return std::any_of(TechnoClass::Array->begin(), TechnoClass::Array->end(), [=, &coords](TechnoClass* pTechno)
		{ return this->IsDesignatorEligible(pOwner, coords, pTechno); });
}

bool SWTypeExt::ExtData::IsLaunchSite(BuildingClass* pBuilding) const
{
	if (pBuilding->IsAlive && pBuilding->Health && !pBuilding->InLimbo && pBuilding->IsPowerOnline())
	{
		if (pBuilding->TemporalTargetingMe || pBuilding->IsBeingWarpedOut())
			return false;

		return BuildingExt::ExtMap.Find(pBuilding)->HasSuperWeapon(this->Get()->ArrayIndex, true);
	}

	return false;
}

std::pair<double, double> SWTypeExt::ExtData::GetLaunchSiteRange(BuildingClass* pBuilding) const
{
	return { this->SW_RangeMinimum.Get(), this->SW_RangeMaximum.Get() };
}

bool SWTypeExt::ExtData::IsAvailable(HouseClass* pHouse) const
{
	const auto pThis = this->Get();

	// check whether the optional aux building exists
	if (pThis->AuxBuilding && pHouse->CountOwnedAndPresent(pThis->AuxBuilding) <= 0)
	{
		return false;
	}

	// allow only certain houses, disallow forbidden houses
	const auto OwnerBits = 1u << pHouse->Type->ArrayIndex;
	if (!(this->SW_RequiredHouses & OwnerBits) || (this->SW_ForbiddenHouses & OwnerBits))
	{
		return false;
	}

	// check that any aux building exist and no neg building
	auto IsBuildingPresent = [pHouse](BuildingTypeClass* pType)
	{
		return pType && pHouse->CountOwnedAndPresent(pType) > 0;
	};

	const auto& Aux = this->SW_AuxBuildings;
	// If building Not Exist
	if (!Aux.empty() && std::none_of(Aux.begin(), Aux.end(), IsBuildingPresent))
	{
		return false;
	}

	const auto& Neg = this->SW_NegBuildings;
	// If building Exist
	if (std::any_of(Neg.begin(), Neg.end(), IsBuildingPresent))
	{
		return false;
	}

	return true;
}

bool SWTypeExt::ExtData::IsLaunchSiteEligible(const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const
{
	if (!this->IsLaunchSite(pBuilding))
		return false;

	if (ignoreRange)
		return true;

	// get the range for this building
	const auto& [minRange, maxRange] = this->GetLaunchSiteRange(pBuilding);
	const auto center = CellClass::Coord2Cell(BuildingExt::GetCenterCoords(pBuilding));
	const auto distance = Coords.DistanceFrom(center);

	// negative range values just pass the test
	return (minRange < 0.0 || distance >= minRange)
		&& (maxRange < 0.0 || distance <= maxRange);
}

template <typename T>
void SWTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Money_Amount)
		.Process(this->UIDescription)
		.Process(this->CameoPriority)
		.Process(this->LimboDelivery_Types)
		.Process(this->LimboDelivery_IDs)
		.Process(this->LimboDelivery_RandomWeightsData)
		.Process(this->LimboDelivery_RollChances)
		.Process(this->LimboKill_Affected)
		.Process(this->LimboKill_IDs)
		.Process(this->RandomBuffer)
		.Process(this->SW_Inhibitors)
		.Process(this->SW_AnyInhibitor)
		.Process(this->SW_Designators)
		.Process(this->SW_AnyDesignator)
		.Process(this->SW_RangeMinimum)
		.Process(this->SW_RangeMaximum)
		.Process(this->SW_RequiredHouses)
		.Process(this->SW_ForbiddenHouses)
		.Process(this->SW_AuxBuildings)
		.Process(this->SW_NegBuildings)
		.Process(this->SW_InitialReady)
		.Process(this->GClock_Shape)
		.Process(this->GClock_Transculency)
		.Process(this->GClock_Palette)
		.Process(this->Detonate_Warhead)
		.Process(this->Detonate_Weapon)
		.Process(this->Detonate_Damage)
		.Process(this->Detonate_AtFirer)
		.Process(this->ChargeTimer)
		.Process(this->ChargeTimer_Backwards)
		.Process(this->SW_Priority)
		.Process(this->SW_Damage)
		.Process(this->CursorType)
		.Process(this->NoCursorType)
		.Process(this->SW_Range)
		.Process(this->Message_ColorScheme)
		;

}

// =============================
// container
SWTypeExt::ExtContainer SWTypeExt::ExtMap;

SWTypeExt::ExtContainer::ExtContainer() : Container("SuperWeaponTypeClass") {}
SWTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x6CE6F6, SuperWeaponTypeClass_CTOR, 0x5)
{
	GET(SuperWeaponTypeClass*, pItem, EAX);
	SWTypeExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x6CEFE0, SuperWeaponTypeClass_SDDTOR, 0x8)
{
	GET(SuperWeaponTypeClass*, pItem, ECX);
	SWTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6CE8D0, SuperWeaponTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6CE800, SuperWeaponTypeClass_SaveLoad_Prefix, 0xA)
{
	GET_STACK(SuperWeaponTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SWTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6CE8BE, SuperWeaponTypeClass_Load_Suffix, 0x7)
{
	SWTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6CE8EA, SuperWeaponTypeClass_Save_Suffix, 0x3)
{
	SWTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x6CEE50, SuperWeaponTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x6CEE43, SuperWeaponTypeClass_LoadFromINI, 0xA)
{
	GET(SuperWeaponTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x3FC);

	SWTypeExt::ExtMap.LoadFromINI(pItem, pINI , R->Origin() == 0x6CEE50);
	return 0;
}
