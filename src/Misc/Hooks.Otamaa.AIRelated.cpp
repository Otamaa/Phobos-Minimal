#include "Hooks.Otamaa.h"

#include <Ext/BuildingType/Body.h>
#include <Ext/Building/Body.h>

#include <FootClass.h>
#include <UnitClass.h>
#include <TeamClass.h>
#include <HouseClass.h>

#ifdef _Teamstuffs
ASMJIT_PATCH(0x6EBB86, TeamClass_MoveToFocus_IsInStray, 0x6)
{
	GET(FootClass*, pFoot, ESI);
	GET(TeamClass*, pThis, EBP);

	if (pFoot->GetHeight() > 0 && pFoot->WhatAmI() == UnitClass::AbsID && pThis->Target)
	{
		auto nCoord = pFoot->GetCoords();
		auto nCoord_target = pThis->Target->GetCoords();
		R->EAX((int)nCoord_target.DistanceFrom(nCoord));
	}
	else
		R->EAX(pFoot->DistanceFrom(pThis->Zone));

	return 0x6EBB91;
}

ASMJIT_PATCH(0x6EBE69, TeamClass_MoveToFocus_SetDestination, 0xA)
{
	GET(FootClass*, pFoot, ESI);

	auto const pType = pFoot->GetTechnoType();

	return (pType->BalloonHover
		|| (pType->WhatAmI() == UnitTypeClass::AbsID
			&& static_cast<UnitTypeClass*>(pType)->JumpJet
			&& static_cast<UnitTypeClass*>(pType)->IsSimpleDeployer)) ?
		0x6EBE9C : 0x6EBE82;
}

ASMJIT_PATCH(0x6EBEDB, TeamClass_MoveToFocus_BalloonHover, 0xA)
{
	GET(FootClass*, pFoot, ESI);

	auto const pType = pFoot->GetTechnoType();

	return (pType->BalloonHover
		|| (pType->WhatAmI() == UnitTypeClass::AbsID
			&& static_cast<UnitTypeClass*>(pType)->JumpJet
			&& static_cast<UnitTypeClass*>(pType)->IsSimpleDeployer)) ?

		0x6EBEEF : 0x6EBEFF;
}
#endif

//?TODO : replace these with patches
#pragma region HouseCheckReplace

//DEFINE_PATCH_TYPED(BYTE, 0x4FAD6B
//	, 0x50 // push eax
//	, 0x8B, 0xCF // mov ecx edi
//	, 0xE8, 0xDD , 0xEC , 0xFF , 0xFF // 4FAD6E , call HouseClass::IsAlliedWith
//	, 0x83, 0xF8 , 0x01
//	, 0x75, 0x64 // if ally , 4FAD7E -> 0x4FADD9
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//);
#include <Ext/SWType/Body.h>
#include <Ext/Super/Body.h>

ASMJIT_PATCH(0x509E00, HouseClass_LS_RemoveTargetLimitIfAvaible, 0x6)
{
	//GET(HouseClass*, pThis, ESI);
	GET_STACK(FakeSuperClass*, pSuper , 0x4); // we know that this is LS ?

	if (pSuper->Type->Type == SuperWeaponType::LightningStorm && pSuper->_GetTypeExtData()->Weather_UseSeparateState) {
		return 0x509E13;
	}

	return 0x0;
}

ASMJIT_PATCH(0x4FAD64, HouseClass_SpecialWeapon_Update, 0x7)
{
	GET(HouseClass*, pThis, EDI);
	GET(FakeBuildingClass*, pThat, ESI);

	if (!pThat->IsAlive || pThat->_GetExtData()->LimboID != -1)
		return 0x4FADD9;

	return pThis->IsAlliedWith(pThat->GetOwningHouse()) ? 0x4FADD9 : 0x4FAD9E;
}

//DEFINE_PATCH_TYPED(BYTE, 0x50A24D
//	, 0x50 // push eax
//	, 0x8B, 0xCF // mov ecx edi //
//	, 0xE8, 0xFB , 0x07 , 0x10 , 0x00 // 50A250 , call HouseClass::IsAlliedWith (0x4F9A50)
//	, 0x83, 0xF8 , 0x00
//	, 0x75, 0x64 // if not ally , -> 0x50A292 , change 0x64
//	  skip untils 50A278 , check the NOP amount
//	  skip untils 50A278 , check the NOP amount
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
//);

ASMJIT_PATCH(0x50A23A, HouseClass_Target_Dominator, 0x6)
{
	GET(HouseClass*, pThis, EDI);
	GET(TechnoClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->Owner) ? 0x50A292 : 0x50A278;
}

ASMJIT_PATCH(0x50A04B, HouseClass_Target_GenericMutator, 0x7)
{
	GET(HouseClass*, pThis, EBX);
	GET(TechnoClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->Owner) ? 0x50A096 : 0x50A087;
}

