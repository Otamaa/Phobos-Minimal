#include "CustomWeapon.h"


#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

bool InRange(TechnoClass* pShooter, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	auto nLoc = pShooter->GetCoords();
	return pShooter->InRange(nLoc, pTarget, pWeapon);
}

void SimulateBurstManager::Update(TechnoClass* pAttacker)
{
	if (!pAttacker)
		return;

	if (!pAttacker->Target || !TechnoExtData::IsActive(pAttacker))
	{
		Clear();
		return;
	}

	// Process exactly the current queue size to avoid infinite loops
	size_t queueSize = simulateBurstQueue.size();

	for (size_t i = 0; i < queueSize; ++i)
	{
		// O(1) removal from front
		auto burst = std::move(simulateBurstQueue.front());
		simulateBurstQueue.pop_front();

		// Check if burst is complete - don't reinsert if done
		if (burst.Index >= burst.Burst)
			continue;

		if (burst.CanFire())
		{
			TechnoClass* pShooter = burst.Shooter;
			AbstractClass* pTarget = burst.Target;
			WeaponTypeClass* pWeaponType = burst.WeaponType;

			auto const pTargetTech = flag_cast_to<TechnoClass*>(pTarget);

			if (pWeaponType
				&& pShooter && pShooter->IsAlive
				&& pTargetTech && !Helpers_DP::IsDeadOrInvisible(pTargetTech)
				&& (!burst.FireData.CheckRange || InRange(pShooter, pTarget, burst.WeaponType))
				&& (!pAttacker->Transporter || (pWeaponType->FireInTransport || burst.FireData.OnlyFireInTransport)))
			{
				SimulateBurstFire(pShooter, pAttacker, pTarget, pWeaponType, &burst);
			}
		}

		// O(1) insertion at back - only if not complete
		simulateBurstQueue.push_back(std::move(burst));
	}
}

bool SimulateBurstManager::FireCustomWeapon(TechnoClass* pShooter,
	TechnoClass* pAttacker,
	AbstractClass* pTarget,
	WeaponTypeClass* pWeapon,
	const CoordStruct& flh,
	const CoordStruct& bulletSourcePos, double rofMult)
{
	bool isFire = false;
	pShooter = WhoIsShooter(pShooter);

	if (pShooter && pWeapon && (!pAttacker->Transporter || pWeapon->FireInTransport))
	{
		const auto typeExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

		const AttachFireData& fireData = typeExt->MyAttachFireDatas;
		CoordStruct fireFLH = flh;

		if (TechnoClass* pTransporter = pAttacker->Transporter)
		{
			if (fireData.UseAlternateFLH)
			{
				auto nIdx = pTransporter->Passengers.IndexOf((FootClass*)pAttacker);
				nIdx = nIdx >= TechnoTypeClass::MaxWeapons ? TechnoTypeClass::MaxWeapons - 1 : nIdx;

				fireFLH = pTransporter->GetTechnoType()->Weapon[nIdx].FLH;
			}
		}
		else if (fireData.OnlyFireInTransport)
		{
			return isFire;
		}

		int burst = pWeapon->Burst;
		int minRange = pWeapon->MinimumRange;
		int range = pWeapon->Range;
		if (pTarget->IsInAir())
		{
			range += pShooter->GetTechnoType()->AirRangeBonus;
		}

		if (burst > 1 && fireData.SimulateBurst)
		{

			int flipY = 1;

			//if (BulletTypeClass* pBulletType = pWeapon->Projectile)
			//{
				//auto bulletTypeExt = BulletTypeExtContainer::Instance.Find(pBulletType);
				//if (bulletTypeExt && bulletTypeExt->MissileData.ReverseVelocity)
				//	flipY = -1;

			//}

			SimulateBurst newBurst (pWeapon, pShooter, pTarget, fireFLH, burst, minRange, range, fireData, flipY);
			SimulateBurstFire(pShooter, pAttacker, pTarget, pWeapon, &newBurst);
			simulateBurstQueue.push_back(std::move(newBurst));
			isFire = true;
		}
		else
		{
			if (!fireData.CheckRange || InRange(pShooter, pTarget, pWeapon))
			{
				Helpers_DP::FireWeaponTo(pShooter, pAttacker, pTarget, pWeapon, fireFLH, bulletSourcePos, fireData.RadialFire, fireData.RadialAngle);
				isFire = true;
			}
		}
	}
	return isFire;
}

void SimulateBurstManager::SimulateBurstFire(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, SimulateBurst* burst)
{
	if (burst->FireData.SimulateBurstMode == 3)
	{
		SimulateBurst* b2 = burst;
		b2->FlipY *= -1;
		SimulateBurstFireOnce(pShooter, pAttacker, pTarget, pWeapon, b2);
	}

	SimulateBurstFireOnce(pShooter, pAttacker, pTarget, pWeapon, burst);

}

