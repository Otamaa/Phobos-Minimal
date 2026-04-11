#include "Body.h"

#include <BulletClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/SWType/NewSuperWeaponType/LightningStorm.h>
#include <Ext/Techno/Body.h>

#include <Locomotor/JumpjetLocomotionClass.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Helpers.h>
#include <Utilities/Macro.h>

#include <VeinholeMonsterClass.h>
#include <Misc/PhobosGlobal.h>

#include <TacticalClass.h>

#pragma region DETONATION
void ApplyExtraWarheads(
	BulletClass* pBullet,
	std::vector<WarheadTypeClass*>& exWH,
	std::vector<int>& exWHDamageOverrides,
	std::vector<double>& exWHChances,
	std::vector<bool>& exWHFull,
	CoordStruct* coords, HouseClass* pOwner) {

	const size_t damageoverride_size = exWHDamageOverrides.size();
	const size_t fulldetonation_size = exWHFull.size();
	const size_t chance_size = exWHChances.size();
	int damage = pBullet->WeaponType ? pBullet->WeaponType->Damage : 0;

	for (size_t i = 0; i < exWH.size(); i++)
	{
		auto const pWH = exWH[i];

		if (damageoverride_size > 0 && damageoverride_size > i)
			damage = exWHDamageOverrides[i];
		else if (damageoverride_size > 0)
			damage = exWHDamageOverrides[damageoverride_size - 1];

		bool detonate = true;

		if (chance_size > 0 && chance_size > i)
			detonate = exWHChances[i] >= ScenarioClass::Instance->Random.RandomDouble();
		else if (chance_size > 0)
			detonate = exWHChances[chance_size - 1] >= ScenarioClass::Instance->Random.RandomDouble();

		bool isFull = true;

		if (fulldetonation_size > 0 && fulldetonation_size > i)
			isFull = exWHFull[i];
		else if (fulldetonation_size > 0)
			isFull = exWHFull[fulldetonation_size - 1];

		if (detonate) {

			if(isFull)
				WarheadTypeExtData::DetonateAt(pWH, pBullet->Target ? pBullet->Target : MapClass::Instance->GetCellAt(coords), *coords, pBullet->Owner, damage , pOwner);
			else
				WarheadTypeExtContainer::Instance.Find(pWH)->DamageAreaWithTarget(*coords, damage, pBullet->Owner, pWH, true, pOwner,
				flag_cast_to<TechnoClass*>(pBullet->Target));
		}
	}
}

