#include "Header.h"

#include <WWKeyboardClass.h>

#include <AbstractClass.h>
#include <TechnoClass.h>
#include <TeamClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <HouseClass.h>
#include <SlaveManagerClass.h>
#include <SpawnManagerClass.h>
#include <TerrainTypeClass.h>
#include <Notifications.h>

#include <Base/Always.h>
#include <Locomotor/HoverLocomotionClass.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/Cast.h>

#include <Helpers/Macro.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/SWType/NewSuperWeaponType/NuclearMissile.h>
#include <Ext/TEvent/Body.h>
#include <Ext/TAction/Body.h>
#include <Ext/Bomb/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/Scenario/Body.h>

#include <New/Type/ArmorTypeClass.h>
#include <New/Type/GenericPrerequisite.h>

#include <New/Entity/FlyingStrings.h>

#include <Misc/PhobosGlobal.h>

#include <Misc/Ares/EVAVoices.h>

#include <strsafe.h>

#include "AresNetEvent.h"
#include "Classes/AresPoweredUnit.h"
#include "Classes/AresJammer.h"

#include "AresChecksummer.h"

#include <versionhelpers.h>

#include <TriggerTypeClass.h>
#include <TagTypeClass.h>

#include <New/PhobosAttachedAffect/Functions.h>

#include <Utilities/GameConfig.h>

bool StaticVars::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Process(ObjectLinkedAlphas)
		.Success();
}

bool StaticVars::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Process(ObjectLinkedAlphas)
		.Success();
}

void StaticVars::Clear()
{
	ObjectLinkedAlphas.clear();
	ShpCompression1Buffer.clear();
	TriggerCounts.clear();
}

void StaticVars::LoadGlobalsConfig()
{
	GameConfig ares_ini { "Ares.ini" };
	ares_ini.OpenINIAction([](CCINIClass* pINI) {
		if (pINI->ReadString("Graphics.Advanced", "DirectX.Force", Phobos::readDefval, Phobos::readBuffer)) {
			if (IS_SAME_STR_(Phobos::readBuffer, "hardware")) {
				AresGlobalData::GFX_DX_Force = 0x01l; //HW
			} else if (IS_SAME_STR_(Phobos::readBuffer, "emulation")) {
				AresGlobalData::GFX_DX_Force = 0x02l; //EM
			}
		}
	});

	if (IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0))
	{
		AresGlobalData::GFX_DX_Force = 0;
	}
}

static constexpr std::array<std::pair<const char*, const char*>, 17u> const SubName =
{ {
	{"NormalTurretWeapon" , "NormalTurretIndex"},
	{"RepairTurretWeapon" , "RepairTurretIndex"} ,
	{"MachineGunTurretWeapon",	"MachineGunTurretIndex"},
	{"FlakTurretWeapon" , "FlakTurretIndex"} ,
	{"PistolTurretWeapon" , "PistolTurretIndex"} ,
	{"SniperTurretWeapon" ,"SniperTurretIndex"} ,
	{"ShockTurretWeapon", "ShockTurretIndex"},
	{"ExplodeTurretWeapon" , "ExplodeTurretIndex"} ,
	{"BrainBlastTurretWeapon" , "BrainBlastTurretIndex"} ,
	{"RadCannonTurretWeapon" , "RadCannonTurretIndex"} ,
	{"ChronoTurretWeapon" , "ChronoTurretIndex" },
	{"TerroristExplodeTurretWeapon" , "TerroristExplodeTurretIndex"} ,
	{"CowTurretWeapon" , "CowTurretIndex" },
	{"InitiateTurretWeapon" , "InitiateTurretIndex"} ,
	{"VirusTurretWeapon" ,	"VirusTurretIndex" },
	{"YuriPrimeTurretWeapon" ,	"YuriPrimeTurretIndex"} ,
	{"GuardianTurretWeapon"	, "GuardianTurretIndex"}
 } };


HashData HashData::GetINIChecksums()
{
	HashData nBuffer;
	if (SessionClass::Instance->GameMode != GameMode::LAN)
	{
		nBuffer = { CCINIClass::RulesHash() , CCINIClass::ArtHash() ,  CCINIClass::AIHash() };
	}
	else
	{
		nBuffer = { CCINIClass::RulesHash_Internet() , CCINIClass::ArtHash_Internet() ,  CCINIClass::AIHash_Internet() };
	}

	if (!nBuffer.Rules)
		nBuffer.Rules = ScenarioClass::GetRulesUniqueID();

	if (!nBuffer.Art)
		nBuffer.Art = ScenarioClass::GetArtUniqueID();

	if (!nBuffer.AI)
		nBuffer.AI = ScenarioClass::GetAIUniqueID();

	return nBuffer;
}

void OwnFunc::ApplyHitAnim(ObjectClass* pTarget, args_ReceiveDamage* args)
{
	if (Unsorted::CurrentFrame % 15)
		return;

	auto const pWarheadExt = WarheadTypeExtContainer::Instance.Find(args->WH);
	auto const pTechno = generic_cast<TechnoClass*>(pTarget);
	auto const pType = pTarget->GetType();
	auto const bIgnoreDefense = args->IgnoreDefenses;
	bool bImmune_pt2 = false;
	bool const bImmune_pt1 =
		(!pWarheadExt->CanAffectInvulnerable(pTechno) && !bIgnoreDefense) ||
		(pType->Immune && !bIgnoreDefense) || pTarget->InLimbo
		;

	if (pTechno)
	{
		const auto pShield = TechnoExtContainer::Instance.Find(pTechno)->GetShield();
		bImmune_pt2 = (pShield && pShield->IsActive())
			|| pTechno->TemporalTargetingMe
			|| pTechno->BeingWarpedOut
			|| pTechno->IsSinking
			;

	}

	if (!bImmune_pt1 && !bImmune_pt2)
	{
		const int nArmor = (int)TechnoExtData::GetArmor(pTarget);

#ifdef COMPILE_PORTED_DP_FEATURES_
		TechnoClass_ReceiveDamage2_DamageText(pTechno, pDamage, pWarheadExt->DamageTextPerArmor[(int)nArmor]);
#endif

		if (const auto pAnimTypeDecided = pWarheadExt->GetArmorHitAnim(nArmor))
		{
			CoordStruct nBuffer { 0, 0 , 0 };

			if (pTechno)
			{
				auto const pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType());

				if (!pTechnoTypeExt->HitCoordOffset.empty())
				{
					if ((pTechnoTypeExt->HitCoordOffset.size() > 1) && pTechnoTypeExt->HitCoordOffset_Random.Get())
						nBuffer = pTechnoTypeExt->HitCoordOffset[ScenarioClass::Instance->Random.RandomFromMax(pTechnoTypeExt->HitCoordOffset.size() - 1)];
					else
						nBuffer = pTechnoTypeExt->HitCoordOffset[0];
				}
			}

			auto const nCoord = pTarget->GetCenterCoords() + nBuffer;
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimTypeDecided, nCoord),
				args->Attacker ? args->Attacker->GetOwningHouse() : args->SourceHouse, pTarget->GetOwningHouse(),
				args->Attacker,
				false
			);
		}
	}
}

#pragma region TechnoTypeExt_ExtData

BSurface* TechnoTypeExt_ExtData::GetPCXSurface(TechnoTypeClass* pType, HouseClass* pHouse)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	const auto eliteCameo = TechnoTypeExt_ExtData::CameoIsElite(pType, pHouse);

	return eliteCameo ? pTypeExt->AltCameoPCX.GetSurface() : pTypeExt->CameoPCX.GetSurface();
}

bool TechnoTypeExt_ExtData::CarryallCanLift(AircraftTypeClass* pCarryAll, UnitClass* Target)
{
	if (Target->ParasiteEatingMe)
	{
		return false;
	}

	const auto CarryAllData = TechnoTypeExtContainer::Instance.Find(pCarryAll);
	const auto TargetData = TechnoTypeExtContainer::Instance.Find(Target->Type);

	UnitTypeClass* pTargetType = Target->Type;
	const bool passengerEligible = !pTargetType->Organic && !pTargetType->NonVehicle;

	if (!TargetData->CarryallAllowed.Get(passengerEligible))
		return false;

	const auto& nSize = CarryAllData->CarryallSizeLimit;

	if (nSize.isset() && nSize.Get() > 0)
	{
		return nSize.Get() >= ((TechnoTypeClass*)Target->Type)->Size;
	}

	return true;
}

bool TechnoTypeExt_ExtData::CameoIsElite(TechnoTypeClass* pType, HouseClass* pHouse)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if ((!pType->AltCameo && !pTypeExt->AltCameoPCX.GetSurface()) || !pHouse)
		return false;

	const auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);
	const auto pCountry = pHouse->Type;
	switch (pType->WhatAmI())
	{
	case AbstractType::InfantryType:
	{
		if (pHouse->BarracksInfiltrated && !pType->Naval && pType->Trainable)
		{
			return true;
		}

		return pCountry->VeteranInfantry.Contains(static_cast<InfantryTypeClass*>(pType));

	}
	case AbstractType::UnitType:
	{
		if (pHouse->WarFactoryInfiltrated && !pType->Naval && pType->Trainable)
		{
			return true;
		}
		else if (pHouseExt->Is_NavalYardSpied && pType->Naval && pType->Trainable)
		{
			return true;
		}

		return pCountry->VeteranUnits.Contains((UnitTypeClass*)pType);
	}
	case AbstractType::AircraftType:
	{
		if (pHouseExt->Is_AirfieldSpied && pType->Trainable)
		{
			return true;
		}

		return pCountry->VeteranAircraft.Contains((AircraftTypeClass*)(pType));
	}
	case AbstractType::BuildingType:
	{
		if (auto const pItem = pType->UndeploysInto)
		{
			return pCountry->VeteranUnits.Contains((UnitTypeClass*)(pItem));
		}
		else if (pHouseExt->Is_ConstructionYardSpied && pType->Trainable)
		{
			return true;
		}

		return HouseTypeExtContainer::Instance.Find(pCountry)->VeteranBuildings.Contains((BuildingTypeClass*)(pType));
	}
	}

	return false;
}

void TechnoTypeExt_ExtData::LoadTurrets(TechnoTypeClass* pType, CCINIClass* pINI)
{
	INI_EX iniEx(pINI);

	const auto pSection = pType->ID;
	const int weaponCount = pType->WeaponCount >= 0 ? pType->WeaponCount : 0;
	const int addamount = weaponCount - TechnoTypeClass::MaxWeapons < 0 ? 0 : weaponCount - TechnoTypeClass::MaxWeapons;

	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	pExt->AdditionalTurrentWeapon.resize(addamount, -1);
	pExt->WeaponUINameX.resize(weaponCount);
	pExt->Insignia_Weapon.resize(weaponCount);

	//char buffer[0x100u];
	//read default
	for (size_t i = 0; i < SubName.size(); ++i)
	{
		Valueable<int> read_buff { -1 };
		read_buff.Read(iniEx, pSection, SubName[i].first);

		if (read_buff >= 0)
		{

			Valueable<int> read_buff_ { int(i < 4u ? i : 0u) };
			read_buff_.Read(iniEx, pSection, SubName[i].second);

			if (read_buff_ >= 0)
			{
				*(read_buff < 18 ? (pType->TurretWeapon + read_buff) :
				(pExt->AdditionalTurrentWeapon.data() + (read_buff - TechnoTypeClass::MaxWeapons))) = read_buff_;
			}
		}
	}

	CSFText* CSF_ = pExt->WeaponUINameX.data();
	InsigniaData* Data_ = pExt->Insignia_Weapon.data();

	for (size_t i = 0;
		i < (size_t)weaponCount;

		++i,
		++CSF_,
		++Data_
	)
	{
		std::string _number = std::to_string(i + 1);
		int read_buff;
		int* result = i < 18 ?
			pType->TurretWeapon + i :
			pExt->AdditionalTurrentWeapon.data() + (i - TechnoTypeClass::MaxWeapons);

		if (detail::read(read_buff, iniEx, pSection, (std::string("WeaponTurretIndex") + _number).c_str()) && read_buff >= 0)
		{
			*result = read_buff;
		}

		if (*result < 0 || (pType->TurretCount > 0 && *result >= pType->TurretCount))
		{
			Debug::Log("Weapon %d on [%s] has an invalid turret index of %d.\n", i + 1, pSection, *result);
			//*result = 0; //avoid crash
		}

		if (iniEx.ReadString(pSection, (std::string("WeaponUIName") + _number).c_str()) > 0)
			*CSF_ = iniEx.c_str();
		(*Data_).Shapes.Read(iniEx, pSection, (std::string("Insignia.Weapon") + _number + ".%s").c_str());
		(*Data_).Frame.Read(iniEx, pSection, (std::string("InsigniaFrame.Weapon") + _number + ".%s").c_str());
		(*Data_).Frames.Read(iniEx, pSection, (std::string("InsigniaFrames.Weapon") + _number).c_str());
	}
}

int* TechnoTypeExt_ExtData::GetTurretWeaponIndex(TechnoTypeClass* pType, size_t idx)
{
	if (idx < TechnoTypeClass::MaxWeapons)
	{
		return pType->TurretWeapon + idx;
	}

	const int resultidx = (idx - TechnoTypeClass::MaxWeapons);
	const auto& vec = &TechnoTypeExtContainer::Instance.Find(pType)->AdditionalTurrentWeapon;

	if ((size_t)resultidx < vec->size())
		return vec->data() + resultidx;

	Debug::Log("Techno[%s] Trying to get AdditionalTurretWeaponIndex with out of bound index[%d]\n", pType->ID, idx);
	return nullptr;
}

WeaponStruct* TechnoTypeExt_ExtData::GetWeapon(TechnoTypeClass* pType, int const idx, bool elite)
{
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);
	const auto Vectors = &(elite ? pExt->AdditionalEliteWeaponDatas : pExt->AdditionalWeaponDatas);

	if ((size_t)idx < Vectors->size())
		return Vectors->data() + idx;

	Debug::Log("Techno[%s] Trying to get AdditionalWeapon with out of bound index[%d]\n", pType->ID, idx);
	return nullptr;
}

void TechnoTypeExt_ExtData::ReadWeaponStructDatas(TechnoTypeClass* pType, CCINIClass* pRules)
{
	INI_EX iniEx(pRules);
	INI_EX iniEX_art(CCINIClass::INI_Art());

	const auto pSection = pType->ID;
	const auto pSection_art = pType->ImageFile;
	const int additionalamount = pType->WeaponCount - TechnoTypeClass::MaxWeapons < 0 ? 0 : pType->WeaponCount - TechnoTypeClass::MaxWeapons;
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	//Debug::Log("Resize Additional Weapon [%s] [%d]\n", pSection, additionalamount);

	pExt->AdditionalWeaponDatas.resize(additionalamount);
	pExt->AdditionalEliteWeaponDatas.resize(additionalamount);
	pExt->WeaponUINameX.resize(pType->WeaponCount);
	pExt->AdditionalTurrentWeapon.resize(additionalamount);
	pExt->Insignia_Weapon.resize(pType->WeaponCount);

	//Debug::Log("After Resize Additional Weapon [%s] [%d- E %d]\n", pSection, pExt->AdditionalWeaponDatas.size() , pExt->AdditionalEliteWeaponDatas.size());

	for (int i = 0; i < pType->WeaponCount; ++i)
	{
		const int NextIdx = i < TechnoTypeClass::MaxWeapons ? i : i - TechnoTypeClass::MaxWeapons;
		//Debug::Log("Next Weapon Idx for [%s] [%d]\n", pSection, NextIdx);

		//char buffer[0x40];
		//char bufferWeapon[0x40];

		auto data = (i < TechnoTypeClass::MaxWeapons ? pType->Weapon : pExt->AdditionalWeaponDatas.data()) + NextIdx;
		auto data_e = (i < TechnoTypeClass::MaxWeapons ? pType->EliteWeapon : pExt->AdditionalEliteWeaponDatas.data()) + NextIdx;

		std::string _bufferWeapon = std::string("EliteWeapon") + std::to_string(i + 1);
		detail::read(data->WeaponType, iniEx, pSection, _bufferWeapon.c_str() + 5, true);

		if (!detail::read(data_e->WeaponType, iniEx, pSection, _bufferWeapon.c_str(), true))
			data_e->WeaponType = data->WeaponType;

		detail::read(data->FLH, iniEX_art, pSection_art, (_bufferWeapon + "FLH").data() + 5, false);

		if (!detail::read(data_e->FLH, iniEX_art, pSection_art, (_bufferWeapon + "FLH").c_str(), false))
			data_e->FLH = data->FLH;

		detail::read(data->BarrelLength, iniEX_art, pSection_art, (_bufferWeapon + "BarrelLength").data() + 5, false);

		if (!detail::read(data_e->BarrelLength, iniEX_art, pSection_art, (_bufferWeapon + "BarrelLength").c_str()))
			data_e->BarrelLength = data->BarrelLength;

		detail::read(data->BarrelThickness, iniEX_art, pSection_art, (_bufferWeapon + "BarrelThickness").data() + 5, false);

		if (!detail::read(data_e->BarrelThickness, iniEX_art, pSection_art, (_bufferWeapon + "BarrelThickness").c_str()))
			data_e->BarrelThickness = data->BarrelThickness;

		detail::read(data->TurretLocked, iniEX_art, pSection_art, (_bufferWeapon + "TurretLocked").data() + 5, false);

		if (!detail::read(data_e->TurretLocked, iniEX_art, pSection_art, (_bufferWeapon + "TurretLocked").c_str()))
			data_e->TurretLocked = data->TurretLocked;
	}
}

#pragma endregion

#pragma region TechnoExt_ExtData

//https://bugs.launchpad.net/ares/+bug/1925359
void TechnoExt_ExtData::AddPassengers(BuildingClass* const Grinder, TechnoClass* Vic)
{
	for (auto nPass = Vic->Passengers.GetFirstPassenger();
		nPass;
		nPass = (FootClass*)nPass->NextObject)
	{
		const auto pType = nPass->GetTechnoType();

		if (BuildingExtData::ReverseEngineer(Grinder, Vic))
		{
			if (nPass->Owner && nPass->Owner->ControlledByCurrentPlayer())
			{
				VoxClass::Play(nPass->WhatAmI() == InfantryClass::AbsID ? "EVA_ReverseEngineeredInfantry" : "EVA_ReverseEngineeredVehicle");
				VoxClass::Play(GameStrings::EVA_NewTechAcquired());
			}
		}

		if (const auto FirstTag = Grinder->AttachedTag)
		{
			FirstTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerType, Grinder, CellStruct::Empty, false, nPass);

			if (auto pSecondTag = Grinder->AttachedTag)
			{
				pSecondTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerAnything, Grinder, CellStruct::Empty, false, nullptr);
			}
		}

		AddPassengers(Grinder, nPass);
	}
}

bool TechnoExt_ExtData::IsSabotagable(BuildingClass const* const pThis)
{
	const auto pType = pThis->Type;
	const auto pExt = BuildingTypeExtContainer::Instance.Find(pType);
	const auto civ_occupiable = pType->CanBeOccupied && pType->TechLevel == -1;
	const auto default_sabotabable = pType->CanC4 && !civ_occupiable;

	return pExt->ImmuneToSaboteurs.isset() ? !pExt->ImmuneToSaboteurs : default_sabotabable;
}

bool TechnoExt_ExtData::ApplyC4ToBuilding(InfantryClass* const pThis, BuildingClass* const pBuilding, const bool IsSaboteur)
{
	const auto pInfext = InfantryTypeExtContainer::Instance.Find(pThis->Type);

	if (pBuilding->IsIronCurtained() || pBuilding->IsBeingWarpedOut()
		|| pBuilding->GetCurrentMission() == Mission::Selling
		|| BuildingExtContainer::Instance.Find(pBuilding)->AboutToChronoshift
		)
	{
		pThis->AbortMotion();
		pThis->Uncloak(false);
		const int Rof = pInfext->C4ROF.Get(pThis->GetROF(1));
		pThis->ReloadTimer.Start(Rof);
		if (!IsSaboteur)
		{
			pThis->Scatter(pBuilding->GetCoords(), true, true);
		}
		return false;
	}
	else
		if (pBuilding->IsGoingToBlow)
		{
			const int Rof = pInfext->C4ROF.Get(pThis->GetROF(1));
			pThis->ReloadTimer.Start(Rof);
			if (!IsSaboteur)
			{
				pThis->AbortMotion();
				//need to set target ?
				pThis->SetDestination(nullptr, true);
				pThis->Scatter(pBuilding->GetCoords(), true, true);
			}
			return false;
		}

	// sabotage
	pBuilding->IsGoingToBlow = true;
	pBuilding->C4AppliedBy = pThis;

	const auto pData = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
	const auto delay = pInfext->C4Delay.Get(RulesClass::Instance->C4Delay);

	auto duration = (int)(delay * 900.0);

	// modify good durations only
	if (duration > 0)
	{
		duration = (int)(duration * pData->C4_Modifier);
		if (duration <= 0)
			duration = 1;
	}

	//auto pBldExt = BuildingExtContainer::Instance.Find(pBuilding);
	//if (pInfext->C4Damage.isset())
	//{
	//	pBldExt->C4Damage = pInfext->C4Damage;
	//}
	//
	//pBldExt->C4Warhead = pInfext->C4Warhead.Get(RulesClass::Instance->C4Warhead);
	//pBldExt->C4Owner = pThis->GetOwningHouse();
	pBuilding->Flash(duration / 2);
	pBuilding->GoingToBlowTimer.Start(duration);

	if (!IsSaboteur)
	{
		pThis->SetDestination(nullptr, true);
		pThis->Scatter(pBuilding->GetCoords(), true, true);
	}

	return true;
}

Action TechnoExt_ExtData::GetiInfiltrateActionResult(InfantryClass* pInf, BuildingClass* pBuilding)
{
	auto const pInfType = pInf->Type;
	auto const pBldType = pBuilding->Type;

	if ((pInfType->C4 || pInf->HasAbility(AbilityType::C4)) && pBldType->CanC4)
		return Action::Self_Deploy;

	const bool IsAgent = pInfType->Agent;
	if (IsAgent && pBldType->Spyable)
	{
		auto pBldOwner = pBuilding->GetOwningHouse();
		auto pInfOwner = pInf->GetOwningHouse();

		if (!pBldOwner || (pBldOwner != pInfOwner && !pBldOwner->IsAlliedWith(pInfOwner)))
			return Action::Move;
	}

	auto const bIsSaboteur = TechnoTypeExtContainer::Instance.Find(pInfType)->Saboteur.Get();

	if (bIsSaboteur && IsSabotagable(pBuilding))
		return Action::NoMove;

	return IsAgent || bIsSaboteur || !pBldType->Capturable ? Action::None : Action::Enter;
}

bool TechnoExt_ExtData::IsOperated(TechnoClass* pThis)
{
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pExt->Operators.empty())
	{
		if (pExt->Operator_Any)
			return pThis->Passengers.GetFirstPassenger() != nullptr;

		TechnoExtContainer::Instance.Find(pThis)->Is_Operated = true;
		return true;
	}
	else
	{
		for (NextObject object(pThis->Passengers.GetFirstPassenger()); object; ++object)
		{
			if (pExt->Operators.Contains((TechnoTypeClass*)object->GetType()))
			{
				// takes a specific operator and someone is present AND that someone is the operator, therefore it is operated
				return true;
			}
		}
	}

	return false;
}

bool TechnoExt_ExtData::IsOperatedB(TechnoClass* pThis)
{
	return TechnoExtContainer::Instance.Find(pThis)->Is_Operated || TechnoExt_ExtData::IsOperated(pThis);
}

bool TechnoExt_ExtData::IsPowered(TechnoClass* pThis)
{
	auto pType = pThis->GetTechnoType();

	if (pType && pType->PoweredUnit)
	{
		for (const auto& pBuilding : pThis->Owner->Buildings)
		{
			if (pBuilding->Type->PowersUnit == pType
				&& pBuilding->RegisteredAsPoweredUnitSource
				&& !pBuilding->IsUnderEMP()) // alternatively, HasPower, IsPowerOnline()
			{
				return true;
			}
		}
		// if we reach this, we found no building that currently powers this object
		return false;
	}
	else if (auto& pPower = TechnoExtContainer::Instance.Find(pThis)->PoweredUnit)
	{
		// #617
		return pPower->IsPowered();
	}

	// object doesn't need a particular powering structure, therefore, for the purposes of the game, it IS powered
	return true;
}

void TechnoExt_ExtData::EvalRaidStatus(BuildingClass* pThis)
{
	auto pExt = BuildingExtContainer::Instance.Find(pThis);

	// if the building is still marked as raided, but unoccupied, return it to its previous owner
	if (pExt->OwnerBeforeRaid && !pThis->Occupants.Count)
	{
		// Fix for #838: Only return the building to the previous owner if he hasn't been defeated
		if (!pExt->OwnerBeforeRaid->Defeated)
		{
			pThis->SetOwningHouse(pExt->OwnerBeforeRaid, false);
		}

		pExt->OwnerBeforeRaid = nullptr;
	}
}

//new
bool TechnoExt_ExtData::IsUnitAlive(UnitClass* pUnit)
{
	if (!pUnit->IsAlive)
		return false;

	if (pUnit->InLimbo)
		return false;

	if (pUnit->TemporalTargetingMe)
		return false;

	if (TechnoExtContainer::Instance.Find(pUnit)->Is_DriverKilled)
		return false;

	if (pUnit->BerzerkDurationLeft)
		return false;

	if (pUnit->LocomotorSource)
		return false;

	if (pUnit->DeathFrameCounter > 0)
		return false;

	if (TechnoExtData::IsInWarfactory(pUnit))
		return false;

	return true;
}

//confirmed
void TechnoExt_ExtData::SetSpotlight(TechnoClass* pThis, BuildingLightClass* pSpotlight)
{
	if (pThis->WhatAmI() == BuildingClass::AbsID)
	{
		const auto pBld = (BuildingClass*)pThis;

		if (pBld->Spotlight != pSpotlight)
		{
			GameDelete<true, true>(std::exchange(pBld->Spotlight, pSpotlight));
		}
	}

	if (TechnoExtContainer::Instance.Find(pThis)->BuildingLight != pSpotlight)
	{
		GameDelete<true, true>(std::exchange(TechnoExtContainer::Instance.Find(pThis)->BuildingLight, pSpotlight));
	}
}

//confirmed
bool NOINLINE  TechnoExt_ExtData::CanSelfCloakNow(TechnoClass* pThis)
{
	// cloaked and deactivated units are hard to find otherwise
	if (TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled || pThis->Deactivated)
	{
		return false;
	}

	const auto what = pThis->WhatAmI();
	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (what == BuildingClass::AbsID) {
		if (pExt->CloakPowered && !pThis->IsPowerOnline()) {
			return false;
		}

	} else {
		if (what == InfantryClass::AbsID
				&& pExt->CloakDeployed
				&& !((InfantryClass*)pThis)->IsDeployed())
		{
				return false;
		}
		else if (what == UnitClass::AbsID) {
			if (((UnitClass*)pThis)->DeathFrameCounter > 0)
				return false;
		}
	}

	// allows cloak
	return true;
}

//confirmed
bool NOINLINE TechnoExt_ExtData::IsCloakable(TechnoClass* pThis, bool allowPassive)
{
	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	// object disallowed from cloaking
	if (!pTypeExt->CloakAllowed || pExt->AE.ForceDecloak)
	{
		return false;
	}

	// parachuted units cannot cloak. this makes paradropping
	// units uncloakable like they were in the vanilla game
	if (pThis->Parachute)
	{
		return false;
	}

	// check for active cloak
	if (pThis->IsCloakable() || pThis->HasAbility(AbilityType::Cloak)) {
		if (TechnoExt_ExtData::CanSelfCloakNow(pThis)) {
			return true;
		}
	}

	// if not actively cloakable
	if (allowPassive)
	{
		// cloak generators ignore everything above ground. this
		// fixes hover units not being affected by cloak.
		if (pThis->GetHeight() > RulesExtData::Instance()->
					CloakHeight.Get(RulesClass::Instance->HoverHeight))
		{
			return false;
		}

		// search for cloak generators
		CoordStruct crd = pThis->GetCoords();
		CellClass* pCell = MapClass::Instance->GetCellAt(crd);
		return pCell->CloakGen_InclHouse(pThis->Owner->ArrayIndex);
	}

	return false;
}

//confirmed
bool NOINLINE TechnoExt_ExtData::CloakDisallowed(TechnoClass* pThis, bool allowPassive)
{
	if (TechnoExt_ExtData::IsCloakable(pThis, allowPassive))
	{
		auto pExt = TechnoExtContainer::Instance.Find(pThis);
		return pExt->CloakSkipTimer.InProgress()
			|| pThis->IsUnderEMP()
			|| pThis->IsParalyzed()
			|| pThis->IsBeingWarpedOut()
			|| pThis->IsWarpingIn();
	}

	return true;
}

//confirmed
bool NOINLINE TechnoExt_ExtData::CloakAllowed(TechnoClass* pThis)
{
	if (TechnoExt_ExtData::CloakDisallowed(pThis, true))
	{
		return false;
	}

	if (pThis->CloakState == CloakState::Cloaked)
	{
		return false;
	}

	if (!TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->Cloakable_IgnoreArmTimer
		 && pThis->DiskLaserTimer.InProgress())
	{
		return false;
	}

	if (pThis->CloakDelayTimer.InProgress())
	{
		return false;
	}

	if (pThis->Target && pThis->IsCloseEnoughToAttack(pThis->Target))
	{
		//https://bugs.launchpad.net/ares/+bug/1267287
		const auto pWeaponIdx = pThis->SelectWeapon(pThis->Target);
		const auto pWeapon = pThis->GetWeapon(pWeaponIdx);

		if (pWeapon && pWeapon->WeaponType && pWeapon->WeaponType->DecloakToFire)
			return false;
	}

	if (pThis->WhatAmI() != BuildingClass::AbsID)
	{
		if (pThis->CloakProgress.Value)
			return false;

		if (pThis->LocomotorSource && ((FootClass*)pThis)->IsAttackedByLocomotor)
			return false;
	}

	return true;
}

InfantryTypeClass* TechnoExt_ExtData::GetBuildingCrew(BuildingClass* pThis, int nChance)
{
	// with some luck, and if the building has not been captured, spawn an engineer
	if (!pThis->HasBeenCaptured
		&& nChance > 0
		&& ScenarioClass::Instance->Random.RandomFromMax(99) < nChance)
	{
		return HouseExtData::GetEngineer(pThis->Owner);
	}

	return pThis->TechnoClass::GetCrew();
}

void TechnoExt_ExtData::UpdateFactoryQueues(BuildingClass const* const pBuilding)
{
	if (pBuilding->Type->Factory != AbstractType::None)
	{
		pBuilding->Owner->Update_FactoriesQueues(
			pBuilding->Type->Factory,
			pBuilding->Type->Naval,
			BuildCat::DontCare
		);
	}
}

bool TechnoExt_ExtData::IsBaseNormal(BuildingClass* pBuilding)
{
	if (BuildingExtContainer::Instance.Find(pBuilding)->IsFromSW)
		return true;

	const auto pExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

	if (pExt->AIBaseNormal.isset())
		return pExt->AIBaseNormal;

	if (pBuilding->Type->UndeploysInto && pBuilding->Type->ResourceGatherer || pBuilding->IsStrange())
		return true;

	return false;
}

int TechnoExt_ExtData::GetVictimBountyValue(TechnoClass* pVictim, TechnoClass* pKiller)
{
	int Value = 0;
	const auto pKillerTypeExt = TechnoTypeExtContainer::Instance.Find(pKiller->GetTechnoType());
	const auto pVictimTypeExt = TechnoTypeExtContainer::Instance.Find(pVictim->GetTechnoType());

	switch (pKillerTypeExt->Bounty_Value_Option.Get(RulesExtData::Instance()->Bounty_Value_Option))
	{
	case BountyValueOption::Cost:
		Value = pVictimTypeExt->AttachedToObject->GetCost();
		break;
	case BountyValueOption::Soylent:
		Value = pVictim->GetRefund();
		break;
	case BountyValueOption::ValuePercentOfConst:
		Value = int(pVictimTypeExt->AttachedToObject->GetCost() * pVictimTypeExt->Bounty_Value_PercentOf.Get(pVictim));
		break;
	case BountyValueOption::ValuePercentOfSoylent:
		Value = int(pVictim->GetRefund() * pVictimTypeExt->Bounty_Value_PercentOf.Get(pVictim));
		break;
	default:
		Value = pVictimTypeExt->Bounty_Value.Get(pVictim);
		break;
	}

	if (Value == 0)
		return 0;

	const double nVicMult = pVictimTypeExt->Bounty_Value_mult.Get(pVictim);
	const double nMult = pKillerTypeExt->BountyBonusmult.Get(pKiller);

	return int(Value * nVicMult * nMult);
}

bool TechnoExt_ExtData::KillerAllowedToEarnBounty(TechnoClass* pKiller, TechnoClass* pVictim)
{
	if (!pKiller || !pVictim || !pKiller->Owner || !pVictim->Owner || !TechnoExtData::IsBountyHunter(pKiller))
		return false;

	const auto pHouseTypeExt = HouseTypeExtContainer::Instance.TryFind(pVictim->Owner->Type);

	if (pHouseTypeExt && !pHouseTypeExt->GivesBounty)
		return false;

	const auto pKillerTypeExt = TechnoTypeExtContainer::Instance.Find(pKiller->GetTechnoType());
	const auto pVictimType = pVictim->GetTechnoType();

	if (!pKillerTypeExt->BountyAllow.Eligible(pVictimType))
		return false;

	if (!pKillerTypeExt->BountyDissallow.empty() && pKillerTypeExt->BountyDissallow.Contains(pVictimType))
		return false;

	if (pKiller->Owner->IsAlliedWith(pVictim))
		return false;

	if (pKillerTypeExt->Bounty_IgnoreEnablers || RulesExtData::Instance()->Bounty_Enablers.empty())
		return true;

	for (auto const& pEnablers : RulesExtData::Instance()->Bounty_Enablers)
	{
		if (pKiller->Owner->ActiveBuildingTypes.GetItemCount(pEnablers->ArrayIndex) > 0)
			return true;
	}

	return false;
}

void TechnoExt_ExtData::GiveBounty(TechnoClass* pVictim, TechnoClass* pKiller)
{
	if (!TechnoExt_ExtData::KillerAllowedToEarnBounty(pKiller, pVictim))
		return;

	const auto pKillerTypeExt = TechnoTypeExtContainer::Instance.Find(pKiller->GetTechnoType());
	const int nValueResult = TechnoExt_ExtData::GetVictimBountyValue(pVictim, pKiller);

	if (nValueResult != 0 && pKiller->Owner->AbleToTransactMoney(nValueResult))
	{
		if (pKillerTypeExt->Bounty_Display.Get(RulesExtData::Instance()->Bounty_Display))
		{
			if (pKillerTypeExt->AttachedToObject->MissileSpawn && pKiller->SpawnOwner)
				pKiller = pKiller->SpawnOwner;

			if (pKillerTypeExt->Bounty_ReceiveSound != -1)
				VocClass::PlayAt(pKillerTypeExt->Bounty_ReceiveSound, pKiller->Location);

			pKiller->Owner->TransactMoney(nValueResult);
			TechnoExtContainer::Instance.Find(pKiller)->TechnoValueAmount += nValueResult;
		}
	}
}

