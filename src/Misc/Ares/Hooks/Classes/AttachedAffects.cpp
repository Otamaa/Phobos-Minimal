#include "AttachedAffects.h"

#include <Misc/Ares/Hooks/Header.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <New/PhobosAttachedAffect/PhobosAttachEffectTypeClass.h>

void AresAE::RecalculateStat(AresAEData* ae, TechnoClass* pThis)
{
	double ROF_Mult = 1.0;
	double ReceiveRelativeDamageMult = 1.0;
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	double FP_Mult = pExt->AE_FirePowerMult;
	double Armor_Mult = pExt->AE_ArmorMult;
	double Speed_Mult = pExt->AE_SpeedMult;
	bool Cloak = pThis->GetTechnoType()->Cloakable || pThis->HasAbility(AbilityType::Cloak);
	bool forceDecloak = false;
	bool disableWeapons = false;
	bool disableSelfHeal = false;
	bool untrackable = TechnoExtData::IsUntrackable(pThis);
	auto extraRangeData = &pExt->AE_ExtraRange;
	auto extraCritData = &pExt->AE_ExtraCrit;

	extraRangeData->Clear();
	extraCritData->Clear();

	std::optional<double> cur_timerAE {};

	for (const auto& aeData : ae->Data)
	{
		if (aeData.Type->ROFMultiplier_ApplyOnCurrentTimer) {
			if (!cur_timerAE.has_value())
				cur_timerAE = aeData.Type->ROFMultiplier;
			else
				cur_timerAE.value() *= aeData.Type->ROFMultiplier;
		}

		ROF_Mult *= aeData.Type->ROFMultiplier;
		ReceiveRelativeDamageMult += aeData.Type->ReceiveRelativeDamageMult;
		FP_Mult *= aeData.Type->FirepowerMultiplier;
		Speed_Mult *= aeData.Type->SpeedMultiplier;
		Armor_Mult *= aeData.Type->ArmorMultiplier;
		Cloak |= aeData.Type->Cloakable;
		forceDecloak |= aeData.Type->ForceDecloak;
		disableWeapons |= aeData.Type->DisableWeapons;
		disableSelfHeal |= aeData.Type->DisableSelfHeal;
		untrackable |= aeData.Type->Untrackable;

		if(!(aeData.Type->WeaponRange_Multiplier == 1.0 && aeData.Type->WeaponRange_ExtraRange == 0.0)){

			auto& ranges_ = extraRangeData->ranges.emplace_back();
			ranges_.rangeMult = aeData.Type->WeaponRange_Multiplier;
			ranges_.extraRange = aeData.Type->WeaponRange_ExtraRange * Unsorted::LeptonsPerCell;

			for (auto& allow : aeData.Type->WeaponRange_AllowWeapons)
				extraRangeData->allow.push_back_unique(allow);

			for (auto& disallow : aeData.Type->WeaponRange_DisallowWeapons)
				extraRangeData->disallow.push_back_unique(disallow);
		}
	}

	for (const auto& attachEffect : pExt->PhobosAE)
	{
		if (!attachEffect || !attachEffect->IsActive())
			continue;

		auto const type = attachEffect->GetType();
		FP_Mult *= type->FirepowerMultiplier;
		Speed_Mult *= type->SpeedMultiplier;
		Armor_Mult *= type->ArmorMultiplier;
		ROF_Mult *= type->ROFMultiplier;
		Cloak |= type->Cloakable;
		forceDecloak |= type->ForceDecloak;
		disableWeapons |= type->DisableWeapons;
		disableSelfHeal |= type->DisableSelfHeal;
		untrackable |= type->Untrackable;
		ReceiveRelativeDamageMult += type->ReceiveRelativeDamageMult;

		if (type->ROFMultiplier_ApplyOnCurrentTimer)
		{
			if (!cur_timerAE.has_value())
				cur_timerAE = type->ROFMultiplier;
			else
				cur_timerAE.value() *= type->ROFMultiplier;
		}

		if (!(type->WeaponRange_Multiplier == 1.0 && type->WeaponRange_ExtraRange == 0.0))
		{
			auto& ranges_ = extraRangeData->ranges.emplace_back();
			ranges_.rangeMult = type->WeaponRange_Multiplier;
			ranges_.extraRange = type->WeaponRange_ExtraRange * Unsorted::LeptonsPerCell;

			for (auto& allow : type->WeaponRange_AllowWeapons)
				extraRangeData->allow.push_back_unique(allow);

			for (auto& disallow : type->WeaponRange_DisallowWeapons)
				extraRangeData->disallow.push_back_unique(disallow);
		}

		if (!(type->Crit_Multiplier == 1.0 && type->Crit_ExtraChance == 0.0))
		{
			auto& ranges_ = extraCritData->ranges.emplace_back();
			ranges_.Mult = type->Crit_Multiplier;
			ranges_.extra = type->Crit_ExtraChance;

			for (auto& allow : type->Crit_AllowWarheads)
				extraCritData->allow.push_back_unique(allow);

			for (auto& disallow : type->Crit_DisallowWarheads)
				extraCritData->disallow.push_back_unique(disallow);
		}
	}

	if (cur_timerAE.has_value() && cur_timerAE > 0.0) {
		const int timeleft = pExt->AttachedToObject->DiskLaserTimer.GetTimeLeft();

		if (timeleft > 0) {
			pExt->AttachedToObject->DiskLaserTimer.Start(int(timeleft * cur_timerAE.value()));
		}

		pThis->ROF = static_cast<int>(pThis->ROF * ROF_Mult);
	}

	pThis->FirepowerMultiplier = FP_Mult;
	pThis->ArmorMultiplier = Armor_Mult;
	pExt->AE_ROF = ROF_Mult;
	pExt->AE_ReceiveRelativeDamageMult = ReceiveRelativeDamageMult;
	pThis->Cloakable = Cloak;
	pExt->AE_ForceDecloak = forceDecloak;
	pExt->AE_DisableWeapons = disableWeapons;
	pExt->AE_DisableSelfHeal = disableSelfHeal;
	pExt->AE_Untrackable = untrackable;

	if (pThis->AbstractFlags & AbstractFlags::Foot) {
		((FootClass*)pThis)->SpeedMultiplier = Speed_Mult;
	}
}