void ApplyLogics(WarheadTypeClass* pWH , WeaponTypeClass*pWeapon ,BulletClass * pThis , CoordStruct* coords) {
	auto const pBulletExt = BulletExtContainer::Instance.Find(pThis);
	auto const pOwner = pThis->Owner ? pThis->Owner->Owner : pBulletExt->Owner;
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if(pThis->WeaponType){
		auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pThis->WeaponType);
		ApplyExtraWarheads(pThis , pWeaponExt->ExtraWarheads, pWeaponExt->ExtraWarheads_DamageOverrides, pWeaponExt->ExtraWarheads_DetonationChances, pWeaponExt->ExtraWarheads_FullDetonation, coords, pOwner);
	}

		// Return to sender
	if (pThis->Type && pThis->Owner)
	{
		auto const pTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Type);
		if (pThis->Owner) {
			auto const pExt = TechnoExtContainer::Instance.Find(pThis->Owner);

			if (pExt->AE.flags.HasExtraWarheads) {
				for (auto const& aE : pExt->AeData.Data) {
					if (aE.Type->ExtraWarheads.size() > 0)
						ApplyExtraWarheads(pThis , aE.Type->ExtraWarheads, aE.Type->ExtraWarheads_DamageOverrides, aE.Type->ExtraWarheads_DetonationChances, aE.Type->ExtraWarheads_FullDetonation, coords, pOwner);
				}

				for (auto const& pAE : pExt->PhobosAE) {

					if(!pAE || !pAE->IsActive())
						continue;

					auto const pType = pAE->GetType();

					if (pType->ExtraWarheads.size() > 0)
						ApplyExtraWarheads(pThis , pType->ExtraWarheads, pType->ExtraWarheads_DamageOverrides, pType->ExtraWarheads_DetonationChances, pType->ExtraWarheads_FullDetonation, coords, pOwner);
				}
			}
		}

		if (pTypeExt->ReturnWeapon && pThis->Owner && pThis->Owner->IsAlive)
		{
			auto const RpWeapon = pTypeExt->ReturnWeapon.Get();
			int damage = TechnoExtData::GetDamageMult(pThis->Owner  , RpWeapon->Damage, !pTypeExt->ReturnWeapon_ApplyFirepowerMult);

			if (BulletClass* pBullet = RpWeapon->Projectile->CreateBullet(pThis->Owner, pThis->Owner,
				damage, RpWeapon->Warhead, RpWeapon->Speed, RpWeapon->Bright))
			{
				pBullet->WeaponType = RpWeapon;
				auto const pRBulletExt = BulletExtContainer::Instance.Find(pBullet);
				pRBulletExt->Owner = BulletExtContainer::Instance.Find(pThis)->Owner;

				BulletExtData::SimulatedFiringUnlimbo(pBullet, pThis->Owner->Owner, pWeapon, pThis->Location, false);
				BulletExtData::SimulatedFiringEffects(pBullet, pThis->Owner->Owner, nullptr, false, false);
			}
		}
	}

	PhobosGlobal::Instance()->DetonateDamageArea = true;

	if (pThis->Owner && pThis->Owner->IsAlive && pThis->Owner->InLimbo && !pWH->Parasite && pWHExt->UnlimboDetonate)
	{
		CoordStruct location = *coords;
		const auto pTarget = pThis->Target;
		const bool isInAir = pTarget && pTarget->AbstractFlags & AbstractFlags::Foot ? static_cast<FootClass*>(pTarget)->IsInAir() : false;
		bool success = false;

		if (!pWHExt->UnlimboDetonate_Force)
		{
			const auto pType = GET_TECHNOTYPE(pThis->Owner);
			const auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(location),
									pType->SpeedType, ZoneType::None, pType->MovementZone, false, 1, 1, true,
									false, false, true, CellStruct::Empty, false, false);

			if (const auto pCell = MapClass::Instance->TryGetCellAt(nCell))
			{
				pThis->Owner->OnBridge = pCell->ContainsBridge();
				location = pCell->GetCoordsWithBridge();
			}
			else
			{
				pThis->Owner->OnBridge = false;
			}

			if (isInAir)
				location.Z = coords->Z;

			success = pThis->Owner->Unlimbo(location, pThis->Owner->PrimaryFacing.Current().GetDir());
		}
		else
		{
			if (const auto pCell = MapClass::Instance->TryGetCellAt(location))
				pThis->Owner->OnBridge = pCell->ContainsBridge();
			else
				pThis->Owner->OnBridge = false;

			++Unsorted::ScenarioInit;
			success = pThis->Owner->Unlimbo(location, pThis->Owner->PrimaryFacing.Current().GetDir());
			--Unsorted::ScenarioInit;
		}

		const auto pTechnoExt = TechnoExtContainer::Instance.Find(pThis->Owner);

		if (success)
		{
			if (isInAir)
			{
				pThis->Owner->IsFallingDown = true;
				TechnoExtContainer::Instance.Find(pThis->Owner)->OnParachuted = true;
			}

			if (pWHExt->UnlimboDetonate_KeepTarget
				&& pTarget && pTarget->AbstractFlags & AbstractFlags::Object)
			{
				pThis->Owner->SetTarget(pTarget);
			}
			else
			{
				pThis->Owner->SetTarget(nullptr);
			}

			if (pTechnoExt->IsSelected)
			{
				ScenarioExtData::Instance()->LimboLaunchers.erase(pThis->Owner);
				pThis->Owner->Select();
				pTechnoExt->IsSelected = false;
			}
		}
		else
		{
			if (pTechnoExt->IsSelected)
			{
				ScenarioExtData::Instance()->LimboLaunchers.erase(pThis->Owner);
				pTechnoExt->IsSelected = false;
			}

			pThis->Owner->SetLocation(location);
			pThis->Owner->ReceiveDamage(&pThis->Owner->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pThis->Owner->Owner);
		}
	}
}