AresHijackActionResult TechnoExt_ExtData::GetActionHijack(InfantryClass* pThis, TechnoClass* const pTarget)
{
	if (!pThis || !pTarget || !pThis->IsAlive || !pTarget->IsAlive || pTarget->IsIronCurtained())
		return AresHijackActionResult::None;

	if (pThis->WhatAmI() != InfantryClass::AbsID)
		return AresHijackActionResult::None;

	const auto pType = pThis->Type;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	// this can't steal vehicles
	if (!pType->VehicleThief && !pTypeExt->CanDrive.Get(RulesExtData::Instance()->CanDrive))
	{
		return AresHijackActionResult::None;
	}

	const auto pTargetType = pTarget->GetTechnoType();
	const auto absTarget = pTarget->WhatAmI();
	const auto pTargetUnit = absTarget == UnitClass::AbsID ? static_cast<UnitClass*>(pTarget) : nullptr;

	// bunkered units can't be hijacked.
	if (pTarget->BunkerLinkedItem
		// VehicleThief cannot take `NonVehicle`
		|| (pType->VehicleThief && pTargetUnit && pTargetUnit->Type->NonVehicle))
	{
		return AresHijackActionResult::None;
	}

	//no , this one bit different ?
	const bool IsNotOperated = !TechnoExtContainer::Instance.Find(pThis)->Is_Operated
		&& !TechnoExt_ExtData::IsOperated(pTarget);

	// i'm in a state that forbids capturing
	if (pThis->IsDeployed() || IsNotOperated)
	{
		return AresHijackActionResult::None;
	}

	// target type is not eligible (hijackers can also enter strange buildings)

	if (absTarget != AbstractType::Aircraft
		&& absTarget != AbstractType::Unit
		&& (!pType->VehicleThief || absTarget != AbstractType::Building)
		)
	{
		return AresHijackActionResult::None;
	}

	// target is bad
	if (pTarget->CurrentMission == Mission::Selling
		|| pTarget->IsBeingWarpedOut()
		|| pTargetType->IsTrain
		|| pTargetType->BalloonHover
		|| (absTarget != AbstractType::Unit && !pTarget->IsStrange())
		//|| (absTarget == abs_Unit && ((UnitTypeClass*)pTargetType)->NonVehicle) replaced by Hijacker.Allowed
		|| !pTarget->IsOnFloor())
	{
		return AresHijackActionResult::None;
	}

	// a thief that can't break mind control loses without trying further
	if (pType->VehicleThief && pTarget->IsMindControlled()
		&& !pTypeExt->HijackerBreakMindControl)
	{
		return AresHijackActionResult::None;
	}

	if (pTargetUnit
		&& ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune
		&& RulesClass::Instance->HarvesterUnit.Contains(pTargetUnit->Type))
	{
		return AresHijackActionResult::None;
	}

	//drivers can drive, but only stuff owned by neutrals. if a driver is a vehicle thief
	//also, it can reclaim units even if they are immune to hijacking (see below)
	const auto pHouseTypeExt = HouseTypeExtContainer::Instance.Find(pTarget->Owner->Type);
	const auto specialOwned = pHouseTypeExt->CanBeDriven.Get(pTarget->Owner->Type->MultiplayPassive);
	const auto pTargetTypeExt = TechnoTypeExtContainer::Instance.Find(pTargetType);

	if (specialOwned && pTypeExt->CanDrive.Get(RulesExtData::Instance()->CanDrive) && pTargetTypeExt->CanBeDriven)
	{
		return AresHijackActionResult::Drive;
	}

	// hijacking only affects enemies
	if (pType->VehicleThief)
	{
		// can't steal allied unit (CanDrive and special already handled)
		if (pThis->Owner->IsAlliedWith(pTarget->Owner) || specialOwned)
		{
			return AresHijackActionResult::None;
		}

		if (!pTargetTypeExt->HijackerAllowed)
		{
			return AresHijackActionResult::None;
		}

		// allowed to steal from enemy
		return AresHijackActionResult::Hijack;
	}

	// no hijacking ability
	return AresHijackActionResult::None;
}

bool TechnoExt_ExtData::PerformActionHijack(TechnoClass* pFrom, TechnoClass* const pTarget)
{
	// was the hijacker lost in the process?
	bool ret = false;

	if (const auto pThis = abstract_cast<InfantryClass*>(pFrom))
	{
		const auto pType = pThis->Type;
		const auto pExt = TechnoExtContainer::Instance.Find(pThis);
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		const auto action = TechnoExt_ExtData::GetActionHijack(pThis, pTarget);

		// abort capturing this thing, it looked
		// better from over there...
		if (action == AresHijackActionResult::None)
		{
			pThis->SetDestination(nullptr, true);
			const auto& crd = pTarget->GetCoords();
			pThis->Scatter(crd, true, false);
			return false;
		}

		// prepare for a smooth transition. free the destination from
		// any mind control. #762
		if (pTarget->MindControlledBy)
		{
			pTarget->MindControlledBy->CaptureManager->FreeUnit(pTarget);
		}

		if (pTarget->CaptureManager)
		{
			pTarget->CaptureManager->FreeAll();
		}

		pTarget->MindControlledByAUnit = false;
		if (pTarget->MindControlRingAnim)
		{
			pTarget->MindControlRingAnim->UnInit();
			pTarget->MindControlRingAnim = nullptr;
		}

		bool asPassenger = false;
		const auto pDestTypeExt = TechnoTypeExtContainer::Instance.Find(pTarget->GetTechnoType());

		if (action == AresHijackActionResult::Drive && (!pDestTypeExt->Operators.empty() || pDestTypeExt->Operator_Any))
		{
			asPassenger = true;

			// raise some events in case the driver enters
			// a vehicle that needs an Operator
			if (pTarget->AttachedTag)
			{
				pTarget->AttachedTag->RaiseEvent(TriggerEvent::EnteredBy,
					pThis, CellStruct::Empty, false, nullptr);
			}
		}
		else
		{
			// raise some events in case the hijacker/driver will be
			// swallowed by the vehicle.
			if (pTarget->AttachedTag)
			{
				pTarget->AttachedTag->RaiseEvent(TriggerEvent::DestroyedByAnything,
					pThis, CellStruct::Empty, false, nullptr);
			}

			pTarget->Owner->HasBeenThieved = true;
			if (auto const pTag = pThis->AttachedTag)
			{
				if (pTag->ShouldReplace())
				{
					pTarget->ReplaceTag(pTag);
				}
			}
		}

		// if the hijacker is mind-controlled, free it,
		// too, and attach to the new target. #762
		const auto controller = pThis->MindControlledBy;
		if (controller)
		{
			CaptureExt::FreeUnit(controller->CaptureManager, pThis, true);
		}

		// let's make a steal
		pTarget->SetOwningHouse(pThis->Owner, true);
		pTarget->GotHijacked();
		VocClass::PlayAt(pTypeExt->HijackerEnterSound, pTarget->Location, nullptr);

		// remove the driverless-marker
		TechnoExtContainer::Instance.Find(pTarget)->Is_DriverKilled = 0;

		// save the hijacker's properties
		if (action == AresHijackActionResult::Hijack)
		{
			pTarget->HijackerInfantryType = pType->ArrayIndex;
			TechnoExtContainer::Instance.Find(pTarget)->HijackerOwner = pThis->Owner;
			TechnoExtContainer::Instance.Find(pTarget)->HijackerHealth = pThis->Health;
			TechnoExtContainer::Instance.Find(pTarget)->HijackerVeterancy = pThis->Veterancy.Veterancy;
			TechnoExtData::StoreHijackerLastDisguiseData(pThis, (FootClass*)pTarget);
		}

		// hook up the original mind-controller with the target #762
		if (controller)
		{
			CaptureExt::CaptureUnit(controller->CaptureManager, pThis, true);
		}

		// reboot the slave manager
		if (pTarget->SlaveManager)
		{
			pTarget->SlaveManager->ResumeWork();
		}

		// the hijacker enters and closes the door.
		ret = true;

		// only for the drive action: if the target requires an operator,
		// we add the driver to the passengers list instead of deleting it.
		// this does not check passenger count or size limits.
		if (asPassenger)
		{
			pTarget->AddPassenger(pThis);
			pThis->AbortMotion();
			ret = false;
		}

		pTarget->QueueMission(pThis->Owner->IsControlledByHuman() ? Mission::Guard : Mission::Hunt, true);

		if (auto const pTag = pTarget->AttachedTag)
		{
			pTag->RaiseEvent(TriggerEvent(AresTriggerEvents::VehicleTaken_ByHouse), pTarget, CellStruct::Empty, false, pThis);
		}

		if (pTarget->IsAlive)
		{
			if (auto const pTag2 = pTarget->AttachedTag)
			{
				pTag2->RaiseEvent(TriggerEvent(AresTriggerEvents::VehicleTaken), pTarget, CellStruct::Empty, false, nullptr);
			}
		}
	}

	return ret;
}

bool TechnoExt_ExtData::FindAndTakeVehicle(FootClass* pThis)
{
	const auto pInf = specific_cast<InfantryClass*>(pThis);
	if (!pInf)
		return false;

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pInf->Type);
	if (!pInf->Type->VehicleThief && !pExt->CanDrive.Get(RulesExtData::Instance()->CanDrive))
		return false;

	const auto nDistanceMax = ScenarioClass::Instance->Random.RandomFromMax(128);

	//this one iam not really sure how to implement it
	//it seems Ares one do multiple item comparison before doing hijack ?
	//cant really get right decomp result or maybe just me that not understand ,..
	//these should work fine for now ,..
	auto its = Helpers::Alex::getCellSpreadItems<FootClass>(pThis->Location, (double)pInf->Type->Sight, false, false);
	auto it = std::find_if(its.begin(), its.end(), [&](FootClass* pFoot)
{
	if (pFoot == pThis || pFoot->WhatAmI() != UnitClass::AbsID)
		return false;

	if (GetActionHijack(pInf, pFoot) == AresHijackActionResult::None)
		return false;

	return true;
	});

	if (it != its.end())
	{
		TechnoExtContainer::Instance.Find(pThis)->TakeVehicleMode = true;
		pThis->ShouldGarrisonStructure = true;
		if (pThis->Target != *it || pThis->CurrentMission != Mission::Capture)
		{
			pThis->SetDestination(*it, true);
			pThis->QueueMission(Mission::Capture, true);
			return true;
		}
	}

	TechnoExtContainer::Instance.Find(pThis)->TakeVehicleMode = false;
	pThis->ShouldGarrisonStructure = false;
	return false;
}

Action TechnoExt_ExtData::GetEngineerEnterEnemyBuildingAction(BuildingClass* const pBld)
{
	// only skirmish allows to disable it, so we only check there. for all other
	// modes, it's always on. single player campaigns also use special multi
	// engineer behavior.
	auto const gameMode = SessionClass::Instance->GameMode;

	if (gameMode == GameMode::Skirmish && !GameModeOptionsClass::Instance->MultiEngineer
		|| gameMode == GameMode::Campaign)
	{
		// single player missions are currently hardcoded to "don't do damage".
		return Action::Capture; // TODO: replace this by a new rules tag.
	}

	// damage if multi engineer is enabled and target isn't that low on health.
	// check to always capture tech structures. a structure counts
	// as tech if its initial owner is a multiplayer-passive country.
	auto const pRulesExt = RulesExtData::Instance();

	if (auto pOwner = pBld->InitialOwner)
	{
		if (pOwner->Type->MultiplayPassive && pRulesExt->EngineerAlwaysCaptureTech)
			return Action::Capture;
	}

	if (pBld->GetHealthPercentage() > pRulesExt->AttachedToObject->EngineerCaptureLevel)
	{
		return (pRulesExt->EngineerDamage > 0.0)
			? Action::Damage : Action::NoEnter;
	}

	return Action::Capture;
}

bool TechnoExt_ExtData::CloneBuildingEligible(BuildingClass* pBuilding, bool requirePower)
{
	if (pBuilding->InLimbo ||
		!pBuilding->IsAlive ||
		!pBuilding->IsOnMap ||
		pBuilding->TemporalTargetingMe ||
		pBuilding->IsBeingWarpedOut() ||
		BuildingExtContainer::Instance.Find(pBuilding)->AboutToChronoshift ||
		BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1
	)
	{
		return false;
	}

	if (pBuilding->Type->Powered && requirePower && !pBuilding->IsPowerOnline())
		return false;

	return true;
}

void TechnoExt_ExtData::KickOutClone(BuildingClass* pBuilding, TechnoTypeClass* ProductionType, HouseClass* FactoryOwner)
{
	auto Clone = static_cast<TechnoClass*>(ProductionType->CreateObject(FactoryOwner));

	const auto& nStr = TechnoTypeExtContainer::Instance.Find(pBuilding->Type)->InitialStrength_Cloning;
	if (nStr.isset())
	{
		const auto rStr = GeneralUtils::GetRangedRandomOrSingleValue(nStr);
		const int strength = std::clamp(static_cast<int>(ProductionType->Strength * rStr), 1, ProductionType->Strength);
		Clone->Health = strength;
		Clone->EstimatedHealth = strength;
	}

	if (pBuilding->KickOutUnit(Clone, CellStruct::Empty) != KickOutResult::Succeeded)
	{
		Debug::Log(__FUNCTION__" Called \n");
		TechnoExtData::HandleRemove(Clone, nullptr, false, false);
	}
}

void TechnoExt_ExtData::KickOutClones(BuildingClass* pFactory, TechnoClass* const Production)
{
	const auto FactoryType = pFactory->Type;

	if (FactoryType->Cloning ||
		(FactoryType->Factory != InfantryTypeClass::AbsID &&
			FactoryType->Factory != UnitTypeClass::AbsID)
	)
	{
		return;
	}

	const auto ProductionType = Production->GetTechnoType();
	const auto ProductionTypeData = TechnoTypeExtContainer::Instance.Find(ProductionType);

	if (!ProductionTypeData->Cloneable)
	{
		return;
	}

	const auto isPlayer = pFactory->Owner->IsControlledByHuman();

	auto ProductionTypeAs = ProductionType;
	if (!isPlayer && ProductionTypeData->AI_ClonedAs)
		ProductionTypeAs = ProductionTypeData->AI_ClonedAs;
	else if (ProductionTypeData->ClonedAs)
		ProductionTypeAs = ProductionTypeData->ClonedAs;

	if (!ProductionTypeAs || !ProductionTypeAs->Strength) // ,....
		return;

	auto const FactoryOwner = pFactory->Owner;
	auto const& CloningSources = ProductionTypeData->ClonedAt;
	auto const IsUnit = (FactoryType->Factory != InfantryTypeClass::AbsID);

	// keep cloning vats for backward compat, unless explicit sources are defined
	if (!IsUnit && CloningSources.empty())
	{
		for (auto const& CloningVat : FactoryOwner->CloningVats)
		{
			if (!CloneBuildingEligible(CloningVat, BuildingTypeExtContainer::Instance.Find(CloningVat->Type)->Cloning_RequirePower))
				continue;

			KickOutClone(CloningVat, ProductionTypeAs, FactoryOwner);
		}

		return;
	}

	// and clone from new sources
	if (!CloningSources.empty() || IsUnit)
	{
		for (auto const& CloningVat : FactoryOwner->Buildings)
		{
			if (!CloneBuildingEligible(CloningVat, BuildingTypeExtContainer::Instance.Find(CloningVat->Type)->Cloning_RequirePower))
				continue;

			auto const BType = CloningVat->Type;

			auto ShouldClone = false;
			if (!CloningSources.empty())
			{
				ShouldClone = CloningSources.Contains(CloningVat->Type);
			}
			else if (IsUnit)
			{
				ShouldClone = BuildingTypeExtContainer::Instance.Find(CloningVat->Type)->CloningFacility && (CloningVat->Type->Naval == FactoryType->Naval);
			}

			if (ShouldClone)
			{
				KickOutClone(CloningVat, ProductionTypeAs, FactoryOwner);
			}
		}
	}
}

void TechnoExt_ExtData::InitWeapon(
	TechnoClass* pThis,
	TechnoTypeClass* pType,
	WeaponTypeClass* pWeapon,
	int idxWeapon,
	CaptureManagerClass*& pCapture,
	ParasiteClass*& pParasite,
	TemporalClass*& pTemporal,
	const char* pTagName,
	bool IsFoot
)
{
	constexpr auto const Note = "Constructing an instance of [%s]:\r\n"
		"%s %s (slot %d) has no %s!";

	if (!pWeapon->Projectile)
	{
		Debug::FatalErrorAndExit(
			Note, pType->ID, pTagName, pWeapon->ID, idxWeapon,
			"Projectile");
	}

	auto const pWarhead = pWeapon->Warhead;

	if (!pWarhead)
	{
		Debug::FatalErrorAndExit(
			Note, pType->ID, pTagName, pWeapon->ID, idxWeapon, "Warhead");
	}

	if (pWarhead->MindControl && !pCapture)
	{
		pCapture = GameCreate<CaptureManagerClass>(
			pThis, pWeapon->Damage, pWeapon->InfiniteMindControl);
	}

	if (pWarhead->Temporal && !pTemporal)
	{
		pTemporal = GameCreate<TemporalClass>(pThis);
		pTemporal->WarpPerStep = pWeapon->Damage;
		TechnoExtContainer::Instance.Find(pThis)->idxSlot_Warp = static_cast<BYTE>(idxWeapon);
	}

	if (pWarhead->Parasite && IsFoot && !pParasite)
	{
		pParasite = GameCreate<ParasiteClass>((FootClass*)pThis);
		TechnoExtContainer::Instance.Find(pThis)->idxSlot_Parasite = static_cast<BYTE>(idxWeapon);
	}
}

InfantryClass* TechnoExt_ExtData::RecoverHijacker(FootClass* const pThis)
{
	if (auto const pType = InfantryTypeClass::Array->GetItemOrDefault(
		pThis->HijackerInfantryType))
	{
		const auto pOwner = TechnoExtContainer::Instance.Find(pThis)->HijackerOwner ?
			TechnoExtContainer::Instance.Find(pThis)->HijackerOwner : pThis->Owner;

		pThis->HijackerInfantryType = -1;

		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		if (!pTypeExt->HijackerOneTime && pOwner && !pOwner->Defeated)
		{
			if (auto const pHijacker = static_cast<InfantryClass*>(pType->CreateObject(pOwner)))
			{
				TechnoExtData::RestoreStoreHijackerLastDisguiseData(pHijacker, pThis);
				pHijacker->Health = MaxImpl(TechnoExtContainer::Instance.Find(pThis)->HijackerHealth, 10) / 2;
				pHijacker->Veterancy.Veterancy = TechnoExtContainer::Instance.Find(pThis)->HijackerVeterancy;
				return pHijacker;
			}
		}
	}

	return nullptr;
}

void TechnoExt_ExtData::SpawnSurvivors(FootClass* const pThis, TechnoClass* const pKiller, const bool Select, const bool IgnoreDefenses, const bool PreventPassengersEscape)
{
	auto const pType = pThis->GetTechnoType();
	auto const pOwner = pThis->Owner;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	// do not ever do this again for this unit
	if (!TechnoExtContainer::Instance.Find(pThis)->Is_SurvivorsDone)
	{
		TechnoExtContainer::Instance.Find(pThis)->Is_SurvivorsDone = true;
	}
	else
	{
		return;
	}

	// always eject passengers, but passengers only if not supressed.
	if (!TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled && !IgnoreDefenses)
	{
		// save this, because the hijacker can kill people
		auto pilotCount = pThis->GetCrewCount();

		// process the hijacker
		if (auto const pHijacker = RecoverHijacker(pThis))
		{
			auto const pHijackerTypeExt = TechnoTypeExtContainer::Instance.Find(pHijacker->Type);

			if (!TechnoExtData::EjectRandomly(pHijacker, pThis->Location, 144, Select))
			{
				pHijacker->RegisterDestruction(pKiller);
				Debug::Log(__FUNCTION__" Hijacker Called \n");
				TechnoExtData::HandleRemove(pHijacker, pKiller, false, true);
			}
			else
			{
				// the hijacker will now be controlled instead of the unit
				if (auto const pController = pThis->MindControlledBy)
				{
					CaptureExt::FreeUnit(pController->CaptureManager, pThis, true);
					CaptureExt::CaptureUnit(pController->CaptureManager, pHijacker, true);
					pHijacker->QueueMission(Mission::Guard, true); // override the fate the AI decided upon
				}

				VocClass::PlayAt(pHijackerTypeExt->HijackerLeaveSound, pThis->Location, nullptr);

				// lower than 0: kill all, otherwise, kill n pilots
				pilotCount = ((pHijackerTypeExt->HijackerKillPilots < 0) ? 0 :
					(pilotCount - pHijackerTypeExt->HijackerKillPilots));
			}
		}

		// possibly eject up to pilotCount crew members
		if (pilotCount > 0 && pType->Crewed)
		{
			int pilotChance = pTypeExt->Survivors_PilotChance.Get(pThis);
			if (pilotChance < 0)
			{
				pilotChance = static_cast<int>(RulesClass::Instance->CrewEscape * 100);
			}

			if (pilotChance > 0)
			{

				for (int i = 0; i < pilotCount; ++i)
				{
					if (auto pPilotType = pThis->GetCrew())
					{
						if (ScenarioClass::Instance->Random.RandomRanged(1, 100) <= pilotChance)
						{
							auto const pPilot = static_cast<InfantryClass*>(pPilotType->CreateObject(pOwner));
							pPilot->Health /= 2;
							pPilot->Veterancy.Veterancy = pThis->Veterancy.Veterancy;

							if (!TechnoExtData::EjectRandomly(pPilot, pThis->Location, 144, Select))
							{
								pPilot->RegisterDestruction(pKiller);
								Debug::Log(__FUNCTION__" Pilot Called \n");
								TechnoExtData::HandleRemove(pPilot, pKiller, false, true);
							}
							else if (auto const pTag = pThis->AttachedTag)
							{
								if (pTag->ShouldReplace())
								{
									pPilot->ReplaceTag(pTag);
								}
							}
						}
					}
				}
			}
		}
	}

	if (!PreventPassengersEscape)
	{
		// passenger escape chances
		const auto passengerChance = pTypeExt->Survivors_PassengerChance.Get(pThis);

		//quick exit
		if (passengerChance == 0)
			return;

		const auto what = pThis->WhatAmI();

		// eject or kill all passengers
		while (pThis->Passengers.GetFirstPassenger())
		{
			auto const pPassenger = pThis->RemoveFirstPassenger();
			bool trySpawn = false;
			if (passengerChance > 0)
			{
				trySpawn = ScenarioClass::Instance->Random.RandomRanged(1, 100) <= passengerChance;
			}
			else if (passengerChance == -1 && what == UnitClass::AbsID)
			{
				const Move occupation = pPassenger->IsCellOccupied(pThis->GetCell(), FacingType::None, -1, nullptr, true);
				trySpawn = (occupation == Move::OK || occupation == Move::MovingBlock);
			}

			if (trySpawn && TechnoExtData::EjectRandomly(pPassenger, pThis->Location, 128, Select))
			{
				continue;
			}

			// kill passenger, if not spawned
			pPassenger->RegisterDestruction(pKiller);
			Debug::Log(__FUNCTION__" Passengers Called \n");
			TechnoExtData::HandleRemove(pPassenger, pKiller, false, false);
		}
	}
}

int TechnoExt_ExtData::GetWarpPerStep(TemporalClass* pThis, int nStep)
{
	int nAddStep = 0;
	int nStepR = 0;

	if (!pThis)
		return 0;

	for (TemporalClass* pTemp = pThis; pTemp; pTemp = pTemp->PrevTemporal)
	{
		if (nStep > 50)
			break;

		++nStep;
		auto const pWeapon = pTemp->Owner->GetWeapon(TechnoExtContainer::Instance.Find(pTemp->Owner)->idxSlot_Warp)->WeaponType;

		//if (auto const pTarget = pTemp->Target)
		//	nStepR = MapClass::GetTotalDamage(pWeapon->Damage, pWeapon->Warhead, pTarget->GetTechnoType()->Armor, 0);
		//else
		nStepR = pWeapon->Damage;

		nAddStep += nStepR;
		pTemp->WarpPerStep = nStepR;
	}

	return nAddStep;
}

bool TechnoExt_ExtData::Warpable(TechnoClass* pTarget)
{
	if (!pTarget || pTarget->IsSinking || pTarget->IsCrashing || pTarget->IsIronCurtained())
		return false;

	if (TechnoExtData::IsUnwarpable(pTarget))
		return false;

	if (pTarget->WhatAmI() == BuildingClass::AbsID)
	{
		if (BuildingExtContainer::Instance.Find((BuildingClass*)pTarget)->AboutToChronoshift)
		{
			return false;
		}
	}
	else
	{
		if (TechnoExtData::IsInWarfactory(pTarget, true))
			return false;

		if (TechnoExtData::IsChronoDelayDamageImmune(static_cast<FootClass*>(pTarget)))
			return false;
	}

	return true;
}

void TechnoExt_ExtData::DepositTiberium(TechnoClass* pThis, HouseClass* pHouse, float const amount, float const bonus, int const idxType)
{
	auto pTiberium = TiberiumClass::Array->Items[idxType];
	auto value = 0;

	// always put the purified money on the bank account. otherwise ore purifiers
	// would fill up storage with tiberium that doesn't exist. this is consistent with
	// the original YR, because old GiveTiberium put it on the bank anyhow, despite its name.
	if (bonus > 0.0)
	{
		value += int(bonus * pTiberium->Value * pHouse->Type->IncomeMult);
	}

	// also add the normal tiberium to the global account?
	if (amount > 0.0)
	{
		auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
		if (!pExt->Refinery_UseStorage)
		{
			value += int(amount * pTiberium->Value * pHouse->Type->IncomeMult);
		}
		else
		{
			int decidedIndex = idxType;
			float decidedAmount = amount;
			if (pThis->WhatAmI() == BuildingClass::AbsID && RulesExtData::Instance()->Storage_TiberiumIndex >= 0)
			{
				pTiberium = TiberiumClass::Array->Items[RulesExtData::Instance()->Storage_TiberiumIndex];
				decidedIndex = RulesExtData::Instance()->Storage_TiberiumIndex;
				decidedAmount = (amount * pTiberium->Value) / pTiberium->Value;
			}

			pHouse->GiveTiberium(decidedAmount, decidedIndex);
		}
	}

	// deposit
	if (value > 0)
	{
		pHouse->GiveMoney(value);
	}
}

void TechnoExt_ExtData::RefineTiberium(TechnoClass* pThis, HouseClass* pHouse, float const amount, int const idxType)
{
	const auto refined = BuildingTypeExtData::GetPurifierBonusses(pHouse) * amount;
	// add the tiberium to the house's credits
	TechnoExt_ExtData::DepositTiberium(pThis, pHouse, amount, refined, idxType);
}

bool TechnoExt_ExtData::FiringAllowed(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	const auto nRulesGreen = RulesClass::Instance->ConditionGreen;
	const auto pThatTechnoExt = TechnoExtContainer::Instance.Find(pTarget);
	const auto pThatShield = pThatTechnoExt->GetShield();

	if (pThatShield && pThatShield->IsActive())
	{
		if (!pThatShield->CanBePenetrated(pWeapon->Warhead))
		{
			if (pThatShield->GetType()->CanBeHealed)
			{
				const bool IsFullHP = pThatShield->GetHealthRatio() >= nRulesGreen;
				if (!IsFullHP)
				{
					return true;
				}
				else
				{
					if (pThatShield->GetType()->PassthruNegativeDamage)
						return !(pTarget->GetHealthPercentage_() >= nRulesGreen);
				}
			}

			return false;
		}
	}

	return !(pTarget->GetHealthPercentage_() >= nRulesGreen);
}

UnitTypeClass* TechnoExt_ExtData::GetUnitTypeImage(UnitClass* const pThis)
{
	const auto pData = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	if (pData->WaterImage && !pThis->OnBridge && pThis->GetCell()->LandType == LandType::Water && !pThis->IsAttackedByLocomotor)
	{
		return pData->WaterImage;
	}

	return nullptr;
}

TechnoTypeClass* TechnoExt_ExtData::GetImage(FootClass* pThis)
{
	if (const auto pUnit = specific_cast<UnitClass*>(pThis))
	{
		TechnoTypeClass* Image = pUnit->Type;

		if (UnitTypeClass* const pCustomType = TechnoExt_ExtData::GetUnitTypeImage(pUnit))
		{
			Image = pCustomType;
		}

		if (pUnit->Deployed && pUnit->Type->UnloadingClass)
		{
			Image = pUnit->Type->UnloadingClass;
		}

		if (!pUnit->IsClearlyVisibleTo(HouseClass::CurrentPlayer))
		{
			if (auto pDisUnit = specific_cast<UnitTypeClass*>(pUnit->GetDisguise(true)))
			{
				Image = pDisUnit;
			}
		}

		return Image;
	}

	return pThis->GetTechnoType();
}

void TechnoExt_ExtData::HandleTunnelLocoStuffs(FootClass* pOwner, bool DugIN, bool PlayAnim)
{
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pOwner->GetTechnoType());
	const auto pRules = RulesClass::Instance();
	const auto nSound = (DugIN ? pExt->DigInSound : pExt->DigOutSound).Get(pRules->DigSound);

	VocClass::PlayIndexAtPos(nSound, pOwner->Location);

	if (PlayAnim)
	{
		if (const auto pAnimType = (DugIN ? pExt->DigInAnim : pExt->DigOutAnim).Get(pRules->Dig))
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pOwner->Location),
				pOwner->Owner,
				nullptr,
				false
			);
		}
	}
}


bool TechnoExt_ExtData::IsSameTrech(BuildingClass* currentBuilding, BuildingClass* targetBuilding)
{
	auto pThisTypeExt = BuildingTypeExtContainer::Instance.Find(currentBuilding->Type);
	if (pThisTypeExt->IsTrench <= 0)
	{
		return false;
	}

	return pThisTypeExt->IsTrench == BuildingTypeExtContainer::Instance.Find(targetBuilding->Type)->IsTrench;
}

bool TechnoExt_ExtData::canTraverseTo(BuildingClass* currentBuilding, BuildingClass* targetBuilding)
{
	if (targetBuilding != currentBuilding)
	{
		BuildingTypeClass* pTargetType = targetBuilding->Type;
		if (pTargetType->CanBeOccupied && targetBuilding->Occupants.Count < pTargetType->MaxNumberOccupants)
		{
			if (currentBuilding->Occupants.Count && IsSameTrech(currentBuilding, targetBuilding))
			{
				if (targetBuilding->Location.DistanceFrom(currentBuilding->Location) <= 256.0)
					return true;
			}
		}
	}

	return false;
}

void TechnoExt_ExtData::doTraverseTo(BuildingClass* currentBuilding, BuildingClass* targetBuilding)
{
	BuildingTypeClass* targetBuildingType = targetBuilding->Type;

	// depending on Westwood's handling, this could explode when Size > 1 units are involved...but don't tell the users that
	while (currentBuilding->Occupants.Count && (targetBuilding->Occupants.Count < targetBuildingType->MaxNumberOccupants))
	{
		auto item = currentBuilding->Occupants.Items[0];
		targetBuilding->Occupants.AddItem(item);
		TechnoExtContainer::Instance.Find(item)->GarrisonedIn = targetBuilding;
		currentBuilding->Occupants.Remove<true>(item); // maybe switch Add/Remove if the game gets pissy about multiple of them walking around
	}

	// fix up firing index, as decrementing the source occupants can invalidate it
	if (currentBuilding->FiringOccupantIndex >= currentBuilding->GetOccupantCount())
	{
		currentBuilding->FiringOccupantIndex = 0;
	}

	//const auto oldtgt = currentBuilding->Target;
	//currentBuilding->SetTarget(nullptr);
	//targetBuilding->SetTarget(oldtgt);
	TechnoExt_ExtData::EvalRaidStatus(currentBuilding); // if the traversal emptied the current building, it'll have to be returned to its owner
}

bool TechnoExt_ExtData::AcquireHunterSeekerTarget(TechnoClass* pThis)
{

	if (!pThis->Target)
	{
		std::vector<TechnoClass*> preferredTargets;
		std::vector<TechnoClass*> randomTargets;

		// defaults if SW isn't set
		auto pOwner = pThis->GetOwningHouse();
		SWTypeExtData* pSWExt = nullptr;
		auto canPrefer = true;

		// check the hunter seeker SW
		if (auto const pSuper =
			TechnoExtContainer::Instance.Find(pThis)->LinkedSW
			)
		{
			pOwner = pSuper->Owner;
			pSWExt = SWTypeExtContainer::Instance.Find(pSuper->Type);
			canPrefer = !pSWExt->HunterSeeker_RandomOnly;
		}

		auto const isHumanControlled = pOwner->IsControlledByHuman();
		auto const mode = SessionClass::Instance->GameMode;

		// the AI in multiplayer games only attacks its favourite enemy
		auto const pFavouriteEnemy = HouseClass::Array->GetItemOrDefault(pOwner->EnemyHouseIndex);
		auto const favouriteEnemyOnly = (mode != GameMode::Campaign
			&& pFavouriteEnemy && !isHumanControlled);

		for (auto i : *TechnoClass::Array)
		{
			// techno ineligible
			if (i->Health < 0 || i->InLimbo || !i->IsAlive || i->IsCrashing || i->IsSinking)
			{
				continue;
			}

			if (!MapClass::Instance->IsWithinUsableArea(i->GetCoords()))
				continue;

			if (!i->Location.IsValid() || !i->InlineMapCoords().IsValid())
				continue;

			if (i->IsIronCurtained())
				continue;

			const auto what = i->WhatAmI();

			if (what == BuildingClass::AbsID)
			{
				auto pBuilding = static_cast<BuildingClass*>(i);
				const auto pExt = BuildingExtContainer::Instance.Find(pBuilding);

				if (pExt->LimboID >= 0 || pBuilding->Type->InvisibleInGame)
					continue;
			}
			else
			{
				if (what == UnitClass::AbsID)
				{
					if (((UnitClass*)i)->DeathFrameCounter > 0)
						continue;

				}
			}

			// type prevents this being a target
			auto const pType = i->GetTechnoType();

			// is type to be ignored?
			auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);

			if (pType->Invisible
			|| (pExt->AI_LegalTarget.isset() && !isHumanControlled && !pExt->AI_LegalTarget.Get())
			|| !pType->LegalTarget
			|| pExt->HunterSeekerIgnore
			)
			{
				continue;
			}

			// is the house ok?
			if (favouriteEnemyOnly)
			{
				if (i->Owner != pFavouriteEnemy)
				{
					continue;
				}
			}
			else if (!pSWExt && pOwner->IsAlliedWith(i->Owner))
			{
				// default without SW
				continue;
			}
			else if (pSWExt && !pSWExt->IsHouseAffected(pOwner, i->Owner))
			{
				// use SW
				continue;
			}

			// harvester truce
			if (ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune
				&& what == UnitClass::AbsID)
			{
				if (RulesClass::Instance->HarvesterUnit.Contains(((UnitTypeClass*)pType)))
				{
					continue;
				}
			}

			// allow to exclude certain techno types
			if (pSWExt && !pSWExt->IsTechnoAffected(i))
			{
				continue;
			}

			// in multiplayer games, non-civilian targets are preferred
			// for human players
			auto const isPreferred = mode != GameMode::Campaign && isHumanControlled
				&& !i->Owner->Type->MultiplayPassive && canPrefer;

			// add to the right list
			if (isPreferred)
			{
				preferredTargets.push_back(i);
			}
			else
			{
				randomTargets.push_back(i);
			}
		}

		auto const targets = &(preferredTargets.size() > 0 ? preferredTargets : randomTargets);

		if (auto const count = targets->size())
		{
			// that's our target
			pThis->SetTarget
			(*(targets->data() + (size_t(count == 1 ?
				0 : ScenarioClass::Instance->Random.RandomFromMax(count - 1)))
				));
			return true;
		}
	}

	return false;
}

