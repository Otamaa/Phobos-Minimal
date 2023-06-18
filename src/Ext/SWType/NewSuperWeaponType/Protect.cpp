#include "Protect.h"

#include <Ext/TechnoType/Body.h>
#include <Utilities/Helpers.h>

std::vector<const char*> SW_Protect::GetTypeString() const
{
	return { "Protect" };
}

bool SW_Protect::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::IronCurtain) || 
			(type == SuperWeaponType::ForceShield);
}

bool SW_Protect::CanFireAt(TargetingData const& data, const CellStruct& cell, bool manual) const
{
	auto ret = NewSWType::CanFireAt(data, cell, manual);

	// if this is a force shield requiring buildings and a building is selected, check the modifier
	if (ret && manual && data.TypeExt->Protect_IsForceShield && data.TypeExt->SW_RequiresTarget & SuperWeaponTarget::Building)
	{
		auto pCell = MapClass::Instance->GetCellAt(cell);
		if (auto pBld = pCell->GetBuilding())
		{
			auto pExt = TechnoTypeExt::ExtMap.Find(pBld->GetTechnoType());
			if (pExt->ForceShield_Modifier <= 0.0)
			{
				ret = false;
			}
		}
	}

	return ret;
}

bool SW_Protect::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	SuperWeaponTypeClass* pSW = pThis->Type;
	SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSW);

	if (pThis->IsCharged)
	{
		CellClass* pTarget = MapClass::Instance->GetCellAt(Coords);
		CoordStruct Crd = pTarget->GetCoords();

		bool isForceShield = pData->Protect_IsForceShield;

		int duration = pData->Protect_Duration.Get(isForceShield
			? RulesClass::Instance->ForceShieldDuration : RulesClass::Instance->IronCurtainDuration);

		// play start sound
		if (pSW->StartSound > -1)
		{
			VocClass::PlayAt(pSW->StartSound, Crd, nullptr);
		}

		// set up the special sound when the effect wears off
		if (pThis->Type->SpecialSound > -1)
		{
			int playFadeSoundTime = pData->Protect_PlayFadeSoundTime.Get(isForceShield
				? RulesClass::Instance->ForceShieldPlayFadeSoundTime : 0);

			pThis->SpecialSoundDuration = duration - playFadeSoundTime;
			pThis->SpecialSoundLocation = Crd;
		}

		// shut down power
		int powerOutageDuration = pData->Protect_PowerOutageDuration.Get(isForceShield
			? RulesClass::Instance->ForceShieldBlackoutDuration : 0);

		if (powerOutageDuration > 0)
		{
			pThis->Owner->CreatePowerOutage(powerOutageDuration);
		}

		bool force = pData->Protect_IsForceShield;
		auto range = GetRange(pData);

		auto IronCurtain = [=](TechnoClass* pTechno) -> bool
		{
			// we shouldn't do anything
			if (pTechno->IsImmobilized || pTechno->IsBeingWarpedOut())
			{
				return true;
			}

			// is this thing affected at all?
			if (!pData->IsHouseAffected(pThis->Owner, pTechno->Owner))
			{
				return true;
			}

			if (!pData->IsTechnoAffected(pTechno))
			{
				return true;
			}

			// protect this techno
			pTechno->IronCurtain(duration, pThis->Owner, force);

			return true;
		};

		// protect everything in range
		Helpers::Alex::DistinctCollector<TechnoClass*> items;
		Helpers::Alex::for_each_in_rect_or_range<TechnoClass>(Coords, range.WidthOrRange, range.Height, items);
		items.apply_function_for_each(IronCurtain);
	}

	return true;
}

void SW_Protect::Initialize(SWTypeExt::ExtData* pData)
{
	auto type = pData->Get()->Type;

	// iron curtain and force shield, as well as protect
	pData->SW_AnimHeight = 5;

	if (type == SuperWeaponType::ForceShield)
	{
		// force shield
		pData->Protect_IsForceShield = true;
		pData->SW_RadarEvent = false;

		pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_ForceShieldReady);

		pData->SW_AITargetingMode = SuperWeaponAITargetingMode::ForceShield;
		pData->SW_AffectsHouse = AffectedHouse::Team;
		pData->SW_AffectsTarget = SuperWeaponTarget::Building;
		pData->SW_RequiresHouse = AffectedHouse::Team;
		pData->SW_RequiresTarget = SuperWeaponTarget::Building;

		pData->CursorType = int(MouseCursorType::ForceShield);
		pData->NoCursorType = int(MouseCursorType::NoForceShield);
	}
	else
	{
		// iron curtain and protect
		pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_IronCurtainReady);
		pData->EVA_Detected = VoxClass::FindIndexById(GameStrings::EVA_IronCurtainDetected);
		pData->EVA_Activated = VoxClass::FindIndexById(GameStrings::EVA_IronCurtainActivated);

		pData->CursorType = int(MouseCursorType::IronCurtain);
	}
}

void SW_Protect::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->get_ID();

	INI_EX exINI(pINI);
	pData->Protect_Duration.Read(exINI, section, "Protect.Duration");
	pData->Protect_PowerOutageDuration.Read(exINI, section, "Protect.PowerOutage");
	pData->Protect_PlayFadeSoundTime.Read(exINI, section, "Protect.PlayFadeSoundTime");
}

AnimTypeClass* SW_Protect::GetAnim(const SWTypeExt::ExtData* pData) const
{
	if (pData->SW_Anim.isset()) {
		return pData->SW_Anim;
	}

	if (pData->OwnerObject()->Type == SuperWeaponType::ForceShield)
	{
		return RulesClass::Instance->ForceShieldInvokeAnim;
	}
	else
	{
		return RulesClass::Instance->IronCurtainInvokeAnim;
	}
}

SWRange  SW_Protect::GetRange(const SWTypeExt::ExtData* pData) const
{
	if (!pData->SW_Range->empty()) {
		return pData->SW_Range;
	}
	else if (pData->OwnerObject()->Type == SuperWeaponType::ForceShield) {
		return { RulesClass::Instance->ForceShieldRadius, -1 };
	}

	return { 3, 3 };
}