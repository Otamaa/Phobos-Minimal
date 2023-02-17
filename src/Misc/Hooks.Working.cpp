#include "Hooks.OtamaaBugFix.h"
#include <SpecificStructures.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <New/Entity/FlyingStrings.h>

//DEFINE_HOOK(0x730F1C, ObjectClass_StopCommand, 0x5)
//{
//	GET(ObjectClass*, pObject, ESI);
//
//	if (auto pTechno = generic_cast<TechnoClass*>(pObject)) {
//		if (auto pManager = pTechno->SpawnManager) {
//			if (!pManager->SpawnType->MissileSpawn && pManager->Status != SpawnManagerStatus::Idle && pManager->Target)
//				pManager->ResetTarget();
//		}
//	}
//
//	return 0;
//}

//TODO : retest for desync
DEFINE_HOOK(0x4242F4, AnimClass_Trail_Override, 0x6) // was 4
{
	GET(AnimClass*, pAnim, EDI);
	GET(AnimClass*, pThis, ESI);

	auto nCoord = pThis->GetCoords();
	GameConstruct(pAnim, pThis->Type->TrailerAnim, nCoord, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	{
		if (const auto pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type))
		{
			const auto pTech = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker.Get());
			const auto pOwner = pThis->Owner ? pThis->Owner : pTech ? pTech->GetOwningHouse() : nullptr;
			AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, pTech, false);
		}
	}

	return 0x424322;
}

#ifdef ENABLE_NEWHOOKS
//TODO : retest for desync
DEFINE_HOOK(0x442A2A, BuildingClass_ReceiveDamage_RotateVsAircraft, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	GET(RulesClass*, pRules, ECX);

	if (pThis && pThis->Type)
	{
		if (auto const pStructureExt = BuildingTypeExt::ExtMap.Find(pThis->Type))
		{
			R->AL(pStructureExt->PlayerReturnFire.Get(pRules->PlayerReturnFire));
			return 0x442A30;
		}
	}

	return 0x0;
}
#endif

