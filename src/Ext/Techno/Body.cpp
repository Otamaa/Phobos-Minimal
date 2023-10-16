#include "Body.h"

#include <BuildingClass.h>
#include <HouseClass.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <ScenarioClass.h>
#include <SpawnManagerClass.h>
#include <SlaveManagerClass.h>
#include <InfantryClass.h>
#include <Unsorted.h>

#include <BitFont.h>

#include <New/Entity/FlyingStrings.h>

#include <Ext/Aircraft/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Script/Body.h>

#include <Locomotor/Cast.h>

#include <Misc/AresData.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Enum.h>

#include <Utilities/Cast.h>
#include <Utilities/Macro.h>
#include <Utilities/LocomotionCast.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>
#include <Misc/Ares/Hooks/Header.h>

#include <memory>
#include <Ares_TechnoExt.h>

Point2D TechnoExtData::GetScreenLocation(TechnoClass* pThis)
{
	CoordStruct absolute = pThis->GetCoords();
	Point2D  position = { 0,0 };
	TacticalClass::Instance->CoordsToScreen(&position, &absolute);
	position -= TacticalClass::Instance->TacticalPos;

	return position;
}

Point2D TechnoExtData::GetFootSelectBracketPosition(TechnoClass* pThis, Anchor anchor)
{
	int length = 17;
	Point2D position = GetScreenLocation(pThis);

	if (pThis->WhatAmI() == AbstractType::Infantry)
		length = 8;

	RectangleStruct bracketRect =
	{
		position.X - length + (length == 8) + 1,
		position.Y - 28 + (length == 8),
		length * 2,
		length * 3
	};

	return anchor.OffsetPosition(bracketRect);
}

Point2D TechnoExtData::GetBuildingSelectBracketPosition(TechnoClass* pThis, BuildingSelectBracketPosition bracketPosition)
{
	const auto pBuildingType = static_cast<BuildingTypeClass*>(pThis->GetTechnoType());
	Point2D position = GetScreenLocation(pThis);
	CoordStruct dim2 = CoordStruct::Empty;
	pBuildingType->Dimension2(&dim2);
	Point2D positionFix = Point2D::Empty;
	dim2 = { -dim2.X / 2, dim2.Y / 2, dim2.Z };
	TacticalClass::Instance->CoordsToScreen(&positionFix, &dim2);

	const int foundationWidth = pBuildingType->GetFoundationWidth();
	const int foundationHeight = pBuildingType->GetFoundationHeight(false);
	const int height = pBuildingType->Height * 12;
	const int lengthW = foundationWidth * 7 + foundationWidth / 2;
	const int lengthH = foundationHeight * 7 + foundationHeight / 2;

	position.X += positionFix.X + 3 + lengthH * 4;
	position.Y += positionFix.Y + 4 - lengthH * 2;

	switch (bracketPosition)
	{
	case BuildingSelectBracketPosition::Top:
		break;
	case BuildingSelectBracketPosition::LeftTop:
		position.X -= lengthH * 4;
		position.Y += lengthH * 2;
		break;
	case BuildingSelectBracketPosition::LeftBottom:
		position.X -= lengthH * 4;
		position.Y += lengthH * 2 + height;
		break;
	case BuildingSelectBracketPosition::Bottom:
		position.Y += lengthW * 2 + lengthH * 2 + height;
		break;
	case BuildingSelectBracketPosition::RightBottom:
		position.X += lengthW * 4;
		position.Y += lengthW * 2 + height;
		break;
	case BuildingSelectBracketPosition::RightTop:
		position.X += lengthW * 4;
		position.Y += lengthW * 2;
		break;
	default:
		break;
	}

		return position;
	}

Iterator<DigitalDisplayTypeClass*> NOINLINE TechnoExtData::GetDisplayType(TechnoClass* pThis, TechnoTypeClass* pType, int& length) {
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->DigitalDisplayTypes.empty())
	{
		switch (pThis->WhatAmI())
		{
		case AbstractType::Building:
		{
			const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
			const int height = pBuildingType->GetFoundationHeight(false);
			length = height * 7 + height / 2;
			return RulesExtData::Instance()->Buildings_DefaultDigitalDisplayTypes;
		}
		case AbstractType::Infantry:
		{
			length = 8;
			return RulesExtData::Instance()->Infantry_DefaultDigitalDisplayTypes;
		}
		case AbstractType::Unit:
		{
			return RulesExtData::Instance()->Vehicles_DefaultDigitalDisplayTypes;
		}
		case AbstractType::Aircraft:
		{
			return RulesExtData::Instance()->Aircraft_DefaultDigitalDisplayTypes;
		}
		default:
		{
			return {};
		}

		}
	}

	return pTypeExt->DigitalDisplayTypes;
}

bool GetDisplayTypeData(std::vector<DigitalDisplayTypeClass*>* ret , TechnoClass* pThis , TechnoTypeClass* pType, int& length)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->DigitalDisplayTypes.empty())
	{
		switch (pThis->WhatAmI())
		{
		case AbstractType::Building:
		{
			const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
			const int height = pBuildingType->GetFoundationHeight(false);
			length = height * 7 + height / 2;
			ret = &RulesExtData::Instance()->Buildings_DefaultDigitalDisplayTypes;
			return true;
		}
		case AbstractType::Infantry:
		{
			length = 8;
			ret = &(RulesExtData::Instance()->Infantry_DefaultDigitalDisplayTypes);
			return true;
		}
		case AbstractType::Unit:
		{
			ret = &(RulesExtData::Instance()->Vehicles_DefaultDigitalDisplayTypes);
			return true;
		}
		case AbstractType::Aircraft:
		{
			ret = &(RulesExtData::Instance()->Aircraft_DefaultDigitalDisplayTypes);
			return true;
		}
		default:
		{
			return false;
		}

		}
	}

	ret = &(pTypeExt->DigitalDisplayTypes);
	return true;
}

void TechnoExtData::ProcessDigitalDisplays(TechnoClass* pThis)
{
	if (!Phobos::Config::DigitalDisplay_Enable)
		return;

	auto pType = pThis->GetTechnoType();

	if (TechnoTypeExtContainer::Instance.Find(pType)->DigitalDisplay_Disable)
		return;

	int length = 17;
	if (const auto DisplayTypes = TechnoExtData::GetDisplayType(pThis, pType, length)) {

		const auto pExt = TechnoExtContainer::Instance.Find(pThis);
		const auto What = pThis->WhatAmI();
		const bool isBuilding = What == AbstractType::Building;
		const bool isInfantry = What == AbstractType::Infantry;
		const bool IsCurPlayerObserver = HouseClass::IsCurrentPlayerObserver();

		for (auto pDisplayType : DisplayTypes) {

			if (IsCurPlayerObserver && !pDisplayType->VisibleToHouses_Observer)
				continue;

			if (!IsCurPlayerObserver && !EnumFunctions::CanTargetHouse(pDisplayType->VisibleToHouses, pThis->Owner, HouseClass::CurrentPlayer))
				continue;

			int value = -1;
			int maxValue = -1;

			TechnoExtData::GetValuesForDisplay(pThis, pDisplayType->InfoType, value, maxValue);

			if (value == -1 || maxValue == -1)
				continue;

			const bool hasShield = pExt->Shield != nullptr && !pExt->Shield->IsBrokenAndNonRespawning();
			Point2D position = isBuilding ? GetBuildingSelectBracketPosition(pThis, pDisplayType->AnchorType_Building)
				: GetFootSelectBracketPosition(pThis, pDisplayType->AnchorType);

			position.Y += pType->PixelSelectionBracketDelta;

			if (pDisplayType->InfoType == DisplayInfoType::Shield)
				position.Y += pExt->CurrentShieldType->BracketDelta;

			pDisplayType->Draw(position, length, value, maxValue, isBuilding, isInfantry, hasShield);
		}
	}
}

void TechnoExtData::GetValuesForDisplay(TechnoClass* pThis, DisplayInfoType infoType, int& value, int& maxValue)
{
	const auto pType = pThis->GetTechnoType();
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	switch (infoType)
	{
	case DisplayInfoType::Health:
	{
		value = pThis->Health;
		maxValue = pType->Strength;
		break;
	}
	case DisplayInfoType::Shield:
	{
		if (pExt->Shield == nullptr || pExt->Shield->IsBrokenAndNonRespawning())
			return;

		value = pExt->Shield->GetHP();
		maxValue = pExt->Shield->GetType()->Strength.Get();
		break;
	}
	case DisplayInfoType::Ammo:
	{
		if (pType->Ammo <= 0)
			return;

		value = pThis->Ammo;
		maxValue = pType->Ammo;
		break;
	}
	case DisplayInfoType::MindControl:
	{
		if (pThis->CaptureManager == nullptr)
			return;

		value = pThis->CaptureManager->ControlNodes.Count;
		maxValue = pThis->CaptureManager->MaxControlNodes;
		break;
	}
	case DisplayInfoType::Spawns:
	{
		if (pThis->SpawnManager == nullptr || pType->Spawns == nullptr || pType->SpawnsNumber <= 0)
			return;

		value = pThis->SpawnManager->CountAliveSpawns();
		maxValue = pType->SpawnsNumber;
		break;
	}
	case DisplayInfoType::Passengers:
	{
		if (pType->Passengers <= 0)
			return;

		value = pThis->Passengers.NumPassengers;
		maxValue = pType->Passengers;
		break;
	}
	case DisplayInfoType::Tiberium:
	{
		if (pType->Storage <= 0)
			return;

		value = static_cast<int>(pThis->Tiberium.GetTotalAmount());
		maxValue = pType->Storage;
		break;
	}
	case DisplayInfoType::Experience:
	{
		value = static_cast<int>(pThis->Veterancy.Veterancy * RulesClass::Instance->VeteranRatio * pType->GetCost());
		maxValue = static_cast<int>(2.0 * RulesClass::Instance->VeteranRatio * pType->GetCost());
		break;
	}
	case DisplayInfoType::Occupants:
	{
		if (pThis->WhatAmI() != AbstractType::Building)
			return;

		const auto pBuildingType = static_cast<BuildingTypeClass*>(pType);
		const auto pBuilding = static_cast<BuildingClass*>(pThis);

		if (!pBuildingType->CanBeOccupied)
			return;

		value = pBuilding->Occupants.Count;
		maxValue = pBuildingType->MaxNumberOccupants;
		break;
	}
	case DisplayInfoType::GattlingStage:
	{
		if (!pType->IsGattling)
			return;

		value = pThis->CurrentGattlingStage;
		maxValue = pType->WeaponStages;
		break;
	}
	case DisplayInfoType::IronCurtain:
	{
		if (pThis->IronCurtainTimer.TimeLeft == 0 && pThis->IronCurtainTimer.StartTime == -1)
			return;

		value = pThis->IronCurtainTimer.GetTimeLeft();
		maxValue = pThis->IronCurtainTimer.TimeLeft;
		break;
	}
	case DisplayInfoType::DisableWeapon:
	{
		auto& nTimer = pExt->DisableWeaponTimer;
		if (nTimer.TimeLeft == 0 && nTimer.StartTime == -1)
			return;

		value = nTimer.GetTimeLeft();
		maxValue = nTimer.TimeLeft;
		break;
	}
	case DisplayInfoType::CloakDisable :
	{
		auto& nTimer = pExt->CloakSkipTimer;
		if (nTimer.TimeLeft == 0 && nTimer.StartTime == -1)
			return;

		value = nTimer.GetTimeLeft();
		maxValue = nTimer.TimeLeft;
		break;
	}
	default:
	{
		value = pThis->Health;
		maxValue = pType->Strength;
		break;
	}
	}
}

void TechnoExtData::RestoreLastTargetAndMissionAfterWebbed(InfantryClass* pThis)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTarget = std::exchange(pExt->WebbyLastTarget, nullptr);

	if (pTarget)
		pThis->Override_Mission(pExt->WebbyLastMission, pTarget, pTarget);
	else
		pThis->QueueMission(pThis->Owner->IsControlledByHuman_() ? Mission::Guard : Mission::Hunt ,true);
}

void TechnoExtData::StoreLastTargetAndMissionAfterWebbed(InfantryClass* pThis)
{
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	pExt->WebbyLastMission = pThis->GetCurrentMission();
	pExt->WebbyLastTarget = pThis->Target;
}

void TechnoExtData::StoreHijackerLastDisguiseData(InfantryClass* pThis, FootClass* pVictim)
{
	auto pExt = TechnoExtContainer::Instance.Find(pVictim);
	pExt->HijackerLastDisguiseType = (InfantryTypeClass*)pThis->GetDisguise(true);
	pExt->HijackerLastDisguiseHouse = pThis->GetDisguiseHouse(true);
}

void TechnoExtData::RestoreStoreHijackerLastDisguiseData(InfantryClass* pThis, FootClass* pVictim)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pVictim);
	pThis->ClearDisguise();
	pThis->Disguise = pExt->HijackerLastDisguiseType;
	pThis->DisguisedAsHouse = pExt->HijackerLastDisguiseHouse;
}

NOINLINE WeaponTypeClass* TechnoExtData::GetCurrentWeapon(TechnoClass* pThis, int& weaponIndex, bool getSecondary)
{
	if (!pThis)
		return nullptr;

	auto const pType = pThis->GetTechnoType();
	weaponIndex = getSecondary ? 1 : 0;

	if (pType->TurretCount > 0 && !pType->IsGattling)
	{
		if (getSecondary)
		{
			weaponIndex = -1;
			return nullptr;
		}

		weaponIndex = pThis->CurrentWeaponNumber >= 0 ? pThis->CurrentWeaponNumber : 0;
	}
	else if (pType->IsGattling)
	{
		weaponIndex = pThis->CurrentGattlingStage * 2 + weaponIndex;
	}

	//Debug::Log("%s Getting WeaponIndex %d for %s\n", __FUNCTION__, weaponIndex, pThis->get_ID());
	if(const auto pWpStr = pThis->GetWeapon(weaponIndex))
		return pWpStr->WeaponType;

	return nullptr;
}

NOINLINE WeaponTypeClass* TechnoExtData::GetCurrentWeapon(TechnoClass* pThis, bool getSecondary)
{
	int weaponIndex = 0;
	return TechnoExtData::GetCurrentWeapon(pThis, weaponIndex, getSecondary);
}

bool TechnoExtData::IsCullingImmune(TechnoClass* pThis)
{
	return HasAbility(pThis, PhobosAbilityType::CullingImmune);
}

bool TechnoExtData::IsEMPImmune(TechnoClass* pThis)
{
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ImmuneToEMP)
		return true;

	return HasAbility(pThis, PhobosAbilityType::EmpImmune);
}

bool TechnoExtData::IsPsionicsImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();

	if (pType->ImmuneToPsionics)
		return true;

	return HasAbility(pThis, PhobosAbilityType::PsionicsImmune);
}

bool TechnoExtData::IsCritImmune(TechnoClass* pThis)
{
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ImmuneToCrit)
		return true;

	return HasAbility(pThis, PhobosAbilityType::CritImmune);
}

bool TechnoExtData::IsChronoDelayDamageImmune(FootClass* pThis)
{
	if (!pThis || !pThis->IsWarpingIn())
		return false;

	auto const pLoco = pThis->Locomotor.GetInterfacePtr();

	if (!pLoco)
		return false;

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (VTable::Get(pLoco) != TeleportLocomotionClass::ILoco_vtable)
		return false;

	if (pTypeExt->ChronoDelay_Immune.Get())
		return true;

	return HasAbility(pThis, PhobosAbilityType::ChronoDelayDamageImmune);
}

bool TechnoExtData::IsRadImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToRadiation)
		return true;

	return HasAbility(pThis, PhobosAbilityType::RadImmune);
}

bool TechnoExtData::IsPsionicsWeaponImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToPsionicWeapons)
		return true;

	return HasAbility(pThis, PhobosAbilityType::PsionicsWeaponImmune);
}

bool TechnoExtData::IsPoisonImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToPoison)
		return true;

	return HasAbility(pThis, PhobosAbilityType::PoisonImmune);
}

bool TechnoExtData::IsBerserkImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->ImmuneToBerserk.Get())
		return true;

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pShield = pExt->GetShield();

	if (pShield && pShield->IsActive() && pExt->CurrentShieldType->ImmuneToPsychedelic)
		return true;

	return HasAbility(pThis, PhobosAbilityType::BerzerkImmune);
}

bool TechnoExtData::IsAbductorImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->ImmuneToAbduction)
		return true;

	return HasAbility(pThis, PhobosAbilityType::AbductorImmune);
}

bool TechnoExtData::IsAssaulter(InfantryClass* pThis)
{
	if (pThis->Type->Assaulter)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Assaulter);
}

bool TechnoExtData::IsParasiteImmune(TechnoClass* pThis)
{
	if (pThis->GetTechnoType()->Parasiteable)
		return false;

	return HasAbility(pThis, PhobosAbilityType::ParasiteImmune);
}

bool TechnoExtData::IsUnwarpable(TechnoClass* pThis)
{
	if (!pThis->GetTechnoType()->Warpable)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Unwarpable);
}

bool TechnoExtData::IsBountyHunter(TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->Bounty)
		return true;

	return HasAbility(pThis, PhobosAbilityType::BountyHunter);
}

bool TechnoExtData::IsWebImmune(TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ImmuneToWeb)
		return true;

	return HasAbility(pThis, PhobosAbilityType::WebbyImmune);
}

bool TechnoExtData::IsDriverKillProtected(TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ProtectedDriver)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Protected_Driver);
}

bool TechnoExtData::IsInterceptor()
{
	auto const pThis = this->AttachedToObject;
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(Type);

	if (pTypeExt->Interceptor)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Interceptor);
}