#include <Ext/WarheadType/Body.h>

void AresAE::applyAttachedEffect(WarheadTypeClass* pWH, const CoordStruct& coords, HouseClass* Source)
{
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWHExt->AttachedEffect.Duration != 0)
	{
		// set of affected objects. every object can be here only once.
		const auto items = Helpers::Alex::getCellSpreadItems(coords, pWH->CellSpread, true);

		// affect each object
		for (const auto curTechno : items)
		{
			if (Source && !pWHExt->CanAffectHouse(curTechno->Owner, Source))
			{
				continue;
			}

			if (std::abs(pWHExt->GetVerses(TechnoExtData::GetTechnoArmor(curTechno , pWH)).Verses) < 0.001)
			{
				continue;
			}

			AresAE::Attach(&pWHExt->AttachedEffect, curTechno, pWHExt->AttachedEffect.Duration, Source);
		}
	}
}

AresAttachEffectTypeClass* GetAETypeFromTechnoType(TechnoTypeClass* pType)
{
	return std::addressof(TechnoTypeExtContainer::Instance.Find(pType)->AttachedEffect);
}

AresAEData* GetAEDAtaFromTechno(TechnoClass* pThis)
{
	return std::addressof(TechnoExtContainer::Instance.Find(pThis)->AeData); //dummy
}

void AresAE::UpdateTempoal(AresAEData* ae, TechnoClass* pTechno)
{
	if (!ae->NeedToRecreateAnim)
	{
		ae->NeedToRecreateAnim = true;
		for (auto& ae_ : ae->Data) {
			if(ae_.Type->TemporalHidesAnim) {
				ae_.ClearAnim();
			}
		}
	}
}

void AresAE::Update(AresAEData* ae, TechnoClass* pTechno)
{
	if (pTechno->InLimbo || pTechno->IsImmobilized || pTechno->Transporter)
		return;

	const auto pType = pTechno->GetTechnoType();

	if (!ae->Data.empty())
	{
		const auto state = pTechno->CloakState;

		if (state == CloakState::Cloaked || state == CloakState::Cloaking) {
			if (!ae->NeedToRecreateAnim) {
				for (auto& ae_ : ae->Data) {
					ae_.ClearAnim();
				}

				ae->NeedToRecreateAnim = true;
			}
		}
		else if (ae->NeedToRecreateAnim)
		{
			for (auto& ae_ : ae->Data)
			{
				ae_.CreateAnim(pTechno);
			}

			ae->NeedToRecreateAnim = false;
		}

		for (auto& ae_ : ae->Data)
		{
			auto const pEffectType = ae_.Type;
			auto duration = ae_.Duration;

			auto const isOwnType = (pEffectType->Owner == pType);
			if (isOwnType && pTechno->Deactivated) {
				duration = 0;
			}

			if (duration > 0) {
				--duration;
			}

			if (!duration) {
				if (isOwnType) {
					ae->Isset = false;
					ae->InitialDelay = pEffectType->Delay;
				}
			}

			ae_.Duration = duration;
		}

		auto Iter = std::remove_if(ae->Data.begin(), ae->Data.end(), [](const auto& ae_) {
			return !ae_.Duration;
		});

		if (Iter != ae->Data.end())
		{
			ae->Data.erase(Iter, ae->Data.end());
			RecalculateStat(ae, pTechno);
		}
	}

	if (!ae->Isset)
	{
		const auto pAETypeType = GetAETypeFromTechnoType(pType);
		auto dur = pAETypeType->Duration;

		if (dur)
		{
			int delay = ae->InitialDelay;
			if (delay == 0x7FFFFFFF) {
				delay = pAETypeType->InitialDelay;
			}

			if (delay)
			{
				if (delay > 0)
					ae->InitialDelay = delay - 1;

			}
			else if (!pTechno->Deactivated)
			{
				if (AresAE::Attach(pAETypeType, pTechno, dur, pTechno->Owner))
					ae->Isset = true;
			}
		}
		else
		{
			ae->Isset = 2;// typo ?
		}
	}
}