void TechnoExt_ExtData::UpdateAlphaShape(ObjectClass* pSource)
{
	if (!pSource || !pSource->IsAlive)
		return;

	ObjectTypeClass* pSourceType = pSource->GetType();

	if (!pSourceType)
	{
		return;
	}

	const SHPStruct* pImage = pSourceType->AlphaImage;
	if (!pImage)
	{
		return;
	}

	const auto what = pSource->WhatAmI();
	ObjectClass* pOwner = pSource;

	if (what == AnimClass::AbsID)
	{
		const auto pAnim = (AnimClass*)pSource;
		if (pAnim->OwnerObject)
		{
			pOwner = pAnim->OwnerObject;
		}
	}

	Point2D off { (pImage->Width + 1) / -2, (pImage->Height + 1) / -2 };

	if (pOwner && (pOwner->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
	{
		const auto pFoot = (FootClass*)pOwner;

		if (pFoot->CurrentMapCoords != pFoot->LastMapCoords)
		{

			CoordStruct XYZ = CellClass::Cell2Coord(pFoot->LastMapCoords);
			Point2D xyTL = TacticalClass::Instance->CoordsToClient(XYZ);

			TacticalClass::Instance->RegisterDirtyArea({
				off.X - 30 + xyTL.X ,
				xyTL.Y - 60 + off.Y ,
				pImage->Width + 60 ,
				pImage->Width + 120
			}, true);
		}
	}

	CoordStruct XYZ = pSource->GetCoords();
	Point2D xyTL = TacticalClass::Instance->CoordsToClient(XYZ);

	ObjectTypeClass* pDisguise = nullptr;

	if (pSource->InLimbo || ((pSource->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
		&& (((TechnoClass*)pSource)->Deactivated
			|| ((TechnoClass*)pSource)->CloakState == CloakState::Cloaked
			|| pSource->GetHeight() < -10
			|| pSource->IsDisguised() && (pDisguise = pSource->GetDisguise(true)) && pDisguise->WhatAmI() == AbstractType::TerrainType
			|| what == BuildingClass::AbsID && (pSource->GetCurrentMission() != Mission::Construction && !((BuildingClass*)pSource)->IsPowerOnline()
				|| BuildingExtContainer::Instance.Find(((BuildingClass*)pSource))->LimboID != -1)
			)
	)
	{
		if (auto pAlpha = StaticVars::ObjectLinkedAlphas.get_or_default(pSource))
			GameDelete<true, false>(std::exchange(pAlpha, nullptr));

		return;
	}

	if (Unsorted::CurrentFrame % 2)
	{
		if (StaticVars::ObjectLinkedAlphas.get_or_default(pSource)
			&& what == BuildingClass::AbsID
			&& (pImage->Frames <= 1 || !((BuildingClass*)pSource)->HasTurret() || !((BuildingClass*)pSource)->TurretIsRotating)
			)
			return;

		RectangleStruct ScreenArea = TacticalClass::Instance->VisibleArea();
		++Unsorted::ScenarioInit;
		GameCreate<AlphaShapeClass>(pSource,
		(xyTL.X + off.X + ScreenArea.X),
		(xyTL.Y + off.Y + ScreenArea.Y));
		--Unsorted::ScenarioInit;
		TacticalClass::Instance->RegisterDirtyArea({
		xyTL.X + off.X,
		xyTL.Y + off.Y,
		pImage->Width,
		pImage->Height },
		true);
	}
}

int TechnoExt_ExtData::GetAmmo(TechnoClass* const pThis, WeaponTypeClass* pWeapon)
{
	const auto pExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	for (int i = pExt->Ammo; i > 0; --i)
		pThis->DecreaseAmmo();

	return pExt->Ammo;
}

void TechnoExt_ExtData::DecreaseAmmo(TechnoClass* const pThis, WeaponTypeClass* pWeapon)
{
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (GetAmmo(pThis, pWeapon) > 0)
	{
		if (pThis->WhatAmI() != AircraftClass::AbsID)
		{
			if (pTypeExt->NoAmmoWeapon > -1 && pTypeExt->NoAmmoEffectAnim)
			{
				const auto pCurWeapon = pThis->GetWeapon(pTypeExt->NoAmmoWeapon);
				if (pThis->Ammo <= pTypeExt->NoAmmoAmount && pCurWeapon->WeaponType != pWeapon)
				{
					auto pAnim = GameCreate<AnimClass>(pTypeExt->NoAmmoEffectAnim.Get(), pThis->Location);
					pAnim->SetOwnerObject(pThis);
					pAnim->SetHouse(pThis->Owner);

				}
			}
		}

		if (pThis->WhatAmI() == BuildingClass::AbsID)
		{
			const auto Ammo = reinterpret_cast<BuildingClass*>(pThis)->Type->Ammo;
			if (Ammo > 0 && pThis->Ammo < Ammo)
				pThis->StartReloading();
		}
	}
}

AnimClass* TechnoExt_ExtData::SpawnAnim(CoordStruct& crd, AnimTypeClass* pType, int dist)
{
	if (!pType)
	{
		return nullptr;
	}

	CoordStruct crdAnim = crd;

	if (dist > 0)
	{
		const auto crdNear = MapClass::GetRandomCoordsNear(crd, dist, false);
		crdAnim = MapClass::PickInfantrySublocation(crdNear, true);
	}

	const auto count = ScenarioClass::Instance->Random.RandomRanged(1, 2);
	return GameCreate<AnimClass>(pType, crdAnim, 0, count, 0x600u, 0, false);
}

void TechnoExt_ExtData::PlantBomb(TechnoClass* pSource, ObjectClass* pTarget, WeaponTypeClass* pWeapon)
{
	// ensure target isn't rigged already
	if (pTarget && !pTarget->AttachedBomb)
	{
		const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
		const auto pTechno = generic_cast<TechnoClass*>(pTarget);

		//https://bugs.launchpad.net/ares/+bug/1591335
		if (pTechno && !pWHExt->CanDealDamage(pTechno))
			return;

		BombListClass::Instance->Plant(pSource, pTarget);

		// if target has a bomb, planting was successful
		if (auto pBomb = pTarget->AttachedBomb)
		{
			const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
			BombExtContainer::Instance.Find(pBomb)->Weapon = pWeaponExt;
			pBomb->DetonationFrame = Unsorted::CurrentFrame + pWeaponExt->Ivan_Delay.Get(RulesClass::Instance->IvanTimedDelay);
			pBomb->TickSound = pWeaponExt->Ivan_TickingSound.Get(RulesClass::Instance->BombTickingSound);

			const auto IsAlly = pSource->Owner && pSource->Owner->IsAlliedWith(pTarget);

			pBomb->Type = BombType((!IsAlly && pWeaponExt->Ivan_DeathBomb) || (IsAlly && pWeaponExt->Ivan_DeathBombOnAllies));

			if (pSource->Owner && pSource->Owner->ControlledByCurrentPlayer())
			{
				VocClass::PlayIndexAtPos(pWeaponExt->Ivan_AttachSound.Get(RulesClass::Instance->BombAttachSound)
				, pBomb->Target->Location);
			}
		}
	}
}

bool TechnoExt_ExtData::CanDetonate(TechnoClass* pThis, ObjectClass* pThat)
{
	if (pThis == pThat && ObjectClass::CurrentObjects->Count == 1)
	{
		if (const auto pBomb = pThis->AttachedBomb)
		{
			if (!pBomb->OwnerHouse)
				return false;

			if (pBomb->OwnerHouse->ControlledByCurrentPlayer())
			{
				const auto pData = BombExtContainer::Instance.Find(pBomb);
				const bool bCanDetonateDeathBomb =
					pData->Weapon->Ivan_CanDetonateDeathBomb.Get(RulesClass::Instance->CanDetonateDeathBomb);
				const bool bCanDetonateTimeBomb =
					pData->Weapon->Ivan_CanDetonateTimeBomb.Get(RulesClass::Instance->CanDetonateTimeBomb);

				if (pBomb->Type == BombType::DeathBomb ?
					bCanDetonateDeathBomb : bCanDetonateTimeBomb)
					return true;
			}
		}
	}

	return false;
}

Action TechnoExt_ExtData::GetAction(TechnoClass* pThis, ObjectClass* pThat)
{
	if (!pThat)
		return Action::None;

	if (TechnoExt_ExtData::CanDetonate(pThis, pThat))
		return Action::Detonate;

	if (pThis == pThat && ObjectClass::CurrentObjects->Count == 1)
	{
		if (pThat->AbstractFlags & AbstractFlags::Techno)
		{
			if (pThis->Owner && pThis->Owner->IsAlliedWith(pThat) && pThat->IsSelectable())
			{
				return Action::Select;
			}
		}
	}

	return Action::None;
}

int TechnoExt_ExtData::GetFirstSuperWeaponIndex(BuildingClass* pThis)
{
	const auto pExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);
	const auto count = pExt->GetSuperWeaponCount();
	for (auto i = 0; i < count; ++i)
	{
		const auto idxSW = pExt->GetSuperWeaponIndex(i, pThis->Owner);
		if (idxSW != -1)
		{
			return idxSW;
		}
	}
	return -1;
}

void TechnoExt_ExtData::UpdateDisplayTo(BuildingClass* pThis)
{
	if (pThis->Type->Radar)
	{
		auto pHouse = pThis->Owner;
		DWORD presistData = HouseExtContainer::Instance.Find(pHouse)->RadarPersist.data;

		for (auto walk = pHouse->Buildings.begin(); walk != pHouse->Buildings.end(); ++walk) {
			if (!(*walk)->InLimbo) {
				if (BuildingTypeExtContainer::Instance.Find((*walk)->Type)->SpyEffect_RevealRadar) {
					presistData |= (*walk)->DisplayProductionTo.data;
				}
			}
		}

		//TODO RadarVisible
		pHouse->RadarVisibleTo.data = presistData;
		MapClass::Instance->RedrawSidebar(2);
	}
}

void TechnoExt_ExtData::InfiltratedBy(BuildingClass* EnteredBuilding, HouseClass* Enterer)
{
	auto EnteredType = EnteredBuilding->Type;
	auto Owner = EnteredBuilding->Owner;
	auto pTypeExt = BuildingTypeExtContainer::Instance.Find(EnteredBuilding->Type);
	auto pBldExt = BuildingExtContainer::Instance.Find(EnteredBuilding);
	static constexpr reference<bool, 0x884B8E> tootip_something {};

	bool raiseEva = false;
	const bool IsOwnerControlledByCurrentPlayer = Owner->ControlledByCurrentPlayer();
	const bool IsEntererControlledByCurrentPlayer = Enterer->ControlledByCurrentPlayer();

	if (IsEntererControlledByCurrentPlayer || IsOwnerControlledByCurrentPlayer)
	{
		CellStruct xy = CellClass::Coord2Cell(EnteredBuilding->GetCoords());
		if (RadarEventClass::Create(RadarEventType::BuildingInfiltrated, xy))
		{
			raiseEva = true;
		}
	}

	const bool evaForOwner = IsOwnerControlledByCurrentPlayer && raiseEva;
	const bool evaForEnterer = IsEntererControlledByCurrentPlayer && raiseEva;
	auto pEntererExt = HouseExtContainer::Instance.Find(Enterer);
	bool effectApplied = false;
	bool promotionStolen = false;
	int moneyBefore =  Owner->Available_Money();

	if (!pTypeExt->SpyEffect_Custom)
	{
		if (EnteredType->Radar)
		{
			Owner->ReshroudMap();
			if (!Owner->SpySatActive && evaForOwner)
			{
				VoxClass::Play(GameStrings::EVA_RadarSabotaged);
			}
			if (!Owner->SpySatActive && evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfRadarSabotaged);
			}
			effectApplied = true;
		}
		else if (EnteredType->PowerBonus > 0)
		{
			Owner->CreatePowerOutage(RulesClass::Instance->SpyPowerBlackout);
			if (evaForOwner)
			{
				VoxClass::Play(GameStrings::EVA_PowerSabotaged);
			}
			if (evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				VoxClass::Play(GameStrings::EVA_EnemyBasePoweredDown);
			}
			effectApplied = true;
		}
		else if (!RulesClass::Instance->BuildTech.Count || !RulesClass::Instance->BuildTech.Contains(EnteredType))
		{
			if (EnteredType->SuperWeapon != -1)
			{

				if (auto pSuper = Owner->Supers[EnteredType->SuperWeapon])
				{
					pSuper->Reset();
					if (evaForOwner || evaForEnterer)
					{
						VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
					}
					effectApplied = true;
				}
			}
			else if (EnteredType->Storage > 0 && !EnteredType->Weeder)
			{

				int available = Owner->Available_Money();
				float mult = RulesClass::Instance->SpyMoneyStealPercent;
				auto const& nAIMult = RulesExtData::Instance()->AI_SpyMoneyStealPercent;

				if (!Owner->IsControlledByHuman() && nAIMult.isset())
				{
					mult = nAIMult.Get();
				}
				int bounty = int(mult * mult);

				if (bounty > 0)
				{
					bounty = MinImpl(bounty, available);
					Owner->TakeMoney(bounty);
					Enterer->GiveMoney(bounty);
					if (evaForOwner)
					{
						VoxClass::Play(GameStrings::EVA_CashStolen);
					}

					if (evaForEnterer)
					{
						VoxClass::Play(GameStrings::EVA_BuildingInfCashStolen);
					}
					effectApplied = true;
				}
			}
			else if (EnteredType->Factory != AbstractType::None)
			{
				switch (EnteredType->Factory)
				{
				case UnitTypeClass::AbsID:
					Enterer->WarFactoryInfiltrated = true;
					promotionStolen = true;
					break;
				case InfantryTypeClass::AbsID:
					Enterer->BarracksInfiltrated = true;
					promotionStolen = true;
					break;
				default:
					break;
				}
			}
		}
		else
		{
			switch (EnteredType->AIBasePlanningSide)
			{
			case 0:
				Enterer->Side0TechInfiltrated = true;
				promotionStolen = true;
				break;
			case 1:
				Enterer->Side1TechInfiltrated = true;
				promotionStolen = true;
				break;
			case 2:
				Enterer->Side2TechInfiltrated = true;
				promotionStolen = true;
				break;
			default:
				break;
			}
		}

	}
	else
	{
		if (pTypeExt->SpyEffect_ResetRadar)
		{
			Owner->ReshroudMap();
			if (!Owner->SpySatActive && evaForOwner)
			{
				VoxClass::Play(GameStrings::EVA_RadarSabotaged);
			}
			if (!Owner->SpySatActive && evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfRadarSabotaged);
			}
			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_PowerOutageDuration > 0)
		{
			Owner->CreatePowerOutage(pTypeExt->SpyEffect_PowerOutageDuration);
			if (evaForOwner)
			{
				VoxClass::Play(GameStrings::EVA_PowerSabotaged);
			}
			if (evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				VoxClass::Play(GameStrings::EVA_EnemyBasePoweredDown);
			}
			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_StolenTechIndex_result.any())
		{
			HouseExtContainer::Instance.Find(Enterer)->StolenTech |= pTypeExt->SpyEffect_StolenTechIndex_result;
			Enterer->RecheckTechTree = true;
			if (evaForOwner)
			{
				VoxClass::Play(GameStrings::EVA_TechnologyStolen);
			}

			if (evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				VoxClass::Play(GameStrings::EVA_NewTechAcquired);
			}

			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_UnReverseEngineer)
		{
			Debug::Log("Undoing all Reverse Engineering achieved by house %ls\n", Owner->UIName);
			HouseExtContainer::Instance.Find(Owner)->Reversed.clear();
			Owner->RecheckTechTree = true;

			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}

			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_ResetSW)
		{
			bool somethingReset = false;
			for (auto types : EnteredBuilding->GetTypes())
			{
				if (auto typeExt = BuildingTypeExtContainer::Instance.TryFind(types))
				{
					for (auto i = 0; i < typeExt->GetSuperWeaponCount(); ++i)
					{
						if (auto pSuper = typeExt->GetSuperWeaponByIndex(i, Owner))
						{
							pSuper->Reset();
							somethingReset = true;
						}
					}
				}
			}

			if (somethingReset)
			{
				if (evaForOwner || evaForEnterer)
				{
					VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				}
				effectApplied = true;
			}
		}

		// Did you mean for not launching for real or not, Morton?
		auto launchTheSWHere = [EnteredBuilding, pTypeExt](int const idx, HouseClass* const pHouse, bool realLaunch = false)
			{
				if (const auto pSuper = pHouse->Supers.GetItemOrDefault(idx))
				{
					if (!realLaunch || (pSuper->Granted && pSuper->IsCharged && !pSuper->IsOnHold))
					{
						const int oldstart = pSuper->RechargeTimer.StartTime;
						const int oldleft = pSuper->RechargeTimer.TimeLeft;
						pSuper->SetReadiness(true);
						CoordStruct loc = pTypeExt->SpyEffect_SWTargetCenter.Get() ? EnteredBuilding->GetCenterCoords() : EnteredBuilding->Location;
						pSuper->Launch(CellClass::Coord2Cell(loc), pHouse->IsCurrentPlayer());
						pSuper->Reset();
						if (!realLaunch)
						{
							pSuper->RechargeTimer.StartTime = oldstart;
							pSuper->RechargeTimer.TimeLeft = oldleft;
						}
					}
				}
			};

		auto justGrantTheSW = [](int const idx, HouseClass* const pHouse)
			{
				if (const auto pSuper = pHouse->Supers.GetItemOrDefault(idx))
				{
					if (pSuper->Granted)
						pSuper->SetCharge(100);
					else
					{
						pSuper->Grant(true, false, false);
						if (pHouse->IsCurrentPlayer())
							SidebarClass::Instance->AddCameo(AbstractType::Special, idx);
					}
					SidebarClass::Instance->RepaintSidebar(1);
				}
			};

		if (pTypeExt->SpyEffect_VictimSuperWeapon.isset() && !Owner->IsNeutral())
		{
			launchTheSWHere(pTypeExt->SpyEffect_VictimSuperWeapon.Get(), Owner, pTypeExt->SpyEffect_VictimSW_RealLaunch.Get());

			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}

			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_InfiltratorSuperWeapon.isset())
		{
			const int swidx = pTypeExt->SpyEffect_InfiltratorSuperWeapon.Get();

			if (pTypeExt->SpyEffect_InfiltratorSW_JustGrant.Get())
				justGrantTheSW(swidx, Enterer);
			else
				launchTheSWHere(swidx, Enterer);

			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}

			effectApplied = true;
		}

		if (auto pSuperType = pTypeExt->SpyEffect_SuperWeapon)
		{
			const auto nIdx = pSuperType->ArrayIndex;
			const auto pSuper = Enterer->Supers.Items[nIdx];
			const bool Onetime = !pTypeExt->SpyEffect_SuperWeaponPermanent;
			bool CanLauch = true;

			if (!pSuperType->IsPowered || Enterer->PowerDrain == 0 || Enterer->PowerOutput >= Enterer->PowerDrain)
				CanLauch = false;

			const bool IsCurrentPlayer = Enterer->IsCurrentPlayer();

			if (pSuper->Grant(Onetime, IsCurrentPlayer, CanLauch))
			{
				if (pTypeExt->SpyEffect_SuperWeaponPermanent)
					pSuper->CanHold = false;

				if (IsCurrentPlayer)
				{
					SidebarClass::Instance->AddCameo(AbstractType::Special, nIdx);
					const auto nTab = SidebarClass::GetObjectTabIdx(AbstractType::Special, nIdx, false);
					SidebarClass::Instance->RepaintSidebar(nTab);
				}

				if (evaForOwner || evaForEnterer)
				{
					VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				}

				effectApplied = true;
			}
		}

		if (pTypeExt->SpyEffect_SabotageDelay > 0)
		{
			const int nDelay = int(pTypeExt->SpyEffect_SabotageDelay * 900.0);

			if (nDelay >= 0 && !EnteredBuilding->IsGoingToBlow)
			{
				EnteredBuilding->IsGoingToBlow = true;
				EnteredBuilding->GoingToBlowTimer.Start(nDelay);
				EnteredBuilding->Flash(nDelay / 2);

				if (evaForOwner || evaForEnterer)
				{
					VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
				}

				effectApplied = true;
			}
		}

		{
			int bounty = 0;
			int available = Owner->Available_Money();
			if (pTypeExt->SpyEffect_StolenMoneyAmount > 0)
			{
				bounty = pTypeExt->SpyEffect_StolenMoneyAmount;
			}
			else if (pTypeExt->SpyEffect_StolenMoneyPercentage > 0.0f)
			{
				bounty = int(available * pTypeExt->SpyEffect_StolenMoneyPercentage);
			}

			if (bounty > 0)
			{
				bounty = MinImpl(bounty, available);
				Owner->TakeMoney(bounty);
				Enterer->GiveMoney(bounty);
				if (evaForOwner)
				{
					VoxClass::Play(GameStrings::EVA_CashStolen);
				}
				if (evaForEnterer)
				{
					VoxClass::Play(GameStrings::EVA_BuildingInfCashStolen);
				}

				effectApplied = true;
			}
		}

		{
			if (pTypeExt->SpyEffect_GainVeterancy)
			{
				switch (EnteredType->Factory)
				{
				case UnitTypeClass::AbsID:
					if (!EnteredType->Naval)
						Enterer->WarFactoryInfiltrated = true;
					else
						pEntererExt->Is_NavalYardSpied = true;

					promotionStolen = true;
					break;
				case InfantryTypeClass::AbsID:
					Enterer->BarracksInfiltrated = true;
					promotionStolen = true;
					break;
				case AircraftTypeClass::AbsID:
					pEntererExt->Is_AirfieldSpied = true;
					promotionStolen = true;
					break;
				case BuildingTypeClass::AbsID:
					pEntererExt->Is_ConstructionYardSpied = true;
					promotionStolen = true;
					break;
				default:
					break;
				}
			}
			else
			{
				if (pTypeExt->SpyEffect_AircraftVeterancy)
				{
					pEntererExt->Is_AirfieldSpied = true;
					promotionStolen = true;
				}

				if (pTypeExt->SpyEffect_InfantryVeterancy)
				{
					Enterer->BarracksInfiltrated = true;
					promotionStolen = true;
				}

				if (pTypeExt->SpyEffect_NavalVeterancy)
				{
					pEntererExt->Is_NavalYardSpied = true;
					promotionStolen = true;
				}

				if (pTypeExt->SpyEffect_VehicleVeterancy)
				{
					Enterer->WarFactoryInfiltrated = true;
					promotionStolen = true;
				}

				if (pTypeExt->SpyEffect_BuildingVeterancy)
				{
					pEntererExt->Is_ConstructionYardSpied = true;
					promotionStolen = true;
				}
			}
		}

		/*	RA1-Style Spying, as requested in issue #633
			This sets the respective bit to inform the game that a particular house has spied this building.
			Knowing that, the game will reveal the current production in this building to the players who have spied it.
			In practice, this means: If a player who has spied a factory clicks on that factory,
			he will see the cameo of whatever is being built in the factory.

			Addition 04.03.10: People complained about it not being optional. Now it is.
		*/
		if (pTypeExt->SpyEffect_RevealProduction)
		{
			EnteredBuilding->DisplayProductionTo.Add(Enterer);
			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}
			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_RevealRadar)
		{
			/*	Remember the new persisting radar spy effect on the victim house itself, because
				destroying the building would destroy the spy reveal info in the ExtData, too.
				2013-08-12 AlexB
			*/
			if (pTypeExt->SpyEffect_RevealRadarPersist)
			{
				HouseExtContainer::Instance.Find(Owner)->RadarPersist.Add(Enterer);
			}

			EnteredBuilding->DisplayProductionTo.Add(Enterer);
			TechnoExt_ExtData::UpdateDisplayTo(EnteredBuilding);

			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}

			MapClass::Instance->Map_AI();
			MapClass::Instance->RedrawSidebar(2);
			effectApplied = true;
		}

		if (pTypeExt->SpyEffect_SellDelay.isset())
		{

			if (!pBldExt->AutoSellTimer.HasStarted())
			{
				pBldExt->AutoSellTimer.Start(pTypeExt->SpyEffect_SellDelay > 0 ?
					pTypeExt->SpyEffect_SellDelay : static_cast<int>(RulesClass::Instance->C4Delay));
			}

			if (evaForOwner || evaForEnterer)
			{
				VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			}
			effectApplied = true;
		}
	}
	if (pTypeExt->SpyEffect_Anim && pTypeExt->SpyEffect_Anim_Duration > 0)
	{

		pBldExt->SpyEffectAnim.reset(GameCreate<AnimClass>(pTypeExt->SpyEffect_Anim, EnteredBuilding->GetCoords()));
		pBldExt->SpyEffectAnim->SetOwnerObject(EnteredBuilding);
		pBldExt->SpyEffectAnim->RemainingIterations = 0xFFU;
		pBldExt->SpyEffectAnim->Owner = EnteredBuilding->Owner;

		pBldExt->SpyEffectAnimDuration = pTypeExt->SpyEffect_Anim_Duration;
		effectApplied = true;
	}

	if (promotionStolen)
	{
		Enterer->RecheckTechTree = true;
		if (IsEntererControlledByCurrentPlayer)
		{
			MouseClass::Instance->SidebarNeedsRepaint();
			tootip_something = true;
		}

		if (evaForOwner)
		{
			VoxClass::Play(GameStrings::EVA_TechnologyStolen);
		}

		if (evaForEnterer)
		{
			VoxClass::Play(GameStrings::EVA_BuildingInfiltrated);
			VoxClass::Play(GameStrings::EVA_NewTechAcquired);
		}
		effectApplied = true;
	}

	if (effectApplied)
	{
		EnteredBuilding->UpdatePlacement(PlacementType::Redraw);
	}

	pBldExt->AccumulatedIncome += Owner->Available_Money() - moneyBefore;

	if (!Owner->IsControlledByHuman() && !RulesExtData::Instance()->DisplayIncome_AllowAI)
	{
		CoordStruct coord {};
		EnteredBuilding->GetRenderCoords(&coord);
		FlyingStrings::AddMoneyString(true,
				pBldExt->AccumulatedIncome,
				EnteredBuilding,
				pTypeExt->DisplayIncome_Houses.Get(RulesExtData::Instance()->DisplayIncome_Houses.Get()),
				coord,
				pTypeExt->DisplayIncome_Offset,
				ColorStruct::Empty
		);
		pBldExt->AccumulatedIncome = 0;
	}
}

DirStruct TechnoExt_ExtData::UnloadFacing(UnitClass* pThis)
{
	DirStruct nResult;
	nResult.Raw = 0x4000;

	if (pThis->HasAnyLink())
	{
		if (const auto pBld = specific_cast<BuildingClass*>(pThis->RadioLinks.Items[0]))
		{
			auto const pBldExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);
			if (pBldExt->DockUnload_Facing.isset())
				nResult.Raw = ((size_t)pBldExt->DockUnload_Facing.Get()) << 11;
		}
	}

	return nResult;
}

CellStruct TechnoExt_ExtData::UnloadCell(BuildingClass* pThis)
{
	return BuildingTypeExtContainer::Instance.Find(pThis->Type)->DockUnload_Cell;
}

BuildingClass* TechnoExt_ExtData::BuildingUnload(UnitClass* pThis)
{
	if (const auto pBld = specific_cast<BuildingClass*>(pThis->RadioLinks.Items[0]))
	{
		const auto pBldCells = pBld->InlineMapCoords();
		const auto pThisCells = pThis->InlineMapCoords();

		if ((pBldCells + UnloadCell(pBld)) == pThisCells)
		{
			return pBld;
		}
	}

	return nullptr;
}

void TechnoExt_ExtData::KickOutHospitalArmory(BuildingClass* pThis)
{
	if (pThis->Type->Hospital || pThis->Type->Armory)
	{
		if (FootClass* Passenger = pThis->Passengers.RemoveFirstPassenger())
		{
			pThis->KickOutUnit(Passenger, CellStruct::Empty);
		}
	}
}

static DynamicVectorClass<std::pair<FootClass*, bool>, DllAllocator<std::pair<FootClass*, bool>>> KickList;

void TechnoExt_ExtData::KickOutOfRubble(BuildingClass* pBld)
{
	// iterate over all cells and remove all infantry
	// Note : ares 3.0p1 seems doing faster way to do this
	// not sure if that safe way tho -Otamaa
	KickList.Reset();
	auto const location = MapClass::Instance->GetCellAt(pBld->Location)->MapCoords;
	// get the number of non-end-marker cells and a pointer to the cell data
	for (auto i = pBld->Type->FoundationData; *i != CellStruct::EOL; ++i)
	{
		// remove every techno that resides on this cell
		for (NextObject obj(MapClass::Instance->GetCellAt(location + *i)->
			GetContent()); obj; ++obj)
		{
			if (auto const pFoot = abstract_cast<FootClass*>(*obj))
			{
				if (pFoot->Limbo())
				{
					KickList.AddItem({ pFoot, pFoot->IsSelected });
				}
			}
		}
	}

	// this part kicks out all units we found in the rubble
	for (auto const& [pFoot, bIsSelected] : KickList)
	{
		if (pBld->KickOutUnit(pFoot, location) == KickOutResult::Succeeded)
		{
			if (bIsSelected)
			{
				pFoot->Select();
			}
		}
		else
		{
			pFoot->UnInit();
		}
	}
}

void TechnoExt_ExtData::UpdateSensorArray(BuildingClass* pBld)
{
	if (pBld->Type->SensorArray)
	{
		bool isActive = !pBld->Deactivated && pBld->IsPowerOnline();
		bool wasActive = (BuildingExtContainer::Instance.Find(pBld)->SensorArrayActiveCounter > 0);

		if (isActive != wasActive)
		{
			if (isActive)
			{
				pBld->SensorArrayActivate();
			}
			else
			{
				pBld->SensorArrayDeactivate();
			}
		}
	}
}

BuildingClass* TechnoExt_ExtData::CreateBuilding(
	BuildingClass* pBuilding,
	bool remove,
	BuildingTypeClass* pNewType,
	OwnerHouseKind owner,
	int strength,
	AnimTypeClass* pAnimType
)
{
	pBuilding->Limbo(); // only takes it off the map
	pBuilding->DestroyNthAnim(BuildingAnimSlot::All);
	BuildingClass* pRet = nullptr;

	if (!remove)
	{
		HouseClass* designated =
			//pBuilding->Type->CanBeOccupied && pBuilding->Occupants.Count != 0 ? HouseExtData::FindFirstCivilianHouse() :
			pBuilding->Owner;

		auto pOwner = HouseExtData::GetHouseKind(owner, true, designated);
		pRet = static_cast<BuildingClass*>(pNewType->CreateObject(pOwner));

		if (strength <= -1 && strength >= -100)
		{
			// percentage of original health
			pRet->Health = MaxImpl((-strength * pNewType->Strength) / 100, 1);
		}
		else if (strength > 0)
		{
			pRet->Health = MinImpl(strength, pNewType->Strength);
		}

		const auto direction = pBuilding->PrimaryFacing.Current().GetDir();
		++Unsorted::ScenarioInit;
		const bool res = pRet->Unlimbo(pBuilding->Location, direction);
		--Unsorted::ScenarioInit;

		if (!res)
		{
			Debug::Log(__FUNCTION__" Called \n");
			TechnoExtData::HandleRemove(pRet, nullptr, true, false);
			pRet = nullptr;
		}
	}

	if (pAnimType)
	{
		GameCreate<AnimClass>(pAnimType, pBuilding->GetCoords())->Owner = pBuilding->Owner;
	}

	return pRet;
};

void TechnoExt_ExtData::Destroy(TechnoClass* pTechno, TechnoClass* pKiller, HouseClass* pKillerHouse, WarheadTypeClass* pWarhead)
{
	if (!pKillerHouse && pKiller)
	{
		pKillerHouse = pKiller->Owner;
	}

	if (!pWarhead)
	{
		pWarhead = RulesClass::Instance->C4Warhead;
	}

	int health = pTechno->Health;

	if (pTechno->IsAlive && health > 0 && !pTechno->IsSinking && !pTechno->IsCrashing)
		return;

	if (pTechno->TemporalTargetingMe)
	{
		pTechno->Limbo();
		pTechno->Destroyed(pTechno->TemporalImUsing->Owner);
		TechnoExtData::HandleRemove(pTechno, pKiller, false, false);
		return;
	}

	pTechno->ReceiveDamage(&health, 0, pWarhead, pKiller, true, false, pKillerHouse);
}

bool TechnoExt_ExtData::IsDriverKillable(TechnoClass* pThis, double KillBelowPercent)
{
	const auto what = pThis->WhatAmI();
	if (what != UnitClass::AbsID && what != AircraftClass::AbsID)
		return false;

	if (what == AircraftClass::AbsID)
	{
		const auto pAir = (AircraftClass*)pThis;

		if (pAir->Type->AirportBound || pAir->Type->Dock.Count)
			return false;
	}

	if (pThis->BeingWarpedOut || pThis->IsIronCurtained() || TechnoExtData::IsInWarfactory(pThis, false))
		return false;

	const auto pType = pThis->GetTechnoType();

	if (pType->Natural || pType->Organic)
		return false;

	const auto pThisTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	const bool protecteddriver = TechnoExtData::IsDriverKillProtected(pThis);

	const double maxKillHealth = MinImpl(
		pThisTypeExt->ProtectedDriver_MinHealth.Get(
			protecteddriver ? 0.0 : 1.0),
		KillBelowPercent);

	if (pThis->GetHealthPercentage() > maxKillHealth)
		return false;

	return true;
}