DEFINE_HOOK_AGAIN(0x4426DB, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
DEFINE_HOOK_AGAIN(0x702777, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
DEFINE_HOOK(0x70272E, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
{
	enum
	{
		BuildingClass_TakeDamage_DamageSound = 0x4426DB,
		BuildingClass_TakeDamage_DamageSound_Handled_ret = 0x44270B,

		TechnoClass_TakeDamage_Building_DamageSound_01 = 0x702777,
		TechnoClass_TakeDamage_Building_DamageSound_01_Handled_ret = 0x7027AE,

		TechnoClass_TakeDamage_Building_DamageSound_02 = 0x70272E,
		TechnoClass_TakeDamage_Building_DamageSound_02_Handled_ret = 0x702765,

		Nothing = 0x0
	};

	GET(TechnoClass*, pThis, ESI);

	if (auto const pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		auto const pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
		if (pExt->DisableDamageSound.Get())
		{
			switch (R->Origin())
			{
			case BuildingClass_TakeDamage_DamageSound:
				return BuildingClass_TakeDamage_DamageSound_Handled_ret;
			case TechnoClass_TakeDamage_Building_DamageSound_01:
				return TechnoClass_TakeDamage_Building_DamageSound_01_Handled_ret;
			case TechnoClass_TakeDamage_Building_DamageSound_02:
				return TechnoClass_TakeDamage_Building_DamageSound_02_Handled_ret;
			}
		}
	}

	return Nothing;
}

DEFINE_HOOK(0x4FB63A, HouseClass_PlaceObject_EVA_UnitReady, 0x5)
{
	GET(TechnoClass*, pProduct, ESI);

	auto nSpeak = GameStrings::EVA_UnitReady();

	if (auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pProduct->GetTechnoType()))
	{
		if (CRT::strlen(pTechnoTypeExt->Eva_Complete.data()))
		{
			if (INIClass::IsBlank(pTechnoTypeExt->Eva_Complete.data()) ||
			 !CRT::strcmpi(DEFAULT_STR, pTechnoTypeExt->Eva_Complete.data()) ||
			 !CRT::strcmpi(DEFAULT_STR2, pTechnoTypeExt->Eva_Complete.data()))
			{
				return 0x4FB649;
			}
			else
			{
				nSpeak = pTechnoTypeExt->Eva_Complete.data();
			}
		}
	}

	VoxClass::Play(nSpeak);

	return 0x4FB649;
}

DEFINE_HOOK(0x4FB7CA, HouseClass_RegisterJustBuild_CreateSound_PlayerOnly, 0x6) //9
{
	GET(HouseClass*, pThis, EDI);
	GET(TechnoTypeClass*, pTech, ESI);
	GET_STACK(TechnoClass*, pTech_, STACK_OFFS(0x1C, -0x4));

	if (pTech && pTech_)
	{
		if (auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTech))
		{
			if (pTechnoTypeExt->VoiceCreate.Get() != -1)
				pTech_->QueueVoice(pTechnoTypeExt->VoiceCreate.Get());

			if (!pTechnoTypeExt->CreateSound_Enable.Get())
				return 0x4FB804;

			if (RulesExt::Global()->CreateSound_PlayerOnly.Get())
				return pThis->IsCurrentPlayer() ? 0x0 : 0x4FB804;

		}
	}

	return 0x0;
}

DEFINE_HOOK(0x6A8E25, SidebarClass_StripClass_AI_Building_EVA_ConstructionComplete, 0x5)
{
	GET(TechnoClass*, pTech, ESI);

	if (pTech && (pTech->WhatAmI() == AbstractType::Building) &&
		pTech->GetOwningHouse() &&
		pTech->GetOwningHouse()->IsCurrentPlayer())
	{
		auto nSpeak = GameStrings::EVA_ConstructionComplete();
		if (auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTech->GetTechnoType()))
		{
			if (CRT::strlen(pTechnoTypeExt->Eva_Complete.data()))
			{
				if (INIClass::IsBlank(pTechnoTypeExt->Eva_Complete.data()) ||
				 !CRT::strcmpi(DEFAULT_STR, pTechnoTypeExt->Eva_Complete.data()) ||
				 !CRT::strcmpi(DEFAULT_STR2, pTechnoTypeExt->Eva_Complete.data()))
				{
					return 0x6A8E34;
				}
				else
				{
					nSpeak = pTechnoTypeExt->Eva_Complete.data();
				}
			}
		}

		VoxClass::Play(nSpeak);
	}

	return 0x6A8E34;
}

DEFINE_HOOK(0x736BF3, UnitClass_UpdateRotation_TurretFacing, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	// I still don't know why jumpjet loco behaves differently for the moment
	// so I don't check jumpjet loco or InAir here, feel free to change if it doesn't break performance.
	if (!pThis->Target && !pThis->Type->TurretSpins && (pThis->Type->JumpJet || pThis->Type->BalloonHover))
	{
		pThis->SecondaryFacing.Set_Desired(pThis->PrimaryFacing.Current());
		pThis->TurretIsRotating = pThis->SecondaryFacing.Is_Rotating();
		return 0x736C09;
	}

	return 0;
}

//size
//#ifdef ENABLE_NEWHOOKS
//Crash
//DEFINE_HOOK(0x444014, AircraftClass_ExitObject_DisableRadioContact, 0x5)
//{
//	enum { SkipAllSet = 0x444053, Nothing = 0x0 };
//
//	//GET(BuildingClass*, pBuilding, ESI);
//	GET(AircraftClass*, pProduct, EBP);
//
//	if (pProduct) {
//		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pProduct->Type);
//		if (pTypeExt && !pProduct->Type->AirportBound && pTypeExt->NoAirportBound_DisableRadioContact.Get()) {
//			return SkipAllSet;
//		}
//	}
//
//	return Nothing;
//}

