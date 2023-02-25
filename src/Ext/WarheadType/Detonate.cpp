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

#include <New/Entity/VerticalLaserClass.h>
#include <Misc/InteractWithAres/Body.h>

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
	if (this->Converts_From.size() && this->Converts_To.size())
	{
		// explicitly unsigned because the compiler wants it
		for (unsigned int i = 0; i < this->Converts_From.size(); i++)
		{
			// Check if the target matches upgrade-from TechnoType and it has something to upgrade-to
			if (this->Converts_To.size() >= i && this->Converts_From[i] == pTarget->GetTechnoType())
			{
				//TechnoTypeClass* pResultType = this->Converts_To[i];
				//auto pCurType = pTarget->GetTechnoType();
				//if (pCurType != pResultType)
					//AresData::HandleConvert::Exec(pTarget, pResultType);
			}
		}
	}
}

void WarheadTypeExt::ExtData::applyPermaMC(HouseClass* const Owner, AbstractClass* const Target)
{
	if (Owner)
	{
		if (auto const pTarget = abstract_cast<TechnoClass*>(Target))
		{
			auto const pType = pTarget->GetTechnoType();

			if (pType && !pType->ImmuneToPsionics)
			{
				if (auto const pController = pTarget->MindControlledBy)
				{
					pController->CaptureManager->FreeUnit(pTarget);
				}

				auto const pOriginalOwner = pTarget->GetOwningHouse();
				pTarget->SetOwningHouse(Owner, true);
				pTarget->MindControlledByAUnit = true;
				pTarget->QueueMission(Mission::Guard, false);
				pTarget->OriginallyOwnedByHouse = pOriginalOwner;

				if (auto& pAnim = pTarget->MindControlRingAnim)
				{
					pAnim->UnInit();
					pAnim = nullptr;
				}

				auto const pBld = abstract_cast<BuildingClass*>(pTarget);

				CoordStruct location = pTarget->GetCoords();

				if (pBld)
				{
					location.Z += pBld->Type->Height * Unsorted::LevelHeight;
				}
				else
				{
					location.Z += pType->MindControlRingOffset;
				}

				if (auto const pAnimType = RulesClass::Instance->PermaControlledAnimationType)
				{
					if (auto const pAnim = GameCreate<AnimClass>(pAnimType, location))
					{
						pTarget->MindControlRingAnim = pAnim;
						pAnim->SetOwnerObject(pTarget);
						if (pBld)
						{
							pAnim->ZAdjust = -1024;
						}
					}
				}
			}
		}
	}
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
								bSucceed = pHouse->TransactMoney(TransactMoney);
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
									bSucceed = pBulletTargetHouse->TransactMoney(nTransactVal);
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
									bSucceed = pBulletTargetHouse->TransactMoney(nTransactVal);
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
			bSucceed = pHouse->TransactMoney(TransactMoney);

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
		std::for_each(BulletClass::Array->begin(), BulletClass::Array->end(), [&](BulletClass* pTargetBullet) {
			if(pTargetBullet){
				const auto pBulletExt = BulletExt::ExtMap.Find(pTargetBullet);
				if (pBulletExt->CurrentStrength > 0 && !pTargetBullet->InLimbo) {
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

void WarheadTypeExt::ExtData::Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct coords)
{
	//if (pBullet && pBullet->WeaponType && (pBullet->WeaponType->IsLaser) && pOwner) {

	//	const auto it = std::find_if(VerticalLaserClass::Array.begin(), VerticalLaserClass::Array.end(), [pOwner](auto const pData) { return pData->Owner == pOwner;  });
	//	if(VerticalLaserClass::Array.empty() || it == VerticalLaserClass::Array.end()){

	//		int nHeight = 0;
	//		switch (pBullet->Target->WhatAmI())
	//		{
	//		case AbstractType::Building:
	//		case AbstractType::Infantry:
	//		case AbstractType::Aircraft:
	//		case AbstractType::Unit:
	//			nHeight = static_cast<TechnoClass*>(pBullet->Target)->GetHeight();
	//			break;
	//		default:
	//			nHeight = Map.GetCellFloorHeight(coords);
	//			break;
	//		}

	//		if (auto pLaser = GameCreate<VerticalLaserClass>(pBullet->WeaponType, coords, nHeight))
	//			pLaser->Owner = pOwner;
	//	}
	//}

	auto const nSound = Sound.Get();
	if (nSound != -1)
		VocClass::PlayAt(nSound, coords);

	if (!pBullet) {
		for (auto const& pWeapon : this->DetonatesWeapons) {
			WeaponTypeExt::DetonateAt(pWeapon, coords, pOwner);
		}
	}

	if (pOwner && pBullet) {
		if (TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType())->Interceptor && BulletExt::ExtMap.Find(pBullet)->IsInterceptor)
			this->InterceptBullets(pOwner, pBullet->WeaponType, coords);

		//TechnoExt::PutPassengersInCoords(pBullet->Owner, coords, RulesGlobal->WarpIn, RulesGlobal->BunkerWallsUpSound, false);
	}


	if (pHouse) {
		if (this->BigGap) {
			std::for_each(HouseClass::Array->begin(), HouseClass::Array->end(), [&](HouseClass* pOtherHouse) {
				if (pOtherHouse->IsControlledByHuman() &&	  // Not AI
					!HouseExt::IsObserverPlayer(pOtherHouse) &&		  // Not Observer
					!pOtherHouse->Defeated &&			  // Not Defeated
					pOtherHouse != pHouse &&			  // Not pThisHouse
					!pHouse->IsAlliedWith(pOtherHouse))   // Not Allied
					{ pOtherHouse->ReshroudMap(); }
			});
		}

		if (this->SpySat)
			MapClass::Instance->Reveal(pHouse);

		this->applyTransactMoney(pOwner, pHouse, pBullet, coords);
	}

	this->HasCrit = false;
	this->RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();
	bool ISPermaMC = this->PermaMC && !pBullet;

	// List all Warheads here that respect CellSpread
	const bool isCellSpreadWarhead =
		this->RemoveDisguise ||
		this->RemoveMindControl ||
		this->Crit_Chance ||
		this->Shield_Break ||
		this->Converts ||
		this->Shield_Respawn_Duration > 0 ||
		this->Shield_SelfHealing_Duration > 0 ||
		this->Shield_AttachTypes.size() > 0 ||
		this->Shield_RemoveTypes.size() > 0 ||
		this->Transact || ISPermaMC ||
		this->GattlingStage > 0 ||
		this->GattlingRateUp != 0 ||
		this->AttachTag ||
		this->DirectionalArmor ||
		this->ReloadAmmo != 0 ||
		(this->RevengeWeapon.isset() && this->RevengeWeapon_GrantDuration > 0) ||
		!this->LimboKill_IDs.empty()
#ifdef COMPILE_PORTED_DP_FEATURES
		|| (this->PaintBallData.Color != ColorStruct::Empty)
#endif
		;

	if (isCellSpreadWarhead)
	{
		bool ThisbulletWasIntercepted = false;
		if (pBullet) {
			ThisbulletWasIntercepted = BulletExt::ExtMap.Find(pBullet)->InterceptedStatus == InterceptedStatus::Intercepted;
		}

		const float cellSpread = Get()->CellSpread;

		//if the warhead itself has cellspread
		if (fabs(cellSpread) >= 0.1f)
		{
			std::vector<TechnoClass*> pTargetv = Helpers::Alex::getCellSpreadItems(coords, cellSpread, true);

			std::for_each(pTargetv.begin(), pTargetv.end(), [&](TechnoClass* pTarget) {
				this->DetonateOnOneUnit(pHouse, pTarget, pOwner, pBullet, ThisbulletWasIntercepted);
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
				switch (pBullet->Target->WhatAmI())
				{
				case AbstractType::Building:
				case AbstractType::Aircraft:
				case AbstractType::Unit:
				case AbstractType::Infantry:
				{
					const auto Eligible = [&](TechnoClass* const pTech)
					{
						if (CanDealDamage(pTech) &&
						CanTargetHouse(pHouse, pTech) &&
						pTech->GetTechnoType()->Trainable
						) return pTech;

						return static_cast<TechnoClass* const>(nullptr);
					};

					this->DetonateOnOneUnit(pHouse, static_cast<TechnoClass*>(pBullet->Target), pOwner, pBullet, ThisbulletWasIntercepted);

					if (this->Transact)
						this->TransactOnOneUnit(Eligible(static_cast<TechnoClass*>(pBullet->Target)), pOwner, 1);

				}break;
				case AbstractType::Cell:
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

void WarheadTypeExt::ExtData::DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner, BulletClass* pBullet , bool bulletWasIntercepted)
{
	if (!this->CanDealDamage(pTarget))
		return;

	if (!this->CanTargetHouse(pHouse, pTarget))
		return;

	if (!this->LimboKill_IDs.empty()) {
		BuildingExt::ApplyLimboKill(this->LimboKill_IDs, this->LimboKill_Affected, pTarget->Owner, pHouse);
	}

	this->ApplyShieldModifiers(pTarget);

	if (this->RemoveDisguise)
		this->ApplyRemoveDisguiseToInf(pHouse, pTarget);

	if (this->RemoveMindControl)
		this->ApplyRemoveMindControl(pHouse, pTarget);

	if (this->PermaMC && !pBullet)
		this->applyPermaMC(pHouse, pTarget);

	if (this->Crit_Chance && (!this->Crit_SuppressOnIntercept || !bulletWasIntercepted))
		this->ApplyCrit(pHouse, pTarget, pOwner);

#ifdef COMPILE_PORTED_DP_FEATURES
	auto pExt = TechnoExt::ExtMap.Find(pTarget);

	if (pExt->PaintBallState.get())
		pExt->PaintBallState->Enable(this->PaintBallDuration.Get(), PaintBallData, this->Get());
#endif

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

#ifdef Ares_3_0_p1
	if (this->Converts && AresData::AresDllHmodule != nullptr)
		this->ApplyUpgrade(pHouse, pTarget);
#endif

	if (this->AttachTag)
		this->ApplyAttachTag(pTarget);

	if (this->DirectionalArmor.Get())
		this->ApplyDirectional(pBullet, pTarget);

	if (this->RevengeWeapon.isset() && this->RevengeWeapon_GrantDuration > 0)
		this->ApplyRevengeWeapon(pTarget);
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
	auto pExt = TechnoExt::ExtMap.Find(pTarget);

	{
		int shieldIndex = -1;
		double ratio = 1.0;

		// Remove shield.
		if (pExt->GetShield())
		{
			const auto shieldType = pExt->Shield->GetType();
			shieldIndex = this->Shield_RemoveTypes.IndexOf(shieldType);

			if (shieldIndex >= 0)
			{
				ratio = pExt->Shield->GetHealthRatio();
				pExt->CurrentShieldType = ShieldTypeClass::FindOrAllocate(DEFAULT_STR2);
				pExt->Shield->KillAnim();
				pExt->Shield = nullptr;
			}
		}

		// Attach shield.
		if (Shield_AttachTypes.size() > 0)
		{
			ShieldTypeClass* shieldType = nullptr;

			if (this->Shield_ReplaceOnly)
			{
				if (shieldIndex >= 0)
					shieldType = Shield_AttachTypes[Math::min(shieldIndex, (signed)Shield_AttachTypes.size() - 1)];
			}
			else
			{
				shieldType = Shield_AttachTypes.size() > 0 ? Shield_AttachTypes[0] : nullptr;
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
						pExt->Shield->SetHP((int)(shieldType->Strength * ratio));

						if (pExt->Shield->GetHP() == 0)
							pExt->Shield->SetRespawn(shieldType->Respawn_Rate, shieldType->Respawn, shieldType->Respawn_Rate, true);
					}
				}
			}
		}

		// Apply other modifiers.
		if (pExt->Shield)
		{
			if (this->Shield_AffectTypes.size() > 0 && !this->Shield_AffectTypes.Contains(pExt->Shield->GetType()))
				return;

			if (this->Shield_Break && pExt->Shield->IsActive())
				pExt->Shield->BreakShield(this->Shield_BreakAnim.Get(nullptr), this->Shield_BreakWeapon.Get(nullptr));

			if (this->Shield_Respawn_Duration > 0)
				pExt->Shield->SetRespawn(this->Shield_Respawn_Duration, this->Shield_Respawn_Amount, this->Shield_Respawn_Rate, this->Shield_Respawn_ResetTimer);

			if (this->Shield_SelfHealing_Duration > 0)
			{
				double amount = this->Shield_SelfHealing_Amount.Get(pExt->Shield->GetType()->SelfHealing);
				pExt->Shield->SetSelfHealing(this->Shield_SelfHealing_Duration, amount, this->Shield_SelfHealing_Rate, this->Shield_SelfHealing_ResetTimer);
			}
		}
	}
}

void WarheadTypeExt::ExtData::ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (auto pController = pTarget->MindControlledBy)
		pTarget->MindControlledBy->CaptureManager->FreeUnit(pTarget);
}

void WarheadTypeExt::ExtData::ApplyRemoveDisguiseToInf(HouseClass* pHouse, TechnoClass* pTarget)
{
	//this is here , just in case i need special treatment for `TankDisguiseAsTank`
	if (auto const pFoot = generic_cast<FootClass*>(pTarget)) {
		if (pFoot->IsDisguised()) {
			pFoot->ClearDisguise();
		}
	}
}

void WarheadTypeExt::ExtData::ApplyCrit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner)
{
	double dice;

	if (this->Crit_ApplyChancePerTarget)
		dice = ScenarioClass::Instance->Random.RandomDouble();
	else
		dice = this->RandomBuffer;

	if (this->Crit_Chance < dice)
		return;

	if (auto pExt = TechnoExt::ExtMap.Find(pTarget))
	{
		const auto pSld = pExt->Shield.get();
		if (pSld && pSld->IsActive() && pSld->GetType()->ImmuneToCrit)
			return;
	}

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType());
	
	{
		if (pTypeExt->ImmuneToCrit)
			return;

		if (pTarget->GetHealthPercentage() > this->Crit_AffectBelowPercent)
			return;
	}

	if (!EnumFunctions::CanTargetHouse(this->Crit_AffectsHouses, pHouse, pTarget->GetOwningHouse()))
		return;

	if (!EnumFunctions::IsCellEligible(pTarget->GetCell(), this->Crit_Affects))
		return;

	if (!EnumFunctions::IsTechnoEligible(pTarget, this->Crit_Affects))
		return;

	this->HasCrit = true;

	if (this->Crit_AnimOnAffectedTargets && this->Crit_AnimList.size())
	{
		int idx = this->Get()->EMEffect || this->Crit_AnimList_PickRandom.Get(this->AnimList_PickRandom) ?
			ScenarioClass::Instance->Random.RandomRanged(0, this->Crit_AnimList.size() - 1) : 0;

		if (auto  pAnim = GameCreate<AnimClass>(this->Crit_AnimList[idx], pTarget->Location)) {
			AnimExt::SetAnimOwnerHouseKind(pAnim, pHouse, pTarget->GetOwningHouse(), pOwner, false);
		}
	}

	auto damage = this->Crit_ExtraDamage.Get();

	if (this->Crit_Warhead.isset())
		WarheadTypeExt::DetonateAt(this->Crit_Warhead.Get(), pTarget, pOwner, damage);
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
			pTarget->GattlingAudioPlayed = true;
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
		pTarget->Ammo = Math::clamp(ammo, 0, pData->Ammo);
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

					weapon.Timer.Start(Math::max(nDur, nTime));
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