void TechnoExt_ExtData::ApplyKillDriver(TechnoClass* pTarget, TechnoClass* pKiller, HouseClass* pToOwner, bool ResetVet, Mission passiveMission)
{
	if (!pTarget || (pTarget->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
		return;

	if (pTarget->Owner == pToOwner)
	{
		return;
	}

	TechnoExtContainer::Instance.Find(pTarget)->Is_DriverKilled = pToOwner->Type->MultiplayPassive;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTarget->GetTechnoType());

	if (pTarget->Passengers.GetFirstPassenger())
	{
		if (pTypeExt->Operator_Any)
		{
			// kill first passenger
			auto const pPassenger = pTarget->Passengers.RemoveFirstPassenger();
			pPassenger->RegisterDestruction(pKiller);
			pPassenger->UnInit();

		}
		else if (!pTypeExt->Operators.empty())
		{
			// find the driver cowardly hiding among the passengers, then kill him
			for (NextObject passenger(pTarget->Passengers.GetFirstPassenger()); passenger; ++passenger)
			{
				auto const pPassenger = static_cast<FootClass*>(*passenger);

				if (pTypeExt->Operators.Contains(pPassenger->GetTechnoType()))
				{
					pTarget->Passengers.RemovePassenger(pPassenger);
					pPassenger->RegisterDestruction(pKiller);
					pPassenger->UnInit();
					break;
				}
			}
		}

		// if passengers remain in the vehicle, operator-using or not, they should leave
		if (pTarget->Passengers.GetFirstPassenger())
		{
			TechnoExtData::EjectPassengers((FootClass*)pTarget, -1);
		}
	}

	if (ResetVet)
		pTarget->Veterancy.Reset();

	pTarget->HijackerInfantryType = -1;

	// If this unit is driving under influence, we have to free it first
	if (auto const pController = pTarget->MindControlledBy)
	{
		if (auto const pCaptureManager = pController->CaptureManager)
		{
			pCaptureManager->FreeUnit(pTarget);
		}
	}

	pTarget->MindControlledByAUnit = false;
	pTarget->MindControlledByHouse = nullptr;

	// remove the mind-control ring anim
	if (pTarget->MindControlRingAnim)
	{
		pTarget->MindControlRingAnim->TimeToDie = true;
		pTarget->MindControlRingAnim->UnInit();
		pTarget->MindControlRingAnim = nullptr;
	}

	// If this unit mind controls stuff, we should free the controllees, since they still belong to the previous owner
	if (pTarget->CaptureManager)
	{
		pTarget->CaptureManager->FreeAll();
	}

	// This unit will be freed of its duties
	if (auto const pFoot = abstract_cast<FootClass*>(pTarget))
	{
		if (pFoot->BelongsToATeam())
		{
			pFoot->Team->LiberateMember(pFoot);
		}
	}

	// If this unit spawns stuff, we should kill the spawns, since they still belong to the previous owner
	if (auto const pSpawnManager = pTarget->SpawnManager)
	{
		pSpawnManager->KillNodes();
		pSpawnManager->ResetTarget();
	}

	if (auto const pSlaveManager = pTarget->SlaveManager)
	{
		pSlaveManager->Killed(pKiller);
		pSlaveManager->ZeroOutSlaves();
		pSlaveManager->Owner = pTarget;
		if (pToOwner->Type->MultiplayPassive)
		{
			pSlaveManager->SuspendWork();
		}
		else
		{
			pSlaveManager->ResumeWork();
		}
	}

	// Hand over to a different house
	pTarget->SetOwningHouse(pToOwner);

	if (pToOwner->Type->MultiplayPassive)
	{
		pTarget->QueueMission(passiveMission, true);
	}

	pTarget->SetTarget(nullptr);
	pTarget->SetDestination(nullptr, false);

	if (auto firstTag = pTarget->AttachedTag)
		firstTag->SpringEvent((TriggerEvent)AresTriggerEvents::DriverKilled_ByHouse, pTarget, CellStruct::Empty, false, pToOwner);

	if (pTarget->IsAlive)
	{
		if (auto pSecTag = pTarget->AttachedTag)
			pSecTag->SpringEvent((TriggerEvent)AresTriggerEvents::DriverKiller, pTarget, CellStruct::Empty, false, nullptr);
	}

}

std::pair<TechnoTypeClass*, AbstractType> NOINLINE GetOriginalType(TechnoClass* pThis, TechnoTypeClass* pToType)
{
	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry:
		return { (TechnoTypeClass*)(((InfantryClass*)pThis)->Type) , AbstractType::InfantryType };
	case AbstractType::Unit:
		return { (TechnoTypeClass*)(((UnitClass*)pThis)->Type), AbstractType::UnitType };
	case AbstractType::Aircraft:
		return { (TechnoTypeClass*)(((AircraftClass*)pThis)->Type), AbstractType::AircraftType };
	default:
		Debug::FatalErrorAndExit("%s is not FootClass, conversion not allowed\n", pToType->ID);
		return { nullptr, AbstractType::None };
	}
}

void NOINLINE SetType(TechnoClass* pThis, AbstractType rtti, TechnoTypeClass* pToType)
{
	switch (rtti)
	{
	case AbstractType::Infantry:
	case AbstractType::InfantryType:
		((InfantryClass*)pThis)->Type = (InfantryTypeClass*)pToType;
		break;
	case AbstractType::Unit:
	case AbstractType::UnitType:
		((UnitClass*)pThis)->Type = (UnitTypeClass*)pToType;
		break;
	case AbstractType::Aircraft:
	case AbstractType::AircraftType:
		((AircraftClass*)pThis)->Type = (AircraftTypeClass*)pToType;
		break;
	default:
		break;
	}
}

bool NOINLINE TechnoExt_ExtData::ConvertToType(TechnoClass* pThis, TechnoTypeClass* pToType, bool AdjustHealth, bool IsChangeOwnership)
{
	const auto& [prevType, rtti] = GetOriginalType(pThis, pToType);
	const auto pOldType = prevType;
	Debug::Log("Attempt to convert TechnoType[%s] to [%s]\n", pOldType->ID, pToType->ID);

	if (pToType->WhatAmI() != rtti || pOldType->Spawned != pToType->Spawned || pOldType->MissileSpawn != pToType->MissileSpawn)
	{
		Debug::Log("Incompatible types between %s and %s\n", pOldType->ID, pToType->ID);
		return false;
	}

	const auto pToTypeExt = TechnoTypeExtContainer::Instance.Find(pToType);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	// Detach CLEG targeting
	if (pThis->TemporalImUsing && pThis->TemporalImUsing->Target)
		pThis->TemporalImUsing->Detach();

	HouseClass* const pOwner = pThis->Owner;

	//special cases , in this case dont need to do anything to the counter
	//just convert the techno
	if (!IsChangeOwnership)
	{

		// Remove tracking of old techno
		if (!pThis->InLimbo)
			pOwner->RegisterLoss(pThis, false);

		pOwner->RemoveTracking(pThis);
	}

	const int oldHealth = pThis->Health;

	SetType(pThis, rtti, pToType);

	if (AdjustHealth)
	{
		// Readjust health according to percentage
		pThis->SetHealthPercentage((double)(oldHealth) / (double)pOldType->Strength);
		pThis->EstimatedHealth = pThis->Health;
	}
	else
	{
		pThis->Health = pToType->Strength;
		pThis->EstimatedHealth = pToType->Strength;
	}

	//special cases , in this case dont need to do anything to the counter
	//just convert the techno
	if (!IsChangeOwnership)
	{
		// Add tracking of new techno
		pOwner->AddTracking(pThis);
		if (!pThis->InLimbo)
			pOwner->RegisterGain(pThis, true);
	}

	pOwner->RecheckTechTree = true;
	TechnoExtContainer::Instance.Find(pThis)->Is_Operated = false;
	AresAE::RemoveSpecific(&TechnoExtContainer::Instance.Find(pThis)->AeData, pThis, pOldType);
	PhobosAEFunctions::UpdateSelfOwnedAttachEffects(pThis, pToType);

	if (!pThis->IsAlive)
		return false;

	// remove previous line trail
	GameDelete<true, true>(pThis->LineTrailer);

	// create new one if new type require it
	if (pToType->UseLineTrail)
	{
		pThis->LineTrailer = GameCreate<LineTrail>();

		if (RulesClass::Instance->LineTrailColorOverride != ColorStruct::Empty)
		{
			pThis->LineTrailer->Color = RulesClass::Instance->LineTrailColorOverride;
		}
		else
		{
			pThis->LineTrailer->Color = pToType->LineTrailColor;
		}

		pThis->LineTrailer->SetDecrement(pToType->LineTrailColorDecrement);
		pThis->LineTrailer->Owner = pThis;
	}

	TechnoExtData::InitializeLaserTrail(pThis, true);

	// Reset AutoDeath Timer
	if (pExt->Death_Countdown.HasStarted()) {
		pExt->Death_Countdown.Stop();
		HouseExtData::AutoDeathObjects.erase(pThis);
	}

	if (pExt->PassengerDeletionTimer.IsTicking()
	&& !pToTypeExt->PassengerDeletionType.Enabled)
		pExt->PassengerDeletionTimer.Stop();

	TrailsManager::Construct(static_cast<TechnoClass*>(pThis), true);

	// Update open topped state of potential passengers if transport's OpenTopped value changes.
	bool toOpenTopped = pToType->OpenTopped && !pOldType->OpenTopped;

	if ((toOpenTopped || (!pToType->OpenTopped && pOldType->OpenTopped)) && pThis->Passengers.NumPassengers > 0)
	{
		auto pPassenger = pThis->Passengers.FirstPassenger;

		while (pPassenger)
		{
			if (toOpenTopped)
			{
				pThis->EnteredOpenTopped(pPassenger);
			}
			else
			{
				pThis->ExitedOpenTopped(pPassenger);

				// Lose target & destination
				pPassenger->Guard();

				// OpenTopped adds passengers to logic layer when enabled. Under normal conditions this does not need to be removed since
				// OpenTopped state does not change while passengers are still in transport but in case of type conversion that can happen.
				MapClass::Logics.get().RemoveObject(pPassenger);
			}

			pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
		}
	}

	// replace spawner type and some properties
	if (auto pSpawnManager = pThis->SpawnManager)
	{
		// this check is special for `SupportWeapon`
		// since it not using the type data spawner
		// it using the custom made
		// make sure it wont cause any problem here ,..
		if (pOldType->Spawns)
		{
			if (pSpawnManager->SpawnType != pToType->Spawns)
				pSpawnManager->SpawnType = pToType->Spawns;

			if (pToType->SpawnsNumber > 0)
				pSpawnManager->SpawnCount = pToType->SpawnsNumber;

			if (pToType->SpawnRegenRate > 0)
				pSpawnManager->RegenRate = pToType->SpawnRegenRate;

			if (pToType->SpawnReloadRate > 0)
				pSpawnManager->ReloadRate = pToType->SpawnReloadRate;
		}
	}

	if (pThis->IsDisguised() && pOldType->DisguiseWhenStill != pToType->DisguiseWhenStill)
		pThis->ClearDisguise();

	// Adjust ammo
	int ammoLeft = MinImpl(pThis->Ammo, pToType->Ammo);
	pThis->Ammo = ammoLeft;
	if (ammoLeft < 0 || ammoLeft >= pToType->Ammo)
	{
		pThis->ReloadTimer.Stop();
	}
	else
	{
		int reloadLeft = pThis->ReloadTimer.GetTimeLeft();
		int reloadPrev = 0;
		int reloadNew = 0;
		if (ammoLeft == 0)
		{
			reloadPrev = pOldType->EmptyReload;
			reloadNew = pToType->EmptyReload;
		}
		else if (pThis->Ammo)
		{
			reloadPrev = pOldType->Reload;
			reloadNew = pToType->Reload;
		}
		int pass = reloadPrev - reloadLeft;
		if (pass <= 0 || pass >= reloadNew)
		{
			pThis->ReloadTimer.Stop();
		}
		else
		{
			int reload = reloadNew - pass;
			pThis->ReloadTimer.Start(reload);
		}
	}

	BuildingLightClass* pSpot = nullptr;

	if (pToTypeExt->HasSpotlight)
	{
		pSpot = GameCreate<BuildingLightClass>(pThis);
	}

	TechnoExt_ExtData::SetSpotlight(pThis, pSpot);
	const int value = MinImpl(pToType->ROT, 127);
	(&pThis->PrimaryFacing)->ROT.Raw = value << 8;

	const int valuesec = MinImpl(pToTypeExt->TurretRot.Get(pToType->ROT), 127);
	(&pThis->SecondaryFacing)->ROT.Raw = valuesec << 8;

	// // because we are throwing away the locomotor in a split second, piggybacking
	// // has to be stopped. otherwise the object might remain in a weird state.
	// // throw the piggybacked loco
	// while (LocomotionClass::End_Piggyback(((FootClass*)pThis)->Locomotor));

	//fucker
	const int WeaponCount = (pToType->WeaponCount > 0 ? pToType->WeaponCount : 2);
	if (pThis->CurrentWeaponNumber >= WeaponCount)
		pThis->CurrentWeaponNumber = 0;

	const int TurretCount = (pToType->TurretCount > 0 ? pToType->TurretCount : 2);

	if (pThis->CurrentTurretNumber >= TurretCount)
		pThis->CurrentTurretNumber = 0;


	// Update movement sound if still moving while type changed.
	if (auto const pFoot = abstract_cast<FootClass*>(pThis))
	{
		if (pFoot->Locomotor->Is_Moving_Now() && pFoot->__PlayingMovingSound)
		{
			if (pOldType->MoveSound != pToType->MoveSound)
			{
				// End the old sound.
				pFoot->Audio7.AudioEventHandleStop();

				if (auto const count = pOldType->MoveSound.Count)
				{
					if(pToType->MoveSound.Count) {
						// Play a new sound.
						int soundIndex = pToType->MoveSound[Random2Class::Global->Random() % count];
						VocClass::PlayAt(soundIndex, pFoot->Location, &pFoot->Audio7);
						pFoot->__PlayingMovingSound = true;
					}
				}
				else
				{
					pFoot->__PlayingMovingSound = false;
				}

				pFoot->__MovingSoundDelay = 0;
			}
		}
	}

	//if (pThis->LocomotorSource) {
	//	Debug::Log("Attempt to convert TechnoType[%s] to [%s] when the locomotor is currently manipulated , return\n", pOldType->ID, pToType->ID);
	//	return true;
	//}
	bool move = true;

	// replace the original locomotor to new one
	if (pOldType->Locomotor != pToType->Locomotor)
	{
		if (pOldType->Locomotor == CLSIDs::Teleport && pToType->Locomotor != CLSIDs::Teleport && pThis->WarpingOut)
			TechnoExtContainer::Instance.Find(pThis)->HasRemainingWarpInDelay = true;

		AbstractClass* pTarget = pThis->Target;
		AbstractClass* pDest = pThis->ArchiveTarget;
		Mission prevMission = pThis->GetCurrentMission();

		// throw away the current locomotor and instantiate
		// a new one of the default type for this unit.
		if (auto newLoco = LocomotionClass::CreateInstance(pToType->Locomotor))
		{
			newLoco->Link_To_Object(pThis);
			((FootClass*)pThis)->Locomotor = std::move(newLoco);
			pThis->Override_Mission(prevMission, pTarget, pDest);
		}
	}
	else if (pOldType->Locomotor == CLSIDs::Jumpjet() && pToType->Locomotor == CLSIDs::Jumpjet() && !(pOldType->JumpjetData == pToType->JumpjetData))
	{
		move = false;
		AbstractClass* pTarget = pThis->Target;
		AbstractClass* pDest = pThis->ArchiveTarget;
		Mission prevMission = pThis->GetCurrentMission();

		// throw away the current locomotor and instantiate
		// a new one of the default type for this unit.
		// throw away old loco to ensure the new loco properties is properly adjusted
		if (auto newLoco = LocomotionClass::CreateInstance(pToType->Locomotor))
		{
			newLoco->Link_To_Object(pThis);
			((FootClass*)pThis)->Locomotor = std::move(newLoco);
			((FootClass*)pThis)->Locomotor.GetInterfacePtr()->Move_To(pThis->Location);
			pThis->Override_Mission(prevMission, pTarget, pDest);
		}
	}

	if (move && pToType->BalloonHover && pToType->DeployToLand && pOldType->Locomotor != CLSIDs::Jumpjet() && pToType->Locomotor == CLSIDs::Jumpjet())
	{
		((FootClass*)pThis)->Locomotor.GetInterfacePtr()->Move_To(pThis->Location);
	}

	if (auto pInf = specific_cast<InfantryClass*>(pThis))
	{
		// It's still not recommended to have such idea, please avoid using this
		if (static_cast<InfantryTypeClass*>(pOldType)->Deployer && !static_cast<InfantryTypeClass*>(pToType)->Deployer)
		{
			switch (pInf->SequenceAnim)
			{
			case DoType::Deploy:
			case DoType::Deployed:
			case DoType::DeployedIdle:
				pInf->PlayAnim(DoType::Ready, true); break;
			case DoType::DeployedFire:
				pInf->PlayAnim(DoType::FireUp, true); break;
			default:break;
			}
		}
	}

	pThis->See(0u,0u);

	return true;
}

int TechnoExt_ExtData::GetSelfHealAmount(TechnoClass* pThis)
{

	auto const pType = pThis->GetTechnoType();
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pType->SelfHealing || pThis->HasAbility(AbilityType::SelfHeal))
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		if (pExt->SelfHealing_CombatDelay.GetTimeLeft())
			return 0x0;

		const auto rate = pTypeExt->SelfHealing_Rate.Get(
			RulesClass::Instance->RepairRate);

		const auto frames = MaxImpl(int(rate * 900.0), 1);

		if (Unsorted::CurrentFrame % frames == 0)
		{
			const auto strength = pType->Strength;

			const auto percent = pTypeExt->SelfHealing_Max.Get(pThis);
			const auto maxHealth = std::clamp(int(percent * strength), 1, strength);
			const auto health = pThis->Health;

			if (health < maxHealth)
			{
				const auto amount = pTypeExt->SelfHealing_Amount.Get(pThis);
				return std::clamp(amount, 0, maxHealth - health);
			}
		}
	}

	return 0;
}

void TechnoExt_ExtData::SpawnVisceroid(CoordStruct& crd, UnitTypeClass* pType, int chance, bool ignoreTibDeathToVisc, HouseClass* Owner)
{

	bool created = false;

	// create a small visceroid if available and the cell is free
	// dont create if `pType` is 0 strength because it is not properly listed
	if (ignoreTibDeathToVisc && pType && pType->Strength > 0)
	{
		const auto pCell = MapClass::Instance->GetCellAt(crd);

		if (!(pCell->OccupationFlags & 0x20) && ScenarioClass::Instance->Random.RandomFromMax(99) < chance)
		{
			if (auto pVisc = (UnitClass*)pType->CreateObject(Owner))
			{
				++Unsorted::ScenarioInit;
				created = pVisc->Unlimbo(crd, DirType::North);
				--Unsorted::ScenarioInit;

				if (!created)
				{
					Debug::Log(__FUNCTION__" Called \n");
					TechnoExtData::HandleRemove(pVisc, nullptr, true, true);
				}
			}
		}
	}
}

void TechnoExt_ExtData::TransferOriginalOwner(TechnoClass* pFrom, TechnoClass* pTo)
{
	TechnoExtContainer::Instance.Find(pFrom)->OriginalHouseType = TechnoExtContainer::Instance.Find(pTo)->OriginalHouseType;
}

void TechnoExt_ExtData::TransferIvanBomb(TechnoClass* From, TechnoClass* To)
{
	if (auto Bomb = From->AttachedBomb)
	{
		From->AttachedBomb = nullptr;
		Bomb->Target = To;
		To->AttachedBomb = Bomb;
		To->BombVisible = From->BombVisible;
		// if there already was a bomb attached to target unit, it's gone now...
		// it shouldn't happen though, this is used for (un)deploying objects only
	}
}

void NOINLINE UpdateType(TechnoClass* pThis, TechnoTypeExtData* pOldTypeExt)
{
	if (pOldTypeExt->Convert_Water || pOldTypeExt->Convert_Land)
	{
		TechnoTypeClass* Convert = pOldTypeExt->Convert_Land;
		if (!pThis->OnBridge)
		{ //avoid calling `GetCell()` all the time ?
			CellClass* pCell = pThis->GetCell();
			if (pCell && (pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach))
				Convert = pOldTypeExt->Convert_Water;
		}

		if (Convert && pOldTypeExt->AttachedToObject != Convert)
			TechnoExt_ExtData::ConvertToType(pThis, Convert);
	}
}

void NOINLINE UpdatePassengerTurrent(TechnoClass* pThis, TechnoTypeExtData* pTypeData)
{
	const auto pType = pTypeData->AttachedToObject;
	if (pTypeData->PassengerTurret)
	{
		// 18 = 1 8 = A H = Adolf Hitler. Clearly we can't allow it to come to that.
		pThis->CurrentTurretNumber = MinImpl(
		MinImpl(pThis->Passengers.NumPassengers, TechnoTypeClass::MaxWeapons - 1),
		pType->TurretCount - 1);

	}

	if (pTypeData->PassengerWeapon && !pType->IsGattling && pType->WeaponCount > 0){
		pThis->CurrentWeaponNumber = MinImpl(
			MinImpl(pThis->Passengers.NumPassengers, TechnoTypeClass::MaxWeapons - 1),
			pType->WeaponCount - 1);
	}
}

void NOINLINE UpdatePoweredBy(TechnoClass* pThis, TechnoTypeExtData* pTypeData)
{
	if (!pTypeData->PoweredBy.empty())
	{
		if (!TechnoExtContainer::Instance.Find(pThis)->PoweredUnit)
		{
			TechnoExtContainer::Instance.Find(pThis)->PoweredUnit = std::make_unique<AresPoweredUnit>(pThis);
		}

		if (!TechnoExtContainer::Instance.Find(pThis)->PoweredUnit->Update())
		{
			TechnoExt_ExtData::Destroy(pThis, nullptr, nullptr, nullptr);
		}
	}
}

void NOINLINE UpdateBuildingOperation(TechnoExtData* pData, TechnoTypeExtData* pTypeData)
{
	auto const pThis = pData->AttachedToObject;

	if (TechnoExtContainer::Instance.Find(pThis)->Is_Operated && pThis->WhatAmI() == BuildingClass::AbsID)
	{
		if (pThis->Deactivated
			&& pThis->IsPowerOnline()
			&& !pThis->IsUnderEMP()
			&& TechnoExt_ExtData::IsPowered(pThis)
			)
		{
			pThis->Reactivate();
			pThis->Owner->RecheckTechTree = true; // #885
		}
	}
	else
	{
		auto const pBuildingBelow = pThis->GetCell()->GetBuilding();
		auto const buildingBelowIsMe = pThis == pBuildingBelow;

		if (!pBuildingBelow || (buildingBelowIsMe && pBuildingBelow->IsPowerOnline()))
		{
			bool Override = false;
			if (auto const pFoot = abstract_cast<FootClass*>(pThis))
			{
				if (!pBuildingBelow)
				{
					// immobile, though not disabled. like hover tanks after
					// a repair depot has been sold or warped away.
					Override = ((BYTE)pFoot->Locomotor.GetInterfacePtr()->Is_Powered() == pThis->Deactivated);
				}
			}

			if (TechnoExt_ExtData::IsOperatedB(pThis))
			{ // either does have an operator or doesn't need one, so...
				if (Override || (pThis->Deactivated && !pThis->IsUnderEMP() && TechnoExt_ExtData::IsPowered(pThis)))
				{ // ...if it's currently off, turn it on! (oooh baby)
					pThis->Reactivate();
					if (buildingBelowIsMe)
					{
						pThis->Owner->RecheckTechTree = true; // #885
					}
				}
			}
			else
			{ // doesn't have an operator, so...
				if (!pThis->Deactivated)
				{ // ...if it's not off yet, turn it off!
					pThis->Deactivate();
					if (buildingBelowIsMe)
					{
						pThis->Owner->RecheckTechTree = true; // #885
					}
				}
			}
		}
	}
}

void NOINLINE UpdateRadarJammer(TechnoExtData* pData, TechnoTypeExtData* pTypeData)
{
	auto const pThis = pData->AttachedToObject;

	// prevent disabled units from driving around.
	if (pThis->Deactivated)
	{
		if (auto const pUnit = specific_cast<UnitClass*>(pThis))
		{
			if (pUnit->Locomotor->Is_Moving() && pUnit->Destination && !pThis->LocomotorSource)
			{
				pUnit->SetDestination(nullptr, true);
				pUnit->StopMoving();
			}
		}

		// dropping Radar Jammers (#305) here for now; should check if another TechnoClass::Update hook might be better ~Ren
		if (auto& pJam = TechnoExtContainer::Instance.Find(pThis)->RadarJammer)
		{ // RadarJam should only be non-null if the object is an active radar jammer
			pJam->UnjamAll();
		}
	}
	else
	{
		// dropping Radar Jammers (#305) here for now; should check if another TechnoClass::Update hook might be better ~Ren
		if (pTypeData->RadarJamRadius)
		{
			if (!TechnoExtContainer::Instance.Find(pThis)->RadarJammer)
			{
				TechnoExtContainer::Instance.Find(pThis)->RadarJammer = std::make_unique<AresJammer>(pThis);
			}

			TechnoExtContainer::Instance.Find(pThis)->RadarJammer->Update();
		}
	}
}

#include "Classes/AttachedAffects.h"

void TechnoExt_ExtData::Ares_technoUpdate(TechnoClass* pThis)
{
	const auto pOldType = pThis->GetTechnoType();
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	auto pOldTypeExt = TechnoTypeExtContainer::Instance.Find(pOldType);

	UpdateType(pThis, pOldTypeExt);
	UpdatePassengerTurrent(pThis, pOldTypeExt);
	UpdatePoweredBy(pThis, pOldTypeExt);
	AresAE::Update(&pExt->AeData, pThis);
	UpdateBuildingOperation(pExt, pOldTypeExt);
	UpdateRadarJammer(pExt, pOldTypeExt);

	if (pExt->TechnoValueAmount)
		TechnoExt_ExtData::Ares_AddMoneyStrings(pThis, false);

	const auto pFoot = generic_cast<FootClass*>(pThis);

	if (pFoot
		&& pExt->Is_DriverKilled
		&& pThis->CurrentMission != Mission::Harmless
		&& !pFoot->IsAttackedByLocomotor
		//&& ScenarioClass::Instance->Random.RandomBool()
		)
	{
		pThis->SetTarget(nullptr);
		pThis->EnterIdleMode(false, 1);
		pThis->QueueMission(Mission::Harmless, true);
	}
}

#include <New/Entity/FlyingStrings.h>

void TechnoExt_ExtData::Ares_AddMoneyStrings(TechnoClass* pThis, bool forcedraw)
{
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto value = pExt->TechnoValueAmount;
	if (value && (forcedraw || Unsorted::CurrentFrame >= pExt->Pos))
	{
		pExt->Pos = Unsorted::CurrentFrame - int32_t(RulesExtData::Instance()->DisplayCreditsDelay * -900.0);
		pExt->TechnoValueAmount = 0;
		bool isPositive = value > 0;
		wchar_t moneyStr[0x20];

		const ColorStruct& color = isPositive
			? Drawing::DefaultColors[(int)DefaultColorList::Green] :
			Drawing::DefaultColors[(int)DefaultColorList::Red];

		swprintf_s(moneyStr, L"%ls%ls%d", isPositive ? L"+" : L"-", Phobos::UI::CostLabel, Math::abs(value));

		CoordStruct loc = pThis->GetCoords();
		if (!MapClass::Instance->IsLocationShrouded(loc)
			&& pThis->VisualCharacter(FALSE, HouseClass::CurrentPlayer()) != VisualType::Hidden)
		{
			if (pThis->WhatAmI() == BuildingClass::AbsID)
			{
				loc.Z += 104 * ((BuildingClass*)pThis)->Type->Height;
			}
			else
			{
				loc.Z += 256;
			}

			FlyingStrings::Add(moneyStr, loc, color, {});
		}
	}
}
#pragma endregion

#pragma region TechnoExperienceData

void TechnoExperienceData::AddAirstrikeFactor(TechnoClass*& pKiller, double& d_factor)
{
	// before we do any other logic, check if this kill was committed by an
	// air strike and its designator shall get the experience.
	if (pKiller->Airstrike)
	{
		if (const auto pDesignator = pKiller->Airstrike->Owner)
		{
			const auto pDesignatorExt = TechnoTypeExtContainer::Instance.Find(pDesignator->GetTechnoType());

			if (pDesignatorExt->ExperienceFromAirstrike)
			{
				pKiller = pDesignator;
				d_factor *= pDesignatorExt->AirstrikeExperienceModifier;
			}
		}
	}
}

bool TechnoExperienceData::KillerInTransporterFactor(TechnoClass* pKiller, TechnoClass*& pExpReceiver, double& d_factor, bool& promoteImmediately)
{
	const auto pTransporter = pKiller->Transporter;
	if (!pTransporter)
		return false;

	const auto pTTransporterData = TechnoTypeExtContainer::Instance.Find(pTransporter->GetTechnoType());
	const auto TransporterAndKillerAllied = pTransporter->Owner->IsAlliedWith(pKiller);

	if (pKiller->InOpenToppedTransport)
	{
		// check for passenger of an open topped vehicle. transporter can get
		// experience from passengers; but only if the killer and its transporter
		// are allied. so a captured opentopped vehicle won't get experience from
		// the enemy's orders.

		// if passengers can get promoted and this transport is already elite,
		// don't promote this transport in favor of the real killer.
		const TechnoTypeClass* pTTransporter = pTransporter->GetTechnoType();

		if ((!pTTransporter->Trainable || pTTransporterData->PassengersGainExperience) && (pTransporter->Veterancy.IsElite() || !TransporterAndKillerAllied) && pKiller->GetTechnoType()->Trainable)
		{
			// the passenger gets experience
			pExpReceiver = pKiller;
			d_factor *= pTTransporterData->PassengerExperienceModifier;
		}
		else if (pTTransporter->Trainable && pTTransporterData->ExperienceFromPassengers && TransporterAndKillerAllied)
		{
			// the transporter gets experience
			pExpReceiver = pTransporter;
		}

		return true;
	}

	return false;
}

void TechnoExperienceData::AddExperience(TechnoClass* pExtReceiver, TechnoClass* pVictim, int victimCost, double factor)
{
	const auto pExpReceiverType = pExtReceiver->GetTechnoType();
	const auto pVinctimType = pVictim->GetTechnoType();
	const auto TechnoCost = pExpReceiverType->GetActualCost(pExtReceiver->Owner);
	const auto pVictimTypeExt = TechnoTypeExtContainer::Instance.Find(pVinctimType);
	const auto pKillerTypeExt = TechnoTypeExtContainer::Instance.Find(pExpReceiverType);

	const auto WeightedVictimCost = static_cast<int>(victimCost * factor *
		pKillerTypeExt->Experience_KillerMultiple * pVictimTypeExt->Experience_VictimMultiple);

	if (TechnoCost > 0 && WeightedVictimCost > 0)
	{
		pExtReceiver->Veterancy.Add(TechnoCost, WeightedVictimCost);
	}
}

void TechnoExperienceData::MCControllerGainExperince(TechnoClass* pExpReceiver, TechnoClass* pVictim, double& d_factor, int victimCost)
{
	// mind-controllers get experience, too.
	if (auto pController = pExpReceiver->MindControlledBy)
	{
		if (!pController->Owner->IsAlliedWith(pVictim->Owner))
		{

			// get the mind controllers extended properties
			const auto pTController = pController->GetTechnoType();
			const auto pTControllerData = TechnoTypeExtContainer::Instance.Find(pTController);

			// promote the mind-controller
			if (pTController->Trainable)
			{
				// the mind controller gets its own factor
				AddExperience(pController, pVictim, victimCost, d_factor * pTControllerData->MindControlExperienceSelfModifier);
			}

			// modify the cost of the victim.
			d_factor *= pTControllerData->MindControlExperienceVictimModifier;
		}
	}
}

void TechnoExperienceData::GetSpawnerData(TechnoClass*& pSpawnOut, TechnoClass*& pExpReceiver, double& d_spawnFacor, double& d_ExpFactor)
{
	if (const auto pSpawner = pExpReceiver->SpawnOwner)
	{
		const auto pTSpawner = pSpawner->GetTechnoType();
		if (!pTSpawner->MissileSpawn && pTSpawner->Trainable)
		{
			const auto pTSpawnerData = TechnoTypeExtContainer::Instance.Find(pTSpawner);

			// add experience to the spawn. this is done later so mind-control
			// can be factored in.
			d_spawnFacor = pTSpawnerData->SpawnExperienceSpawnModifier;
			pSpawnOut = pExpReceiver;

			// switch over to spawn owners, and factor in the spawner multiplier
			d_ExpFactor *= pTSpawnerData->SpawnExperienceOwnerModifier;
			pExpReceiver = pSpawner;
		}
	}
}

void TechnoExperienceData::PromoteImmedietely(TechnoClass* pExpReceiver, bool bSilent, bool Flash)
{
	auto newRank = pExpReceiver->Veterancy.GetRemainingLevel();

	if (pExpReceiver->CurrentRanking != newRank)
	{
		if (pExpReceiver->CurrentRanking != Rank::Invalid)
		{
			auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pExpReceiver->GetTechnoType());

			if (pTypeExt->Promote_IncludePassengers)
			{
				auto const& nCur = pExpReceiver->Veterancy;

				for (NextObject object(pExpReceiver->Passengers.GetFirstPassenger()); object; ++object)
				{
					if (auto const pFoot = generic_cast<FootClass*>(*object))
					{
						if (!pFoot->GetTechnoType()->Trainable)
							continue;

						pFoot->Veterancy = nCur;// sync veterancy with the Transporter
						PromoteImmedietely(pFoot, true, false);
					}
				}
			}

			int sound = -1;
			int eva = -1;
			int flash = 0;
			TechnoTypeClass* pNewType = nullptr;
			double promoteExp = 0.0;
			auto const pRules = RulesClass::Instance.get();
			AnimTypeClass* Promoted_PlayAnim = nullptr;

			if (newRank == Rank::Veteran)
			{
				flash = pTypeExt->Promote_Vet_Flash.Get(RulesExtData::Instance()->VeteranFlashTimer);
				sound = pTypeExt->Promote_Vet_Sound.Get(pRules->UpgradeVeteranSound);
				eva = pTypeExt->Promote_Vet_Eva;
				pNewType = pTypeExt->Promote_Vet_Type;
				promoteExp = pTypeExt->Promote_Vet_Exp;
				Promoted_PlayAnim = pTypeExt->Promote_Vet_Anim.Get(RulesExtData::Instance()->Promote_Vet_Anim);
			}
			else if (newRank == Rank::Elite)
			{
				flash = pTypeExt->Promote_Elite_Flash.Get(pRules->EliteFlashTimer);
				sound = pTypeExt->Promote_Elite_Sound.Get(pRules->UpgradeEliteSound);
				eva = pTypeExt->Promote_Elite_Eva;
				pNewType = pTypeExt->Promote_Elite_Type;
				promoteExp = pTypeExt->Promote_Elite_Exp;
				Promoted_PlayAnim = pTypeExt->Promote_Elite_Anim.Get(RulesExtData::Instance()->Promote_Elite_Anim);
			}

			if (pNewType && TechnoExt_ExtData::ConvertToType(pExpReceiver, pNewType) && promoteExp != 0.0)
			{
				newRank = pExpReceiver->Veterancy.AddAndGetRank(promoteExp);
			}

			if (!bSilent && pExpReceiver->Owner->ControlledByCurrentPlayer())
			{
				VocClass::PlayIndexAtPos(sound, (pExpReceiver->Transporter ? pExpReceiver->Transporter : pExpReceiver)->Location, nullptr);
				VoxClass::PlayIndex(eva);
			}

			if (Flash && flash > 0)
			{
				pExpReceiver->Flashing.DurationRemaining = flash;
			}

			if (Promoted_PlayAnim && !pExpReceiver->InLimbo)
			{
				auto pAnim = GameCreate<AnimClass>(Promoted_PlayAnim, pExpReceiver->Location, 0, 1, 0x600u, 0, 0);
				pAnim->SetOwnerObject(pExpReceiver);

				if (pExpReceiver->WhatAmI() == BuildingClass::AbsID)
					pAnim->ZAdjust = -1024;
			}

			AEProperties::Recalculate(pExpReceiver);
			pExpReceiver->See(0u,0u);
		}

		pExpReceiver->CurrentRanking = newRank;
	}
}