//#endif
//
//DEFINE_HOOK(0x415302, AircraftClass_MissionUnload_IsDropship, 0x8)
//{
//	GET(AircraftClass*, pThis, ESI);
//
//	if (pThis->Destination) {
//		if (pThis->Type->IsDropship) {
//			CellStruct nCell = CellStruct::Empty;
//			if (pThis->SelectNavalTargeting(pThis->Destination) != 11) {
//				if (auto pTech = pThis->Destination->AsTechno()) {
//					auto nCoord = pTech->GetCoords();
//					nCell = CellClass::Coord2Cell(nCoord);
//
//					if (nCell != CellStruct::Empty) {
//						if (auto pCell = Map[nCell]) {
//							for (auto pOccupy = pCell->FirstObject;
//								pOccupy;
//								pOccupy = pOccupy->NextObject) {
//								if (pOccupy->WhatAmI() == AbstractType::Building) {
//									auto pGoodLZ = pThis->GoodLandingZone();
//									pThis->SetDestination(pGoodLZ, true);
//								}
//								else {
//									nCoord = pThis->GetCoords();
//									pOccupy->Scatter(nCoord, true, true);
//								}
//							}
//						}
//					}
//				}
//			}
//		} else {
//			return 0x41531B;
//		}
//	}
//
//	return 0x41530C;
//}

//DEFINE_HOOK(0x4CD8C9, FlyLocomotionClass_Movement_AI_DisableTSExp, 0x9)
//{
//	GET(FootClass*, pFoot, EDX);
//	auto const& pTypeExt = TechnoTypeExt::ExtMap.Find(pFoot->GetTechnoType());
//	return pTypeExt->Disable_C4WarheadExp.Get() ? 0x4CD9C0 : 0x0;
//}

#ifndef ENABLE_NEWHOOKS
template<bool CheckNotHuman = true>
static Iterator<AnimTypeClass*> GetDeathBodies(InfantryTypeClass* pThisType, const ValueableVector<AnimTypeClass*>& nOverrider)
{
	if (!nOverrider.empty())
		return nOverrider;

	if (pThisType->DeadBodies.Count > 0)
	{
		return pThisType->DeadBodies;
	}

	if constexpr (CheckNotHuman)
	{
		if (!pThisType->NotHuman)
		{
			return RulesGlobal->DeadBodies;
		}
	}

	return { };
}

DEFINE_HOOK(0x520BCC, InfantryClass_DoingAI_FetchDoType, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(int, nDoType, EAX);
	InfantryExt::ExtMap.Find(pThis)->CurrentDoType = nDoType;
	return 0x0;
}

//TODO : retest for desync
//DEFINE_HOOK(0x520BE5, InfantryClass_DoingAI_DeadBodies, 0x6)
//{
//	GET(InfantryClass* const, pThis, ESI);
//
//	auto const Iter = GetDeathBodies(pThis->Type, {});
//	constexpr auto Die = [](int x) { return x - 11; };
//
//	if (!Iter.empty()) {
//		int nIdx = 0;
//
//		if(InfantryTypeExt::ExtMap.Find(pThis->Type)->DeathBodies_UseDieSequenceAsIndex.Get()){
//			auto pInfExt = InfantryExt::ExtMap.Find(pThis);
//			nIdx = Die(pInfExt->CurrentDoType);
//			if (nIdx > Iter.size())
//				nIdx = Iter.size();
//		} else {
//			nIdx = ScenarioGlobal->Random.RandomFromMax(Iter.size() - 1);
//		}
//
//		if (AnimTypeClass* pSelected = Iter.at(nIdx)) {
//			if (const auto pAnim = GameCreate<AnimClass>(pSelected, pThis->GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0)) {
//				AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);
//			}
//		}
//	}
//
//    pThis->UnInit();
//	return 0x520E52;
//}