ASMJIT_PATCH(0x5094F9, HouseClass_AdjustThreats, 0x6)
{
	return R->EBX<HouseClass*>()->IsAlliedWith(R->ESI<HouseClass*>()) ? 0x5095B6 : 0x509532;
}

ASMJIT_PATCH(0x4F9432, HouseClass_Attacked, 0x6)
{
	return R->EDI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x4F9474 : 0x4F9478;
}

ASMJIT_PATCH(0x4FBD1C, HouseClass_DoesEnemyBuildingExist, 0x6)
{
	return R->ESI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x4FBD57 : 0x4FBD47;
}

ASMJIT_PATCH(0x5003BA, HouseClass_FindJuicyTarget, 0x6)
{
	return R->EDI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x5003F7 : 0x5004B1;
}

ASMJIT_PATCH(0x4F9A90, HouseClass_IsAlly_ObjectClass, 0x7)
{
	GET_STACK(ObjectClass*, pTarget, 0x4);
	GET(HouseClass*, pThis, ECX);
	GET_STACK(DWORD , caller , 0x0);

	bool result = false;
	if (pTarget) {

		bool eligible = true;
		if(auto pTech = flag_cast_to<TechnoClass*, false>(pTarget)){
			if(!pTarget->IsAlive){
				Debug::LogInfo(__FUNCTION__" DeadTechno[{}] is used , called from [{}]", (void*)pTarget, (unsigned)caller);
				eligible = false;
			}
		}

		if(eligible)
			result = pThis->IsAlliedWith(pTarget->GetOwningHouse());
	}

	R->AL(result);
	return 0x4F9ADE;
}

//breaking stack ??
// ASMJIT_PATCH(0x4F9A50, HouseClass_IsAlly_HouseClass, 0x6)
// {
// 	GET_STACK(HouseClass*, pTarget, 0x4);
// 	GET(HouseClass*, pThis, ECX);
//
// 	bool result = false;
// 	if (pTarget) {
// 		result = pThis->IsAlliedWith(pTarget->ArrayIndex);
// 	}
//
// 	R->EAX(result);
// 	return 0x4F9A8C;
// }

 ASMJIT_PATCH(0x4F9B0A, HouseClass_IsAlly_AbstractClass, 0x6)
 {
 	GET(HouseClass*, pThis, ESI);
 	GET(HouseClass*, pThat, EAX);

 	R->AL(pThis->IsAlliedWith(pThat));
 	return 0x4F9B43;
 }

ASMJIT_PATCH(0x501548, HouseClass_IsAllowedToAlly_1, 0x6)
{
	return R->ESI<HouseClass*>()->IsAlliedWith(R->EDI<HouseClass*>()) ? 0x501575 : 0x50157C;
}

ASMJIT_PATCH(0x5015F2, HouseClass_IsAllowedToAlly_2, 0x6)
{
	return 0x501628 - (int)R->ESI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>());
}

ASMJIT_PATCH(0x4F9D01, HouseClass_MakeAlly_3, 0x6)
{
	return R->ESI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x4F9D34 : 0x4F9D40;
}

ASMJIT_PATCH(0x4F9E10, HouseClass_MakeAlly_4, 0x8)
{
	return R->EBP<HouseClass*>()->IsAlliedWith(R->ESI<HouseClass*>()) ? 0x4F9E49 : 0x4F9EC9;
}

ASMJIT_PATCH(0x4F9E56, HouseClass_MakeAlly_5, 0x9)
{
	GET(HouseClass* , pHouse , EBP);
	GET(HouseClass* , pHouse_2 , ESI);

	if (!pHouse_2->IsAlliedWith(HouseClass::CurrentPlayer()))
		return 0x4F9EBD;

	return pHouse->IsAlliedWith(HouseClass::CurrentPlayer()) ? 0x4F9EB1 : 0x4F9EBD;
}

#pragma endregion