void TechnoExtData::CreateInitialPayload(bool forced)
{
	if (!forced) {

		if (this->PayloadCreated)
		{
			return;
		}

		this->PayloadCreated = true;
	}
	auto const pThis = this->AttachedToObject;
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	if (pTypeExt->InitialPayload_Types.empty())
		return;

	auto const pBld = specific_cast<BuildingClass*>(pThis);

	auto freeSlots = (pBld && pBld->Type->CanBeOccupied)
		? pBld->Type->MaxNumberOccupants - pBld->GetOccupantCount()
		: pType->Passengers - pThis->Passengers.NumPassengers;

	auto const sizePayloadNum = pTypeExt->InitialPayload_Nums.size();
	auto const sizePayloadRank = pTypeExt->InitialPayload_Vet.size();
	auto const sizePyloadAddTeam = pTypeExt->InitialPayload_AddToTransportTeam.size();

	for (auto i = 0u; i < pTypeExt->InitialPayload_Types.size(); ++i)
	{
		auto const pPayloadType = pTypeExt->InitialPayload_Types[i];

		if (!pPayloadType)
		{
			continue;
		}

		// buildings and aircraft aren't valid payload, and building payload
		// can only be infantry
		auto const absPayload = pPayloadType->WhatAmI();
		if (absPayload == AbstractType::BuildingType
			|| absPayload == AbstractType::AircraftType
			|| (pBld && absPayload != AbstractType::InfantryType))
		{
			continue;
		}

		// if there are no nums, index gets huge and invalid, which means 1
		auto const idxPayloadNum = MinImpl(i + 1, sizePayloadNum) - 1;
		auto const payloadNum = (idxPayloadNum < sizePayloadNum)
			? pTypeExt->InitialPayload_Nums[idxPayloadNum] : 1;

		auto const rank = idxPayloadNum < sizePayloadRank ?
			pTypeExt->InitialPayload_Vet[idxPayloadNum] : Rank::Invalid;

		auto const addtoteam = idxPayloadNum < sizePyloadAddTeam ?
			pTypeExt->InitialPayload_AddToTransportTeam[idxPayloadNum] : false;

		// never fill in more than allowed
		auto const count = MinImpl(payloadNum, freeSlots);
		freeSlots -= count;

		for (auto j = 0; j < count; ++j)
		{
			auto const pObject = (TechnoClass*)pPayloadType->CreateObject(pThis->Owner);

			if (!pObject)
				continue;

			if (rank == Rank::Veteran)
				pObject->Veterancy.SetVeteran();
			else if (rank == Rank::Elite)
				pObject->Veterancy.SetElite();

			if (pBld)
			{
				// buildings only allow infantry payload, so this in infantry
				auto const pPayload = static_cast<InfantryClass*>(pObject);

				if (pBld->Type->CanBeOccupied)
				{
					pBld->Occupants.AddItem(pPayload);
					auto const pCell = pThis->GetCell();
					TechnoExtContainer::Instance.Find(pPayload)->GarrisonedIn = pBld;
					pThis->UpdateThreatInCell(pCell);
				}
				else
				{
					pPayload->Limbo();

					if (pBld->Type->InfantryAbsorb)
					{
						pPayload->Absorbed = true;

						if (pPayload->CountedAsOwnedSpecial)
						{
							--pPayload->Owner->OwnedInfantry;
							pPayload->CountedAsOwnedSpecial = false;
						}

						if (pBld->Type->ExtraPowerBonus > 0)
						{
							pBld->Owner->RecheckPower = true;
						}
					}
					else
					{
						pPayload->SendCommand(RadioCommand::RequestLink, pBld);
					}

					pBld->Passengers.AddPassenger(pPayload);
					pPayload->AbortMotion();
				}

			}
			else
			{
				auto const pPayload = static_cast<FootClass*>(pObject);
				pPayload->SetLocation(pThis->Location);
				pPayload->Limbo();

				if (pType->OpenTopped)
				{
					pThis->EnteredOpenTopped(pPayload);
				}

				pPayload->Transporter = pThis;

				auto const old = std::exchange(VocClass::VoicesEnabled(), false);
				pThis->AddPassenger(pPayload);
				VocClass::VoicesEnabled = old;

				if (addtoteam) {
					if (auto pTeam = ((FootClass*)pThis)->Team){
						pTeam->AddMember(pPayload, true);
					}
				}
			}
		}
	}
}

bool TechnoExtData::HasAbility(TechnoClass* pThis, PhobosAbilityType nType)
{
	const bool IsVet = pThis->Veterancy.IsVeteran();
	const bool IsElite = pThis->Veterancy.IsElite();

	if (!IsVet && !IsElite)
	{
		return false;
	}

	return HasAbility(IsVet ? Rank::Veteran : Rank::Elite, pThis, nType);
}

bool TechnoExtData::HasImmunity(TechnoClass* pThis, int nType)
{
	const bool IsVet = pThis->Veterancy.IsVeteran();
	const bool IsElite = pThis->Veterancy.IsElite();

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (IsVet)
	{
		return pTypeExt->R_ImmuneToType.Contains(nType) || pTypeExt->V_ImmuneToType.Contains(nType);
	}
	else if (IsElite)
	{
		return  pTypeExt->R_ImmuneToType.Contains(nType) ||
			pTypeExt->V_ImmuneToType.Contains((int)nType) ||
			pTypeExt->E_ImmuneToType.Contains((int)nType);
	}

	return pTypeExt->R_ImmuneToType.Contains(nType);
}

bool TechnoExtData::IsCullingImmune(Rank vet, TechnoClass* pThis)
{
	return HasAbility(vet, pThis, PhobosAbilityType::CullingImmune);
}

bool TechnoExtData::IsEMPImmune(Rank vet, TechnoClass* pThis)
{
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ImmuneToEMP)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::EmpImmune);
}

bool TechnoExtData::IsPsionicsImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();

	if (pType->ImmuneToPsionics)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::PsionicsImmune);
}

bool TechnoExtData::IsCritImmune(Rank vet, TechnoClass* pThis)
{
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ImmuneToCrit)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::CritImmune);
}

bool TechnoExtData::IsChronoDelayDamageImmune(Rank vet, FootClass* pThis)
{
	if (!pThis)
		return false;

	auto const pLoco = pThis->Locomotor.GetInterfacePtr();

	if (!pLoco)
		return false;

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (VTable::Get(pLoco) != TeleportLocomotionClass::ILoco_vtable)
		return false;

	if (!pThis->IsWarpingIn())
		return false;

	if (pTypeExt->ChronoDelay_Immune.Get())
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::ChronoDelayDamageImmune);
}

bool TechnoExtData::IsRadImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToRadiation)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::RadImmune);
}

bool TechnoExtData::IsPsionicsWeaponImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToPsionicWeapons)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::PsionicsWeaponImmune);
}

bool TechnoExtData::IsPoisonImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToPoison)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::PoisonImmune);
}

bool TechnoExtData::IsBerserkImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->ImmuneToBerserk.Get())
		return true;

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pShield = pExt->GetShield();

	if (pShield && pShield->IsActive() && pExt->CurrentShieldType->ImmuneToPsychedelic)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::BerzerkImmune);
}

bool TechnoExtData::IsAbductorImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->ImmuneToAbduction)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::AbductorImmune);
}

bool TechnoExtData::IsAssaulter(Rank vet, InfantryClass* pThis)
{
	if (pThis->Type->Assaulter)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::Assaulter);
}

bool TechnoExtData::IsParasiteImmune(Rank vet, TechnoClass* pThis)
{
	if (pThis->GetTechnoType()->Parasiteable)
		return false;

	return HasAbility(vet, pThis, PhobosAbilityType::ParasiteImmune);
}

bool TechnoExtData::IsUnwarpable(Rank vet, TechnoClass* pThis)
{
	if (!pThis->GetTechnoType()->Warpable)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::Unwarpable);
}

bool TechnoExtData::IsBountyHunter(Rank vet, TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->Bounty)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::BountyHunter);
}

bool TechnoExtData::IsWebImmune(Rank vet, TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ImmuneToWeb)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::WebbyImmune);
}

bool TechnoExtData::IsDriverKillProtected(Rank vet, TechnoClass* pThis)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ProtectedDriver)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::Protected_Driver);
}

bool TechnoExtData::HasAbility(Rank vet, TechnoClass* pThis, PhobosAbilityType nType)
{
	if (nType == PhobosAbilityType::None)
		return false;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (vet == Rank::Veteran)
	{
		return pTypeExt->Phobos_VeteranAbilities.at((int)nType);
	}
	else if (vet == Rank::Elite)
	{
		return  pTypeExt->Phobos_VeteranAbilities.at((int)nType) || pTypeExt->Phobos_EliteAbilities.at((int)nType);
	}

	return false;
}

bool TechnoExtData::HasImmunity(Rank vet, TechnoClass* pThis, int nType)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (vet == Rank::Veteran)
	{
		return pTypeExt->R_ImmuneToType.Contains(nType) || pTypeExt->V_ImmuneToType.Contains(nType);
	}
	else if (vet == Rank::Elite)
	{
		return  pTypeExt->R_ImmuneToType.Contains(nType) || pTypeExt->V_ImmuneToType.Contains((int)nType) || pTypeExt->E_ImmuneToType.Contains((int)nType);
	}

	return pTypeExt->R_ImmuneToType.Contains(nType);
}

#include <Ext/TerrainType/Body.h>

bool TechnoExtData::IsCrushable(ObjectClass* pVictim, TechnoClass* pAttacker)
{
	if (!pVictim || !pAttacker || pVictim->IsBeingWarpedOut())
		return false;

	if (pVictim->IsIronCurtained())
		return false;

	if (pAttacker->Owner && pAttacker->Owner->IsAlliedWith_(pVictim))
		return false;

	auto const pAttackerType = pAttacker->GetTechnoType();
	auto const pVictimTechno = abstract_cast<TechnoClass*>(pVictim);
	auto const pAttackerTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pAttackerType);

	if (!pVictimTechno)
	{
		if (auto const pTerrain = specific_cast<TerrainClass*>(pVictim))
		{
			if (pTerrain->Type->Immune || pTerrain->Type->SpawnsTiberium || !pTerrain->Type->Crushable)
				return false;

			const auto pTerrainExt = TerrainTypeExtContainer::Instance.Find(pTerrain->Type);
			if (pTerrainExt->IsPassable)
				return false;

			return pAttackerTechnoTypeExt->CrushLevel.Get(pAttacker) > pTerrainExt->CrushableLevel;
		}

		return false;
	}

	auto const pWhatVictim = pVictim->WhatAmI();
	auto const pVictimType = pVictim->GetTechnoType();

	if (pAttackerType->OmniCrusher)
	{
		if (pWhatVictim == BuildingClass::AbsID || pVictimType->OmniCrushResistant)
			return false;
	}
	else
	{
		if (pVictimTechno->Uncrushable || !pVictimType->Crushable)
			return false;
	}

	auto const pVictimTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pVictimType);

	if (pWhatVictim == InfantryClass::AbsID)
	{
		const auto& crushableLevel = static_cast<InfantryClass*>(pVictim)->IsDeployed() ?
			pVictimTechnoTypeExt->DeployCrushableLevel :
			pVictimTechnoTypeExt->CrushableLevel;

		if (pAttackerTechnoTypeExt->CrushLevel.Get(pAttacker) < crushableLevel.Get(pVictimTechno))
			return false;
	}

	if (TechnoExtData::IsChronoDelayDamageImmune(abstract_cast<FootClass*>(pVictim)))
	{
		return false;
	}

	//auto const pExt = TechnoExtContainer::Instance.Find(pVictimTechno);
	//if (auto const pShieldData = pExt->Shield.get()) {
	//	auto const pWeaponIDx = pAttacker->SelectWeapon(pVictim);
	//	auto const pWeapon = pAttacker->GetWeapon(pWeaponIDx);

	//	if (pWeapon && pWeapon->WeaponType &&
	//		pShieldData->IsActive() && !pShieldData->CanBeTargeted(pWeapon->WeaponType)) {
	//		return false;
	//	}
	//}

	return true;
}

AreaFireReturnFlag TechnoExtData::ApplyAreaFire(TechnoClass* pThis, CellClass*& pTargetCell, WeaponTypeClass* pWeapon)
{
	const auto pExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	switch (pExt->AreaFire_Target.Get())
	{
	case AreaFireTarget::Random:
	{
		std::vector<CellStruct> adjacentCells {};
		GeneralUtils::AdjacentCellsInRange(adjacentCells, static_cast<size_t>(pWeapon->Range.ToDouble() + 0.99));
		size_t const size = adjacentCells.size();

		for (int i = 0; i < (int)size; i++)
		{
			const int rand = ScenarioClass::Instance->Random.RandomFromMax(size - 1);
			CellStruct const tgtPos = pTargetCell->MapCoords + adjacentCells.at((i + rand) % size);
			CellClass* const tgtCell = MapClass::Instance->GetCellAt(tgtPos);
			bool allowBridges = tgtCell && tgtCell->ContainsBridge() && (pThis->OnBridge || (tgtCell->Level + Unsorted::BridgeLevels) == pThis->GetCell()->Level);

			if (EnumFunctions::AreCellAndObjectsEligible(tgtCell, pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), pThis->Owner, true , allowBridges))
			{
				pTargetCell = tgtCell;
				return AreaFireReturnFlag::Continue;
			}
		}

		return AreaFireReturnFlag::DoNotFire;
	}
	case AreaFireTarget::Self:
	{
		if (!EnumFunctions::AreCellAndObjectsEligible(pThis->GetCell(), pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), nullptr, false, pThis->OnBridge))
			return AreaFireReturnFlag::DoNotFire;

		return AreaFireReturnFlag::SkipSetTarget;
	}
	default:
	{
		auto pCell = pTargetCell;
		bool allowBridges = pCell && pCell->ContainsBridge() && (pThis->OnBridge || (pCell->Level + Unsorted::BridgeLevels) == pThis->GetCell()->Level);

		if (!EnumFunctions::AreCellAndObjectsEligible(pTargetCell, pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), nullptr, false , allowBridges))
			return AreaFireReturnFlag::DoNotFire;
	}
	}

	return AreaFireReturnFlag::ContinueAndReturn;
}

int TechnoExtData::GetThreadPosed(TechnoClass* pThis)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (const auto pShieldData = pExt->GetShield()) {
		if (pShieldData->IsActive()) {
			auto const pShiedType = pShieldData->GetType();
			if (pShiedType->ThreadPosed.isset())
				return pShiedType->ThreadPosed.Get();
		}
	}

	return pExt->Type->ThreatPosed;
}

bool TechnoExtData::IsReallyTechno(TechnoClass* pThis)
{
	const auto pAddr = (((DWORD*)pThis)[0]);
	if (pAddr != UnitClass::vtable
		&& pAddr != AircraftClass::vtable
		&& pAddr != InfantryClass::vtable
		&& pAddr != BuildingClass::vtable)
	{
		return false;
	}

	return true;
}

int TechnoExtData::GetDeployFireWeapon(UnitClass* pThis)
{
	if (pThis->Type->DeployFireWeapon == -1)
		return pThis->TechnoClass::SelectWeapon(pThis->Target ? pThis->Target : pThis->GetCell());

	return pThis->Type->DeployFireWeapon;
}

void TechnoExtData::SetMissionAfterBerzerk(TechnoClass* pThis, bool Immediete)
{
	auto const pType = pThis->GetTechnoType();

	const Mission nEndMission = !pType->ResourceGatherer ?
		pThis->IsArmed() ?
		pThis->Owner && pThis->Owner->IsHumanPlayer ? Mission::Guard : Mission::Area_Guard
		: Mission::Sleep
		: Mission::Harvest;

	pThis->QueueMission(nEndMission, Immediete);
}

std::pair<TechnoClass*, CellClass*> TechnoExtData::GetTargets(ObjectClass* pObjTarget, AbstractClass* pTarget)
{
	TechnoClass* pTargetTechno = nullptr;
	CellClass* targetCell = nullptr;

	//pTarget nullptr check already done above this hook
	if (pTarget && pTarget->WhatAmI() == CellClass::AbsID)
	{
		return { nullptr , targetCell = static_cast<CellClass*>(pTarget) };
	}
	else if (pObjTarget)
	{
		// it is an techno target
		if (((pObjTarget->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None))
		{
			pTargetTechno = static_cast<TechnoClass*>(pObjTarget);
			if (!pTargetTechno->IsInAir())	// Ignore target cell for airborne technos.
				targetCell = pTargetTechno->GetCell();
		}
		else // non techno target , but still an object
		{
			targetCell = pObjTarget->GetCell();
		}
	}

	return { pTargetTechno , targetCell };
}

bool TechnoExtData::ObjectHealthAllowFiring(ObjectClass* pTargetObj, WeaponTypeClass* pWeapon)
{
	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (pTargetObj && pWeaponExt->Targeting_Health_Percent.isset())
	{
		auto const pHP = pTargetObj->GetHealthPercentage();

		if (!pWeaponExt->Targeting_Health_Percent_Below.Get() && pHP <= pWeaponExt->Targeting_Health_Percent.Get())
			return false;
		else if (pWeaponExt->Targeting_Health_Percent_Below.Get() && pHP >= pWeaponExt->Targeting_Health_Percent.Get())
			return false;
	}

	return true;
}

bool TechnoExtData::CheckCellAllowFiring(CellClass* pCell, WeaponTypeClass* pWeapon)
{
	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (pCell && !EnumFunctions::IsCellEligible(pCell, pWeaponExt->CanTarget, true , true))
	{
		return false;
	}

	return true;
}

bool TechnoExtData::TechnoTargetAllowFiring(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);;

	if (!EnumFunctions::IsTechnoEligible(pTarget, pWeaponExt->CanTarget) ||
		!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, pTarget->Owner))
	{
		return false;
	}

	return true;
}

bool TechnoExtData::FireOnceAllowFiring(TechnoClass* pThis, WeaponTypeClass* pWeapon, AbstractClass* pTarget)
{
	const auto pTechnoExt = TechnoExtContainer::Instance.Find(pThis);

	if (auto pUnit = specific_cast<UnitClass*>(pThis))
	{
		if (!pUnit->Type->IsSimpleDeployer && !pUnit->Deployed && pTarget)
		{
			if (pUnit->Type->DeployFire && pWeapon->FireOnce)
			{
				if (pTechnoExt->DeployFireTimer.GetTimeLeft() > 0)
					return false;
			}
		}
	}

	return true;
}

bool TechnoExtData::CheckFundsAllowFiring(TechnoClass* pThis, WarheadTypeClass* pWH)
{
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);
	const int nMoney = pWHExt->TransactMoney;
	if (nMoney != 0 && !pThis->Owner->CanTransactMoney(nMoney))
		return false;

	return true;
}

bool TechnoExtData::InterceptorAllowFiring(TechnoClass* pThis, ObjectClass* pTarget)
{
	//this code is used to remove Techno as auto target consideration , so interceptor can find target faster
	const auto pTechnoExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTechnoExt->IsInterceptor() && pTechnoTypeExt->Interceptor_OnlyTargetBullet.Get())
	{
		if (!pTarget || pTarget->WhatAmI() == BulletClass::AbsID)
		{
			return false;
		}
	}

	return true;
}

bool TechnoExtData::TargetTechnoShieldAllowFiring(TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	const auto pTargetTechnoExt = TechnoExtContainer::Instance.Find(pTarget);
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);

	if (const auto pShieldData = pTargetTechnoExt->Shield.get())
	{
		if (pShieldData->IsActive())
		{
			if (!pShieldData->CanBePenetrated(pWeapon->Warhead))
			{
				if (pWHExt->GetVerses(pShieldData->GetType()->Armor).Verses < 0.0 && pShieldData->GetType()->CanBeHealed)
				{
					const bool IsFullHP = pShieldData->GetHealthRatio() >= RulesClass::Instance->ConditionGreen;
					if (!IsFullHP)
						return true;
					else
					{
						if (pShieldData->GetType()->PassthruNegativeDamage)
						{
							return !(pShieldData->GetHealthRatio() >= RulesClass::Instance->ConditionGreen);
						}
					}
				}

				return false;
			}
		}
	}

	return true;
}

bool TechnoExtData::IsAbductable(TechnoClass* pThis, WeaponTypeClass* pWeapon, FootClass* pFoot)
{

	if (!pFoot->IsAlive
		|| pFoot->InLimbo
		|| pFoot->IsIronCurtained()
		|| pFoot->IsSinking
		|| pFoot->IsCrashing
		|| TechnoExtContainer::Instance.Find(pFoot)->Is_DriverKilled) {
		return false;
	}

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	//Don't abduct the target if it has more life then the abducting percent

	if (pWeaponExt->Abductor_AbductBelowPercent < pFoot->GetHealthPercentage()) {
		return false;
	}

	if (pWeaponExt->Abductor_MaxHealth > 0 && pWeaponExt->Abductor_MaxHealth < pFoot->Health) {
		return false;
	}

	if (TechnoExtData::IsAbductorImmune(pFoot))
		return false;

	if (!TechnoExtData::IsEligibleSize(pThis, pFoot))
		return false;

	if (!TechnoTypeExtData::PassangersAllowed(pThis->GetTechnoType(), pFoot->GetTechnoType()))
		return false;

	return true;
}