void TechnoExperienceData::UpdateVeterancy(TechnoClass*& pExpReceiver, TechnoClass* pKiller, TechnoClass* pVictim, int VictimCost, double& d_factor, bool promoteImmediately)
{
	if (pExpReceiver)
	{
		// no way to get experience by proxy by an enemy unit. you cannot
		// promote your mind-controller by capturing friendly units.
		if (pExpReceiver->Owner->IsAlliedWith(pKiller))
		{

			// if this is a non-missile spawn, handle the spawn manually and switch over to the
			// owner then. this way, a mind-controlled owner is supported.
			TechnoClass* pSpawn = nullptr;
			double SpawnFactor = 1.0;

			GetSpawnerData(pSpawn, pExpReceiver, SpawnFactor, d_factor);
			MCControllerGainExperince(pExpReceiver, pVictim, d_factor, VictimCost);

			// default. promote the unit this function selected.
			AddExperience(pExpReceiver, pVictim, VictimCost, d_factor);

			// if there is a spawn, let it get its share.
			if (pSpawn)
			{
				AddExperience(pSpawn, pVictim, VictimCost, d_factor * SpawnFactor);
			}

			// gunners need to be promoted manually, or they won't only get
			// the experience until after they exited their transport once.
			if (promoteImmediately)
			{
				PromoteImmedietely(pExpReceiver, false, false);
			}
		}
	}
}

void TechnoExperienceData::EvaluateExtReceiverData(TechnoClass*& pExpReceiver, TechnoClass* pKiller, double& d_factor, bool& promoteImmediately)
{
	const auto pKillerTechnoType = pKiller->GetTechnoType();
	const auto pKillerTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pKillerTechnoType);

	if (!KillerInTransporterFactor(pKiller, pExpReceiver, d_factor, promoteImmediately))
	{
		if (pKillerTechnoType->Gunner)
		{
			// an IFV can get experience, too, but we have to have an extra check
			// because the gunner is not the killer.
			FootClass* pGunner = pKiller->Passengers.GetFirstPassenger();
			const auto& nKillerVet = pKiller->Veterancy;
			if (pKillerTechnoType->Trainable && !nKillerVet.IsElite() && (!pGunner || pKillerTechnoTypeExt->ExperienceFromPassengers))
			{
				// the IFV gets credited
				pExpReceiver = pKiller;
			}
			else if (pGunner
				&& (nKillerVet.IsElite() || !pKillerTechnoTypeExt->ExperienceFromPassengers)
				&& pGunner->GetTechnoType()->Trainable && pKillerTechnoTypeExt->PassengersGainExperience)
			{

				pExpReceiver = pGunner;
				d_factor *= pKillerTechnoTypeExt->PassengerExperienceModifier;
				promoteImmediately = true;
			}

		}
		else if (pKillerTechnoType->Trainable)
		{

			// the killer itself gets credited.
			pExpReceiver = pKiller;

		}
		else if (pKillerTechnoType->MissileSpawn)
		{

			// unchanged game logic
			if (TechnoClass* pSpawner = pKiller->SpawnOwner)
			{
				TechnoTypeClass* pTSpawner = pSpawner->GetTechnoType();
				if (pTSpawner->Trainable)
				{
					pExpReceiver = pSpawner;
				}
			}

		}
		else if (pKiller->CanOccupyFire())
		{
			// game logic, with added check for Trainable
			if (BuildingClass* pKillerBld = specific_cast<BuildingClass*>(pKiller))
			{
				InfantryClass* pOccupant = pKillerBld->Occupants[pKillerBld->FiringOccupantIndex];
				if (pOccupant->Type->Trainable)
				{
					pExpReceiver = pOccupant;
				}
			}
		}
	}
}

#pragma endregion

#pragma region FirewallFunctions

DWORD FirewallFunctions::GetFirewallFlags(BuildingClass* pThis)
{
	auto pCell = MapClass::Instance->GetCellAt(pThis->Location);
	DWORD flags = 0;
	for (size_t direction = 0; direction < 8; direction += 2)
	{
		if (auto pNeighbour = pCell->GetNeighbourCell((FacingType)direction))
		{
			if (auto pBld = pNeighbour->GetBuilding())
			{
				if (BuildingTypeExtContainer::Instance.Find(pBld->Type)->Firestorm_Wall
					&& pBld->Owner == pThis->Owner
					&& !pBld->InLimbo
					&& pBld->IsAlive
				)
				{
					flags |= 1 << (direction >> 1);
				}
			}
		}
	}

	return flags;
}

void FirewallFunctions::ImmolateVictims(TechnoClass* pThis)
{
	auto const pCell = pThis->GetCell();
	for (NextObject object(pCell->FirstObject); object; ++object)
	{
		if (auto pFoot = abstract_cast<FootClass*>(*object))
		{
			if (!pFoot->GetType()->IgnoresFirestorm)
			{
				FirewallFunctions::ImmolateVictim(pThis, pFoot);
			}
		}
	}
}

bool FirewallFunctions::ImmolateVictim(TechnoClass* pThis, ObjectClass* const pVictim, bool const destroy)
{
	if (pVictim && pVictim->IsAlive && pVictim->Health > 0 && !pVictim->InLimbo)
	{
		auto const pRulesExt = RulesExtData::Instance();

		if (destroy)
		{
			const auto pWarhead = pRulesExt->FirestormWarhead.Get(
				RulesClass::Instance->C4Warhead);

			auto damage = pVictim->Health;
			pVictim->ReceiveDamage(&damage, 0, pWarhead, pThis, true, true, pThis->Owner);
		}

		auto const& pType = (pVictim->GetHeight() < 100)
			? pRulesExt->FirestormGroundAnim
			: pRulesExt->FirestormAirAnim;

		if (pType)
		{
			auto const crd = pVictim->GetCoords();
			HouseClass* pTarget = nullptr;
			switch (pVictim->WhatAmI())
			{
			case AbstractType::Building:
			case AbstractType::Unit:
			case AbstractType::Aircraft:
			case AbstractType::Infantry:
				pTarget = ((TechnoClass*)pVictim)->Owner;
				break;
			case AbstractType::Bullet:
			{
				const auto pBlt = (BulletClass*)pVictim;
				pTarget = pBlt->Owner ? pBlt->Owner->Owner : BulletExtContainer::Instance.Find(pBlt)->Owner;
			}
			break;
			default:
				break;
			}

			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, crd, 0, 1, 0x600, -10, false),
				pThis->Owner,
				pTarget,
				pThis,
				false
			);

		}

		return true;
	}

	return false;
}

void FirewallFunctions::UpdateFirewall(BuildingClass* pThis, bool const changedState)
{
	if (pThis->InLimbo || !pThis->IsAlive)
	{
		return;
	}

	auto const active = pThis->Owner->FirestormActive;

	if (!changedState)
	{
		// update only the idle anim
		auto& Anim = pThis->GetAnim(BuildingAnimSlot::SpecialTwo);

		// (0b0101 || 0b1010) == part of a straight line
		auto const connections = pThis->FirestormWallFrame & 0xF;
		if (active && (Unsorted::CurrentFrame & 7) && !Anim
			&& connections != 0b0101 && connections != 0b1010
			&& (ScenarioClass::Instance->Random.Random() & 0xF) == 0)
		{
			if (AnimTypeClass* pType = RulesExtData::Instance()->FirestormIdleAnim)
			{
				auto const crd = pThis->GetCoords() - CoordStruct { 740, 740, 0 };
				Anim = GameCreate<AnimClass>(pType, crd, 0, 1, 0x604, -10);
				Anim->IsBuildingAnim = true;
			}
		}
	}
	else
	{
		// update the frame, cell passability and active anim
		auto const idxFrame = FirewallFunctions::GetFirewallFlags(pThis)
			+ (active ? 32u : 0u);

		if (pThis->FirestormWallFrame != idxFrame)
		{
			pThis->FirestormWallFrame = idxFrame;
			pThis->GetCell()->RecalcAttributes(0xFFFFFFFF);
			pThis->UpdatePlacement(PlacementType::Redraw);
		}

		auto& Anim = pThis->GetAnim(BuildingAnimSlot::Special);

		auto const connections = idxFrame & 0xF;
		if (active && connections != 0b0101 && connections != 0b1010 && !Anim)
		{
			if (auto const& pType = RulesExtData::Instance()->FirestormActiveAnim)
			{
				auto const crd = pThis->GetCoords() - CoordStruct { 128, 128, 0 };
				Anim = GameCreate<AnimClass>(pType, crd, 1, 0, 0x600, -10);
				Anim->IsFogged = pThis->IsFogged;
				Anim->IsBuildingAnim = true;
			}
		}
		else if (Anim)
		{
			Anim->TimeToDie = true;
			Anim->UnInit();
			Anim = nullptr;
		}
	}

	if (active)
	{
		FirewallFunctions::ImmolateVictims(pThis);
	}
}

void FirewallFunctions::UpdateFirewallLinks(BuildingClass* pThis)
{
	if (BuildingTypeExtContainer::Instance.Find(pThis->Type)->Firestorm_Wall)
	{
		// update this
		FirewallFunctions::UpdateFirewall(pThis, true);

		// and all surrounding buildings
		auto const pCell = MapClass::Instance->GetCellAt(pThis->Location);
		for (size_t i = 0u; i < 8; i += 2)
		{
			if (auto const pNeighbour = pCell->GetNeighbourCell((FacingType)i))
			{
				if (auto const pBld = pNeighbour->GetBuilding())
				{
					if (BuildingTypeExtContainer::Instance.Find(pBld->Type)->Firestorm_Wall)
						FirewallFunctions::UpdateFirewall(pBld, true);
				}
			}
		}
	}
}

bool FirewallFunctions::IsActiveFirestormWall(BuildingClass* const pBuilding, HouseClass const* const pIgnore)
{
	if (HouseExtData::IsAnyFirestormActive && pBuilding && pBuilding->Owner != pIgnore && pBuilding->Owner->FirestormActive)
	{
		if (!pBuilding->InLimbo && pBuilding->IsAlive)
		{
			return BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->Firestorm_Wall;
		}
	}

	return false;
}

bool FirewallFunctions::sameTrench(BuildingClass* currentBuilding, BuildingClass* targetBuilding)
{
	const auto currentTypeExtData = BuildingTypeExtContainer::Instance.Find(currentBuilding->Type);
	const auto targetTypeExtData = BuildingTypeExtContainer::Instance.Find(targetBuilding->Type);

	return ((currentTypeExtData->IsTrench > -1) && (currentTypeExtData->IsTrench == targetTypeExtData->IsTrench));
}

bool FirewallFunctions::canLinkTo(BuildingClass* currentBuilding, BuildingClass* targetBuilding)
{
	// Different owners // and owners not allied
	if ((currentBuilding->Owner != targetBuilding->Owner) && !currentBuilding->Owner->IsAlliedWith(targetBuilding->Owner))
	{ //<-- see thread 1424
		return false;
	}

	BuildingTypeExtData* currentTypeExtData = BuildingTypeExtContainer::Instance.Find(currentBuilding->Type);
	BuildingTypeExtData* targetTypeExtData = BuildingTypeExtContainer::Instance.Find(targetBuilding->Type);

	// Firewalls
	if (BuildingTypeExtContainer::Instance.Find(currentBuilding->Type)->Firestorm_Wall
		&& BuildingTypeExtContainer::Instance.Find(targetBuilding->Type)->Firestorm_Wall)
	{
		return true;
	}

	// Trenches
	if (FirewallFunctions::sameTrench(currentBuilding, targetBuilding))
	{
		return true;
	}

	return false;
}

void FirewallFunctions::BuildLines(BuildingClass* theBuilding, CellStruct selectedCell, HouseClass* buildingOwner)
{
	// check if this building is linkable at all and abort if it isn't
	if (!BuildingTypeExtData::IsLinkable(theBuilding->Type))
	{
		return;
	}

	short maxLinkDistance = static_cast<short>(theBuilding->Type->GuardRange / 256); // GuardRange governs how far the link can go, is saved in leptons

	for (size_t direction = 0; direction <= 7; direction += 2)
	{
		// the 4 straight directions of the simple compass
		CellStruct directionOffset = CellSpread::GetNeighbourOffset(direction); // coordinates of the neighboring cell in the given direction relative to the current cell (e.g. 0,1)
		int linkLength = 0; // how many cells to build on from center in direction to link up with a found building

		CellStruct cellToCheck = selectedCell;
		for (short distanceFromCenter = 1; distanceFromCenter <= maxLinkDistance; ++distanceFromCenter)
		{
			cellToCheck += directionOffset; // adjust the cell to check based on current distance, relative to the selected cell

			CellClass* cell = MapClass::Instance->TryGetCellAt(cellToCheck);

			if (!cell)
			{ // don't parse this cell if it doesn't exist (duh)
				break;
			}

			if (BuildingClass* OtherEnd = cell->GetBuilding())
			{ // if we find a building...
				if (FirewallFunctions::canLinkTo(theBuilding, OtherEnd))
				{ // ...and it is linkable, we found what we needed
					linkLength = distanceFromCenter - 1; // distanceFromCenter directly would be on top of the found building
					break;
				}

				break; // we found a building, but it's not linkable
			}

			if (!cell->CanThisExistHere(theBuilding->Type->SpeedType, theBuilding->Type, buildingOwner))
			{ // abort if that buildingtype is not allowed to be built there
				break;
			}
		}

		// build a line of this buildingtype from the found building (if any) to the newly built one
		CellStruct cellToBuildOn = selectedCell;
		for (int distanceFromCenter = 1; distanceFromCenter <= linkLength; ++distanceFromCenter)
		{
			cellToBuildOn += directionOffset;

			if (CellClass* cell = MapClass::Instance->GetCellAt(cellToBuildOn))
			{
				if (BuildingClass* tempBuilding = (BuildingClass*)(theBuilding->Type->CreateObject(buildingOwner)))
				{
					CoordStruct coordBuffer = CellClass::Cell2Coord(cellToBuildOn);

					++Unsorted::ScenarioInit; // put the building there even if normal rules would deny - e.g. under units
					bool Put = tempBuilding->Unlimbo(coordBuffer, DirType::North);
					--Unsorted::ScenarioInit;

					if (Put)
					{
						tempBuilding->QueueMission(Mission::Construction, false);
						tempBuilding->DiscoveredBy(buildingOwner);
						tempBuilding->IsReadyToCommence = 1;
					}
					else
					{
						Debug::Log(__FUNCTION__"Called!\n");
						TechnoExtData::HandleRemove(tempBuilding, nullptr, true, true);
					}
				}
			}
		}
	}
}

int FirewallFunctions::GetImageFrameIndex(BuildingClass* pThis)
{
	BuildingTypeExtData* pData = BuildingTypeExtContainer::Instance.Find(pThis->Type);

	if (pData->Firestorm_Wall)
	{
		return static_cast<int>(pThis->FirestormWallFrame);

		/* this is the code the game uses to calculate the firewall's frame number when you place/remove sections... should be a good base for trench frames

			int frameIdx = 0;
			CellClass *Cell = this->GetCell();
			for(int direction = 0; direction <= 7; direction += 2) {
				if(BuildingClass *B = Cell->GetNeighbourCell(direction)->GetBuilding()) {
					if(B->IsAlive && !B->InLimbo) {
						frameIdx |= (1 << (direction >> 1));
					}
				}
			}

		*/
	}

	return (pData->IsTrench >= 0) - 1;
}

#pragma endregion

#pragma region AresEMPulse

void AresEMPulse::CreateEMPulse(WarheadTypeClass* pWarhead, const CoordStruct& Target, TechnoClass* Firer)
{
	if (!pWarhead)
	{
		return;
	}

	// set of affected objects. every object can be here only once.
	auto const items = Helpers::Alex::getCellSpreadItems(
		Target, pWarhead->CellSpread, true);

	// affect each object
	for (const auto& pItem : items)
	{
		AresEMPulse::deliverEMPDamage(pItem, Firer, pWarhead);
	}
}

void AresEMPulse::Destroy(TechnoClass* pTechno, TechnoClass* pKiller, HouseClass* pKillerHouse, WarheadTypeClass* pWarhead)
{
	if (!pKillerHouse && pKiller)
	{
		pKillerHouse = pKiller->Owner;
	}

	if (!pWarhead)
	{
		pWarhead = RulesClass::Instance->C4Warhead;
	}

	int health = pTechno->GetType()->Strength;
	pTechno->ReceiveDamage(&health, 0, pWarhead, pKiller, true, false, pKillerHouse);
}

AnimTypeClass* AresEMPulse::GetSparkleAnimType(TechnoClass const* const pTechno)
{
	auto const pType = pTechno->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	return pTypeExt->EMP_Sparkles.Get(RulesClass::Instance->EMPulseSparkles);
}

void AresEMPulse::announceAttack(TechnoClass* Techno)
{
	enum class AttackEvents { None = 0, Base = 1, Harvester = 2 };
	AttackEvents Event = AttackEvents::None;

	// find out what event is the most appropriate.
	if (Techno && Techno->Owner == HouseClass::CurrentPlayer)
	{
		if (auto pBuilding = specific_cast<BuildingClass*>(Techno))
		{
			if (pBuilding->Type->ResourceGatherer)
			{
				// slave miner, for example
				Event = AttackEvents::Harvester;
			}
			else if (!pBuilding->Type->Insignificant && !pBuilding->Type->BaseNormal)
			{
				Event = AttackEvents::Base;
			}
		}
		else if (auto pUnit = specific_cast<UnitClass*>(Techno))
		{
			if (pUnit->Type->Harvester || pUnit->Type->ResourceGatherer)
			{
				Event = AttackEvents::Harvester;
			}
		}
	}

	// handle the event.
	switch (Event)
	{
	case AttackEvents::Harvester:
		if (RadarEventClass::Create(RadarEventType::HarvesterAttacked, Techno->GetMapCoords()))
			VoxClass::Play(GameStrings::EVA_OreMinerUnderAttack, -1, -1);
		break;
	case AttackEvents::Base:
		HouseClass::CurrentPlayer->BuildingUnderAttack(specific_cast<BuildingClass*>(Techno));
		break;
	case AttackEvents::None:
	default:
		break;
	}
}

void AresEMPulse::updateSpawnManager(TechnoClass* Techno, ObjectClass* Source)
{
	auto pSM = Techno->SpawnManager;

	if (!pSM)
	{
		return;
	}

	if (Techno->EMPLockRemaining > 0)
	{
		// crash all spawned units that are visible. else, they'd land somewhere else.
		for (const auto& pSpawn : pSM->SpawnedNodes)
		{
			// kill every spawned unit that is in the air. exempt missiles.
			if (pSpawn->IsSpawnMissile == FALSE && pSpawn->Unit)
			{
				auto Status = pSpawn->Status;
				if (Status >= SpawnNodeStatus::TakeOff && Status <= SpawnNodeStatus::Returning)
				{
					AresEMPulse::Destroy(pSpawn->Unit, abstract_cast<TechnoClass*>(Source), nullptr, nullptr);
				}
			}
		}

		// pause the timers so spawning and regenerating is deferred.
		pSM->SpawnTimer.Pause();
		pSM->UpdateTimer.Pause();
	}
	else
	{
		// resume counting.
		pSM->SpawnTimer.Resume();
		pSM->UpdateTimer.Resume();
	}
}

void AresEMPulse::updateRadarBlackout(BuildingClass* const pBuilding)
{
	if (pBuilding->Type->Radar)
	{
		pBuilding->Owner->RecheckRadar = true;
		return; //one of just check once
	}

	for (auto pType : pBuilding->GetTypes())
	{
		if (pType && pType->SpySat)
		{
			pBuilding->Owner->RecheckRadar = true;
			return; //one of just check once
		}
	}
}

bool AresEMPulse::IsTypeEMPProne(TechnoClass* pTechno)
{

	auto const pType = pTechno->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (!pTypeExt->ImmuneToEMP.isset())
	{
		bool TypeImmune = false;
		auto const abs = pTechno->WhatAmI();

		if (abs == AbstractType::Building)
		{
			auto const pBld = static_cast<BuildingClass const*>(pTechno);

			TypeImmune = !pBld->Type->InvisibleInGame
				&& (pBld->Type->Powered && pBld->Type->PowerDrain > 0
				 || pBld->Type->Radar
				 || pBld->Type->SuperWeapon >= 0
				 || pBld->Type->SuperWeapon2 >= 0
				 || pBld->Type->UndeploysInto
				 || pBld->Type->PowersUnit
				 || pBld->Type->Sensors
				 || pBld->Type->LaserFencePost
				 || pBld->Type->GapGenerator);
		}
		else if (abs == AbstractType::Infantry)
		{
			// affected only if this is a cyborg.
			TypeImmune = static_cast<InfantryClass const*>(pTechno)->Type->Cyborg;
		}
		else
		{
			// if this is a vessel or vehicle that is organic: no effect.
			TypeImmune = !pType->Organic;
		}

		pTypeExt->ImmuneToEMP = !TypeImmune;
	}

	return pTypeExt->ImmuneToEMP.Get();
}

bool AresEMPulse::isCurrentlyEMPImmune(WarheadTypeClass* pWarhead, TechnoClass* Target, HouseClass* SourceHouse)
{
	if (auto pBldLinked = specific_cast<BuildingClass*>(Target->BunkerLinkedItem))
	{
		if (!pWarhead->PenetratesBunker)
			return true;
	}

	// objects currently doing some time travel are exempt
	if (Target->IsBeingWarpedOut())
	{
		return true;
	}

	// iron curtained objects can not be affected by EMPs
	if (Target->IsIronCurtained())
	{
		return true;
	}

	if (Target->WhatAmI() == AbstractType::Unit)
	{
		if (BuildingClass* pBld = MapClass::Instance->GetCellAt(Target->Location)->GetBuilding())
		{
			if (pBld->Type->WeaponsFactory)
			{
				if (pBld->IsUnderEMP() || pBld == Target->GetNthLink(0))
				{
					return true;
				}

				// units requiring an operator can't deactivate on the bib
				// because nobody could enter it afterwards.
				if (!TechnoExt_ExtData::IsOperatedB(Target))
				{
					return true;
				}
			}
		}
	}

	// the current status does allow this target to
	// be affected by EMPs. It may be immune, though.
	return isEMPImmune(Target, SourceHouse);
}

bool AresEMPulse::isEMPImmune(TechnoClass* Target, HouseClass* SourceHouse)
{
	if (TechnoExtData::IsEMPImmune(Target))
		return true;

	// if houses differ, TypeImmune does not count.
	if (Target->Owner == SourceHouse)
	{
		// ignore if type immune. don't even try.
		if (AresEMPulse::isEMPTypeImmune(Target))
		{
			// This unit can fire emps and type immunity
			// grants it to never be affected.
			return true;
		}
	}

	return false;
}

bool AresEMPulse::isEMPTypeImmune(TechnoClass* Target)
{
	auto pType = Target->GetTechnoType();
	if (!pType->TypeImmune)
	{
		return false;
	}

	const int WeaponCount = pType->TurretCount <= 0 ? 2 : pType->WeaponCount;

	for (auto i = 0; i < WeaponCount; ++i)
	{

		if (auto pWeaponType = Target->GetWeapon(i)->WeaponType)
		{
			if (WarheadTypeExtContainer::Instance.Find(pWeaponType->Warhead)->EMP_Duration != 0)
			{
				// this unit can fire emps and type immunity
				// grants it to never be affected.
				return true;
			}
		}
	}

	return false;
}

bool AresEMPulse::IsDeactivationAdvisable(TechnoClass* Target)
{
	switch (Target->CurrentMission)
	{
	case Mission::Selling:
	case Mission::Construction:
		return false;
	}
	return true;
}

bool AresEMPulse::IsDeactivationAdvisableB(TechnoClass* Target)
{
	switch (Target->CurrentMission)
	{
	case Mission::Selling:
	case Mission::Construction:
		return false;
	}

	switch (Target->QueuedMission)
	{
	case Mission::Selling:
	case Mission::Construction:
		return false;
	}

	return true;
}

void AresEMPulse::UpdateSparkleAnim(TechnoClass* pFrom, TechnoClass* pTo)
{
	AnimTypeClass* pSparkle = nullptr;

	if (auto& pCurSparkle = TechnoExtContainer::Instance.Find(pFrom)->EMPSparkleAnim)
	{
		const auto pSpecific = AresEMPulse::GetSparkleAnimType(pFrom);

		if (pSpecific != pCurSparkle->Type)
			pSparkle = pSpecific;
	}

	AresEMPulse::UpdateSparkleAnim(pTo, pSparkle);
}

void AresEMPulse::UpdateSparkleAnim(TechnoClass* pWho, AnimTypeClass* pAnim)
{
	if (TechnoTypeExtContainer::Instance.Find(pWho->GetTechnoType())->IsDummy)
		return;

	auto& Anim = TechnoExtContainer::Instance.Find(pWho)->EMPSparkleAnim;

	if (pWho->IsUnderEMP())
	{
		if (!Anim)
		{
			auto const pAnimType = pAnim ? pAnim
				: AresEMPulse::GetSparkleAnimType(pWho);

			if (pAnimType)
			{
				Anim.reset(GameCreate<AnimClass>(pAnimType, pWho->Location));
				Anim->SetOwnerObject(pWho);

				if (pWho->WhatAmI() == BuildingClass::AbsID)
				{
					Anim->ZAdjust = -1024;
				}
			}
		}
	}
	else if (Anim)
	{
		// finish this loop, then disappear
		Anim->RemainingIterations = 0;
		Anim.release();
	}
}

bool AresEMPulse::thresholdExceeded(TechnoClass* Victim)
{
	auto const pData = TechnoTypeExtContainer::Instance.Find(Victim->GetTechnoType());
	if (pData->EMP_Threshold != 0 && Victim->EMPLockRemaining > (Math::abs(pData->EMP_Threshold)))
	{
		if (pData->EMP_Threshold > 0)
		{
			return true;
		}
		else
		{
			FootClass* pFoot = nullptr;
			bool InAir = Victim->IsInAir();

			if (Victim->AbstractFlags & AbstractFlags::Foot)
			{
				pFoot = (FootClass*)Victim;
			}

			return InAir && !Victim->Parachute && !Victim->IsCrashing
				&& (!pFoot || !pFoot->IsLetGoByLocomotor || !pFoot->IsAttackedByLocomotor);
		}
	}

	return false;
}

bool AresEMPulse::isEligibleEMPTarget(TechnoClass* const pTarget, HouseClass* const pSourceHouse, WarheadTypeClass* pWarhead)
{
	if (!WarheadTypeExtContainer::Instance.Find(pWarhead)->CanTargetHouse(pSourceHouse, pTarget))
		return false;

	return !AresEMPulse::isCurrentlyEMPImmune(pWarhead, pTarget, pSourceHouse);
}

void AresEMPulse::deliverEMPDamage(TechnoClass* const pTechno, TechnoClass* const pFirer, WarheadTypeClass* pWarhead)
{
	auto const pHouse = pFirer ? pFirer->Owner : nullptr;
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);

	if (AresEMPulse::isEligibleEMPTarget(pTechno, pHouse, pWarhead))
	{
		auto const pType = pTechno->GetTechnoType();
		const auto armor = TechnoExtData::GetTechnoArmor(pTechno, pWarhead);
		auto const& Verses = pWHExt->GetVerses(armor).Verses;

		if (Math::abs(Verses) < 0.001)
		{
			return;
		}

		auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);
		// get the target-specific multiplier
		auto modifier = pWHExt->EMP_Duration > 0 ? pExt->EMP_Modifier : 1.0;

		// respect verses

		auto duration = static_cast<int>(pWHExt->EMP_Duration * modifier);


		// get the new capped value
		auto const oldValue = static_cast<int>(pTechno->EMPLockRemaining);
		auto const newValue = Helpers::Alex::getCappedDuration(
			oldValue, duration, pWHExt->EMP_Cap);

		// can not be less than zero
		pTechno->EMPLockRemaining = (MaxImpl(newValue, 0));

		auto diedFromPulse = false;
		auto const underEMPBefore = (oldValue > 0);
		auto const underEMPAfter = (pTechno->EMPLockRemaining > 0);
		auto const newlyUnderEMP = !underEMPBefore && underEMPAfter;

		if (underEMPBefore && !underEMPAfter)
		{
			// newly de-paralyzed
			AresEMPulse::DisableEMPEffect(pTechno);

			if (auto v19 = pTechno->AttachedTag)
			{
				v19->RaiseEvent(TriggerEvent(AresTriggerEvents::RemoveEMP_ByHouse), pTechno, CellStruct::Empty, 0, pFirer);
			}

			if (pTechno->IsAlive)
			{
				if (auto  v20 = pTechno->AttachedTag)
				{
					v20->RaiseEvent(TriggerEvent(AresTriggerEvents::RemoveEMP), pTechno, CellStruct::Empty, 0, 0);
				}
			}

		}
		else if (newlyUnderEMP)
		{
			// newly paralyzed unit
			diedFromPulse = AresEMPulse::EnableEMPEffect(pTechno, pFirer);

			if (!diedFromPulse && pWHExt->Malicious)
			{
				// warn the player
				AresEMPulse::announceAttack(pTechno);
			}

			if (pTechno->IsAlive)
			{
				if (auto v19 = pTechno->AttachedTag)
				{
					v19->RaiseEvent(TriggerEvent(AresTriggerEvents::UnderEMP_ByHouse), pTechno, CellStruct::Empty, 0, pFirer);
				}

				if (pTechno->IsAlive)
				{
					if (auto  v20 = pTechno->AttachedTag)
					{
						v20->RaiseEvent(TriggerEvent(AresTriggerEvents::UnderEMP), pTechno, CellStruct::Empty, 0, nullptr);
					}
				}
			}

		}
		else if (oldValue == newValue)
		{
			// no relevant change
			return;
		}

		// is techno destroyed by EMP?
		if (diedFromPulse || (underEMPAfter && AresEMPulse::thresholdExceeded(pTechno)))
		{
			AresEMPulse::Destroy(pTechno, pFirer, nullptr, nullptr);
		}
		else if (newlyUnderEMP || pWHExt->EMP_Sparkles)
		{
			// set the sparkle animation
			AresEMPulse::UpdateSparkleAnim(pTechno, pWHExt->EMP_Sparkles);
		}
	}
}

bool AresEMPulse::EnableEMPEffect(TechnoClass* const pVictim, ObjectClass* const pSource)
{
	auto const abs = pVictim->WhatAmI();

	if (abs == AbstractType::Building)
	{
		auto const pBuilding = static_cast<BuildingClass*>(pVictim);
		auto const pOwner = pBuilding->Owner;

		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;

		pBuilding->DisableStuff();
		AresEMPulse::updateRadarBlackout(pBuilding);
		pBuilding->NeedsRedraw = true;
	}
	else if (abs == AbstractType::Aircraft)
	{
		// crash flying aircraft
		auto const pAircraft = static_cast<AircraftClass*>(pVictim);
		if (pAircraft->GetHeight() > 0 && !pAircraft->IsLetGoByLocomotor && !pAircraft->IsAttackedByLocomotor)
		{
			return true;
		}
	}

	// cache the last mission this thing did
	TechnoExtContainer::Instance.Find(pVictim)->EMPLastMission = pVictim->CurrentMission;

	// detach temporal
	if (pVictim->IsWarpingSomethingOut())
	{
		pVictim->TemporalImUsing->LetGo();
	}

	// remove the unit from its team
	if (auto const pFoot = abstract_cast<FootClass*>(pVictim))
	{
		if (pFoot->LocomotorTarget)
			pFoot->LocomotorImblued(true);

		if (pFoot->BelongsToATeam())
		{
			pFoot->Team->LiberateMember(pFoot);
		}
	}

	// deactivate and sparkle
	if (!pVictim->Deactivated && AresEMPulse::IsDeactivationAdvisable(pVictim))
	{
		auto const selected = pVictim->IsSelected;
		auto const pFocus = pVictim->ArchiveTarget;

		pVictim->Deactivate();

		if (selected)
		{
			auto const feedback = Unsorted::MoveFeedback();
			Unsorted::MoveFeedback() = false;
			pVictim->Select();
			Unsorted::MoveFeedback() = feedback;
		}

		if (abs == AbstractType::Building)
		{
			pVictim->ArchiveTarget = pFocus;
		}
	}

	// release all captured units.
	if (pVictim->CaptureManager)
	{
		pVictim->CaptureManager->FreeAll();
	}

	// update managers.
	AresEMPulse::updateSpawnManager(pVictim, pSource);

	if (auto const pSlaveManager = pVictim->SlaveManager)
	{
		pSlaveManager->SuspendWork();
	}

	// the unit still lives.
	return false;
}

