#include "Body.h"

#include <Misc/Hooks.Otamaa.h>

#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

namespace EMPulseCannonTemp
{
	int weaponIndex = 0;
}

//=================================================================================

// this one setting the building target
// either it is non EMPulse or EMPulse
ASMJIT_PATCH(0x44CCE7, BuildingClass_Mi_Missile_GenericSW, 6)
{
	enum { ProcessEMPulse = 0x44CD18, ReturnFromFunc = 0x44D599 };
	GET(BuildingClass* const, pThis, ESI);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (!pThis->Type->EMPulseCannon)
	{
		// originally, this part was related to chem missiles
		const auto pTarget = pExt->SuperTarget.IsValid()
			? pExt->SuperTarget : pThis->Owner->NukeTarget;

		pThis->Fire(MapClass::Instance->GetCellAt(pTarget), 0);
		pThis->QueueMission(Mission::Guard, false);

		R->EAX(1);
		return ReturnFromFunc;
	}

	if (pExt->SuperTarget.IsValid())
	{
		pThis->Owner->EMPTarget = pExt->SuperTarget;
	}

	// Obtain the weapon used by the EMP weapon
	int weaponIndex = 0;
	const auto pLinked = TechnoExtContainer::Instance.Find(pThis)->LinkedSW;
	auto const pSWExt = SWTypeExtContainer::Instance.Find(pLinked->Type);
	auto pTargetCell = MapClass::Instance->GetCellAt(pThis->Owner->EMPTarget);

	if (pSWExt->EMPulse_WeaponIndex >= 0)
	{
		weaponIndex = pSWExt->EMPulse_WeaponIndex;
	}
	else
	{
		AbstractClass* pTarget = pTargetCell;

		if (const auto pObject = pTargetCell->GetContent())
			pTarget = pObject;

		weaponIndex = pThis->SelectWeapon(pTarget);
	}

	const auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

	// Innacurate random strike Area calculation
	int radius = BulletTypeExtContainer::Instance.Find(pWeapon->Projectile)->EMPulseCannon_InaccurateRadius;
	radius = radius < 0 ? 0 : radius;

	if (radius > 0)
	{
		if (pExt->RandomEMPTarget == CellStruct::Empty)
		{
			// Calculate a new valid random target coordinate
			do
			{
				pExt->RandomEMPTarget.X = (short)ScenarioClass::Instance->Random.RandomRanged(pThis->Owner->EMPTarget.X - radius, pThis->Owner->EMPTarget.X + radius);
				pExt->RandomEMPTarget.Y = (short)ScenarioClass::Instance->Random.RandomRanged(pThis->Owner->EMPTarget.Y - radius, pThis->Owner->EMPTarget.Y + radius);
			}
			while (!MapClass::Instance->IsWithinUsableArea(pExt->RandomEMPTarget, false));
		}

		pThis->Owner->EMPTarget = pExt->RandomEMPTarget; // Value overwrited every frame
	}

	if (pThis->MissionStatus == 3) {

		pExt->RandomEMPTarget = CellStruct::Empty;

		// Restart the super weapon firing process if there is enough ammo set for the current weapon
		if (pThis->Type->Ammo > 0 && pThis->Ammo > 0)
		{
			int ammo = WeaponTypeExtContainer::Instance.Find(pWeapon)->Ammo.Get();
			pThis->Ammo -= ammo;
			pThis->Ammo = pThis->Ammo < 0 ? 0 : pThis->Ammo;

			if (pThis->Ammo >= ammo)
				pThis->MissionStatus = 0;

			if (!pThis->ReloadTimer.InProgress())
				pThis->ReloadTimer.Start(pThis->Type->Reload);

			if (pThis->Ammo == 0 && pThis->Type->EmptyReload >= 0 && pThis->ReloadTimer.GetTimeLeft() > pThis->Type->EmptyReload)
				pThis->ReloadTimer.Start(pThis->Type->EmptyReload);
		}
	}

	return ProcessEMPulse;
}

ASMJIT_PATCH(0x44CEEC, BuildingClass_Mi_Missile_State2_EMPulseSelectWeapon, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	// Obtain the weapon used by the EMP weapon
	int weaponIndex = 0;
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	const auto pLinked = TechnoExtContainer::Instance.Find(pThis)->LinkedSW;
	auto const pSWExt = SWTypeExtContainer::Instance.Find(pLinked->Type);
	const auto celltarget = pExt->SuperTarget.IsValid()
		? pExt->SuperTarget : pThis->Owner->EMPTarget;

	auto pTargetCell = MapClass::Instance->GetCellAt(celltarget);

	if (pSWExt->EMPulse_WeaponIndex >= 0)
	{
		weaponIndex = pSWExt->EMPulse_WeaponIndex;
	}
	else
	{
		AbstractClass* pTarget = pTargetCell;

		if (const auto pObject = pTargetCell->GetContent())
			pTarget = pObject;

		weaponIndex = pThis->SelectWeapon(pTarget);
	}

	EMPulseCannonTemp::weaponIndex = weaponIndex;

	R->EAX(pThis->GetWeapon(EMPulseCannonTemp::weaponIndex));
	return 0x44CEF8;
}

//=================================================================================

