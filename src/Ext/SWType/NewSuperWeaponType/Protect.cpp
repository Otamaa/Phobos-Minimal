#include "Protect.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/SWType/Body.h>

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

bool SW_Protect::CanFireAt(const TargetingData* pTargeting, const CellStruct& cell, bool manual) const
{
	auto ret = NewSWType::CanFireAt(pTargeting, cell, manual);

	// if this is a force shield requiring buildings and a building is selected, check the modifier
	if (ret && manual && pTargeting->TypeExt->Protect_IsForceShield && pTargeting->TypeExt->SW_RequiresTarget & SuperWeaponTarget::Building)
	{
		auto pCell = MapClass::Instance->GetCellAt(cell);
		if (auto pBld = pCell->GetBuilding())
		{
			auto pExt = TechnoTypeExtContainer::Instance.Find(pBld->GetTechnoType());
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
	SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pSW);

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

		const auto range = this->GetRange(pData);

		// protect everything in range
		Helpers::Alex::DistinctCollector<TechnoClass*> items;
		Helpers::Alex::for_each_in_rect_or_range<TechnoClass>(Coords, range.WidthOrRange, range.Height, items);
		items.apply_function_for_each([=](TechnoClass* pTechno) {

			// we shouldn't do anything
			if (pTechno->IsImmobilized || pTechno->IsBeingWarpedOut())
				return true;

			// is this thing affected at all?
			if (!pData->IsHouseAffected(pThis->Owner, pTechno->Owner))
				return true;

			if (!pData->IsTechnoAffected(pTechno))
				return true;

			// protect this techno
			pTechno->IronCurtain(duration, pThis->Owner, pData->Protect_IsForceShield);
			return true;
		});
	}

	return true;
}

void SW_Protect::Initialize(SWTypeExtData* pData)
{
	auto type = pData->AttachedToObject->Type;

	// iron curtain and force shield, as well as protect
	pData->SW_AnimHeight = 5;

	if (type == SuperWeaponType::ForceShield)
	{
		pData->AttachedToObject->Action = Action::ForceShield;
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
		pData->AttachedToObject->Action = Action::IronCurtain;
		// iron curtain and protect
		pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_IronCurtainReady);
		pData->EVA_Detected = VoxClass::FindIndexById(GameStrings::EVA_IronCurtainDetected);
		pData->EVA_Activated = VoxClass::FindIndexById(GameStrings::EVA_IronCurtainActivated);
		pData->SW_AITargetingMode = SuperWeaponAITargetingMode::IronCurtain;
		pData->CursorType = int(MouseCursorType::IronCurtain);
	}
}

void SW_Protect::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->get_ID();

	INI_EX exINI(pINI);
	pData->Protect_Duration.Read(exINI, section, "Protect.Duration");
	pData->Protect_PowerOutageDuration.Read(exINI, section, "Protect.PowerOutage");
	pData->Protect_PlayFadeSoundTime.Read(exINI, section, "Protect.PlayFadeSoundTime");
}

bool SW_Protect::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}

AnimTypeClass* SW_Protect::GetAnim(const SWTypeExtData* pData) const
{
	return pData->SW_Anim.Get(pData->AttachedToObject->Type == SuperWeaponType::ForceShield ?
		RulesClass::Instance->ForceShieldInvokeAnim : RulesClass::Instance->IronCurtainInvokeAnim);
}

SWRange SW_Protect::GetRange(const SWTypeExtData* pData) const
{
	if (!pData->SW_Range->empty()) {
		return pData->SW_Range;
	}
	else if (pData->AttachedToObject->Type == SuperWeaponType::ForceShield) {
		return { RulesClass::Instance->ForceShieldRadius };
	}

	return { 3, 3 };
}
