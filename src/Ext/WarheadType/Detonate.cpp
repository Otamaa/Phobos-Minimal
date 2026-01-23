#include "Body.h"

#include <Ext/House/Body.h>
#include <InfantryClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <AnimTypeClass.h>
#include <AnimClass.h>
#include <BitFont.h>
#include <TagTypeClass.h>

#include <Utilities/Helpers.h>
#include <Ext/Anim/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <New/Entity/FlyingStrings.h>

#include <Ext/SWType/NewSuperWeaponType/Reveal.h>

#include <New/Entity/VerticalLaserClass.h>
#include <Ext/Event/Body.h>

#include <Misc/DamageArea.h>

// Wrapper for DamageArea::Apply() that sets a pointer in WarheadTypeExt::ExtData that is used to figure 'intended' target of the Warhead detonation, if set and there's no CellSpread.
DamageAreaResult WarheadTypeExtData::DamageAreaWithTarget(CoordStruct coords, int damage, TechnoClass* pSource, WarheadTypeClass* pWH, bool affectsTiberium, HouseClass* pSourceHouse, TechnoClass* pTarget)
{
	auto const pWarheadTypeExt = WarheadTypeExtContainer::Instance.Find(pWH);
	pWarheadTypeExt->IntendedTarget = pTarget;
	auto result = DamageArea::Apply(&coords, damage, pSource, pWH, true, pSourceHouse);
	pWarheadTypeExt->IntendedTarget = nullptr;
	return result;
}

void WarheadTypeExtData::ApplyLocomotorInfliction(TechnoClass* pTarget) const
{
	auto pTargetFoot = flag_cast_to<FootClass*>(pTarget);
	if (!pTargetFoot)
		return;

	// same locomotor? no point to change
	CLSID targetCLSID { };
	CLSID inflictCLSID = this->This()->Locomotor;
	IPersistPtr pLocoPersist = pTargetFoot->Locomotor;
	if (SUCCEEDED(pLocoPersist->GetClassID(&targetCLSID)) && targetCLSID == inflictCLSID)
		return;

	// prevent endless piggyback
	IPiggybackPtr pTargetPiggy = pTargetFoot->Locomotor;
	if (pTargetPiggy != nullptr && pTargetPiggy->Is_Piggybacking())
		return;

	LocomotionClass::ChangeLocomotorTo(pTargetFoot, inflictCLSID);
}

void WarheadTypeExtData::ApplyLocomotorInflictionReset(TechnoClass* pTarget) const
{
	auto pTargetFoot = flag_cast_to<FootClass*>(pTarget);

	if (!pTargetFoot)
		return;

	// remove only specific inflicted locomotor if specified
	CLSID removeCLSID = this->This()->Locomotor;
	if (removeCLSID != CLSID())
	{
		CLSID targetCLSID { };
		IPersistPtr pLocoPersist = pTargetFoot->Locomotor;
		if (SUCCEEDED(pLocoPersist->GetClassID(&targetCLSID)) && targetCLSID != removeCLSID)
			return;
	}

	// // we don't want to remove non-ok-to-end locos
	// IPiggybackPtr pTargetPiggy = pTargetFoot->Locomotor;
	// if (pTargetPiggy != nullptr && (!pTargetPiggy->Is_Ok_To_End()))
	// 	return;

	LocomotionClass::End_Piggyback(pTargetFoot->Locomotor);
}

void WarheadTypeExtData::ApplyDirectional(BulletClass* pBullet, TechnoClass* pTarget) const
{
	//if (!pBullet || pBullet->IsInAir() != pTarget->IsInAir() || pBullet->GetCell() != pTarget->GetCell() || pTarget->IsIronCurtained())
	//	return;

	//if (pTarget->WhatAmI() != AbstractType::Unit || pBullet->Type->Vertical)
	//	return;

	//const auto pTarExt = TechnoExtContainer::Instance.Find(pTarget);
	//if (!pTarExt || (pTarExt->Shield && pTarExt->Shield->IsActive()))
	//	return;

	//const auto pTarType = pTarget->GetTechnoType();
	//const auto pTarTypeExt = TechnoTypeExtContainer::Instance.Find(pTarType);

	//const int tarFacing = pTarget->PrimaryFacing.Current().GetValue<16>();
	//int bulletFacing = BulletExtContainer::Instance.Find(pBullet)->InitialBulletDir.get().GetValue<16>();

	//const int angle = Math::abs(bulletFacing - tarFacing);
	//auto frontField = 64 * this->DirectionalArmor_FrontField;
	//auto backField = 64 * this->DirectionalArmor_BackField;

	//if (angle >= 128 - frontField && angle <= 128 + frontField)//�����ܻ�
	//	pTarExt->ReceiveDamageMultiplier = this->DirectionalArmor_FrontMultiplier.Get();
	//else if ((angle < backField && angle >= 0) || (angle > 192 + backField && angle <= 256))//�����ܻ�
	//	pTarExt->ReceiveDamageMultiplier = this->DirectionalArmor_BackMultiplier.Get();
	//else//�����ܻ�
	//	pTarExt->ReceiveDamageMultiplier = this->DirectionalArmor_SideMultiplier.Get();
}