ASMJIT_PATCH(0x44CE46, BuildingClass_Mi_Missile_State1_EMPulse_Pulsball, 5)
{
	GET(BuildingClass*, pThis, ESI);

	const auto pSWExt = SWTypeExtContainer::Instance.Find(TechnoExtContainer::Instance.Find(pThis)->LinkedSW->Type);
	const auto delay = pSWExt->EMPulse_PulseDelay;

	// also support no pulse ball
	if (auto pPulseBall = pSWExt->EMPulse_PulseBall)
	{
		CoordStruct flh;
		pThis->GetFLH(&flh, 0, CoordStruct::Empty);
		auto pAnim = GameCreate<AnimClass>(pPulseBall, flh);
		pAnim->Owner = pThis->GetOwningHouse();
		((FakeAnimClass*)pAnim)->_GetExtData()->Invoker = pThis;
	}

	pThis->MissionStatus = 2;
	R->EAX(delay);
	return 0x44CEC2;
}

ASMJIT_PATCH(0x44C9F3, BuildingClass_Mi_Missile_State0_NukePsiWarn, 0x5)
{
	GET(BuildingClass* const, pThis, ESI);
	GET(HouseClass*, pOwner, EBP);
	GET(CellClass*, pCell, EAX);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	AnimClass* PsiWarn =
		BulletClass::CreateDamagingBulletAnim(pOwner,
		pCell,
		nullptr,
		SWTypeExtContainer::Instance.Find(pExt->LinkedSW->Type)->Nuke_PsiWarning
		);

	R->EDI(PsiWarn);
	return 0x44CA74;
}

// Create bullet pointing up to the sky
ASMJIT_PATCH(0x44CA97, BuildingClass_MI_Missile_State2_NukeBulletCreate, 0x6)
{
	enum
	{
		SkipGameCode = 0x44CAF2,
		DeleteBullet = 0x44CC42,
		SetUpNext = 0x44CCA7,
		SetRate = 0x44CCB1
	};

	GET(BuildingClass* const, pThis, ESI);

	auto pTarget = MapClass::Instance->GetCellAt(pThis->Owner->NukeTarget);
	auto pSuper = TechnoExtContainer::Instance.Find(pThis)->LinkedSW;

	if (WeaponTypeClass* pWeapon = pSuper->Type->WeaponType)
	{
		//speed harcoded to 255
		if (auto pCreated = pWeapon->Projectile->CreateBullet(pTarget, pThis, pWeapon->Damage, pWeapon->Warhead, 255, pWeapon->Bright || pWeapon->Warhead->Bright))
		{
			BulletExtContainer::Instance.Find(pCreated)->NukeSW = pSuper->Type;
			pCreated->Range = WeaponTypeExtContainer::Instance.Find(pWeapon)->GetProjectileRange();
			pCreated->SetWeaponType(pWeapon);

			if (pThis->PsiWarnAnim)
			{
				pThis->PsiWarnAnim->SetBullet(pCreated);
				pThis->PsiWarnAnim = nullptr;
			}

			//Limbo-in the bullet will remove the `TechnoClass` owner from the bullet !
			//pThis->Limbo();

			CoordStruct nFLH;
			pThis->GetFLH(&nFLH, 0, CoordStruct::Empty);

			// Otamaa : the original calculation seems causing missile to be invisible
			//auto nCos = 0.00004793836;
			//auto nCos = Math::cos(1.570748388432313); // Accuracy is different from the game
			//auto nSin = 0.99999999885;
			//auto nSin = Math::sin(1.570748388432313);// Accuracy is different from the game

			const auto nMult = pCreated->Type->Vertical ? 10.0 : 100.0;
			//const auto nX = nCos * nCos * nMult;
			//const auto nY = nCos * nSin * nMult;
			//const auto nZ = nSin * nMult;

			if (!pCreated->MoveTo(nFLH, { 0.0, 0.0 , nMult }))
				return DeleteBullet;

			if (auto const pAnimType = SWTypeExtContainer::Instance.Find(pSuper->Type)->Nuke_TakeOff.Get(RulesClass::Instance->NukeTakeOff))
			{
				auto pAnim = GameCreate<AnimClass>(pAnimType, nFLH);
				if (!pAnim->ZAdjust)
					pAnim->ZAdjust = -100;

				pAnim->SetHouse(pThis->GetOwningHouse());
				((FakeAnimClass*)pAnim)->_GetExtData()->Invoker = pThis;
			}

			return SetUpNext;
		}
	}

	return SetRate;

}

ASMJIT_PATCH(0x44D46E, BuildingClass_Mi_Missile_State2_EMPPulseBulletWeapon, 0x8)
{
	GET(BuildingClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBP);
	GET(FakeBulletClass*, pBullet, EDI);
	pBullet->SetWeaponType(pWeapon);

	if (pBullet->Type->Arcing && !pBullet->_GetTypeExtData()->Arcing_AllowElevationInaccuracy)
	{
		REF_STACK(VelocityClass, velocity, STACK_OFFSET(0xE8, -0xD0));
		REF_STACK(CoordStruct, crdSrc, STACK_OFFSET(0xE8, -0x8C));
		GET_STACK(CoordStruct, crdTgt, STACK_OFFSET(0xE8, -0x4C));

		pBullet->_GetExtData()->ApplyArcingFix(crdSrc, crdTgt, velocity);
	}

	//if (pWeapon) {
	BulletExtData::SimulatedFiringEffects(pBullet, pThis->Owner, pThis, true, true);
	//}

	return 0;
}