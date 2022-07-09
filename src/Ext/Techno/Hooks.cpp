#include "Body.h"

#include <InfantryClass.h>
#include <ScenarioClass.h>
#include <BuildingClass.h>
#include <ScenarioClass.h>
#include <UnitClass.h>
#include <JumpjetLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/House/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Cast.h>
#include <Utilities/Debug.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif

DEFINE_HOOK(0x6F42F7, TechnoClass_Init_NewEntities, 0x2)
{
	GET(TechnoClass*, pThis, ESI);

	TechnoExt::InitializeItems(pThis);
	return 0;
}

DEFINE_HOOK(0x73DE90, UnitClass_SimpleDeployer_TransferLaserTrails, 0x6)
{
	GET(UnitClass*, pUnit, ESI);

	TechnoExt::InitializeLaserTrail(pUnit, true);
#ifdef COMPILE_PORTED_DP_FEATURES
	TrailsManager::Construct(static_cast<TechnoClass*>(pUnit), true);
#endif
	return 0;
}

DEFINE_HOOK(0x702E4E, TechnoClass_Save_Killer_Techno, 0x6)
{
	GET(TechnoClass*, pKiller, EDI);
	GET(TechnoClass*, pVictim, ECX);

	if (pKiller && pVictim)
		TechnoExt::ObjectKilledBy(pVictim, pKiller);

	return 0;
}

DEFINE_HOOK(0x517D69, InfantryClass_Init_InitialStrength, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

 	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())) {
		auto strength = pTypeExt->InitialStrength.Get(R->EDX<int>());
		pThis->Health = strength;
		pThis->EstimatedHealth = strength;
	}

	return 0;
}

#define SET_INITIAL_HP(addr , destReg , name)\
DEFINE_HOOK(addr, name, 0x6) {\
GET(TechnoClass*, pThis, ESI);\
if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())) {\
R->##destReg##(pTypeExt->InitialStrength.Get(R->##destReg##<int>()));} return 0; }

SET_INITIAL_HP(0x7355C0, EAX ,UnitClass_Init_InitialStrength)
SET_INITIAL_HP(0x414057, EAX ,AircraftClass_Init_InitialStrength)
SET_INITIAL_HP(0x442C7B, ECX ,BuildingClass_Init_InitialStrength)

#undef SET_INITIAL_HP
// Issue #271: Separate burst delay for weapon type
// Author: Starkku
DEFINE_HOOK(0x6FD05E, TechnoClass_Rearm_Delay_BurstDelays, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	int burstDelay = -1;

	if (pWeaponExt->Burst_Delays.size() > (size_t)pThis->CurrentBurstIndex)
		burstDelay = pWeaponExt->Burst_Delays[pThis->CurrentBurstIndex - 1];
	else if (pWeaponExt->Burst_Delays.size() > 0)
		burstDelay = pWeaponExt->Burst_Delays[pWeaponExt->Burst_Delays.size() - 1];

	if (burstDelay >= 0)
	{
		R->EAX(burstDelay);
		return 0x6FD099;
	}

	// Restore overridden instructions
	GET(int, idxCurrentBurst, ECX);
	return idxCurrentBurst <= 0 || idxCurrentBurst > 4 ? 0x6FD084 : 0x6FD067;
}

DEFINE_HOOK(0x6F3B37, TechnoClass_Transform_6F3AD0_BurstFLH_1, 0x7)
{
	GET(TechnoClass*, pThis, EBX);
	GET_STACK(int, weaponIndex, STACK_OFFS(0xD8, -0x8));

	std::pair<bool,CoordStruct> nResult = TechnoExt::GetBurstFLH(pThis, weaponIndex);

	if (!nResult.first && pThis->WhatAmI() == AbstractType::Infantry) {
		nResult = TechnoExt::GetInfantryFLH(static_cast<InfantryClass*>(pThis), weaponIndex);
	}

	if (nResult.first)
	{
		R->ECX(nResult.second.X);
		R->EBP(nResult.second.Y);
		R->EAX(nResult.second.Z);
	}

	return 0;
}

DEFINE_HOOK(0x6F3C88, TechnoClass_Transform_6F3AD0_BurstFLH_2, 0x6)
{
	GET(TechnoClass*, pThis, EBX);
	GET_STACK(int, weaponIndex, STACK_OFFS(0xD8, -0x8));

	if (TechnoExt::GetBurstFLH(pThis, weaponIndex).first)
		R->EAX(0);

	return 0;
}

// Issue #237 NotHuman additional animations support
// Author: Otamaa
DEFINE_HOOK(0x518505, InfantryClass_TakeDamage_NotHuman, 0x4)
{
	enum { Delete = 0x518619, DoOtherAffects = 0x518515 };
	GET(InfantryClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, receiveDamageArgs, STACK_OFFS(0xD0, -0x4));

	// Die1-Die5 sequences are offset by 10
	constexpr auto Die = [](int x) { return x + 10; };

	int resultSequence = Die(1);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	R->ECX(pThis);

	if (pTypeExt->NotHuman_RandomDeathSequence.Get())
		resultSequence = ScenarioClass::Instance->Random.RandomRanged(Die(1), Die(5));

	if (receiveDamageArgs.WH)
	{
		if (auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(receiveDamageArgs.WH))
		{
			if (auto pDeathAnim = pWarheadExt->NotHuman_DeathAnim.Get(nullptr))
			{
				if (auto pAnim = GameCreate<AnimClass>(pDeathAnim, pThis->Location)) {
					auto pInvoker = receiveDamageArgs.Attacker ? receiveDamageArgs.Attacker->GetOwningHouse() : nullptr;
					AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->GetOwningHouse(), receiveDamageArgs.Attacker, true);
					pAnim->ZAdjust = pThis->GetZAdjustment();
					return Delete;
				}
			}
			else
			{
				int whSequence = pWarheadExt->NotHuman_DeathSequence.Get();
				if (whSequence > 0)
					resultSequence = Math::min(Die(whSequence), Die(5));
			}
		}
	}

	//BugFix : when the sequence not declared , it keep the infantry alive ! , wtf WW ?!
	return (!pThis->PlayAnim(static_cast<DoType>(resultSequence), true)) ? Delete : DoOtherAffects;
}

