#ifdef COMPILE_PORTED_DP_FEATURES
#include "CustomWeapon.h"


#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

bool InRange(TechnoClass* pShooter, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	auto nLoc = pShooter->GetCoords();
	return pShooter->InRange(nLoc,pTarget, pWeapon);
}

void CustomWeaponManager::Update(TechnoClass* pAttacker)
{
	if (!pAttacker
		|| !pAttacker->Target
		|| !TechnoExt::IsActive(pAttacker)) {
		Clear();
	}
	else {

		for(; !simulateBurstQueue.empty();){

			SimulateBurst burst = simulateBurstQueue[0];
			simulateBurstQueue.erase(simulateBurstQueue.begin());

			if (burst.Index < burst.Burst)
			{
				if (burst.CanFire())
				{
					TechnoClass* pShooter = burst.Shooter;
					AbstractClass* pTarget = burst.Target;
					WeaponTypeClass* pWeaponType = burst.WeaponType;

					auto const pTargetTech = generic_cast<TechnoClass*>(pTarget);

					if (pWeaponType
						&& pShooter && pShooter->IsAlive
						&& pTargetTech && !Helpers_DP::IsDeadOrInvisible(pTargetTech)
						&& (!burst.FireData.CheckRange || InRange(pShooter, pTarget, burst.WeaponType)) &&
						(!pAttacker->Transporter || (pWeaponType->FireInTransport || burst.FireData.OnlyFireInTransport))
					) {
						SimulateBurstFire(pShooter, pAttacker, pTarget, pWeaponType, burst);
					}
				}

				simulateBurstQueue.push_back(burst);
			}
		}
	}
}

bool CustomWeaponManager::FireCustomWeapon(TechnoClass* pShooter,
	TechnoClass* pAttacker,
	AbstractClass* pTarget,
	WeaponTypeClass* pWeapon,
	const CoordStruct& flh,
	const CoordStruct& bulletSourcePos, double rofMult , FireBulletToTarget callback)
{
	bool isFire = false;
	pShooter = WhoIsShooter(pShooter);

	if (pShooter && pWeapon && (!pAttacker->Transporter || pWeapon->FireInTransport))
	{
		if (auto const typeExt = WeaponTypeExt::ExtMap.Find(pWeapon))
		{
			AttachFireData& fireData = typeExt->MyAttachFireDatas;
			CoordStruct fireFLH = flh;

			if (TechnoClass* pTransporter = pAttacker->Transporter)
			{
				if (fireData.UseAlternateFLH)
				{
					fireFLH = pTransporter->GetTechnoType()->Weapon[pTransporter->Passengers.IndexOf((FootClass*)pAttacker)].FLH;
				}
			}
			else if (fireData.OnlyFireInTransport)
			{
				return isFire;
			}

			int burst = pWeapon->Burst;
			int minRange = pWeapon->MinimumRange;
			int range = pWeapon->Range;
			if (pTarget->IsInAir()) {
				range += pShooter->GetTechnoType()->AirRangeBonus;
			}

			if (burst > 1 && fireData.SimulateBurst)
			{

				int flipY = 1;

				//if (BulletTypeClass* pBulletType = pWeapon->Projectile)
				//{
					//auto bulletTypeExt = BulletTypeExt::ExtMap.Find(pBulletType);
					//if (bulletTypeExt && bulletTypeExt->MissileData.ReverseVelocity)
					//	flipY = -1;

				//}

				SimulateBurst newBurst = SimulateBurst(pWeapon, pShooter, pTarget, fireFLH, burst, minRange, range, fireData, flipY , callback);
				SimulateBurstFire(pShooter, pAttacker, pTarget, pWeapon, newBurst);
				simulateBurstQueue.push_back(std::move(newBurst));
				isFire = true;
			}
			else
			{
				if (!fireData.CheckRange || InRange(pShooter, pTarget, pWeapon))
				{
					Helpers_DP::FireWeaponTo(pShooter, pAttacker, pTarget, pWeapon, fireFLH, callback, bulletSourcePos, fireData.RadialFire, fireData.RadialAngle);
					isFire = true;
				}
			}
		}
		else
		{
			Helpers_DP::FireWeaponTo(pShooter, pAttacker, pTarget, pWeapon, flh, callback, bulletSourcePos);
			isFire = true;
		}

	}
	return isFire;
}