void TechnoExtData::SendPlane(AircraftTypeClass* Aircraft, size_t Amount, HouseClass* pOwner, Rank SendRank, Mission SendMission, AbstractClass* pTarget, AbstractClass* pDest)
{
	if (!Aircraft || !pOwner || Amount <= 0)
		return;

	//safeguard
	Mission result = Mission::None;
	switch (SendMission)
	{
	case Mission::Move:
	{
		if (!pDest)
			pDest = pTarget;

		result = SendMission;
	}
	break;
	case Mission::ParadropApproach:
	case Mission::Attack:
	case Mission::SpyplaneApproach:
		result = SendMission;
		break;
	default:
		result = Mission::SpyplaneApproach;
		break;
	}

	const auto edge = pOwner->GetHouseEdge();

	for (size_t i = 0; i < Amount; ++i)
	{
		++Unsorted::ScenarioInit;
		auto const pPlane = static_cast<AircraftClass*>(Aircraft->CreateObject(pOwner));
		--Unsorted::ScenarioInit;

		if (!pPlane)
			continue ;

		pPlane->Spawned = true;
		//randomized
		const auto nCell = MapClass::Instance->PickCellOnEdge(edge, CellStruct::Empty, CellStruct::Empty, SpeedType::Winged, true, MovementZone::Normal);
		pPlane->QueueMission(result, false);

		if (SendRank != Rank::Rookie && SendRank != Rank::Invalid && pPlane->CurrentRanking < SendRank)
			pPlane->Veterancy.SetRank(SendRank);

		if (pDest)
			pPlane->SetDestination(pDest, true);

		if (pTarget)
			pPlane->SetTarget(pTarget);

		bool UnLimboSucceeded = AircraftExt::PlaceReinforcementAircraft(pPlane , nCell);

		if (!UnLimboSucceeded)
		{
			pPlane->UnInit();
		}
		else
		{

			// we cant create InitialPayload when mutex atives
			// so here we handle the InitialPayload Creation !
			// this way we can make opentopped airstrike happen !
			TechnoExtContainer::Instance.Find(pPlane)->CreateInitialPayload();

			if (pPlane->Passengers.GetTotalSize() > 0)
				pPlane->HasPassengers = true;

			pPlane->NextMission();
		}
	}
}

/*
 * Object should NOT be placed on the map (->Limbo() it or don't Put in the first place)
 * otherwise Bad Things (TM) will happen. Again.
 */
bool TechnoExtData::CreateWithDroppod(FootClass* Object, const CoordStruct& XYZ)
{
	auto MyCell = MapClass::Instance->GetCellAt(XYZ);
	if (Object->IsCellOccupied(MyCell, FacingType::None, -1, nullptr, false) != Move::OK)
	{
		return false;
	}
	else
	{
		LocomotionClass::ChangeLocomotorTo(Object, CLSIDs::DropPod);
		CoordStruct xyz = XYZ;
		xyz.Z = 0;

		Object->SetLocation(xyz);
		Object->SetDestination(MyCell, 1);
		Object->Locomotor->Move_To(XYZ);
		Object->PrimaryFacing.Set_Current(DirStruct()); // TODO : random or let loco set the facing

		if (!Object->InLimbo)
		{
			Object->See(0, 0);
			Object->QueueMission(Object->Owner && Object->Owner->IsControlledByHuman_() ? Mission::Guard : Mission::Hunt, 0);
			Object->NextMission();
			return true;
		}

		return false;
	}
}

bool TechnoExtData::TargetFootAllowFiring(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	if ((pTarget->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
	{
		const auto pFoot = static_cast<FootClass*>(pTarget);
		const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

		if (pWeaponExt->Abductor.Get() && !TechnoExtData::IsAbductable(pThis, pWeapon, pFoot))
			return false;

		if (auto const pUnit = specific_cast<UnitClass*>(pTarget))
		{
			if (pUnit->DeathFrameCounter > 0)
				return false;
		}

		if (TechnoExtData::IsChronoDelayDamageImmune(pFoot))
			return false;
	}

	return true;
}

ObjectTypeClass* TechnoExtData::SetInfDefaultDisguise(TechnoClass* const pThis, TechnoTypeClass* const pType)
{
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (auto pDisguise = pExt->DefaultDisguise.Get(nullptr))
	{
		pThis->Disguised = true;
		pThis->DisguisedAsHouse = pThis->Owner;
		return pDisguise;
	}

	return nullptr;
}

void TechnoExtData::UpdateMCOverloadDamage(TechnoClass* pOwner)
{
	auto pThis = pOwner->CaptureManager;

	if (!pThis || !pThis->InfiniteMindControl || pOwner->InLimbo)
		return;

	const auto pOwnerTypeExt = TechnoTypeExtContainer::Instance.Find(pOwner->GetTechnoType());

	if (pThis->OverloadPipState > 0)
		--pThis->OverloadPipState;

	if (pThis->OverloadDamageDelay <= 0)
	{

		const auto OverloadCount = pOwnerTypeExt->Overload_Count.GetElements(RulesClass::Instance->OverloadCount);

		if (OverloadCount.empty())
			return;

		int nCurIdx = 0;
		const int nNodeCount = pThis->ControlNodes.Count;

		for (int i = 0; i < (int)(OverloadCount.size()); ++i)
		{
			if (nNodeCount > OverloadCount[i])
			{
				nCurIdx = i + 1;
			}
		}

		const auto nOverloadfr = pOwnerTypeExt->Overload_Frames.GetElements(RulesClass::Instance->OverloadFrames);
		pThis->OverloadDamageDelay = nOverloadfr.GetItemAtOrMax(nCurIdx);

		const auto nOverloadDmg = pOwnerTypeExt->Overload_Damage.GetElements(RulesClass::Instance->OverloadDamage);
		auto nDamage = nOverloadDmg.GetItemAtOrMax(nCurIdx);

		if (nDamage <= 0)
		{
			pThis->OverloadDeathSoundPlayed = false;
		}
		else
		{
			pThis->OverloadPipState = 10;
			auto const pWarhead = pOwnerTypeExt->Overload_Warhead.Get(RulesClass::Instance->C4Warhead);
			pOwner->ReceiveDamage(&nDamage, 0, pWarhead, 0, 0, 0, 0);

			if (!pThis->OverloadDeathSoundPlayed)
			{
				VocClass::PlayIndexAtPos(pOwnerTypeExt->Overload_DeathSound.Get(RulesClass::Instance->MasterMindOverloadDeathSound), pOwner->Location, 0);
				pThis->OverloadDeathSoundPlayed = true;
			}

			if (auto const pParticle = pOwnerTypeExt->Overload_ParticleSys.Get(RulesClass::Instance->DefaultSparkSystem))
			{
				for (int i = pOwnerTypeExt->Overload_ParticleSysCount.Get(5); i > 0; --i)
				{
					auto const nRandomY = ScenarioClass::Instance->Random.RandomRanged(-200, 200);
					auto const nRamdomX = ScenarioClass::Instance->Random.RandomRanged(-200, 200);
					auto nLoc = pOwner->Location;

					if (pParticle->BehavesLike == ParticleSystemTypeBehavesLike::Smoke)
						nLoc.Z += 100;

					CoordStruct nParticleCoord { pOwner->Location.X + nRamdomX, nRandomY + pOwner->Location.Y, nLoc };
					GameCreate<ParticleSystemClass>(pParticle, nParticleCoord, pOwner->GetCell(), pOwner, CoordStruct::Empty, pOwner->Owner);
				}
			}

			if (nCurIdx > 0 && pOwner->IsAlive)
			{
				double const nBase = (nCurIdx != 1) ? 0.015 : 0.029999999;
				double const nCopied_base = (ScenarioClass::Instance->Random.RandomFromMax(100) < 50) ? -nBase : nBase;
				pOwner->RockingSidewaysPerFrame = static_cast<float>(nCopied_base);
			}
		}

	}
	else
	{
		--pThis->OverloadDamageDelay;
	}
}

bool TechnoExtData::AllowedTargetByZone(TechnoClass* pThis, ObjectClass* pTarget, const TargetZoneScanType& zoneScanType, WeaponTypeClass* pWeapon, std::optional<std::reference_wrapper<const ZoneType>> zone)
{
	if (!pThis || !pTarget)
		return false;

	if (pThis->WhatAmI() == AircraftClass::AbsID)
		return true;

	const auto pThisType = pThis->GetTechnoType();
	const MovementZone mZone = pThisType->MovementZone;
	const ZoneType currentZone = zone ? zone.value() :
		MapClass::Instance->GetMovementZoneType(pThis->InlineMapCoords(), mZone, pThis->OnBridge);

	if (currentZone != ZoneType::None)
	{
		if (zoneScanType == TargetZoneScanType::Any)
			return true;

		const ZoneType targetZone =
			MapClass::Instance->GetMovementZoneType(pTarget->InlineMapCoords(), mZone, pTarget->OnBridge);

		if (zoneScanType == TargetZoneScanType::Same)
		{
			if (currentZone != targetZone)
				return false;
		}
		else
		{
			if (!pWeapon)
			{
				const int weaponIndex = pThis->SelectWeapon(pTarget);

				if (weaponIndex < 0)
					return false;

				if (const auto pWpStruct = pThis->GetWeapon(weaponIndex))
					pWeapon = pWpStruct->WeaponType;
				else
					return false;
			}

			auto const speedType = pThisType->SpeedType;
			const auto cellStruct = MapClass::Instance->NearByLocation(pTarget->InlineMapCoords(),
				speedType, -1, mZone, false, 1, 1, true,
				false, false, speedType != SpeedType::Float, CellStruct::Empty, false, false);

			auto const pCell = MapClass::Instance->GetCellAt(cellStruct);

			if (!pCell)
				return false;

			const double distance = pCell->GetCoordsWithBridge().DistanceFrom(pTarget->GetCenterCoords());

			if (distance > pWeapon->Range)
				return false;
		}
	}

	return true;
}

//ToDo : Auto regenerate and transferable passengers (Problem : Driver killed and operator stuffs )
void TechnoExtData::PutPassengersInCoords(TechnoClass* pTransporter, const CoordStruct& nCoord, AnimTypeClass* pAnimToPlay, int nSound, bool bForce)
{
	if (!pTransporter || !pTransporter->Passengers.NumPassengers || !MapClass::Instance->IsWithinUsableArea(nCoord))
		return;

	//TODO : check if passenger is actually allowed to go outside
	auto pPassenger = pTransporter->Passengers.RemoveFirstPassenger();
	CoordStruct nDest = nCoord;

	//if (bForce)
	{
		//TechnoTypeClass* pPassengerType = pPassenger->GetTechnoType();
		//auto const pPassengerMZone = pPassengerType->MovementZone;
		//auto const pPassengerSpeedType = pPassengerType->SpeedType;


		//auto const pCellFrom = Map.GetCellAt(nCoord);
		//auto const nZone = Map.Zone_56D230(&pCellFrom->MapCoords, pPassengerMZone, pCellFrom->ContainsBridgeEx());

		//if (!Map[nCoord]->IsClearToMove(pPassengerSpeedType, false, false, nZone, pPassengerMZone, -1, 1))
		{
			nDest = MapClass::Instance->GetRandomCoordsNear(nCoord, ScenarioClass::Instance->Random.RandomFromMax(2000), ScenarioClass::Instance->Random.RandomFromMax(1));
		}
	}

	MapClass::Instance->GetCellAt(nCoord)->ScatterContent(pTransporter->GetCoords(), true, true, false);

	bool Placed = false;
	if (bForce)
	{
		++Unsorted::ScenarioInit;
		Placed = pPassenger->Unlimbo(nDest, DirType::North);
		--Unsorted::ScenarioInit;
	}
	else
	{
		Placed = pPassenger->Unlimbo(nDest, DirType::North);
		//Placed = CreateWithDroppod(pPassenger, nDest , LocomotionClass::CLSIDs::Teleport);
	}

	//Only remove passengers from the Transporter if it succeeded
	if (Placed)
	{
		pPassenger->UpdatePlacement(PlacementType::Remove);
		pPassenger->OnBridge = MapClass::Instance->GetCellAt(nCoord)->ContainsBridgeEx();
		pPassenger->UpdatePlacement(PlacementType::Put);
		pPassenger->StopMoving();
		pPassenger->SetDestination(nullptr, true);
		pPassenger->SetTarget(nullptr);
		pPassenger->CurrentTargets.Clear();
		pPassenger->SetFocus(nullptr);
		pPassenger->unknown_C4 = 0; // don't ask
		pPassenger->unknown_5A0 = 0;
		pPassenger->CurrentGattlingStage = 0;
		pPassenger->SetCurrentWeaponStage(0);
		pPassenger->SetLocation(nDest);
		pPassenger->LiberateMember();

		if (pPassenger->SpawnManager)
		{
			pPassenger->SpawnManager->ResetTarget();
		}

		pPassenger->ClearPlanningTokens(nullptr);

		pPassenger->DiscoveredBy(pTransporter->GetOwningHouse());

		if (auto pFoot = generic_cast<FootClass*>(pTransporter))
		{
			if (pTransporter->GetTechnoType()->Gunner)
			{
				pFoot->RemoveGunner(pPassenger);
			}

			if (pTransporter->GetTechnoType()->OpenTopped)
			{
				pFoot->ExitedOpenTopped(pPassenger);
			}
		}
		else if (auto pBuilding = specific_cast<BuildingClass*>(pTransporter))
		{
			if (pBuilding->Absorber())
			{
				pPassenger->Absorbed = false;
				if (pBuilding->Type->ExtraPowerBonus > 0)
				{
					pBuilding->Owner->RecheckPower = true;
				}
			}
		}

		VocClass::PlayIndexAtPos(nSound, nDest);

		if (pAnimToPlay)
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimToPlay, nDest),
				pTransporter->GetOwningHouse(),
				nullptr,
				pTransporter,
				false
			);
		}

		if (pPassenger->CurrentMission != Mission::Guard)
			pPassenger->Override_Mission(Mission::Area_Guard);
	}
	else
	{
		pTransporter->AddPassenger(pPassenger);
	}
}

void TechnoExtData::SyncIronCurtainStatus(TechnoClass* pFrom, TechnoClass* pTo)
{
	if (pFrom->IsIronCurtained() && pFrom->ProtectType == ProtectTypes::IronCurtain)
	{
		const auto bSyncIC = TechnoTypeExtContainer::Instance.Find(pFrom->GetTechnoType())->IronCurtain_SyncDeploysInto
			.Get(RulesExtData::Instance()->IronCurtain_SyncDeploysInto);

		if (bSyncIC)
		{
			pTo->IronCurtain(pFrom->IronCurtainTimer.GetTimeLeft(), pFrom->Owner, false);
			pTo->IronTintStage = pFrom->IronTintStage;
		}
	}
}

void TechnoExtData::PlayAnim(AnimTypeClass* const pAnim, TechnoClass* pInvoker)
{
	if (pAnim && pInvoker) {
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnim, pInvoker->Location),
			pInvoker->GetOwningHouse(),
			nullptr,
			pInvoker,
			false
		);
	}
}

double TechnoExtData::GetDamageMult(TechnoClass* pSouce, bool ForceDisable)
{
	if (!pSouce || !pSouce->IsAlive || ForceDisable)
		return 1.0;

	const auto pType = pSouce->GetTechnoType();

	if (!pType)
		return 1.0;

	const bool firepower = pSouce->HasAbility(AbilityType::Firepower);

	const auto nFirstMult = !firepower ? 1.0 : RulesClass::Instance->VeteranCombat;
	const auto nSecondMult = (!pSouce->Owner || !pSouce->Owner->Type) ? 1.0 : pSouce->Owner->Type->FirepowerMult;
	return nFirstMult * pSouce->FirepowerMultiplier * nSecondMult; ;
}

const BurstFLHBundle* TechnoExtData::PickFLHs(TechnoClass* pThis, int weaponidx)
{
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	const std::vector<BurstFLHBundle>* res = pThis->WhatAmI() == InfantryClass::AbsID ? ((InfantryClass*)pThis)->IsDeployed() && !pExt->DeployedWeaponBurstFLHs.empty() ?
		&pExt->DeployedWeaponBurstFLHs : &pExt->CrouchedWeaponBurstFLHs : &pExt->WeaponBurstFLHs;

	if (res->empty() || res->size() <= (size_t)weaponidx)
		return nullptr;

	return &(*res)[weaponidx];
}

std::pair<bool, CoordStruct> TechnoExtData::GetBurstFLH(TechnoClass* pThis, int weaponIndex)
{
	if (!pThis || weaponIndex < 0)
		return { false ,  CoordStruct::Empty };

	const auto pickedFLHs = PickFLHs(pThis ,weaponIndex);

	if(!pickedFLHs)
		return  { false ,  CoordStruct::Empty };

	const std::vector<CoordStruct>* selected = (pThis->Veterancy.IsElite() ? &pickedFLHs->EFlh : &pickedFLHs->Flh);

	if (!selected->empty() && (int)selected->size() > pThis->CurrentBurstIndex) {
		return { true , (*selected)[pThis->CurrentBurstIndex] };
	}

	return { false , CoordStruct::Empty };
}

const Nullable<CoordStruct>* TechnoExtData::GetInfrantyCrawlFLH(InfantryClass* pThis, int weaponIndex)
{
	const auto pTechnoType = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pThis->IsDeployed())
	{
		if (weaponIndex == 0)
		{
			return pThis->Veterancy.IsElite() ?
				&pTechnoType->E_DeployedPrimaryFireFLH :
				&pTechnoType->DeployedPrimaryFireFLH;
		}
		else if (weaponIndex == 1)
		{
			return pThis->Veterancy.IsElite() ?
				&pTechnoType->E_DeployedSecondaryFireFLH :
				&pTechnoType->DeployedSecondaryFireFLH;
		}
	}
	else
	{
		if (pThis->Crawling)
		{
			if (weaponIndex == 0)
			{
				return pThis->Veterancy.IsElite() ?

					pTechnoType->E_PronePrimaryFireFLH.isset() ?
					&pTechnoType->E_PronePrimaryFireFLH :
					&pTechnoType->Elite_PrimaryCrawlFLH
					:

					pTechnoType->PronePrimaryFireFLH.isset() ?
					&pTechnoType->PronePrimaryFireFLH :
					&pTechnoType->PrimaryCrawlFLH
					;
			}
			else if (weaponIndex == 1)
			{
				return pThis->Veterancy.IsElite() ?
					pTechnoType->E_ProneSecondaryFireFLH.isset() ?
					&pTechnoType->E_ProneSecondaryFireFLH :
					&pTechnoType->E_ProneSecondaryFireFLH
					:

					pTechnoType->ProneSecondaryFireFLH.isset() ?
					&pTechnoType->ProneSecondaryFireFLH :
					&pTechnoType->SecondaryCrawlFLH
					;
			}
		}
	}

	return nullptr;
}

const Armor TechnoExtData::GetTechnoArmor(TechnoClass* pThis, WarheadTypeClass* pWarhead)
{
	auto const pTargetTechnoExt = TechnoExtContainer::Instance.Find(pThis);
	auto nArmor = pThis->GetTechnoType()->Armor;

	if (!pTargetTechnoExt)
		return nArmor;

	auto const pShieldData = pTargetTechnoExt->Shield.get();

	if (!pShieldData)
		return nArmor;

	if (pShieldData->IsActive())
	{
		if (pShieldData->CanBePenetrated(pWarhead))
			return nArmor;

		return pShieldData->GetType()->Armor.Get();
	}

	return nArmor;
}