// Customizable OpenTopped Properties
// Author: Otamaa

DEFINE_HOOK(0x6F72D2, TechnoClass_IsCloseEnoughToTarget_OpenTopped_RangeBonus, 0xC)
{
	GET(TechnoClass* const, pThis, ESI);

	if (auto pTransport = pThis->Transporter)
	{
		if (auto pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			R->EAX(pExt->OpenTopped_RangeBonus.Get(RulesClass::Instance->OpenToppedRangeBonus));
			return 0x6F72DE;
		}
	}

	return 0;
}

DEFINE_HOOK(0x71A82C, TemporalClass_AI_Opentopped_WarpDistance, 0xC)
{
	GET(TemporalClass* const, pThis, ESI);

	if (auto pTransport = pThis->Owner->Transporter)
	{
		if (auto pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			R->EDX(pExt->OpenTopped_WarpDistance.Get(RulesClass::Instance->OpenToppedWarpDistance));
			return 0x71A838;
		}
	}

	return 0;
}

DEFINE_HOOK(0x7098B9, TechnoClass_TargetSomethingNearby_AutoFire, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	if (auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (pExt->AutoFire)
		{
			if (pExt->AutoFire_TargetSelf)
				pThis->SetTarget(pThis);
			else
				pThis->SetTarget(pThis->GetCell());

			return 0x7099B8;
		}
	}

	return 0;
}

DEFINE_HOOK(0x702819, TechnoClass_ReceiveDamage_Decloak, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0xC4, -0xC));

	bool AllowDecloak = true;
	if (auto pExt = WarheadTypeExt::ExtMap.Find(pWarhead)) {
		AllowDecloak = pExt->DecloakDamagedTargets.Get();
	}

	if(AllowDecloak)
		pThis->Uncloak(false);

	return 0x702823;
}

