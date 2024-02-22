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

/*
DEFINE_PATCH_TYPED(BYTE, 0x4FAD64
	, 0x88, 0x16
	, 0x8B, 0xCE
	, 0xFF, 0x52, 0x3C
	, 0x50
	, 0x8B, 0xCF
	, 0xE8

);
*/

DEFINE_HOOK(0x4FAD64, HouseClass_SpecialWeapon_Update, 0x7)
{
	GET(HouseClass*, pThis, EDI);
	GET(BuildingClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->GetOwningHouse()) ? 0x4FADD9 : 0x4FAD9E;
}

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

DEFINE_HOOK(0x6F7D90, TechnoClass_Threat_Forbidden, 0x6)
{
	GET(ObjectClass*, pTarget, ESI);

	if (pTarget->InLimbo || !pTarget->IsAlive)
		return 0x6F894F;

	if (const auto pTechno = generic_cast<TechnoClass*>(pTarget))
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