void WarheadTypeExtData::applyIronCurtain(const CoordStruct& coords, HouseClass* Owner, int damage) const
{
	if (this->IC_Duration != 0)
	{
		// set of affected objects. every object can be here only once.
		// affect each object
		for (auto curTechno : Helpers::Alex::getCellSpreadItems(coords, this->This()->CellSpread,
			true,
			this->CellSpread_Cylinder,
			this->AffectsInAir,
			this->AffectsGround,
			false
		))
		{

			// affects enemies or allies respectively?
			if (!this->CanAffectHouse(curTechno->Owner, Owner)) {
				continue;
			}

			auto pType = GET_TECHNOTYPE(curTechno);
			// respect verses the boolean way
			if (Math::abs(this->GetVerses(TechnoExtData::GetTechnoArmor(curTechno , this->This())).Verses) < 0.001)
			{
				continue;
			}

			// affect each object
			{
				// duration modifier
				int duration = this->IC_Duration;

				// modify good durations only
				if (duration > 0)
				{
					duration = static_cast<int>(duration * TechnoTypeExtContainer::Instance.Find(pType)->IronCurtain_Modifier);
				}

				// get the values
				if (curTechno->ProtectType == ProtectTypes::ForceShield)
				{
					// damage the victim before ICing it
					if (damage) {
						curTechno->ReceiveDamage(&damage, 0, this->This(), nullptr, true, false, Owner);
					}

					// unit may be destroyed already.
					if (curTechno->IsAlive) {
						// start and prevent the multiplier from being applied twice
						curTechno->IronCurtain(duration, Owner, false);
						curTechno->IronCurtainTimer.Start(duration);
					}

					continue;
				}

				int timeLeft = curTechno->IronCurtainTimer.GetTimeLeft();
				int oldValue = timeLeft <= 0 ? 0 : timeLeft;
				int newValue = Helpers::Alex::getCappedDuration(oldValue, duration, this->IC_Cap);

				// update iron curtain
				if (oldValue <= 0)
				{
					// start iron curtain effect?
					if (newValue > 0)
					{
						// damage the victim before ICing it
						if (damage)
						{
							curTechno->ReceiveDamage(&damage, 0, this->This(), nullptr, true, false, Owner);
						}

						// unit may be destroyed already.
						if (curTechno->IsAlive)
						{
							// start and prevent the multiplier from being applied twice
							curTechno->IronCurtain(newValue, Owner, false);
							curTechno->IronCurtainTimer.Start(newValue);
						}
					}
				}
				else
				{
					// iron curtain effect is already on.
					if (newValue > 0)
					{
						// set new length and reset tint stage
						curTechno->IronCurtainTimer.Start(newValue);
						curTechno->IronTintStage = 4;
					}
					else
					{
						// turn iron curtain off
						curTechno->IronCurtainTimer.Stop();
					}
				}
			}
		}
	}
}

void WarheadTypeExtData::applyIronCurtain(TechnoClass* curTechno, HouseClass* Owner, int damage) const
{
	if (this->IC_Duration != 0)
	{

		// affect each object
		{
			// duration modifier
			int duration = this->IC_Duration;

			auto pType = GET_TECHNOTYPE(curTechno);

			// modify good durations only
			if (duration > 0)
			{
				duration = static_cast<int>(duration * TechnoTypeExtContainer::Instance.Find(pType)->IronCurtain_Modifier);
			}

			// get the values
			int oldValue = (curTechno->IronCurtainTimer.Expired() ? 0 : curTechno->IronCurtainTimer.GetTimeLeft());
			int newValue = Helpers::Alex::getCappedDuration(oldValue, duration, this->IC_Cap);

			// update iron curtain
			if (oldValue <= 0)
			{
				// start iron curtain effect?
				if (newValue > 0)
				{
					// damage the victim before ICing it
					if (damage)
					{
						curTechno->ReceiveDamage(&damage, 0, this->This(), nullptr, true, false, Owner);
					}

					// unit may be destroyed already.
					if (curTechno->IsAlive)
					{
						// start and prevent the multiplier from being applied twice
						curTechno->IronCurtain(newValue, Owner, false);
						curTechno->IronCurtainTimer.Start(newValue);
					}
				}
			}
			else
			{
				// iron curtain effect is already on.
				if (newValue > 0)
				{
					// set new length and reset tint stage
					curTechno->IronCurtainTimer.Start(newValue);
					curTechno->IronTintStage = 4;
				}
				else
				{
					// turn iron curtain off
					curTechno->IronCurtainTimer.Stop();
				}
			}
		}
	}
}

void WarheadTypeExtData::ApplyAttachTag(TechnoClass* pTarget) const
{
	if (!this->AttachTag)
		return;

	const auto pType = GET_TECHNOTYPE(pTarget);
	bool AllowType = true;
	bool IgnoreType = false;

	if (!this->AttachTag_Types.empty())
	{
		AllowType = this->AttachTag_Types.Contains(pType);
	}

	if (!this->AttachTag_Types.empty())
	{
		IgnoreType = this->AttachTag_Types.Contains(pType);
	}

	if (!AllowType || IgnoreType)
		return;

	auto TagID = this->AttachTag.data();
	auto Imposed = this->AttachTag_Imposed;

	if ((!pTarget->AttachedTag || Imposed))
	{
		auto pTagType = TagTypeClass::FindOrAllocate(TagID);
		pTarget->AttachTrigger(TagClass::GetInstance(pTagType));
	}
}