DEFINE_HOOK(0x71067B, TechnoClass_EnterTransport_LaserTrails, 0x7)
{
	GET(TechnoClass*, pTechno, EDI);

	if (auto pTechnoExt = TechnoExt::GetExtData(pTechno))
	{
		if (pTechnoExt->LaserTrails.size())
		{
			for (auto const& pLaserTrail : pTechnoExt->LaserTrails)
			{
				pLaserTrail->Visible = false;
				pLaserTrail->LastLocation.Reset();
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x5F4F4E, ObjectClass_Unlimbo_LaserTrails, 0x7)
{
	GET(ObjectClass*, pThis, ECX);

	if (auto pTechno = generic_cast<TechnoClass*>(pThis))
	{
		if (auto pTechnoExt = TechnoExt::GetExtData(pTechno))
		{
			if (!pTechnoExt->LaserTrails.empty())
			{
				for (auto const& pLaserTrail : pTechnoExt->LaserTrails)
				{
					if (pLaserTrail)
					{
						pLaserTrail->LastLocation.Reset();
						pLaserTrail->Visible = true;
					}
				}
			}
#ifdef COMPILE_PORTED_DP_FEATURES
			TrailsManager::Hide(pTechno);
#endif
		}
	}

	return 0;
}

// Update ammo rounds
DEFINE_HOOK(0x6FB086, TechnoClass_Reload_ReloadAmount, 0x8)
{
	GET(TechnoClass* const, pThis, ECX);

	TechnoExt::UpdateSharedAmmo(pThis);

	return 0;
}

DEFINE_HOOK(0x6FD446, TechnoClass_FireLaser_IsSingleColor, 0x7)
{
	GET(WeaponTypeClass* const, pWeapon, ECX);
	GET(LaserDrawClass* const, pLaser, EAX);

	if (auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		if (!pLaser->IsHouseColor && pWeaponExt->Laser_IsSingleColor)
			pLaser->IsHouseColor = true;
	}

	return 0;
}

DEFINE_HOOK(0x701DFF, TechnoClass_ReceiveDamage_FlyingStrings, 0x7)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(int* const, pDamage, EBX);

	if (Phobos::Debug_DisplayDamageNumbers && *pDamage)
		TechnoExt::DisplayDamageNumberString(pThis, *pDamage, false);

	return 0;
}

DEFINE_HOOK(0x6FA793, TechnoClass_AI_SelfHealGain, 0x5)
{
	enum { SkipGameSelfHeal = 0x6FA941 };

	GET(TechnoClass*, pThis, ESI);

	TechnoExt::ApplyGainedSelfHeal(pThis);

	return SkipGameSelfHeal;
}

DEFINE_HOOK(0x70A4FB, TechnoClass_Draw_Pips_SelfHealGain, 0x5)
{
	enum { SkipGameDrawing = 0x70A6C0 };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x74, -0x4));
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFS(0x74, -0xC));

	TechnoExt::DrawSelfHealPips(pThis, pLocation, pBounds);

	return SkipGameDrawing;
}

DEFINE_HOOK(0x54AEC0, JumpjetLoco_Process, 0x8)
{
	GET_STACK(ILocomotion*, iLoco, 0x4);
	const auto pLoco = static_cast<JumpjetLocomotionClass*>(iLoco);
	const auto pThis = pLoco->Owner;
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt && pTypeExt->JumpjetTurnToTarget.Get(RulesExt::Global()->JumpjetTurnToTarget) &&
		pThis->WhatAmI() == AbstractType::Unit && pThis->IsInAir() && !pType->TurretSpins && pLoco)
	{
		if (const auto pTarget = pThis->Target)
		{
			const CoordStruct source = pThis->Location;
			const CoordStruct target = pTarget->GetCoords();
			const DirStruct tgtDir = DirStruct(Math::arctanfoo(source.Y - target.Y, target.X - source.X));
			if (pThis->GetRealFacing().current().value32() != tgtDir.value32())
				pLoco->Facing.turn(tgtDir);
		}
	}
	return 0;
}

//#ifdef DISGUISE_HOOKS
DEFINE_HOOK(0x6F534E, TechnoClass_DrawExtras_Insignia, 0x5)
{
	enum { SkipGameCode = 0x6F5388 };

	GET(TechnoClass*, pThis, EBP);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x98, -0x4));
	GET(RectangleStruct*, pBounds, ESI);

	if (pThis->VisualCharacter(false, nullptr) != VisualType::Hidden)
		TechnoExt::DrawInsignia(pThis, pLocation, pBounds);

	return SkipGameCode;
}
//#endif

DEFINE_HOOK(0x70EFE0, TechnoClass_GetMaxSpeed, 0x6)
{
	enum { SkipGameCode = 0x70EFF2 };

	GET(TechnoClass*, pThis, ECX);

	int maxSpeed = 0;

	if (pThis && pThis->GetTechnoType())
	{
		maxSpeed = pThis->GetTechnoType()->Speed;
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pTypeExt && pTypeExt->UseDisguiseMovementSpeed && pThis->IsDisguised())
		{
			if (auto const pType = type_cast<TechnoTypeClass*>(pThis->Disguise))
				maxSpeed = pType->Speed;
		}
	}

	R->EAX(maxSpeed);
	return SkipGameCode;
}

