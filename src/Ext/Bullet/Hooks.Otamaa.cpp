#include "Body.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Helpers.h>

#include <TechnoClass.h>
#include <TacticalClass.h>

#include <Fundamentals.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif

#pragma region Otamaa
/*
static DamageAreaResult __fastcall BulletClass_DamageArea
(
	CoordStruct* pCoord,
	int nDamage,
	TechnoClass* pSource,
	WarheadTypeClass* pWarhead,
	bool AffectTiberium, //true
	HouseClass* pSourceHouse //nullptr
)
{
	if (auto const BulletOwner = pSource && !pSourceHouse ? pSource->GetOwningHouse() : HouseExt::FindCivilianSide())
		pSourceHouse = BulletOwner;

	auto const nCoord = *pCoord;
	return Map.DamageArea(nCoord, nDamage, pSource, pWarhead, pWarhead->Tiberium, pSourceHouse);
}

//DEFINE_POINTER_CALL(0x469A83, &BulletClass_DamageArea);
*/

DEFINE_HOOK(0x46889D, BulletClass_Unlimbo_FlakScatter_SetTargetCoords, 0x8)
{
	GET(BulletClass*, pThis, EBX);
	pThis->TargetCoords = { R->EAX<int>(),R->EDX<int>(),R->ESI<int>() };
	return 0x0;
}

DEFINE_HOOK(0x46B1D6, BulletClass_DrawVXL_Palette, 0x6)
{
	GET_STACK(BulletClass*, pThis, STACK_OFFS(0xF8, 0xE4));
	GET(BulletTypeClass*, pThisType, EDX);
	GET(Point2D*, pPoint, ECX);
	GET(int, nRect_X, EBP);
	GET(int, nRect_Y, ESI);

	R->Stack(STACK_OFFS(0xF8, 0xE4), Point2D { pPoint->X + nRect_X , pPoint->Y + nRect_Y });
	R->EAX(ColorScheme::Array()->Items);

	int nIdx = pThisType->Color;
	if (pThisType->FirersPalette && (pThis->InheritedColor != -1))
		nIdx = pThis->InheritedColor;

	R->ECX(nIdx);

	return 0x46B1F2;
}

DEFINE_HOOK(0x5F5A86, ObjectClass_SpawnParachuted_Animation_Bulet, 0x6)
{
	GET(RulesClass*, pRules, ECX);
	GET(BulletClass*, pBullet, ESI);

	if (auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type))
	{
		R->EDX(pBulletTypeExt->Parachute.Get(pRules->BombParachute));
		return 0x5F5A8C;
	}

	return 0x0;
}

#ifdef ENABLE_BULLETADJUSTVELHOOKS
DEFINE_HOOK(0x466D19, BulletClass_Update_AdjustingVelocity, 0x6)
{
	R->ECX(R->EBP());
	return 0;
}

/*
static int __fastcall ProjectileMotion_Exec(
CoordStruct* pCoord,
BulletVelocity* pVel,
CoordStruct* pSecondCoord,
DirStruct* pDir,
bool bInAir,
bool bAirburs,
BulletClass* pBullet, //replaced by hook above
bool bLevel)
{
	return BulletClass::ProjectileMotion(pCoord, pVel, pSecondCoord, pDir, bInAir, bAirburs, pBullet->Type->VeryHigh, bLevel);
}

DEFINE_POINTER_CALL(0x466D31, ProjectileMotion_Exec);

DEFINE_HOOK(0x5B260B, BulletClass_ProjectileMotion_Fix1, 0x7)
{
	GET_BASE(BulletClass*, pBullet, 0x18);
	R->AL(pBullet->Type->VeryHigh);
	R->ECX(R->Stack<DWORD>(0x38));
	return 0x5B2612;
}*/

DEFINE_HOOK(0x5B271A, BulletClass_ProjectileMotion_Fix2, 0x5)
{
	GET_BASE(BulletClass*, pBullet, 0x18);
	return pBullet->Type->VeryHigh ? 0x5B272D : 0x5B2721;
}

DEFINE_HOOK(0x5B2778, BulletClass_ProjectileMotion_AscentAngle, 0x7)
{
	//GET_BASE(BulletClass*, pBullet, 0x18);
	R->Stack(0x18, (Math::clamp((0x4000 - 0x2000), 0, 0x4000)));
	return 0x5B277F;
}

DEFINE_HOOK(0x5B260B, BulletClass_ProjectileMotion_DescentAngle, 0x7)
{
	GET_BASE(BulletClass*, pBullet, 0x18);
	GET_STACK(int, nPointLength, 0x38);

	int DescentAngle = 3;
	return nPointLength <=
		((pBullet->Type->VeryHigh ? 6 : DescentAngle) << 8)
		? 0x5B289C : 0x5B2627;
}

DEFINE_HOOK(0x5B2721, BulletClass_ProjectileMotion_Cruise, 0x5)
{
	//GET_BASE(BulletClass*, pBullet, 0x18);
	GET(int, nLepton, EAX);

	bool bLockedOnTrajectory = false;
	int nCruiseLevel = 5;

	if (bLockedOnTrajectory || nLepton >= nCruiseLevel)
		nLepton = nCruiseLevel;

	R->EAX(nLepton);
	return 0x5B2732;
}