bool WarheadTypeExtData::applyPermaMC(HouseClass* const Owner, AbstractClass* const Target) const
{
	if (!Owner || !this->PermaMC)
		return false;

	const auto pTargetTechno = flag_cast_to<TechnoClass*>(Target);
	if (!pTargetTechno)
		return false;

	if (//!this->CanDealDamage(pTargetTechno) ||
		TechnoExtData::IsPsionicsImmune(pTargetTechno))
		return false;

	if (TechnoExtContainer::Instance.Find(pTargetTechno)->Is_DriverKilled)
		return false;

	const auto pType = GET_TECHNOTYPE(pTargetTechno);

	if (auto const pController = pTargetTechno->MindControlledBy)
	{
		pController->CaptureManager->FreeUnit(pTargetTechno);
	}

	const auto pOriginalOwner = pTargetTechno->GetOwningHouse();
	pTargetTechno->SetOwningHouse(Owner);
	pTargetTechno->MindControlledByAUnit = true;
	pTargetTechno->QueueMission(Mission::Guard, false);
	pTargetTechno->OriginallyOwnedByHouse = pOriginalOwner;
	pTargetTechno->MindControlledByHouse = nullptr;

	if (auto& pAnim = pTargetTechno->MindControlRingAnim)
	{
		pAnim->TimeToDie = true;
		pAnim->UnInit();
		pAnim = nullptr;
	}

	if (auto const pAnimType = RulesClass::Instance->PermaControlledAnimationType)
	{
		auto const pBld = cast_to<BuildingClass*, false>(pTargetTechno);

		CoordStruct location = pTargetTechno->GetCoords();

		if (pBld)
		{
			location.Z += pBld->Type->Height * Unsorted::LevelHeight;
		}
		else
		{
			location.Z += pType->MindControlRingOffset;
		}

		{
			auto const pAnim = GameCreate<AnimClass>(pAnimType, location);
			pTargetTechno->MindControlRingAnim = pAnim;
			pAnim->SetOwnerObject(pTargetTechno);
			if (pBld)
			{
				pAnim->ZAdjust = -1024;
			}
		}
	}

	return true;
}

void WarheadTypeExtData::applyStealMoney(TechnoClass* const Owner, TechnoClass* const Target) const
{
	const int nStealAmout = StealMoney.Get();

	if (nStealAmout != 0)
	{
		if (Owner && Target)
		{
			auto pBulletOwnerHouse = Owner->GetOwningHouse();
			auto pBulletTargetHouse = Target->GetOwningHouse();

			if (pBulletOwnerHouse && pBulletTargetHouse)
			{
				if ((!pBulletOwnerHouse->IsNeutral() && !HouseExtData::IsObserverPlayer(pBulletOwnerHouse))
					&& (!pBulletTargetHouse->IsNeutral() && !HouseExtData::IsObserverPlayer(pBulletTargetHouse)))
				{
					if (pBulletOwnerHouse->CanTransactMoney(nStealAmout) && pBulletTargetHouse->CanTransactMoney(-nStealAmout))
					{
						pBulletOwnerHouse->TransactMoney(nStealAmout);
						FlyingStrings::Instance.AddMoneyString(Steal_Display.Get(), nStealAmout, Owner, Steal_Display_Houses.Get(), Owner->GetCoords(), Steal_Display_Offset.Get(), ColorStruct::Empty);
						pBulletTargetHouse->TransactMoney(-nStealAmout);
						FlyingStrings::Instance.AddMoneyString(Steal_Display.Get(), -nStealAmout, Target, Steal_Display_Houses.Get(), Target->GetCoords(), Steal_Display_Offset.Get(), ColorStruct::Empty);

					}
				}
			}
		}
	}
}

void WarheadTypeExtData::applyTransactMoney(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct const& coords) const
{
	int nTransactVal = 0;
	bool bForSelf = true;
	bool bSucceed = false;

	auto const TransactMoneyFunc = [&]()
		{
			if (pBullet && pBullet->Target)
			{
				this->applyStealMoney(pBullet->Owner, static_cast<TechnoClass*>(pBullet->Target));

				if (TransactMoney != 0)
				{
					auto const pBulletTargetHouse = pBullet->Target->GetOwningHouse();
					if (pBulletTargetHouse)
					{
						if ((!pHouse->IsNeutral() && !HouseExtData::IsObserverPlayer(pHouse)) && (!pBulletTargetHouse->IsNeutral() && !HouseExtData::IsObserverPlayer(pBulletTargetHouse)))
						{
							if (Transact_AffectsOwner.Get() && pBulletTargetHouse == pHouse)
							{
								nTransactVal = TransactMoney;
								if (nTransactVal != 0 && pHouse->CanTransactMoney(nTransactVal))
								{
									pHouse->TransactMoney(TransactMoney);
									bSucceed = true;
									return;
								}
							}

							if (pHouse->IsAlliedWith(pBulletTargetHouse))
							{
								if (Transact_AffectsAlly.Get() && pBulletTargetHouse != pHouse)
								{
									nTransactVal = TransactMoney_Ally.Get(TransactMoney);
									if (nTransactVal != 0 && pBulletTargetHouse->CanTransactMoney(nTransactVal))
									{
										pBulletTargetHouse->TransactMoney(nTransactVal);
										bSucceed = true;
										bForSelf = false;
										return;
									}
								}
							}
							else
							{
								if (Transact_AffectsEnemies.Get())
								{
									nTransactVal = TransactMoney_Enemy.Get(TransactMoney);
									if (nTransactVal != 0 && pBulletTargetHouse->CanTransactMoney(nTransactVal))
									{
										pBulletTargetHouse->TransactMoney(nTransactVal);
										bSucceed = true;
										bForSelf = false;
										return;
									}
								}
							}
						}
					}
				}
			}

			nTransactVal = TransactMoney;
			if (nTransactVal != 0 && pHouse->CanTransactMoney(nTransactVal))
			{
				pHouse->TransactMoney(TransactMoney);
				bSucceed = true;
			}
		};

	TransactMoneyFunc();

	if (nTransactVal != 0 && bSucceed && TransactMoney_Display.Get())
	{
		auto displayCoord = TransactMoney_Display_AtFirer ? (pOwner ? pOwner->Location : coords) : (!bForSelf ? pBullet && pBullet->Target ? pBullet->Target->GetCoords() : coords : coords);
		auto pDrawOwner = TransactMoney_Display_AtFirer ? (pOwner ? pOwner : nullptr) : (!bForSelf ? (pBullet && pBullet->Target ? flag_cast_to<TechnoClass*>(pBullet->Target) : nullptr) : nullptr);

		FlyingStrings::Instance.AddMoneyString(true, nTransactVal, pDrawOwner, TransactMoney_Display_Houses.Get(), displayCoord, TransactMoney_Display_Offset.Get(), ColorStruct::Empty);
	}
}