DEFINE_HOOK_AGAIN(0x6B73BE, SpawnManagerClass_AI_SpawnTimer, 0x6)
DEFINE_HOOK(0x6B73AD, SpawnManagerClass_AI_SpawnTimer, 0x5)
{
	GET(SpawnManagerClass* const, pThis, ESI);

	if (pThis->Owner)
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType()))
		{
			if (pTypeExt->Spawner_DelayFrames.isset())
				R->ECX(pTypeExt->Spawner_DelayFrames.Get());
		}
	}

	return 0;
}

DEFINE_HOOK(0x6B7265, SpawnManagerClass_AI_UpdateTimer, 0x6)
{
	GET(SpawnManagerClass* const, pThis, ESI);

	if (pThis->Owner && pThis->Status == SpawnManagerStatus::Launching && pThis->CountDockedSpawns() != 0)
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType()))
		{
			if (pTypeExt->Spawner_DelayFrames.isset()) {
				R->EAX(std::min(pTypeExt->Spawner_DelayFrames.Get(), 10));
			}
		}
	}

	return 0;
}

// Reimplements the game function with few changes / optimizations
DEFINE_HOOK(0x7012C0, TechnoClass_WeaponRange, 0x4)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIndex,0x4);

	int result = 0;

	if (const auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType)
	{
		result = pWeapon->Range;
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (!pTypeExt)
			return 0x0;

		if (pThis->GetTechnoType()->OpenTopped && !pTypeExt->OpenTopped_IgnoreRangefinding.Get())
		{
			int smallestRange = INT32_MAX;
			auto pPassenger = pThis->Passengers.FirstPassenger;

			while (pPassenger && (pPassenger->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None) {
				int openTWeaponIndex = pPassenger->GetTechnoType()->OpenTransportWeapon;
				int tWeaponIndex = 0;

				if (openTWeaponIndex != -1)
					tWeaponIndex = openTWeaponIndex;
				else if (pPassenger->HasTurret())
					tWeaponIndex = pPassenger->CurrentWeaponNumber;

				if (WeaponTypeClass* pTWeapon = pPassenger->GetWeapon(tWeaponIndex)->WeaponType) {
					if (pTWeapon->Range < smallestRange)
						smallestRange = pTWeapon->Range;
				}
				//else{
				//Debug::Log("Techno[%s] With Passengers[%s] Failed To get WeaponType To Check Range ! \n",pThis->get_ID(),pPassenger->get_ID()); }


				pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
			}

			if (result > smallestRange)
				result = smallestRange;
		}
	}

	if (result == 0 && pThis->GetTechnoType()->OpenTopped && pThis->WhatAmI() == AbstractType::Aircraft) {
		result = pThis->GetTechnoType()->GuardRange;
		if(result == 0)
			Debug::Log("Warning ! , range of Aircraft[%s] return 0 result will cause Aircraft to stuck ! \n", pThis->get_ID());
	}

	R->EAX(result);
	return 0x701393;
}

/*
DEFINE_HOOK(0x4DEAEE, FootClass_IronCurtain, 0x6)
{
	GET(FootClass*, pThis, ECX);
	if (pTypeExt->CanBeIronCurtain)
		return 0x4DEB38;
	return 0;
}*/

DEFINE_HOOK(0x522600, InfantryClass_IronCurtain, 0x6)
{
	GET(InfantryClass*, pThis, ECX);
	GET_STACK(int, nDuration, 0x4);
	GET_STACK(HouseClass*, pSource, 0x8);
	GET_STACK(bool, ForceShield, 0xC);

	R->EAX(pThis->FootClass::IronCurtain(nDuration, pSource, ForceShield));
	return 0x522639;
}