DEFINE_HOOK(0x518B98, InfantryClass_ReceiveDamage_DeadBodies, 0x8)
{
	GET(InfantryClass*, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, receiveDamageArgs, STACK_OFFS(0xD0, -0x4));

	if (!InfantryExt::ExtMap.Find(pThis)->IsUsingDeathSequence)
	{
		auto pWHExt = WarheadTypeExt::ExtMap.Find(receiveDamageArgs.WH);
		auto const Iter = GetDeathBodies<false>(pThis->Type, pWHExt->DeadBodies);

		if (!Iter.empty())
		{
			if (AnimTypeClass* pSelected = Iter.at(ScenarioGlobal->Random.RandomFromMax(Iter.size() - 1)))
			{
				if (const auto pAnim = GameCreate<AnimClass>(pSelected, pThis->GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0))
				{
					AnimExt::SetAnimOwnerHouseKind(pAnim, receiveDamageArgs.Attacker ? receiveDamageArgs.Attacker->GetOwningHouse() : receiveDamageArgs.SourceHouse, pThis->GetOwningHouse(), true);
				}
			}
		}
	}

	pThis->UnInit();
	return 0x518BA0;
}
#endif

//#include "TestTurrent.h"
//
//std::vector<std::pair<ImageDatas, ImageDatas>> TTypeEXt::AdditionalTurrents { };

//#include <JumpjetLocomotionClass.h>
//#include <Ext/Building/Body.h>
//
//static int GetHeight(CellClass* pCell, const Nullable<int>& nOffset)
//{
//	int nHeight = 0;
//	if (auto pBuilding = pCell->GetBuilding())
//	{
//	   auto pBldExt = BuildingExt::ExtMap.Find(pBuilding);
//
//		CoordStruct vCoords = { 0, 0, 0 };
//		if(!pBldExt->LimboID != -1)
//			pBuilding->Type->Dimension2(&vCoords);
//
//		nHeight = vCoords.Z;
//	}
//	else
//	{
//		if (pCell->FindTechnoNearestTo({ 0,0 }, 0, 0))
//			nHeight = 85;
//	}
//
//	if (nOffset.isset()) {
//		nHeight -= nOffset.Get();
//		if (nHeight < 0)
//			nHeight = 0;
//	}
//
//	int nCellHeight = pCell->GetFloorHeight({ 128,128 });
//	if (pCell->ContainsBridge())
//		nCellHeight += 416;
//
//	return nHeight + nCellHeight;
//}
//
//DEFINE_HOOK(0x54D820, JumpJetLocomotionClass_UpdateHeightAboveObject, 0x6)
//{
//	GET(JumpjetLocomotionClass*, pLoco, ECX);
//
//	auto pOwner = pLoco->LinkedTo;
//	auto pCell = Map.GetCellAt(pOwner->Location);
//	int nHeight = GetHeight(pCell, {});
//
//	if (pLoco->__currentSpeed <= 0.0)
//	{
//		R->EAX(nHeight);
//	}
//	else
//	{
//		auto nFace = pLoco->Facing.Current().GetFacing<8>();
//		auto nAdj = AdjacentCoord[nFace];
//		auto nRes = CoordStruct { pOwner->Location.X + nAdj.X , pOwner->Location.Y + nAdj.Y };
//		auto pCell2 = Map.GetCellAt(nRes);
//		int nHeight2 = GetHeight(pCell2, {});
//		if (nHeight2 <= nHeight)
//			nHeight2 = (nHeight + nHeight2) / 2;
//
//		R->EAX(nHeight2);
//	}
//
//	return 0x54D917;
//}