void WarheadTypeExtData::InterceptBullets(TechnoClass* pOwner, BulletClass* pBullet, CoordStruct coords)
{
	const float cellSpread = this->This()->CellSpread;

	if (cellSpread == 0.0)
	{
		if (auto const pTargetBullet = cast_to<BulletClass*>(pBullet->Target))
		{
			if (BulletTypeExtContainer::Instance.Find(pTargetBullet->Type)->Interceptable) {
				// 1/8th of a cell as a margin of error.
				if (!pBullet->SpawnNextAnim && (pTargetBullet->Type->Inviso || pTargetBullet->Location.DistanceFrom(coords) <= Unsorted::LeptonsPerCell / 8.0)) {
					BulletExtData::InterceptBullet(pTargetBullet, pOwner, pBullet);
				}
			}
		}
	}
	else
	{
		BulletClass::Array->for_each([&](BulletClass* pTargetBullet) {
			 if (pTargetBullet)
			 {
				 const auto pBulletExt = BulletExtContainer::Instance.Find(pTargetBullet);
				 if (pBulletExt->CurrentStrength > 0 && !pTargetBullet->InLimbo)
				 {
					 auto const pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pTargetBullet->Type);
					 // Cells don't know about bullets that may or may not be located on them so it has to be this way.
					 if (pBulletTypeExt->Interceptable && !pBullet->SpawnNextAnim &&
						 pTargetBullet->Location.DistanceFrom(coords) <= (cellSpread * Unsorted::LeptonsPerCell))
						 BulletExtData::InterceptBullet(pTargetBullet, pOwner, pBullet);
				 }
			 }
		});
	}
}

static void SpawnCrate(std::vector<int>& types, std::vector<int>& weights, CoordStruct& place)
{
	if (!types.empty()) {
		const int index = GeneralUtils::ChooseOneWeighted(ScenarioClass::Instance->Random.RandomDouble(), weights);

		if ((size_t)index < types.size()) {
			MapClass::Instance->Place_Crate(CellClass::Coord2Cell(place), (PowerupEffects)types[index]);
		}
	}
}

static bool NOINLINE IsCellSpreadWH(WarheadTypeExtData* pData)
{
	// List all Warheads here that respect CellSpread

	return pData->IsCellSpreadWH
		;
}

