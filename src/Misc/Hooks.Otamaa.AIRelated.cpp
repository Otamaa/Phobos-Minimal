#include "Hooks.Otamaa.h"

#include <Ext/BuildingType/Body.h>
#include <Ext/Building/Body.h>

#include <FootClass.h>
#include <UnitClass.h>
#include <TeamClass.h>
#include <HouseClass.h>

DEFINE_HOOK(0x6EBB86, TeamClass_MoveToFocus_IsInStray, 0x6)
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
		R->EAX(pFoot->DistanceFrom(pThis->SpawnCell));

	return 0x6EBB91;
}

DEFINE_HOOK(0x6EBE69, TeamClass_MoveToFocus_SetDestination, 0xA)
{
	GET(FootClass*, pFoot, ESI);

	auto const pType = pFoot->GetTechnoType();

	return (pType->BalloonHover
		|| (pType->WhatAmI() == UnitTypeClass::AbsID
			&& static_cast<UnitTypeClass*>(pType)->JumpJet
			&& static_cast<UnitTypeClass*>(pType)->IsSimpleDeployer)) ?
		0x6EBE9C : 0x6EBE82;
}

DEFINE_HOOK(0x6EBEDB, TeamClass_MoveToFocus_BalloonHover, 0xA)
{
	GET(FootClass*, pFoot, ESI);

	auto const pType = pFoot->GetTechnoType();

	return (pType->BalloonHover
		|| (pType->WhatAmI() == UnitTypeClass::AbsID
			&& static_cast<UnitTypeClass*>(pType)->JumpJet
			&& static_cast<UnitTypeClass*>(pType)->IsSimpleDeployer)) ?

		0x6EBEEF : 0x6EBEFF;
}

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

DEFINE_HOOK(0x4FAD64, HouseClass_SpecialWeapon_Update, 0x7)
{
	GET(HouseClass*, pThis, EDI);
	GET(BuildingClass*, pThat, ESI);

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

DEFINE_HOOK(0x50A23A, HouseClass_Target_Dominator, 0x6)
{
	GET(HouseClass*, pThis, EDI);
	GET(TechnoClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->Owner) ? 0x50A292 : 0x50A278;
}

DEFINE_HOOK(0x50A04B, HouseClass_Target_GenericMutator, 0x7)
{
	GET(HouseClass*, pThis, EBX);
	GET(TechnoClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->Owner) ? 0x50A096 : 0x50A087;
}

DEFINE_HOOK(0x5094F9, HouseClass_AdjustThreats, 0x6)
{
	return R->EBX<HouseClass*>()->IsAlliedWith(R->ESI<HouseClass*>()) ? 0x5095B6 : 0x509532;
}

DEFINE_HOOK(0x4F9432, HouseClass_Attacked, 0x6)
{
	return R->EDI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x4F9474 : 0x4F9478;
}

DEFINE_HOOK(0x4FBD1C, HouseClass_DoesEnemyBuildingExist, 0x6)
{
	return R->ESI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x4FBD57 : 0x4FBD47;
}

DEFINE_HOOK(0x5003BA, HouseClass_FindJuicyTarget, 0x6)
{
	return R->EDI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x5003F7 : 0x5004B1;
}

DEFINE_HOOK(0x5047F5, HouseClass_UpdateAngetNodes, 0x6)
{
	return R->EAX<HouseClass*>()->IsAlliedWith(R->EDX<HouseClass*>()) ? 0x504826 : 0x504820;
}

DEFINE_HOOK(0x4F9A90, HouseClass_IsAlly_ObjectClass, 0x7)
{
	GET_STACK(TechnoClass*, pTarget, 0x4);
	GET(HouseClass*, pThis, ECX);

	bool result = false;
	if (pTarget)
	{
		result = pThis->IsAlliedWith(pTarget->GetOwningHouse());
	}

	R->AL(result);
	return 0x4F9ADE;
}

//breaking stack ??
// DEFINE_HOOK(0x4F9A50, HouseClass_IsAlly_HouseClass, 0x6)
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

DEFINE_HOOK(0x4F9B0A, HouseClass_IsAlly_AbstractClass, 0x6)
{
	GET(HouseClass*, pThis, ESI);
	GET(HouseClass*, pThat, EAX);

	R->AL(pThis->IsAlliedWith(pThat));
	return 0x4F9B43;
}

DEFINE_HOOK(0x501548, HouseClass_IsAllowedToAlly_1, 0x6)
{
	return R->ESI<HouseClass*>()->IsAlliedWith(R->EDI<HouseClass*>()) ? 0x501575 : 0x50157C;
}

DEFINE_HOOK(0x5015F2, HouseClass_IsAllowedToAlly_2, 0x6)
{
	return 0x501628 - (int)R->ESI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>());
}

