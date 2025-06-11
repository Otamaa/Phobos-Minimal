#include "Body.h"

#include <InfantryClass.h>
#include <ScenarioClass.h>
#include <BuildingClass.h>
#include <UnitClass.h>
#include <SlaveManagerClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/House/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Infantry/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Cast.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>
#include <New/PhobosAttachedAffect/Functions.h>
#include <New/Entity/FlyingStrings.h>

#include <Misc/SyncLogging.h>

#include <BombClass.h>
#include <SpawnManagerClass.h>

// ASMJIT_PATCH(0x448277 , BuildingClass_SetOwningHouse_Additionals , 5)
// {
// 	GET(BuildingClass* const, pThis, ESI);
// 	REF_STACK(bool, announce, STACK_OFFSET(0x58, 0x8));
//
// 	pThis->NextMission();
//
// 	announce = announce && !pThis->Type->IsVehicle();
//
// 	if(announce && (pThis->Type->Powered || pThis->Type->PoweredSpecial))
// 		pThis->UpdatePowerDown();
//
// 	return 0x0;
// }

//ASMJIT_PATCH(0x4483C0, BuildingClass_SetOwningHouse_MuteSound, 0x6)
//{
//	GET(BuildingClass* const, pThis, ESI);
//	REF_STACK(bool, announce, STACK_OFFSET(0x60, 0x8));
//
//	return announce ? 0 : 0x44848F; //early bailout
//}

// AFAIK, only used by the teleport of the Chronoshift SW
ASMJIT_PATCH(0x70337D, HouseClass_RegisterDestruction_SaveKillerInfo, 0x6)
{
	GET(HouseClass*, pHouse, EDI);
	GET(TechnoClass*, pVictim, ESI);

	TechnoExtData::ObjectKilledBy(pVictim, pHouse);

	return 0;
}

ASMJIT_PATCH(0x51AA40, InfantryClass_Assign_Destination_DisallowMoving, 0x5)
{
	GET(InfantryClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pDest, 0x4);
	GET_STACK(unsigned int, callerAddress, 0x0);

	//const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (!SyncLogger::HooksDisabled)
		SyncLogger::AddDestinationChangeSyncLogEvent(pThis, pDest, callerAddress);

	//if (pExt->IsWebbed && pThis->ParalysisTimer.HasTimeLeft())
	//{
	//	if (pThis->Target)
	//	{
	//		pThis->SetTarget(nullptr);
	//		pThis->FootClass::SetDestination(nullptr, false);
	//		pThis->QueueMission(Mission::Sleep, false);
	//	}
	//
	//	return 0x51B1DE;
	//}

	return 0;
}

ASMJIT_PATCH(0x702E4E, TechnoClass_RegisterDestruction_SaveKillerInfo, 0x6)
{
	GET(TechnoClass*, pKiller, EDI);
	GET(TechnoClass*, pVictim, ECX);

	TechnoExtData::ObjectKilledBy(pVictim, pKiller);

	return 0;
}

// TunnelLocomotionClass_IsToHaveShadow, skip shadow on all but idle.
// TODO: Investigate if it is possible to fix the shadows not tilting on the burrowing etc. states.
//DEFINE_JUMP(LJMP, 0x72A070, 0x72A07F);

#include <Locomotor/TunnelLocomotionClass.h>

Matrix3D* __stdcall TunnelLocomotionClass_ShadowMatrix(ILocomotion* iloco, Matrix3D* ret, VoxelIndexKey* key)
{
	auto tLoco = static_cast<TunnelLocomotionClass*>(iloco);
	tLoco->LocomotionClass::Shadow_Matrix(ret, key);

	if (tLoco->State != TunnelLocomotionClass::State::IDLE)
	{
		double theta = 0.;
		switch (tLoco->State)
		{
		case TunnelLocomotionClass::State::DIGGING:
			if (key)key->Invalidate();
			theta = Math::HalfPi;
			if (auto total = tLoco->Timer.Rate)
				theta *= 1.0 - double(tLoco->Timer.GetTimeLeft()) / double(total);
			break;
		case TunnelLocomotionClass::State::DUG_IN:
			theta = Math::HalfPi;
			break;
		case TunnelLocomotionClass::State::PRE_DIG_OUT:
			theta = -Math::HalfPi;
			break;
		case TunnelLocomotionClass::State::DIGGING_OUT:
			if (key)key->Invalidate();
			theta = -Math::HalfPi;
			if (auto total = tLoco->Timer.Rate)
				theta *= double(tLoco->Timer.GetTimeLeft()) / double(total);
			break;
		case TunnelLocomotionClass::State::DUG_OUT:
			if (key)key->Invalidate();
			theta = Math::HalfPi;
			if (auto total = tLoco->Timer.Rate)
				theta *= double(tLoco->Timer.GetTimeLeft()) / double(total);
			break;
		default:break;
		}
		ret->ScaleX((float)Math::cos(theta));// I know it's ugly
	}
	return ret;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5A4C, TunnelLocomotionClass_ShadowMatrix);

ASMJIT_PATCH(0x708FC0, TechnoClass_ResponseMove_Pickup, 0x5)
{
	enum { SkipResponse = 0x709015 };

	GET(TechnoClass*, pThis, ECX);

	if (auto const pAircraft = cast_to<AircraftClass*, false>(pThis))
	{
		if (pAircraft->Type->Carryall && pAircraft->HasAnyLink() &&
			pAircraft->Destination && (pAircraft->Destination->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
		{

			auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pAircraft->Type);

			if (!pTypeExt->VoicePickup.empty())
			{

				pThis->QueueVoice(pTypeExt->VoicePickup[Random2Class::NonCriticalRandomNumber->Random() & pTypeExt->VoicePickup.size()]);

				R->EAX(1);
				return SkipResponse;
			}
		}
	}

	return 0;
}

ASMJIT_PATCH(0x71A82C, TemporalClass_AI_Opentopped_WarpDistance, 0x6) //C
{
	GET(TemporalClass* const, pThis, ESI);

	if (auto const pTransport = pThis->Owner->Transporter)
	{
		if(pTransport->IsAlive) {
			R->EDX(TechnoTypeExtContainer::Instance.Find(pTransport->GetTechnoType())
				->OpenTopped_WarpDistance.Get(RulesClass::Instance->OpenToppedWarpDistance));
			return 0x71A838;
		}
	}

	return 0;
}

#include <Ext/Super/Body.h>

ASMJIT_PATCH(0x6F6CFE, TechnoClass_Unlimbo_LaserTrails, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	//const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (!pExt->LaserTrails.empty())
	{
		for (auto& pLaserTrail : pExt->LaserTrails)
		{
			pLaserTrail.LastLocation.clear();
			pLaserTrail.Visible = true;
		}
	}

	TrailsManager::Hide(pThis);

	return 0;
}

ASMJIT_PATCH(0x6FB086, TechnoClass_Reload_ReloadAmount_UpdateSharedAmmo, 0x8)
{
	GET(TechnoClass* const, pThis, ECX);

	TechnoExtData::UpdateSharedAmmo(pThis);

	return 0;
}

ASMJIT_PATCH(0x70A4FB, TechnoClass_Draw_Pips_SelfHealGain, 0x5)
{
	enum { SkipGameDrawing = 0x70A6C0 };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x74, -0x4));
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFS(0x74, -0xC));

	const auto pType = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pType->NoExtraSelfHealOrRepair)
		return SkipGameDrawing;

	// if (const auto pFoot = generic_cast<FootClass*>(pThis)){
	// 	if (const auto pParasiteFoot = pFoot->ParasiteEatingMe) {
	// 		TechnoExtData::DrawParasitedPips(pFoot, pLocation, pBounds);
	// 	}
	// }

	TechnoExtData::DrawSelfHealPips(pThis, pLocation, pBounds);

	return SkipGameDrawing;
}

