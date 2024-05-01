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
#include <Misc/Ares/Hooks/AresNetEvent.h>
#include <Ares_TechnoExt.h>

void WarheadTypeExtData::ApplyLocomotorInfliction(TechnoClass* pTarget) const
{
	auto pTargetFoot = abstract_cast<FootClass*>(pTarget);
	if (!pTargetFoot)
		return;

	// same locomotor? no point to change
	CLSID targetCLSID { };
	CLSID inflictCLSID = this->AttachedToObject->Locomotor;
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
	auto pTargetFoot = abstract_cast<FootClass*>(pTarget);

	if (!pTargetFoot)
		return;

	// remove only specific inflicted locomotor if specified
	CLSID removeCLSID = this->AttachedToObject->Locomotor;
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

	//const int angle = abs(bulletFacing - tarFacing);
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
		auto items = Helpers::Alex::getCellSpreadItems(coords, this->AttachedToObject->CellSpread, true);

		// affect each object
		for (auto curTechno : items)
		{
			// affects enemies or allies respectively?
			if (!this->CanAffectHouse(curTechno->Owner, Owner))
			{
				continue;
			}

			auto pType = curTechno->GetTechnoType();
			// respect verses the boolean way
			if (std::abs(this->GetVerses(TechnoExtData::GetTechnoArmor(curTechno , this->AttachedToObject)).Verses) < 0.001)
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
							curTechno->ReceiveDamage(&damage, 0, this->AttachedToObject, nullptr, true, false, Owner);
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

			auto pType = curTechno->GetTechnoType();

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
						curTechno->ReceiveDamage(&damage, 0, this->AttachedToObject, nullptr, true, false, Owner);
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

	const auto pType = pTarget->GetTechnoType();
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

void WarheadTypeExtData::ApplyUpgrade(HouseClass* pHouse, TechnoClass* pTarget) const
{
	TechnoTypeConvertData::ApplyConvert(this->ConvertsPair, pHouse, pTarget, this->Convert_SucceededAnim);
}

bool WarheadTypeExtData::applyPermaMC(HouseClass* const Owner, AbstractClass* const Target) const
{
	if (!Owner || !this->PermaMC)
		return false;

	const auto pTargetTechno = abstract_cast<TechnoClass*>(Target);
	if (!pTargetTechno)
		return false;

	if (//!this->CanDealDamage(pTargetTechno) ||
		TechnoExtData::IsPsionicsImmune(pTargetTechno))
		return false;

	if (TechnoExtContainer::Instance.Find(pTargetTechno)->Is_DriverKilled)
		return false;

	const auto pType = pTargetTechno->GetTechnoType();

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
		auto const pBld = specific_cast<BuildingClass*>(pTargetTechno);

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
						FlyingStrings::AddMoneyString(Steal_Display.Get(), nStealAmout, Owner, Steal_Display_Houses.Get(), Owner->GetCoords(), Steal_Display_Offset.Get());
						pBulletTargetHouse->TransactMoney(-nStealAmout);
						FlyingStrings::AddMoneyString(Steal_Display.Get(), -nStealAmout, Target, Steal_Display_Houses.Get(), Target->GetCoords(), Steal_Display_Offset.Get());

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
		auto pDrawOwner = TransactMoney_Display_AtFirer ? (pOwner ? pOwner : nullptr) : (!bForSelf ? (pBullet && pBullet->Target ? generic_cast<TechnoClass*>(pBullet->Target) : nullptr) : nullptr);

		FlyingStrings::AddMoneyString(true, nTransactVal, pDrawOwner, TransactMoney_Display_Houses.Get(), displayCoord, TransactMoney_Display_Offset.Get());
	}
}

void WarheadTypeExtData::InterceptBullets(TechnoClass* pOwner, WeaponTypeClass* pWeapon, CoordStruct coords) const
{
	if (!pOwner || !pWeapon)
		return;

	const float cellSpread = this->AttachedToObject->CellSpread;

	if (cellSpread == 0.0)
	{
		if (auto const pBullet = specific_cast<BulletClass*>(pOwner->Target))
		{
			// 1/8th of a cell as a margin of error.
			if (BulletTypeExtContainer::Instance.Find(pBullet->Type)->Interceptable && (pWeapon->Projectile->Inviso || pBullet->Location.DistanceFrom(coords) <= Unsorted::LeptonsPerCell / 8.0))
				BulletExtData::InterceptBullet(pBullet, pOwner, pWeapon);
		}
	}
	else
	{
		BulletClass::Array->for_each([&](BulletClass* pTargetBullet)
 {
	 if (pTargetBullet)
	 {
		 const auto pBulletExt = BulletExtContainer::Instance.Find(pTargetBullet);
		 if (pBulletExt->CurrentStrength > 0 && !pTargetBullet->InLimbo)
		 {
			 auto const pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pTargetBullet->Type);
			 // Cells don't know about bullets that may or may not be located on them so it has to be this way.
			 if (pBulletTypeExt->Interceptable &&
				 pTargetBullet->Location.DistanceFrom(coords) <= (cellSpread * Unsorted::LeptonsPerCell))
				 BulletExtData::InterceptBullet(pTargetBullet, pOwner, pWeapon);
		 }
	 }
		});
	}
}