void CustomWeaponManager::SimulateBurstFire(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, SimulateBurst& burst)
{
	if (burst.FireData.SimulateBurstMode == 3)
	{
		SimulateBurst b2 = burst.Clone();
		b2.FlipY *= -1;
		SimulateBurstFireOnce(pShooter, pAttacker, pTarget, pWeapon, b2);
	}
	
	SimulateBurstFireOnce(pShooter, pAttacker, pTarget, pWeapon, burst);

}

void CustomWeaponManager::SimulateBurstFireOnce(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, SimulateBurst& burst)
{
	// Pointer<TechnoClass> pShooter = WhoIsShooter(pShooter);
	CoordStruct sourcePos = Helpers_DP::GetFLHAbsoluteCoords(pShooter, burst.FLH, true, burst.FlipY);
	CoordStruct targetPos = pTarget->GetCoords();
	VelocityClass bulletVelocity {};

	if (burst.FireData.RadialFire)
	{
		RadialFireHelper radialFireHelper { pShooter, burst.Burst, burst.FireData.RadialAngle };
		bulletVelocity = radialFireHelper.GetBulletVelocity(burst.Index);
	} else {
		bulletVelocity = Helpers_DP::GetBulletVelocity(sourcePos, targetPos);
	}

	auto pBullet = Helpers_DP::FireBulletTo(pAttacker, pTarget, pWeapon, sourcePos, targetPos, bulletVelocity);

	if (burst.Callback && pBullet) {
		burst.Callback(burst.Index, burst.Burst, pBullet, pTarget);
	}

	burst.CountOne();
}

TechnoClass* CustomWeaponManager::WhoIsShooter(TechnoClass* pAttacker) const
{
	return pAttacker->Transporter ? pAttacker->Transporter: pAttacker;
}

void  CustomWeaponManager::InvalidatePointer(void* ptr, bool bRemoved)
{
	for (size_t pos = 0; pos < simulateBurstQueue.size(); pos++) {
		auto const& sBData = simulateBurstQueue[pos];
		if (sBData.Target == ptr || sBData.Shooter == ptr)
		{
			simulateBurstQueue.erase(simulateBurstQueue.begin() + pos); //because it queue , we need to remove it instead
		}
	}
}

void FireWeaponManager::Clear()
{
	DelayFires.clear();
}

void FireWeaponManager::FireWeaponManager_Clear()
{
	Clear();
	CWeaponManager.release();
}

void FireWeaponManager::Insert(int weaponIndex, AbstractClass* pTarget, int delay, int count)
{
	DelayFires.emplace_back(std::move(std::make_unique<DelayFireWeapon>(weaponIndex, pTarget, delay, count)));
}

void FireWeaponManager::Insert(WeaponTypeClass* pWeapon, AbstractClass* pTarget, int delay, int count)
{
	DelayFires.emplace_back(std::move(std::make_unique<DelayFireWeapon>(pWeapon, pTarget, delay, count)));
}

bool FireWeaponManager::FireCustomWeapon(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, const CoordStruct& flh, const CoordStruct& bulletSourcePos, double rofMult , FireBulletToTarget callback)
{
	return CWeaponManager->FireCustomWeapon(pShooter, pAttacker, pTarget, pWeapon, flh, bulletSourcePos, rofMult, callback);
}

void FireWeaponManager::InvalidatePointer(void* ptr, bool bRemoved)
{
	for (size_t pos = 0; pos < DelayFires.size(); pos++)
	{
		if (DelayFires[pos]->Target == ptr)
		{
				//Debug::Log("Found Invalid Target from FireWeaponManager ! , Cleaning Up ! \n");
				DelayFires.erase(DelayFires.begin() + pos); //because it queue , we need to remove it instead
		}
	}

	CWeaponManager->InvalidatePointer(ptr, bRemoved);
}

void FireWeaponManager::TechnoClass_Update_CustomWeapon(TechnoClass* pAttacker)
{
	for (; !DelayFires.empty();)
	{
		auto delayFire = std::move(DelayFires[0]);
		DelayFires.erase(DelayFires.begin());

		if (delayFire->TimesUp())
		{
			if (delayFire->FireOwnWeapon)
			{
				pAttacker->Fire(delayFire->Target, delayFire->WeaponIndex);
			}
			else
			{
				Helpers_DP::FireWeaponTo(pAttacker, pAttacker, delayFire->Target, delayFire->Weapon, CoordStruct::Empty);
			}
			delayFire->ReduceOnce();
		}

		if (delayFire->NotDone()) {
			DelayFires.emplace_back(std::move(delayFire));
		}
	}

	CWeaponManager->Update(pAttacker);
}
#endif