ASMJIT_PATCH(0x70EFE0, TechnoClass_GetMaxSpeed, 0x8) //6
{
	enum { SkipGameCode = 0x70EFF2 };

	GET(TechnoClass*, pThis, ECX);

	int maxSpeed = 0;

	if (auto pType = pThis->GetTechnoType() )
	{
		if (TechnoTypeExtContainer::Instance.Find(pType)->UseDisguiseMovementSpeed)
			pType = TechnoExtData::GetSimpleDisguiseType(pThis, false, false);

		maxSpeed = pType->Speed;
	}

	R->EAX(maxSpeed);
	return SkipGameCode;
}

ASMJIT_PATCH(0x6B0B9C, SlaveManagerClass_Killed_DecideOwner, 0x6) //0x8
{
	//Kill slave is buged because it doesnt do IgnoreDamage -Otamaa
	enum { KillTheSlave = 0x6B0BDF, SkipSetEax = 0x6B0BB4, LoopCheck = 0x6B0C0B };

	GET_STACK(const SlaveManagerClass*, pThis, STACK_OFFS(0x24, 0x10));
	GET(InfantryClass*, pSlave, ESI);
	GET(TechnoClass*, pKiller, EBX);
	GET_STACK(HouseClass*, pDefaultRetHouse, STACK_OFFS(0x24, 0x14));

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pSlave->Type);
	{
		if (pTypeExt->Death_WithMaster.Get() || pTypeExt->Slaved_ReturnTo == SlaveReturnTo::Suicide)
		{
			auto nStr = pSlave->Health;
			pSlave->ReceiveDamage(&nStr, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);
			return LoopCheck;
		}

		const auto pVictim = pThis->Owner ? pThis->Owner->GetOwningHouse() : pSlave->GetOwningHouse();
		R->EAX(HouseExtData::GetSlaveHouse(pTypeExt->Slaved_ReturnTo, pKiller->GetOwningHouse(), pVictim ? pVictim : pDefaultRetHouse));
		return SkipSetEax;
	}

	//	return 0x0;
}

#include <Misc/Ares/Hooks/Header.h>

ASMJIT_PATCH(0x6FD054, TechnoClass_RearmDelay_ForceFullDelay, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET(int, currentBurstIdx, ECX);

	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	pExt->LastRearmWasFullDelay = false;

	bool rearm = currentBurstIdx >= pWeapon->Burst;

	if (pExt->ForceFullRearmDelay) {
		pExt->ForceFullRearmDelay = false;
		pThis->CurrentBurstIndex = 0;
		rearm = true;
	}

	if (!rearm)
	{
		const int burstDelay = WeaponTypeExtData::GetBurstDelay(pWeapon, pThis->CurrentBurstIndex);

		if (burstDelay >= 0)
		{
			R->EAX(burstDelay);
			return 0x6FD099;
		}

		// Restore overridden instructions
		return currentBurstIdx <= 0 || currentBurstIdx > 4 ? 0x6FD084 : 0x6FD067;
	}

	TechnoExtContainer::Instance.Find(pThis)->LastRearmWasFullDelay = true;
	int nResult = 0;
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->ROF_Random.Get())
	{
		const auto nDefault = Point2D { RulesExtData::Instance()->ROF_RandomDelay->X , RulesExtData::Instance()->ROF_RandomDelay->Y };
		nResult += GeneralUtils::GetRangedRandomOrSingleValue(pTypeExt->Rof_RandomMinMax.Get(nDefault));
	}

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (pWeaponExt->ROF_RandomDelay.isset())
	{
		nResult += GeneralUtils::GetRangedRandomOrSingleValue(pWeaponExt->ROF_RandomDelay);
	}

	int _ROF = int(
		(double)pWeapon->ROF *
		TechnoExtContainer::Instance.Find(pThis)->AE.ROFMultiplier *
		pThis->Owner->ROFMultiplier + nResult
	);

	if (pThis->HasAbility(AbilityType::ROF))
		_ROF = int(_ROF * RulesClass::Instance->VeteranROF);

	auto const Building = cast_to<BuildingClass*, false>(pThis);

	if (pThis->CanOccupyFire())
	{
		const auto occupant = pThis->GetOccupantCount();

		if (occupant > 0)
		{
			_ROF /= occupant;
		}

		auto OccupyRofMult = RulesClass::Instance->OccupyROFMultiplier;

		if (Building)
		{
			OccupyRofMult = BuildingTypeExtContainer::Instance.Find(Building->Type)->BuildingOccupyROFMult.Get(OccupyRofMult);
		}

		if (OccupyRofMult > 0.0)
			_ROF = int(float(_ROF) / OccupyRofMult);

	}

	if (pThis->BunkerLinkedItem && !Building)
	{
		auto BunkerMult = RulesClass::Instance->BunkerROFMultiplier;
		if (auto const pBunkerIsBuilding = cast_to<BuildingClass*, false>(pThis->BunkerLinkedItem))
		{
			BunkerMult = BuildingTypeExtContainer::Instance.Find(pBunkerIsBuilding->Type)->BuildingBunkerROFMult.Get(BunkerMult);
		}

		if (BunkerMult != 0.0)
			_ROF = int(float(_ROF) / BunkerMult);
	}

	R->EAX(_ROF);
	return 0x6FD1F5;
}

// TODO :
// yeah , fuckers !!
  //struct VampireState {
	// struct Data {
	//   bool Enabled;
	//   bool AffectAir;
	//   AffectedHouse Affected_House;
	//   AbstractType Affected_abs;
	//   HelperedVector<TechnoTypeClass*> Affected_Types;
	//   HelperedVector<TechnoTypeClass*> Exclude_Types;
	//   Vector2D<double> Chances;
	//   double Percent;
	//   int TriggeredTimes;
	// };

	// HelperedVector<Data> Packed {};

	// consteval void Clear() {
	//	 Packed.clear();
	// }

	// COMPILETIMEEVAL bool Enabled() {
	//	 return !Packed.empty();
	// }

	// //only add the data that can affect this current techno
	// void Init()
	// {

	// }

	// //update trigger count
	// void Trigger() {
	//	 for (auto& data : Packed) {
	//		 if (data.Enabled && data.TriggeredTimes > 0) {
	//			 --data.TriggeredTimes;

	//			 if (data.TriggeredTimes <= 0)
	//				 data.Enabled = false;
	//		 }
	//	 }
	// }

	// // apply the multiplier to the attacker ??
	// void Apply() {

	// }
	// COMPILETIMEEVAL bool Eligible(TechnoClass* attacker, HouseClass* attackerOwner , bool isInAir) {




	//	 return true;
	// }
 //};