std::pair<bool, CoordStruct> TechnoExtData::GetInfantryFLH(InfantryClass* pThis, int weaponIndex)
{
	if (!pThis || weaponIndex < 0)
		return { false , CoordStruct::Empty };

	const auto pickedFLH = TechnoExtData::GetInfrantyCrawlFLH(pThis, weaponIndex);

	if (pickedFLH && pickedFLH->isset() && pickedFLH->Get())
	{
		return { true , pickedFLH->Get() };
	}

	return{ false , CoordStruct::Empty };
}

CoordStruct TechnoExtData::PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger, int maxAttempts)
{
	if (!pThis || !pPassenger)
		return CoordStruct::Empty;

	//guarantee
	if (maxAttempts < 1)
		maxAttempts = 1;

	CellClass* pCell = pThis->GetCell();
	CellStruct placeCoords = CellStruct::Empty;
	auto pTypePassenger = pPassenger->GetTechnoType();
	SpeedType speedType = SpeedType::Track;
	MovementZone movementZone = MovementZone::Normal;

	if (pPassenger->WhatAmI() != AircraftClass::AbsID)
	{
		speedType = pTypePassenger->SpeedType;
		movementZone = pTypePassenger->MovementZone;
	}

	Point2D ExtDistance = { 1,1 };
	for (int i = 0; i < maxAttempts; ++i)
	{

		placeCoords = pCell->MapCoords - CellStruct { (short)(ExtDistance.X / 2), (short)(ExtDistance.Y / 2) };
		placeCoords = MapClass::Instance->NearByLocation(placeCoords, speedType, -1, movementZone, false, ExtDistance.X, ExtDistance.Y, true, false, false, false, CellStruct::Empty, false, false);

		pCell = MapClass::Instance->GetCellAt(placeCoords);

		if ((pThis->IsCellOccupied(pCell, FacingType::None, -1, nullptr, false) == Move::OK) && MapClass::Instance->IsWithinUsableArea(pCell->GetCoordsWithBridge())) {
			return pCell->GetCoordsWithBridge();
		}

		++ExtDistance;
	}

	return CoordStruct::Empty;
}

void TechnoExtData::DrawSelectBrd(const TechnoClass* pThis, TechnoTypeClass* pType, int iLength, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry, bool sIsDisguised)
{
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (!pTypeExt->UseCustomSelectBrd.Get(RulesExtData::Instance()->UseSelectBrd.Get(Phobos::Config::EnableSelectBrd)))
		return;

	SHPStruct* SelectBrdSHP = pTypeExt->SHP_SelectBrdSHP
		.Get(isInfantry ? RulesExtData::Instance()->SHP_SelectBrdSHP_INF : RulesExtData::Instance()->SHP_SelectBrdSHP_UNIT);

	if (!SelectBrdSHP)
		return;

	ConvertClass* SelectBrdPAL = (pTypeExt->SHP_SelectBrdPAL ?
		pTypeExt->SHP_SelectBrdPAL :
		(isInfantry ? RulesExtData::Instance()->SHP_SelectBrdPAL_INF : RulesExtData::Instance()->SHP_SelectBrdPAL_UNIT))
		->GetConvert<PaletteManager::Mode::Temperate>();

	if (!SelectBrdPAL)
		return;

	Point2D vPos = { 0, 0 };
	Point2D vLoc = *pLocation;
	int frame, XOffset, YOffset;

	const Point3D selectbrdFrame = pTypeExt->SelectBrd_Frame.Get((isInfantry ? RulesExtData::Instance()->SelectBrd_Frame_Infantry : RulesExtData::Instance()->SelectBrd_Frame_Unit));

	const auto nFlag = BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass | EnumFunctions::GetTranslucentLevel(pTypeExt->SelectBrd_TranslucentLevel.Get(RulesExtData::Instance()->SelectBrd_DefaultTranslucentLevel.Get()));
	const auto canSee = sIsDisguised && pThis->DisguisedAsHouse ? pThis->DisguisedAsHouse->IsAlliedWith_(HouseClass::CurrentPlayer) :
		pThis->Owner->IsAlliedWith_(HouseClass::CurrentPlayer)
		|| HouseClass::CurrentPlayer->IsObserver()
		|| pTypeExt->SelectBrd_ShowEnemy.Get(RulesExtData::Instance()->SelectBrd_DefaultShowEnemy.Get());

	const Point2D offs = pTypeExt->SelectBrd_DrawOffset.Get((isInfantry ?
		RulesExtData::Instance()->SelectBrd_DrawOffset_Infantry : RulesExtData::Instance()->SelectBrd_DrawOffset_Unit));

	XOffset = offs.X;
	YOffset = pTypeExt->AttachedToObject->PixelSelectionBracketDelta + offs.Y;
	vLoc.Y -= 5;

	if (iLength == 8)
	{
		vPos.X = vLoc.X + 1 + XOffset;
		vPos.Y = vLoc.Y + 6 + YOffset;
	}
	else
	{
		vPos.X = vLoc.X + 2 + XOffset;
		vPos.Y = vLoc.Y + 6 + YOffset;
	}

	if (pThis->IsSelected && canSee)
	{
		if (pThis->IsGreenHP())
			frame = selectbrdFrame.X;
		else if (pThis->IsYellowHP())
			frame = selectbrdFrame.Y;
		else
			frame = selectbrdFrame.Z;

		DSurface::Temp->DrawSHP(SelectBrdPAL, SelectBrdSHP,
			frame, &vPos, pBound, nFlag, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

std::pair<TechnoTypeClass*, HouseClass*> TechnoExtData::GetDisguiseType(TechnoClass* pTarget, bool CheckHouse, bool CheckVisibility, bool bVisibleResult)
{
	TechnoTypeClass* pTypeOut = pTarget->GetTechnoType();
	HouseClass* pHouseOut = pTarget->GetOwningHouse();

	//Building cant disguise , so dont bother to check
	if (pTarget->WhatAmI() == BuildingClass::AbsID)
		return { pTypeOut , pHouseOut };

	const bool bIsVisible = !CheckVisibility ? bVisibleResult : (pTarget->IsClearlyVisibleTo(HouseClass::CurrentPlayer));

	if (pTarget->IsDisguised() && !bIsVisible)
	{
		if (CheckHouse)
		{
			if (const auto pDisguiseHouse = pTarget->GetDisguiseHouse(true))
			{
				pHouseOut = pDisguiseHouse;
			}
		}

		if (pTarget->Disguise != pTypeOut)
		{
			if (const auto pDisguiseType = type_cast<TechnoTypeClass*, true>(pTarget->Disguise))
			{
				return { pDisguiseType, pHouseOut };
			}
		}
	}

	return { pTypeOut, pHouseOut };
}

std::tuple<CoordStruct, SHPStruct*, int> GetInsigniaDatas(TechnoClass* pThis, TechnoTypeExtData* pTypeExt)
{
	bool isCustomInsignia = false;
	SHPStruct* pShapeFile = FileSystem::PIPS_SHP;
	const auto nCurRank = pThis->CurrentRanking;
	int frameIndex = pTypeExt->InsigniaFrame.GetFromSpecificRank(nCurRank);
	Point3D insigniaFrames = pTypeExt->InsigniaFrames.Get();
	CoordStruct drawOffs = pTypeExt->InsigniaDrawOffset.Get();
	int DefaultFrameIdx = -1;

	if (SHPStruct* pShapeFileHere = pTypeExt->Insignia.GetFromSpecificRank(nCurRank))
	{
		pShapeFile = pShapeFileHere;
		isCustomInsignia = true;
		DefaultFrameIdx = 0;
	}

	if (pTypeExt->AttachedToObject->Gunner && !pTypeExt->Insignia_Weapon.empty())
	{
		const int weaponIndex = pThis->CurrentWeaponNumber;
		auto const& data = pTypeExt->Insignia_Weapon[weaponIndex];

		if (auto const pCustomShapeFile = data.Shapes.GetFromSpecificRank(nCurRank))
		{
			pShapeFile = pCustomShapeFile;
			isCustomInsignia = true;
			DefaultFrameIdx = 0;
		}

		const int frame = data.Frame.GetFromSpecificRank(nCurRank);

		if (frame != -1)
			frameIndex = frame;

		const auto frames = data.Frames.GetEx();

		if (frames->X != -1 && frames->Y != -1 && frames->Z != -1)
			insigniaFrames = *frames;
	}

	int nRankPerFrames = insigniaFrames.X;
	switch (nCurRank)
	{
	case Rank::Elite:
		DefaultFrameIdx = !isCustomInsignia ? 15 : DefaultFrameIdx;
		nRankPerFrames = insigniaFrames.Z;
		break;
	case Rank::Veteran:
		DefaultFrameIdx = !isCustomInsignia ? 14 : DefaultFrameIdx;
		nRankPerFrames = insigniaFrames.Y;
		break;
	default:
		break;
	}

	const int FrameCalc = (frameIndex == -1 ? nRankPerFrames : frameIndex);
	const int frameIndexRet = FrameCalc == -1 ? DefaultFrameIdx : FrameCalc;

	return { drawOffs, pShapeFile  , frameIndexRet };
}

void TechnoExtData::DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	if (pThis->CurrentRanking == Rank::Invalid)
		return;

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const bool IsObserverPlayer = HouseExtData::IsObserverPlayer();
	Point2D offset = *pLocation;
	TechnoTypeClass* pTechnoType = nullptr;
	auto const& [pTechnoTyper, pOwner] = TechnoExtData::GetDisguiseType(pThis, true, true, false);
	const bool isDisguised = pTechnoTyper != pExt->Type;

	if (isDisguised && IsObserverPlayer)
	{
		pTechnoType = pExt->Type;
	}
	else
	{
		pTechnoType = pTechnoTyper;
	}

	if (!pTechnoType)
		return;

	TechnoTypeExtData* pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

	if (!pTypeExt->DrawInsignia)
		return;

	bool IsAlly = false;
	if(pOwner) {
		IsAlly = pOwner->IsAlliedWith_(HouseClass::CurrentPlayer);
	}

	const bool isVisibleToPlayer = IsAlly
		|| IsObserverPlayer
		|| pTypeExt->Insignia_ShowEnemy.Get(RulesExtData::Instance()->EnemyInsignia);

	if (!isVisibleToPlayer)
		return;

	auto const& [drawOffs, pShapeFile, frameIndex] =
		GetInsigniaDatas(pThis, pTypeExt);

	if (frameIndex != -1 && pShapeFile)
	{
		offset.X += (5 + drawOffs.X);
		offset.Y += (pThis->WhatAmI() != InfantryClass::AbsID ? 4 : 2 + drawOffs.Y);

		DSurface::Temp->DrawSHP(
			FileSystem::PALETTE_PAL, pShapeFile, frameIndex, &offset, pBounds, BlitterFlags(0xE00),
			0, -2 + drawOffs.Z, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	return;
}

bool TechnoExtData::CheckIfCanFireAt(TechnoClass* pThis, AbstractClass* pTarget)
{
	const int wpnIdx = pThis->SelectWeapon(pTarget);
	const FireError fErr = pThis->GetFireError(pTarget, wpnIdx, true);
	if (fErr != FireError::ILLEGAL
		&& fErr != FireError::CANT
		&& fErr != FireError::MOVING
		&& fErr != FireError::RANGE)

	{
		return pThis->IsCloseEnough(pTarget, wpnIdx);
	}
	else
		return false;
}

void TechnoExtData::ForceJumpjetTurnToTarget(TechnoClass* pThis)
{
	const auto pFoot = abstract_cast<UnitClass*>(pThis);
	if (!pFoot)
		return;

	const auto pType = pThis->GetTechnoType();
	const auto pLoco = locomotion_cast<JumpjetLocomotionClass*, true>(pFoot->Locomotor);

	if (pLoco && pThis->IsInAir()
		&& !pType->TurretSpins)
	{
		if (TechnoTypeExtContainer::Instance.Find(pType)->JumpjetTurnToTarget.Get(RulesExtData::Instance()->JumpjetTurnToTarget)
		   && pFoot->GetCurrentSpeed() == 0)
		{
			if (const auto pTarget = pThis->Target)
			{
				if (!pLoco->Facing.Is_Rotating() && TechnoExtData::CheckIfCanFireAt(pThis, pTarget))
				{
					const CoordStruct source = pThis->Location;
					const CoordStruct target = pTarget->GetCoords();
					const DirStruct tgtDir = DirStruct(static_cast<double>(source.Y - target.Y), static_cast<double>(target.X - source.X));

					if (pThis->GetRealFacing().GetFacing<32>() != tgtDir.GetFacing<32>())
						pLoco->Facing.Set_Desired(tgtDir);
				}
			}
		}
	}
}
#include <format>

// convert UTF-8 string to wstring
std::wstring Str2Wstr(const std::string& str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

void TechnoExtData::DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage, WarheadTypeClass* pWH)
{
	if (!pThis || !pThis->IsAlive || pThis->InLimbo || !pThis->IsOnMyView())
		return;

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	const ColorStruct color = isShieldDamage ? damage > 0 ? Phobos::Defines::ShieldPositiveDamageColor : Phobos::Defines::ShieldPositiveDamageColor :
		damage > 0 ? Drawing::DefaultColors[(int)DefaultColorList::Red] : Drawing::DefaultColors[(int)DefaultColorList::Green];

	std::wstring damageStr;

	if (Phobos::Otamaa::IsAdmin)
		damageStr = Str2Wstr(std::format("[{}] {} ({})", pThis->get_ID(), pWH->ID, damage));
	else
		damageStr = std::format(L"{}", damage);

	auto coords = pThis->GetCenterCoords();
	int maxOffset = 30;
	int width = 0, height = 0;
	BitFont::Instance->GetTextDimension(damageStr.c_str(), &width, &height, 120);

	if (pExt->DamageNumberOffset >= maxOffset || pExt->DamageNumberOffset.empty())
		pExt->DamageNumberOffset = -maxOffset;

	if (auto pBuilding = specific_cast<BuildingClass*>(pThis))
		coords.Z += 104 * pBuilding->Type->Height;
	else
		coords.Z += 256;

	if (auto const pCell = MapClass::Instance->TryGetCellAt(coords))
	{
		if (!pCell->IsFogged() && !pCell->IsShrouded())
		{
			if (pThis->VisualCharacter(0, HouseClass::CurrentPlayer()) != VisualType::Hidden)
			{
				FlyingStrings::Add(damageStr.c_str(), coords, color, Point2D { pExt->DamageNumberOffset - (width / 2), 0 });
			}
		}
	}

	pExt->DamageNumberOffset = pExt->DamageNumberOffset + width;
}

int TechnoExtData::GetSizeLeft(FootClass* const pFoot)
{
	return pFoot->GetTechnoType()->Passengers - pFoot->Passengers.GetTotalSize();
}

void TechnoExtData::Stop(TechnoClass* pThis, Mission const& eMission)
{
	pThis->ForceMission(eMission);
	pThis->CurrentTargets.Clear();
	pThis->SetFocus(nullptr);
	pThis->Stun();
}

bool TechnoExtData::IsOnLimbo(TechnoClass* pThis, bool bIgnore)
{
	return !bIgnore && pThis->InLimbo && !pThis->Transporter;
}

bool TechnoExtData::IsDeactivated(TechnoClass* pThis, bool bIgnore)
{
	return !bIgnore && pThis->Deactivated;
}

bool TechnoExtData::IsUnderEMP(TechnoClass* pThis, bool bIgnore)
{
	return !bIgnore && pThis->IsUnderEMP();
}

bool TechnoExtData::IsActive(TechnoClass* pThis, bool bCheckEMP, bool bCheckDeactivated, bool bIgnoreLimbo, bool bIgnoreIsOnMap, bool bIgnoreAbsorb)
{
	if (!TechnoExtData::IsAlive(pThis, bIgnoreLimbo, bIgnoreIsOnMap, bIgnoreAbsorb))
		return false;

	if (pThis->BeingWarpedOut || pThis->TemporalTargetingMe || IsUnderEMP(pThis, !bCheckEMP) || IsDeactivated(pThis, !bCheckDeactivated))
		return false;

	return true;
}

bool TechnoExtData::IsAlive(TechnoClass* pThis, bool bIgnoreLimbo, bool bIgnoreIsOnMap, bool bIgnoreAbsorb)
{
	if (!pThis || !pThis->IsAlive || pThis->Health <= 0)
		return false;

	if ((IsOnLimbo(pThis, !bIgnoreLimbo)) || (pThis->Absorbed && !bIgnoreAbsorb) || (!pThis->IsOnMap && !bIgnoreIsOnMap))
		return false;

	if (pThis->IsCrashing || pThis->IsSinking)
		return false;

	if (pThis->WhatAmI() == UnitClass::AbsID)
		return (static_cast<UnitClass*>(pThis)->DeathFrameCounter > 0) ? false : true;

	return true;
}

void TechnoExtData::ObjectKilledBy(TechnoClass* pVictim, TechnoClass* pKiller)
{
	if (!pKiller || !pVictim)
		return;

	TechnoClass* pObjectKiller = (pKiller->GetTechnoType()->Spawned || pKiller->GetTechnoType()->MissileSpawn) && pKiller->SpawnOwner ?
		pKiller->SpawnOwner : pKiller;

	if (pObjectKiller && pObjectKiller->BelongsToATeam()) {
		if (auto const pFootKiller = generic_cast<FootClass*>(pObjectKiller)) {
			auto pKillerExt = TechnoExtContainer::Instance.Find(pObjectKiller);

			if(auto const pFocus = generic_cast<TechnoClass*>(pFootKiller->Team->Focus))
				pKillerExt->LastKillWasTeamTarget = TeamExtData::GroupAllowed(pFocus->GetTechnoType(), pVictim->GetTechnoType())
			 	//pFocus->GetTechnoType() == pVictim->GetTechnoType()
				;

			auto pKillerTeamExt = TeamExtContainer::Instance.Find(pFootKiller->Team);

			if (pKillerTeamExt->ConditionalJump_EnabledKillsCount)
			{
				bool isValidKill =
					 pKillerTeamExt->ConditionalJump_Index < 0 ?
					 false :
					 ScriptExtData::EvaluateObjectWithMask(pVictim, pKillerTeamExt->ConditionalJump_Index, -1, -1, pKiller);

				if (isValidKill || pKillerExt->LastKillWasTeamTarget)
				    pKillerTeamExt->ConditionalJump_Counter++;
			}

			// Special case for interrupting current action
			if (pKillerTeamExt->AbortActionAfterKilling
				&& pKillerExt->LastKillWasTeamTarget)
			{
				pKillerTeamExt->AbortActionAfterKilling = false;
				auto pTeam = pFootKiller->Team;

				const auto&[curAction , curArgs] = pTeam->CurrentScript->GetCurrentAction();
				const auto&[nextAction , nextArgs] = pTeam->CurrentScript->GetNextAction();

				Debug::Log("DEBUG: [%s] [%s] %d = %d,%d - Force next script action after successful kill: %d = %d,%d\n"
					, pTeam->Type->ID
					, pTeam->CurrentScript->Type->ID
					, pTeam->CurrentScript->CurrentMission
					, curAction
					, curArgs
					, pTeam->CurrentScript->CurrentMission + 1
					, nextAction
					, nextArgs
				);

				// Jumping to the next line of the script list
				pTeam->StepCompleted = true;

			}

		}
	}
}

void TechnoExtData::UpdateMCRangeLimit()
{
	auto const pThis = this->AttachedToObject;
	auto const pCManager = pThis->CaptureManager;

	if (!pCManager)
		return;

	const int Range = TechnoTypeExtContainer::Instance.Find(Type)->MindControlRangeLimit.Get();

	if (Range <= 0)
		return;

	for (const auto& node : pCManager->ControlNodes)
	{
		if (pThis->DistanceFrom(node->Unit) > Range)
			pCManager->FreeUnit(node->Unit);
	}
}

void TechnoExtData::UpdateInterceptor()
{
	auto const pThis = this->AttachedToObject;
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(this->Type);

	if (!this->IsInterceptor() || pThis->Target)
		return;

	if (pThis->WhatAmI() == AircraftClass::AbsID && !pThis->IsInAir())
		return;

	if (auto const pTransport = pThis->Transporter)
	{
		if (pTransport->WhatAmI() == AircraftClass::AbsID && !pTransport->IsInAir())
			return;

		if (TechnoExtData::IsInWarfactory(pTransport))
			return;
	}

	if (TechnoExtData::IsInWarfactory(pThis))
		return;

	BulletClass* pTargetBullet = nullptr;

	const auto& guardRange = pTypeExt->Interceptor_GuardRange.Get(pThis);
	const auto& minguardRange = pTypeExt->Interceptor_MinimumGuardRange.Get(pThis);

	for (auto pBullet : *BulletClass::Array)
	{
		const auto distance = pBullet->Location.DistanceFrom(pThis->Location);

		if (distance > guardRange || distance < minguardRange || pBullet->InLimbo)
			continue;

		const int weaponIndex = pThis->SelectWeapon(pBullet);
		const auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

		if (pTypeExt->Interceptor_ConsiderWeaponRange.Get() &&
			(distance > pWeapon->Range || distance < pWeapon->MinimumRange))
			continue;

		const auto pBulletExt = BulletExtContainer::Instance.Find(pBullet);
		const auto pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);

		if (!pBulletTypeExt->Interceptable ||
			pBulletExt->CurrentStrength <= 0 ||
			pBulletExt->InterceptedStatus == InterceptedStatus::Targeted)
			continue;

		if (pBulletTypeExt->Armor.isset())
		{
			auto const pWhExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
			if (std::abs(pWhExt->GetVerses(pBulletTypeExt->Armor).Verses)
				//GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead , pBulletTypeExt->Armor))
				< 0.001)
				continue;
		}

		const auto bulletOwner = pBullet->Owner ? pBullet->Owner->GetOwningHouse() : pBulletExt->Owner;

		if (EnumFunctions::CanTargetHouse(pTypeExt->Interceptor_CanTargetHouses.Get(), pThis->Owner, bulletOwner))
		{
			pTargetBullet = pBullet;
			break;
		}
	}

	if (pTargetBullet)
	{
		pThis->SetTarget(pTargetBullet);
	}
}

void TechnoExtData::UpdateSpawnLimitRange()
{
	auto const pThis = this->AttachedToObject;
	const auto pExt = TechnoTypeExtContainer::Instance.Find(Type);
	auto const pManager = pThis->SpawnManager;

	if (!pExt->Spawn_LimitedRange || !pManager)
		return;

	int weaponRange = 0;
	const int weaponRangeExtra = pExt->Spawn_LimitedExtraRange.Get() * 256;

	auto setWeaponRange = [&weaponRange](WeaponTypeClass* pWeaponType)
	{
		if (pWeaponType && pWeaponType->Spawner && pWeaponType->Range > weaponRange)
			weaponRange = pWeaponType->Range;
	};

	if (Type->IsGattling || pThis->CurrentWeaponNumber > 0)
	{
		if (auto const pCurWeapon = pThis->GetWeapon(pThis->CurrentWeaponNumber))
			setWeaponRange(pCurWeapon->WeaponType);
	}
	else
	{
		if (auto const pPriWeapon = pThis->GetWeapon(0))
			setWeaponRange(pPriWeapon->WeaponType);

		if (auto const pSecWeapon = pThis->GetWeapon(1))
			setWeaponRange(pSecWeapon->WeaponType);
	}

	weaponRange += weaponRangeExtra;

	if (pManager->Target && (pThis->DistanceFrom(pManager->Target) > weaponRange))
		pManager->ResetTarget();
}

bool TechnoExtData::IsHarvesting(TechnoClass* pThis)
{
	if (!TechnoExtData::IsActive(pThis, true, true))
		return false;

	const auto slave = pThis->SlaveManager;
	if (slave && slave->State != SlaveManagerStatus::Ready)
		return true;

	if (pThis->WhatAmI() == AbstractType::Building && pThis->IsPowerOnline())
		return true;

	const auto mission = pThis->GetCurrentMission();
	if ((mission == Mission::Harvest || mission == Mission::Unload || mission == Mission::Enter)
		&& TechnoExtData::HasAvailableDock(pThis))
	{
		return true;
	}

	return false;
}

bool TechnoExtData::HasAvailableDock(TechnoClass* pThis)
{
	for (auto const& pBld : pThis->GetTechnoType()->Dock)
	{
		if (pThis->Owner->CountOwnedAndPresent(pBld) > 0)
			return true;
	}

	return false;
}

void TechnoExtData::InitializeLaserTrail(TechnoClass* pThis, bool bIsconverted)
{
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pType = pThis->GetTechnoType();

	if (pThis->WhatAmI() == AbstractType::Building || pType->Invisible)
		return;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (bIsconverted)
		pExt->LaserTrails.clear();

	auto const pOwner = pThis->GetOwningHouse() ? pThis->GetOwningHouse() : HouseExtData::FindCivilianSide();

	if (pExt->LaserTrails.empty())
	{
		for (auto const& entry : pTypeExt->LaserTrailData)
		{
			pExt->LaserTrails.emplace_back(
					LaserTrailTypeClass::Array[entry.idxType].get(), pOwner->LaserColor, entry.FLH, entry.IsOnTurret);
		}
	}

}

bool TechnoExtData::FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType)
{
	if (!pWeaponType)
		return false;

	WeaponTypeExtData::DetonateAt(pWeaponType, pThis, pThis, true, nullptr);
	return true;
}

Matrix3D TechnoExtData::GetTransform(TechnoClass* pThis, VoxelIndexKey* pKey)
{
	Matrix3D Mtx { };
	// Step 1: get body transform matrix
	if (pThis && (pThis->AbstractFlags & AbstractFlags::Foot) && ((FootClass*)pThis)->Locomotor)
	{
		((FootClass*)pThis)->Locomotor.GetInterfacePtr()->Draw_Matrix(&Mtx, pKey);
		return Mtx;
	}

	// no locomotor means no rotation or transform of any kind (f.ex. buildings) - Kerbiter

	Mtx.MakeIdentity();
	return 	Mtx;
}

// reversed from 6F3D60
CoordStruct TechnoExtData::GetFLHAbsoluteCoords(TechnoClass* pThis, const CoordStruct& pCoord, bool isOnTurret, const CoordStruct& Overrider)
{
	auto const pType = pThis->GetTechnoType();
	Matrix3D mtx = TechnoExtData::GetTransform(pThis);

	// Steps 2-3: turret offset and rotation
	if (isOnTurret && pThis->HasTurret())
	{
		TechnoTypeExtContainer::Instance.Find(pType)->ApplyTurretOffset(&mtx, 1.0);
		double turretRad = (pThis->TurretFacing().GetFacing<32>() - 8) * -(Math::Pi / 16);
		double bodyRad = (pThis->PrimaryFacing.Current().GetFacing<32>() - 8) * -(Math::Pi / 16);
		float angle = static_cast<float>(turretRad - bodyRad);
		mtx.RotateZ(angle);
	}

	// Step 4: apply FLH offset
	mtx.Translate(static_cast<float>(pCoord.X), static_cast<float>(pCoord.Y), static_cast<float>(pCoord.Z));

	Vector3D<float> result = Matrix3D::MatrixMultiply(mtx, Vector3D<float>::Empty);
	// Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	// Step 5: apply as an offset to global object coords
	CoordStruct location = Overrider ? Overrider : pThis->GetCoords();
	location += { static_cast<int>(result.X), static_cast<int>(result.Y), static_cast<int>(result.Z) };
	// += { std::lround(result.X), std::lround(result.Y), std::lround(result.Z) };

	return location;
}

double TechnoExtData::GetROFMult(TechnoClass const* pTech)
{
	const bool rofAbility = pTech->HasAbility(AbilityType::ROF);

	return !rofAbility ? 1.0 :
		RulesClass::Instance->VeteranROF * ((!pTech->Owner || !pTech->Owner->Type) ?
			1.0 : pTech->Owner->Type->ROFMult);
}

int TechnoExtData::GetEatPassangersTotalTime(TechnoTypeClass* pTransporterData, FootClass const* pPassenger)
{
	auto const pData = TechnoTypeExtContainer::Instance.Find(pTransporterData);
	auto const pThis = this->AttachedToObject;
	auto const& pDelType = pData->PassengerDeletionType;

	int nRate = 0;

	if (pDelType->UseCostAsRate.Get())
	{
		// Use passenger cost as countdown.
		auto timerLength = static_cast<int>(pPassenger->GetTechnoType()->Cost * pDelType->CostMultiplier);
		const auto nCostRateCap = pDelType->CostRateCap.Get(-1);
		if (nCostRateCap > 0)
			timerLength = MinImpl(timerLength, nCostRateCap);

		nRate = timerLength;
	}
	else
	{
		// Use explicit rate optionally multiplied by unit size as countdown.
		auto timerLength = pDelType->Rate.Get();
		if (pDelType->Rate_SizeMultiply.Get() && pPassenger->GetTechnoType()->Size > 1.0)
			timerLength *= static_cast<int>(pPassenger->GetTechnoType()->Size + 0.5);

		nRate = timerLength;
	}

	if (pDelType->Rate_AffectedByVeterancy)
	{
		auto const nRof = TechnoExtData::GetROFMult(pThis);
		if (nRof != 1.0)
		{
			nRate = static_cast<int>(nRate * nRof);
		}
	}

	return nRate;
}

void TechnoExtData::UpdateEatPassengers()
{
	auto const pThis = this->AttachedToObject;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(this->Type);

	if (!pTypeExt->PassengerDeletionType || !TechnoExtData::IsActive(pThis))
		return;

	auto const& pDelType = pTypeExt->PassengerDeletionType;

	if ((pDelType->Rate > 0 || pDelType->UseCostAsRate))
	{
		if (pThis->Passengers.NumPassengers > 0)
		{
			// Passengers / CargoClass is essentially a stack, last in, first out (LIFO) kind of data structure
			FootClass* pPassenger = nullptr;          // Passenger to potentially delete
			FootClass* pPreviousPassenger = nullptr;  // Passenger immediately prior to the deleted one in the stack
			ObjectClass* pLastPassenger = nullptr;    // Passenger that is last in the stack
			auto pCurrentPassenger = pThis->Passengers.GetFirstPassenger();

			// Find the first entered passenger that is eligible for deletion.
			while (pCurrentPassenger)
			{
				if (EnumFunctions::CanTargetHouse(pDelType->AllowedHouses, pThis->Owner, pCurrentPassenger->Owner))
				{
					pPreviousPassenger = abstract_cast<FootClass*>(pLastPassenger);
					pPassenger = pCurrentPassenger;
				}

				pLastPassenger = pCurrentPassenger;
				pCurrentPassenger = abstract_cast<FootClass*>(pCurrentPassenger->NextObject);
			}

			if (!pPassenger)
			{
				this->PassengerDeletionTimer.Stop();
				return;
			}

			if (!this->PassengerDeletionTimer.HasStarted()) // Execute only if timer has been stopped or not started
			{
				// Setting & start countdown. Bigger units needs more time
				int timerLength = this->GetEatPassangersTotalTime(Type, pPassenger);

				if (timerLength <= 0)
					return;

				this->PassengerDeletionTimer.Start(timerLength);
			}

			if (this->PassengerDeletionTimer.Completed()) // Execute only if timer has ran out after being started
			{
				--pThis->Passengers.NumPassengers;

				if (pLastPassenger)
					pLastPassenger->NextObject = nullptr;

				if (pPreviousPassenger)
					pPreviousPassenger->NextObject = pPassenger->NextObject;

				if (pThis->Passengers.NumPassengers <= 0)
					pThis->Passengers.FirstPassenger = nullptr;

				if (auto const pPassengerType = pPassenger->GetTechnoType())
				{
					pPassenger->LiberateMember();

					auto const& nReportSound = pDelType->ReportSound;
					VocClass::PlayIndexAtPos(nReportSound.Get(), pThis->Location);

					auto const pThisOwner = pThis->GetOwningHouse();

					if (const auto pAnimType = pDelType->Anim.Get(nullptr))
					{
						if (auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location))
						{
							pAnim->SetOwnerObject(pThis);
							AnimExtData::SetAnimOwnerHouseKind(pAnim, pThisOwner, pPassenger->GetOwningHouse(), pThis, false);
						}
					}

					// Check if there is money refund
					if (pDelType->Soylent &&
						EnumFunctions::CanTargetHouse(pDelType->SoylentAllowedHouses, pThis->Owner, pPassenger->Owner))
					{
						if (const int nMoneyToGive = (int)(pPassenger->GetTechnoType()->GetRefund(pPassenger->Owner, true) *
							pDelType->SoylentMultiplier))
						{
							pThis->Owner->TransactMoney(nMoneyToGive);

							if (pDelType->DisplaySoylent)
							{
								FlyingStrings::AddMoneyString(true, nMoneyToGive, pThis,
									pDelType->DisplaySoylentToHouses, pThis->Location, pDelType->DisplaySoylentOffset);
							}
						}
					}

					// Handle gunner change.
					if (pThis->GetTechnoType()->Gunner)
					{
						if (auto const pFoot = abstract_cast<FootClass*>(pThis))
						{
							pFoot->RemoveGunner(pPassenger);
							FootClass* pGunner = nullptr;

							for (auto pNext = pThis->Passengers.FirstPassenger; pNext; pNext = abstract_cast<FootClass*>(pNext->NextObject))
								pGunner = pNext;

							pFoot->ReceiveGunner(pGunner);
						}
					}

					if (pThis->GetTechnoType()->OpenTopped)
					{
						pThis->ExitedOpenTopped(pPassenger);
					}

					if (const auto pBld = specific_cast<BuildingClass*>(pThis))
					{
						if (pBld->Absorber())
						{
							pPassenger->Absorbed = false;

							if (pBld->Type->ExtraPowerBonus > 0)
							{
								pBld->Owner->RecheckPower = true;
							}
						}
					}

					//auto const pPassengerOwner = pPassenger->Owner;

					//if (!pPassengerOwner->IsNeutral() && !pThis->GetTechnoType()->Insignificant)
					//{
					//	pPassengerOwner->RegisterLoss(pPassenger, false);
					//	pPassengerOwner->RemoveTracking(pPassenger);

					//	if (!pPassengerOwner->RecheckTechTree)
					//		pPassengerOwner->RecheckTechTree = true;
					//}


					pPassenger->RegisterDestruction(pDelType->DontScore ? nullptr : pThis);
					Debug::Log(__FUNCTION__" Called \n");
					TechnoExtData::HandleRemove(pPassenger, pDelType->DontScore ? nullptr : pThis, false, false);
				}

				this->PassengerDeletionTimer.Stop();
			}
		}
	}
}

bool NOINLINE TechnoExtData::CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex)
{
	if (pThis->GetTechnoType()->Ammo > 0)
	{
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
		if (pThis->Ammo <= pExt->NoAmmoAmount && (pExt->NoAmmoWeapon == weaponIndex || pExt->NoAmmoWeapon == -1))
			return true;
	}

	return false;
}