DEFINE_HOOK(0x466BBC, BulletClass_AI_MissileROTVar, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	GET(RulesClass*, pRules, ECX);

	double dRes = pRules->MissileROTVar;
	if (auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
	{
		dRes = pBulletTypeExt->MissileROTVar.Get(pRules->MissileROTVar);
	}

	R->ESI(&dRes);
	return 0x466BC2;
}

DEFINE_HOOK(0x466E9F, BulletClass_AI_MissileSafetyAltitude, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	GET(int, comparator, EAX);

	int nAltitude = RulesGlobal->MissileSafetyAltitude;
	if (auto const& pBulletTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
	{
		nAltitude = pBulletTypeExt->MissileSafetyAltitude.Get(RulesGlobal->MissileSafetyAltitude);
	}

	return comparator >= nAltitude
		? 0x466EAD : 0x466EB6;
}

#endif

#ifdef ENABLE_CELLSPREAD_LOCOWH
static void  ManipulateLoco(FootClass* pFirer, AbstractClass* pTarget, BulletClass* pBullet, bool Area)
{
	AbstractClass* pTarget_1 = pTarget;

	if (!Area)
	{
		if (pFirer->LocomotorTarget)
		{
			pFirer->LocomotorImblued(false);
		}

		pTarget_1 = pBullet->Target;
	}

	if (auto pFoot_T = generic_cast<FootClass*>(pTarget_1))
	{
		if (pTarget_1->WhatAmI() == AbstractType::Aircraft || pTarget_1->WhatAmI() == AbstractType::Unit)
		{
			if (auto pExt = WarheadTypeExt::ExtMap.Find(pBullet->WH))
			{
				if (!pExt->CanTargetHouse(pBullet->GetOwningHouse(), pFoot_T))
					return;

				if (!pExt->CanDealDamage(pFoot_T))
					return;
			}

			if (auto pUnit = specific_cast<UnitClass*>(pTarget))
			{

				if (pFoot_T->RadioLinks.IsAllocated &&
					pFoot_T->RadioLinks.IsInitialized &&
					pFoot_T->RadioLinks.Items)
				{
					auto pLink = *pFoot_T->RadioLinks.Items;
					if (pLink && pLink->WhatAmI() == AbstractType::Building && ((BuildingClass*)pLink)->Type->WeaponsFactory)
					{
						if (Map[pFoot_T->Location]->GetBuilding() == pLink)
							return;
					}
				}

				if (pUnit->Deploying || pUnit->Undeploying)
				{
					return;
				}
			}

			if (pFoot_T->InWhichLayer() != Layer::Ground || pFoot_T->IsAttackedByLocomotor)
				return;

			pFirer->FootClass_ImbueLocomotor(pFoot_T, pBullet->WH->Locomotor);
		}
	}
}

DEFINE_HOOK(0x4694CB, BulletClass_Logics_Locomotor, 0x6)
{
	enum
	{
		IsNotLocoRet = 0x469705
		, Return = 0x469AA4
	};

	GET(BulletClass*, pThis, ESI);
	GET(WarheadTypeClass*, pWH, EAX);

	if (!pWH->IsLocomotor)
		return IsNotLocoRet;

	if (!pThis->Owner)
		return Return;

	auto pTarget = pThis->Target;
	if (pThis->Owner->LocomotorTarget == pTarget
	  || pThis->Owner->LocomotorSource
	  || (pThis->Owner->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None && ((FootClass*)pThis->Owner)->unknown_bool_6AC)
	{
		return Return;
	}

	if (pWH->CellSpread == 0.0)
		ManipulateLoco((FootClass*)pThis->Owner, pTarget, pThis, false);
	else
	{
		for (auto pTarget_s : Helpers::Alex::getCellSpreadItems(pTarget->GetCoords(), pWH->CellSpread, true))
		{ ManipulateLoco((FootClass*)pThis->Owner, pTarget_s, pThis, true); }
	}

	return Return;
}

DEFINE_HOOK(0x707B95, TechnoClass_PointerExpired_LocoSource, 0x7)
{
	GET(TechnoClass*, pAttacker, ECX);
	GET(TechnoClass*, pVictim, ESI);

	if (pAttacker->LocomotorTarget == pVictim)
	{
		pAttacker->LocomotorImblued(true);
		pAttacker->LocomotorTarget = nullptr;
		pVictim->LocomotorSource = nullptr;
		return 0x707BB2;
	}

	if (!pVictim || (pVictim->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
	{
		pVictim->LocomotorSource = nullptr;
		return 0x707BB2;
	}

	if (pVictim->GetHeight() > 0)
	{
		pVictim->IsCrashing = 1;
		pVictim->IsBeingManipulated = 1;
		pVictim->BeingManipulatedBy = pVictim;
		pVictim->ChronoWarpedByHouse = pAttacker->Owner;
		pVictim->Stun();
		pVictim->SetDestination(nullptr, true);
	}

	if (auto pUnitV = specific_cast<UnitClass*>(pVictim))
		pUnitV->unknown_int_6D4 = 1;

	pVictim->SetDestination(nullptr, true);
	pVictim->LocomotorSource = nullptr;
	return 0x707BB2;
}

DEFINE_HOOK(0x7102F9, FootClass_ImbueLocomotor_SetDestination, 0x5)
{
	GET(TechnoClass*, pThis, EBX);
	GET(FootClass*, pThat, ESI);

	pThis->LocomotorTarget = pThat;
	pThat->LocomotorSource = generic_cast<FootClass*>(pThis);

	if (!pThis->GetTechnoType()->Insignificant
	  || pThat->WhatAmI() != AbstractType::Aircraft)
	{
		pThat->SetDestination(pThis, true);
		pThat->Deselect();
	}
	else
	{
		pThis->Scatter(CoordStruct::Empty, true, false);
		pThis->Deselect();
	}

	return 0x71033A;
}
#endif
