#include "Body.h"

/*
; Bullet\Hooks.Bounce.cpp
*/

namespace Utils
{
	void DetonateBullet(WeaponTypeClass* pWeapon, BulletClass* pFrom, CoordStruct const& nCoord)
	{
		CoordStruct coords = nCoord;
		auto payback = pFrom->Owner;

		if (!generic_cast<TechnoClass*>(pFrom->Owner)) {
			payback = nullptr;
		}

		if (auto v9 = pWeapon->Projectile->CreateBullet(pFrom->GetCell(), payback, pWeapon->Damage, pWeapon->Warhead, 0, pWeapon->Bright))
		{
			v9->WeaponType = pWeapon;
			if (pWeapon->Projectile->ShrapnelWeapon) {
				v9->SetLocation(coords);
			}

			v9->Limbo();
			v9->Detonate(coords);
			v9->UnInit();
		}
	}
}

DEFINE_HOOK(0x467BDB, BulletClass_Update_BounceOnSomething, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x1AC, 0x164));
	GET(bool, bForceDetonate, EBX);

	auto pExt = BulletExtContainer::Instance.Find(pThis);
	auto pTypeExt = pExt->TypeExt;

	if (pTypeExt->BounceAmount)
	{
		//auto pOwner = pThis->Owner ? pThis->Owner->Owner : pExt->Owner;

		auto pCell = Map.GetCellAt(nCoord);
		bool bAlt = (static_cast<unsigned int>(pCell->Flags) >> 8) & 1;
		//Point2D Offs = { static_cast<short>(nCoord.X / 256),static_cast<short>(nCoord.Y / 256) };
		auto pObj = pCell->GetSomeObject({ nCoord.X,nCoord.Y }, bAlt);
		if (!pObj)
		{
			if (pObj == pExt->LastObject)
			{
				if (!bForceDetonate || (pExt->Bouncing && --pExt->BounceAmount <= 0))
				{
					R->EBX(pExt->Bouncing && pExt->BounceAmount <= 0);
					return 0;
				}
			}

			pExt->LastObject = pObj;
			if ((((DWORD*)pObj)[0]) == BuildingClass::vtable)
			{
				auto pBldCell = pObj->GetCell();
				auto pBldCoord = pBldCell->GetCoords();

				if (pBldCoord.Y < nCoord.Y)
				{
					pThis->Velocity.Y = -pThis->Velocity.Y;
				}
				else
				{
					pThis->Velocity.X = -pThis->Velocity.X;
				}

				if (auto pWeapon = pTypeExt->BounceHitWeapon)
				{
					Utils::DetonateBullet(pWeapon, pThis, nCoord);
				}

				pThis->Velocity *= pThis->Type->Elasticity;

				if (!pExt->Bouncing)
				{
					pExt->BounceAmount = pTypeExt->BounceAmount;
					pExt->Bouncing = true;
					R->EBX(false);
					return 0x467C0C;
				}

				if (pExt->BounceAmount)
				{
					R->EBX(false);
					return 0x467C0C;
				}

				R->EBX(pExt->Bouncing && pExt->BounceAmount <= 0);
				return 0;
			}

			if ((!pTypeExt->BounceOnTerrain && pObj->WhatAmI() == AbstractType::Terrain)
				|| nCoord.DistanceFrom(pObj->Location) >= 128.0
				)
			{
				if (!bForceDetonate || (pExt->Bouncing && --pExt->BounceAmount <= 0))
				{
					R->EBX(pExt->Bouncing && pExt->BounceAmount <= 0);
					return 0;
				}
			}

			if (pObj->Location.Y < nCoord.Y)
			{
				pThis->Velocity.Y = -pThis->Velocity.Y;
			}
			else
			{
				pThis->Velocity.X = -pThis->Velocity.X;
			}

			if (auto pWeapon = pTypeExt->BounceHitWeapon)
			{
				Utils::DetonateBullet(pWeapon, pThis, nCoord);
			}

			pThis->Velocity = (pThis->Velocity * pThis->Type->Elasticity);

			if (!pExt->Bouncing)
			{
				pExt->BounceAmount = pTypeExt->BounceAmount;
				pExt->Bouncing = true;
				R->EBX(false);
				return 0x467C0C;
			}

			if (pExt->BounceAmount)
			{
				R->EBX(false);
				return 4619276;
			}

			R->EBX(pExt->Bouncing && pExt->BounceAmount <= 0);
			return 0;

		}
	}

	return 0;
}