// void NOINLINE ApplyVampire(int* pRealDamage, WarheadTypeClass* pWH, DamageState damageState, TechnoClass* pAttacker, HouseClass* pAttackingHouse)
// {
// 	if(!pAttacker || !pAttacker->IsAlive || pAttacker->IsCrashing || pAttacker->IsSinking || pAttacker->TemporalTargetingMe)
// 		return;

// 	bool inAir = pTechno->IsInAir();

// 	vampireEffect->Trigger();
// 	int damage = -(int)(*pRealDamage * ae->AEData.Vampire.Percent);
// 	if (damage != 0) {
// 		pAttacker->TakeDamage(damage, pAttacker->GetTechnoType()->Crewed, true, pAttacker, pAttackingHouse);
// 	}
// }

ASMJIT_PATCH(0x4D9992, FootClass_PointerGotInvalid_Parasite, 0x7)
{
	enum { SkipGameCode = 0x4D99D3 };

	GET(FootClass*, pThis, ESI);
	GET(AbstractClass*, pAbstract, EDI);
	GET(FootClass*, pParasiteOwner, EAX);
	GET(bool, bRemoved, EBX);

	if (pParasiteOwner == pAbstract && (!pParasiteOwner->Health || !Game::InScenario2()))
	{
		pThis->ParasiteEatingMe = nullptr;
		return SkipGameCode;
	}

	if (!bRemoved)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
		bRemoved = pTypeExt->Cloak_KickOutParasite.Get(RulesExtData::Instance()->Cloak_KickOutParasite);
	}

	if (pParasiteOwner && pParasiteOwner->Health > 0)
		pParasiteOwner->ParasiteImUsing->PointerExpired(pAbstract, bRemoved);

	if (pThis == pAbstract && bRemoved)
		pThis->ParasiteEatingMe = nullptr;

	return SkipGameCode;
}

ASMJIT_PATCH(0x709B2E, TechnoClass_DrawPips_Sizes, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	REF_STACK(int, pipWidth, STACK_OFFSET(0x74, -0x1C));

	const bool isBuilding = pThis->WhatAmI() == AbstractType::Building;

	Point2D size = Point2D::Empty;
	const auto pType = pThis->GetTechnoType();

	if (pType->PipScale == PipScale::Ammo)
	{
		size = TechnoTypeExtContainer::Instance.Find(pType)->AmmoPipSize.Get((isBuilding ?
			RulesExtData::Instance()->Pips_Ammo_Buildings_Size : RulesExtData::Instance()->Pips_Ammo_Size));
	}
	else
	{
		size = (isBuilding ? RulesExtData::Instance()->Pips_Generic_Buildings_Size : RulesExtData::Instance()->Pips_Generic_Size).Get();
	}

	pipWidth = size.X;
	R->ESI(size.Y);

	return 0;
}