/*
static bool Kill = false;

DEFINE_HOOK(0x457C90, BuildingClass_IronCuratin, 0x6)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(HouseClass*, pSource, 0x8);

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())) {
		if (Kill && pThis->IsAlive && pThis->Health > 0 && !pThis->TemporalTargetingMe) {
			R->EAX(pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pSource));
			return 0x457CDB;
		}
	}

	return 0;
}

DEFINE_HOOK(0x4DEAEE, FootClass_IronCurtain, 0x6)
{
	GET(FootClass*, pThis, ECX);
	GET_STACK(HouseClass*, pSource, 0x0);
	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	IronCurtainAffects ironAffect = IronCurtainAffects::Affect;

	if (pThis->WhatAmI() != AbstractType::Infantry)
		pSource = R->Stack<HouseClass*>(0x18);

	if (pType->Organic || pThis->WhatAmI() == AbstractType::Infantry)
	{
		if (pTypeExt->IronCurtain_Affect.isset())
			ironAffect = pTypeExt->IronCurtain_Affect.Get();
		else
			ironAffect = RulesExt::Global()->IronCurtainToOrganic.Get();
	}
	else
	{
		if (pTypeExt->IronCurtain_Affect.isset())
			ironAffect = pTypeExt->IronCurtain_Affect.Get();
	}

	if (ironAffect == IronCurtainAffects::Kill)
	{
		R->EAX(pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pSource));
	}
	else if (ironAffect == IronCurtainAffects::Affect)
	{
		R->ESI(pThis);
		return 0x4DEB38;
	}

	return 0x4DEBA2;
}*/
#include <SlaveManagerClass.h>

DEFINE_HOOK(0x6B0B9C, SlaveManagerClass_Killed_DecideOwner, 0x8)
{
	enum { KillTheSlave = 0x6B0BDF ,SkipSetEax = 0x6B0BB4 };

	GET_STACK(const SlaveManagerClass*, pThis, STACK_OFFS(0x24, 0x10));
	GET(InfantryClass*, pSlave, ESI);
	GET(TechnoClass*, pKiller, EBX);
	GET_STACK(HouseClass*, pDefaultRetHouse, STACK_OFFS(0x24, 0x14));

	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pSlave->GetTechnoType()))
	{
		if (pTypeExt->Death_WithMaster.Get() || pTypeExt->Slaved_ReturnTo == SlaveReturnTo::Suicide)
			return KillTheSlave;

		const auto pVictim = pThis->Owner ? pThis->Owner->GetOwningHouse() : pSlave->GetOwningHouse();
		R->EAX(HouseExt::GetSlaveHouse(pTypeExt->Slaved_ReturnTo, pKiller->GetOwningHouse(), pVictim ? pVictim : pDefaultRetHouse));
		return SkipSetEax;
	}

	return 0x0;
}

DEFINE_HOOK(0x443C81, BuildingClass_ExitObject_InitialClonedHealth, 0x7)
{
	GET(BuildingClass*, pBuilding, ESI);
	GET(FootClass*, pFoot, EDI);

	if (pBuilding && pBuilding->Type->Cloning && pFoot) {
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->GetTechnoType())) {
			if (auto const pTypeUnit = pFoot->GetTechnoType()) {
				auto const& [rangeX, rangeY] = pTypeExt->InitialStrength_Cloning.Get();

				if (rangeX || rangeY) {
					const double percentage = rangeX >= rangeY ? rangeX :
					static_cast<double>(ScenarioClass::Instance->Random.RandomRanged(static_cast<int>(rangeX * 100), static_cast<int>(rangeY * 100)) / 100.0);
					const int strength = Math::clamp(static_cast<int>(pTypeUnit->Strength * percentage), 1, pTypeUnit->Strength);
					pFoot->Health = strength;
					pFoot->EstimatedHealth = strength;
				}
			}
		}
	}

	return 0;
}

// Basically a hack to make game and Ares pick laser properties from non-Primary weapons.
DEFINE_HOOK(0x70E1A5, TechnoClass_GetTurretWeapon_LaserWeapon, 0x6)
{
	enum { ReturnResult = 0x70E1C7, Continue = 0x70E1AB };

	GET(TechnoClass* const, pThis, ESI);

	if (pThis->WhatAmI() == AbstractType::Building) {
		if (auto const pExt = TechnoExt::GetExtData(pThis)) {
			if (!pExt->CurrentLaserWeaponIndex.empty()) {
				R->EAX(pThis->GetWeapon(pExt->CurrentLaserWeaponIndex.get()));
				return ReturnResult;
			}
		}
	}

	// Restore overridden instructions.
	R->EAX(pThis->GetTechnoType());
	return Continue;
}