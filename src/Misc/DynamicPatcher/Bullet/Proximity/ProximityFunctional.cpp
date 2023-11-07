#include "ProximityFunctional.h"

#include <BulletClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>

#include <Utilities/Helpers.Alex.h>
#include <Misc/Otamaa/Misc/DynamicPatcher/Helpers/Helpers.h>

void ProximityFunctional::Put(BulletClass* pBullet)
{
	if (!pBullet || !pBullet->Type)
		return;

	auto pBulletExt = BulletExtContainer::Instance.Find(pBullet);
	auto pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);

	if (!pBulletExt || !pBulletTypeExt)
		return;

	if (auto pWeapon = pBullet->WeaponType)
	{
		if (auto ext = WeaponTypeExtContainer::Instance.Find(pWeapon))
		{
			int range = ext->AnotherData.ProximityRangeDatas.Range;
			if (ext->AnotherData.ProximityRangeDatas.Random)
				range = ScenarioClass::Instance()->Random.RandomRanged(ext->AnotherData.ProximityRangeDatas.MinRange, ext->AnotherData.ProximityRangeDatas.MaxRange);

			pBulletExt->AnotherData.BulletProximityRange.reset(GameCreate<ProximityRange>(range));
		}
	}

	if (pBulletTypeExt->AnotherData.BulletProximityData.Force)
		pBulletExt->AnotherData.BulletProximity.reset(GameCreate<Proximity>(pBulletTypeExt->AnotherData.BulletProximityData, pBullet->Owner, pBullet->Type->CourseLockDuration));

}

void ProximityFunctional::AI(BulletClass* pBullet)
{

	if (!pBullet || !pBullet->Type)
		return;

	auto pBulletExt = BulletExtContainer::Instance.Find(pBullet);

	if (!pBulletExt)
		return;

	CoordStruct sourcePos = pBullet->Location;
	BulletVelocity& velocity = pBullet->Velocity;

	auto const BulletProxRange = pBulletExt->AnotherData.BulletProximityRange.get();
	auto nBuff = CoordStruct{(int)velocity.X, (int)velocity.Y, (int)velocity.Z};
	sourcePos += nBuff;

	if (BulletProxRange && BulletProxRange->Enable)
	{
		if (sourcePos.DistanceFrom(pBullet->TargetCoords) <= BulletProxRange->Range)
		{
			if (!ProximityFunctional::ManualDetonation(pBullet, sourcePos, true, pBullet->Owner))
				Debug::Log("Proximity Range Failed to Detonate ! \n");
		}
	}

	auto const BulletProx = pBulletExt->AnotherData.BulletProximity.get();

	if (!BulletProx || !BulletProx->Enable)
		return;

	if (!BulletProx->IsSafe())
	{
		auto pBulletOwner = pBullet->Owner;
		size_t cellSpread = (size_t)((BulletProx->Data.Arm.Get() / 256) + 1);

		if (auto pCell = MapClass::Instance->TryGetCellAt(sourcePos))
		{
			if (pCell != BulletProx->pCheckedCell)
			{
				BulletProx->pCheckedCell = pCell;
				CoordStruct cellPos = pCell->GetCoords();
				ValueableVector<TechnoClass*> pTechnoSet;

				const auto AffectTarget = [&BulletProx, pBullet](TechnoClass* pTarget)
				{
					HouseClass* pTargetOwner = pTarget->GetOwningHouse();
					HouseClass* pSourceOwner = pBullet->Owner ? pBullet->Owner->GetOwningHouse() : nullptr;

					if (pTarget && pTargetOwner && pSourceOwner)
					{
						if (pSourceOwner == pTargetOwner)
						{
							return BulletProx->Data.AffectsAllies || BulletProx->Data.AffectsOwner;
						}
						else if (pSourceOwner->IsAlliedWith(pTargetOwner))
						{
							return BulletProx->Data.AffectsAllies.Get();
						}
						else
						{
							return BulletProx->Data.AffectsEnemies.Get();
						}
					}

					return false;
				};

				for (CellSpreadEnumerator it(cellSpread); it; ++it)
				{
					CellStruct cur = pCell->MapCoords;
					CellStruct offset = (*it);
					if (auto pNewCell = MapClass::Instance->TryGetCellAt(cur + offset))
					{
						auto const pContent = pNewCell->GetContent();
						for (auto pObj = pContent; pObj; pObj = pObj->NextObject)
						{
							if (auto pTech = generic_cast<TechnoClass*>(pObj))
							{
								if (!Helpers_DP::IsDeadOrStand(pTech))
								{
									if (pTech != pBulletOwner)
									{
										if (pTech->WhatAmI() != AbstractType::Building || pNewCell == pCell)
										{
											if (!pTechnoSet.Contains(pTech))
												pTechnoSet.push_back(pTech);
										}
									}
								}
							}
						}


						if (auto pJumpjet = pNewCell->Jumpjet)
							if (!Helpers_DP::IsDeadOrStand(pJumpjet))
								if (pJumpjet != pBulletOwner)
									if (!pTechnoSet.Contains(pJumpjet))
										pTechnoSet.push_back(pJumpjet);
					}
				}

				for (auto const& pTechnArr : *TechnoClass::Array)
				{
					if (!Helpers_DP::IsDeadOrStand(pTechnArr))
					{
						if (pTechnArr != pBulletOwner)
						{
							if (pTechnArr->GetHeight() > 0)
							{
								auto nCoord = pTechnArr->GetCoords();
								nCoord.Z = cellPos.Z;
								if (nCoord.DistanceFrom(cellPos) <= cellSpread * 256.0)
									if (!pTechnoSet.Contains(pTechnArr))
										pTechnoSet.push_back(pTechnArr);
							}
						}
					}
				}

				if (!pTechnoSet.empty())
				{
					for (auto const& pTechno : pTechnoSet)
					{
						CoordStruct targetPos = pTechno->GetCoords();
						bool hit = false;

						if (auto pBuilding = specific_cast<BuildingClass*>(pTechno))
						{
							int height = pBuilding->Type->Height;
							hit = sourcePos.Z <= (targetPos.Z + height * Unsorted::LevelHeight + BulletProx->Data.ZOffset);

							if (hit && BulletProx->Data.PenetrationBuildingOnce)
								hit = !BulletProx->CheckAndMarkBuilding(pBuilding);
						}
						else
						{
							CoordStruct sourceTestPos = cellPos;
							sourceTestPos.Z = sourcePos.Z;
							CoordStruct targetTestPos = targetPos + CoordStruct {0, 0, BulletProx->Data.ZOffset};
							hit = targetTestPos.DistanceFrom(sourceTestPos) <= BulletProx->Data.Arm;
						}

						if (hit && AffectTarget(pTechno))
						{
							CoordStruct detonatePos = targetPos;
							if (ManualDetonation(pBullet, sourcePos, !BulletProx->Data.Penetration, pBulletOwner, pTechno, detonatePos))
							{
								break;
							}
							else
							{
								Debug::Log("Proximity Failed to Detonate ! \n");
							}
						}
					}
				}
			}
		}
	}
}