ASMJIT_PATCH(0x70A36E, TechnoClass_DrawPips_Ammo, 0x6)
{
	enum { SkipGameDrawing = 0x70A4EC };

	GET(TechnoClass*, pThis, ECX);
	LEA_STACK(RectangleStruct*, offset, STACK_OFFSET(0x74, -0x24));
	GET_STACK(RectangleStruct*, rect, STACK_OFFSET(0x74, 0xC));
	GET(int, pipWrap, EBX);
	GET_STACK(int, pipCount, STACK_OFFSET(0x74, -0x54));
	GET_STACK(int, maxPips, STACK_OFFSET(0x74, -0x60));
	GET(int, yOffset, ESI);
	GET_STACK(SHPStruct*, pDefault, 0x74 - 0x48);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	Point2D position = { offset->X + pTypeExt->AmmoPipOffset->X, offset->Y + pTypeExt->AmmoPipOffset->Y };
	ConvertClass* pConvert = pTypeExt->AmmoPip_Palette.GetConvert() ?
		pTypeExt->AmmoPip_Palette.GetConvert()
		: FileSystem::PALETTE_PAL();

	auto pSHApe = pTypeExt->AmmoPip_shape.Get(pDefault);

	if (pipWrap > 0)
	{
		int levels = maxPips / pipWrap - 1;

		for (int i = 0; i < pipWrap; i++)
		{
			int frame = pTypeExt->PipWrapAmmoPip;

			if (levels >= 0)
			{
				int counter = i + pipWrap * levels;
				int frameCounter = levels;
				bool calculateFrame = true;

				while (counter >= pThis->Ammo)
				{
					frameCounter--;
					counter -= pipWrap;

					if (frameCounter < 0)
					{
						calculateFrame = false;
						break;
					}
				}

				if (calculateFrame)
					frame = frameCounter + frame + 1;
			}

			position.X += offset->Width;
			position.Y += yOffset;

			DSurface::Temp->DrawSHP(pConvert, pSHApe,
				frame, &position, rect, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}
	else
	{
		int ammoFrame = pTypeExt->AmmoPip;
		int emptyFrame = pTypeExt->EmptyAmmoPip;

		for (int i = 0; i < maxPips; i++)
		{
			if (i >= pipCount && emptyFrame < 0)
				break;

			int frame = i >= pipCount ? emptyFrame : ammoFrame;
			position.X += offset->Width;
			position.Y += yOffset;

			DSurface::Temp->DrawSHP(pConvert, pSHApe,
				frame, &position, rect, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

	return SkipGameDrawing;
}

ASMJIT_PATCH(0x5F46AE, ObjectClass_Select, 0x7)
{
	GET(ObjectClass*, pThis, ESI);

	if (RulesExtData::Instance()->SelectFlashTimer > 0 && pThis->GetOwningHouse() && pThis->GetOwningHouse()->ControlledByCurrentPlayer())
		pThis->Flash(RulesExtData::Instance()->SelectFlashTimer);

	return 0x0;
}
ASMJIT_PATCH_AGAIN(0x5F4718, ObjectClass_Select, 0x7)

#include <EventClass.h>

// Do not explicitly reset target for KeepTargetOnMove vehicles when issued move command.
ASMJIT_PATCH(0x4C7462, EventClass_Execute_KeepTargetOnMove, 0x5)
{
	enum { SkipGameCode = 0x4C74C0 };

	GET(EventClass*, pThis, ESI);
	GET(TechnoClass*, pTechno, EDI);
	GET(AbstractClass*, pTarget, EBX);

	if (pTechno->WhatAmI() != AbstractType::Unit)
		return 0;

	auto const mission = static_cast<Mission>(pThis->Data.MegaMission.Mission);
	auto const pExt = TechnoExtContainer::Instance.Find(pTechno);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pExt->Type);

	if ((mission == Mission::Move) && pTypeExt->KeepTargetOnMove && pTechno->Target && !pTarget)
	{
		if (pTechno->IsCloseEnoughToAttack(pTechno->Target))
		{
			auto const pDestination = pThis->Data.MegaMission.Destination.As_Abstract();
			pTechno->SetDestination(pDestination, true);
			pExt->KeepTargetOnMove = true;
			return SkipGameCode;
		}
	}
	pExt->KeepTargetOnMove = false;
	return 0;
}

void UpdateKeepTargetOnMove(TechnoClass* pThis)
{	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pExt->Type);

	if (!pExt->KeepTargetOnMove)
		return;

	if (!pThis->Target)
	{
		pExt->KeepTargetOnMove = false;
		return;
	}

	if (!pTypeExt->KeepTargetOnMove)
	{
		pThis->SetTarget(nullptr);
		pExt->KeepTargetOnMove = false;
		return;
	}

	if (pThis->CurrentMission == Mission::Guard)
	{
		if (!pTypeExt->KeepTargetOnMove_NoMorePursuit)
		{
			pThis->QueueMission(Mission::Attack, false);
			pExt->KeepTargetOnMove = false;
			return;
		}
	}
	else if (pThis->CurrentMission != Mission::Move)
	{
		return;
	}

	const int weaponIndex = pThis->SelectWeapon(pThis->Target);

	if (auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType)
	{
		pExt->AdditionalRange = static_cast<int>(pTypeExt->KeepTargetOnMove_ExtraDistance.Get());

		if (!pThis->IsCloseEnough(pThis->Target, weaponIndex))
		{
			pThis->SetTarget(nullptr);
			pExt->KeepTargetOnMove = false;
		}
	}
}

// Reset the target if beyond weapon range.
// This was originally in UnitClass::Mission_Move() but because that
// is only checked every ~15 frames, it can cause responsiveness issues.
ASMJIT_PATCH(0x736480, UnitClass_AI_KeepTargetOnMove, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	//auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	//auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	UpdateKeepTargetOnMove(pThis);
	//pExt->UpdateGattlingRateDownReset();

	return 0;
}

ASMJIT_PATCH(0x6B7600, SpawnManagerClass_AI_InitDestination, 0x6)
{
	enum { SkipGameCode1 = 0x6B795A, SkipGameCode2 = 0x6B795A };

	GET(SpawnManagerClass* const, pThis, ESI);
	GET(AircraftClass* const, pSpawnee, EDI);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Owner->GetTechnoType());

	if (pTypeExt->Spawner_AttackImmediately)
	{
		pSpawnee->SetTarget(pThis->Target);
		pSpawnee->QueueMission(Mission::Attack, true);
		pSpawnee->IsReturningFromAttackRun = false;
	}
	else
	{
		auto const mapCoords = pThis->Owner->GetMapCoords();
		auto const pCell = MapClass::Instance->GetCellAt(mapCoords);
		pSpawnee->SetDestination(pCell->GetNeighbourCell(FacingType::North), true);
		pSpawnee->QueueMission(Mission::Move, false);
	}

	return R->Origin() == 0x6B7600 ? SkipGameCode1 : SkipGameCode2;
}ASMJIT_PATCH_AGAIN(0x6B769F, SpawnManagerClass_AI_InitDestination, 0x7)

void DrawFactoryProgress(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	if (!RulesExtData::Instance()->FactoryProgressDisplay)
		return;

	if (!HouseClass::IsCurrentPlayerObserver())
		return;

	BuildingClass* const pBuilding = static_cast<BuildingClass*>(pThis);

	if(pBuilding->Type->InvisibleInGame || pBuilding->Type->Invisible )
		return;

	//CellClass* pCell = pBuilding->GetCell();
	BuildingTypeClass* const pBuildingType = pBuilding->Type;
	HouseClass* const pHouse = pBuilding->Owner;
	FactoryClass* pPrimaryFactory = nullptr;
	FactoryClass* pSecondaryFactory = nullptr;

	if (pHouse->IsControlledByHuman())
	{
		if (!pBuilding->IsPrimaryFactory)
			return;

		switch (pBuilding->Type->Factory)
		{
		case AbstractType::BuildingType:
			pPrimaryFactory = pHouse->GetPrimaryFactory(AbstractType::BuildingType, false, BuildCat::DontCare);
			pSecondaryFactory = pHouse->GetPrimaryFactory(AbstractType::BuildingType, false, BuildCat::Combat);
			break;
		case AbstractType::InfantryType:
			pPrimaryFactory = pHouse->GetPrimaryFactory(AbstractType::InfantryType, false, BuildCat::Combat);
			break;
		case AbstractType::UnitType:
			pPrimaryFactory = pHouse->GetPrimaryFactory(AbstractType::UnitType, pBuildingType->Naval, BuildCat::Combat);
			break;
		case AbstractType::AircraftType:
			pPrimaryFactory = pHouse->GetPrimaryFactory(AbstractType::AircraftType, false, BuildCat::Combat);
			break;
		default:
			return;
		}
	}
	else // AIs have no Primary factories
	{
		pPrimaryFactory = pBuilding->Factory;

		if (!pPrimaryFactory)
			return;
	}

	const bool havePrimary = pPrimaryFactory && pPrimaryFactory->Object;
	const bool haveSecondary = pSecondaryFactory && pSecondaryFactory->Object;

	if (!havePrimary && !haveSecondary)
		return;

	const int maxLength = pBuildingType->GetFoundationHeight(false) * 15 >> 1;
	const Point2D location = TechnoExtData::GetBuildingSelectBracketPosition(pBuilding, BuildingSelectBracketPosition::Top) + Point2D { 5, 3 };

	if (havePrimary)
	{
		const int curLength = std::clamp(static_cast<int>((static_cast<double>(pPrimaryFactory->GetProgress()) / 54) * maxLength), 0, maxLength);
		Point2D position = location;

		for (int frameIdx = curLength; frameIdx; --frameIdx, position.X -= 4, position.Y += 2)
			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, 3, &position, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

		for (int frameIdx = maxLength - curLength; frameIdx; --frameIdx, position.X -= 4, position.Y += 2)
			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, 0, &position, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	if (haveSecondary)
	{
		const int curLength = std::clamp(static_cast<int>((static_cast<double>(pSecondaryFactory->GetProgress()) / 54) * maxLength), 0, maxLength);
		Point2D position = havePrimary ? location + Point2D { 6, 3 } : location;

		for (int frameIdx = curLength; frameIdx; --frameIdx, position.X -= 4, position.Y += 2)
			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, 3, &position, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

		for (int frameIdx = maxLength - curLength; frameIdx; --frameIdx, position.X -= 4, position.Y += 2)
			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, 0, &position, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void DrawSuperProgress(TechnoClass* pThis, Point2D* pLocation ,  RectangleStruct* pBounds)
{
	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	if (!RulesExtData::Instance()->FactoryProgressDisplay)
		return;

	if (!HouseClass::IsCurrentPlayerObserver())
		return;

	BuildingClass* const pBuilding = static_cast<BuildingClass*>(pThis);

	if(pBuilding->Type->InvisibleInGame || pBuilding->Type->Invisible )
		return;

	//CellClass* pCell = pBuilding->GetCell();
	BuildingTypeClass* const pBuildingType = pBuilding->Type;

	if (pBuildingType->SuperWeapon == -1)
		return;

	SuperClass* const pSuper = pThis->Owner->Supers.Items[pBuildingType->SuperWeapon];

	if (!pSuper || pSuper->RechargeTimer.TimeLeft <= 1)
		return;

	const int maxLength = pBuildingType->GetFoundationHeight(false) * 15 >> 1;
	const int curLength = std::clamp(static_cast<int>((static_cast<double>(pSuper->RechargeTimer.TimeLeft - pSuper->RechargeTimer.GetTimeLeft()) / pSuper->RechargeTimer.TimeLeft) * maxLength), 0, maxLength);
	Point2D position = TechnoExtData::GetBuildingSelectBracketPosition(pBuilding , BuildingSelectBracketPosition::Top) + Point2D { 5, 3 };

	for (int frameIdx = curLength; frameIdx; --frameIdx, position.X -= 4, position.Y += 2)
		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, 5, &position, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

	for (int frameIdx = maxLength - curLength; frameIdx; --frameIdx, position.X -= 4, position.Y += 2)
		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, 0, &position, pBounds, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
}

ASMJIT_PATCH(0x6F5EE3, TechnoClass_DrawExtras_DrawAboveHealth, 0x9)
{
	GET(TechnoClass*, pThis, EBP);
	GET(Point2D*, pLoc, EDI);
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFSET(0x98, 0x8));

	DrawFactoryProgress(pThis, pLoc , pBounds);
	DrawSuperProgress(pThis, pLoc, pBounds);

	return 0;
}

ASMJIT_PATCH(0x6B77B4, SpawnManagerClass_Update_RecycleSpawned, 0x7)
{
	enum { Recycle = 0x6B77FF, NoRecycle = 0x6B7838 };

	GET(SpawnManagerClass* const, pThis, ESI);
	GET(TechnoClass* const, pSpawner, EDI);
	GET(CellStruct* const, pCarrierMapCrd, EBP);

	auto const pCarrier = pThis->Owner;
	auto const pCarrierTypeExt = TechnoTypeExtContainer::Instance.Find(pCarrier->GetTechnoType());
	auto const spawnerCrd = pSpawner->GetCoords();

	auto shouldRecycleSpawned = [&]()
	{
		auto const recycleCrd = pCarrierTypeExt->Spawner_RecycleFLH->IsValid()
			? TechnoExtData::GetFLHAbsoluteCoords(pCarrier, pCarrierTypeExt->Spawner_RecycleFLH, pCarrierTypeExt->Spawner_RecycleOnTurret)
			: pCarrier->GetCoords();

		auto const deltaCrd = spawnerCrd - recycleCrd;
		const int recycleRange = pCarrierTypeExt->Spawner_RecycleRange.Get();

		if (recycleRange < 0)
		{
			// This is a fix to vanilla behavior. Buildings bigger than 1x1 will recycle the spawner correctly.
			// 182 is √2/2 * 256. 20 is same to vanilla behavior.
			return (pCarrier->WhatAmI() == AbstractType::Building)
				? (deltaCrd.X <= 182 && deltaCrd.Y <= 182 && deltaCrd.Z < 20)
				: (pSpawner->GetMapCoords() == *pCarrierMapCrd && deltaCrd.Z < 20);
		}
		return deltaCrd.Length() <= recycleRange;
	};

	if (shouldRecycleSpawned())
	{
		if (pCarrierTypeExt->Spawner_RecycleAnim)
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pCarrierTypeExt->Spawner_RecycleAnim, spawnerCrd), pSpawner->Owner, nullptr, pSpawner, false, true);
		}

		pSpawner->SetLocation(pCarrier->GetCoords());
		return Recycle;
	}

	return NoRecycle;
}

// Change destination to RecycleFLH.
ASMJIT_PATCH(0x4D962B, FootClass_SetDestination_RecycleFLH, 0x5)
{
	GET(FootClass* const, pThis, EBP);
	GET(CoordStruct*, pDestCrd, EAX);

	auto pCarrier = pThis->SpawnOwner;

	if (pCarrier && pCarrier == pThis->Destination) // This is a spawner returning to its carrier.
	{
		auto pCarrierTypeExt = TechnoTypeExtContainer::Instance.Find(pCarrier->GetTechnoType());

		if (pCarrierTypeExt->Spawner_RecycleFLH->IsValid())
			*pDestCrd += TechnoExtData::GetFLHAbsoluteCoords(pCarrier, pCarrierTypeExt->Spawner_RecycleFLH, pCarrierTypeExt->Spawner_RecycleOnTurret) - pCarrier->GetCoords();

	}

   return 0;
}

ASMJIT_PATCH(0x6FA540, TechnoClass_AI_ChargeTurret, 0x6)
{
	enum { SkipGameCode = 0x6FA5BE };

	GET(TechnoClass*, pThis, ESI);

	if (pThis->ROF <= 0)
	{
		pThis->CurrentTurretNumber = 0;
		return SkipGameCode;
	}

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	int timeLeft = pThis->DiskLaserTimer.GetTimeLeft();

	if (pExt->ChargeTurretTimer.HasStarted())
		timeLeft = pExt->ChargeTurretTimer.GetTimeLeft();
	else if (pExt->ChargeTurretTimer.Expired())
		pExt->ChargeTurretTimer.Stop();

	int turretCount = pType->TurretCount;
	int turretIndex = std::max(0, timeLeft * turretCount / pThis->ROF);

	if (turretIndex >= turretCount)
		turretIndex = turretCount - 1;

	pThis->CurrentTurretNumber = turretIndex;
	return SkipGameCode;
}

ASMJIT_PATCH(0x7364DC, UnitClass_Update_SinkSpeed, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	GET(int, CoordZ, EDX);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	R->EDX(CoordZ - (pTypeExt->SinkSpeed - 5));
	return 0;
}

#include <Ext/InfantryType/Body.h>

ASMJIT_PATCH(0x521D94, InfantryClass_CurrentSpeed_ProneSpeed, 0x6)
{
	GET(FakeInfantryClass*, pThis, ESI);
	GET(int, currentSpeed, ECX);

	auto multiplier = pThis->_GetTypeExtData()->ProneSpeed.Get(
		RulesExtData::Instance()->InfantrySpeedData.getSpeed(pThis->Type->Crawls));
	currentSpeed = (int)(currentSpeed * multiplier);

	//if (pThis->Type->Crawls) {
	//	// Crawling: move at one-third speed
	//	currentSpeed /= 3;
	//}
	//else {
	//	// Prone but not crawling: move at 1.5x speed
	//	currentSpeed += (currentSpeed / 2);
	//}

	R->ECX(currentSpeed);
	return 0x521DC5;
}

ASMJIT_PATCH(0x4B3DF0, LocomotionClass_Process_DamagedSpeedMultiplier, 0x6)// Drive
{
	GET(FootClass*, pLinkedTo, ECX);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pLinkedTo->GetTechnoType());

	const double multiplier = pTypeExt->DamagedSpeed.Get(RulesExtData::Instance()->DamagedSpeed);
	__asm fmul multiplier;

	return R->Origin() + 0x6;
}ASMJIT_PATCH_AGAIN(0x6A343F, LocomotionClass_Process_DamagedSpeedMultiplier, 0x6)// Ship

