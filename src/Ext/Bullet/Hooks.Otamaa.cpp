#include "Body.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/House/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Helpers.h>

#include <TechnoClass.h>
#include <TacticalClass.h>

#include <Fundamentals.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

DEFINE_HOOK(0x46B1D6, BulletClass_DrawVXL_Palette, 0x6)
{
	GET_STACK(BulletClass*, pThis, STACK_OFFS(0xF8, 0xE4));
	GET(BulletTypeClass*, pThisType, EDX);
	GET(Point2D*, pPoint, ECX);
	GET(int, nRect_X, EBP);
	GET(int, nRect_Y, ESI);

	R->Stack(STACK_OFFS(0xF8, 0xE4), Point2D { pPoint->X + nRect_X , pPoint->Y + nRect_Y });
	R->EAX(ColorScheme::Array()->Items);
	R->ECX((pThis->InheritedColor != -1) ? pThis->InheritedColor : pThisType->Color);

	return 0x46B1F2;
}

DEFINE_HOOK(0x5F5A86, ObjectClass_SpawnParachuted_Animation_Bulet, 0x6)
{
	GET(RulesClass*, pRules, ECX);
	GET(BulletClass*, pBullet, ESI);

	R->EDX(BulletTypeExt::ExtMap.Find(pBullet->Type)->Parachute.Get(pRules->BombParachute));
	return 0x5F5A8C;
}

#pragma region Otamaa

//DEFINE_HOOK(0x469D12, BulletClass_Logics_CheckDoAirburst_MaxDebris, 0x8)
//{
//	GET(BulletClass*, pThis, ESI);
//	GET(int, nMaxCount, EAX);
//
//	return (nMaxCount > 0) ? 0x469D1A : 0x0;
//}

DEFINE_HOOK(0x469D3C, BulletClass_Logics_Debris, 0xA)
{
	GET(BulletClass*, pThis, ESI);
	GET(int, nTotalSpawn, EBX);
	GET(WarheadTypeClass*, pWarhead, EAX);

	auto pExt = BulletExt::ExtMap.Find(pThis);
	HouseClass* const pOWner = pThis->Owner ? pThis->Owner->GetOwningHouse() : (pExt->Owner ? pExt->Owner : HouseExt::FindCivilianSide());
	HouseClass* const Victim = (pThis->Target) ? pThis->Target->GetOwningHouse() : nullptr;
	CoordStruct nCoords { 0,0,0 };
	auto const& nDebrisTypes = pWarhead->DebrisTypes;

	if (nDebrisTypes.Count > 0 && pWarhead->DebrisMaximums.Count > 0)
	{
		nCoords = pThis->GetCoords();
		for (int nCurIdx = 0; nCurIdx < nDebrisTypes.Count; ++nCurIdx)
		{
			if (nCurIdx > pWarhead->DebrisMaximums.Count)
				break;

			if (!pWarhead->DebrisMaximums[nCurIdx])
				continue;

				int nAmountToSpawn = abs(int(ScenarioClass::Instance->Random.Random())) % pWarhead->DebrisMaximums[nCurIdx];
				nAmountToSpawn = LessOrEqualTo(nAmountToSpawn, nTotalSpawn);
				nTotalSpawn -= nAmountToSpawn;

				for (; nAmountToSpawn > 0; --nAmountToSpawn)
				{
					if (auto const pVoxelAnimType = nDebrisTypes[nCurIdx])
						if (auto pVoxAnim = GameCreate<VoxelAnimClass>(pVoxelAnimType, &nCoords, pOWner))
						{
							if (auto pVoxelAnimExt = VoxelAnimExt::ExtMap.Find(pVoxAnim))
								pVoxelAnimExt->Invoker = pThis->Owner;
						}
				}

			if (nTotalSpawn <= 0)
			{
				nTotalSpawn = 0;
				break;
			}
		}
	}

	if (!nDebrisTypes.Count && (nTotalSpawn > 0))
	{
		const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);
		const auto AnimDebris = pWHExt->DebrisAnimTypes.GetElements(RulesClass::Instance->MetallicDebris);

		if (!AnimDebris.empty())
		{
			nCoords = pThis->GetCoords();
			nCoords.Z += 20;

			for (int i = 0; i < nTotalSpawn; ++i)
			{
				if (auto const pAnimType = AnimDebris[ScenarioClass::Instance->Random(0, AnimDebris.size() - 1)])
				{
					if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoords))
					{
						AnimExt::SetAnimOwnerHouseKind(pAnim, pOWner, Victim, pThis->Owner, false);
					}
				}
			}
		}
	}

	return 0x469EBA;
}
#pragma endregion

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

			pFirer->FootClass_ImbueLocomotor(pFoot_T, pBullet->WH->Locomotor.get());
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

//DEFINE_HOOK(0x466D19, BulletClass_Update_AdjustingVelocity, 0x6)
//{
//	R->ECX(R->EBP());
//	return 0;
//}

//DEFINE_HOOK(0x5B271A, BulletClass_ProjectileMotion_Fix2, 0x5)
//{
//	GET_BASE(BulletClass*, pBullet, 0x18);
//	return pBullet->Type->VeryHigh ? 0x5B272D : 0x5B2721;
//}
//
//DEFINE_HOOK(0x5B260B, BulletClass_ProjectileMotion_DescentAngle, 0x7)
//{
//	GET_BASE(BulletClass*, pBullet, 0x18);
//	GET_STACK(int, nData, 0x38);
//
//	return nData <= ((pBullet->Type->VeryHigh ? 6 : 3) << 8)
//		? 0x5B289C : 0x5B2627;
//}
//
//DEFINE_HOOK(0x5B2778, BulletClass_ProjectileMotion_AscentAngle, 0x7)
//{
//	//GET_BASE(BulletClass*, pBullet, 0x18);
//	//R->Stack(0x18, (std::clamp((0x4000 - 0x2000), 0, 0x4000)));
//	R->Stack<WORD>(0x18, 0x2000);
//	return 0x5B277F;
//}
//
//DEFINE_HOOK(0x5B2721, BulletClass_ProjectileMotion_Cruise, 0x5)
//{
//	//(BulletClass*, pBullet, 0x18);
//	GET(int, nLepton, EAX);
//
//	//bool bLockedOnTrajectory = false;
//	//int nCruiseLevel = 5;
//	//if (bLockedOnTrajectory || nLepton >= nCruiseLevel)
//	//	nLepton = nCruiseLevel;
//
//	R->EAX(nLepton >= 5 ? 5 : nLepton);
//	return 0x5B2732;
//}

DEFINE_HOOK(0x466BAF, BulletClass_AI_MissileROTVar, 0x6)
{
	GET(BulletClass*, pThis, EBP);

	const auto nFrame = (Unsorted::CurrentFrame + pThis->Fetch_ID()) % 15;
	const double nMissileROTVar = BulletTypeExt::ExtMap.Find(pThis->Type)
		->MissileROTVar.Get(RulesClass::Instance->MissileROTVar);

	R->EAX(int(std::sin(static_cast<double>(nFrame) *
		0.06666666666666667 *
		6.283185307179586) *
		nMissileROTVar + nMissileROTVar + 1.0) *
		static_cast<double>(pThis->Type->ROT)
	);

	return 0x466C14;
}

DEFINE_HOOK(0x466E9F, BulletClass_AI_MissileSafetyAltitude, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	GET(int, comparator, EAX);
	auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);
	return comparator >= pBulletTypeExt->GetMissileSaveAltitude(RulesClass::Instance)
		? 0x466EAD : 0x466EB6;
}