//int __cdecl UnitClass_GetFireError_AIDeployFire(REGISTERS* R)
//{
//
//	bool DeployToFire_RequiresAIOnly = false;
//	GET(InfantryClass*, v1, ESI);
//	auto v2 = v1->Type;
//	if (!DeployToFire_RequiresAIOnly || v1->Owner->IsControlledByCurrentPlayer())
//	{
//		if (!*(v2 + 3602) || !v1->CanDeployNow())
//			return 0x7410B7;
//
//		return !Map.GetCellAt(v1->Location)->IsBuildable() ? 0x7410A8 :0x7410B7;
//	}
//	else
//	{
//		auto v22 = v1->GetCoords();
//		auto v7 =  Map.GetCellAt(v22);
//		auto v8 = *(v7 + 236);
//		auto v9 = !v7->IsBuildable() && v8 != 2 && v8 != 6;
//		if (*(v3 + 989))
//		{
//			if (!v1->f.t.cargo.CargoHold)
//				return 0x7410B7;
//			if (!v9)
//			{
//				MapClass::NearByLocation(
//				  0x87F7E8,
//				  &cell1,
//				  &cell2,
//				  SPEED_FOOT,
//				  -1,
//				  MZONE_NORMAL,
//				  0,
//				  1,
//				  1,
//				  0,
//				  0,
//				  0,
//				  0,
//				  &cellstrunct::empty,
//				  0,
//				  0);
//				cell2 = cell1;
//				if (cell1.X || cell1.Y)
//				{
//					v12 = cell1.X + (cell1.Y << 9);
//					if (v12 > 0x3FFFF || (v13 = *(MEMORY[0x87F924] + 4 * v12)) == 0)
//					{
//						v13 = 11263056;
//						MEMORY[0xABDC74] = cell1;
//					}
//					(v1->f.t.r.m.o.a.vftable->Stop_Driver)(v1);
//					v1->f.t.r.m.o.a.vftable->t.Assign_Target(v1, 0);
//					v1->f.t.r.m.o.a.vftable->t.Assign_Destination(v1, v13, 1);
//					v1->f.t.r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(v1, MISSION_MOVE, 0);
//					v1->f.t.r.m.o.a.vftable->t.r.m.Commence(v1);
//					v1->f.t.r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(v1, MISSION_UNLOAD, 0);
//				}
//			}
//			v14 = v1->f.t.r.m.o.a.vftable->CanDeployNow(v1);
//			v15 = 0x7410B7;
//			if (v14)
//				v15 = 0x7410A8;
//			result = v15;
//		}
//		else
//		{
//			if (!v9)
//				return 0x7410B7;
//			v10 = !v1->f.t.r.m.o.a.vftable->CanDeployNow(v1);
//			result = 7606440;
//			if (v10)
//				return 0x7410B7;
//		}
//	}
//	return result;
//}
//#include <Ext/WeaponType/Body.h>
//
//DEFINE_HOOK(0x4A730D , DiskLaserClass_Update_CalculateFacing, 0x6)
//{
//	GET(DiskLaserClass*, pThis, ESI);
//	GET(int, nFace, EDX);
//
//	auto pExt = WeaponTypeExt::ExtMap.Find(pThis->Weapon);
//
//	if (!pExt->DiskLaser_FiringOffset.isset())
//		return 0x0;
//
//	auto nOffset = (pExt->DiskLaser_FiringOffset.Get().Y - pExt->DiskLaser_FiringOffset.Get().X) >> 3;
//	pThis->DrawOffset.X = ((((pExt->DiskLaser_FiringOffset.Get().Y - pExt->DiskLaser_FiringOffset.Get().X) >> 4)
//					+ (nOffset -1) & (((nFace >> 11) + 1) >> 1)))
//					% nOffset;
//
//	return 0x4A7329;
//}
//
//DEFINE_HOOK(0x4A748E , DiskLaserClass_Update_CalculatePoint, 0x5)
//{
//	GET(DiskLaserClass*, pThis, ESI);
//
//	auto pExt = WeaponTypeExt::ExtMap.Find(pThis->Weapon);
//
//	if (!pExt->DiskLaser_FiringOffset.isset())
//		return 0x0;
//
//	auto nOffset = (pExt->DiskLaser_FiringOffset.Get().Y - pExt->DiskLaser_FiringOffset.Get().X) >> 3;
//
//	LEA_STACK(CoordStruct*, pCoord, 0x44);
//
//	auto nFLH = pThis->Owner->GetFLH(pCoord, 0, CoordStruct::Empty);
//
//	R->Stack(0x38, nFLH->X);
//	R->Stack(0x3C, nFLH->Y);
//	R->EBP(nFLH->Z);
//	R->ECX(pThis->DrawOffset.Y);
//
//	R->EBX((pThis->DrawOffset.X + nOffset - pThis->DrawOffset.Y) % nOffset);
//	R->EDX((nOffset - pThis->DrawOffset.Y + pThis->DrawOffset.X - 1) % nOffset);
//	R->EAX((pThis->DrawOffset.X + pThis->DrawOffset.Y) % nOffset);
//	R->EDI((pThis->DrawOffset.Y + pThis->DrawOffset.X + 1) % nOffset);
//
//	return 0x4A7507;
//}