ASMJIT_PATCH(0x655DDD, RadarClass_ProcessPoint_RadarInvisible, 0x6)
{
	enum { Invisible = 0x655E66, GoOtherChecks = 0x655E19 , Continue = 0x0};

	GET_STACK(bool, isInShrouded, STACK_OFFSET(0x40, 0x4));
	GET(ObjectClass*, pObject, EBP);

	if (auto pTechno = flag_cast_to<TechnoClass*>(pObject)){

		if (isInShrouded && !pTechno->Owner->IsControlledByHuman())
			return Invisible;

		auto pType = pTechno->GetTechnoType();

		if (pType->RadarInvisible)
		{
			auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

			if (!pTypeExt->RadarInvisibleToHouse.isset() || EnumFunctions::CanTargetHouse(pTypeExt->RadarInvisibleToHouse, pTechno->Owner, HouseClass::CurrentPlayer))
				return Invisible;
		}
	}

	return GoOtherChecks;
}

//ASMJIT_PATCH(0x5F4032, ObjectClass_FallingDown_ToDead, 0x6)
//{
//	GET(ObjectClass*, pThis, ESI);
//
//	if (auto const pTechno = flag_cast_to<TechnoClass*>(pThis))
//	{
//		auto pCell = pTechno->GetCell();
//		auto pType = pTechno->GetTechnoType();
//		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
//
//		if (!pCell || !pCell->IsClearToMove(pType->SpeedType, true, true, ZoneType::None, pType->MovementZone, pCell->GetLevel(), pCell->ContainsBridge()))
//			return 0;
//
//
//		double ratio = pCell->Tile_Is_Water() && !pTechno->OnBridge ?
//				pTypeExt->FallingDownDamage_Water.Get(pTypeExt->FallingDownDamage.Get())
//				: pTypeExt->FallingDownDamage.Get();
//
//		int damage = 0;
//
//		if (ratio < 0.0)
//			damage = int(pThis->Health * Math::abs(ratio));
//		else if (ratio >= 0.0 && ratio <= 1.0)
//			damage = int(pThis->GetTechnoType()->Strength * ratio);
//		else
//			damage = int(ratio);
//
//		pThis->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);
//
//		if (pThis->Health > 0 && pThis->IsAlive)
//		{
//			pThis->IsABomb = false;
//
//			if (pThis->WhatAmI() == AbstractType::Infantry)
//			{
//				auto pInf = static_cast<InfantryClass*>(pTechno);
//				const bool isWater = pCell->Tile_Is_Water();
//
//				if (isWater && pInf->SequenceAnim != DoType::Swim)
//					pInf->PlayAnim(DoType::Swim, true, false);
//				else if (!isWater && pInf->SequenceAnim != DoType::Guard)
//					pInf->PlayAnim(DoType::Guard, true, false);
//			}
//		} else {
//			pTechno->UpdatePosition((int)PCPType::During);
//		}
//
//		return 0x5F405B;
//	}
//
//	return 0;
//}