#include <AircraftTrackerClass.h>

void NOINLINE DeleteTechnoPtr(TechnoClass* pThis) {
	GameDelete<true, false>(pThis);
}

void TechnoExtData::HandleRemove(TechnoClass* pThis, TechnoClass* pSource, bool SkipTrackingRemove, bool Delete)
{
	// kill passenger cargo to prevent memleak
	pThis->KillPassengers(pSource);

	const auto pThisType = pThis->GetTechnoType();
	const auto nWhat = pThis->WhatAmI();
	Debug::Log(__FUNCTION__" Called For[%s - %x][%s - %x]\n", pThisType->ID, pThis, pThis->Owner ? pThis->Owner->get_ID() : GameStrings::NoneStr(), pThis->Owner);

	if (nWhat == BuildingClass::AbsID)
	{
		static_cast<BuildingClass*>(pThis)->KillOccupants(nullptr);
	} else {

		((FootClass*)pThis)->LiberateMember();

		const auto flight = pThis->GetLastFlightMapCoords();
		if(flight.IsValid())
			AircraftTrackerClass::Instance->Remove(pThis);
	}

	if (!SkipTrackingRemove)
	{
		if (const auto pOwner = pThis->GetOwningHouse())
		{
			if (!pThisType->DontScore && !pThisType->Insignificant)
			{
				pOwner->RegisterLoss(pThis, false);
				pOwner->RemoveTracking(pThis);

				if (!pOwner->RecheckTechTree)
					pOwner->RecheckTechTree = true;
			}
		}
	}


	if (Delete)
		GameDelete<true, false>(pThis);
	else
		pThis->UnInit();
}

void TechnoExtData::KillSelf(TechnoClass* pThis, bool isPeaceful)
{
	if (isPeaceful)
	{
		// this shit is not really good idea to pull of
		// some stuffs doesnt really handled properly , wtf
		bool SkipRemoveTracking = false;
		if (!pThis->InLimbo){
			SkipRemoveTracking = true;
			pThis->Limbo();
		}

		Debug::Log(__FUNCTION__" Called \n");
		TechnoExtData::HandleRemove(pThis, nullptr, SkipRemoveTracking, false);
	}
	else
	{
		if(pThis->IsAlive)
			pThis->ReceiveDamage(&pThis->GetType()->Strength, 0, RulesClass::Instance()->C4Warhead, nullptr, false, false, pThis->Owner);
	}
}

KillMethod NOINLINE GetKillMethod(KillMethod deathOption)
{
	if (deathOption == KillMethod::Random)
	{ //ensure no death loop , only random when needed to
		return ScenarioClass::Instance->Random.RandomRangedSpecific<KillMethod>(KillMethod::Explode, KillMethod::Sell);
	}

	return deathOption;
}