void WarheadTypeExtData::Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct coords, int damage)
{
	VocClass::SafeImmedietelyPlayAt(Sound, &coords);
	for (auto const& pSys : this->DetonateParticleSystem)
	{
		GameCreate<ParticleSystemClass>(pSys, coords, nullptr, nullptr, CoordStruct::Empty, pHouse);
	}

	SpawnCrate(this->SpawnsCrate_Types, this->SpawnsCrate_Weights, coords);

	if (!pBullet)
	{
		for (auto const& pWeapon : this->DetonatesWeapons)
		{
			WeaponTypeExtData::DetonateAt3(pWeapon, coords, pOwner, true, pHouse);
		}
	}

	if (pOwner && pBullet)
	{
		if (pOwner->IsAlive && pBullet->IsAlive && BulletExtContainer::Instance.Find(pBullet)->InterceptorTechnoType){
				this->InterceptBullets(pOwner, pBullet, coords);
		}

		//TechnoExtData::PutPassengersInCoords(pBullet->Owner, coords, RulesGlobal->WarpIn, RulesGlobal->BunkerWallsUpSound, false);
	}


	if (pHouse)
	{
		if (this->BigGap) {
			HouseClass::Array->for_each([&](HouseClass* pOtherHouse) {
				 if (pOtherHouse->IsControlledByHuman() &&	  // Not AI
					 !HouseExtData::IsObserverPlayer(pOtherHouse) &&		  // Not Observer
					 !pOtherHouse->Defeated &&			  // Not Defeated
					 pOtherHouse != pHouse &&			  // Not pThisHouse
					 !pOtherHouse->SpySatActive && // No SpySat
					 !pHouse->IsAlliedWith(pOtherHouse))   // Not Allied
				 {
					 pOtherHouse->ReshroudMap();
				 }
			});
		} else {
			if (this->CreateGap > 0)
			{
				const auto pCurrent = HouseClass::CurrentPlayer();

				if (pCurrent &&
					!pCurrent->IsObserver() &&		// Not Observer
					!pCurrent->Defeated &&						// Not Defeated
					pCurrent != pHouse &&						// Not pThisHouse
					!pCurrent->SpySatActive &&				// No SpySat
					!pCurrent->IsAlliedWith(pHouse))			// Not Allied
				{
					CellClass::CreateGap(pCurrent, this->CreateGap, coords);
				}
			}
			else if (this->CreateGap < 0)
			{
				HouseClass::Array->for_each([&](HouseClass* pOtherHouse)
				{
					if (pOtherHouse->IsControlledByHuman() &&	  // Not AI
						!HouseExtData::IsObserverPlayer(pOtherHouse) &&		  // Not Observer
						!pOtherHouse->Defeated &&			  // Not Defeated
						pOtherHouse != pHouse &&			  // Not pThisHouse
						!pOtherHouse->SpySatActive && // No SpySat
						!pHouse->IsAlliedWith(pOtherHouse))   // Not Allied
					{
						MapClass::Instance->Reshroud(pOtherHouse);
					}
				});
			}
		}

		if (this->Reveal < 0 //|| this->SpySat
			)
		{
			MapClass::Instance->Reveal(pHouse);
		}
		else if (this->Reveal > 0)
		{
			SW_Reveal::RevealMap(CellClass::Coord2Cell(coords), (float)this->Reveal.Get(), 0, pHouse);
		}

		this->applyTransactMoney(pOwner, pHouse, pBullet, coords);
	}

	 this->GetCritChance(pOwner, this->CritCurrentChance);

	 if (!this->Crit_ApplyChancePerTarget)
		this->CritRandomBuffer = ScenarioClass::Instance->Random.RandomDouble();

	//const bool ISPermaMC = this->PermaMC && !pBullet;

	if ((this->IsCellSpreadWH || this->CritCurrentChance > 0.0) && this->ApplyPerTargetEffectsOnDetonate.Get(RulesExtData::Instance()->ApplyPerTargetEffectsOnDetonate))
	{
		if (this->Crit_ActiveChanceAnims.size() > 0 && this->CritCurrentChance > 0.0)
		{
			int idx = ScenarioClass::Instance->Random.RandomRanged(0, this->Crit_ActiveChanceAnims.size() - 1);
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(this->Crit_ActiveChanceAnims[idx], coords),
				pHouse,
				nullptr,
				pOwner,
				false, false);
		}

		const bool ThisbulletWasIntercepted = pBullet ? bool(BulletExtContainer::Instance.Find(pBullet)->InterceptedStatus & InterceptedStatus::Intercepted) : false;
		const float cellSpread = this->This()->CellSpread;


		//if the warhead itself has cellspread
		if (Math::abs(cellSpread) >= 0.1f)
		{
			std::vector<TechnoClass*> pTargetv = Helpers::Alex::getCellSpreadItems(coords, cellSpread,
				true,
				this->CellSpread_Cylinder,
				this->AffectsInAir,
				this->AffectsGround,
				false
				);

			std::ranges::for_each(pTargetv, [&](TechnoClass* pTarget) {
				this->DetonateOnOneUnit(pHouse, pTarget, coords , damage, pOwner, pBullet, ThisbulletWasIntercepted);
			});

			if (this->Transact)
			{
				this->TransactOnAllUnits(pTargetv, pHouse, pOwner);
			}

		} //no cellspread but it has bullet
		else if (pBullet && pBullet->Target)
		{
			{
				if (pBullet->DistanceFrom(pBullet->Target) < Unsorted::LeptonsPerCell / 4) {
					switch (pBullet->Target->WhatAmI())
					{
					case BuildingClass::AbsID:
					case AircraftClass::AbsID:
					case UnitClass::AbsID:
					case InfantryClass::AbsID:
					{
						const auto Eligible = [&](TechnoClass* const pTech)
							{
								if (CanDealDamage(pTech) &&
								CanTargetHouse(pHouse, pTech) &&
								GET_TECHNOTYPE(pTech)->Trainable
								) return pTech;

								return static_cast<TechnoClass* const>(nullptr);
							};

						this->DetonateOnOneUnit(pHouse, static_cast<TechnoClass*>(pBullet->Target), coords , damage, pOwner, pBullet, ThisbulletWasIntercepted);

						if (this->Transact)
							this->TransactOnOneUnit(Eligible(static_cast<TechnoClass*>(pBullet->Target)), pOwner, 1);

					}break;
					case CellClass::AbsID:
					{
						if (this->Transact)
							this->TransactOnOneUnit(nullptr, pOwner, 1);
					}break;
					default:
						break;
					}
				}
			}

		}
		else if (auto pIntended = this->IntendedTarget)
		{
			if(!this->IntendedTarget->IsAlive){
				this->IntendedTarget = nullptr;
				return;
			}

			if (coords.DistanceFrom(pIntended->GetCoords()) < double(Unsorted::LeptonsPerCell / 4)) {
				this->DetonateOnOneUnit(pHouse, pIntended, coords , damage, pOwner, pBullet, ThisbulletWasIntercepted);

				if (this->Transact) {

					//since we are on last chain of the event , we can do these thing
					const auto NotEligible = [this, pHouse, pOwner](TechnoClass* const pTech) {
							if (!CanDealDamage(pTech))
								return true;

							if (!GET_TECHNOTYPE(pTech)->Trainable && this->Transact_Experience_IgnoreNotTrainable.Get())
								return true;

							return !CanTargetHouse(pHouse, pTech);
					};

					if(!NotEligible(pIntended))
						this->TransactOnOneUnit(pIntended, pOwner, 1);
				}
			}
		}
	}
}