bool AresAE::Remove(AresAEData* ae)
{
	if (!ae->Data.empty())
	{
		ae->NeedToRecreateAnim = true;
		for (auto& ae_ : ae->Data)
			ae_.ClearAnim();

		auto const it = std::remove_if(
			ae->Data.begin(), ae->Data.end(),
			[](auto const& Item)
		{
			return static_cast<bool>(Item.Type->DiscardOnEntry);
		});

		if (it != ae->Data.end())
		{
			ae->Data.erase(it, ae->Data.end());
			return true;
		}
	}

	return false;
}

void AresAE::Remove(AresAEData* ae, TechnoClass* pTechno)
{
	if (AresAE::Remove(ae))
		RecalculateStat(ae, pTechno);
}

void AresAE::RemoveSpecific(AresAEData* ae, TechnoClass* pTechno, AbstractTypeClass* pRemove)
{
	ae->Isset = 0;

	if (!ae->Data.empty())
	{
		const auto iter = std::remove_if(ae->Data.begin(), ae->Data.end(), [pRemove](const auto& ae_)
		{
			return ae_.Type->Owner == pRemove;
		});

		if (iter != ae->Data.end())
		{
			ae->Data.erase(iter , ae->Data.end());
			RecalculateStat(ae, pTechno);
		}
	}
}

bool AresAE::Attach(AresAttachEffectTypeClass* pType, TechnoClass* pTargetTechno, int duration, HouseClass* pInvokerOwner)
{
	if (!pType->PenetratesIC && pTargetTechno->IsIronCurtained())
		return false;

	auto pData = GetAEDAtaFromTechno(pTargetTechno);

	if (!pType->Cumulative)
	{
		auto const it = std::find_if(pData->Data.begin(), pData->Data.end(),
			[=](auto const& item) { return item.Type == pType; });

		if (it != pData->Data.end())
		{

			it->Duration =it->Type->Duration;

			if (pType->AnimType && pType->AnimResetOnReapply) {
				it->CreateAnim(pTargetTechno);
			}

			if (pType->ForceDecloak)
			{
				auto const state = pTargetTechno->CloakState;
				if (state == CloakState::Cloaked
					|| state == CloakState::Cloaking)
				{
					pTargetTechno->Uncloak(true);
				}
			}

			return true;
		}
	}

	auto& crated = pData->Data.emplace_back();
	crated.Type = pType;
	crated.Duration = duration;
	crated.Invoker = pInvokerOwner;
	crated.CreateAnim(pTargetTechno);
	RecalculateStat(pData, pTargetTechno);

	if (pType->ForceDecloak)
	{
		if (pTargetTechno->CloakState == CloakState::Cloaked || pTargetTechno->CloakState == CloakState::Cloaking)
		{
			pTargetTechno->Uncloak(true);
		}
	}

	return true;
}

void AresAE::TransferAttachedEffects(TechnoClass* From, TechnoClass* To)
{
	auto FromData = GetAEDAtaFromTechno(From);
	auto ToData = GetAEDAtaFromTechno(To);

	ToData->Data.clear();

	// while recreation itself isn't the best idea, less hassle and more reliable
	// list gets intact in the end
	for (const auto& Item : FromData->Data)
	{
		AresAE::Attach(Item.Type, To, Item.Duration, Item.Invoker);
	}

	FromData->Data.clear();
	FromData->Isset = false;
	RecalculateStat(ToData, To);
}

void AresAE::ClearAnim()
{
	this->Anim.clear();
}

void AresAE::ReplaceAnim(TechnoClass* pTechno, AnimClass* pNewAnim)
{
	this->ClearAnim();

	pNewAnim->SetOwnerObject(pTechno);
	pNewAnim->RemainingIterations = -1;

	if (auto pInvoker = this->Invoker) {
		pNewAnim->Owner = pInvoker;
	}

	this->Anim.reset(pNewAnim);
}

void AresAE::CreateAnim(TechnoClass* pTechno)
{
	const auto state = pTechno->CloakState;

	if (state != CloakState::Cloaked
	&& state != CloakState::Cloaking
	&& (!pTechno->TemporalTargetingMe || !this->Type->TemporalHidesAnim))
	{
		if (auto pType = this->Type->AnimType)
		{
			this->ReplaceAnim(pTechno, GameCreate<AnimClass>(pType, pTechno->Location));
		}
	}
}