ASMJIT_PATCH(0x469AA4, BulletClass_Logics_Extras, 0x5)
{
	GET(BulletClass* , pThis ,ESI);
	GET_BASE(CoordStruct*, coords, 0x8);
	ApplyLogics(pThis->WH , pThis->WeaponType, pThis,  coords);

	return 0;
}

#pragma endregion

DEFINE_FUNCTION_JUMP(LJMP , 0x48A4F0 , WarheadTypeExtData::SelectCombatAnim)

static WarheadTypeClass* LocomotorWarhead;

// Customize Jumpjet properties on warhead
ASMJIT_PATCH(0x4696CE, BulletClass_Detonate_ImbueLocomotor, 0x6)
{
	enum { SkipGameCode = 0x469AA4 };

	GET(BulletClass* const, pBullet, ESI);
	GET(FootClass* const, pTarget, EDI);
	const auto pWH = pBullet->WH;

	LocomotorWarhead = pWH;
	pBullet->Owner->ImbueLocomotor(pTarget, pWH->Locomotor);
	LocomotorWarhead = nullptr;
	return SkipGameCode;
}


ASMJIT_PATCH(0x54AD41, JumpjetLocomotionClass_Link_To_Object_LocomotorWarhead, 0x8)
{
	enum { SkipGameCode = 0x54ADF8 };

	GET(ILocomotion*, pThis, EBP);
	GET(FootClass*, pLinkedTo, EBX);
	const auto pLoco = static_cast<JumpjetLocomotionClass*>(pThis);
	const auto pLinkedToExt = TechnoExtContainer::Instance.Find(pLinkedTo);
	const auto pType = pLinkedTo->GetTechnoType();

	if (const auto pLocomotorWarhead = LocomotorWarhead)
	{
		const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pLocomotorWarhead);
		pLoco->TurnRate = pWHExt->JumpjetTurnRate.Get(pType->JumpJetData.TurnRate);
		pLoco->Speed = pLinkedToExt->JumpjetSpeed = pWHExt->JumpjetSpeed.Get(pType->JumpJetData.Speed);
		pLoco->Climb = pWHExt->JumpjetClimb.Get(pType->JumpJetData.Climb);
		pLoco->Crash = pWHExt->JumpjetCrash.Get(pType->JumpJetData.Crash);
		pLoco->Height = std::max(pWHExt->JumpjetHeight.Get(pType->JumpJetData.Height), Unsorted::CellHeight);
		pLoco->Acceleration = pWHExt->JumpjetAccel.Get(pType->JumpJetData.Accel);
		pLoco->Wobbles = pWHExt->JumpjetWobbles.Get(pType->JumpJetData.Wobbles);
		pLoco->Deviation = pWHExt->JumpjetDeviation.Get(pType->JumpJetData.Deviation);
		pLoco->NoWobbles = pWHExt->JumpjetNoWobbles.Get(pType->JumpJetData.NoWobbles);
	}
	else
	{
		pLoco->TurnRate = pType->JumpJetData.TurnRate;
		pLoco->Speed = pLinkedToExt->JumpjetSpeed = pType->JumpJetData.Speed;
		pLoco->Climb = pType->JumpJetData.Climb;
		pLoco->Crash = pType->JumpJetData.Crash;
		pLoco->Height = std::max(pType->JumpJetData.Height, Unsorted::CellHeight);
		pLoco->Acceleration = pType->JumpJetData.Accel;
		pLoco->Wobbles = pType->JumpJetData.Wobbles;
		pLoco->Deviation = pType->JumpJetData.Deviation;
		pLoco->NoWobbles = pType->JumpJetData.NoWobbles;
	}

	return SkipGameCode;
}