void WarheadTypeExtData::DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, const CoordStruct& coords, int damage, TechnoClass* pOwner, BulletClass* pBullet, bool bulletWasIntercepted)
{
	if (!this->CanDealDamage(pTarget) || !this->CanTargetHouse(pHouse, pTarget))
		return;

	if (this->RemoveMindControl) {
		const HouseClass* pOldHouse = pTarget->Owner;
		const HouseClass* pNewHouse = this->ApplyRemoveMindControl(pHouse, pTarget);

		//recheck the targeting
		if(pNewHouse != pOldHouse && !this->CanTargetHouse(pHouse, pTarget))
			return;
	}

	TechnoTypeConvertData::ApplyConvert(this->ConvertsPair, pHouse, pTarget, this->Convert_SucceededAnim);

	if (this->BuildingSell || this->BuildingUndeploy) {
		this->ApplyBuildingUndeploy(pTarget);

		if (!pTarget->IsAlive)
			return;
	}

	if (this->RemoveDisguise)
		this->ApplyRemoveDisguise(pHouse, pTarget);

	this->applyWebby(pTarget, pHouse, pOwner);

	if (!pTarget->IsAlive)
		return;

	auto pExt = TechnoExtContainer::Instance.Find(pTarget);

	if(this->PaintBallDuration.isset() && this->PaintBallData.Color != ColorStruct::Empty) {
		auto& paintball = pExt->PaintBallStates[this->This()];
		paintball.SetData(this->PaintBallData);
		paintball.Init();

		if(this->PaintBallDuration < 0 || this->PaintBallData.Accumulate){
			int value = paintball.timer.GetTimeLeft() + this->PaintBallDuration;

			if (value <= 0) {
				paintball.timer.Stop();
			} else {
				paintball.timer.Add(value);
			}

		} else{

			if (this->PaintBallData.Override && paintball.timer.GetTimeLeft()) {
				paintball.timer.Start(this->PaintBallDuration);
			} else {
				paintball.timer.Start(this->PaintBallDuration);
			}
		}
	}

	if (this->GattlingStage > 0) {
		this->ApplyGattlingStage(pTarget, this->GattlingStage);
	}

	if (this->GattlingRateUp != 0) {
		this->ApplyGattlingRateUp(pTarget, this->GattlingRateUp);
	}

	if (this->ReloadAmmo != 0) {
		this->ApplyReloadAmmo(pTarget, this->ReloadAmmo);
	}

	if (this->AttachTag)
		this->ApplyAttachTag(pTarget);

	if (this->ReverseEngineer) {
		HouseExtContainer::Instance.Find(pHouse)->ReverseEngineer(pTarget);
	}

	this->ApplyShieldModifiers(pTarget);

	if (!this->PhobosAttachEffects.AttachTypes.empty()
	|| !this->PhobosAttachEffects.RemoveTypes.empty()
	|| !this->PhobosAttachEffects.RemoveGroups.empty()
	) {
		this->ApplyAttachEffects(pTarget, pHouse, pOwner);
	}

	if (this->RevengeWeapon && this->RevengeWeapon_GrantDuration > 0) {
		this->ApplyRevengeWeapon(pTarget);

		if (!pTarget->IsAlive)
			return;
	}

	if (this->CritCurrentChance > 0.0 && (!this->Crit_SuppressWhenIntercepted || !bulletWasIntercepted)) {
		this->ApplyCrit(pHouse, pTarget, pOwner);

		if (!pTarget->IsAlive)
			return;
	}

	if (this->PenetratesTransport_Level > 0 && damage) {
		this->ApplyPenetratesTransport(pTarget, pOwner, pHouse, coords, damage);

		if (!pTarget->IsAlive)
			return;
	}

	if (!this->LimboKill_IDs.empty()) {
		BuildingExtData::ApplyLimboKill(this->LimboKill_IDs, this->LimboKill_Affected, pTarget->Owner, pHouse);

		if (!pTarget->IsAlive)
			return;
	}


	//if (this->DirectionalArmor.Get())
	//	this->ApplyDirectional(pBullet, pTarget);

	//if (this->InflictLocomotor)
	//	this->ApplyLocomotorInfliction(pTarget);

	//if (this->RemoveInflictedLocomotor)
	//	this->ApplyLocomotorInflictionReset(pTarget);

	if (this->PermaMC)
		this->applyPermaMC(pHouse, pTarget);
}

//void WarheadTypeExtData::DetonateOnAllUnits(HouseClass* pHouse, const CoordStruct coords, const float cellSpread, TechnoClass* pOwner)
//{
//	for (auto pTarget : Helpers::Alex::getCellSpreadItems(coords, cellSpread, true))
//	{
//		this->DetonateOnOneUnit(pHouse, pTarget, pOwner);
//	}
//}

