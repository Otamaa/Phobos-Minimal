#include "BounceTrajectory.h"

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>

#include <WeaponTypeClass.h>

namespace Utils
{
	void DetonateBullet(WeaponTypeClass* pWeapon, BulletClass* pFrom, CoordStruct const& nCoord)
	{
		CoordStruct coords = nCoord;
		auto payback = pFrom->Owner;

		if (!flag_cast_to<TechnoClass*>(pFrom->Owner))
		{
			payback = nullptr;
		}

		if (auto v9 = pWeapon->Projectile->CreateBullet(pFrom->GetCell(), payback, pWeapon->Damage, pWeapon->Warhead, 0, pWeapon->Bright))
		{
			v9->WeaponType = pWeapon;
			if (pWeapon->Projectile->ShrapnelWeapon)
			{
				v9->SetLocation(coords);
			}

			v9->Limbo();
			v9->Detonate(coords);
			v9->UnInit();
		}
	}
}


bool BounceTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectoryType::Load(Stm, RegisterForChange) &&
	Stm.Process(this->BounceAmount, false)
	   .Process(this->BounceWeapon, true);
}

bool BounceTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectoryType::Save(Stm) &&
	Stm.Process(this->BounceAmount, false)
		.Process(this->BounceWeapon, true);
}

bool BounceTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!this->PhobosTrajectoryType::Read(pINI, pSection))
		return false;

	INI_EX exINI { pINI };

	Valueable<int> nBounceAmount { 0 };
	nBounceAmount.Read(exINI, pSection, "Trajectory.Bounce.Amount");

	this->BounceAmount = Math::abs(nBounceAmount.Get());
	this->BounceWeapon.Read(exINI, pSection, "Trajectory.Bounce.Weapon");
	return true;
}

bool BounceTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return PhobosTrajectory::Load(Stm, RegisterForChange) &&
	Stm
		.Process(this->IsBouncing, false)
		.Process(this->BounceLeft, false)
		;
}

bool BounceTrajectory::Save(PhobosStreamWriter& Stm) const
{
	return PhobosTrajectory::Save(Stm) &&
	Stm
		.Process(this->IsBouncing, false)
		.Process(this->BounceLeft, false)
		;
}

// Do some math here to set the initial speed of your proj
// Also set some needed properties here
void BounceTrajectory::OnUnlimbo(CoordStruct* pCoord, VelocityClass* pVelocity)
{
	auto const pType = this->GetTrajectoryType();
	auto const pBullet = this->AttachedTo;
	this->SetInaccurate();
	this->DetonationDistance = pType->DetonationDistance.Get(Leptons(102));
	this->BounceLeft = pType->BounceAmount;

	//pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	//pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	//pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);
	pBullet->Velocity *= (this->GetTrajectorySpeed() / pBullet->Velocity.Length());
}

// Some early checks on each game frame here.
// Return true to detonate the bullet immediately afterwards.
bool BounceTrajectory::OnAI() { return false; }

void BounceTrajectory::OnAIPreDetonate() { }

// Where you update the speed and position
// pSpeed: The speed of this proj in the next frame
// pPosition: Current position of the proj, and in the next frame it will be *pSpeed + *pPosition
void BounceTrajectory::OnAIVelocity(VelocityClass* pSpeed, VelocityClass* pPosition) { }

// Where additional checks based on bullet reaching its target coordinate can be done.
// Vanilla code will do additional checks regarding buildings on target coordinate and Vertical projectiles and will detonate the projectile if they pass.
// Return value determines what is done regards to the game checks: they can be skipped, executed as normal or treated as if the condition is already satisfied.
TrajectoryCheckReturnType BounceTrajectory::OnAITargetCoordCheck(CoordStruct& coords)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