void TechnoExtData::KillSelf(TechnoClass* pThis, const KillMethod& deathOption, bool RegisterKill, AnimTypeClass* pVanishAnim)
{
	if (!pThis || deathOption == KillMethod::None || !pThis->IsAlive)
		return;

	auto const pWhat = VTable::Get(pThis);

	switch (GetKillMethod(deathOption))
	{
	case KillMethod::Explode:
	{
		if (pThis && pThis->IsAlive)
		{
			auto nHealth = pThis->GetType()->Strength;
			pThis->ReceiveDamage(&nHealth, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, nullptr);
		}

	}break;
	case KillMethod::Vanish:
	{
		// this shit is not really good idea to pull off
		// some stuffs doesnt really handled properly , wtf

		if (pVanishAnim && !pThis->InLimbo && (pThis->Location.IsValid() && pThis->InlineMapCoords().IsValid()))
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pVanishAnim, pThis->GetCoords()),
				pThis->GetOwningHouse(),
				nullptr,
				 true
			);
		}

		pThis->Stun();
		bool skiptrackingremove = false;

		if (!pThis->InLimbo){
			skiptrackingremove = true;
			pThis->Limbo();
		}

		if (RegisterKill)
			pThis->RegisterKill(pThis->Owner);

		Debug::Log(__FUNCTION__" Called \n");
		TechnoExtData::HandleRemove(pThis, nullptr, skiptrackingremove, false);

	}break;
	case KillMethod::Sell:
	{
		if (pWhat == BuildingClass::vtable)
		{
			const auto pBld = static_cast<BuildingClass*>(pThis);

			if (pBld->HasBuildup && (pBld->CurrentMission != Mission::Selling || pBld->CurrentMission != Mission::Unload))
			{
				BuildingExtContainer::Instance.Find(pBld)->Silent = true;
				pBld->Sell(true);
				return;
			}
		}
		else if (pThis->AbstractFlags & AbstractFlags::Foot)
		{
			const auto pFoot = static_cast<FootClass*>(pThis);

			if (pWhat != InfantryClass::vtable && pFoot->CurrentMission != Mission::Unload)
			{
				if (auto const pCell = pFoot->GetCell())
				{
					if (auto const pBuilding = pCell->GetBuilding())
					{
						if (pBuilding->Type->UnitRepair)
						{
							pFoot->Sell(true);
							return;
						}
					}
				}
			}
		}

		if (pThis && pThis->IsAlive)
		{
			pThis->ReceiveDamage(&pThis->GetType()->Strength, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, nullptr);
		}

	}break;
	}
}

// Feature: Kill Object Automatically
bool TechnoExtData::CheckDeathConditions()
{
	auto const pThis = this->AttachedToObject;
	const auto pTypeThis = this->Type;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(this->Type);

	const KillMethod nMethod = pTypeExt->Death_Method.Get();
	const auto pVanishAnim = pTypeExt->AutoDeath_VanishAnimation.Get();

	if (nMethod == KillMethod::None)
		return false;

	// Death if no ammo
	if (pTypeExt->Death_NoAmmo)
	{
		if (pTypeThis->Ammo > 0 && pThis->Ammo <= 0)
		{
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
			return !pThis->IsAlive;
		}
	}

	//if (pTypeExt->Death_IfChangeOwnership) {
	//	if (pThis->GetOriginalOwner() != pThis->Owner)
	//	{
	//		TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
	//		return !pThis->IsAlive;
	//	}
	//}

	const auto existTechnoTypes = [pThis](const ValueableVector<TechnoTypeClass*>& vTypes, AffectedHouse affectedHouse, bool any, bool allowLimbo)
	{
		const auto existSingleType = [pThis, affectedHouse, allowLimbo](TechnoTypeClass* pType)
		{
			for (HouseClass* pHouse : *HouseClass::Array)
			{
				if (EnumFunctions::CanTargetHouse(affectedHouse, pThis->Owner, pHouse))
				{
					if(allowLimbo) {
						for (const auto& limbo : HouseExtContainer::Instance.Find(pHouse)->LimboTechno) {
							if (!limbo->IsAlive)
								continue;

							const auto limboType = limbo->GetTechnoType();
							if (!limboType->Insignificant && !limboType->DontScore && limboType== pType)
								return true;
						}
					}

					return  pHouse->CountOwnedAndPresent(pType) > 0;
				}
			}

			return false;
		};

		return any
			? vTypes.Any_Of(existSingleType)
			: vTypes.All_Of(existSingleType);
	};

	// Death if nonexist
	if (!pTypeExt->AutoDeath_Nonexist.empty())
	{
		if (!existTechnoTypes(pTypeExt->AutoDeath_Nonexist,
			pTypeExt->AutoDeath_Nonexist_House,
			!pTypeExt->AutoDeath_Nonexist_Any, pTypeExt->AutoDeath_Nonexist_AllowLimboed))
		{
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);

			return !pThis->IsAlive;
		}
	}

	// Death if exist
	if (!pTypeExt->AutoDeath_Exist.empty())
	{
		if (existTechnoTypes(pTypeExt->AutoDeath_Exist,
			pTypeExt->AutoDeath_Exist_House,
			pTypeExt->AutoDeath_Exist_Any,
			pTypeExt->AutoDeath_Exist_AllowLimboed))
		{
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);

			return !pThis->IsAlive;
		}
	}

	// Death if countdown ends
	if (pTypeExt->Death_Countdown > 0)
	{
		if (!Death_Countdown.HasStarted())
		{
			Death_Countdown.Start(pTypeExt->Death_Countdown);
			if (pThis->Owner)
				HouseExtContainer::Instance.Find(pThis->Owner)->AutoDeathObjects.insert(pThis, nMethod);
		}
		else if (Death_Countdown.Completed())
		{
			TechnoExtData::KillSelf(pThis, nMethod, pVanishAnim);
			return !pThis->IsAlive;
		}
	}

	return false;
}

void TechnoExtData::ApplyGainedSelfHeal(TechnoClass* pThis , bool wasDamaged)
{
	TechnoTypeClass* pType = pThis->GetTechnoType();

	if (pThis->Health)
	{
		const auto pWhat = pThis->WhatAmI();
		const bool isBuilding = pWhat == AbstractType::Building;
		const int healthDeficit = pType->Strength - pThis->Health;
		const bool allow = !RulesExtData::Instance()->GainSelfHealAllowMultiplayPassive && (pThis->Owner && pThis->Owner->Type->MultiplayPassive);

		if(healthDeficit > 0 && allow) {

			const bool isOrganic = pWhat == AbstractType::Infantry || (pWhat == AbstractType::Unit && pType->Organic);
			const auto defaultSelfHealType = isBuilding ? SelfHealGainType::None : isOrganic ? SelfHealGainType::Infantry : SelfHealGainType::Units;
			bool applyHeal = false;
			int amount = 0;

			switch (TechnoTypeExtContainer::Instance.Find(pType)->SelfHealGainType.Get(defaultSelfHealType))
			{
			case SelfHealGainType::Infantry:
			{
				if (!pThis->Owner->InfantrySelfHeal)
					return;

				const int count = RulesExtData::Instance()->InfantryGainSelfHealCap.isset() ?
					std::clamp(pThis->Owner->InfantrySelfHeal, 1, RulesExtData::Instance()->InfantryGainSelfHealCap.Get()) :
					pThis->Owner->InfantrySelfHeal;

				amount = RulesClass::Instance->SelfHealInfantryAmount * count;

				if (!(Unsorted::CurrentFrame % RulesClass::Instance->SelfHealInfantryFrames) && amount)
					applyHeal = true;
			}
			break;
			case SelfHealGainType::Units:
			{
				if (!pThis->Owner->UnitsSelfHeal)
					return;

				const int count = RulesExtData::Instance()->UnitsGainSelfHealCap.isset() ?
					std::clamp(pThis->Owner->UnitsSelfHeal, 1, RulesExtData::Instance()->UnitsGainSelfHealCap.Get()) :
					pThis->Owner->UnitsSelfHeal;

				amount = RulesClass::Instance->SelfHealUnitAmount * count;

				if (!(Unsorted::CurrentFrame % RulesClass::Instance->SelfHealUnitFrames) && amount)
					applyHeal = true;
			}
			break;
			default:
				return;
			}

			if (applyHeal && amount)
			{
				if (amount >= healthDeficit)
					amount = healthDeficit;

				if (Phobos::Debug_DisplayDamageNumbers)
					FlyingStrings::AddNumberString(amount, pThis->Owner, AffectedHouse::All, Drawing::DefaultColors[(int)DefaultColorList::White], pThis->Location, Point2D::Empty, false, L"");

				pThis->Health += amount;
			}
		}

		if (wasDamaged && (pThis->GetHealthPercentage() > RulesClass::Instance->ConditionYellow
			|| pThis->GetHeight() < -10))
		{
			bool Rubbled = false;

			if (isBuilding) {

				const auto pBuilding = static_cast<BuildingClass*>(pThis);
				//const auto pExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

				// restore rubble after reaching green health ?
				// not sure if this good implementation
				// maybe better to put it somewhere on update function ,..
				//if(pThis->GetHealthPercentage() >= RulesClass::Instance->ConditionGreen
				//	&& (pExt->RubbleIntact || pExt->RubbleIntactRemove))
				//{
				//	auto pRubble = TechnoExt_ExtData::CreateBuilding(pBuilding,
				//				pExt->RubbleIntactRemove,
				//				pExt->RubbleIntact,
				//				pExt->RubbleIntactOwner,
				//				pExt->RubbleIntactStrength,
				//				pExt->RubbleIntactAnim
				//				);
				//
				//	if (pRubble)
				//	{
				//		const bool wasSelected = pBuilding->IsSelected;
				//		Rubbled = true;
				//
				//		if(pExt->TunnelType != -1 || pBuilding->Absorber() || pBuilding->Type->CanBeOccupied)
				//		{
				//			CellStruct Cell = pThis->GetMapCoords();
				//			if(auto pData = HouseExtData::GetTunnelVector(pBuilding->Type , pBuilding->Owner)) {
				//				if (!pData->Vector.empty() && !TunnelFuncs::FindSameTunnel(pBuilding))
				//				{
				//					int nOffset = 0;
				//					auto nPos = pData->Vector.end();
				//
				//					while (std::begin(pData->Vector) != std::end(pData->Vector))
				//					{
				//						nOffset++;
				//						const auto Offs = CellSpread::CellOfssets[nOffset % CellSpread::CellOfssets.size()];
				//						auto const& [status, pPtr] = TunnelFuncs::UnlimboOne(&pData->Vector, pBuilding, (Cell.X + Offs.X) | ((Cell.Y + Offs.Y) << 16));
				//						if (!status)
				//						{
				//							TunnelFuncs::KillFootClass(pPtr, nullptr);
				//						}
				//					}
				//
				//					pData->Vector.clear();
				//				}
				//
				//			}else
				//			if(pBuilding->Absorber())
				//			{
				//				for(auto pFirst = pBuilding->Passengers.GetFirstPassenger();
				//						 pFirst;
				//						pFirst = generic_cast<FootClass*>(pFirst->NextObject)
				//				){
				//					pRubble->KickOutUnit(pFirst, Cell);
				//				}
				//			}
				//			else
				//			if(pBuilding->Type->CanBeOccupied && pBuilding->Occupants.Count != 0) {
				//				pBuilding->KickAllOccupants(false , false);
				//				if ( pBuilding->Owner->IsControlledByCurrentPlayer() )
				//				{
				//					auto idx = pExt->AbandonedSound.Get(RulesClass::Instance->BuildingAbandonedSound);
				//					if(RadarEventClass::Create(RadarEventType::GarrisonAbandoned , Cell))
				//						VoxClass::Play(GameStrings::EVA_StructureAbandoned());
				//				}
				//			}
				//		}
				//
				//		if (wasSelected) {
				//			pRubble->Select();
				//		}
				//
				//		TechnoExtData::HandleRemove(pBuilding, nullptr, false, false);
				//	}
				//}
				//else
				{
					pBuilding->UpdatePlacement(PlacementType::Redraw);
					pBuilding->ToggleDamagedAnims(false);
				}

			}

			if(!Rubbled) {
				if (auto& dmgParticle = pThis->DamageParticleSystem) {
					dmgParticle->UnInit();
					dmgParticle = nullptr;
				}
			}
		}
	}

	return;
}

void TechnoExtData::ApplyDrainMoney(TechnoClass* pThis)
{
	const auto pSource = pThis->DrainingMe;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pSource->GetTechnoType());
	const auto pRules = RulesClass::Instance();
	const auto nDrainDelay = pTypeExt->DrainMoneyFrameDelay.Get(pRules->DrainMoneyFrameDelay);

	if ((Unsorted::CurrentFrame % nDrainDelay) == 0)
	{
		auto nDrainAmount = pTypeExt->DrainMoneyAmount.Get(pRules->DrainMoneyAmount);
		if (nDrainAmount != 0)
		{
			if (!pThis->Owner->CanTransactMoney(nDrainAmount)
				|| !pSource->Owner->CanTransactMoney(nDrainAmount))
				return;

			pThis->Owner->TransactMoney(-nDrainAmount);
			pSource->Owner->TransactMoney(nDrainAmount);

			if (pTypeExt->DrainMoney_Display)
			{
				auto const pDest = pTypeExt->DrainMoney_Display_AtFirer.Get() ? pSource : pThis;
				FlyingStrings::AddMoneyString(true, nDrainAmount, pDest,
					pTypeExt->DrainMoney_Display_Houses, pDest->Location,
					pTypeExt->DrainMoney_Display_Offset, ColorStruct::Empty);
			}
		}
	}
}