void AresEMPulse::DisableEMPEffect(TechnoClass* const pVictim)
{
	auto const abs = pVictim->WhatAmI();

	auto hasPower = true;

	if (abs == AbstractType::Building)
	{
		auto const pBuilding = static_cast<BuildingClass*>(pVictim);
		hasPower = pBuilding->IsPowerOnline();

		auto const pOwner = pBuilding->Owner;
		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;

		auto const pType = pBuilding->Type;
		if (hasPower || pType->LaserFencePost)
		{
			pBuilding->EnableStuff();
		}
		AresEMPulse::updateRadarBlackout(pBuilding);
		pBuilding->NeedsRedraw = true;
	}

	if (hasPower && pVictim->Deactivated)
	{
		auto const pFocus = pVictim->ArchiveTarget;
		pVictim->Reactivate();
		if (abs == AbstractType::Building)
		{
			pVictim->ArchiveTarget = pFocus;
		}
	}

	// allow to spawn units again.
	AresEMPulse::updateSpawnManager(pVictim);

	if (auto const pSlaveManager = pVictim->SlaveManager)
	{
		pSlaveManager->ResumeWork();
	}

	// update the animation
	AresEMPulse::UpdateSparkleAnim(pVictim);

	// get harvesters back to work and ai units to hunt
	if (auto const pFoot = abstract_cast<FootClass*>(pVictim))
	{
		auto hasMission = false;
		if (abs == AbstractType::Unit)
		{
			auto const pUnit = static_cast<UnitClass*>(pVictim);
			if (pUnit->Type->Harvester || pUnit->Type->ResourceGatherer)
			{
				// prevent unloading harvesters from being irritated.
				auto const mission = TechnoExtContainer::Instance.Find(pVictim)->EMPLastMission != Mission::Guard
					? TechnoExtContainer::Instance.Find(pVictim)->EMPLastMission : Mission::Enter;

				pUnit->QueueMission(mission, true);
				hasMission = true;
			}
		}

		if (!hasMission && !pFoot->Owner->IsControlledByHuman())
		{
			pFoot->QueueMission(RulesExtData::Instance()->EMPAIRecoverMission.Get(Mission::Hunt), false);
		}
	}
}

bool AresEMPulse::EnableEMPEffect2(TechnoClass* const pVictim)
{
	auto const abs = pVictim->WhatAmI();

	if (abs == AbstractType::Building)
	{
		auto const pBuilding = static_cast<BuildingClass*>(pVictim);
		auto const pOwner = pBuilding->Owner;

		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;

		pBuilding->DisableStuff();
		AresEMPulse::updateRadarBlackout(pBuilding);
		pBuilding->NeedsRedraw = true;
	}
	else if (abs == AbstractType::Aircraft)
	{
		// crash flying aircraft
		auto const pAircraft = static_cast<AircraftClass*>(pVictim);
		if (pAircraft->GetHeight() > 0 && !pAircraft->IsLetGoByLocomotor && !pAircraft->IsAttackedByLocomotor)
		{
			return true;
		}
	}

	// deactivate and sparkle
	if (!pVictim->Deactivated && AresEMPulse::IsDeactivationAdvisable(pVictim))
	{
		// cache the last mission this thing did
		TechnoExtContainer::Instance.Find(pVictim)->EMPLastMission = pVictim->CurrentMission;

		// detach temporal
		if (pVictim->IsWarpingSomethingOut())
		{
			pVictim->TemporalImUsing->LetGo();
		}

		// remove the unit from its team
		if (auto const pFoot = abstract_cast<FootClass*>(pVictim))
		{
			if (pFoot->LocomotorTarget)
				pFoot->LocomotorImblued(true);

			if (pFoot->BelongsToATeam())
			{
				pFoot->Team->LiberateMember(pFoot);
			}
		}

		auto const selected = pVictim->IsSelected;
		auto const pFocus = pVictim->ArchiveTarget;

		pVictim->Deactivate();

		if (selected)
		{
			auto const feedback = Unsorted::MoveFeedback();
			Unsorted::MoveFeedback() = false;
			pVictim->Select();
			Unsorted::MoveFeedback() = feedback;
		}

		if (abs == AbstractType::Building)
		{
			pVictim->ArchiveTarget = pFocus;
		}
		else
		{
			pVictim->QueueMission(Mission::Sleep, true);
		}

		// release all captured units.
		if (pVictim->CaptureManager)
		{
			pVictim->CaptureManager->FreeAll();
		}

		// update managers.
		AresEMPulse::updateSpawnManager(pVictim, nullptr);

		if (auto const pSlaveManager = pVictim->SlaveManager)
		{
			pSlaveManager->SuspendWork();
		}
	}

	// the unit still lives.
	return false;
}

void AresEMPulse::DisableEMPEffect2(TechnoClass* const pVictim)
{
	auto const abs = pVictim->WhatAmI();

	auto hasPower = TechnoExt_ExtData::IsPowered(pVictim) && TechnoExt_ExtData::IsOperated(pVictim);

	if (abs == AbstractType::Building)
	{
		auto const pBuilding = static_cast<BuildingClass*>(pVictim);
		hasPower = hasPower && pBuilding->IsPowerOnline();

		auto const pOwner = pBuilding->Owner;
		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;

		if (hasPower)
		{
			pBuilding->EnableStuff();
		}
		AresEMPulse::updateRadarBlackout(pBuilding);
		pBuilding->NeedsRedraw = true;
	}

	if (hasPower && pVictim->Deactivated)
	{
		auto const pFocus = pVictim->ArchiveTarget;
		pVictim->Reactivate();
		if (abs == AbstractType::Building)
		{
			pVictim->ArchiveTarget = pFocus;
		}

		// allow to spawn units again.
		AresEMPulse::updateSpawnManager(pVictim);

		if (auto const pSlaveManager = pVictim->SlaveManager)
		{
			pSlaveManager->ResumeWork();
		}

		// get harvesters back to work and ai units to hunt
		if (auto const pFoot = abstract_cast<FootClass*>(pVictim))
		{
			auto hasMission = false;
			if (abs == AbstractType::Unit)
			{
				auto const pUnit = static_cast<UnitClass*>(pVictim);
				if (pUnit->Type->Harvester || pUnit->Type->ResourceGatherer)
				{
					// prevent unloading harvesters from being irritated.
					auto const mission = TechnoExtContainer::Instance.Find(pVictim)->EMPLastMission != Mission::Guard
						? TechnoExtContainer::Instance.Find(pVictim)->EMPLastMission : Mission::Enter;

					pUnit->QueueMission(mission, true);
					hasMission = true;
				}
			}

			if (!hasMission && !pFoot->Owner->IsControlledByHuman())
			{
				pFoot->QueueMission(RulesExtData::Instance()->EMPAIRecoverMission.Get(Mission::Hunt), false);
			}
		}
	}
}
#pragma endregion

#pragma region AresPoweredUnit

bool AresPoweredUnit::IsPoweredBy(HouseClass* const pOwner) const
{
	auto const pType = this->Techno->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	auto const& PoweredBy = pTypeExt->PoweredBy;

	for (auto const& pBuilding : pOwner->Buildings)
	{
		auto const inArray = PoweredBy.Contains(pBuilding->Type);

		if (inArray && !pBuilding->BeingWarpedOut && !pBuilding->IsUnderEMP())
		{
			if (TechnoExt_ExtData::IsOperated(pBuilding) && pBuilding->IsPowerOnline())
			{
				return true;
			}
		}
	}

	return false;
}

void AresPoweredUnit::PowerUp()
{
	auto const pTechno = this->Techno;
	if (!pTechno->IsUnderEMP() && TechnoExt_ExtData::IsOperated(pTechno))
	{
		AresEMPulse::DisableEMPEffect2(pTechno);
	}
}

bool AresPoweredUnit::PowerDown()
{
	auto const pTechno = this->Techno;

	if (AresEMPulse::IsDeactivationAdvisableB(pTechno))
	{
		// destroy if EMP.Threshold would crash this unit when in air
		if (AresEMPulse::EnableEMPEffect2(pTechno)
			|| (TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType())->EMP_Threshold
				&& pTechno->IsInAir()))
		{
			return false;
		}
	}

	return true;
}

bool AresPoweredUnit::Update()
{
	if ((Unsorted::CurrentFrame - this->LastScan) < ScanInterval)
	{
		return true;
	}

	auto const pTechno = this->Techno;

	if (!pTechno->IsAlive || !pTechno->Health || pTechno->InLimbo)
	{
		return true;
	}

	const auto curMission = pTechno->CurrentMission;
	this->LastScan = Unsorted::CurrentFrame;

	if (curMission == Mission::Selling || curMission == Mission::Construction)
		return true;

	const auto queueMission = pTechno->QueuedMission;

	if (queueMission == Mission::Selling || queueMission == Mission::Construction)
		return true;


	auto const pOwner = pTechno->Owner;
	auto const hasPower = this->IsPoweredBy(pOwner);

	this->Powered = hasPower;

	if (hasPower && pTechno->Deactivated)
	{
		this->PowerUp();
	}
	else if (!hasPower && !pTechno->Deactivated)
	{
		// don't shutdown units inside buildings (warfac, barracks, shipyard) because that locks up the factory and the robot tank did it
		auto const whatAmI = pTechno->WhatAmI();
		if ((whatAmI != InfantryClass::AbsID && whatAmI != UnitClass::AbsID) || (!pTechno->GetCell()->GetBuilding()))
		{
			return this->PowerDown();
		}
	}

	return true;
}

#pragma endregion

#pragma region AresJammer
//! \param TargetBuilding The building whose eligibility to check.
bool AresJammer::IsEligible(BuildingClass* TargetBuilding)
{
	/* Current requirements for being eligible:
		- not an ally (includes ourselves)
		- either a radar or a spysat
	*/

	if (!this->AttachedToObject->Owner->IsAlliedWith(TargetBuilding->Owner))
	{
		if (TargetBuilding->Type->Radar)
			return true;

		for (auto pType : TargetBuilding->GetTypes())
		{
			if (pType && pType->SpySat)
			{
				return true;
			}
		}
	}

	return false;
}

void AresJammer::Update()
{
	// we don't want to scan & crunch numbers every frame - this limits it to ScanInterval frames
	if ((Unsorted::CurrentFrame - this->LastScan) < this->ScanInterval)
	{
		return;
	}

	// save the current frame for future reference
	this->LastScan = Unsorted::CurrentFrame;

	// walk through all buildings
	for (auto const curBuilding : *BuildingClass::Array)
	{
		if (!AresJammer::IsEligible(curBuilding))
			continue;

		// for each jammable building ...
		// ...check if it's in range, and jam or unjam based on that
		if (this->InRangeOf(curBuilding))
		{
			this->Jam(curBuilding);
		}
		else
		{
			this->Unjam(curBuilding);
		}
	}
}

//! \param TargetBuilding The building to check the distance to.
bool AresJammer::InRangeOf(BuildingClass* TargetBuilding)
{
	auto const pExt = TechnoTypeExtContainer::Instance.Find(this->AttachedToObject->GetTechnoType());
	auto const& JammerLocation = this->AttachedToObject->Location;
	auto const JamRadiusInLeptons = 256.0 * pExt->RadarJamRadius;

	return TargetBuilding->Location.DistanceFrom(JammerLocation) <= JamRadiusInLeptons;
}

//! \param TargetBuilding The building to jam.
void AresJammer::Jam(BuildingClass* TargetBuilding)
{
	//keep item unique
	auto& jammMap = BuildingExtContainer::Instance.Find(TargetBuilding)->RegisteredJammers;

	jammMap.push_back_unique(this->AttachedToObject);

	if (jammMap.size() == 1)
	{
		TargetBuilding->Owner->RecheckRadar = true;
	}

	this->Registered = true;
}

//! \param TargetBuilding The building to unjam.
void AresJammer::Unjam(BuildingClass* TargetBuilding) const
{
	//keep item unique
	auto& jammMap = BuildingExtContainer::Instance.Find(TargetBuilding)->RegisteredJammers;
	jammMap.remove(this->AttachedToObject);

	if (jammMap.empty()) {
		TargetBuilding->Owner->RecheckRadar = true;
	}
}

void AresJammer::UnjamAll()
{
	if (this->Registered)
	{
		this->Registered = false;
		for (auto const item : *BuildingClass::Array)
		{
			this->Unjam(item);
		}
	}
}

#pragma endregion

#pragma region AresScriptExt

static std::array<const char*, 4> Move_to_own_building_SearchType { {
	"least threat" , "highest threat" , "least nearest" , "least farthest"
} };

bool AresScriptExt::Handle(TeamClass* pTeam, ScriptActionNode* pTeamMission, bool bThirdArd)
{
	//if(pTeamMission->Action == TeamMissionType::Move_to_own_building) {
	//	uint16 hi = (pTeamMission->Argument >> 0x10) & 0xFFFF;
	//	uint16 lo = pTeamMission->Argument & 0xFFFF;
	//	const char* FindType = "Dunno";
	//	if(hi < Move_to_own_building_SearchType.size())
	//		FindType = Move_to_own_building_SearchType[hi];
	//
	//	Debug::Log("Team[%x - %s] Executing MoveToOwnBuilding (%d)[hi %d(%s) , Lo %d(BldTypeMax %d)] \n" ,
	//		pTeam , pTeam->get_ID() ,pTeamMission->Argument, hi , FindType , lo , BuildingTypeClass::Array->Count );
	//
	//}else
	//	Debug::Log("Team[%x - %s] Executing [(Action)%d - (Argument)%d] \n" , pTeam , pTeam->get_ID() , pTeamMission->Action , pTeamMission->Argument);

	switch (pTeamMission->Action)
	{
	case TeamMissionType::Garrison_building:
	{
		FootClass* pCur = nullptr;
		if (auto pFirst = pTeam->FirstUnit)
		{
			auto pNext = pFirst->NextTeamMember;
			do
			{
				TechnoExtContainer::Instance.Find(pFirst)->TakeVehicleMode = false;

				if (pFirst->GarrisonStructure())
					pTeam->RemoveMember(pFirst, -1, 1);

				pCur = pNext;

				if (pNext)
					pNext = pNext->NextTeamMember;

				pFirst = pCur;

			}
			while (pCur);
		}

		pTeam->StepCompleted = true;
		return true;
	}

	case TeamMissionType::Move_to_own_building:
	case TeamMissionType::Attack_enemy_building:
	case TeamMissionType::Chrono_prep_for_abwp:
	{
		const uint16 lo = pTeamMission->Argument & 0xFFFF;

		if (lo > BuildingTypeClass::Array->Count)
		{
			Debug::FatalError("Team[%x - %s] Executing %d but the BuildingType Index is too big(%d of %d) !\n",
				pTeam, pTeam->get_ID(), pTeamMission->Action, lo, BuildingTypeClass::Array->Count);
		}
	}break;
	default:
		break;
	}

	if (pTeamMission->Action >= TeamMissionType::count)
	{
		switch ((AresScripts)pTeamMission->Action)
		{
		case AresScripts::AuxilarryPower:
		{
			HouseExtContainer::Instance.Find(pTeam->Owner)->AuxPower += pTeamMission->Argument;
			pTeam->Owner->RecheckPower = true;
			pTeam->StepCompleted = true;
			return true;
		}
		case AresScripts::KillDrivers:
		{
			const auto pToHouse = HouseExtData::FindSpecial();
			FootClass* pCur = nullptr;
			if (auto pFirst = pTeam->FirstUnit)
			{
				auto pNext = pFirst->NextTeamMember;
				do
				{
					if (pFirst->Health > 0 && pFirst->IsAlive && pFirst->IsOnMap && !pFirst->InLimbo)
					{
						if (!TechnoExtContainer::Instance.Find(pFirst)->Is_DriverKilled
							&& TechnoExt_ExtData::IsDriverKillable(pFirst, 1.0))
						{
							TechnoExt_ExtData::ApplyKillDriver(pFirst, nullptr, pToHouse, false, Mission::Harmless);
						}
					}

					pCur = pNext;

					if (pNext)
						pNext = pNext->NextTeamMember;

					pFirst = pCur;

				}
				while (pCur);
			}

			pTeam->StepCompleted = true;
			return true;
		}
		case AresScripts::TakeVehicles:
		{
			FootClass* pCur = nullptr;
			if (auto pFirst = pTeam->FirstUnit)
			{
				auto pNext = pFirst->NextTeamMember;
				do
				{
					TechnoExtContainer::Instance.Find(pFirst)->TakeVehicleMode = true;

					if (pFirst->GarrisonStructure())
						pTeam->RemoveMember(pFirst, -1, 1);

					pCur = pNext;

					if (pNext)
						pNext = pNext->NextTeamMember;

					pFirst = pCur;

				}
				while (pCur);
			}

			pTeam->StepCompleted = true;
			return true;
		}
		case AresScripts::ConvertType:
		{
			FootClass* pCur = nullptr;
			if (auto pFirst = pTeam->FirstUnit)
			{
				auto pNext = pFirst->NextTeamMember;
				do
				{
					const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pFirst->GetTechnoType());
					if (pTypeExt->Convert_Script)
					{
						const auto& pConvertReq = pTypeExt->Convert_Scipt_Prereq;
						if (pConvertReq.empty() || Prereqs::HouseOwnsAll(pTeam->Owner, (int*)pConvertReq.data(), (int)pConvertReq.size()))
						{
							TechnoExt_ExtData::ConvertToType(pFirst, pTypeExt->Convert_Script);
						}
					}

					pCur = pNext;

					if (pNext)
						pNext = pNext->NextTeamMember;

					pFirst = pCur;

				}
				while (pCur);
			}

			pTeam->StepCompleted = true;
			return true;
		}
		case AresScripts::SonarReveal:
		{
			const auto nDur = pTeamMission->Argument;
			for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			{
				auto& nSonarTime = TechnoExtContainer::Instance.Find(pUnit)->CloakSkipTimer;
				if (nDur > nSonarTime.GetTimeLeft())
				{
					nSonarTime.Start(nDur);
				}
				else if (nDur <= 0)
				{
					if (nDur == 0)
					{
						nSonarTime.Stop();
					}
				}
			}

			pTeam->StepCompleted = true;
			return true;
		}
		case AresScripts::DisableWeapons:
		{
			const auto nDur = pTeamMission->Argument;
			for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			{
				auto& nTimer = TechnoExtContainer::Instance.Find(pUnit)->DisableWeaponTimer;
				if (nDur > nTimer.GetTimeLeft())
				{
					nTimer.Start(nDur);
				}
				else if (nDur <= 0 && nDur == 0)
				{
					nTimer.Stop();
				}
			}

			pTeam->StepCompleted = true;
			return true;
		}
		default:
			break;
		}
	}

	return false;
}

#pragma endregion

#pragma region AresWPWHExt

bool AresWPWHExt::conductAbduction(WeaponTypeClass* pWeapon, TechnoClass* pOwner, AbstractClass* pTarget, CoordStruct nTargetCoords)
{

	const auto pData = WeaponTypeExtContainer::Instance.Find(pWeapon);

	// ensuring a few base parameters
	if (!pData->Abductor || !pOwner || !pTarget)
	{
		return false;
	}

	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pData->AttachedToObject->Warhead);
	const auto Target = abstract_cast<FootClass*>(pTarget);

	if (!Target)
	{
		// the target was not a valid passenger type
		return false;
	}

	if (nTargetCoords == CoordStruct::Empty)
		nTargetCoords = pTarget->GetCoords();

	const auto Attacker = pOwner;
	const auto pTargetType = Target->GetTechnoType();
	const auto AttackerType = Attacker->GetTechnoType();

	if (!pWHExt->CanAffectHouse(Attacker->Owner, Target->GetOwningHouse()))
	{
		return false;
	}

	if (!TechnoExtData::IsAbductable(Attacker, pData->AttachedToObject, Target))
	{
		return false;
	}

	//if it's owner meant to be changed, do it here
	HouseClass* pDesiredOwner = Attacker->Owner ? Attacker->Owner : HouseExtData::FindSpecial();

	//if it's owner meant to be changed, do it here
	if ((pData->Abductor_ChangeOwner && !TechnoExtData::IsPsionicsImmune(Target)))
		Target->SetOwningHouse(pDesiredOwner);

	// if we ended up here, the target is of the right type, and the attacker can take it
	// so we abduct the target...
	Target->EnterIdleMode(true, 0);
	Target->StopMoving();
	Target->SetDestination(nullptr, true); // Target->UpdatePosition(int) ?
	Target->SetTarget(nullptr);
	Target->CurrentTargets.Clear(); // Target->ShouldLoseTargetNow ?
	Target->SetArchiveTarget(nullptr);
	Target->QueueMission(Mission::Sleep, true);
	Target->unknown_C4 = 0; // don't ask
	Target->unknown_5A0 = 0;
	Target->CurrentGattlingStage = 0;
	Target->SetCurrentWeaponStage(0);

	// the team should not wait for me
	if (Target->BelongsToATeam())
	{
		Target->Team->LiberateMember(Target);
	}

	// if this unit is being mind controlled, break the link
	if (const auto MindController = Target->MindControlledBy)
	{
		if (const auto MC = MindController->CaptureManager)
		{
			MC->FreeUnit(Target);
		}
	}

	// if this unit is a mind controller, break the link
	if (Target->CaptureManager)
	{
		Target->CaptureManager->FreeAll();
	}

	// if this unit is currently in a state of temporal flux, get it back to our time-frame
	if (Target->TemporalTargetingMe)
	{
		Target->TemporalTargetingMe->Detach();
	}

	//if the target is spawned, detach it from it's spawner
	Target->DetachSpecificSpawnee(pDesiredOwner);

	// if the unit is a spawner, kill the spawns
	if (Target->SpawnManager)
	{
		Target->SpawnManager->KillNodes();
		Target->SpawnManager->ResetTarget();
	}

	//if the unit is a slave, it should be freed
	Target->FreeSpecificSlave(pDesiredOwner);

	// If the unit is a SlaveManager, free the slaves
	if (auto pSlaveManager = Target->SlaveManager)
	{
		pSlaveManager->Killed(Attacker);
		pSlaveManager->ZeroOutSlaves();
		Target->SlaveManager->Owner = Target;
	}

	// if we have an abducting animation, play it
	if (auto pAnimType = pData->Abductor_AnimType)
	{
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, nTargetCoords),
			Attacker->Owner,
			Target->Owner,
			Attacker,
			false
		);
	}

	//Target->Locomotor.GetInterfacePtr()->Force_Track(-1, CoordStruct::Empty);
	//CoordStruct coordsUnitSource = Target->GetCoords();
	//Target->Locomotor.GetInterfacePtr()->Mark_All_Occupation_Bits(0);
	//Target->MarkAllOccupationBits(coordsUnitSource);

	Target->ClearPlanningTokens(nullptr);
	Target->Flashing.DurationRemaining = 0;

	if (!Target->Limbo())
	{
		Debug::FatalError("Abduction: Target unit %p (%s) could not be removed.\n", Target, Target->get_ID());
		return false;
	}

	// because we are throwing away the locomotor in a split second, piggybacking
	// has to be stopped. otherwise the object might remain in a weird state.
	while (LocomotionClass::End_Piggyback(Target->Locomotor)) { };

	// throw away the current locomotor and instantiate
	// a new one of the default type for this unit.
	if (auto NewLoco = LocomotionClass::CreateInstance(pTargetType->Locomotor)) {
		Target->Locomotor = std::move(NewLoco);
		Target->Locomotor->Link_To_Object(Target);
	}

	//Target->AnnounceExpiredPointer(false);
	Target->OnBridge = false; // ????
	Target->NextObject = 0; // ??
	//Target->UpdatePlacement(PlacementType::Remove);

	// handling for Locomotor weapons: since we took this unit from the Magnetron
	// in an unfriendly way, set these fields here to unblock the unit
	if (Target->IsAttackedByLocomotor || Target->IsLetGoByLocomotor)
	{
		Target->IsAttackedByLocomotor = false;
		Target->IsLetGoByLocomotor = false;
	}

	Target->Transporter = Attacker;
	if (AttackerType->OpenTopped && Target->Owner->IsAlliedWith(Attacker))
	{
		Attacker->EnteredOpenTopped(Target);
	}

	if (Attacker->WhatAmI() == BuildingClass::AbsID)
	{
		Target->Absorbed = true;
	}

	Attacker->AddPassenger(Target);
	Attacker->Undiscover();

	if (auto v29 = Target->AttachedTag)
		v29->RaiseEvent(TriggerEvent(AresTriggerEvents::Abducted_ByHouse), Target, CellStruct::Empty, false, Attacker);

	if (Target->IsAlive)
	{
		if (auto v30 = Target->AttachedTag)
			v30->RaiseEvent(TriggerEvent(AresTriggerEvents::Abducted), Target, CellStruct::Empty, false, nullptr);
	}

	if (auto v31 = Attacker->AttachedTag)
		v31->RaiseEvent(TriggerEvent(AresTriggerEvents::AbductSomething_OfHouse), Attacker, CellStruct::Empty, false, Target->GetOwningHouse());// pTarget->Owner

	if (Attacker->IsAlive)
	{
		if (auto v32 = Attacker->AttachedTag)
			v32->RaiseEvent(TriggerEvent(AresTriggerEvents::AbductSomething), Attacker, CellStruct::Empty, false, nullptr);
	}

	return true;
}

bool AresWPWHExt::applyOccupantDamage(BulletClass* pThis)
{
	auto const pBuilding = specific_cast<BuildingClass*>(pThis->Target);

	// if that pointer is null, something went wrong
	if (!pBuilding)
	{
		return false;
	}

	auto const pTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Type);
	auto const pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

	auto const occupants = pBuilding->Occupants.Count;
	auto const& passThrough = pBldTypeExt->UCPassThrough;

	if (!occupants || !passThrough)
	{
		return false;
	}

	auto& Random = ScenarioClass::Instance->Random;
	if (pTypeExt->SubjectToTrenches && Random.RandomDouble() >= passThrough)
	{
		return false;
	}

	auto const idxPoorBastard = Random.RandomFromMax(occupants - 1);
	auto const pPoorBastard = pBuilding->Occupants[idxPoorBastard];
	auto const& fatalRate = pBldTypeExt->UCFatalRate;

	if (fatalRate > 0.0 && Random.RandomDouble() < fatalRate)
	{
		pPoorBastard->Destroyed(pThis->Owner);
		pPoorBastard->UnInit();
		pBuilding->Occupants.RemoveAt<true>(idxPoorBastard);
		pBuilding->UpdateThreatInCell(pBuilding->GetCell());
	}
	else
	{
		auto const& multiplier = pBldTypeExt->UCDamageMultiplier.Get();
		auto adjustedDamage = static_cast<int>(std::ceil(pThis->Health * multiplier));
		pPoorBastard->ReceiveDamage(&adjustedDamage, 0, pThis->WH, pThis->Owner, false, true, pThis->GetOwningHouse());
	}

	if (pBuilding->FiringOccupantIndex >= pBuilding->GetOccupantCount())
	{
		pBuilding->FiringOccupantIndex = 0;
	}

	// if the last occupant was killed and this building was raided,
	// it needs to be returned to its owner. (Bug #700)
	TechnoExt_ExtData::EvalRaidStatus(pBuilding);

	return true;
}

void AresWPWHExt::applyKillDriver(WarheadTypeClass* pWH, TechnoClass* pKiller, TechnoClass* pVictim)
{
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (!pKiller || !pWHExt->KillDriver || !pVictim)
		return;

	if (!pWHExt->CanAffectHouse(pKiller->Owner, pVictim->Owner))
		return;

	if (!TechnoExt_ExtData::IsDriverKillable(pVictim, pWHExt->KillDriver_KillBelowPercent))
		return;

	if (ScenarioClass::Instance->Random.RandomDouble() <= pWHExt->KillDriver_Chance)
	{
		HouseClass* Owner = HouseExtData::GetHouseKind(pWHExt->KillDriver_Owner, false, nullptr, pKiller->Owner, pVictim->Owner);
		if (!Owner)
			Owner = HouseExtData::FindSpecial();

		TechnoExt_ExtData::ApplyKillDriver(pVictim, pKiller, Owner, pWHExt->KillDriver_ResetVeterancy, Mission::Harmless);
	}
}
#pragma endregion

#pragma region AresTActionExt

std::pair<TriggerAttachType, bool> AresTActionExt::GetFlag(AresNewTriggerAction nAction)
{
	switch (nAction)
	{
	case AresNewTriggerAction::AuxiliaryPower:
	case AresNewTriggerAction::SetEVAVoice:
		return { TriggerAttachType::None , true };
	case AresNewTriggerAction::KillDriversOf:
	case AresNewTriggerAction::SetGroup:
		return { TriggerAttachType::Object , true };
	default:
		return { TriggerAttachType::None , false };
	}
}

std::pair<LogicNeedType, bool> AresTActionExt::GetMode(AresNewTriggerAction nAction)
{
	switch (nAction)
	{
	case AresNewTriggerAction::AuxiliaryPower:
		return { LogicNeedType::NumberNSuper  , true };
	case AresNewTriggerAction::KillDriversOf:
		return { LogicNeedType::None , true };
	case AresNewTriggerAction::SetEVAVoice:
	case AresNewTriggerAction::SetGroup:
		return { LogicNeedType::Number, true };
	default:
		return { LogicNeedType::None , false };
	}
}

bool AresTActionExt::ActivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (pHouse->FirestormActive)
	{
		AresHouseExt::SetFirestormState(pHouse, true);
	}

	return true;
}

bool AresTActionExt::DeactivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (pHouse->FirestormActive)
	{
		AresHouseExt::SetFirestormState(pHouse, false);
	}
	return true;
}

bool AresTActionExt::AuxiliaryPower(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto pDecidedHouse = pAction->FindHouseByIndex(pTrigger, pAction->Value);

	if (!pDecidedHouse)
		return false;

	HouseExtContainer::Instance.Find(pDecidedHouse)->AuxPower += pAction->Value2;
	pDecidedHouse->RecheckPower = true;
	return true;
}

bool AresTActionExt::KillDriversOf(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto pDecidedHouse = pAction->FindHouseByIndex(pTrigger, pAction->Value);
	if (!pDecidedHouse)
		pDecidedHouse = HouseExtData::FindSpecial();

	for (auto pUnit : *FootClass::Array)
	{
		if (pUnit->Health > 0 && pUnit->IsAlive && pUnit->IsOnMap && !pUnit->InLimbo)
		{
			if (pUnit->AttachedTag && pUnit->AttachedTag->ContainsTrigger(pTrigger))
			{
				if (!TechnoExtContainer::Instance.Find(pUnit)->Is_DriverKilled
					&& TechnoExt_ExtData::IsDriverKillable(pUnit, 1.0))
				{
					TechnoExt_ExtData::ApplyKillDriver(pUnit, nullptr, pDecidedHouse, false, Mission::Harmless);
				}
			}
		}
	}

	return true;
}

bool AresTActionExt::SetEVAVoice(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (pAction->Value >= (int)EVAVoices::Types.size())
	{
		return false;
	}

	VoxClass::EVAIndex = MaxImpl(pAction->Value, -1);
	return true;
}

bool AresTActionExt::SetGroup(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (auto pTech = generic_cast<TechnoClass*>(pObject))
	{
		pTech->Group = pAction->Value;
		return true;
	}

	return false;
}

//TODO : re-eval
bool AresTActionExt::LauchhNuke(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto pFind = WeaponTypeClass::Find(GameStrings::NukePayload);
	if (!pFind)
		return false;

	const auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);
	auto nCoord = CellClass::Cell2Coord(nLoc);
	nCoord.Z = MapClass::Instance->GetCellFloorHeight(nCoord);

	if (MapClass::Instance->GetCellAt(nCoord)->ContainsBridge())
		nCoord.Z += Unsorted::BridgeHeight;

	SW_NuclearMissile::DropNukeAt(nullptr, nCoord, nullptr, pHouse, pFind);

	//if (auto pBullet = pFind->Projectile->CreateBullet(MapClass::Instance->GetCellAt(nCoord), nullptr, pFind->Damage, pFind->Warhead, 50, false))
	//{
	//	pBullet->SetWeaponType(pFind);
	//	VelocityClass nVel {};
	//
	//	double nSin = Math::sin(1.570748388432313);
	//	double nCos = Math::cos(1.570748388432313);
	//
	//	double nX = nCos * nCos * -100.0;
	//	double nY = nCos * nSin * -100.0;
	//	double nZ = nSin * -100.0;
	//
	//	BulletExtContainer::Instance.Find(pBullet)->Owner = pHouse;
	//	pBullet->MoveTo({ nCoord.X , nCoord.Y , nCoord.Z + 20000 }, nVel);
	//	return true;
	//}

	return false;
}

//TODO : re-eval
#include <lib/gcem/gcem.hpp>

bool AresTActionExt::LauchhChemMissile(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	const auto pFind = WeaponTypeClass::Find(GameStrings::ChemLauncher);
	if (!pFind)
		return false;

	auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);

	if (auto pBullet = pFind->Projectile->CreateBullet(MapClass::Instance->GetCellAt(nLoc), nullptr, pFind->Damage, pFind->Warhead, 20, false))
	{
		pBullet->SetWeaponType(pFind);
		constexpr double nSin = gcem::sin(1.570748388432313);
		constexpr double nCos = gcem::cos(-0.00009587672516830327);

		BulletExtContainer::Instance.Find(pBullet)->Owner = pHouse;
		auto nCell = MapClass::Instance->Localsize_586AC0(&nLoc, false);

		pBullet->MoveTo(
			{ nCell.X + 128 , nCell.Y + 128 , 0 },
			{ nCos * nCos * 100.0  , nCos * nSin * 100.0  , nSin * 100.0 }
		);
		return true;
	}

	return false;
}

bool AresTActionExt::LightstormStrike(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);

	// get center of cell coords
	auto const pCell = MapClass::Instance->GetCellAt(nLoc);
	auto coords = pCell->GetCoordsWithBridge();

	// create a cloud animation
	if (coords.IsValid())
	{
		// select the anim
		auto const& itClouds = RulesClass::Instance->WeatherConClouds;
		auto const pAnimType = itClouds.Items[ScenarioClass::Instance->Random.RandomFromMax(itClouds.Count - 1)];

		if (pAnimType)
		{
			coords.Z += GeneralUtils::GetLSAnimHeightFactor(pAnimType, pCell, true);

			if (coords.IsValid())
			{
				// create the cloud and do some book keeping.auto const
				auto pAnim = GameCreate<AnimClass>(pAnimType, coords);
				pAnim->SetHouse(pHouse);
				LightningStorm::CloudsManifesting->AddItem(pAnim);
				LightningStorm::CloudsPresent->AddItem(pAnim);
			}
		}
	}

	return true;
}