void SpawnCrate(std::vector<int>& types, std::vector<int>& weights, CoordStruct& place)
{
	if (!types.empty()) {
		const int index = GeneralUtils::ChooseOneWeighted(ScenarioClass::Instance->Random.RandomDouble(), weights);

		if ((size_t)index < types.size()) {
			MapClass::Instance->Place_Crate(CellClass::Coord2Cell(place), (PowerupEffects)types[index]);
		}
	}
}

bool NOINLINE IsCellSpreadWH(WarheadTypeExtData* pData)
{
	// List all Warheads here that respect CellSpread

	return //pData->RemoveDisguise ||
		//pData->RemoveMindControl ||
		//pData->Crit_Chance ||
		pData->Shield_Break ||
		(pData->Converts && !pData->ConvertsPair.empty()) ||
		pData->Shield_Respawn_Duration > 0 ||
		pData->Shield_SelfHealing_Duration > 0 ||
		!pData->Shield_AttachTypes.empty() ||
		!pData->Shield_RemoveTypes.empty() ||
		pData->Transact ||
		pData->PermaMC ||
		pData->GattlingStage > 0 ||
		pData->GattlingRateUp != 0 ||
		pData->AttachTag ||
		//pData->DirectionalArmor ||
		pData->ReloadAmmo != 0
		|| (pData->RevengeWeapon.isset() && pData->RevengeWeapon_GrantDuration > 0)
		|| !pData->LimboKill_IDs.empty()
		|| (pData->PaintBallData.Color != ColorStruct::Empty)
		|| pData->InflictLocomotor
		|| pData->RemoveInflictedLocomotor
		|| pData->IC_Duration != 0

		|| pData->AttachEffect_AttachTypes.size() > 0
		|| pData->AttachEffect_RemoveTypes.size() > 0
		|| pData->AttachEffect_RemoveGroups.size() > 0

		;
}

