#include "AttachedAffects.h"

#include <Misc/Ares/Hooks/Header.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

void NOINLINE AresAE::RecalculateStat(AresAEData* ae, TechnoClass* pThis)
{
	double ROF_Mult = 1.0;
	double FP_Mult = pThis->align_154->AE_FirePowerMult;
	double Armor_Mult = pThis->align_154->AE_ArmorMult;
	double Speed_Mult = pThis->align_154->AE_SpeedMult;
	BYTE Cloak = (BYTE)pThis->CanICloakByDefault() | pThis->align_154->AE_Cloak;

	for (const auto& ae : ae->Data)
	{
		//the class aligment is different , so it probably broke something
		ROF_Mult *= ae.Type->ROFMultiplier;
		FP_Mult *= ae.Type->FirepowerMultiplier;
		Speed_Mult *= ae.Type->SpeedMultiplier;
		Armor_Mult *= ae.Type->ArmorMultiplier;
		Cloak |= (BYTE)ae.Type->Cloakable;
	}

	pThis->FirepowerMultiplier = FP_Mult;
	pThis->ArmorMultiplier = Armor_Mult;
	pThis->align_154->AE_ROF = ROF_Mult;
	pThis->Cloakable = Cloak;

	if (pThis->AbstractFlags & AbstractFlags::Foot)
	{

		//if (Speed_Mult < 1.0 && TechnoExt::IsInWarfactory(pThis))
		//	Speed_Mult = 1.0; //negate all speed bonusses when it is still in warfactory

		((FootClass*)pThis)->SpeedMultiplier = Speed_Mult;
	}
}

#include <Ext/WarheadType/Body.h>

void AresAE::applyAttachedEffect(WarheadTypeClass* pWH, const CoordStruct& coords, HouseClass* Source)
{
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (pWHExt->AttachedEffect.Duration != 0)
	{
		// set of affected objects. every object can be here only once.
		const auto items = Helpers::Alex::getCellSpreadItems(coords, pWH->CellSpread, true);

		// affect each object
		for (const auto curTechno : items)
		{
			// don't attach to dead
			if (!curTechno || curTechno->InLimbo || !curTechno->IsAlive || !curTechno->Health)
			{
				continue;
			}

			if (Source && !pWHExt->CanAffectHouse(curTechno->Owner, Source))
			{
				continue;
			}

			if (std::abs(pWHExt->GetVerses(curTechno->GetType()->Armor).Verses) < 0.001)
			{
				continue;
			}

			AresAE::Attach(&pWHExt->AttachedEffect, curTechno, pWHExt->AttachedEffect.Duration, Source);
		}
	}
}

AresAttachEffectTypeClass* GetAETypeFromTechnoType(TechnoTypeClass* pType)
{
	return std::addressof(TechnoTypeExt::ExtMap.Find(pType)->AttachedEffect);
}

AresAEData* GetAEDAtaFromTechno(TechnoClass* pThis)
{
	return std::addressof(TechnoExt::ExtMap.Find(pThis)->AeData); //dummy
}

void NOINLINE AresAE::UpdateTempoal(AresAEData* ae, TechnoClass* pTechno)
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

void NOINLINE AresAE::Update(AresAEData* ae, TechnoClass* pTechno)
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

bool NOINLINE AresAE::Remove(AresAEData* ae)
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

void NOINLINE AresAE::Remove(AresAEData* ae, TechnoClass* pTechno)
{
	if (AresAE::Remove(ae))
		RecalculateStat(ae, pTechno);
}

void NOINLINE AresAE::RemoveSpecific(AresAEData* ae, TechnoClass* pTechno, AbstractTypeClass* pRemove)
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

bool NOINLINE AresAE::Attach(AresAttachEffectTypeClass* pType, TechnoClass* pTargetTechno, int duration, HouseClass* pInvokerOwner)
{
	if (pTargetTechno->IsIronCurtained() && !pType->PenetratesIC)
		return false;

	auto pData = GetAEDAtaFromTechno(pTargetTechno);

	if (!pType->Cumulative)
	{
		auto const it = std::find_if(pData->Data.begin(), pData->Data.end(),
			[=](auto const& item) { return item.Type == pType; });

		if (it != pData->Data.end())
		{
			auto& Item = *it;

			Item.Duration = Item.Type->Duration;

			if (pType->AnimType && pType->AnimResetOnReapply) {
				Item.CreateAnim(pTargetTechno);
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

void NOINLINE AresAE::ClearAnim()
{
	this->Anim.reset(nullptr);
}

void NOINLINE AresAE::ReplaceAnim(TechnoClass* pTechno, AnimClass* pNewAnim)
{
	this->ClearAnim();

	pNewAnim->SetOwnerObject(pTechno);
	pNewAnim->RemainingIterations = -1;

	if (auto pInvoker = this->Invoker) {
		pNewAnim->Owner = pInvoker;
	}

	this->Anim.reset(pNewAnim);
}

void NOINLINE AresAE::CreateAnim(TechnoClass* pTechno)
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