// Where additional checks based on a TechnoClass instance in same cell as the bullet can be done.
// Vanilla code will do additional trajectory alterations here if there is an enemy techno in the cell.
// Return value determines what is done regards to the game checks: they can be skipped, executed as normal or treated as if the condition is already satisfied.
// pTechno: TechnoClass instance in same cell as the bullet.
TrajectoryCheckReturnType BounceTrajectory::OnAITechnoCheck(TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

//DEFINE_HOOK(0x467609, BulletClass_AI_CheckBounce, 0x6)
//{
//	GET(BulletClass*, pThis, EBP);
//
//	const auto pExt = BulletExtContainer::Instance.Find(pThis);
//
//	if (pExt->Trajectory && pExt->Trajectory->Flag == TrajectoryFlag::Bounce)
//	{
//		auto const pBounce = reinterpret_cast<BounceTrajectory*>(pExt->Trajectory);
//		return pBounce->GetTrajectoryType()->BounceAmount ? 0x467615 : 0x46777A;
//	}
//
//	return 0x0;
//}
//
//DEFINE_HOOK(0x46786C, BulletClass_AI_Before_TechnoCheck, 0x6)
//{
//	GET(BulletClass*, pThis, EBP);
//	GET(int, nHeight, EAX);
//	const auto pExt = BulletExtContainer::Instance.Find(pThis);
//
//	if (pExt->Trajectory && pExt->Trajectory->Flag == TrajectoryFlag::Bounce)
//	{
//		auto const pBounce = reinterpret_cast<BounceTrajectory*>(pExt->Trajectory);
//		return pBounce->GetTrajectoryType()->BounceAmount || nHeight >= 208 ? 0x467890 : 0x467879;
//	}
//
//	return nHeight >= 208 ? 0x467890 : 0x467879;
//}

/*
DEFINE_HOOK(0x467BDB, BulletClass_AI_BounceOnSomething, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x1AC, 0x164));
	GET(bool, bForceDetonate, EBX);

	const auto pExt = BulletExtContainer::Instance.Find(pThis);

	if (pExt->Trajectory && pExt->Trajectory->Flag == TrajectoryFlag::Bounce)
	{
		auto pCell = Map.GetCellAt(nCoord);
		bool bAlt = (static_cast<unsigned int>(pCell->Flags) >> 8) & 1;
		auto pObj = pCell->GetSomeObject({ nCoord.X,nCoord.Y }, bAlt);
		const auto pTraj = reinterpret_cast<BounceTrajectory*>(pExt->Trajectory);
		const auto pTrajType = pTraj->GetTrajectoryType();

		if (!pObj)
		{
			if (pObj == pExt->LastObject)
			{
				if (!bForceDetonate || (pTraj->IsBouncing && --pTraj->BounceLeft <= 0))
				{
					R->EBX(pTraj->IsBouncing && pTraj->BounceLeft <= 0);
					return 0;
				}
			}

			pExt->LastObject = pObj;
			if (pObj->WhatAmI() == AbstractType::Building)
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

				if (auto pWeapon = pTrajType->BounceWeapon)
				{
					Utils::DetonateBullet(pWeapon, pThis, nCoord);
				}

				pThis->Velocity *= pThis->Type->Elasticity;

				if (!pTraj->IsBouncing)
				{
					pTraj->BounceLeft = pTrajType->BounceAmount;
					pTraj->IsBouncing = true;
					R->EBX(false);
					return 0x467C0C;
				}

				if (pTraj->BounceLeft)
				{
					R->EBX(false);
					return 0x467C0C;
				}

				R->EBX(pTraj->IsBouncing && pTraj->BounceLeft <= 0);
				return 0;
			}

			if ((!pTypeExt->BounceOnTerrain && pObj->WhatAmI() == AbstractType::Terrain)
				|| nCoord.DistanceFrom(pObj->Location) >= 128.0
				)
			{
				if (!bForceDetonate || (pTraj->IsBouncing && --pTraj->BounceLeft <= 0))
				{
					R->EBX(pTraj->IsBouncing && pTraj->BounceLeft <= 0);
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

			if (auto pWeapon = pTrajType->BounceWeapon)
			{
				Utils::DetonateBullet(pWeapon, pThis, nCoord);
			}

			pThis->Velocity = (pThis->Velocity * pThis->Type->Elasticity);

			if (!pTraj->IsBouncing)
			{
				pTraj->BounceLeft = pTrajType->BounceAmount;
				pTraj->IsBouncing = true;
				R->EBX(false);
				return 0x467C0C;
			}

			if (pTraj->BounceLeft)
			{
				R->EBX(false);
				return 0x467C0C;
			}

			R->EBX(pTraj->IsBouncing && pTraj->BounceLeft <= 0);
			return 0;

		}
	}

	return 0;
}

DEFINE_HOOK(0x46779B, BulletClass_AI_DetonateNow, 0x8)
{
	GET(BulletClass*, pThis, EBP);
	const auto pExt = BulletExtContainer::Instance.Find(pThis);

	if (!pExt->Trajectory || pExt->Trajectory->Flag != TrajectoryFlag::Bounce) {
		R->Stack(0x20, true);
		R->Stack(0x18, true);
		return 0x4677A8;
	}

	const auto pTraj = reinterpret_cast<BounceTrajectory*>(pExt->Trajectory);
	const auto pTrajType = pTraj->GetTrajectoryType();

	if (pTraj->IsBouncing)
	{
		if (--pTraj->BounceLeft <= 0)
		{
			R->Stack(0x20, true);
			R->Stack(0x18, true);
			return 0x4677A8;
		}
	}
	else
	{
		pTraj->BounceLeft = pTrajType->BounceAmount;
		pTraj->IsBouncing = true;
	}

	GET_STACK(int, nX, 0x68);
	GET_STACK(int, nY, 0x70);
	GET_STACK(int, nZ, 0x78);

	const CoordStruct nCoord = { nX , nY , nZ };

	auto pCell = Map.GetCellAt(nCoord);
	bool bAlt = (static_cast<unsigned int>(pCell->Flags) >> 8) & 1;
	auto nObj = pCell->GetSomeObject({ nX,nY }, bAlt);

	if (!nObj || nObj != pThis->Target)
	{
		if (auto v23 = pTrajType->BounceWeapon)
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
}*/