#ifdef _FUllBackport
//these are different depend on the thing
BuildingClass* __fastcall Find_Enemy_Building(
		BuildingTypeClass* type,
		HouseClass* house,
		TechnoClass* attacker,
		int find_type,
		bool OnlyTargetHouseEnemy)
{
	if (BuildingClass::Array->Count <= 0) {
		return nullptr;
	}

	int v10 = -1;
	int v29 = -1;
	int v30 = -1;
	BuildingClass* last = nullptr;
	BuildingClass* last2 = nullptr;

	for (auto pBld : *BuildingClass::Array) {
		if (pBld->Type != type)
			continue;

		bool IsSameHouse = pBld->Owner == house;
		if (pBld->Owner != house && !pBld->Owner->Type->MultiplayPassive && attacker->Owner->IsAlliedWith(pBld)) {
			continue;
		}

		if (pBld->InLimbo || BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
			continue;

		switch (find_type)
		{
		case 0:
		{
			auto coord = pBld->GetCoords();
			auto cell = CellClass::Coord2Cell(coord);
			v10 = -1 - MapClass::Instance->GetThreatPosed(cell, attacker->Owner);

			break;
		}
		case 1:
		{
			auto coord = pBld->GetCoords();
			auto cell = CellClass::Coord2Cell(coord);
			v10 = MapClass::Instance->GetThreatPosed(cell, attacker->Owner);

			break;
		}
		case 2:
		{
			v10 = -1 - (int)(attacker->Location - pBld->Location).Length();
			break;
		}
		case 3:
		{
			v10 = (int)(attacker->Location - pBld->Location).Length();
			break;
		}
		default:
			break;
		}

		if (v10 > v29 && IsSameHouse) {
			last = pBld;
			v29 = v10;
		}

		if (v10 > v30) {
			last2 = pBld;
			v30 = v10;
		}
	}

	if (!last || v29 <= v30) {
		if (!OnlyTargetHouseEnemy) {
			return last2;
		}
	}

	return last;
}

BuildingClass* __fastcall Find_Own_Building(
		BuildingTypeClass* type,
		HouseClass* unused,
		TechnoClass* attacker,
		int find_type)
{
	if (!attacker || attacker->Owner->Buildings.Count <= 0 )
	{
		return nullptr;
	}

	int v30 = -1;
	BuildingClass* last = nullptr;

	for (auto& pBld : attacker->Owner->Buildings)
	{
		int v10 = -1;

		if (pBld->Type == type){
			if (!pBld->InLimbo && BuildingExtContainer::Instance.Find(pBld)->LimboID <= -1) {
				switch (find_type)
				{
				case 0:
				{
					auto coord = pBld->GetCoords();
					v10 = -1 - MapClass::Instance->GetThreatPosed(coord, attacker->Owner);

					break;
				}
				case 1:
				{
					auto coord = pBld->GetCoords();
					v10 = MapClass::Instance->GetThreatPosed(coord, attacker->Owner);

					break;
				}
				case 2:
				{
					v10 = -1 - (int)(pBld->Location - attacker->Location).Length();
					break;
				}
				case 3:
				{
					v10 = (int)(pBld->Location - attacker->Location).Length();
					break;
				}
				default:
					break;
				}
			}
		}

		if (v10 > v30) {
			v30 = v10;
			last = pBld;
		}
	}

	return last;
}

DEFINE_FUNCTION_JUMP(CALL, 0x6EE376, Find_Enemy_Building)
DEFINE_FUNCTION_JUMP(CALL, 0x6EE45D, Find_Enemy_Building)
DEFINE_FUNCTION_JUMP(CALL, 0x6EFFAF, Find_Enemy_Building)
DEFINE_FUNCTION_JUMP(CALL, 0x6EE693, Find_Own_Building)
#else

ASMJIT_PATCH(0x6EEC6D, FindTargetBuilding_LimboDelivered, 0x6)
{
	enum { advance = 0x6EEE21, ret = 0x0 };
	GET(BuildingClass*, pBuilding, ESI);

	if (!pBuilding->IsAlive)
		return advance;

	return BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1 ?
		advance : ret;
}


ASMJIT_PATCH(0x6EEEF2, FindOwnBuilding_LimboDelivered, 0xA)
{
	enum { advance = 0x6EF0D7, ret = 0x0 };
	GET(BuildingClass*, pBuilding, ESI);

	if (pBuilding->InLimbo || !pBuilding->IsAlive)
		return advance;

	return BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1 ?
		advance : ret;
}

#endif

ASMJIT_PATCH(0x6EA184, TeamClass_Regroup_LimboDelivered, 0x6)
{
	enum { advance = 0x6EA38C, ret = 0x6EA192 };
	GET(BuildingClass*, pBuilding, ESI);

	if(!pBuilding->IsAlive || pBuilding->InLimbo)
		return advance;

	return BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1 ?
		advance : ret;
}


ASMJIT_PATCH(0x6EE8D9, TeamClass_Scout_LimboDelivered, 0x9)
{
	enum { advance = 0x6EE928, ret = 0x0 };
	GET(BuildingClass**, pBuilding, ESI);

	if ((*pBuilding)->InLimbo || !(*pBuilding)->IsAlive)
		return advance;

	return BuildingExtContainer::Instance.Find(*pBuilding)->LimboID != -1 ?
		advance : ret;
}

//ASMJIT_PATCH(0x6F7D90, TechnoClass_EvalueateObject_Forbidden, 0x6)
//{
//	GET(ObjectClass*, pTarget, ESI);
//
//	// alive check here was too late
//	if (pTarget->InLimbo)
//		return 0x6F894F;
//
//
//
//	return 0x6F7D9E;
//}