ASMJIT_PATCH(0x6B74F0, SpawnManagerClass_AI_UseTurretFacing, 0x5)
{
	GET(SpawnManagerClass* const, pThis, ESI);

	auto const pTechno = pThis->Owner;

	if (pTechno->HasTurret() && TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType())->Spawner_UseTurretFacing)
		R->EAX(pTechno->SecondaryFacing.Current().Raw);

	return 0;
}

#include <AirstrikeClass.h>

// Allow airstrike flare draw to foot
DEFINE_JUMP(LJMP, 0x6D481D, 0x6D482D)

ASMJIT_PATCH(0x6F348F, TechnoClass_WhatWeaponShouldIUse_Airstrike, 0x7)
{
	enum { Primary = 0x6F37AD, Secondary = 0x6F3807 };

	GET(TechnoClass*, pTargetTechno, EBP);

	if (!pTargetTechno)
		return Primary;

	GET(WarheadTypeClass*, pSecondaryWH, ECX);
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pSecondaryWH);

	if (!EnumFunctions::IsTechnoEligible(pTargetTechno, pWHExt->AirstrikeTargets))
		return Primary;

	const auto pTargetType = pTargetTechno->GetTechnoType();

	if (pTargetTechno->AbstractFlags & AbstractFlags::Foot)
	{
		const auto pTargetTypeExt = TechnoTypeExtContainer::Instance.Find(pTargetType);
		return pTargetTypeExt->AllowAirstrike.Get(true) ? Secondary : Primary;
	}

	const auto pTargetTypeExt = TechnoTypeExtContainer::Instance.Find(pTargetType);
	return pTargetTypeExt->AllowAirstrike.Get(static_cast<BuildingTypeClass*>(pTargetType)->CanC4) && (!pTargetType->ResourceDestination || !pTargetType->ResourceGatherer) ? Secondary : Primary;
}

// ASMJIT_PATCH(0x41D97B, AirstrikeClass_Fire_SetAirstrike, 0x7)
// {
// 	enum { ContinueIn = 0x41D9A0, Skip = 0x41DA0B };

// 	GET(AirstrikeClass*, pThis, EDI);
// 	GET(TechnoClass*, pTarget, ESI);
// 	const auto pTargetExt = TechnoExtContainer::Instance.Find(pTarget);
// 	pTargetExt->AirstrikeTargetingMe = pThis;
// 	pTarget->StartAirstrikeTimer(100000);

// 	if(auto pBld = cast_to<BuildingClass* , false>(pTarget)){
// 		pBld->IsAirstrikeTargetingMe = true;
// 		pBld->Mark(MarkType::Redraw);
// 	}

// 	return Skip;
// }

ASMJIT_PATCH(0x41DA52, AirstrikeClass_ResetTarget_OriginalTarget, 0x6)
{
	enum { SkipGameCode = 0x41DA7C };

	R->EDI(R->ESI());

	return SkipGameCode;
}

ASMJIT_PATCH(0x41DA80, AirstrikeClass_ResetTarget_NewTarget, 0x6)
{
	enum { SkipGameCode = 0x41DA9C };

	R->ESI(R->EBX());

	return SkipGameCode;
}

ASMJIT_PATCH(0x41DAA4, AirstrikeClass_ResetTarget_ResetForOldTarget, 0xA)
{
	enum { SkipGameCode = 0x41DAAE };

	GET(TechnoClass*, pTargetTechno, EDI);
	const auto pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);
	pTargetExt->AirstrikeTargetingMe = nullptr;

	return SkipGameCode;
}