bool AresTActionExt::MeteorStrike(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	static constexpr reference<int, 0x842AFC, 5u> MeteorAddAmount {};

	const auto pSmall = AnimTypeClass::Find(GameStrings::METSMALL);
	const auto pBig = AnimTypeClass::Find(GameStrings::METLARGE);

	if (!pSmall && !pBig)
		return false;

	auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);
	CoordStruct nCoord = CellClass::Cell2Coord(nLoc);
	nCoord.Z = MapClass::Instance->GetCellFloorHeight(nCoord);

	if (MapClass::Instance->GetCellAt(nCoord)->ContainsBridge())
		nCoord.Z += Unsorted::BridgeHeight;

	const auto amount = MeteorAddAmount[pAction->Value % MeteorAddAmount.size()] + ScenarioClass::Instance->Random.Random() % 3;
	if (amount <= 0)
		return true;

	const int nTotal = 70 * amount;

	for (int i = nTotal; i > 0; --i)
	{
		auto nRandX = ScenarioClass::Instance->Random.Random() % nTotal;
		auto nRandY = ScenarioClass::Instance->Random.Random() % nTotal;
		CoordStruct nAnimLoc { nRandX + nCoord.X ,nRandY + nCoord.Y ,nCoord.Z };

		AnimTypeClass* pSelected = pBig;
		int nRandHere = abs(ScenarioClass::Instance->Random.Random()) & 0x80000001;
		bool v13 = nRandHere == 0;
		if (nRandHere < 0)
		{
			v13 = ((nRandHere - 1) | 0xFFFFFFFE) == -1;
		}

		if (v13)
		{
			pSelected = pSmall;
		}

		if (pSelected)
		{
			auto pAnim = GameCreate<AnimClass>(pSelected, nAnimLoc, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
			pAnim->Owner = pHouse;
		}
	}

	return true;
}

bool AresTActionExt::PlayAnimAt(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (const auto pAnimType = AnimTypeClass::Array->GetItemOrDefault(pAction->Value))
	{
		auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);
		CoordStruct nCoord = CellClass::Cell2Coord(nLoc);
		nCoord.Z = MapClass::Instance->GetCellFloorHeight(nCoord);

		if (MapClass::Instance->GetCellAt(nCoord)->ContainsBridge())
			nCoord.Z += Unsorted::BridgeHeight;
		//Debug::Log("Trigger %s - Tag %s PlayAnimAt at(%d %d %d) Anim[%s - %d]\n",
		//	pAction->TriggerType ? pAction->TriggerType->get_ID() : NONE_STR,
		//	pAction->TagType ? pAction->TagType->get_ID() : NONE_STR,
		//	nCoord.X, nCoord.Y, nCoord.Z,
		//	pAnimType->ID,
		//	pAction->Value
		//);

		auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
		pAnim->IsPlaying = !pAction->Param3;
		pAnim->Owner = pHouse;
	}

	return true;
}

bool AresTActionExt::DoExplosionAt(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (const auto pWeaponType = WeaponTypeClass::Array->GetItemOrDefault(pAction->Value))
	{
		auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);
		CoordStruct nCoord = CellClass::Cell2Coord(nLoc);
		nCoord.Z = MapClass::Instance->GetCellFloorHeight(nCoord);
		const auto pCell = MapClass::Instance->GetCellAt(nCoord);

		if (pCell->ContainsBridge())
			nCoord.Z += Unsorted::BridgeHeight;

		//Debug::Log("Trigger %s - Tag %s DoExplosion at(%d %d %d) Weapon[%s] Warhead[%s]\n",
		//	pAction->TriggerType ? pAction->TriggerType->get_ID() : NONE_STR ,
		//	pAction->TagType ? pAction->TagType->get_ID() : NONE_STR,
		//	nCoord.X, nCoord.Y , nCoord.Z,
		//	pWeaponType->ID,
		//	pWeaponType->Warhead->ID
		//);

		if (auto pAnimType = MapClass::SelectDamageAnimation(pWeaponType->Damage, pWeaponType->Warhead, pCell->LandType, nCoord))
		{
			auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, AnimFlag::AnimFlag_2000 | AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, -15, 0);
			pAnim->IsPlaying = true;
			pAnim->Owner = pHouse;
		}

		MapClass::FlashbangWarheadAt(pWeaponType->Damage, pWeaponType->Warhead, nCoord);
		MapClass::DamageArea(&nCoord, pWeaponType->Damage, nullptr, pWeaponType->Warhead, true, pHouse);
	}

	return true;
}

bool AresTActionExt::EnableTrigger(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (pTrigger)
	{
		TriggerClass::Array->for_each([pTrigger](TriggerClass* pTrig){

			if (pTrig == pTrigger) {
				if (ScenarioClass::Instance->Difficulty1 == AIDifficulty::Easy && pTrig->Type->Difficulty[0]
					|| ScenarioClass::Instance->Difficulty1 == AIDifficulty::Normal && pTrig->Type->Difficulty[1]
					|| ScenarioClass::Instance->Difficulty1 == AIDifficulty::Hard && pTrig->Type->Difficulty[2]) {

					pTrig->Enable();
				}
			}
		});
	}

	return true;
}

bool AresTActionExt::Retint(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location, DefaultColorList col) {

	TintStruct copy_ = ScenarioClass::Instance->NormalLighting.Tint;
	switch (col)
	{

	case DefaultColorList::Red:
		copy_.Red = pAction->Value * 10;
		copy_.Green *= 10;
		copy_.Blue *= 10;
		ScenarioClass::RecalcLighting(copy_.Red, copy_.Green, copy_.Blue, false);
		ScenarioClass::Instance->NormalLighting.Tint.Red = pAction->Value;
		break;
	case DefaultColorList::Green:
		copy_.Green = pAction->Value * 10;
		copy_.Red *= 10;
		copy_.Blue *= 10;
		ScenarioClass::RecalcLighting(copy_.Red, copy_.Green, copy_.Blue, false);
		ScenarioClass::Instance->NormalLighting.Tint.Green = pAction->Value;
		break;
	case DefaultColorList::Blue:
		copy_.Blue = pAction->Value * 10;
		copy_.Red *= 10;
		copy_.Green *= 10;
		ScenarioClass::RecalcLighting(copy_.Red, copy_.Green, copy_.Blue, false);
		ScenarioClass::Instance->NormalLighting.Tint.Blue = pAction->Value;
		break;
	default:
		return false;
	}

	ScenarioExtData::UpdateLightSources = true;
	return true;
}

bool AresTActionExt::Execute(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location, bool& ret)
{
	switch ((AresNewTriggerAction)pAction->ActionKind)
	{
	case AresNewTriggerAction::AuxiliaryPower:
	{
		ret = AuxiliaryPower(pAction, pHouse, pObject, pTrigger, location);
		break;
	}
	case AresNewTriggerAction::KillDriversOf:
	{
		ret = KillDriversOf(pAction, pHouse, pObject, pTrigger, location);
		break;
	}
	case AresNewTriggerAction::SetEVAVoice:
	{
		ret = SetEVAVoice(pAction, pHouse, pObject, pTrigger, location);
		break;
	}
	case AresNewTriggerAction::SetGroup:
	{
		ret = SetGroup(pAction, pHouse, pObject, pTrigger, location);
		break;
	}
	default:
	{

		switch (pAction->ActionKind)
		{
		case TriggerAction::PlayAnimAt:
			ret = PlayAnimAt(pAction, pHouse, pObject, pTrigger, location);
			return true;
		case TriggerAction::MeteorShower:
			ret = MeteorStrike(pAction, pHouse, pObject, pTrigger, location);
			return true;
		case TriggerAction::LightningStrike:
			ret = LightstormStrike(pAction, pHouse, pObject, pTrigger, location);
			return true;
		case TriggerAction::ActivateFirestorm:
			ret = ActivateFirestorm(pAction, pHouse, pObject, pTrigger, location);
			return true;
		case TriggerAction::DeactivateFirestorm:
			ret = DeactivateFirestorm(pAction, pHouse, pObject, pTrigger, location);
			return true;
		case TriggerAction::NukeStrike:
			ret = LauchhNuke(pAction, pHouse, pObject, pTrigger, location);
			return true;
		case TriggerAction::ChemMissileStrike:
			ret = LauchhChemMissile(pAction, pHouse, pObject, pTrigger, location);
			return true;
		case TriggerAction::DoExplosionAt:
			ret = DoExplosionAt(pAction, pHouse, pObject, pTrigger, location);
			return true;
		case TriggerAction::RetintRed :
			ret = Retint(pAction, pHouse, pObject, pTrigger, location, DefaultColorList::Red);
			return true;
		case TriggerAction::RetintGreen:
			ret = Retint(pAction, pHouse, pObject, pTrigger, location, DefaultColorList::Green);
			return true;
		case TriggerAction::RetintBlue:
			ret = Retint(pAction, pHouse, pObject, pTrigger, location, DefaultColorList::Blue);
			return true;
		default:
			break;
		}

		return false;
	}
	}

	return true;
}
#pragma endregion

#pragma region AresTEventExt

NOINLINE HouseClass* AresTEventExt::ResolveHouseParam(int const param, HouseClass* const pOwnerHouse)
{
	if (param == -1)
		return nullptr;

	if (param == 8997)
	{
		return pOwnerHouse;
	}

	if (HouseClass::Index_IsMP(param))
	{
		return HouseClass::FindByIndex(param);
	}

	return HouseClass::FindByCountryIndex(param);
}

std::pair<Persistable, bool> AresTEventExt::GetPersistableFlag(AresTriggerEvents nAction)
{
	switch (nAction)
	{
	case AresTriggerEvents::UnderEMP:
	case AresTriggerEvents::UnderEMP_ByHouse:
	case AresTriggerEvents::RemoveEMP:
	case AresTriggerEvents::RemoveEMP_ByHouse:
	case AresTriggerEvents::EnemyInSpotlightNow:
	case AresTriggerEvents::ReverseEngineered:
	case AresTriggerEvents::HouseOwnTechnoType:
	case AresTriggerEvents::HouseDoesntOwnTechnoType:
	case AresTriggerEvents::AttackedOrDestroyedByAnybody:
	case AresTriggerEvents::AttackedOrDestroyedByHouse:
	case AresTriggerEvents::TechnoTypeDoesntExistMoreThan:
		return { Persistable::unk_0x100  , true };
	case AresTriggerEvents::DriverKiller:
	case AresTriggerEvents::DriverKilled_ByHouse:
	case AresTriggerEvents::VehicleTaken:
	case AresTriggerEvents::VehicleTaken_ByHouse:
	case AresTriggerEvents::Abducted:
	case AresTriggerEvents::Abducted_ByHouse:
	case AresTriggerEvents::AbductSomething:
	case AresTriggerEvents::AbductSomething_OfHouse:
	case AresTriggerEvents::SuperActivated:
	case AresTriggerEvents::SuperDeactivated:
	case AresTriggerEvents::SuperNearWaypoint:
	case AresTriggerEvents::ReverseEngineerAnything:
	case AresTriggerEvents::ReverseEngineerType:
	case AresTriggerEvents::DestroyedByHouse:
	case AresTriggerEvents::AllKeepAlivesDestroyed:
	case AresTriggerEvents::AllKeppAlivesBuildingDestroyed:
		return { Persistable::unk_0x101  , true };
	default:
		return { Persistable::None  , false };
	}
}

std::pair<LogicNeedType, bool > AresTEventExt::GetLogicNeed(AresTriggerEvents nAction)
{
	switch (nAction)
	{
	case AresTriggerEvents::UnderEMP://
	case AresTriggerEvents::RemoveEMP: //
	case AresTriggerEvents::EnemyInSpotlightNow://
	case AresTriggerEvents::DriverKiller://
	case AresTriggerEvents::VehicleTaken://
	case AresTriggerEvents::Abducted://
	case AresTriggerEvents::AbductSomething://
	case AresTriggerEvents::SuperActivated://
	case AresTriggerEvents::SuperDeactivated://
	case AresTriggerEvents::ReverseEngineerAnything://
	case AresTriggerEvents::AttackedOrDestroyedByAnybody://
		return { LogicNeedType::None  , true };
	case AresTriggerEvents::UnderEMP_ByHouse://
	case AresTriggerEvents::RemoveEMP_ByHouse://
	case AresTriggerEvents::DriverKilled_ByHouse:
	case AresTriggerEvents::VehicleTaken_ByHouse:
	case AresTriggerEvents::Abducted_ByHouse:
	case AresTriggerEvents::AbductSomething_OfHouse:
	case AresTriggerEvents::AttackedOrDestroyedByHouse:
	case AresTriggerEvents::DestroyedByHouse:
	case AresTriggerEvents::AllKeepAlivesDestroyed:
	case AresTriggerEvents::AllKeppAlivesBuildingDestroyed:
		return { LogicNeedType::House  , true };
	case AresTriggerEvents::SuperNearWaypoint:
	case AresTriggerEvents::ReverseEngineered:
	case AresTriggerEvents::ReverseEngineerType:
	case AresTriggerEvents::HouseOwnTechnoType:
	case AresTriggerEvents::HouseDoesntOwnTechnoType:
	case AresTriggerEvents::TechnoTypeDoesntExistMoreThan:
		return { LogicNeedType::NumberNTech  , true };
	default:
		return { LogicNeedType::None  , false };
	}
}

std::pair<bool, TriggerAttachType> AresTEventExt::GetAttachFlags(AresTriggerEvents nEvent)
{
	switch (nEvent)
	{
	case AresTriggerEvents::UnderEMP:
	case AresTriggerEvents::UnderEMP_ByHouse:
	case AresTriggerEvents::RemoveEMP:
	case AresTriggerEvents::RemoveEMP_ByHouse:
	case AresTriggerEvents::EnemyInSpotlightNow:
	case AresTriggerEvents::DriverKiller:
	case AresTriggerEvents::DriverKilled_ByHouse:
	case AresTriggerEvents::VehicleTaken:
	case AresTriggerEvents::VehicleTaken_ByHouse:
	case AresTriggerEvents::Abducted:
	case AresTriggerEvents::Abducted_ByHouse:
	case AresTriggerEvents::AbductSomething:
	case AresTriggerEvents::AbductSomething_OfHouse:
	case AresTriggerEvents::ReverseEngineerAnything:
	case AresTriggerEvents::ReverseEngineerType:
	case AresTriggerEvents::AttackedOrDestroyedByAnybody:
	case AresTriggerEvents::AttackedOrDestroyedByHouse:
	{
		return { true , TriggerAttachType::Object };
	}
	case AresTriggerEvents::SuperActivated:
	case AresTriggerEvents::SuperDeactivated:
	case AresTriggerEvents::SuperNearWaypoint:
	case AresTriggerEvents::ReverseEngineered:
	case AresTriggerEvents::HouseOwnTechnoType:
	case AresTriggerEvents::HouseDoesntOwnTechnoType:
	case AresTriggerEvents::DestroyedByHouse:
	case AresTriggerEvents::AllKeepAlivesDestroyed:
	case AresTriggerEvents::AllKeppAlivesBuildingDestroyed:
	{
		return { true ,TriggerAttachType::House };
	}
	case AresTriggerEvents::TechnoTypeDoesntExistMoreThan:
	{
		return { true ,TriggerAttachType::Logic };
	}
	}

	return { false ,TriggerAttachType::None };
}