DEFINE_HOOK(0x467609, BulletClass_Update_CheckBounce, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	auto pExt = BulletExtContainer::Instance.Find(pThis);
	return pExt->TypeExt->BounceAmount ? 0x467615 : 0x46777A;
}

DEFINE_HOOK(0x4679CA, BulletClass_Update_CheckDistToObject, 0x5)
{
	R->AL(R->EAX<int>() < 128);
	return 0x4679D7;
}

DEFINE_HOOK(0x46794B, BulletClass_Update_CheckNearbyTechno, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	GET(TechnoClass*, pTarget, ESI);
	GET(TechnoClass*, pFirer, EAX);

	auto pExt = BulletExtContainer::Instance.Find(pThis);

	if (pExt->TypeExt->BounceAmount)
	{
		if (pTarget->WhatAmI() == AbstractType::Infantry)
			R->AL(pExt->TypeExt->BounceOnInfantry);
		else if (pTarget->WhatAmI() == AbstractType::Unit)
			R->AL(pExt->TypeExt->BounceOnVehicle);
		return 0x467957;
	}

	R->AL(pFirer->Owner->IsAlliedWith_(pTarget));
	return 0x467957;
}

DEFINE_HOOK(0x46786C, BulletClass_Update_ContactTarget, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	GET(int, nHeight, EAX);

	auto pExt = BulletExtContainer::Instance.Find(pThis);

	return (pExt->TypeExt->BounceAmount) || nHeight >= 208 ? 0x467890 : 0x467879;
}

DEFINE_HOOK(0x4678DC, BulletClass_Update_CrossingBuilding, 0x7)
{
	GET(BulletClass*, pThis, EBP);
	GET(TechnoClass*, pTarget, ESI);
	GET(TechnoClass*, pExTechno, EAX);

	auto pExt = BulletExtContainer::Instance.Find(pThis);
	auto pTypeExt = pExt->TypeExt;

	if (pTarget == pThis->Owner)
		return 0x4678F8;

	if (!pTypeExt->BounceAmount)
		return pTarget == pExTechno && pThis->GetHeight() < 208 ? 0x467879 : 0x4678F8;
	else
		return pTypeExt->BounceOnBuilding ? 0x4678F8:0x467879;
}

DEFINE_HOOK(0x46779B, BulletClass_Update_DetonateNow, 0x8)
{
	GET(BulletClass*, pThis, EBP);

	auto pExt = BulletExtContainer::Instance.Find(pThis);
	auto pTypeExt = pExt->TypeExt;

	if (!pTypeExt->BounceAmount)
	{
		R->Stack(0x20, true);
		R->Stack(0x18, true);
		return 0x4677A8;
	}

	if (pExt->Bouncing)
	{
		if (--pExt->BounceAmount <= 0)
		{
			R->Stack(0x20, true);
			R->Stack(0x18, true);
			return 0x4677A8;
		}
	}
	else
	{
		pExt->BounceAmount = pTypeExt->BounceAmount;
		pExt->Bouncing = true;
	}

	GET_STACK(int, nX, 0x68);
	GET_STACK(int, nY, 0x70);
	GET_STACK(int, nZ, 0x78);

	CoordStruct nCoord = { nX , nY , nZ };

	auto pCell = Map.GetCellAt(nCoord);
	bool bAlt = (static_cast<unsigned int>(pCell->Flags) >> 8) & 1;
	auto nObj = pCell->GetSomeObject({ nX,nY }, bAlt);

	if (!nObj || nObj != pThis->Target)
	{
		if (auto v23 = pTypeExt->BounceHitWeapon)
		{
			Utils::DetonateBullet(v23, pThis, nCoord);
		}
		return 0x4677A8;
	}

	auto nObjLoc = nObj->Location;
	if (auto pBuilding = specific_cast<BuildingClass*>(nObj))
	{
		auto nBldLoc = pBuilding->GetCoords();
		auto nFoundWidth = pBuilding->Type->GetFoundationWidth();
		auto nFoundHeight = pBuilding->Type->GetFoundationHeight(false);
		nBldLoc.X += nFoundWidth / 2;
		nBldLoc.Y += nFoundHeight / 2;
		nObjLoc = nBldLoc;
	}

	pThis->Detonate(nObjLoc);
	pThis->Limbo();
	pThis->UnInit();
	return 0x467FEE;
}