ASMJIT_PATCH(0x41DAD4, AirstrikeClass_ResetTarget_ResetForNewTarget, 0x6)
{
	enum { SkipGameCode = 0x41DADA };

	GET(AirstrikeClass*, pThis, EBP);
	GET(TechnoClass*, pTargetTechno, ESI);
	const auto pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);
	pTargetExt->AirstrikeTargetingMe = pThis;

	return SkipGameCode;
}

ASMJIT_PATCH(0x41DBD4, AirstrikeClass_Stop_ResetForTarget, 0x7)
{
	enum { SkipGameCode = 0x41DC3A };

	GET(AirstrikeClass*, pThis, EBP);
	GET(ObjectClass*, pTarget, ESI);

	if (const auto pTargetTechno = flag_cast_to<TechnoClass*>(pTarget))
	{
		auto vtable = VTable::Get(pTargetTechno);

		if(!pTargetTechno->IsAlive || (vtable != UnitClass::vtable && vtable != InfantryClass::vtable && vtable != AircraftClass::vtable && vtable != BuildingClass::vtable))
			return SkipGameCode;

		AirstrikeClass* pLastTargetingMe = nullptr;

		for (int idx = AirstrikeClass::Array->Count - 1; idx >= 0; --idx)
		{
			const auto pAirstrike = AirstrikeClass::Array->Items[idx];

			if (pAirstrike != pThis && pAirstrike->Target == pTarget)
			{
				pLastTargetingMe = pAirstrike;
				break;
			}
		}

		const auto pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);
		pTargetExt->AirstrikeTargetingMe = pLastTargetingMe;

		if (!pLastTargetingMe && Game::IsActive)
			pTarget->Mark(MarkType::Redraw);
	}

	return SkipGameCode;
}

ASMJIT_PATCH(0x41D604, AirstrikeClass_PointerGotInvalid_ResetForTarget, 0x6)
{
	enum { SkipGameCode = 0x41D634 };

	GET(ObjectClass*, pTarget, EAX);

	if (const auto pTargetTechno = flag_cast_to<TechnoClass*>(pTarget))
		TechnoExtContainer::Instance.Find(pTargetTechno)->AirstrikeTargetingMe = nullptr;

	return SkipGameCode;
}

ASMJIT_PATCH(0x65E97F, HouseClass_CreateAirstrike_SetTargetForUnit, 0x6)
{
	GET_STACK(AirstrikeClass*, pThis, STACK_OFFSET(0x38, 0x1C));

	if(const auto pOwner = pThis->Owner){
		if (const auto pTarget = flag_cast_to<TechnoClass*>(pOwner->Target)) {
			R->EAX(pTarget);
			return 0x65E983;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x51EAE0, TechnoClass_WhatAction_AllowAirstrike, 0x7)
{
	enum { CanAirstrike = 0x51EB06, Cannot = 0x51EB15 };

	GET(ObjectClass*, pObject, ESI);

	if (const auto pTechno = flag_cast_to<TechnoClass*>(pObject))
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType());

		if (const auto pBuilding = cast_to<BuildingClass* , false>(pTechno))
		{
			const auto pBuildingType = pBuilding->Type;
			return pTypeExt->AllowAirstrike.Get(pBuildingType->CanC4) && !pBuildingType->InvisibleInGame ? CanAirstrike : Cannot;
		}
		else
		{
			return pTypeExt->AllowAirstrike.Get(true) ? CanAirstrike : Cannot;
		}
	}

	return Cannot;
}

// ASMJIT_PATCH(0x70782D, TechnoClass_PointerGotInvalid_Airstrike, 0x6)
// {
// 	GET(TechnoClass*, pThis, ESI);
// 	GET(AbstractClass*, pAbstract, EBP);
//
// 	if(const auto pExt = TechnoExtContainer::Instance.Find(pThis))
// 		AnnounceInvalidPointer(pExt->AirstrikeTargetingMe, pAbstract);
//
// 	AnnounceInvalidPointer(pThis->Airstrike, pAbstract);
//
// 	return 0x70783B;
// }

#pragma region GetEffectTintIntensity

ASMJIT_PATCH(0x70E92F, TechnoClass_UpdateAirstrikeTint, 0x5)
{
	enum { ContinueIn = 0x70E96E, Skip = 0x70EC9F };

	GET(TechnoClass*, pThis, ESI);
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	return pExt->AirstrikeTargetingMe ? ContinueIn : Skip;
}

// ASMJIT_PATCH(0x43FDD6, BuildingClass_AI_Airstrike, 0x6)
// {
// 	enum { SkipGameCode = 0x43FDF1 };

// 	GET(BuildingClass*, pThis, ESI);
// 	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

// 	if (pExt->AirstrikeTargetingMe)
// 		pThis->Mark(MarkType::Redraw);

// 	return SkipGameCode;
// }

ASMJIT_PATCH(0x43F9E0, BuildingClass_Mark_Airstrike, 0x6)
{
	enum { ContinueTintIntensity = 0x43FA0F, NonAirstrike = 0x43FA19 };

	GET(BuildingClass*, pThis, EDI);
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	return pExt->AirstrikeTargetingMe ? ContinueTintIntensity : NonAirstrike;
}

ASMJIT_PATCH(0x448DF1, BuildingClass_SetOwningHouse_Airstrike, 0x6)
{
	enum { ContinueTintIntensity = 0x448E0D, NonAirstrike = 0x448E17 };

	GET(BuildingClass*, pThis, ESI);
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	return pExt->AirstrikeTargetingMe ? ContinueTintIntensity : NonAirstrike;
}

ASMJIT_PATCH(0x451ABC, BuildingClass_PlayAnim_Airstrike, 0x6)
{
	enum { ContinueTintIntensity = 0x451AEB, NonAirstrike = 0x451AF5 };

	GET(BuildingClass*, pThis, ESI);
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	return pExt->AirstrikeTargetingMe ? ContinueTintIntensity : NonAirstrike;
}

ASMJIT_PATCH(0x452041, BuildingClass_452000_Airstrike, 0x6)
{
	enum { ContinueTintIntensity = 0x452070, NonAirstrike = 0x45207A };

	GET(BuildingClass*, pThis, ESI);
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	return pExt->AirstrikeTargetingMe ? ContinueTintIntensity : NonAirstrike;
}

ASMJIT_PATCH(0x456E5A, BuildingClass_Flash_Airstrike, 0x6)
{
	enum { ContinueTintIntensity = 0x456E89, NonAirstrike = 0x456E93 };

	GET(BuildingClass*, pThis, ESI);
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	return pExt->AirstrikeTargetingMe ? ContinueTintIntensity : NonAirstrike;
}

DEFINE_FUNCTION_JUMP(CALL, 0x450A5D, FakeBuildingClass::_GetAirstrikeInvulnerabilityIntensity); // BuildingClass_Animation_AI

#pragma endregion

ASMJIT_PATCH(0x4D6D34, FootClass_MissionAreaGuard_Harvester, 0x5)
{
	enum { GoGuardArea = 0x4D6D69 };

	GET(FootClass*, pThis, ESI);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	return pTypeExt->Harvester_CanGuardArea && pThis->Owner->IsControlledByHuman() ? GoGuardArea : 0;
}