void TechnoExtData::DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	if (!RulesExtData::Instance()->GainSelfHealAllowMultiplayPassive && (pThis->Owner && pThis->Owner->Type->MultiplayPassive))
		return;

	bool drawPip = false;
	bool isInfantryHeal = false;
	int selfHealFrames = 0;

	auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	auto const& nSelfHealType = pExt->SelfHealGainType;

	if (nSelfHealType.isset() && nSelfHealType.Get() == SelfHealGainType::None)
		return;

	auto const pWhat = pThis->WhatAmI();
	const bool hasInfantrySelfHeal = nSelfHealType.isset() && nSelfHealType.Get() == SelfHealGainType::Infantry;
	const bool hasUnitSelfHeal = nSelfHealType.isset() && nSelfHealType.Get() == SelfHealGainType::Units;
	const bool isOrganic = pWhat == InfantryClass::AbsID
		|| (pThis->GetTechnoType()->Organic && (pWhat == UnitClass::AbsID));

	if (pThis->Owner->InfantrySelfHeal > 0 && (hasInfantrySelfHeal || (isOrganic && !hasUnitSelfHeal)))
	{
		drawPip = true;
		selfHealFrames = RulesClass::Instance->SelfHealInfantryFrames;
		isInfantryHeal = true;
	}
	else if (pThis->Owner->UnitsSelfHeal > 0
		&& (hasUnitSelfHeal || (pWhat == UnitClass::AbsID && !isOrganic)))
	{
		drawPip = true;
		selfHealFrames = RulesClass::Instance->SelfHealUnitFrames;
	}

	if (drawPip)
	{
		Point2D pipFrames { 0,0 };
		bool isSelfHealFrame = false;
		int xOffset = 0;
		int yOffset = 0;

		if (Unsorted::CurrentFrame % selfHealFrames <= 5
			&& pThis->Health < pThis->GetTechnoType()->Strength)
		{
			isSelfHealFrame = true;
		}

		int nBracket = TechnoExtData::GetDisguiseType(pThis, false, true).first->PixelSelectionBracketDelta;

		switch (pWhat)
		{
		case UnitClass::AbsID:
		case AircraftClass::AbsID:
		{
			const auto& offset = RulesExtData::Instance()->Pips_SelfHeal_Units_Offset.Get();
			pipFrames = RulesExtData::Instance()->Pips_SelfHeal_Units.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		case InfantryClass::AbsID:
		{
			const auto& offset = RulesExtData::Instance()->Pips_SelfHeal_Infantry_Offset.Get();
			pipFrames = RulesExtData::Instance()->Pips_SelfHeal_Infantry.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		case BuildingClass::AbsID:
		{
			const auto pType = static_cast<BuildingTypeClass*>(pThis->GetTechnoType());
			int fHeight = pType->GetFoundationHeight(false);
			int yAdjust = -Unsorted::CellHeightInPixels / 2;

			const auto& offset = RulesExtData::Instance()->Pips_SelfHeal_Buildings_Offset.Get();
			pipFrames = RulesExtData::Instance()->Pips_SelfHeal_Buildings.Get();
			xOffset = offset.X + Unsorted::CellWidthInPixels / 2 * fHeight;
			yOffset = offset.Y + yAdjust * fHeight + pType->Height * yAdjust;
		}
		break;
		}

		int pipFrame = isInfantryHeal ? pipFrames.X : pipFrames.Y;

		Point2D position { pLocation->X + xOffset, pLocation->Y + yOffset };

		auto flags = BlitterFlags::bf_400 | BlitterFlags::Centered;

		if (isSelfHealFrame)
			flags = flags | BlitterFlags::Darken;

		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
			pipFrame, &position, pBounds, flags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void TechnoExtData::TransformFLHForTurret(TechnoClass* pThis, Matrix3D& mtx, bool isOnTurret)
{
	auto const pType = pThis->GetTechnoType();

	// turret offset and rotation
	if (isOnTurret && pThis->HasTurret())
	{
		TechnoTypeExtContainer::Instance.Find(pType)->ApplyTurretOffset(&mtx, TechnoTypeExtData::TurretMultiOffsetDefaultMult);
		double turretRad = (pThis->TurretFacing().GetFacing<32>() - 8) * -(Math::Pi / 16);
		double bodyRad = (pThis->PrimaryFacing.Current().GetFacing<32>() - 8) * -(Math::Pi / 16);
		float angle = (float)(turretRad - bodyRad);

		mtx.RotateZ(angle);
	}
}

Matrix3D TechnoExtData::GetFLHMatrix(TechnoClass* pThis, const CoordStruct& nCoord, bool isOnTurret)
{
	Matrix3D nMTX = TechnoExtData::GetTransform(abstract_cast<FootClass*>(pThis));
	TechnoExtData::TransformFLHForTurret(pThis, nMTX, isOnTurret);

	// apply FLH offset
	nMTX.Translate((float)nCoord.X, (float)nCoord.Y, (float)nCoord.Z);

	return nMTX;
}

CoordStruct TechnoExtData::GetFLHAbsoluteCoordsB(TechnoClass* pThis, const CoordStruct& nCoord, bool isOnTurret)
{
	Vector3D<float> result = Matrix3D::MatrixMultiply(
	TechnoExtData::GetFLHMatrix(pThis, nCoord, isOnTurret), Vector3D<float>::Empty);

	// Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	// apply as an offset to global object coords
	CoordStruct location = pThis->GetCoords();
	location += { std::lround(result.X), std::lround(result.Y), std::lround(result.Z) };

	return location;
}

void TechnoExtData::UpdateSharedAmmo(TechnoClass* pThis)
{
	if (!pThis)
		return;

	const auto pType = pThis->GetTechnoType();

	if (!pType->OpenTopped || !pThis->Passengers.NumPassengers)
		return;

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (!pExt->Ammo_Shared || !pType->Ammo)
		return;

	auto passenger = pThis->Passengers.FirstPassenger;

	do
	{

		TechnoTypeClass* passengerType = passenger->GetTechnoType();

		if (TechnoTypeExtContainer::Instance.Find(passengerType)->Ammo_Shared.Get())
		{
			if (pExt->Ammo_Shared_Group.Get() < 0 ||
				pExt->Ammo_Shared_Group.Get() == TechnoTypeExtContainer::Instance.Find(passengerType)->Ammo_Shared_Group.Get())
			{
				if (pThis->Ammo > 0 && (passenger->Ammo < passengerType->Ammo))
				{
					pThis->Ammo--;
					passenger->Ammo++;
				}
			}
		}

		passenger = static_cast<FootClass*>(passenger->NextObject);

	}
	while (passenger);
}

double TechnoExtData::GetCurrentSpeedMultiplier(FootClass* pThis)
{
	double houseMultiplier = 1.0;

	switch (pThis->WhatAmI())
	{
	case AircraftClass::AbsID:
		houseMultiplier = pThis->Owner->Type->SpeedAircraftMult;
		break;
	case InfantryClass::AbsID:
		houseMultiplier = pThis->Owner->Type->SpeedInfantryMult;
		break;
	case UnitClass::AbsID:
		houseMultiplier = pThis->Owner->Type->SpeedUnitsMult;
		break;
	}

	return pThis->SpeedMultiplier * houseMultiplier *
		(pThis->HasAbility(AbilityType::Faster) ? RulesClass::Instance->VeteranSpeed : 1.0);
}

void TechnoExtData::UpdateMindControlAnim()
{
	const auto pThis = this->AttachedToObject;

	if (pThis->IsMindControlled())
	{
		if (pThis->MindControlRingAnim && !MindControlRingAnimType)
		{
			MindControlRingAnimType = pThis->MindControlRingAnim->Type;
		}
		else if (!pThis->MindControlRingAnim && MindControlRingAnimType &&
			pThis->CloakState == CloakState::Uncloaked && !pThis->InLimbo && pThis->IsAlive)
		{
			auto coords = pThis->GetCoords();
			int offset = 0;
			const auto pBuilding = specific_cast<BuildingClass*>(pThis);

			if (pBuilding)
				offset = Unsorted::LevelHeight * pBuilding->Type->Height;
			else
				offset = pThis->GetTechnoType()->MindControlRingOffset;

			coords.Z += offset;

			if (auto anim = GameCreate<AnimClass>(MindControlRingAnimType, coords, 0, 1))
			{
				pThis->MindControlRingAnim = anim;
				pThis->MindControlRingAnim->SetOwnerObject(pThis);

				if (pBuilding)
					pThis->MindControlRingAnim->ZAdjust = -1024;
			}
		}
	}
	else if (MindControlRingAnimType)
	{
		MindControlRingAnimType = nullptr;
	}
}

std::pair<const std::vector<WeaponTypeClass*>*, const std::vector<int>*> TechnoExtData::GetFireSelfData()
{
	const auto pThis = this->AttachedToObject;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(Type);

	if (pThis->IsRedHP() && !pTypeExt->FireSelf_Weapon_RedHeath.empty() && !pTypeExt->FireSelf_ROF_RedHeath.empty())
	{
		return  { &pTypeExt->FireSelf_Weapon_RedHeath , &pTypeExt->FireSelf_ROF_RedHeath };
	}
	else if (pThis->IsYellowHP() && !pTypeExt->FireSelf_Weapon_YellowHeath.empty() && !pTypeExt->FireSelf_ROF_YellowHeath.empty())
	{
		return  { &pTypeExt->FireSelf_Weapon_YellowHeath , &pTypeExt->FireSelf_ROF_YellowHeath };
	}
	else if (pThis->IsGreenHP() && !pTypeExt->FireSelf_Weapon_GreenHeath.empty() && !pTypeExt->FireSelf_ROF_GreenHeath.empty())
	{
		return  { &pTypeExt->FireSelf_Weapon_GreenHeath , &pTypeExt->FireSelf_ROF_GreenHeath };
	}

	return  { &pTypeExt->FireSelf_Weapon , &pTypeExt->FireSelf_ROF };

}

void TechnoExtData::UpdateOnTunnelEnter()
{
	if (!this->IsInTunnel)
	{
		if (auto& pShieldData = this->Shield)
			pShieldData->SetAnimationVisibility(false);

		for (auto pos = this->LaserTrails.begin();
			pos != this->LaserTrails.end(); ++pos)
		{
			pos->Visible = false;
			pos->LastLocation.clear();
		}

		TrailsManager::Hide(this->AttachedToObject);

		this->IsInTunnel = true;
	}
}

std::pair<WeaponTypeClass*, int> TechnoExtData::GetDeployFireWeapon(TechnoClass* pThis, AbstractClass* pTarget)
{
	auto const pType = pThis->GetTechnoType();
	int weaponIndex = pType->DeployFireWeapon;

	if (pThis->WhatAmI() == UnitClass::AbsID)
	{
		if (auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType))
		{
			// Only apply DeployFireWeapon on vehicles if explicitly set.
			if (!pTypeExt->DeployFireWeapon.isset())
			{
				weaponIndex = 0;
				if (pThis->GetFireError(pThis->GetCell(), 0, true) != FireError::OK)
					weaponIndex = 1;
			}
		}
	}

	if (weaponIndex == -1)
	{
		weaponIndex = pThis->SelectWeapon(pTarget);
	}
	else if (weaponIndex < -1)
		return { nullptr   , -1 };

	if (auto const pWs = pThis->GetWeapon(weaponIndex))
	{
		return { pWs->WeaponType  , weaponIndex };
	}

	return { nullptr , -1 };
}

void TechnoExtData::UpdateType(TechnoTypeClass* currentType)
{
	auto const pThis = this->AttachedToObject;
	const auto pOldType = this->Type;
	const auto pOldTypeExt = TechnoTypeExtContainer::Instance.Find(pOldType);
	this->Type = currentType;
	auto const pTypeExtData = TechnoTypeExtContainer::Instance.Find(currentType);

	//if (auto pSpawnManager = pThis->SpawnManager)
	//{
	//	if (currentType->Spawns && pSpawnManager->SpawnType != currentType->Spawns)
	//	{
	//		pSpawnManager->SpawnType = currentType->Spawns;
	//
	//		if (currentType->SpawnsNumber > 0)
	//			pSpawnManager->SpawnCount = currentType->SpawnsNumber;
	//
	//		pSpawnManager->RegenRate = currentType->SpawnRegenRate;
	//		pSpawnManager->ReloadRate = currentType->SpawnReloadRate;
	//	}
	//}

	// https://bugs.launchpad.net/ares/+bug/1891753
	//if (pThis->IsDisguised() && pOldType->DisguiseWhenStill && !currentType->DisguiseWhenStill)
	//	pThis->ClearDisguise();

	//else if(currentType->Spawns && currentType->SpawnsNumber > 0)
	//{
	//	pThis->SpawnManager = GameCreate<SpawnManagerClass>(
	//		pThis,
	//		currentType->Spawns ,
	//		currentType->SpawnsNumber ,
	//		currentType->SpawnRegenRate ,
	//		currentType->SpawnReloadRate);
	//}

	TechnoExtData::InitializeLaserTrail(pThis, true);

	// Reset Shield
	// This part should have been done by UpdateShield

	// Reset AutoDeath Timer
	if (this->Death_Countdown.HasStarted())
	{
		this->Death_Countdown.Stop();

		if (pThis->Owner)
		{
			HouseExtContainer::Instance.Find(pThis->Owner)->AutoDeathObjects.erase(pThis);
		}
	}

	// Reset PassengerDeletion Timer - TODO : unchecked
	if (this->PassengerDeletionTimer.IsTicking()
		&& pTypeExtData->PassengerDeletionType
		&& pTypeExtData->PassengerDeletionType->Rate <= 0)
		this->PassengerDeletionTimer.Stop();

	TrailsManager::Construct(static_cast<TechnoClass*>(pThis), true);

	/*if (!pTypeExtData->MyFighterData.Enable && this->MyFighterData)
		this->MyFighterData.reset(nullptr);

	else if (pTypeExtData->MyFighterData.Enable && !this->MyFighterData)
	{
		this->MyFighterData = std::make_unique<FighterAreaGuard>();
		this->MyFighterData->OwnerObject = (AircraftClass*)pThis;
	}

	if(!pTypeExtData->DamageSelfData.Enable && this->DamageSelfState)
		this->DamageSelfState.reset(nullptr);
	else if (pTypeExtData->DamageSelfData.Enable && !this->DamageSelfState)
		DamageSelfState::OnPut(this->DamageSelfState, pTypeExtData->DamageSelfData);

	if (!pTypeExtData->MyGiftBoxData.Enable && this->MyGiftBox)
		this->MyGiftBox.reset(nullptr);
	else if (pTypeExtData->MyGiftBoxData.Enable && !this->MyGiftBox)
		GiftBoxFunctional::Init(this, pTypeExtData);*/
}

void TechnoExtData::UpdateBuildingLightning()
{
	auto const pThis = this->AttachedToObject;

	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	auto pBldExt = BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pThis));
	if (pBldExt->LighningNeedUpdate)
	{
		pThis->UpdatePlacement(PlacementType::Redraw);
		pBldExt->LighningNeedUpdate = false;

	}
}

void TechnoExtData::UpdateShield()
{
	auto const pThis = this->AttachedToObject;

	if (!this->CurrentShieldType)
		Debug::FatalErrorAndExit("Techno[%s] Missing CurrentShieldType ! \n", pThis->get_ID());

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(this->Type);

	// Set current shield type if it is not set.
	if (!this->CurrentShieldType->Strength && pTypeExt->ShieldType->Strength)
		this->CurrentShieldType = pTypeExt->ShieldType;

	// Create shield class instance if it does not exist.
	if (this->CurrentShieldType && this->CurrentShieldType->Strength && !this->Shield)
	{
		this->Shield = std::make_unique<ShieldClass>(pThis);
		//this->Shield->OnInit();
	}

	if (const  auto pShieldData = this->GetShield())
		pShieldData->OnUpdate();
}

void TechnoExtData::UpdateMobileRefinery()
{
	auto const pThis = this->AttachedToObject;

	if (!(pThis->AbstractFlags & AbstractFlags::Foot))
		return;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(Type);

	if (!pTypeExt->MobileRefinery || !pThis->Owner || (pTypeExt->MobileRefinery_TransRate > 0 &&
		Unsorted::CurrentFrame % pTypeExt->MobileRefinery_TransRate))
		return;

	const int cellCount =
		std::clamp(static_cast<int>(pTypeExt->MobileRefinery_FrontOffset.size()), 1,
				static_cast<int>(pTypeExt->MobileRefinery_LeftOffset.size()));

	CoordStruct flh = { 0,0,0 };

	for (int idx = 0; idx < cellCount; idx++)
	{
		flh.X = static_cast<int>(pTypeExt->MobileRefinery_FrontOffset.size()) > idx ? pTypeExt->MobileRefinery_FrontOffset[idx] * Unsorted::LeptonsPerCell : 0;
		flh.Y = static_cast<int>(pTypeExt->MobileRefinery_LeftOffset.size()) > idx ? pTypeExt->MobileRefinery_LeftOffset[idx] * Unsorted::LeptonsPerCell : 0;
		auto nPos = TechnoExtData::GetFLHAbsoluteCoords(pThis, flh, false);
		const CellClass* pCell = MapClass::Instance->GetCellAt(nPos);

		if (!pCell)
			continue;

		nPos.Z += pThis->Location.Z;

		if (const int tValue = pCell->GetContainedTiberiumValue())
		{
			const int tibValue = TiberiumClass::Array->GetItem(pCell->GetContainedTiberiumIndex())->Value;
			const int tAmount = static_cast<int>(tValue * 1.0 / tibValue);
			const int amount = pTypeExt->MobileRefinery_AmountPerCell ? MinImpl(tAmount, pTypeExt->MobileRefinery_AmountPerCell.Get()) : tAmount;
			pCell->ReduceTiberium(amount);
			const int value = static_cast<int>(amount * tibValue * pTypeExt->MobileRefinery_CashMultiplier);

			if (pThis->Owner->CanTransactMoney(value))
			{
				pThis->Owner->TransactMoney(value);
				FlyingStrings::AddMoneyString(pTypeExt->MobileRefinery_Display, value, pThis, AffectedHouse::All, nPos, Point2D::Empty, pTypeExt->MobileRefinery_DisplayColor);
			}

			if (!pTypeExt->MobileRefinery_Anims.empty())
			{
				AnimTypeClass* pAnimType = nullptr;
				int facing = pThis->PrimaryFacing.Current().GetFacing<8>();

				if (facing >= 7)
					facing = 0;
				else
					facing++;

				switch (pTypeExt->MobileRefinery_Anims.size())
				{
				case 1:
					pAnimType = pTypeExt->MobileRefinery_Anims[0];
					break;
				case 8:
					pAnimType = pTypeExt->MobileRefinery_Anims[facing];
					break;
				default:
					pAnimType = pTypeExt->MobileRefinery_Anims[
						ScenarioClass::Instance->Random.RandomFromMax(pTypeExt->MobileRefinery_Anims.size() - 1)];
					break;
				}

				if (pAnimType)
				{
					if (auto pAnim = GameCreate<AnimClass>(pAnimType, nPos))
					{
						AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);

						if (pTypeExt->MobileRefinery_AnimMove)
							pAnim->SetOwnerObject(pThis);
					}
				}
			}
		}
	}
}

void TechnoExtData::UpdateRevengeWeapons()
{
	if (this->RevengeWeapons.empty())
		return;

	for (size_t i = 0; i < this->RevengeWeapons.size(); i++)
	{
		auto const& weapon = this->RevengeWeapons[i];

		if (weapon.Timer.Expired())
			this->RevengeWeapons.erase(this->RevengeWeapons.begin() + i);
	}
}

void TechnoExtData::UpdateAircraftOpentopped()
{
	auto const pThis = this->AttachedToObject;

	if (!TechnoExtData::IsAlive(pThis, true, true, true))
		return;

	if (Type->Passengers > 0 && !AircraftOpentoppedInitEd)
	{

		for (NextObject object(pThis->Passengers.GetFirstPassenger()); object; ++object)
		{
			if (auto const pInf = generic_cast<FootClass*>(*object))
			{
				if (!pInf->Transporter || !pInf->InOpenToppedTransport)
				{
					if (Type->OpenTopped)
						pThis->EnteredOpenTopped(pInf);

					if (Type->Gunner)
						abstract_cast<FootClass*>(pThis)->ReceiveGunner(pInf);

					pInf->Transporter = pThis;
					pInf->Undiscover();
				}
			}
		}

		AircraftOpentoppedInitEd = true;
	}
}

void TechnoExtData::DepletedAmmoActions()
{
	const auto pUnit = specific_cast<UnitClass*>(this->AttachedToObject);

	if (!pUnit || (pUnit->Type->Ammo <= 0) || !pUnit->Type->IsSimpleDeployer)
		return;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(Type);

	const bool skipMinimum = pTypeExt->Ammo_AutoDeployMinimumAmount < 0;
	const bool skipMaximum = pTypeExt->Ammo_AutoDeployMaximumAmount < 0;

	if (skipMinimum && skipMaximum)
		return;

	const bool moreThanMinimum = pUnit->Ammo >= pTypeExt->Ammo_AutoDeployMinimumAmount;
	const bool lessThanMaximum = pUnit->Ammo <= pTypeExt->Ammo_AutoDeployMaximumAmount;

	if ((skipMinimum || moreThanMinimum) && (skipMaximum || lessThanMaximum))
		pUnit->QueueMission(Mission::Unload, true);
}

void TechnoExtData::UpdateLaserTrails()
{
	auto const pThis = this->AttachedToObject;

	if (LaserTrails.empty())
		return;

	for (auto& trail : LaserTrails)
	{
		if (pThis->CloakState == CloakState::Cloaked && !trail.Type->CloakVisible)
			continue;

		if (!IsInTunnel)
			trail.Visible = true;

		if (pThis->WhatAmI() == AircraftClass::AbsID && !pThis->IsInAir() && trail.LastLocation.isset())
			trail.LastLocation.clear();

		CoordStruct trailLoc = TechnoExtData::GetFLHAbsoluteCoords(pThis, trail.FLH, trail.IsOnTurret);

		if (pThis->CloakState == CloakState::Uncloaking && !trail.Type->CloakVisible)
			trail.LastLocation = trailLoc;
		else
			trail.Update(trailLoc);
	}
}

void TechnoExtData::UpdateGattlingOverloadDamage()
{
	auto const pThis = this->AttachedToObject;

	if (!pThis->IsAlive)
		return;

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(Type);
	const auto pType = Type;

	if (!pType->IsGattling || !pTypeExt->Gattling_Overload.Get())
		return;

	auto const nDelay = abs(pTypeExt->Gattling_Overload_Frames.Get(0));

	if (!nDelay)
		return;

	auto const curValue = pThis->GattlingValue;
	auto const maxValue = pThis->Veterancy.IsElite() ? pType->EliteStage[pType->WeaponStages - 1] : pType->WeaponStage[pType->WeaponStages - 1];

	if (GattlingDmageDelay <= 0)
	{
		int nStage = curValue;
		if (nStage < maxValue)
		{
			GattlingDmageDelay = -1;
			GattlingDmageSound = false;
			return;
		}

		GattlingDmageDelay = nDelay;
		auto nDamage = pTypeExt->Gattling_Overload_Damage.Get();

		if (nDamage <= 0)
		{
			GattlingDmageSound = false;
		}
		else
		{
			auto const pWarhead = pTypeExt->Gattling_Overload_Warhead.Get(RulesClass::Instance->C4Warhead);
			pThis->ReceiveDamage(&nDamage, 0, pWarhead, 0, 0, 0, 0);

			if (!GattlingDmageSound)
			{
				if (pTypeExt->Gattling_Overload_DeathSound.isset())
					VocClass::PlayIndexAtPos(pTypeExt->Gattling_Overload_DeathSound, pThis->Location, 0);

				GattlingDmageSound = true;
			}

			if (auto const pParticle = pTypeExt->Gattling_Overload_ParticleSys.Get(nullptr))
			{
				for (int i = pTypeExt->Gattling_Overload_ParticleSysCount.Get(1); i > 0; --i)
				{
					auto const nRandomY = ScenarioClass::Instance->Random(-200, 200);
					auto const nRamdomX = ScenarioClass::Instance->Random(-200, 200);
					auto nLoc = pThis->Location;

					if (pParticle->BehavesLike == ParticleSystemTypeBehavesLike::Smoke)
						nLoc.Z += 100;

					CoordStruct nParticleCoord { pThis->Location.X + nRamdomX, nRandomY + pThis->Location.Y, pThis->Location.Z + 100 };
					GameCreate<ParticleSystemClass>(pParticle, nParticleCoord, pThis->GetCell(), pThis, CoordStruct::Empty, pThis->Owner);
				}
			}

			if (pThis->WhatAmI() == UnitClass::AbsID && pThis->IsAlive && pThis->IsVoxel())
			{
				double const nBase = ScenarioClass::Instance->Random.RandomBool() ? 0.015 : 0.029999999;
				double const nCopied_base = (ScenarioClass::Instance->Random.RandomFromMax(100) < 50) ? -nBase : nBase;
				pThis->RockingSidewaysPerFrame = (float)nCopied_base;
			}
		}
	}
	else
	{
		--GattlingDmageDelay;
	}
}

bool TechnoExtData::UpdateKillSelf_Slave()
{
	auto const pThis = this->AttachedToObject;

	if (VTable::Get(pThis) != InfantryClass::vtable)
		return false;

	const auto pInf = static_cast<InfantryClass*>(pThis);

	if (!pInf->Type->Slaved)
		return false;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(Type);

	if (!pInf->SlaveOwner && (pTypeExt->Death_WithMaster.Get()
		|| pTypeExt->Slaved_ReturnTo == SlaveReturnTo::Suicide))
	{

		const KillMethod nMethod = pTypeExt->Death_Method.Get();

		if (nMethod != KillMethod::None)
			TechnoExtData::KillSelf(pInf, nMethod, pTypeExt->AutoDeath_VanishAnimation);
	}

	return !pThis->IsAlive;
}