void WarheadTypeExtData::ApplyShieldModifiers(TechnoClass* pTarget)
{
	if (!pTarget)
		return;

	auto pExt = TechnoExtContainer::Instance.Find(pTarget);


	int shieldIndex = -1;
	double oldRatio = 1.0;

	// Remove shield.
	if (pExt->GetShield())
	{
		shieldIndex = this->Shield_RemoveTypes.IndexOf(pExt->Shield->GetType());

		if (shieldIndex >= 0 || this->Shield_RemoveAll)
		{
			oldRatio = pExt->Shield->GetHealthRatio();
			pExt->CurrentShieldType = ShieldTypeClass::FindOrAllocate(DEFAULT_STR2);;
			pExt->Shield.reset(nullptr);
		}
	}

	// Attach shield.
	if (!this->Shield_AttachTypes.empty())
	{
		ShieldTypeClass* shieldType = nullptr;

		if (this->Shield_ReplaceOnly)
		{
			if (shieldIndex >= 0)
			{
				const int nMax = (Shield_AttachTypes.size() - 1);
				shieldType = Shield_AttachTypes[MinImpl(shieldIndex, nMax)];
			}
			else if (this->Shield_RemoveAll)
				shieldType = Shield_AttachTypes[0];

		}
		else
		{
			shieldType = Shield_AttachTypes[0];
		}

		if (shieldType)
		{
			if (shieldType->Strength && (!pExt->Shield || (this->Shield_ReplaceNonRespawning && pExt->Shield->IsBrokenAndNonRespawning() &&
				pExt->Shield->GetFramesSinceLastBroken() >= this->Shield_MinimumReplaceDelay)))
			{
				pExt->CurrentShieldType = shieldType;
				pExt->Shield = std::make_unique<ShieldClass>(pTarget, true);
				pExt->Shield->UpdateTint();

				if (this->Shield_ReplaceOnly && this->Shield_InheritStateOnReplace)
				{
					pExt->Shield->SetHP((int)(shieldType->Strength * oldRatio));

					if (pExt->Shield->GetHP() == 0)
						pExt->Shield->SetRespawn(
							shieldType->Respawn_Rate,
							shieldType->Respawn,
							shieldType->Respawn_Rate,
							shieldType->Respawn_RestartInCombat,
							-1,
							true,
							&shieldType->Respawn_Anim.AsVector()
						);
				}
			}
		}
	}

	// Apply other modifiers.
	if (pExt->GetShield())
	{
		const auto pCurrentType = pExt->Shield->GetType();

		if (!this->Shield_AffectTypes.empty() && !this->Shield_AffectTypes.Contains(pCurrentType))
			return;

		if (this->Shield_Break && pExt->Shield->IsActive() && this->Shield_Break_Types.Eligible(this->Shield_AffectTypes, pCurrentType))
			pExt->Shield->BreakShield(this->Shield_BreakAnim, this->Shield_BreakWeapon);

		if ((this->Shield_Respawn_Duration > 0 || this->Shield_Respawn_RestartTimer)
			&& this->Shield_Respawn_Types.Eligible(this->Shield_AffectTypes, pCurrentType)) {
				pExt->Shield->SetRespawn(
					this->Shield_Respawn_Duration,
					this->Shield_Respawn_Amount.Get(pCurrentType->Respawn),
					this->Shield_Respawn_Rate,
					this->Shield_Respawn_RestartInCombat.Get(pCurrentType->Respawn_RestartInCombat),
					this->Shield_Respawn_RestartInCombatDelay,
					this->Shield_Respawn_RestartTimer,
					&this->Shield_Respawn_Anim.AsVector(),
					this->Shield_Respawn_Weapon
				);
		}

		if ((this->Shield_SelfHealing_Duration > 0 || this->Shield_SelfHealing_RestartTimer)
		 	&& this->Shield_SelfHealing_Types.Eligible(this->Shield_AffectTypes, pCurrentType))
		{
			pExt->Shield->SetSelfHealing(
				this->Shield_SelfHealing_Duration,
				this->Shield_SelfHealing_Amount.Get(pCurrentType->SelfHealing),
				this->Shield_SelfHealing_Rate,
				this->Shield_SelfHealing_RestartInCombat.Get(pCurrentType->SelfHealing_RestartInCombat),
				this->Shield_SelfHealing_RestartInCombatDelay,
				this->Shield_SelfHealing_RestartTimer
			);
		}
	}
}

HouseClass*  WarheadTypeExtData::ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget) const
{
	if (const auto pController = pTarget->MindControlledBy) {
		pController->CaptureManager->FreeUnit(pTarget);
		return pTarget->Owner;
	}

	return nullptr;
}

void WarheadTypeExtData::ApplyRemoveDisguise(HouseClass* pHouse, TechnoClass* pTarget) const
{
	//this is here , just in case i need special treatment for `TankDisguiseAsTank`
	if(pTarget->IsAlive)
	{
		if (pTarget->IsDisguised())
		{
			if (auto pSpy = cast_to<InfantryClass*, false>(pTarget))
			   pSpy->Disguised = false;
		    else if (auto pMirage = cast_to<UnitClass*, false>(pTarget))
			   pMirage->ClearDisguise();
		}
	}
}

// https://github.com/Phobos-developers/Phobos/pull/1263
 // TODO : update