void WarheadTypeExtData::Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct coords, int damage)
{
	VocClass::PlayIndexAtPos(Sound, coords);
	for (auto const& pSys : this->DetonateParticleSystem)
	{
		GameCreate<ParticleSystemClass>(pSys, coords, nullptr, nullptr, CoordStruct::Empty, pHouse);
	}

	SpawnCrate(this->SpawnsCrate_Types, this->SpawnsCrate_Weights, coords);

	if (!pBullet)
	{
		for (auto const& pWeapon : this->DetonatesWeapons)
		{
			WeaponTypeExtData::DetonateAt(pWeapon, coords, pOwner, true, pHouse);
		}
	}

	if (pOwner && pBullet)
	{
		if (pOwner->IsAlive && pBullet->IsAlive)
			if (TechnoExtContainer::Instance.Find(pOwner)->IsInterceptor() && BulletExtContainer::Instance.Find(pBullet)->IsInterceptor)
				this->InterceptBullets(pOwner, pBullet->WeaponType, coords);

		//TechnoExtData::PutPassengersInCoords(pBullet->Owner, coords, RulesGlobal->WarpIn, RulesGlobal->BunkerWallsUpSound, false);
	}


	if (pHouse)
	{
		if (this->BigGap)
		{
			HouseClass::Array->for_each([&](HouseClass* pOtherHouse)
 {
	 if (pOtherHouse->IsControlledByHuman() &&	  // Not AI
		 !HouseExtData::IsObserverPlayer(pOtherHouse) &&		  // Not Observer
		 !pOtherHouse->Defeated &&			  // Not Defeated
		 pOtherHouse != pHouse &&			  // Not pThisHouse
		 !pHouse->IsAlliedWith(pOtherHouse))   // Not Allied
	 {
		 pOtherHouse->ReshroudMap();
	 }
			});
		}

		if (this->Reveal < 0)
		{
			MapClass::Instance->Reveal(pHouse);
		}
		else if (this->Reveal > 0)
		{
			SW_Reveal::RevealMap(CellClass::Coord2Cell(coords), (float)this->Reveal.Get(), 0, pHouse);
		}

		this->applyTransactMoney(pOwner, pHouse, pBullet, coords);
	}

	this->Crit_CurrentChance = this->GetCritChance(pOwner);

	this->RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();
	//const bool ISPermaMC = this->PermaMC && !pBullet;

	if (IsCellSpreadWH(this) || this->Crit_CurrentChance > 0.0)
	{
		this->HasCrit = false;
		const bool ThisbulletWasIntercepted = pBullet ? BulletExtContainer::Instance.Find(pBullet)->InterceptedStatus == InterceptedStatus::Intercepted : false;
		const float cellSpread = this->AttachedToObject->CellSpread;

		//if the warhead itself has cellspread
		if (fabs(cellSpread) >= 0.1f)
		{
			std::vector<TechnoClass*> pTargetv = Helpers::Alex::getCellSpreadItems(coords, cellSpread, true);

			std::for_each(pTargetv.begin(), pTargetv.end(), [&](TechnoClass* pTarget)
 {
	 this->DetonateOnOneUnit(pHouse, pTarget, damage, pOwner, pBullet, ThisbulletWasIntercepted);
			});

			if (this->Transact)
			{
				this->TransactOnAllUnits(pTargetv, pHouse, pOwner);
			}

		}
		else
		{
			//no cellspread but it has bullet
			if (pBullet && pBullet->Target)
			{
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
							pTech->GetTechnoType()->Trainable
							) return pTech;

							return static_cast<TechnoClass* const>(nullptr);
						};

					this->DetonateOnOneUnit(pHouse, static_cast<TechnoClass*>(pBullet->Target), damage, pOwner, pBullet, ThisbulletWasIntercepted);

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
}

void WarheadTypeExtData::DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, int damage, TechnoClass* pOwner, BulletClass* pBullet, bool bulletWasIntercepted)
{
	if (!this->CanDealDamage(pTarget))
		return;

	if (!this->CanTargetHouse(pHouse, pTarget))
		return;

	this->applyWebby(pTarget, pHouse, pOwner);

	if (!this->LimboKill_IDs.empty())
	{
		BuildingExtData::ApplyLimboKill(this->LimboKill_IDs, this->LimboKill_Affected, pTarget->Owner, pHouse);
	}

	this->ApplyShieldModifiers(pTarget);

	//if (this->RemoveDisguise)
	//	this->ApplyRemoveDisguiseToInf(pHouse, pTarget);

	//if (this->RemoveMindControl)
	//	this->ApplyRemoveMindControl(pHouse, pTarget);

	if (this->PermaMC)
		this->applyPermaMC(pHouse, pTarget);

	if (this->Crit_CurrentChance > 0.0 && (!this->Crit_SuppressOnIntercept || !bulletWasIntercepted))
	{
		this->ApplyCrit(pHouse, pTarget, pOwner);

		if (!pTarget->IsAlive)
			return;
	}

	auto pExt = TechnoExtContainer::Instance.Find(pTarget);

	if(this->PaintBallDuration.isset() && this->PaintBallData.Color != ColorStruct::Empty) {
		auto& paintball = pExt->PaintBallStates[this->AttachedToObject];
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

	if (this->GattlingStage > 0)
	{
		this->ApplyGattlingStage(pTarget, this->GattlingStage);
	}

	if (this->GattlingRateUp != 0)
	{
		this->ApplyGattlingRateUp(pTarget, this->GattlingRateUp);
	}

	if (this->ReloadAmmo != 0)
	{
		this->ApplyReloadAmmo(pTarget, this->ReloadAmmo);
	}

	if (this->Converts)
		this->ApplyUpgrade(pHouse, pTarget);

	if (this->AttachTag)
		this->ApplyAttachTag(pTarget);

	//if (this->DirectionalArmor.Get())
	//	this->ApplyDirectional(pBullet, pTarget);

	if (this->RevengeWeapon.isset() && this->RevengeWeapon_GrantDuration > 0)
		this->ApplyRevengeWeapon(pTarget);

	if (this->InflictLocomotor)
		this->ApplyLocomotorInfliction(pTarget);

	if (this->RemoveInflictedLocomotor)
		this->ApplyLocomotorInflictionReset(pTarget);

	if (this->AttachEffect_AttachTypes.size() > 0 || this->AttachEffect_RemoveTypes.size() > 0 || this->AttachEffect_RemoveGroups.size() > 0)
		this->ApplyAttachEffects(pTarget, pHouse, pOwner);
}

//void WarheadTypeExtData::DetonateOnAllUnits(HouseClass* pHouse, const CoordStruct coords, const float cellSpread, TechnoClass* pOwner)
//{
//	for (auto pTarget : Helpers::Alex::getCellSpreadItems(coords, cellSpread, true))
//	{
//		this->DetonateOnOneUnit(pHouse, pTarget, pOwner);
//	}
//}

void WarheadTypeExtData::ApplyShieldModifiers(TechnoClass* pTarget) const
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

		if (shieldIndex >= 0)
		{
			oldRatio = pExt->Shield->GetHealthRatio();
			pExt->CurrentShieldType = ShieldTypeClass::Array[0].get();
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

				if (this->Shield_ReplaceOnly && this->Shield_InheritStateOnReplace)
				{
					pExt->Shield->SetHP((int)(shieldType->Strength * oldRatio));

					if (pExt->Shield->GetHP() == 0)
						pExt->Shield->SetRespawn(shieldType->Respawn_Rate, shieldType->Respawn, shieldType->Respawn_Rate, this->Shield_Respawn_RestartTimer);
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
			pExt->Shield->BreakShield(this->Shield_BreakAnim.Get(nullptr), this->Shield_BreakWeapon.Get(nullptr));

		if (this->Shield_Respawn_Duration > 0 && this->Shield_Respawn_Types.Eligible(this->Shield_AffectTypes, pCurrentType))
			pExt->Shield->SetRespawn(this->Shield_Respawn_Duration, this->Shield_Respawn_Amount, this->Shield_Respawn_Rate, this->Shield_Respawn_RestartTimer);

		if (this->Shield_SelfHealing_Duration > 0 && this->Shield_SelfHealing_Types.Eligible(this->Shield_AffectTypes, pCurrentType))
		{
			double amount = this->Shield_SelfHealing_Amount.Get(pCurrentType->SelfHealing);
			pExt->Shield->SetSelfHealing(this->Shield_SelfHealing_Duration, amount, this->Shield_SelfHealing_Rate,
				this->Shield_SelfHealing_RestartInCombat.Get(pExt->Shield->GetType()->SelfHealing_RestartInCombat),
				this->Shield_SelfHealing_RestartInCombatDelay, this->Shield_SelfHealing_RestartTimer);
		}
	}
}

void WarheadTypeExtData::ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget) const
{
	if (const auto pController = pTarget->MindControlledBy)
		pController->CaptureManager->FreeUnit(pTarget);
}