DEFINE_HOOK(0x4F9D01, HouseClass_MakeAlly_3, 0x6)
{
	return R->ESI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x4F9D34 : 0x4F9D40;
}

DEFINE_HOOK(0x4F9E10, HouseClass_MakeAlly_4, 0x8)
{
	return R->EBP<HouseClass*>()->IsAlliedWith(R->ESI<HouseClass*>()) ? 0x4F9E49 : 0x4F9EC9;
}

DEFINE_HOOK(0x4F9E56, HouseClass_MakeAlly_5, 0x9)
{
	GET(HouseClass* , pHouse , EBP);
	GET(HouseClass* , pHouse_2 , ESI);

	if (!pHouse_2->IsAlliedWith(HouseClass::CurrentPlayer()))
		return 0x4F9EBD;

	return pHouse->IsAlliedWith(HouseClass::CurrentPlayer()) ? 0x4F9EB1 : 0x4F9EBD;
}

#pragma endregion

DEFINE_HOOK(0x6EA192, TeamClass_Regroup_LimboDelivered, 0x6)
{
	enum { advance = 0x6EA38C, ret = 0x0 };
	GET(BuildingClass*, pBuilding, ESI);
	return BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1 ?
		advance : ret;
}

DEFINE_HOOK(0x6EEC6D, TeamClass_FindTargetBuilding_LimboDelivered, 0x6)
{
	enum { advance = 0x6EEE45, ret = 0x0 };
	GET(BuildingClass*, pBuilding, ESI);
	return BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1 ?
		advance : ret;
}

DEFINE_HOOK(0x6EE8D9, TeamClass_Scout_LimboDelivered, 0x9)
{
	enum { advance = 0x6EE928, ret = 0x0 };
	GET(BuildingClass**, pBuilding, ESI);
	return BuildingExtContainer::Instance.Find(*pBuilding)->LimboID != -1 ?
		advance : ret;
}

DEFINE_HOOK(0x6EEEF2, TeamClass_6EEEA0_LimboDelivered, 0xA)
{
	enum { advance = 0x6EF0D7, ret = 0x0 };
	GET(BuildingClass*, pBuilding, ESI);
	return BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1 ?
		advance : ret;
}

DEFINE_HOOK(0x6F7D90, TechnoClass_Threat_Forbidden, 0x6)
{
	GET(ObjectClass*, pTarget, ESI);

	if (pTarget->InLimbo || !pTarget->IsAlive)
		return 0x6F894F;

	if (const auto pTechno = flag_cast_to<TechnoClass*>(pTarget))
	{

		if (pTechno->IsCrashing || pTechno->IsSinking)
			return 0x6F894F;

		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType());

		if (pTypeExt->IsDummy)
			return 0x6F894F;

		switch (pTechno->WhatAmI())
		{
		case AbstractType::Building:
		{
			const auto pBld = (BuildingClass*)pTarget;

			if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
				return 0x6F894F;

			break;
		}
		case AbstractType::Unit:
		{

			const auto pUnit = (UnitClass*)pTarget;

			if (pUnit->DeathFrameCounter > 0)
				return 0x6F894F;

			break;
		}
		default:
			break;
		}
	}

	return 0x6F7D9E;
}