DEFINE_HOOK(0x44D455, BuildingClass_Mission_Missile_EMPPulseBulletWeapon, 0x8)
{
#ifdef COMPILE_PORTED_DP_FEATURES

	GET(BuildingClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBP);
	GET_STACK(BulletClass*, pBullet, STACK_OFFSET(0xF0, -0xA4));
	LEA_STACK(CoordStruct*, pCoord, STACK_OFFSET(0xF0, -0x8C));

	if (pWeapon && pBullet)
	{
		CoordStruct src = pThis->GetFLH(0, pThis->GetRenderCoords());
		CoordStruct dest = *pCoord;
		auto const pTarget = pBullet->Target ? pBullet->Target : Map[dest];

		// Draw bullet effect
		Helpers_DP::DrawBulletEffect(pWeapon, src, dest, pThis, pTarget);
		// Draw particle system
		Helpers_DP::AttachedParticleSystem(pWeapon, src, pTarget, pThis, dest);
		// Play report sound
		Helpers_DP::PlayReportSound(pWeapon, src);
		// Draw weapon anim
		Helpers_DP::DrawWeaponAnim(pWeapon, src, dest, pThis, pTarget);
	}
#endif
	return 0;
}

DEFINE_HOOK(0x51A2EF, InfantryClass_PCP_Enter_Bio_Reactor_Sound, 0x6)
{
	GET(BuildingClass*, pBuilding, EDI);

	auto const nSound = BuildingTypeExt::ExtMap.Find(pBuilding->Type)->EnterBioReactorSound.Get(RulesGlobal->EnterBioReactorSound);
	
	if (nSound != -1) {
		VocClass::PlayAt(nSound, pBuilding->GetCoords(), 0);
	}

	return 0x51A30F;
}

DEFINE_HOOK(0x44DBBC, InfantryClass_PCP_Leave_Bio_Reactor_Sound, 0x7)
{
	GET(BuildingClass*, pThis, EBP);

	auto const nSound = BuildingTypeExt::ExtMap.Find(pThis->Type)->LeaveBioReactorSound.Get(RulesGlobal->LeaveBioReactorSound);
	
	if(nSound != -1) {
		VocClass::PlayAt(nSound, pThis->GetCoords(), 0);
	}

	return 0x44DBDA;
}

//TODO : Add PerTechnoOverride
DEFINE_HOOK(0x639DD8, TechnoClass_PlanningManager_DecideEligible, 0x5)
{
	enum { CanUse = 0x639DDD, ContinueCheck = 0x639E03 };

	GET(TechnoClass*, pThis, ESI);
	auto const pWhat = pThis->WhatAmI();

	if (pWhat == AbstractType::Infantry || pWhat == AbstractType::Unit || pWhat == AbstractType::Aircraft)
		return CanUse;

	return ContinueCheck;
}

//what ?
//DEFINE_HOOK(0x5F53DD, ObjectClass_NoRelative, 0x8)
//{
//	GET(ObjectClass*, pObject, ESI);
//	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFSET(0x24, 0x4));
//
//	if (TechnoClass* pThis = abstract_cast<TechnoClass*>(pObject))
//	{
//		const TechnoTypeClass* pType = pThis->GetTechnoType();
//		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
//
//		if (!args->IgnoreDefenses && pTypeExt->AllowMinHealth > 0)
//		{
//			R->EBP(pType->Strength);
//
//			//Ares Hook 0x5F53E5
//			return 0x5F53F3;
//		}
//	}
//
//	R->EAX(pObject->GetType());
//
//	return 0x5F53E5;
//}

DEFINE_HOOK(0x5F53ED, ObjectClass_ReceiveDamage_StackReused, 0x6)
{
	enum { DecideResult = 0x5F5416, RecalculateDamage = 0x5F53F3 };
	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFSET(0x24, 0x4));
	return args->IgnoreDefenses ? DecideResult : RecalculateDamage;
}

