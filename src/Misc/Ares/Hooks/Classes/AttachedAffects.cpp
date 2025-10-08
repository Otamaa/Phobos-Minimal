#include "AttachedAffects.h"

#include <Misc/Ares/Hooks/Header.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <New/PhobosAttachedAffect/PhobosAttachEffectTypeClass.h>

#include <Ext/WarheadType/Body.h>

#include <Utilities/Helpers.h>

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

			if (Math::abs(pWHExt->GetVerses(TechnoExtData::GetTechnoArmor(curTechno , pWH)).Verses) < 0.001)
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

		if (ae->Data.remove_all_if([](const auto& ae_) { return !ae_.Duration; })) {
			AEProperties::Recalculate(pTechno);
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
		if (fast_remove_if(ae->Data,[](auto& Item){
			Item.ClearAnim();
			return static_cast<bool>(Item.Type->DiscardOnEntry);
		})) {
			return true;
		}
	}

	return false;
}

void AresAE::Remove(AresAEData* ae, TechnoClass* pTechno)
{
	if (AresAE::Remove(ae))
		AEProperties::Recalculate(pTechno);
}

void AresAE::RemoveSpecific(AresAEData* ae, TechnoClass* pTechno, AbstractTypeClass* pRemove)
{
	ae->Isset = 0;
	if(ae->Data.remove_all_if([pRemove](const auto& ae_) {
			return ae_.Type->Owner == pRemove;
	})) {
		AEProperties::Recalculate(pTechno);
	}
}

bool AresAE::Attach(AresAttachEffectTypeClass* pType, TechnoClass* pTargetTechno, int duration, HouseClass* pInvokerOwner)
{
	if (!pType->PenetratesIC && pTargetTechno->IsIronCurtained())
		return false;

	auto pData = GetAEDAtaFromTechno(pTargetTechno);

	if (!pType->Cumulative)
	{
		auto const it = std::ranges::find_if(pData->Data,
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
	AEProperties::Recalculate(pTargetTechno);

	if (pType->ForceDecloak)
	{
		if (pTargetTechno->IsInCloakState())
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
	PhobosAttachEffectClass::TransferAttachedEffects(From, To);
	AEProperties::Recalculate(To);
}

void AresAE::ClearAnim()
{
	this->Anim.clear();
}
#include <Ext/Anim/Body.h>

void AresAE::ReplaceAnim(TechnoClass* pTechno, AnimClass* pNewAnim)
{
	this->ClearAnim();

	pNewAnim->SetOwnerObject(pTechno);
	pNewAnim->RemainingIterations = 0xffffffff;
	auto pAnimExt = ((FakeAnimClass*)pNewAnim)->_GetExtData();

	pAnimExt->IsAttachedEffectAnim = true;

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