void WarheadTypeExtData::ApplyCrit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner)
{
	if (TechnoExtData::IsCritImmune(pTarget))
		return;

	double dice;

	if (this->Crit_ApplyChancePerTarget|| !this->ApplyPerTargetEffectsOnDetonate.Get(RulesExtData::Instance()->ApplyPerTargetEffectsOnDetonate))
		dice = ScenarioClass::Instance->Random.RandomDouble();
	else
		dice = this->CritRandomBuffer;

	if (this->CritCurrentChance < dice)
		return;

	auto const pTargetExt = TechnoExtContainer::Instance.Find(pTarget);

	auto pSld = pTargetExt->Shield.get();

	if (pSld && pSld->IsActive() && pSld->GetType()->ImmuneToCrit)
		return;

	if (!TechnoExtData::IsHealthInThreshold(pTarget, this->Crit_AffectAbovePercent, this->Crit_AffectBelowPercent))
		return;

	if (pHouse && !EnumFunctions::CanTargetHouse(this->Crit_AffectsHouses, pHouse, pTarget->Owner))
		return;

	if (!EnumFunctions::IsCellEligible(pTarget->GetCell(), this->Crit_Affects))
		return;

	if (!EnumFunctions::IsTechnoEligible(pTarget, this->Crit_Affects))
		return;

	this->CritActive = true;

	if (this->Crit_AnimOnAffectedTargets && this->Crit_AnimList.size())
	{
		if (!this->Crit_AnimList_CreateAll.Get(false))
		{
			const int idx = this->This()->EMEffect || this->Crit_AnimList_PickRandom.Get(false) ?
				ScenarioClass::Instance->Random.RandomRanged(0, this->Crit_AnimList.size() - 1) : 0;

			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(this->Crit_AnimList[idx], pTarget->Location),
			pHouse,
			pTarget->GetOwningHouse(),
			pOwner,
			false, false);
		}
		else
		{
			for (auto const& pType : this->Crit_AnimList)
			{
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pTarget->Location),
				pHouse,
				pTarget->GetOwningHouse(),
				pOwner,
				false, false);
			}
		}
	}

	auto damage = this->Crit_ExtraDamage.Get();
		damage = static_cast<int>(TechnoExtData::GetDamageMult(pOwner, damage, !this->Crit_ExtraDamage_ApplyFirepowerMult));

	if (this->Crit_Warhead)
	{
		if (this->Crit_Warhead_FullDetonation)
			WarheadTypeExtData::DetonateAt(this->Crit_Warhead.Get(), pTarget, pOwner, damage, pHouse);
		else
			this->DamageAreaWithTarget(pTarget->GetCoords(), damage, pOwner, this->Crit_Warhead, true, pHouse, pTarget);
	}
	else
		pTarget->ReceiveDamage(&damage, 0, this->This(), pOwner, false, false, pHouse);
}

void WarheadTypeExtData::ApplyGattlingStage(TechnoClass* pTarget, int Stage) const
{
	auto pData = GET_TECHNOTYPE(pTarget);

	if (pData->IsGattling)
	{
		// if exceeds, pick the largest stage
		if (Stage > pData->WeaponStages)
		{
			Stage = pData->WeaponStages;
		}

		pTarget->CurrentGattlingStage = Stage - 1;
		if (Stage == 1)
		{
			pTarget->GattlingValue = 0;
			pTarget->GattlingAudioPlayed = false;
		}
		else
		{
			pTarget->GattlingValue = pTarget->Veterancy.IsElite() ? pData->EliteStage[Stage - 2] : pData->WeaponStage[Stage - 2];
			//pTarget->GattlingAudioPlayed = true;
			pTarget->Audio4.AudioEventHandleStop();
			pTarget->GattlingAudioPlayed = false;
		}
	}
}

void WarheadTypeExtData::ApplyGattlingRateUp(TechnoClass* pTarget, int RateUp) const
{
	auto pData = GET_TECHNOTYPE(pTarget);

	if (pData->IsGattling)
	{
		auto curValue = pTarget->GattlingValue + RateUp;
		auto maxValue = pTarget->Veterancy.IsElite() ? pData->EliteStage[pData->WeaponStages - 1] : pData->WeaponStage[pData->WeaponStages - 1];

		//set current weapon stage manually
		if (curValue <= 0)
		{
			pTarget->GattlingValue = 0;
			pTarget->CurrentGattlingStage = 0;
			pTarget->GattlingAudioPlayed = false;
		}
		else if (curValue >= maxValue)
		{
			pTarget->GattlingValue = maxValue;
			pTarget->CurrentGattlingStage = pData->WeaponStages - 1;
			pTarget->GattlingAudioPlayed = true;
		}
		else
		{
			pTarget->GattlingValue = curValue;
			pTarget->GattlingAudioPlayed = true;
			for (int i = 0; i < pData->WeaponStages; i++)
			{
				if (pTarget->Veterancy.IsElite() && curValue < pData->EliteStage[i])
				{
					pTarget->CurrentGattlingStage = i;
					break;
				}
				else if (curValue < pData->WeaponStage[i])
				{
					pTarget->CurrentGattlingStage = i;
					break;
				}
			}
		}
	}
}

void WarheadTypeExtData::ApplyReloadAmmo(TechnoClass* pTarget, int ReloadAmount) const
{
	auto pData = GET_TECHNOTYPE(pTarget);

	if (pData->Ammo > 0)
	{
		auto const ammo = pTarget->Ammo + ReloadAmount;
		pTarget->Ammo = std::clamp(ammo, 0, pData->Ammo);
	}
}

void WarheadTypeExtData::ApplyRevengeWeapon(TechnoClass* pTarget) const
{
	auto const maxCount = this->RevengeWeapon_MaxCount;
	if (!maxCount)
		return;

	int count = 0;

	if (auto const pExt = TechnoExtContainer::Instance.Find(pTarget))
	{
		if (!this->RevengeWeapon_Cumulative)
		{
			for (auto& weapon : pExt->RevengeWeapons)
			{
				// If it is same weapon just refresh timer.
				if (weapon.SourceWarhead && weapon.SourceWarhead == this->This())
				{
					auto const nDur = this->RevengeWeapon_GrantDuration.Get();
					auto const nTime = weapon.Timer.GetTimeLeft();

					weapon.Timer.Start(MaxImpl(nDur, nTime));
					return;
				}

				count++;
			}
		}

		if (maxCount < 0 || count < maxCount)
		{
			pExt->RevengeWeapons.emplace_back(this->RevengeWeapon.Get(), this->RevengeWeapon_GrantDuration, this->RevengeWeapon_AffectsHouses, this->This());
		}
	}
}