// Compares two weapons and returns index of which one is eligible to fire against current target (0 = first, 1 = second), or -1 if neither works.
int TechnoExtData::PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno,
 AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback, bool allowAAFallback)
{
	CellClass* pTargetCell = nullptr;

	// Ignore target cell for airborne target technos.
	if (!pTargetTechno || !pTargetTechno->IsInAir())
	{
		if (auto const pCell = abstract_cast<CellClass*>(pTarget))
			pTargetCell = pCell;
		else if (auto const pObject = abstract_cast<ObjectClass*>(pTarget)) {
			if (!pObject->IsAlive)
				pTargetCell = MapClass::Instance->TryGetCellAt(pObject->Location);
			else
				pTargetCell = pObject->GetCell();
		}
	}

	const auto pWeaponStructOne = pThis->GetWeapon(weaponIndexOne);
	const auto pWeaponStructTwo = pThis->GetWeapon(weaponIndexTwo);

	if (!pWeaponStructOne && !pWeaponStructTwo)
		return -1;
	else if (!pWeaponStructTwo)
		return weaponIndexOne;
	else if (!pWeaponStructOne)
		return weaponIndexTwo;

	auto const pWeaponOne = pWeaponStructOne->WeaponType;
	auto const pWeaponTwo = pWeaponStructTwo->WeaponType;

	if (auto const pSecondExt = WeaponTypeExtContainer::Instance.TryFind(pWeaponTwo))
	{
		if ((pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pSecondExt->CanTarget, true , true)) ||
			(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondExt->CanTarget) ||
				!EnumFunctions::CanTargetHouse(pSecondExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
		{
			return weaponIndexOne;
		}
		else if (auto const pFirstExt = WeaponTypeExtContainer::Instance.TryFind(pWeaponOne))
		{
			bool secondaryIsAA = pTargetTechno && pTargetTechno->IsInAir() && pWeaponTwo->Projectile->AA;

			if (!allowFallback && (!allowAAFallback || !secondaryIsAA) && !TechnoExtData::CanFireNoAmmoWeapon(pThis, 1))
				return weaponIndexOne;

			if ((pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pFirstExt->CanTarget, true , true)) ||
				(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pFirstExt->CanTarget) ||
					!EnumFunctions::CanTargetHouse(pFirstExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
			{
				return weaponIndexTwo;
			}
		}
	}

	auto const pType = pThis->GetTechnoType();

	// Handle special case with NavalTargeting / LandTargeting.
	if (!pTargetTechno && pTargetCell && (pType->NavalTargeting == NavalTargetingType::Naval_primary || pType->LandTargeting == LandTargetingType::Land_secondary)
		&& pTargetCell->LandType != LandType::Water && pTargetCell->LandType != LandType::Beach)
	{
		return weaponIndexTwo;
	}

	return -1;
}

bool TechnoExtData::IsInWarfactory(TechnoClass* pThis, bool bCheckNaval)
{
	if (pThis->WhatAmI() != UnitClass::AbsID || pThis->IsInAir() || !pThis->IsTethered)
		return false;

	auto const pContact = pThis->GetNthLink();

	if (!pContact)
		return false;

	auto const pCell = pThis->GetCell();

	if (!pCell)
		return false;

	auto const pBld = pCell->GetBuilding();

	if (!pBld || pBld != pContact)
		return false;

	if (pBld->Type->WeaponsFactory || (bCheckNaval && pBld->Type->Naval))
	{
		return true;
	}

	return false;
}

CoordStruct TechnoExtData::GetPutLocation(CoordStruct current, int distance)
{
	// this whole thing does not at all account for cells which are completely occupied.
	const CellStruct tmpCoords = CellSpread::AdjacentCell[ScenarioClass::Instance->Random.RandomFromMax(7)];

	current.X += tmpCoords.X * distance;
	current.Y += tmpCoords.Y * distance;

	const auto tmpCell = MapClass::Instance->TryGetCellAt(current);

	if (!tmpCell)
	{
		return CoordStruct::Empty;
	}

	return tmpCell->FindInfantrySubposition(current, false, false, false, current.Z);
}

bool TechnoExtData::EjectSurvivor(FootClass* Survivor, CoordStruct loc, bool Select, std::optional<bool> InAir)
{
	const CellClass* pCell = MapClass::Instance->TryGetCellAt(loc);

	if (const auto pBld = pCell->GetBuilding())
	{
		const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

		if (!pTypeExt->IsPassable)
			return false;

		if (pTypeExt->Firestorm_Wall &&
			pBld->Owner &&
			pBld->Owner->FirestormActive)
			return false;
	}

	Survivor->OnBridge = pCell->ContainsBridge();
	const int floorZ = pCell->GetCoordsWithBridge().Z;

	bool Chuted = false;
	if(!InAir.has_value()){
		Chuted = (loc.Z - floorZ > 2 * Unsorted::LevelHeight);
	}
	else { Chuted = InAir.value(); }

	if (Chuted)
	{
		// HouseClass::CreateParadrop does this when building passengers for a paradrop... it might be a wise thing to mimic!
		Survivor->Limbo();

		if (!Survivor->SpawnParachuted(loc) || pCell->GetBuilding())
		{
			return false;
		}
	}
	else
	{
		loc.Z = floorZ;
		if (!Survivor->Unlimbo(loc, ScenarioClass::Instance->Random.RandomRangedSpecific<DirType>(DirType::North, DirType::NorthWest)))
		{
			return false;
		}
	}

	Survivor->Transporter = nullptr;
	Survivor->LastMapCoords = pCell->MapCoords;

	// don't ask, don't tell
	if (Chuted)
	{
		const bool scat = Survivor->OnBridge;
		const auto occupation = scat ? pCell->AltOccupationFlags : pCell->OccupationFlags;

		if ((occupation & 0x1C) == 0x1C)
		{
			pCell->ScatterContent(CoordStruct::Empty, true, true, scat);
		}
	}
	else
	{
		Survivor->Scatter(CoordStruct::Empty, true, false);
		Survivor->QueueMission(!Survivor->Owner->IsControlledByHuman() ? Mission::Hunt : Mission::Guard, 0);
	}

	Survivor->ShouldEnterOccupiable = false;
	Survivor->ShouldGarrisonStructure = false;

	if (Select)
	{
		Survivor->Select();
	}

	return true;
}

void TechnoExtData::EjectPassengers(FootClass* pThis, int howMany)
{
	if (howMany < 0)
	{
		howMany = pThis->Passengers.NumPassengers;
	}

	auto const openTopped = pThis->GetTechnoType()->OpenTopped;

	for (int i = 0; i < howMany && pThis->Passengers.FirstPassenger; ++i)
	{
		FootClass* passenger = pThis->RemoveFirstPassenger();
		if (!EjectRandomly(passenger, pThis->Location, 128, false))
		{
			passenger->RegisterDestruction(nullptr);
			passenger->UnInit();
		}
		else if (openTopped)
		{
			pThis->ExitedOpenTopped(passenger);
		}
	}
}

bool TechnoExtData::EjectRandomly(FootClass* pEjectee, CoordStruct const& location, int distance, bool select, std::optional<bool> InAir)
{
	CoordStruct destLoc = GetPutLocation(location, distance);

	if (destLoc == CoordStruct::Empty || !MapClass::Instance->IsWithinUsableArea(destLoc))
		return false;

	return EjectSurvivor(pEjectee, destLoc, select , InAir);
}

bool TechnoExtData::ReplaceArmor(REGISTERS* R, TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	auto const pTargetTechnoExt = TechnoExtContainer::Instance.Find(pTarget);

	if (!pTargetTechnoExt)
		return false;

	auto const pShieldData = pTargetTechnoExt->Shield.get();

	if (!pShieldData)
		return false;

	if (pShieldData->IsActive())
	{
		if (pShieldData->CanBePenetrated(pWeapon->Warhead))
			return false;

		R->EAX(pShieldData->GetType()->Armor.Get());
		return true;
	}

	return false;
}

int TechnoExtData::GetInitialStrength(TechnoTypeClass* pType, int nHP)
{
	return TechnoTypeExtContainer::Instance.Find(pType)->InitialStrength.Get(nHP);
}

// Feature for common usage : TechnoType conversion -- Trsdy
// BTW, who said it was merely a Type pointer replacement and he could make a better one than Ares?
bool TechnoExtData::ConvertToType(FootClass* pThis, TechnoTypeClass* pToType)
{
	// In case not using Ares 3.0. Only update necessary vanilla properties
	AbstractType rtti;
	TechnoTypeClass** nowTypePtr;

	// Different types prohibited
	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry:
		nowTypePtr = reinterpret_cast<TechnoTypeClass**>(&(static_cast<InfantryClass*>(pThis)->Type));
		rtti = AbstractType::InfantryType;
		break;
	case AbstractType::Unit:
		nowTypePtr = reinterpret_cast<TechnoTypeClass**>(&(static_cast<UnitClass*>(pThis)->Type));
		rtti = AbstractType::UnitType;
		break;
	case AbstractType::Aircraft:
		nowTypePtr = reinterpret_cast<TechnoTypeClass**>(&(static_cast<AircraftClass*>(pThis)->Type));
		rtti = AbstractType::AircraftType;
		break;
	default:
		Debug::Log("%s is not FootClass, conversion not allowed\n", pToType->get_ID());
		return false;
	}

	if (pToType->WhatAmI() != rtti)
	{
		Debug::Log("Incompatible types between %s and %s\n", pThis->get_ID(), pToType->get_ID());
		return false;
	}

	// Detach CLEG targeting
	auto tempUsing = pThis->TemporalImUsing;
	if (tempUsing && tempUsing->Target)
		tempUsing->Detach();

	HouseClass* const pOwner = pThis->Owner;

	// Remove tracking of old techno
	if (!pThis->InLimbo)
		pOwner->RegisterLoss(pThis, false);
	pOwner->RemoveTracking(pThis);

	int oldHealth = pThis->Health;

	// Generic type-conversion
	TechnoTypeClass* prevType = *nowTypePtr;
	*nowTypePtr = pToType;

	// Readjust health according to percentage
	pThis->AdjustStrength((double)(oldHealth) / (double)prevType->Strength);
	pThis->EstimatedHealth = pThis->Health;

	// Add tracking of new techno
	pOwner->AddTracking(pThis);
	if (!pThis->InLimbo)
		pOwner->RegisterGain(pThis, false);
	pOwner->RecheckTechTree = true;

	// Update Ares AttachEffects -- skipped
	// Ares RecalculateStats -- skipped

	// Adjust ammo
	pThis->Ammo = MinImpl(pThis->Ammo, pToType->Ammo);
	// Ares ResetSpotlights -- skipped

	// Adjust ROT
	if (rtti == AbstractType::AircraftType)
		pThis->SecondaryFacing.Set_ROT(pToType->ROT);
	else
		pThis->PrimaryFacing.Set_ROT(pToType->ROT);
	// Adjust Ares TurretROT -- skipped
	//  pThis->SecondaryFacing.SetROT(TechnoTypeExtContainer::Instance.Find(pToType)->TurretROT.Get(pToType->ROT));

	// Locomotor change, referenced from Ares 0.A's abduction code, not sure if correct, untested
	CLSID nowLocoID;
	ILocomotion* iloco = pThis->Locomotor.GetInterfacePtr();
	const auto& toLoco = pToType->Locomotor;
	if ((SUCCEEDED(static_cast<LocomotionClass*>(iloco)->GetClassID(&nowLocoID)) && nowLocoID != toLoco))
	{
		// because we are throwing away the locomotor in a split second, piggybacking
		// has to be stopped. otherwise the object might remain in a weird state.
		while (LocomotionClass::End_Piggyback(pThis->Locomotor));
		// throw away the current locomotor and instantiate
		// a new one of the default type for this unit.
		if (auto newLoco = LocomotionClass::CreateInstance(toLoco))
		{
			newLoco->Link_To_Object(pThis);
			pThis->Locomotor = std::move(newLoco);
		}
	}

	// TODO : Jumpjet locomotor special treatement, some brainfart, must be uncorrect, HELP ME!
	const auto& jjLoco = CLSIDs::Jumpjet();
	if (pToType->BalloonHover && pToType->DeployToLand && prevType->Locomotor != jjLoco && toLoco == jjLoco)
		pThis->Locomotor->Move_To(pThis->Location);

	return true;
}

bool TechnoExtData::IsEligibleSize(TechnoClass* pThis, TechnoClass* pPassanger)
{
	auto pThisType = pThis->GetTechnoType();
	auto const pThisTypeExt = TechnoTypeExtContainer::Instance.Find(pThisType);
	auto pThatType = pPassanger->GetTechnoType();

	if (pThatType->Size > pThisType->SizeLimit)
		return false;

	if (pThisTypeExt->Passengers_BySize.Get())
	{
		if (pThatType->Size > (pThisType->Passengers - pThis->Passengers.GetTotalSize()))
			return false;
	}
	else if (pThis->Passengers.NumPassengers >= pThisType->Passengers)
	{
		return false;
	}

	return true;
}

bool TechnoExtData::IsTypeImmune(TechnoClass* pThis, TechnoClass* pSource)
{
	if (!pThis || !pSource)
		return false;

	auto const pType = pThis->GetTechnoType();

	if (!pType->TypeImmune)
		return false;

	return pType == pSource->GetTechnoType() && pThis->Owner == pSource->Owner;
}

// =============================
// load / save

template <typename T>
void TechnoExtData::Serialize(T& Stm)
{
	//Debug::Log("Processing Element From TechnoExt ! \n");

	Stm
		.Process(this->Initialized)
		.Process(this->Type)
		.Process(this->AbsType)
		.Process(this->AE_ROF)
		.Process(this->AE_FirePowerMult)
		.Process(this->AE_ArmorMult)
		.Process(this->AE_SpeedMult)
		.Process(this->AE_Cloak)
		.Process(this->idxSlot_Wave)
		.Process(this->idxSlot_Beam)
		.Process(this->idxSlot_Warp)
		.Process(this->idxSlot_Parasite)
		.Process(this->GarrisonedIn)
		.Process(this->EMPSparkleAnim)
		.Process(this->EMPLastMission)
		.Process(this->PoweredUnit)
		.Process(this->RadarJammer)
		.Process(this->BuildingLight)
		.Process(this->OriginalHouseType)
		.Process(this->CloakSkipTimer)
		.Process(this->HijackerHealth)
		.Process(this->HijackerOwner)
		.Process(this->HijackerVeterancy)
		.Process(this->Is_SurvivorsDone)
		.Process(this->Is_DriverKilled)
		.Process(this->Is_Operated)
		.Process(this->Is_UnitLostMuted)
		.Process(this->TakeVehicleMode)
		.Process(this->TechnoValueAmount)
		.Process(this->Shield)
		.Process(this->LaserTrails)
		.Process(this->ReceiveDamage)
		.Process(this->LastKillWasTeamTarget)
		.Process(this->PassengerDeletionTimer)
		.Process(this->CurrentShieldType)
		.Process(this->LastWarpDistance)
		.Process(this->Death_Countdown)
		.Process(this->MindControlRingAnimType)
		.Process(this->DamageNumberOffset)
		.Process(this->CurrentLaserWeaponIndex)
		.Process(this->OriginalPassengerOwner)
		.Process(this->IsInTunnel)
		.Process(this->DeployFireTimer)
		.Process(this->DisableWeaponTimer)
		.Process(this->RevengeWeapons)
		.Process(this->GattlingDmageDelay)
		.Process(this->GattlingDmageSound)
		.Process(this->AircraftOpentoppedInitEd)
		.Process(this->EngineerCaptureDelay)
		.Process(this->FlhChanged)
		.Process(this->IsMissisleSpawn)
		.Process(this->ReceiveDamageMultiplier)
		.Process(this->SkipLowDamageCheck)
		.Process(this->aircraftPutOffsetFlag)
		.Process(this->aircraftPutOffset)
		.Process(this->SkipVoice)
		.Process(this->ExtraWeaponTimers)
		.Process(this->Trails)
		.Process(this->MyGiftBox)
		.Process(this->PaintBallState)
		.Process(this->DamageSelfState)
		.Process(this->CurrentWeaponIdx)
		.Process(this->MyFighterData)
		.Process(this->WarpedOutDelay)
		.Process(this->AltOccupation)
		.Process(this->MyOriginalTemporal)
		.Process(this->SupressEVALost)
		.Process(this->SelfHealing_CombatDelay)
		.Process(this->PayloadCreated)
		.Process(this->LinkedSW)
		.Process(this->SuperTarget)
		.Process(this->HijackerLastDisguiseType)
		.Process(this->HijackerLastDisguiseHouse)
		.Process(this->Convert_Deploy_Delay)
		.Process(this->WHAnimRemainingCreationInterval)
		.Process(this->IsWebbed)
		.Process(this->WebbedAnim)
		.Process(this->WebbyLastTarget)
		.Process(this->WebbyLastMission)
		.Process(this->FreeUnitDone)
		.Process(this->AeData)
		;
	//should put this inside techo ext , ffs

	this->MyWeaponManager.Serialize(Stm);
	this->MyDriveData.Serialize(Stm);
	this->MyDiveData.Serialize(Stm);
	//this->MyJJData.Serialize(Stm);
	this->MySpawnSuport.Serialize(Stm);
}

bool TechnoExtData::InvalidateIgnorable(AbstractClass* ptr)
{
	switch (ptr->WhatAmI())
	{
	case AnimClass::AbsID:
	case HouseClass::AbsID:
	case BuildingClass::AbsID:
	case AircraftClass::AbsID:
	case UnitClass::AbsID:
	case InfantryClass::AbsID:
	case TemporalClass::AbsID:
	case SuperClass::AbsID:
		return false;
	}

	return true;
}

void TechnoExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	if (auto& pShield = this->Shield)
		pShield->InvalidatePointer(ptr, bRemoved);

	MyWeaponManager.InvalidatePointer(ptr, bRemoved);

	AnnounceInvalidPointer(LinkedSW, ptr);
	AnnounceInvalidPointer(OriginalPassengerOwner, ptr);
	AnnounceInvalidPointer(GarrisonedIn, ptr , bRemoved);
	AnnounceInvalidPointer(WebbyLastTarget, ptr);
	AnnounceInvalidPointer(BuildingLight, ptr);
}

TechnoExtContainer TechnoExtContainer::Instance;

// =============================
// container hooks

DEFINE_HOOK(0x6F3260, TechnoClass_CTOR, 0x5)
{
	GET(TechnoClass*, pItem, ESI);
	TechnoExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x6F4500, TechnoClass_DTOR, 0x5)
{
	GET(TechnoClass*, pItem, ECX);
	TechnoExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x70C250, TechnoClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x70BF50, TechnoClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TechnoClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	TechnoExtContainer::Instance.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x70C249, TechnoClass_Load_Suffix, 0x5)
{
	TechnoExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x70C264, TechnoClass_Save_Suffix, 0x5)
{
	TechnoExtContainer::Instance.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x70783B, TechnoClass_Detach, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, target, EBP);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	TechnoExtContainer::Instance.InvalidatePointerFor(pThis, target, all);

	return pThis->BeingManipulatedBy == target ? 0x707843 : 0x707849;
}

DEFINE_OVERRIDE_HOOK(0x710415, TechnoClass_AnimPointerExpired_add, 6)
{
	GET(AnimClass*, pAnim, EAX);
	GET(TechnoClass*, pThis, ECX);

	if (auto pExt = TechnoExtContainer::Instance.Find(pThis))
	{
		if (pExt->EMPSparkleAnim.get() == pAnim)
			pExt->EMPSparkleAnim.release();

		if (auto& pShield = pExt->Shield)
			pShield->InvalidatePointer(pAnim, false);

		if (pExt->WebbedAnim == pAnim)
			pExt->WebbedAnim.release();

		pExt->AeData.InvalidatePointer(pAnim, pThis);
	}

	return 0x0;
}