bool AresTEventExt::FindTechnoType(TEventClass* pThis, int args, HouseClass* pWho)
{
	const auto pType = TEventExtContainer::Instance.Find(pThis)->GetTechnoType();
	if (!pType)
		return false;

	if (args <= 0)
		return true;

	if (!pType->Insignificant && !pType->DontScore)
	{
		HouseClass** const arr = pWho ? &pWho : HouseClass::Array->Items;
		HouseClass** const nEnd = arr + (pWho ? 1 : HouseClass::Array->Count);

		int i = args;

		for (HouseClass** nPos = arr; nPos != nEnd; ++nPos)
		{
			i -= (*nPos)->CountOwnedNow(pType);

			if (i <= 0)
			{
				return true;
			}
		}
	}
	else
	{
		int i = args;
		TechnoClass** arrayItems = nullptr;
		int arrayCount = 0;

		switch (pType->WhatAmI())
		{
		case AbstractType::AircraftType:
		{
			arrayItems = (TechnoClass**)AircraftClass::Array->Items;
			arrayCount = AircraftClass::Array->Count;
			break;
		}
		case AbstractType::UnitType:
		{
			arrayItems = (TechnoClass**)UnitClass::Array->Items;
			arrayCount = UnitClass::Array->Count;
			break;
		}
		case AbstractType::InfantryType:
		{
			arrayItems = (TechnoClass**)InfantryClass::Array->Items;
			arrayCount = InfantryClass::Array->Count;
			break;
		}
		case AbstractType::BuildingType:
		{
			arrayItems = (TechnoClass**)BuildingClass::Array->Items;
			arrayCount = BuildingClass::Array->Count;
			break;
		}
		default:
			break;
		}

		if (arrayCount > 0 && arrayItems)
		{
			const auto arrayItemsEnd = arrayItems + arrayCount;
			for (auto walk = arrayItems; walk != arrayItemsEnd; ++walk)
			{
				if (pWho && pWho != (*walk)->Owner)
					continue;

				if ((*walk)->GetTechnoType() == pType)
				{
					i--;

					if (i <= 0)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

// the function return is deciding if the case is handled or not
// the bool result pointer is for the result of the Event itself
bool AresTEventExt::HasOccured(TEventClass* pThis, EventArgs& Args, bool& result)
{
	{
		switch ((AresTriggerEvents)pThis->EventKind)
		{
		case AresTriggerEvents::UnderEMP:
		case AresTriggerEvents::UnderEMP_ByHouse:
		case AresTriggerEvents::RemoveEMP:
		case AresTriggerEvents::RemoveEMP_ByHouse:
		{
			const auto pTechno = generic_cast<TechnoClass*>(Args.Object);

			if (pTechno && pThis->EventKind == Args.EventType)
			{

				switch ((AresTriggerEvents)Args.EventType)
				{
				case AresTriggerEvents::UnderEMP:
				{
					result = pTechno->EMPLockRemaining > 0;
					return true;
				}
				case AresTriggerEvents::UnderEMP_ByHouse:
				{
					if (Args.Source && ((TechnoClass*)(Args.Source))->Owner->ArrayIndex == pThis->Value)
					{
						result = pTechno->EMPLockRemaining > 0;
						return true;
					}
					break;
				}
				case AresTriggerEvents::RemoveEMP:
				{
					result = pTechno->EMPLockRemaining <= 0;
					return true;
				}
				case AresTriggerEvents::RemoveEMP_ByHouse:
				{
					if (Args.Source && ((TechnoClass*)(Args.Source))->Owner->ArrayIndex == pThis->Value)
					{
						result = pTechno->EMPLockRemaining <= 0;
						return true;
					}
					break;
				}
				}
			}

			result = false;
			return true;
		}
		case AresTriggerEvents::EnemyInSpotlightNow:
		{
			result = true;
			return true;
		}
		case AresTriggerEvents::DriverKiller:
		{
			result = generic_cast<FootClass*>(Args.Object)
				&& pThis->EventKind == Args.EventType;

			return true;
		}
		case AresTriggerEvents::DriverKilled_ByHouse:
		{
			result = generic_cast<FootClass*>(Args.Object)
				&& pThis->EventKind == Args.EventType
				&& Args.Source
				&& ((TechnoClass*)Args.Source)->Owner->ArrayIndex == pThis->Value;

			return true;
		}
		case AresTriggerEvents::VehicleTaken:
		{
			result = generic_cast<FootClass*>(Args.Object)
				&& pThis->EventKind == Args.EventType;

			return true;
		}
		case AresTriggerEvents::VehicleTaken_ByHouse:
		{
			result = generic_cast<FootClass*>(Args.Object)
				&& pThis->EventKind == Args.EventType
				&& Args.Source
				&& ((TechnoClass*)Args.Source)->Owner->ArrayIndex == pThis->Value;

			return true;
		}
		case AresTriggerEvents::Abducted:
		case AresTriggerEvents::AbductSomething:
		case AresTriggerEvents::Abducted_ByHouse:
		case AresTriggerEvents::AbductSomething_OfHouse:
		{
			const auto pTechno = generic_cast<FootClass*>(Args.Object);

			if (pTechno && pThis->EventKind == Args.EventType)
			{
				switch ((AresTriggerEvents)Args.EventType)
				{
				case AresTriggerEvents::Abducted:
				case AresTriggerEvents::AbductSomething:
				{
					result = true;
					return true;
				}
				case AresTriggerEvents::Abducted_ByHouse:
				{
					if (generic_cast<TechnoClass*>(Args.Source) && ((TechnoClass*)(Args.Source))->Owner->ArrayIndex == pThis->Value)
					{
						result = true;
						return true;
					}

					break;
				}
				case AresTriggerEvents::AbductSomething_OfHouse:
				{
					if (specific_cast<HouseClass*>(Args.Source) && ((HouseClass*)(Args.Source))->ArrayIndex == pThis->Value)
					{
						result = true;
						return true;
					}

					break;
				}
				}
			}

			result = false;
			return true;
		}
		case AresTriggerEvents::SuperActivated:
		case AresTriggerEvents::SuperDeactivated:
		{
			result = pThis->EventKind == Args.EventType
				&& Args.Source
				&& Args.Source->WhatAmI() == AbstractType::Super
				&& ((SuperClass*)Args.Source)->Type->ArrayIndex == pThis->Value;

			return true;
		}
		case AresTriggerEvents::SuperNearWaypoint:
		{
			struct PackedDatas
			{
				SuperClass* Super;
				CellStruct Cell;
			};

			if ((pThis->EventKind == Args.EventType) && IS_SAME_STR_(((PackedDatas*)Args.Source)->Super->Type->ID, pThis->String))
			{
				const auto nCell = ScenarioClass::Instance->GetWaypointCoords(pThis->Value);
				CellStruct nDesired = { ((PackedDatas*)Args.Source)->Cell.X - nCell.X ,((PackedDatas*)Args.Source)->Cell.Y - nCell.Y };
				if (nDesired.pow() <= 5.0)
				{
					result = true;
					return true;
				}
			}

			result = false;
			return true;
		}
		case AresTriggerEvents::ReverseEngineered:
		{
			if (!Args.Owner)
				result = false;
			else
			{
				result = HouseExtContainer::Instance.Find(Args.Owner)->Reversed.any_of
				([&](TechnoTypeClass* pTech)
 {
	 return pTech == TEventExtContainer::Instance.Find(pThis)->GetTechnoType();
				});
			}

			return true;
		}
		case AresTriggerEvents::ReverseEngineerAnything:
		{
			result = (pThis->EventKind == Args.EventType);
			return true;
		}
		case AresTriggerEvents::ReverseEngineerType:
		{
			result = ((TechnoClass*)Args.Source)->GetTechnoType() == TEventExtContainer::Instance.Find(pThis)->GetTechnoType();
			return true;
		}
		case AresTriggerEvents::HouseOwnTechnoType:
		{
			result = FindTechnoType(pThis, pThis->Value, Args.Owner);
			return true;
		}
		case AresTriggerEvents::HouseDoesntOwnTechnoType:
		{
			result = !FindTechnoType(pThis, pThis->Value + 1, Args.Owner);
			return true;
		}
		case AresTriggerEvents::AttackedOrDestroyedByAnybody:
		{
			result = (pThis->EventKind == Args.EventType);
			return true;
		}
		case AresTriggerEvents::AttackedOrDestroyedByHouse:
		{
			result = (pThis->EventKind == Args.EventType)
				&& Args.Source
				&& ((TechnoClass*)Args.Source)->Owner->ArrayIndex == pThis->Value;

			return true;
		}
		case AresTriggerEvents::DestroyedByHouse:
		{
			result = ((AresTriggerEvents)Args.EventType == AresTriggerEvents::DestroyedByHouse)
				&& Args.Source
				&& ((HouseClass*)Args.Source)->ArrayIndex == pThis->Value;

			return true;
		}
		case AresTriggerEvents::TechnoTypeDoesntExistMoreThan:
		{
			result = FindTechnoType(pThis, pThis->Value + 1, nullptr);
			return true;
		}
		case AresTriggerEvents::AllKeepAlivesDestroyed:
		{
			HouseClass* pHouse = pThis->Value == 0x2325 ?
				nullptr : HouseClass::Index_IsMP(pThis->Value) ?
				HouseClass::FindByIndex(pThis->Value) : HouseClass::FindByCountryIndex(pThis->Value);

			result = pHouse && HouseExtContainer::Instance.Find(pHouse)->KeepAliveCount <= 0;
			return true;
		}
		case AresTriggerEvents::AllKeppAlivesBuildingDestroyed:
		{
			HouseClass* pHouse = pThis->Value == 0x2325 ?
				nullptr : HouseClass::Index_IsMP(pThis->Value) ?
				HouseClass::FindByIndex(pThis->Value) : HouseClass::FindByCountryIndex(pThis->Value);

			result = pHouse && HouseExtContainer::Instance.Find(pHouse)->KeepAliveBuildingCount <= 0;
			return true;
		}
		default:

			switch (pThis->EventKind)
			{
			case TriggerEvent::TechTypeExists:
			{
				//TechnoTypeExist
				result = FindTechnoType(pThis, pThis->Value, nullptr);
				return true;
			}
			case TriggerEvent::TechTypeDoesntExist:
			{
				//TechnoTypeDoesntExist
				result = !FindTechnoType(pThis, 1, nullptr);
				return true;
			}
			default:
				break;
			}

			break;
		}
	}

	return false;
}

#pragma endregion

#pragma region TunnelFuncs

bool TunnelFuncs::FindSameTunnel(BuildingClass* pTunnel)
{
	const auto pOwner = pTunnel->Owner;
	if (!pOwner)
		return false;

	//found new building
	return pOwner->Buildings.any_of([pTunnel](BuildingClass* pBld)
	{
		if (pTunnel != pBld && pBld->Health > 0 && !pBld->InLimbo && pBld->IsOnMap)
		{
			if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
				return false;

			const auto nCurMission = pBld->CurrentMission;
			if (nCurMission != Mission::Construction && nCurMission != Mission::Selling)
			{
				if (BuildingTypeExtContainer::Instance.Find(pBld->Type)->TunnelType == BuildingTypeExtContainer::Instance.Find(pTunnel->Type)->TunnelType)
				{
					return true;
				}
			}
		}

		return false;
	});
}

void TunnelFuncs::KillFootClass(FootClass* pFoot, TechnoClass* pKiller)
{
	if (!pFoot || !pFoot->IsAlive)
		return;

	pFoot->RegisterDestruction(pKiller);
	Debug::Log(__FUNCTION__" Called \n");
	TechnoExtData::HandleRemove(pFoot, pKiller, false, false);
}

void TunnelFuncs::DestroyTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, TechnoClass* pKiller)
{
	if (pTunnelData->empty() || FindSameTunnel(pTunnel))
		return;

	for (auto pFoot : *pTunnelData)
	{
		KillFootClass(pFoot, pKiller);
	}

	pTunnelData->clear();
}

void TunnelFuncs::EnterTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, FootClass* pFoot)
{
	pFoot->SetTarget(nullptr);
	pFoot->OnBridge = false;
	pFoot->unknown_C4 = 0;
	pFoot->GattlingValue = 0;
	pFoot->SetGattlingStage(0);

	if (auto const pCapturer = pFoot->MindControlledBy)
	{
		if (const auto pCmanager = pCapturer->CaptureManager)
		{
			pCmanager->FreeUnit(pFoot);
		}
	}

	if (!pFoot->Limbo())
	{
		Debug::Log("Techno[%s] Trying to enter Tunnel[%s] but failed ! \n", pFoot->get_ID(), pTunnel->get_ID());
		return;
	}

	VocClass::PlayIndexAtPos(pTunnel->Type->EnterTransportSound, pTunnel->Location);

	pFoot->Undiscover();

	if (pFoot->GetCurrentMission() == Mission::Hunt)
		pFoot->AbortMotion();

	pTunnelData->push_back(pFoot);
}

bool TunnelFuncs::CanEnterTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, FootClass* pEnterer)
{
	if (pEnterer->SendCommand(RadioCommand::QueryCanEnter, pTunnel) != RadioCommand::AnswerPositive)
		return false;

	EnterTunnel(pTunnelData, pTunnel, pEnterer);
	return true;
}

std::vector<int>* TunnelFuncs::PopulatePassangerPIPData(TechnoClass* pThis, TechnoTypeClass* pType, bool& Fail)
{
	static std::vector<int> PipData;

	int nPassangersTotal = pType->GetPipMax();
	if (nPassangersTotal < 0)
		nPassangersTotal = 0;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	PipData.clear();

	if (const auto pBld = specific_cast<BuildingClass*>(pThis))
	{
		const TunnelData* pTunnelData = HouseExtData::GetTunnelVector(pBld->Type, pThis->Owner);
		const bool Absorber = pBld->Absorber();

		if (!pTunnelData)
		{
			if (pThis->Passengers.NumPassengers > nPassangersTotal)
			{
				Fail = true;
				return &PipData;
			}

			PipData.resize(nPassangersTotal);

			int nCargoSize = 0;
			for (auto pPassenger = pThis->Passengers.GetFirstPassenger();
				pPassenger;
				pPassenger = generic_cast<FootClass*>(pPassenger->NextObject))
			{
				const auto pPassengerType = pPassenger->GetTechnoType();

				auto nSize = !Absorber ? (int)pPassengerType->Size : 1;
				if (nSize <= 0)
					nSize = 1;

				int nPip = 1;
				const auto what = pPassenger->WhatAmI();

				if (what == InfantryClass::AbsID)
					nPip = (int)(static_cast<InfantryTypeClass*>(pPassengerType)->Pip);
				else if (what == UnitClass::AbsID)
					nPip = 5;

				//fetch first cargo size and change the pip
				PipData[nCargoSize] = nPip;
				for (int i = nSize - 1; i > 0; --i)
				{ //check if the extra size is there and increment it to
			   // total size
					PipData[nCargoSize + i] = 3;     //set extra size to pip index 3
				}

				nCargoSize += nSize;
			}

			return &PipData;
		}
		else
		{
			const int nTotal = pTunnelData->MaxCap > nPassangersTotal ? nPassangersTotal : pTunnelData->MaxCap;
			PipData.resize(nTotal);

			if ((int)pTunnelData->Vector.size() > nTotal)
			{
				Fail = true;
				return &PipData;
			}

			for (size_t i = 0; i < pTunnelData->Vector.size(); ++i)
			{
				auto const& pContent = pTunnelData->Vector[i];
				const auto what = pContent->WhatAmI();

				int nPip = 1;
				if (what == InfantryClass::AbsID)
					nPip = (int)(static_cast<InfantryClass*>(pContent)->Type->Pip);
				else if (what == UnitClass::AbsID)
					nPip = 4;

				PipData[i] = nPip;
			}

			return &PipData;
		}
	}
	else
	{
		if (pThis->Passengers.NumPassengers > nPassangersTotal)
		{
			Fail = true;
			return &PipData;
		}

		PipData.resize(nPassangersTotal);

		int nCargoSize = 0;
		for (auto pPassenger = pThis->Passengers.GetFirstPassenger();
			pPassenger;
			pPassenger = generic_cast<FootClass*>(pPassenger->NextObject))
		{
			const auto pPassengerType = pPassenger->GetTechnoType();

			auto nSize = pTypeExt->Passengers_BySize.Get() ? (int)pPassengerType->Size : 1;
			if (nSize <= 0)
				nSize = 1;

			const auto what = pPassenger->WhatAmI();

			int nPip = 1;
			if (what == InfantryClass::AbsID)
				nPip = (int)(static_cast<InfantryTypeClass*>(pPassengerType)->Pip);
			else if (what == UnitClass::AbsID)
				nPip = 5;

			//fetch first cargo size and change the pip
			PipData[nCargoSize] = nPip;
			for (int i = nSize - 1; i > 0; --i)
			{ //check if the extra size is there and increment it to
		   // total size
				PipData[nCargoSize + i] = 3;     //set extra size to pip index 3
			}

			nCargoSize += nSize;
		}

		return &PipData;
	}
}

std::pair<bool, FootClass*> TunnelFuncs::UnlimboOne(std::vector<FootClass*>* pVector, BuildingClass* pTunnel, DWORD Where)
{
	auto pPassenger = pVector->back();
	auto nCoord = pTunnel->GetCoords();

	const auto nBldFacing = ((((short)pTunnel->PrimaryFacing.Current().Raw >> 7) + 1) >> 1);

	pPassenger->OnBridge = pTunnel->OnBridge;
	pPassenger->SetLocation(nCoord);

	++Unsorted::ScenarioInit();
	bool Succeeded = pPassenger->Unlimbo(nCoord, (DirType)nBldFacing);
	--Unsorted::ScenarioInit();

	if (Succeeded)
	{
		pVector->pop_back();
		pPassenger->Scatter(CoordStruct::Empty, true, false);
		return { true,  pPassenger };
	}

	return { false,  pPassenger };
}

bool TunnelFuncs::UnloadOnce(FootClass* pFoot, BuildingClass* pTunnel, bool silent)
{
	const auto facing = (((((short)pTunnel->PrimaryFacing.Current().Raw + 0x8000) >> 12) + 1) >> 1);
	const auto Loc = pTunnel->GetMapCoords();

	int nOffset = 0;
	bool IsLessThanseven = true;
	bool Succeeded = false;
	CellStruct nResult;
	CellClass* CurrentAdj = nullptr;
	CellClass* NextCell = nullptr;
	size_t nFacing = 0;

	//TODO Fix these loop
	for (int i = 0;; ++i)
	{
		nFacing = (facing + i) & 7;
		const CellStruct tmpCoords = CellSpread::AdjacentCell[nFacing];
		nResult = tmpCoords + Loc;
		CellStruct next = tmpCoords + tmpCoords + Loc;

		CurrentAdj = MapClass::Instance->GetCellAt(nResult);
		NextCell = MapClass::Instance->GetCellAt(next);

		const auto nLevel = pTunnel->GetCellLevel();
		const auto nOccupyResult = pFoot->IsCellOccupied(CurrentAdj, (FacingType)nFacing, nLevel, nullptr, true);
		const auto nNextnOccupyResult = pFoot->IsCellOccupied(NextCell, (FacingType)nFacing, nLevel, nullptr, true);

		if ((!(int)nNextnOccupyResult) &&
			(!IsLessThanseven || (!(int)nOccupyResult)) &&
			(CurrentAdj->Flags & CellFlags::BridgeHead) == CellFlags::Empty)
			break;

		IsLessThanseven = i == 7 ? false : true;

		++nOffset;

		if (nOffset >= 16)
			return false;
	}

	const auto pFootType = pFoot->GetTechnoType();
	++Unsorted::ScenarioInit();

	CoordStruct nResultC = CellClass::Cell2Coord(nResult);
	if (pFoot->WhatAmI() == AbstractType::Infantry)
	{
		nResultC = MapClass::Instance->PickInfantrySublocation(nResultC, false);
	}
	else
	{
		const auto nNearby = MapClass::Instance->NearByLocation(nResult, pFootType->SpeedType, -1, MovementZone::None, false, 1, 1, false, false, false, true, CellStruct::Empty, false, false);
		nResultC = CellClass::Cell2Coord(nNearby);
	}

	Succeeded = pFoot->Unlimbo(nResultC, DirType(32 * (nFacing & 0x3FFFFFF)));
	--Unsorted::ScenarioInit();

	if (Succeeded)
	{
		if (!silent)
			VocClass::PlayIndexAtPos(pTunnel->Type->LeaveTransportSound, pTunnel->Location);

		pFoot->QueueMission(Mission::Move, false);
		pFoot->SetDestination(IsLessThanseven ? NextCell : CurrentAdj, true);
		return true;
	}

	TunnelFuncs::KillFootClass(pFoot, nullptr);
	return false;
}

void TunnelFuncs::HandleUnload(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel)
{
	if (UnloadOnce(pTunnelData->back(), pTunnel))
		pTunnelData->pop_back();
}
#pragma endregion

#pragma region AresHouseExt

bool AresHouseExt::CheckBasePlanSanity(HouseClass* const pThis)
{
	// this shouldn't happen, but you never know
	if (pThis->IsControlledByHuman() || pThis->IsNeutral())
	{
		return true;
	}

	auto AllIsWell = true;

	auto const pRules = RulesClass::Instance();
	auto const pType = pThis->Type;

	auto const errorMsg = "AI House[%x] of country [%s] cannot build any object in "
		"%s. The AI ain't smart enough for that.\n";

	// if you don't have a base unit buildable, how did you get to base
	// planning? only through crates or map actions, so have to validate base
	// unit in other situations
	auto const idxParent = pType->FindParentCountryIndex();
	auto const canBuild = pRules->BaseUnit.any_of([pThis, idxParent](UnitTypeClass const* const pItem)
 {
	 return pThis->CanExpectToBuild(pItem, idxParent);
	});

	if (!canBuild)
	{
		AllIsWell = false;
		Debug::FatalError(errorMsg, pType->ID, "BaseUnit");
	}

	auto CheckList = [pThis, pType, idxParent, errorMsg, &AllIsWell](
		Iterator<BuildingTypeClass const*> const list,
		const char* const ListName) -> void
		{
			if (!HouseExtData::FindBuildable(pThis, idxParent, list))
			{
				AllIsWell = false;
				Debug::FatalError(errorMsg, pThis, pType->ID, ListName);
			}
		};

	// commented out lists that do not cause a crash, according to testers
	//CheckList(make_iterator(pRules->Shipyard), "Shipyard");
	CheckList(make_iterator(pRules->BuildPower), "BuildPower");
	CheckList(make_iterator(pRules->BuildRefinery), "BuildRefinery");
	CheckList(make_iterator(pRules->BuildWeapons), "BuildWeapons");
	//CheckList(make_iterator(pRules->BuildConst), "BuildConst");
	//CheckList(make_iterator(pRules->BuildBarracks), "BuildBarracks");
	//CheckList(make_iterator(pRules->BuildTech), "BuildTech");
	//CheckList(make_iterator(pRules->BuildRadar), "BuildRadar");
	//CheckList(make_iterator(pRules->ConcreteWalls), "ConcreteWalls");
	//CheckList(make_iterator(pRules->BuildDummy), "BuildDummy");
	//CheckList(make_iterator(pRules->BuildNavalYard), "BuildNavalYard");


	CheckList(HouseTypeExtContainer::Instance.Find(pType)->GetPowerplants(), "Powerplants");

	//auto const pSide = SideClass::Array->GetItemOrDefault(pType->SideIndex);
	//if(auto const pSideExt = SideExtContainer::Instance.Find(pSide)) {
	//	CheckList(make_iterator(pSideExt->BaseDefenses), "Base Defenses");
	//}

	return AllIsWell;
}

void AresHouseExt::UpdateTogglePower(HouseClass* pThis)
{
	auto pRulesExt = RulesExtData::Instance();

	if (!pRulesExt->TogglePowerAllowed
		|| pRulesExt->TogglePowerDelay <= 0
		|| pRulesExt->TogglePowerIQ < 0
		|| pRulesExt->TogglePowerIQ > pThis->IQLevel2
		|| pThis->Buildings.Count == 0
		|| pThis->IsBeingDrained
		|| pThis->IsControlledByHuman()
		|| pThis->PowerBlackoutTimer.InProgress())
	{
		return;
	}

	if (Unsorted::CurrentFrame % pRulesExt->TogglePowerDelay == 0)
	{
		struct ExpendabilityStruct
		{
		private:
			constexpr std::tuple<const int&, BuildingClass&> Tie() const
			{
				// compare with tie breaker to prevent desyncs
				return std::tie(this->Value, *this->Building);
			}

		public:
			constexpr bool operator < (const ExpendabilityStruct& rhs) const
			{
				return this->Tie() < rhs.Tie();
			}

			constexpr bool operator > (const ExpendabilityStruct& rhs) const
			{
				return this->Tie() > rhs.Tie();
			}

			BuildingClass* Building;
			int Value;
		};

		// properties: the higher this value is, the more likely
		// this building is turned off (expendability)
		auto GetExpendability = [](BuildingClass* pBld) -> int
			{
				auto pType = pBld->Type;

				// disable super weapons, because a defenseless base is
				// worse than one without super weapons
				if (pType->HasSuperWeapon())
				{
					return pType->PowerDrain * 20 / 10;
				}

				// non-base defenses should be disabled before going
				// to the base defenses. but power intensive defenses
				// might still evaluate worse
				if (!pType->IsBaseDefense)
				{
					return pType->PowerDrain * 15 / 10;
				}

				// default case, use power
				return pType->PowerDrain;
			};

		// create a list of all buildings that can be powered down
		// and give each building an expendability value
		std::vector<ExpendabilityStruct> Buildings;
		Buildings.reserve(pThis->Buildings.Count);

		const auto HasLowPower = pThis->HasLowPower();

		for (auto const& pBld : pThis->Buildings)
		{
			if (pBld->InLimbo || BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
				continue;

			auto pType = pBld->Type;
			if (pType->CanTogglePower() && pType->PowerDrain > 0)
			{
				// if low power, we get buildings with StuffEnabled, if enough
				// power, we look for builidings that are disabled
				if (pBld->StuffEnabled == HasLowPower)
				{
					Buildings.emplace_back(pBld, GetExpendability(pBld));
				}
			}
		}

		int Surplus = pThis->PowerOutput - pThis->PowerDrain;

		if (HasLowPower)
		{
			// most expendable building first
			std::sort(Buildings.begin(), Buildings.end(), std::greater<>());

			// turn off the expendable buildings until power is restored
			for (const auto& item : Buildings)
			{
				auto Drain = item.Building->Type->PowerDrain;

				item.Building->GoOffline();
				Surplus += Drain;

				if (Surplus >= 0)
				{
					break;
				}
			}
		}
		else
		{
			// least expendable building first
			std::sort(Buildings.begin(), Buildings.end(), std::less<>());

			// turn on as many of them as possible
			for (const auto& item : Buildings)
			{
				auto Drain = item.Building->Type->PowerDrain;
				if (Surplus - Drain >= 0)
				{
					item.Building->GoOnline();
					Surplus -= Drain;
				}
			}
		}
	}
}

bool AresHouseExt::UpdateAnyFirestormActive(bool const lastChange)
{
	HouseExtData::IsAnyFirestormActive = lastChange;

	// if last change activated one, there is at least one. else...
	if (!lastChange)
	{
		for (auto pHouse : *HouseClass::Array)
		{
			if (pHouse->FirestormActive)
			{
				HouseExtData::IsAnyFirestormActive = true;
				break;
			}
		}
	}

	return HouseExtData::IsAnyFirestormActive;
}

void AresHouseExt::SetFirestormState(HouseClass* pHouse, bool const active)
{
	if (pHouse->FirestormActive == active)
	{
		return;
	}

	pHouse->FirestormActive = active;
	UpdateAnyFirestormActive(active);

	DynamicVectorClass<CellStruct> AffectedCoords;

	for (auto const& pBld : pHouse->Buildings)
	{
		if (BuildingTypeExtContainer::Instance.Find(pBld->Type)->Firestorm_Wall)
		{
			FirewallFunctions::UpdateFirewall(pBld, true);
			auto const temp = pBld->GetMapCoords();
			AffectedCoords.AddItem(temp);
		}
	}

	MapClass::Instance->Update_Pathfinding_1();
	MapClass::Instance->Update_Pathfinding_2(AffectedCoords);
}

void AresHouseExt::FormulateTypeList(std::vector<TechnoTypeClass*>& types, TechnoTypeClass** items, int count, int houseidx)
{
	if (!count)
		return;

	const auto end = items + count;
	for (auto find = items; find != end; ++find)
	{
		if ((*find)->AllowedToStartInMultiplayer)
		{
			if ((*find)->InOwners(houseidx) && ((*find))->TechLevel <= Game::TechLevel())
			{
				types.push_back(*find);
			}
		}
	}
}

std::vector<TechnoTypeClass*> AresHouseExt::GetTypeList()
{
	DWORD avaibleHouses = 0u;
	HelperedVector<TechnoTypeClass*> types;
	types.reserve(InfantryTypeClass::Array->Count + UnitTypeClass::Array->Count);

	for (auto pHouse : *HouseClass::Array)
	{
		if (!pHouse->Type->MultiplayPassive)
		{

			const auto& data = HouseTypeExtContainer::Instance.Find(pHouse->Type)->StartInMultiplayer_Types;
			if (data.HasValue())
			{
				types.insert(types.end(), data.begin(), data.end());
			}
			else
			{
				avaibleHouses |= 1 << pHouse->Type->ArrayIndex;
			}
		}
	}

	FormulateTypeList(types, (TechnoTypeClass**)UnitTypeClass::Array->Items, UnitTypeClass::Array->Count, avaibleHouses);
	FormulateTypeList(types, (TechnoTypeClass**)InfantryTypeClass::Array->Items, InfantryTypeClass::Array->Count, avaibleHouses);

	//remove any `BaseUnit` included
	//base unit given for free then ?
	types.remove_if([](TechnoTypeClass* pItem) {
		for (int i = 0; i < RulesClass::Instance->BaseUnit.Count; ++i) {
			if (pItem == (RulesClass::Instance->BaseUnit.Items[i])) {
				return true;
			}
		}

		return false;
	});

	//idk these part
	//but lets put it here
	//need someone to test this to make sure if the calculation were correct :s
	//-Otamaa
	types.remove_all_duplicates_noshort();
	return types;
}

int AresHouseExt::GetTotalCost(const Nullable<int>& fixed)
{
	if (GameModeOptionsClass::Instance->UnitCount <= 0)
		return 0;

	int totalCost = 0;
	if (fixed.isset())
	{
		totalCost = fixed;
	}
	else
	{

		auto types = GetTypeList();
		int total_ = 0;

		for (auto& tech : types)
		{
			total_ += tech->GetCost();
		}

		const int what = !types.size() ? 1 : types.size();
		totalCost = (total_ + (what >> 1)) / what;
		Debug::Log("Unit cost of %d derived from %u units totalling %d credits.\n", totalCost, what, total_);
	}

	return totalCost * GameModeOptionsClass::Instance->UnitCount;
}

#pragma endregion

#pragma region CustomFoundation

DWORD CustomFoundation::FoundationLength(CellStruct const* const pFoundation)
{
	auto pFCell = pFoundation;
	while (*pFCell != BuildingTypeExtData::FoundationEndMarker)
	{
		++pFCell;
	}

	// include the end marker
	return static_cast<DWORD>(std::distance(pFoundation, pFCell + 1));
}

const std::vector<CellStruct>* CustomFoundation::GetCoveredCells(
	BuildingClass* const pThis, CellStruct const mainCoords,
	int const shadowHeight)
{
	auto const pFoundation = pThis->GetFoundationData(false);
	//auto const len = FoundationLength(pFoundation);

	PhobosGlobal::Instance()->TempCoveredCellsData.clear();
	//PhobosGlobal::Instance()->TempCoveredCellsData.reserve(len * shadowHeight);

	auto pFCell = pFoundation;

	while (*pFCell != BuildingTypeExtData::FoundationEndMarker)
	{
		auto actualCell = mainCoords + *pFCell;
		for (auto i = shadowHeight; i > 0; --i)
		{
			PhobosGlobal::Instance()->TempCoveredCellsData.push_back(actualCell);
			--actualCell.X;
			--actualCell.Y;
		}
		++pFCell;
	}

	PhobosGlobal::Instance()->TempCoveredCellsData.remove_all_duplicates([](const CellStruct& lhs, const CellStruct& rhs) -> bool {
		 return lhs.X > rhs.X || lhs.X == rhs.X && lhs.Y > rhs.Y;
	});

	return &PhobosGlobal::Instance()->TempCoveredCellsData;
}

void CustomFoundation::GetDisplayRect(RectangleStruct& a1, CellStruct* a2)
{
	int v2 = -512;
	int v3 = -512;
	int v4 = 512;
	int v5 = 512;

	if (*a2 == CellStruct::EOL)
	{
		a1.X = 0;
		a1.Y = 0;
		a1.Width = 0;
		a1.Height = 0;
		return;
	}

	int16_t* v6 = &a2->Y;

	while (true)
	{
		int16_t v8 = *(v6 - 1);
		int v12 = v3;
		int v11 = v5;
		int v9 = v4;
		int v10 = v2;
		if (v8 == 0x7FFF)
		{
			v9 = v4;
			if (*v6 == 0x7FFF)
				break;
		}
		v3 = *v6;
		v6 += 2;
		v2 = v8;
		v5 = v3;
		v4 = v8;
		if (v8 >= v9)
			v4 = v9;
		if (v8 <= v10)
			v2 = v10;
		if (v3 >= v11)
			v5 = v11;
		if (v3 <= v12)
			v3 = v12;
	}

	a1.X = v4;
	a1.Y = v5;
	a1.Width = v2;
	a1.Height = v3;
}

#pragma endregion

#pragma region MouseClassExt

const MouseCursor* MouseClassExt::GetCursorData(MouseCursorType nMouse)
{
	if (!CursorTypeClass::Array.empty())
	{
		return CursorTypeClass::Array[(int)nMouse].CursorData.operator->();
	}

	// if the custom cursor array is empty , then fix then index
	if (nMouse >= MouseCursorType::count)
		nMouse = MouseCursorType::SpyPlane;

	return MouseCursor::DefaultCursors.begin() + (size_t)nMouse;
}

const MouseCursor* MouseClassExt::GetCursorDataFromRawAction(Action nAction)
{
	const auto fixedAction = MouseClassExt::GetActionIndex(nAction);
	const auto Idx = fixedAction < MouseClassExt::CursorIdx.size() ? CursorIdx[fixedAction].Idx : 0;
	return Idx != (size_t)MouseCursorType::Default ? MouseClassExt::GetCursorData((MouseCursorType)Idx) : nullptr;
}

#pragma region MappedAction

void MouseClassExt::ClearMappedAction()
{
	std::memset(CursorIdx.data(), 0, sizeof(MappedActions) * CursorIdx.size());
}

void MouseClassExt::InsertMappedAction(MouseCursorType nCursorIdx, Action nAction, bool Shrouded)
{
	CursorIdx[(size_t)nAction].Idx = (size_t)nCursorIdx;
	CursorIdx[(size_t)nAction].AllowShourd = Shrouded;
}

void MouseClassExt::InsertSWMappedAction(MouseCursorType nCursorIdx, Action nAction, bool Shrouded)
{
	switch ((AresNewActionType)nAction)
	{
	case AresNewActionType::SuperWeaponAllowed:
	{
		CursorIdx[(size_t)Action::count].Idx = (size_t)nCursorIdx;
		CursorIdx[(size_t)Action::count].AllowShourd = Shrouded;
	}
	break;
	case AresNewActionType::SuperWeaponDisallowed:
	{
		CursorIdx[(size_t)Action::count + 1].Idx = (size_t)nCursorIdx;
		CursorIdx[(size_t)Action::count + 1].AllowShourd = Shrouded;
	}
	break;
	default:
		break;
	}
}

int MouseClassExt::ByWeapon(TechnoClass* pThis, int nWeaponIdx, bool OutOfRange)
{
	//TODO :
	//if (auto pBuilding = specific_cast<BuildingClass*>(pThis)) {
	//	if(!pBuilding->IsPowerOnline())
	//		return OutOfRange ? MouseCursorType::NoEnter : MouseCursorType::Enter;
	//}

	if (const auto pWeaponS = pThis->GetWeapon(nWeaponIdx))
	{
		if (const auto pWeapon = pWeaponS->WeaponType)
		{
			const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
			return ((OutOfRange ?
				pWeaponExt->Cursor_AttackOutOfRange : pWeaponExt->Cursor_Attack).Get());
		}
	}

	return int(OutOfRange ?
	MouseCursorType::AttackOutOfRange : MouseCursorType::Attack);
}

#pragma endregion

//7E198C - vtable
void MouseClassExt::_Update(const int* keyCode, const Point2D* mouseCoords)
{
	ClearMappedAction();
	const auto pCursorData = GetCursorData(this->MouseCursorIndex);
	const auto nFrameRate = pCursorData->GetFrameRate();

	if (nFrameRate && !MouseClass::Timer->GetTimeLeft())
	{
		this->MouseCursorCurrentFrame++;
		this->MouseCursorCurrentFrame %= pCursorData->GetMouseFrameCount(this->MouseCursorIsMini);
		MouseClass::Timer->Start(nFrameRate);
		const int baseframe = pCursorData->GetMouseFrame(this->MouseCursorIsMini) + this->MouseCursorCurrentFrame;
		const auto pShape = MouseClass::ShapeData();
		WWMouseClass::Instance->Draw(pCursorData->GetMouseHotSpot(pShape), pShape, baseframe);
	}

	this->ScrollClass::Update_(keyCode, mouseCoords);
}

//7E19B0 - vtable
bool MouseClassExt::_Override_Mouse_Shape(MouseCursorType mouse, bool wsmall)
{
	const auto pCursorData = GetCursorData(mouse);

	if (pCursorData->SmallFrame == -1 || !pCursorData->SmallFrameCount)
		wsmall = false;

	if (!MouseClass::ShapeOverride()
		|| MouseClass::ShapeData()
			&& (mouse != this->MouseCursorIndex || wsmall != this->MouseCursorIsMini))
	{

		MouseClass::ShapeOverride = true;
		MouseClass::Timer->Start(pCursorData->GetFrameRate());
		this->MouseCursorCurrentFrame = 0;
		const int nFrame = pCursorData->GetMouseFrame(wsmall);
		const auto pShape = MouseClass::ShapeData();
		WWMouseClass::Instance->Draw(pCursorData->GetMouseHotSpot(pShape), pShape, nFrame);
		this->MouseCursorIndex = mouse;
		this->MouseCursorIsMini = wsmall;
		return true;
	}
	return false;
}

//7E19B8 - vtable
void MouseClassExt::_Mouse_Small(bool wsmall)
{
	if (this->MouseCursorIsMini != wsmall)
	{
		const auto pCursorData = GetCursorData(this->MouseCursorIndex);
		this->MouseCursorIsMini = wsmall;
		const int baseframe = pCursorData->GetMouseFrame(wsmall) + this->MouseCursorCurrentFrame;
		const auto pShape = MouseClass::ShapeData();
		WWMouseClass::Instance->Draw(pCursorData->GetMouseHotSpot(pShape), pShape, baseframe);
	}
}

//5BDBC0 - Not a vtable
int MouseClassExt::_Get_Mouse_Current_Frame(MouseCursorType mouse, bool wsmall) const
{
	return GetCursorData(mouse)->GetMouseFrame(wsmall) + this->MouseCursorCurrentFrame;
}

//5BDB90 - Not a vtable
int MouseClassExt::_Get_Mouse_Frame(MouseCursorType mouse, bool wsmall) const
{
	return GetCursorData(mouse)->GetMouseFrame(wsmall);
}

//5BDC00 - Not a vtable
Point2D MouseClassExt::_Get_Mouse_Hotspot(MouseCursorType mouse) const
{
	return GetCursorData(mouse)->GetMouseHotSpot(MouseClass::ShapeData());
}

//5BE970 - Not a vtable
int MouseClassExt::_Get_Mouse_Start_Frame(MouseCursorType mouse) const
{
	return GetCursorData(this->MouseCursorIndex)->StartFrame;
}

//5BE990 - Not a vtable
int MouseClassExt::_Get_Mouse_Frame_Count(MouseCursorType mouse) const
{
	return GetCursorData(this->MouseCursorIndex)->FrameCount;
}

size_t MouseClassExt::GetActionIndex(Action nAction)
{
	switch ((AresNewActionType)nAction)
	{
	case AresNewActionType::SuperWeaponAllowed:
	{
		return (size_t)Action::count;
	}
	case AresNewActionType::SuperWeaponDisallowed:
	{
		return (size_t)Action::count + 1;
	}
	default:
		return (size_t)nAction;
	}
}

MouseCursorType MouseClassExt::ValidateCursorType(Action nAction)
{
	const auto IdxToSelect = GetActionIndex(nAction);

	if (IdxToSelect < MouseClassExt::CursorIdx.size()
		&& MouseClassExt::CursorIdx[IdxToSelect].Idx != (size_t)MouseCursorType::Default)
	{
		return (MouseCursorType)MouseClassExt::CursorIdx[IdxToSelect].Idx;
	}

	switch (nAction)
	{
	case Action::Move:
		return MouseCursorType::Move;
	case Action::NoMove:
	case Action::NoIvanBomb:
		return MouseCursorType::NoMove;
	case Action::Enter:
	case Action::Capture:
	case Action::Repair:
	case Action::EnterTunnel:
		return MouseCursorType::Enter;
	case Action::Self_Deploy:
	case Action::AreaAttack:
		return MouseCursorType::Deploy;
	case Action::Attack:
		return MouseCursorType::Attack;
	case Action::Harvest:
		return MouseCursorType::AttackOutOfRange;
	case Action::Select:
	case Action::ToggleSelect:
	case Action::SelectBeacon:
		return MouseCursorType::Select;
	case Action::Eaten:
		return MouseCursorType::EngineerRepair;
	case Action::Sell:
		return MouseCursorType::Sell;
	case Action::SellUnit:
		return MouseCursorType::SellUnit;
	case Action::NoSell:
		return MouseCursorType::NoSell;
	case Action::NoRepair:
	case Action::NoGRepair:
		return MouseCursorType::NoRepair;
	case Action::Sabotage:
	case Action::Demolish:
		return MouseCursorType::Demolish;
	case Action::Tote:
		return MouseCursorType::PsychicReveal | MouseCursorType::Move_NE; // custom 87
	case Action::Nuke:
		return MouseCursorType::Nuke;
	case Action::GuardArea:
		return MouseCursorType::Protect;
	case Action::Heal:
	case Action::PlaceWaypoint:
	case Action::TibSunBug:
	case Action::EnterWaypointMode:
	case Action::FollowWaypoint:
	case Action::SelectWaypoint:
	case Action::LoopWaypointPath:
	case Action::AttackWaypoint:
	case Action::EnterWaypoint:
	case Action::PatrolWaypoint:
		return MouseCursorType::Disallowed;
	case Action::GRepair:
		return MouseCursorType::Repair;
	case Action::NoDeploy:
		return MouseCursorType::NoDeploy;
	case Action::NoEnterTunnel:
	case Action::NoEnter:
		return MouseCursorType::NoEnter;
	case Action::TogglePower:
		return MouseCursorType::NoForceShield | MouseCursorType::Move_NW; // custom 88
	case Action::NoTogglePower:
		return MouseCursorType::GeneticMutator | MouseCursorType::Move_NW; // custom 89
	case Action::IronCurtain:
		return MouseCursorType::IronCurtain;
	case Action::LightningStorm:
		return MouseCursorType::LightningStorm;
	case Action::ChronoSphere:
	case Action::ChronoWarp:
		return MouseCursorType::Chronosphere;
	case Action::ParaDrop:
	case Action::AmerParaDrop:
		return MouseCursorType::ParaDrop;
	case Action::IvanBomb:
		return MouseCursorType::IvanBomb;
	case Action::Detonate:
	case Action::DetonateAll:
		return MouseCursorType::Detonate;
	case Action::DisarmBomb:
		return MouseCursorType::Disarm;
	case Action::PlaceBeacon:
		return MouseCursorType::Beacon;
	case Action::AttackMoveNav:
	case Action::AttackMoveTar:
		return MouseCursorType::AttackOutOfRange2;
	case Action::PsychicDominator:
		return MouseCursorType::PsychicDominator;
	case Action::SpyPlane:
		return MouseCursorType::SpyPlane;
	case Action::GeneticConverter:
		return MouseCursorType::GeneticMutator;
	case Action::ForceShield:
		return MouseCursorType::ForceShield;
	case Action::NoForceShield:
		return MouseCursorType::NoForceShield;
	case Action::Airstrike:
		return MouseCursorType::AirStrike;
	case Action::PsychicReveal:
		return MouseCursorType::PsychicReveal;
	}

	return MouseCursorType::Default;
}

Action MouseClassExt::ValidateShroudedAction(Action nAction)
{
	switch (nAction)
	{
	case Action::Attack:
	{
		return Action::Move;
	}
	case Action::NoMove:
	{
		auto const& nObjDvc = ObjectClass::CurrentObjects(); //current object with cursor

		if (nObjDvc.Count)
		{
			if (auto T = generic_cast<TechnoClass*>(nObjDvc[0]))
			{
				if (T->GetTechnoType()->MoveToShroud)
				{
					return Action::Move;
				}
			}
		}

		return Action::NoMove;
	}
	break;
	case Action::Enter:
	case Action::Self_Deploy:
	case Action::Harvest:
	case Action::Select:
	case Action::ToggleSelect:
	case Action::Capture:
	case Action::Repair:
	case Action::Sabotage:
	case Action::DontUse2:
	case Action::DontUse3:
	case Action::DontUse4:
	case Action::DontUse5:
	case Action::DontUse6:
	case Action::DontUse7:
	case Action::DontUse8:
	case Action::Damage:
	case Action::GRepair:
	case Action::EnterTunnel:
	case Action::DragWaypoint:
	case Action::AreaAttack:
	case Action::IvanBomb:
	case Action::NoIvanBomb:
	case Action::Detonate:
	case Action::DetonateAll:
	case Action::DisarmBomb:
	case Action::SelectNode:
	case Action::AttackSupport:
	case Action::Demolish:
	case Action::Airstrike:
	{
		if (!MouseClassExt::CursorIdx[(size_t)nAction].AllowShourd)
			return Action::None;
		else
			break;
	}
	case Action::Eaten:
	case Action::NoGRepair:
	case Action::NoRepair:
		return Action::NoRepair;
	case Action::SellUnit:
	case Action::Sell:
		return Action::NoSell;
	case Action::TogglePower:
		return Action::NoTogglePower;
	case Action::NoEnterTunnel:
		return Action::NoEnter;
	case Action::ChronoWarp:
		return Action::ChronoSphere;
	case Action::SelectBeacon:
		return Action::Select;
	case Action::AmerParaDrop:
		return Action::ParaDrop;
	default:
		break;
	}

	return nAction;
}
#pragma endregion

void AresGlobalData::ReadAresRA2MD(CCINIClass* Ini)
{
	Debug::Log("--------- Loading Ares global settings -----------\n");

	if (Ini)
	{
		auto const section2 = GameStrings::Colors();
		colorCount = std::clamp(Ini->ReadInteger(section2, "Count", colorCount), 8, 17);

		auto const ParseColorInt = [&Ini](const char* section, const char* key, int defColor) -> int
			{
				ColorStruct ndefault(defColor & 0xFF, (defColor >> 8) & 0xFF, (defColor >> 16) & 0xFF);
				auto const color = Ini->ReadColor(section, key, ndefault);
				return color.R | color.G << 8 | color.B << 16;
			};

		auto const section = "UISettings";

		auto const ReadColor = [&Ini, section2, ParseColorInt]
		(
			const std::string& name,
			ColorData& value,
			int colorRGB,
			const char* defTooltip,
			const char* defColorScheme
		)
			{
				// load the tooltip string

				if (Ini->ReadString(section2, (name + ".Tooltip").c_str(), defTooltip, Phobos::readBuffer))
					value.sttToolTipSublineText = StringTable::LoadString(Phobos::readBuffer);

				if (Ini->ReadString(section2, (name + ".ColorScheme").c_str(), defColorScheme, Phobos::readBuffer))
					PhobosCRT::strCopy(value.colorScheme, Phobos::readBuffer);

				value.colorRGB = ParseColorInt(section2, (name + ".DisplayColor").c_str(), colorRGB);
				value.colorSchemeIndex = -1;
				value.selectedIndex = -1;
			};

		// menu colors. the color of labels, button texts, list items, stuff and others
		uiColorText = ParseColorInt(section, "Color.Text", 0xFFFF);

		// original color schemes
		static constexpr reference<int, 0x8316A8, 0x9> const DefaultColors {};
		constexpr std::string Slot_tags[] = {
			"Slot1", "Slot2", "Slot3", "Slot4",
			"Slot5", "Slot6", "Slot7", "Slot8",
			"Slot9", "Slot10", "Slot11", "Slot12",
			"Slot13", "Slot14", "Slot15", "Slot16"
		};

		ReadColor("Observer", Colors[0], DefaultColors[8], GameStrings::STT_PlayerColorObserver, GameStrings::LightGrey);
		ReadColor(Slot_tags[0], Colors[1], DefaultColors[0], GameStrings::STT_PlayerColorGold, GameStrings::LightGold);
		ReadColor(Slot_tags[1], Colors[2], DefaultColors[1], GameStrings::STT_PlayerColorRed, GameStrings::DarkRed);
		ReadColor(Slot_tags[2], Colors[3], DefaultColors[2], GameStrings::STT_PlayerColorBlue, "DarkBlue");
		ReadColor(Slot_tags[3], Colors[4], DefaultColors[3], GameStrings::STT_PlayerColorGreen, "DarkGreen");
		ReadColor(Slot_tags[4], Colors[5], DefaultColors[4], GameStrings::STT_PlayerColorOrange, "Orange");
		ReadColor(Slot_tags[5], Colors[6], DefaultColors[5], GameStrings::STT_PlayerColorSkyBlue, "DarkSky");
		ReadColor(Slot_tags[6], Colors[7], DefaultColors[6], GameStrings::STT_PlayerColorPurple, "Purple");
		ReadColor(Slot_tags[7], Colors[8], DefaultColors[7], GameStrings::STT_PlayerColorPink, "Magenta");

		// additional color schemes so just increasing Count will produce nice colors
		ReadColor(Slot_tags[8], Colors[9], 0xEF5D94, "STT:PlayerColorLilac", "NeonBlue");
		ReadColor(Slot_tags[9], Colors[10], 0xE7FF73, "STT:PlayerColorLightBlue", "LightBlue");
		ReadColor(Slot_tags[10], Colors[11], 0x63EFFF, "STT:PlayerColorLime", GameStrings::Yellow);
		ReadColor(Slot_tags[11], Colors[12], 0x5AC308, "STT:PlayerColorTeal", GameStrings::Green);
		ReadColor(Slot_tags[12], Colors[13], 0x0055BD, "STT:PlayerColorBrown", GameStrings::Red);
		ReadColor(Slot_tags[13], Colors[14], 0x808080, "STT:PlayerColorCharcoal", GameStrings::Grey);

		// blunt stuff
		ReadColor(Slot_tags[14], Colors[15], DefaultColors[8], "NOSTR:LightGrey", GameStrings::LightGrey);
		ReadColor(Slot_tags[15], Colors[16], DefaultColors[8], "NOSTR:LightGrey", GameStrings::LightGrey);

		uiColorTextButton = ParseColorInt(section, "Color.Button.Text", uiColorText);
		uiColorTextRadio = ParseColorInt(section, "Color.Radio.Text", uiColorText);
		uiColorTextCheckbox = ParseColorInt(section, "Color.Checkbox.Text", uiColorText);
		uiColorTextLabel = ParseColorInt(section, "Color.Label.Text", uiColorText);
		uiColorTextList = ParseColorInt(section, "Color.List.Text", uiColorText);
		uiColorTextCombobox = ParseColorInt(section, "Color.Combobox.Text", uiColorText);
		uiColorTextGroupbox = ParseColorInt(section, "Color.Groupbox.Text", uiColorText);
		uiColorTextSlider = ParseColorInt(section, "Color.Slider.Text", uiColorText);
		uiColorTextEdit = ParseColorInt(section, "Color.Edit.Text", uiColorText);
		uiColorTextObserver = ParseColorInt(section, "Color.Observer.Text", 0xEEEEEE);
		uiColorCaret = ParseColorInt(section, "Color.Caret", 0xFFFF);
		uiColorSelection = ParseColorInt(section, "Color.Selection", 0xFF);
		uiColorSelectionCombobox = ParseColorInt(section, "Color.Combobox.Selection", uiColorSelection);
		uiColorSelectionList = ParseColorInt(section, "Color.List.Selection", uiColorSelection);
		uiColorSelectionObserver = ParseColorInt(section, "Color.Observer.Selection", 0x626262);
		uiColorBorder1 = ParseColorInt(section, "Color.Border1", 0xC5BEA7);
		uiColorBorder2 = ParseColorInt(section, "Color.Border2", 0x807A68);
		uiColorDisabled = ParseColorInt(section, "Color.Disabled", 0x9F);
		uiColorDisabledLabel = ParseColorInt(section, "Color.Label.Disabled", uiColorDisabled);
		uiColorDisabledCombobox = ParseColorInt(section, "Color.Combobox.Disabled", uiColorDisabled);
		uiColorDisabledSlider = ParseColorInt(section, "Color.Slider.Disabled", uiColorDisabled);
		uiColorDisabledButton = ParseColorInt(section, "Color.Button.Disabled", 0xA7);
		uiColorDisabledCheckbox = ParseColorInt(section, "Color.Checkbox.Disabled", uiColorDisabled);
		uiColorDisabledList = ParseColorInt(section, "Color.List.Disabled", uiColorDisabled);
		uiColorDisabledObserver = ParseColorInt(section, "Color.Observer.Disabled", 0x8F8F8F);

		// read the mod's version info
		if (Ini->ReadString("VersionInfo", GameStrings::Name, Phobos::readDefval, Phobos::readBuffer, std::size(ModName)))
		{
			PhobosCRT::strCopy(ModName, Phobos::readBuffer);
		}

		if (Ini->ReadString("VersionInfo", "Version", Phobos::readDefval, Phobos::readBuffer, std::size(ModVersion)))
		{
			PhobosCRT::strCopy(ModVersion, Phobos::readBuffer);
		}

		AresSafeChecksummer crc;
		crc.Add(ModName, strlen(ModName));
		crc.Commit();
		crc.Add(ModVersion, strlen(ModVersion));
		ModIdentifier = Ini->ReadInteger("VersionInfo", "Identifier", static_cast<int>(crc.GetValue()));

		Debug::Log("Color count is %d\n", colorCount);
		Debug::Log("Mod is %s (%s) with %X\n",
			ModName,
			ModVersion,
			ModIdentifier
		);
	}

	Debug::Log("-------------------Complete ----------------------\n");
}

void AresGlobalData::ReadAresRA2MD()
{
	CCFileClass UIMD_ini { "UIMD.INI" };

	if (UIMD_ini.Exists() && UIMD_ini.Open(FileAccessMode::Read))
	{
		CCINIClass INI_UIMD { };
		INI_UIMD.ReadCCFile(&UIMD_ini);
		AresGlobalData::ReadAresRA2MD(&INI_UIMD);
	}
}