bool ProximityFunctional::ManualDetonation(BulletClass* pBullet, CoordStruct sourcePos, bool KABOOM, TechnoClass* pBulletOwner, AbstractClass* pTarget, CoordStruct detonatePos)
{
	if (!pBullet)
		return false;

	auto pBulletExt = BulletExtContainer::Instance.Find(pBullet);

	if (!pBulletExt)
		return false;

	if (!KABOOM && Helpers_DP::IsDead(pBulletOwner))
		KABOOM = true;

	auto const BulletProx = pBulletExt->AnotherData.BulletProximity.get();

	KABOOM = KABOOM || (BulletProx && BulletProx->Explodes());

	if (KABOOM)
	{
		pBullet->Detonate(sourcePos);
		pBullet->Limbo();
		//GameDelete<true,false>(pBullet);
		pBullet->UnInit();
		return true;
	}
	else if (pTarget)
	{
		if (detonatePos == CoordStruct::Empty)
			detonatePos = sourcePos;

		int damage = pBullet->Health;
		auto pWH = pBullet->WH;

		if (!pWH)
			return false;

		auto pPenetrateWeapon = BulletProx->Data.PenetrationWeapon.Get();
		if (pBulletOwner && pPenetrateWeapon)
		{
			damage = pPenetrateWeapon->Damage;
			pWH = pPenetrateWeapon->Warhead;

		}

		if (!pWH)
			return false;

		if (auto pPenetrateWH = BulletProx->Data.PenetrationWarhead.Get())
			pWH = pPenetrateWH;

		MapClass::DamageArea(detonatePos, damage, pBulletOwner, pWH, pWH->Tiberium, pBulletOwner->GetOwningHouse());
		LandType landType = BulletProx->pCheckedCell ? LandType::Clear : BulletProx->pCheckedCell->LandType;

		if (auto pAnimType = MapClass::SelectDamageAnimation(damage, pWH, landType, sourcePos))
			if (auto pAnim = GameCreate<AnimClass>(pAnimType, sourcePos))
				pAnim->Owner = pBulletOwner->GetOwningHouse();

		BulletProx->ThroughOnce();
	}

	return KABOOM;
}