void GetBulletVelocity(VelocityClass& nVel, CoordStruct& sourcePos, CoordStruct& targetPos, TechnoClass* pShooter, int burst, int radial, bool radialFire, int idx)
{
	if (radialFire)
	{
		RadialFireHelper radialFireHelper { pShooter, burst, radial };
		nVel = radialFireHelper.GetBulletVelocity(idx);
	}
	else
	{
		nVel = Helpers_DP::GetBulletVelocity(sourcePos, targetPos);
	}
}

void SimulateBurstManager::SimulateBurstFireOnce(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, SimulateBurst* burst)
{
	// Pointer<TechnoClass> pShooter = WhoIsShooter(pShooter);
	CoordStruct sourcePos = Helpers_DP::GetFLHAbsoluteCoords(pShooter, burst->FLH, true, burst->FlipY);
	CoordStruct targetPos = pTarget->GetCoords();
	VelocityClass bulletVelocity {};
	GetBulletVelocity(bulletVelocity, sourcePos, targetPos, pShooter, burst->Burst, burst->FireData.RadialAngle, burst->FireData.RadialFire, burst->Index);
	Helpers_DP::FireBulletTo(pAttacker, pTarget, pWeapon, sourcePos, targetPos, bulletVelocity);
	burst->CountOne();
}

TechnoClass* SimulateBurstManager::WhoIsShooter(TechnoClass* pAttacker) const
{
	return pAttacker->Transporter ? pAttacker->Transporter : pAttacker;
}

void SimulateBurstManager::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	if(bRemoved) {

		auto it = simulateBurstQueue.begin();
		while (it != simulateBurstQueue.end())
		{
			if ( ((it)->Target == ptr || (it)->Shooter == ptr))
				it = simulateBurstQueue.erase(it);
			else
				++it;
		}
	}
}

bool SimulateBurstManager::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(simulateBurstQueue, RegisterForChange)
		.Success();
}

bool SimulateBurstManager::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(simulateBurstQueue)
		.Success();
}

void DelayFireManager::Clear()
{
	DelayFires.clear();
	//DelayFires.reserve(100);
}

void DelayFireManager::Insert(int weaponIndex, AbstractClass* pTarget, int delay, int count)
{
	DelayFires.emplace_back(weaponIndex, pTarget, delay, count);
}

void DelayFireManager::Insert(WeaponTypeClass* pWeapon, AbstractClass* pTarget, int delay, int count)
{
	DelayFires.emplace_back(pWeapon, pTarget, delay, count);
}

bool DelayFireManager::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Stm
		.Process(DelayFires, RegisterForChange)
		.Success();
}

bool DelayFireManager::Save(PhobosStreamWriter& Stm) const
{
	return Stm
		.Process(DelayFires)
		.Success();
}

void DelayFireManager::Update(TechnoClass* pAttacker)
{
	size_t queueSize = this->DelayFires.size();

	for (size_t i = 0; i < queueSize; ++i)
	{
		// O(1) removal from front
		auto delayFire = std::move(this->DelayFires.front());
		this->DelayFires.pop_front();

		if (delayFire.TimesUp())
		{
			if (delayFire.FireOwnWeapon)
				pAttacker->Fire(delayFire.Target, delayFire.WeaponIndex);
			else
				Helpers_DP::FireWeaponTo(pAttacker, pAttacker, delayFire.Target,
					delayFire.Weapon, CoordStruct::Empty);

			delayFire.ReduceOnce();
		}

		// Only reinsert if not complete
		if (delayFire.NotDone()) {
			// O(1) insertion at back
			this->DelayFires.push_back(std::move(delayFire));
		}
	}
}

void DelayFireManager::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	if(bRemoved){
		auto it = DelayFires.begin();
		while (it != DelayFires.end())
		{
			if ((it)->Target == ptr)
				it = DelayFires.erase(it);
			else
				++it;
		}
	}
}

void DelayFireManager::TechnoClass_Update_CustomWeapon(TechnoClass* pAttacker)
{
	auto pExt = TechnoExtContainer::Instance.Find(pAttacker);

	if(auto delayedfire = pExt->Get_DelayFireManager()){
		delayedfire->Update(pAttacker);

		if (delayedfire->DelayFires.empty()) {
			Phobos::gEntt->remove<DelayFireManager>(pExt->MyEntity);
		}
	}

	if (auto pSimulator = pExt->Get_SimulateBurstManager()) {
		pSimulator->Update(pAttacker);
		if (pSimulator->simulateBurstQueue.empty()) {
			Phobos::gEntt->remove<SimulateBurst>(pExt->MyEntity);
		}
	}
}
