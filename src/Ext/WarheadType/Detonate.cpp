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
#include <Misc/AresData.h>
#include <Misc/Ares/Hooks/AresNetEvent.h>
#include <Ares_TechnoExt.h>

void WarheadTypeExt::ExtData::ApplyLocomotorInfliction(TechnoClass* pTarget)
{
	auto pTargetFoot = abstract_cast<FootClass*>(pTarget);
	if (!pTargetFoot)
		return;

	// same locomotor? no point to change
	CLSID targetCLSID { };
	CLSID inflictCLSID = this->OwnerObject()->Locomotor;
	IPersistPtr pLocoPersist = pTargetFoot->Locomotor;
	if (SUCCEEDED(pLocoPersist->GetClassID(&targetCLSID)) && targetCLSID == inflictCLSID)
		return;

	// prevent endless piggyback
	IPiggybackPtr pTargetPiggy = pTargetFoot->Locomotor;
	if (pTargetPiggy != nullptr && pTargetPiggy->Is_Piggybacking())
		return;

	LocomotionClass::ChangeLocomotorTo(pTargetFoot, inflictCLSID);
}

void WarheadTypeExt::ExtData::ApplyLocomotorInflictionReset(TechnoClass* pTarget)
{
	auto pTargetFoot = abstract_cast<FootClass*>(pTarget);

	if (!pTargetFoot)
		return;

	// remove only specific inflicted locomotor if specified
	CLSID removeCLSID = this->OwnerObject()->Locomotor;
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

void WarheadTypeExt::ExtData::ApplyDirectional(BulletClass* pBullet, TechnoClass* pTarget)
{
	//if (!pBullet || pBullet->IsInAir() != pTarget->IsInAir() || pBullet->GetCell() != pTarget->GetCell() || pTarget->IsIronCurtained())
	//	return;

	//if (pTarget->WhatAmI() != AbstractType::Unit || pBullet->Type->Vertical)
	//	return;

	//const auto pTarExt = TechnoExt::ExtMap.Find(pTarget);
	//if (!pTarExt || (pTarExt->Shield && pTarExt->Shield->IsActive()))
	//	return;

	//const auto pTarType = pTarget->GetTechnoType();
	//const auto pTarTypeExt = TechnoTypeExt::ExtMap.Find(pTarType);

	//const int tarFacing = pTarget->PrimaryFacing.Current().GetValue<16>();
	//int bulletFacing = BulletExt::ExtMap.Find(pBullet)->InitialBulletDir.get().GetValue<16>();

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

void WarheadTypeExt::ExtData::applyIronCurtain(const CoordStruct& coords, HouseClass* Owner, int damage)
{
	CellStruct cellCoords = MapClass::Instance->GetCellAt(coords)->MapCoords;

	if (this->IC_Duration != 0)
	{
		// set of affected objects. every object can be here only once.
		auto items = Helpers::Alex::getCellSpreadItems(coords, this->OwnerObject()->CellSpread, true);

		// affect each object
		for (auto curTechno : items)
		{
			// don't protect the dead
			if (!curTechno || curTechno->InLimbo || !curTechno->IsAlive || !curTechno->Health)
			{
				continue;
			}

			// affects enemies or allies respectively?
			if (!this->CanAffectHouse(curTechno->Owner, Owner))
			{
				continue;
			}

			auto pType = curTechno->GetType();
			// respect verses the boolean way
			if (std::abs(this->GetVerses(pType->Armor).Verses) < 0.001)
			{
				continue;
			}

			// affect each object
			{
				// duration modifier
				int duration = this->IC_Duration;

				auto pType = curTechno->GetTechnoType();

				// modify good durations only
				if (duration > 0)
				{
					duration = static_cast<int>(duration * TechnoTypeExt::ExtMap.Find(pType)->IronCurtain_Modifier);
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
							curTechno->ReceiveDamage(&damage, 0, this->OwnerObject(), nullptr, true, false, Owner);
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

void WarheadTypeExt::ExtData::applyIronCurtain(TechnoClass* curTechno, HouseClass* Owner, int damage)
{
	if (this->IC_Duration != 0) {

		// affect each object
		{
			// duration modifier
			int duration = this->IC_Duration;

			auto pType = curTechno->GetTechnoType();

			// modify good durations only
			if (duration > 0) {
				duration = static_cast<int>(duration * TechnoTypeExt::ExtMap.Find(pType)->IronCurtain_Modifier);
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
					if (damage) {
						curTechno->ReceiveDamage(&damage, 0, this->OwnerObject(), nullptr, true, false, Owner);
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

void WarheadTypeExt::ExtData::ApplyAttachTag(TechnoClass* pTarget)
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

void WarheadTypeExt::ExtData::ApplyUpgrade(HouseClass* pHouse, TechnoClass* pTarget)
{
	TechnoTypeConvertData::ApplyConvert(this->ConvertsPair, pHouse, pTarget , this->Convert_SucceededAnim);
}

bool WarheadTypeExt::ExtData::applyPermaMC(HouseClass* const Owner, AbstractClass* const Target)
{
	if (!Owner || !this->PermaMC)
		return false;

	const auto pTargetTechno = abstract_cast<TechnoClass*>(Target);
	if (!pTargetTechno)
		return false;

	if (//!this->CanDealDamage(pTargetTechno) ||
		TechnoExt::IsPsionicsImmune(pTargetTechno))
		return false;

	if (pTargetTechno->align_154->Is_DriverKilled)
		return false;

	const auto pType = pTargetTechno->GetTechnoType();

	if (auto const pController = pTargetTechno->MindControlledBy) {
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

		if (pBld) {
			location.Z += pBld->Type->Height * Unsorted::LevelHeight;
		} else {
			location.Z += pType->MindControlRingOffset;
		}

		if (auto const pAnim = GameCreate<AnimClass>(pAnimType, location))
		{
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

void WarheadTypeExt::ExtData::applyStealMoney(TechnoClass* const Owner, TechnoClass* const Target)
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
				if ((!pBulletOwnerHouse->IsNeutral() && !HouseExt::IsObserverPlayer(pBulletOwnerHouse))
					&& (!pBulletTargetHouse->IsNeutral() && !HouseExt::IsObserverPlayer(pBulletTargetHouse)))
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

void WarheadTypeExt::ExtData::applyTransactMoney(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct const& coords)
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
					if ((!pHouse->IsNeutral() && !HouseExt::IsObserverPlayer(pHouse)) && (!pBulletTargetHouse->IsNeutral() && !HouseExt::IsObserverPlayer(pBulletTargetHouse)))
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

						if (pHouse->IsAlliedWith_(pBulletTargetHouse))
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
		if (nTransactVal != 0 && pHouse->CanTransactMoney(nTransactVal)){
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

void WarheadTypeExt::ExtData::InterceptBullets(TechnoClass* pOwner, WeaponTypeClass* pWeapon, CoordStruct coords)
{
	if (!pOwner || !pWeapon)
		return;

	const float cellSpread = this->Get()->CellSpread;

	if (cellSpread == 0.0)
	{
		if (auto const pBullet = specific_cast<BulletClass*>(pOwner->Target))
		{
			// 1/8th of a cell as a margin of error.
			if (BulletTypeExt::ExtMap.Find(pBullet->Type)->Interceptable && pBullet->Location.DistanceFrom(coords) <= Unsorted::LeptonsPerCell / 8.0)
				BulletExt::InterceptBullet(pBullet, pOwner, pWeapon);
		}
	}
	else
	{
		std::for_each(BulletClass::Array->begin(), BulletClass::Array->end(), [&](BulletClass* pTargetBullet)
 {
	 if (pTargetBullet)
	 {
		 const auto pBulletExt = BulletExt::ExtMap.Find(pTargetBullet);
		 if (pBulletExt->CurrentStrength > 0 && !pTargetBullet->InLimbo)
		 {
			 auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find(pTargetBullet->Type);
			 // Cells don't know about bullets that may or may not be located on them so it has to be this way.
			 if (pBulletTypeExt->Interceptable &&
				 pTargetBullet->Location.DistanceFrom(coords) <= (cellSpread * Unsorted::LeptonsPerCell))
				 BulletExt::InterceptBullet(pTargetBullet, pOwner, pWeapon);
		 }
	 }
		});
	}
}

bool NOINLINE IsCellSpreadWH(WarheadTypeExt::ExtData* pData)
{
	// List all Warheads here that respect CellSpread

	return //pData->RemoveDisguise ||
		//pData->RemoveMindControl ||
		pData->Crit_Chance ||
		pData->Shield_Break ||
		(pData->Converts && !pData->ConvertsPair.empty()) ||
		pData->Shield_Respawn_Duration > 0 ||
		pData->Shield_SelfHealing_Duration > 0 ||
		pData->Shield_AttachTypes.size() > 0 ||
		pData->Shield_RemoveTypes.size() > 0 ||
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
		;
}

void WarheadTypeExt::ExtData::Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct coords, int damage)
{
	VocClass::PlayIndexAtPos(Sound, coords);
	if (!this->DetonateParticleSystem.empty()) {
		for (auto const& pSys : this->DetonateParticleSystem) {
			GameCreate<ParticleSystemClass>(pSys, coords, nullptr, pOwner, CoordStruct::Empty, pHouse);
		}
	}

	if (!pBullet)
	{
		for (auto const& pWeapon : this->DetonatesWeapons)
		{
			WeaponTypeExt::DetonateAt(pWeapon, coords, pOwner, true , pHouse);
		}
	}

	if (pOwner && pBullet)
	{
		if(pOwner->IsAlive && pBullet->IsAlive)
			if (TechnoExt::ExtMap.Find(pOwner)->IsInterceptor() && BulletExt::ExtMap.Find(pBullet)->IsInterceptor)
				this->InterceptBullets(pOwner, pBullet->WeaponType, coords);

		//TechnoExt::PutPassengersInCoords(pBullet->Owner, coords, RulesGlobal->WarpIn, RulesGlobal->BunkerWallsUpSound, false);
	}


	if (pHouse)
	{
		if (this->BigGap)
		{
			std::for_each(HouseClass::Array->begin(), HouseClass::Array->end(), [&](HouseClass* pOtherHouse)
 {
	 if (pOtherHouse->IsControlledByHuman() &&	  // Not AI
		 !HouseExt::IsObserverPlayer(pOtherHouse) &&		  // Not Observer
		 !pOtherHouse->Defeated &&			  // Not Defeated
		 pOtherHouse != pHouse &&			  // Not pThisHouse
		 !pHouse->IsAlliedWith_(pOtherHouse))   // Not Allied
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

	this->HasCrit = false;
	this->RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();
	//const bool ISPermaMC = this->PermaMC && !pBullet;

	if (IsCellSpreadWH(this))
	{
		const bool ThisbulletWasIntercepted = pBullet ? BulletExt::ExtMap.Find(pBullet)->InterceptedStatus == InterceptedStatus::Intercepted : false;
		const float cellSpread = Get()->CellSpread;

		//if the warhead itself has cellspread
		if (fabs(cellSpread) >= 0.1f)
		{
			std::vector<TechnoClass*> pTargetv = Helpers::Alex::getCellSpreadItems(coords, cellSpread, true);

			std::for_each(pTargetv.begin(), pTargetv.end(), [&](TechnoClass* pTarget) {
				 this->DetonateOnOneUnit(pHouse, pTarget, damage, pOwner, pBullet, ThisbulletWasIntercepted);
			});

			if (this->Transact) {
				this->TransactOnAllUnits(pTargetv, pHouse, pOwner);
			}

			pTargetv.clear();

		}
		else
		{
			//no cellspread but it has bullet
			if (pBullet && pBullet->Target)
			{
				switch (GetVtableAddr(pBullet->Target))
				{
				case BuildingClass::vtable:
				case AircraftClass::vtable:
				case UnitClass::vtable:
				case InfantryClass::vtable:
				{
					const auto Eligible = [&](TechnoClass* const pTech)
					{
						if (CanDealDamage(pTech) &&
						CanTargetHouse(pHouse, pTech) &&
						pTech->GetTechnoType()->Trainable
						) return pTech;

						return static_cast<TechnoClass* const>(nullptr);
					};

					this->DetonateOnOneUnit(pHouse, static_cast<TechnoClass*>(pBullet->Target), damage, pOwner,  pBullet, ThisbulletWasIntercepted);

					if (this->Transact)
						this->TransactOnOneUnit(Eligible(static_cast<TechnoClass*>(pBullet->Target)), pOwner, 1);

				}break;
				case CellClass::vtable:
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

void WarheadTypeExt::ExtData::DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, int damage, TechnoClass* pOwner, BulletClass* pBullet, bool bulletWasIntercepted)
{
	if (!this->CanDealDamage(pTarget))
		return;

	if (!this->CanTargetHouse(pHouse, pTarget))
		return;

	if (!this->LimboKill_IDs.empty()) {
		BuildingExt::ApplyLimboKill(this->LimboKill_IDs, this->LimboKill_Affected, pTarget->Owner, pHouse);
	}

	this->ApplyShieldModifiers(pTarget);

	//if (this->RemoveDisguise)
	//	this->ApplyRemoveDisguiseToInf(pHouse, pTarget);

	//if (this->RemoveMindControl)
	//	this->ApplyRemoveMindControl(pHouse, pTarget);

	if (this->PermaMC)
		this->applyPermaMC(pHouse, pTarget);

	if (this->Crit_Chance && (!this->Crit_SuppressOnIntercept || !bulletWasIntercepted)){
		this->ApplyCrit(pHouse, pTarget, pOwner);

		if (!pTarget->IsAlive)
			return;
	}

	auto pExt = TechnoExt::ExtMap.Find(pTarget);

	if (pExt->PaintBallState.get())
		pExt->PaintBallState->Enable(this->PaintBallDuration.Get(), PaintBallData, this->Get());

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
}

//void WarheadTypeExt::ExtData::DetonateOnAllUnits(HouseClass* pHouse, const CoordStruct coords, const float cellSpread, TechnoClass* pOwner)
//{
//	for (auto pTarget : Helpers::Alex::getCellSpreadItems(coords, cellSpread, true))
//	{
//		this->DetonateOnOneUnit(pHouse, pTarget, pOwner);
//	}
//}

void WarheadTypeExt::ExtData::ApplyShieldModifiers(TechnoClass* pTarget)
{
	if (!pTarget)
		return;

	auto pExt = TechnoExt::ExtMap.Find(pTarget);


		int shieldIndex = -1;
		double oldRatio = 1.0;

		// Remove shield.
		if (pExt->GetShield()) {



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
				if (shieldIndex >= 0){
					const int nMax = (Shield_AttachTypes.size() - 1);
					shieldType = Shield_AttachTypes[MinImpl(shieldIndex, nMax) ];
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

void WarheadTypeExt::ExtData::ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (const auto pController = pTarget->MindControlledBy)
		pController->CaptureManager->FreeUnit(pTarget);
}

void WarheadTypeExt::ExtData::ApplyRemoveDisguise(HouseClass* pHouse, TechnoClass* pTarget)
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

void WarheadTypeExt::ExtData::ApplyCrit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner)
{
	if (TechnoExt::IsCritImmune(pTarget))
		return;

	const auto& tresh = this->Crit_GuaranteeAfterHealthTreshold.Get(pTarget);

	if(!tresh->isset() || pTarget->GetHealthPercentage() > tresh->Get()){
		const double dice = this->Crit_ApplyChancePerTarget ?
			ScenarioClass::Instance->Random.RandomDouble() : this->RandomBuffer;

		if (this->Crit_Chance < dice)
			return;
	}

	if (auto pExt = TechnoExt::ExtMap.Find(pTarget))
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

	if (this->Crit_AnimOnAffectedTargets && this->Crit_AnimList.size())
	{
		const int idx = this->Get()->EMEffect || this->Crit_AnimList_PickRandom.Get(this->AnimList_PickRandom) ?
			ScenarioClass::Instance->Random.RandomFromMax(this->Crit_AnimList.size() - 1) : 0;

		if (auto  pAnim = GameCreate<AnimClass>(this->Crit_AnimList[idx], pTarget->Location))
		{
			AnimExt::SetAnimOwnerHouseKind(pAnim, pHouse, pTarget->GetOwningHouse(), pOwner, false);
		}
	}

	auto damage = this->Crit_ExtraDamage.Get();

	if (this->Crit_Warhead.isset())
		WarheadTypeExt::DetonateAt(this->Crit_Warhead.Get(), pTarget, pOwner, damage , pHouse);
	else
		pTarget->ReceiveDamage(&damage, 0, this->Get(), pOwner, false, false, pHouse);
}

void WarheadTypeExt::ExtData::ApplyGattlingStage(TechnoClass* pTarget, int Stage)
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

void WarheadTypeExt::ExtData::ApplyGattlingRateUp(TechnoClass* pTarget, int RateUp)
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

void WarheadTypeExt::ExtData::ApplyReloadAmmo(TechnoClass* pTarget, int ReloadAmount)
{
	auto pData = pTarget->GetTechnoType();
	if (pData->Ammo > 0)
	{
		auto const ammo = pTarget->Ammo + ReloadAmount;
		pTarget->Ammo = std::clamp(ammo, 0, pData->Ammo);
	}
}

void WarheadTypeExt::ExtData::ApplyRevengeWeapon(TechnoClass* pTarget)
{
	auto const maxCount = this->RevengeWeapon_MaxCount;
	if (!maxCount)
		return;

	int count = 0;

	if (auto const pExt = TechnoExt::ExtMap.Find(pTarget))
	{
		if (!this->RevengeWeapon_Cumulative)
		{
			for (auto& weapon : pExt->RevengeWeapons)
			{
				// If it is same weapon just refresh timer.
				if (weapon.SourceWarhead && weapon.SourceWarhead == this->Get())
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
			pExt->RevengeWeapons.emplace_back(this->RevengeWeapon.Get(), this->RevengeWeapon_GrantDuration, this->RevengeWeapon_AffectsHouses, this->Get());
		}
	}
}