void WarheadTypeExtData::ApplyRemoveDisguise(HouseClass* pHouse, TechnoClass* pTarget) const
{
	//this is here , just in case i need special treatment for `TankDisguiseAsTank`
	if (auto const pFoot = generic_cast<FootClass*>(pTarget))
	{
		if (pFoot->IsDisguised())
		{
			pFoot->ClearDisguise();
		}
	}
}

void WarheadTypeExtData::ApplyCrit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner)
{
	if (TechnoExtData::IsCritImmune(pTarget))
		return;

	const auto& tresh = this->Crit_GuaranteeAfterHealthTreshold.Get(pTarget);

	if (!tresh->isset() || pTarget->GetHealthPercentage() > tresh->Get())
	{
		const double dice = this->Crit_ApplyChancePerTarget ?
			ScenarioClass::Instance->Random.RandomDouble() : this->RandomBuffer;

		if (this->Crit_CurrentChance < dice)
			return;
	}

	if (auto pExt = TechnoExtContainer::Instance.Find(pTarget))
	{
		const auto pSld = pExt->Shield.get();
		if (pSld && pSld->IsActive() && pSld->GetType()->ImmuneToCrit)
			return;
	}

	if (pTarget->GetHealthPercentage() > this->Crit_AffectBelowPercent)
		return;

	if (!EnumFunctions::CanTargetHouse(this->Crit_AffectsHouses, pHouse, pTarget->GetOwningHouse()))
		return;

	if (!EnumFunctions::IsCellEligible(pTarget->GetCell(), this->Crit_Affects))
		return;

	if (!EnumFunctions::IsTechnoEligible(pTarget, this->Crit_Affects))
		return;

	this->HasCrit = true;

	if (this->Crit_AnimOnAffectedTargets && !this->Crit_AnimList.empty())
	{
		const int idx = this->AttachedToObject->EMEffect || this->Crit_AnimList_PickRandom.Get(this->AnimList_PickRandom) ?
			ScenarioClass::Instance->Random.RandomFromMax(this->Crit_AnimList.size() - 1) : 0;

		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(this->Crit_AnimList[idx], pTarget->Location),
			pHouse,
			pTarget->GetOwningHouse(),
			pOwner,
			false
		);
	}

	auto damage = this->Crit_ExtraDamage.Get();

	if (this->Crit_Warhead.isset())
		WarheadTypeExtData::DetonateAt(this->Crit_Warhead.Get(), pTarget, pOwner, damage, pHouse);
	else
		pTarget->ReceiveDamage(&damage, 0, this->AttachedToObject, pOwner, false, false, pHouse);
}

void WarheadTypeExtData::ApplyGattlingStage(TechnoClass* pTarget, int Stage) const
{
	auto pData = pTarget->GetTechnoType();
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
	auto pData = pTarget->GetTechnoType();
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
	auto pData = pTarget->GetTechnoType();
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
				if (weapon.SourceWarhead && weapon.SourceWarhead == this->AttachedToObject)
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
			pExt->RevengeWeapons.emplace_back(this->RevengeWeapon.Get(), this->RevengeWeapon_GrantDuration, this->RevengeWeapon_AffectsHouses, this->AttachedToObject);
		}
	}
}