DEFINE_HOOK(0x5F5416, ObjectClass_AfterDamageCalculate, 0x6)
{
	enum
	{
		Nothing = 0x0,
		CheckForZeroDamage = 0x5F5456,
		DecideResult = 0x5F5498,
		SkipDecdeResult = 0x5F546A
	};

	GET(ObjectClass*, pObject, ESI);
	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFSET(0x24, 0x4));

	*reinterpret_cast<DWORD*>(&args->IgnoreDefenses) = pObject->GetType()->Strength;

	if (!(pObject->AbstractFlags & AbstractFlags::Techno)) {	
		return CheckForZeroDamage;
	}

	return Nothing;
}

DEFINE_HOOK(0x6FA167, TechnoClass_AI_DrainMoney, 0x5)
{
	enum { SkipGameCode = 0x6FA1C5 };

	GET(TechnoClass*, pThis, ESI);

	const auto pSource = pThis->DrainingMe;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pSource->GetTechnoType());
	const auto pRules = RulesClass::Instance();
	const auto nDrainDelay = pTypeExt->DrainMoneyFrameDelay.Get(pRules->DrainMoneyFrameDelay);

	if ((Unsorted::CurrentFrame % nDrainDelay) == 0)
	{
		if (auto nDrainAmount = pTypeExt->DrainMoneyAmount.Get(pRules->DrainMoneyAmount))
		{
			if (nDrainAmount > 0)
				nDrainAmount = Math::min(nDrainAmount, (int)pThis->Owner->Available_Money());
			else
				nDrainAmount = Math::max(nDrainAmount, -(int)pSource->Owner->Available_Money());

			if (nDrainAmount)
			{
				pThis->Owner->TransactMoney(-nDrainAmount);
				pSource->Owner->TransactMoney(nDrainAmount);

				if (pTypeExt->DrainMoney_Display)
				{
					const auto& displayCoords = pTypeExt->DrainMoney_Display_AtFirer ? pSource->Location : pThis->Location;
					FlyingStrings::AddMoneyString(true, nDrainAmount, pSource,
						pTypeExt->DrainMoney_Display_Houses, displayCoords,
						pTypeExt->DrainMoney_Display_Offset, ColorStruct::Empty);
				}
			}
		}
	}

	return SkipGameCode;
}

bool NOINLINE IsCrusable(ObjectClass* pVictim, TechnoClass* pAttacker)
{
	if (!pVictim || !pAttacker || pVictim->IsBeingWarpedOut())
		return false;

	if (pVictim->IsIronCurtained())
		return false;

	if (pAttacker->Owner && pAttacker->Owner->IsAlliedWith(pVictim))
		return false;

	auto const pVictimTechno = abstract_cast<TechnoClass*>(pVictim);

	if (!pVictimTechno)
		return false;

	auto const pWhatVictim = pVictim->WhatAmI();
	auto const pAttackerType = pAttacker->GetTechnoType();
	auto const pVictimType = pVictim->GetTechnoType();

	if (pAttackerType->OmniCrusher) {
		if (pWhatVictim == AbstractType::Building || pVictimType->OmniCrushResistant)
			return false;
	} else {
		if (pVictimTechno->Uncrushable || !pVictimType->Crushable)
			return false;
	}

	auto const pVictimTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pVictimType);

	if (pWhatVictim == AbstractType::Infantry)
	{
		auto const pAttackerTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pAttackerType);
		auto const& crushableLevel = static_cast<InfantryClass*>(pVictim)->IsDeployed() ?
			pVictimTechnoTypeExt->DeployCrushableLevel :
			pVictimTechnoTypeExt->CrushableLevel;

		if (pAttackerTechnoTypeExt->CrushLevel.Get(pAttacker) < crushableLevel.Get(pVictimTechno))
			return false;
	}

	if (TechnoExt::IsChronoDelayDamageImmune(abstract_cast<FootClass*>(pVictim)))
	{
		return false;
	}

	//auto const pExt = TechnoExt::ExtMap.Find(pVictimTechno);
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