DEFINE_JUMP(LJMP, 0x741406, 0x741427);

ASMJIT_PATCH(0x736F61, UnitClass_UpdateFiring_FireUp, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(int, weaponIndex, EDI);

	const auto pType = pThis->Type;

	if (pType->Turret || pType->Voxel)
		return 0;

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	// SHP vehicles have no secondary action frames, so it does not need SecondaryFire.
	const int fireUp = pTypeExt->FireUp;
	CDTimerClass& Timer = pExt->FiringAnimationTimer;

	if (fireUp >= 0 && !pType->OpportunityFire &&
		pThis->Locomotor->Is_Really_Moving_Now())
	{
		if (Timer.InProgress())
			Timer.Stop();

		return 0x736F73;
	}

	const int frames = pType->FiringFrames;
	if (!Timer.InProgress() && frames >= 1)
	{
		pThis->CurrentFiringFrame = 2 * frames - 1;
		Timer.Start(pThis->CurrentFiringFrame);
	}

	if (fireUp >= 0 && frames >= 1)
	{
		int cumulativeDelay = 0;
		int projectedDelay = 0;
		auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
		auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
		const bool allowBurst = pWeaponExt && pWeaponExt->Burst_FireWithinSequence;

		// Calculate cumulative burst delay as well cumulative delay after next shot (projected delay).
		if (allowBurst)
		{
			for (int i = 0; i <= pThis->CurrentBurstIndex; i++)
			{
				const int burstDelay = WeaponTypeExtData::GetBurstDelay(pWeapon , i);
				int delay = 0;

				if (burstDelay > -1)
					delay = burstDelay;
				else
					delay = ScenarioClass::Instance->Random.RandomRanged(3, 5);

				// Other than initial delay, treat 0 frame delays as 1 frame delay due to per-frame processing.
				if (i != 0)
					delay = std::max(delay, 1);

				cumulativeDelay += delay;

				if (i == pThis->CurrentBurstIndex)
					projectedDelay = cumulativeDelay + delay;
			}
		}

		const int frame = (Timer.TimeLeft - Timer.GetTimeLeft());

		if (frame % 2 != 0)
			return 0x736F73;

		if (frame / 2 != fireUp + cumulativeDelay)
		{
			return 0x736F73;
		}
		else if (allowBurst)
		{
			// If projected frame for firing next shot goes beyond the sequence frame count, cease firing after this shot and start rearm timer.
			if (fireUp + projectedDelay > frames)
				pExt->ForceFullRearmDelay = true;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x70DE40, TechnoClass_GattlingValueRateDown_GattlingRateDownDelay, 0xA)
{
	enum { Return = 0x70DE62 };

	GET(BuildingClass* const, pThis, ECX);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (pTypeExt->RateDown_Delay < 0)
		return Return;

	++pExt->AccumulatedGattlingValue;
	auto remain = pExt->AccumulatedGattlingValue;

	if (!pExt->ShouldUpdateGattlingValue)
		remain -= pTypeExt->RateDown_Delay;

	if (remain <= 0)
		return Return;

	// Time's up
	GET_STACK(int, rateDown, STACK_OFFSET(0x0, 0x4));
	pExt->AccumulatedGattlingValue = 0;
	pExt->ShouldUpdateGattlingValue = true;

	if (pThis->Ammo <= pTypeExt->RateDown_Ammo)
		rateDown = pTypeExt->RateDown_Cover;

	if (!rateDown)
	{
		pThis->GattlingValue = 0;
		return Return;
	}

	const int newValue = pThis->GattlingValue - (rateDown * remain);
	pThis->GattlingValue = (newValue <= 0) ? 0 : newValue;
	return Return;
}

ASMJIT_PATCH(0x70DE70, TechnoClass_GattlingRateUp_GattlingRateDownReset, 0x5)
{
	GET(TechnoClass* const, pThis, ECX);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	pExt->AccumulatedGattlingValue = 0;
	pExt->ShouldUpdateGattlingValue = false;

	return 0;
}

ASMJIT_PATCH(0x70E01E, TechnoClass_GattlingRateDown_GattlingRateDownDelay, 0x6)
{
	enum { SkipGameCode = 0x70E04D };

	GET(TechnoClass* const, pThis, ESI);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->RateDown_Delay < 0)
		return SkipGameCode;

	GET_STACK(int, rateMult, STACK_OFFSET(0x10, 0x4));

	pExt->AccumulatedGattlingValue += rateMult;
	auto remain = pExt->AccumulatedGattlingValue;

	if (!pExt->ShouldUpdateGattlingValue)
		remain -= pTypeExt->RateDown_Delay;

	if (remain <= 0 && rateMult)
		return SkipGameCode;

	// Time's up
	pExt->AccumulatedGattlingValue = 0;
	pExt->ShouldUpdateGattlingValue = true;

	if (!rateMult)
	{
		pThis->GattlingValue = 0;
		return SkipGameCode;
	}

	const auto rateDown = (pThis->Ammo <= pTypeExt->RateDown_Ammo) ? pTypeExt->RateDown_Cover.Get() : pTypeExt->AttachedToObject->RateDown;

	if (!rateDown)
	{
		pThis->GattlingValue = 0;
		return SkipGameCode;
	}

	const int newValue = pThis->GattlingValue - (rateDown * remain);
	pThis->GattlingValue = (newValue <= 0) ? 0 : newValue;
	return SkipGameCode;
}

#pragma region SetTarget

ASMJIT_PATCH(0x4DF4A5, FootClass_UpdateAttackMove_SetTarget, 0x6)
{
	GET(FootClass*, pThis, ESI);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (R->Origin() != 0x4DF4A5)
	{
		pThis->Target = nullptr;
		pExt->UpdateGattlingRateDownReset();

		return R->Origin() + 0xA;
	}
	else
	{
		GET(AbstractClass*, pTarget, EAX);

		pThis->Target = pTarget;
		pExt->UpdateGattlingRateDownReset();

		return 0x4DF4AB;
	}
}ASMJIT_PATCH_AGAIN(0x4DF3D3, FootClass_UpdateAttackMove_SetTarget, 0xA)
ASMJIT_PATCH_AGAIN(0x4DF46A, FootClass_UpdateAttackMove_SetTarget, 0xA)

ASMJIT_PATCH(0x6FCF3E, TechnoClass_SetTarget_After, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EDI);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (const auto pUnit = cast_to<UnitClass*, false>(pThis))
	{
		const auto pUnitType = pUnit->Type;

		if (!pUnitType->Turret && !pUnitType->Voxel)
		{
			const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pUnit->Type);

			if (!pTarget || pTypeExt->FireUp < 0 || pTypeExt->FireUp_ResetInRetarget
				|| !pThis->IsCloseEnough(pTarget, pThis->SelectWeapon(pTarget)))
			{
				pUnit->CurrentFiringFrame = -1;
				pExt->FiringAnimationTimer.Stop();
			}
		}
	}

	pThis->Target = pTarget;
	pExt->UpdateGattlingRateDownReset();

	return 0x6FCF44;
}

#pragma endregion