DEFINE_HOOK(0x5F6CD0, ObjectClass_IsCrushable, 0x6)
{
	enum { SkipGameCode = 0x5F6D90 };

	GET(ObjectClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, STACK_OFFSET(0x8, -0x4));
	R->AL(IsCrusable(pThis, pTechno));

	return SkipGameCode;
}

DEFINE_HOOK(0x5F53F3, ObjectClass_ReceiveDamage_RecalculateDamages, 0x6)
{
	GET(ObjectClass*, pObject, ESI);
	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFSET(0x24, 0x4));

	if (TechnoClass* const pThis = abstract_cast<TechnoClass*>(pObject))
	{
		//const auto pExt = TechnoExt::ExtMap.Find(pThis);
		const auto pWHExt = WarheadTypeExt::ExtMap.Find(args->WH);
		const auto pThisType = pThis->GetTechnoType();

		//this already calculate distance damage from epicenter
		*args->Damage = MapClass::GetTotalDamage(*args->Damage, args->WH, pThisType->Armor , args->DistanceToEpicenter);

		if (pWHExt->RecalculateDistanceDamage
			&& args->Attacker 
			&& !(!pWHExt->RecalculateDistanceDamage_IgnoreMaxDamage && *args->Damage == RulesGlobal->MaxDamage)) {

			const auto range = args->Attacker->DistanceFrom(pObject);
			const auto range_factor = range / (pWHExt->RecalculateDistanceDamage_Add_Factor.Get() * 256);
			const auto add = (pWHExt->RecalculateDistanceDamage_Add.Get() * range_factor);

			const auto multiply = pow((pWHExt->RecalculateDistanceDamage_Multiply.Get()), range_factor);

			*args->Damage += std::clamp((static_cast<int>((*args->Damage + add) * multiply) - *args->Damage), pWHExt->RecalculateDistanceDamage_Min.Get(), pWHExt->RecalculateDistanceDamage_Max.Get());

			if(pWHExt->RecalculateDistanceDamage_ProcessVerses)
			*args->Damage *= GeneralUtils::GetWarheadVersusArmor(args->WH, pThisType->Armor);

			if (pWHExt->RecalculateDistanceDamage_Display || Phobos::Debug_DisplayDamageNumbers)
			{
				TechnoClass* pOwner = pWHExt->RecalculateDistanceDamage_Display_AtFirer ? args->Attacker : pThis;
				FlyingStrings::AddMoneyString(true, *args->Damage, pOwner,
					AffectedHouse::All, pOwner->Location,
					pWHExt->RecalculateDistanceDamage_Display_Offset, ColorStruct::Empty);
			}
		}

		return 0x5F5416;
	}

	return 0;
}

 //Patches TechnoClass::Kill_Cargo/KillPassengers (push ESI -> push EBP)
 //Fixes recursive passenger kills not being accredited
 //to proper techno but to their transports
DEFINE_HOOK(0x707CF2, TechnoClass_KillCargo_FixKiller, 0x8)
{
	GET(TechnoClass*, pKiller, EBP);
	GET(TechnoClass*, pCargo, ESI);

	pCargo->KillCargo(pKiller);
	return 0x707CFA;
}

// Redirect UnitClass::GetFLH to InfantryClass::GetFLH (used to be TechnoClass::GetFLH)
DEFINE_JUMP(VTABLE, 0x7F5D20, 0x523250);

DEFINE_HOOK(0x47257C, CaptureManagerClass_TeamChooseAction_Random, 0x6)
{
	GET(FootClass*, pFoot, EAX);

	if (const auto pTeam = pFoot->Team)
	{
		if (auto nTeamDecision = pTeam->Type->MindControlDecision)
		{
			if (nTeamDecision > 5)
				nTeamDecision = ScenarioClass::Instance->Random.RandomRanged(1, 5);

			R->EAX(nTeamDecision);
			return 0x47258F;
		}
	}

	return 0x4725B0;
}