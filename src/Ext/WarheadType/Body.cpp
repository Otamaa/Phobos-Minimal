#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>

#include <Ext/BulletType/Body.h>
#include <New/Type/ArmorTypeClass.h>
#include <Ext/Techno/Body.h>

template<> const DWORD Extension<WarheadTypeClass>::Canary = 0x22222222;
WarheadTypeExt::ExtContainer WarheadTypeExt::ExtMap;
void WarheadTypeExt::ExtData::Initialize()
{
	this->IsNukeWarhead = !std::strcmp(RulesExt::Global()->NukeWarheadName.data(), this->OwnerObject()->get_ID());
	this->Launchs.reserve(2);
}

bool WarheadTypeExt::ExtData::CanAffectHouse(HouseClass* pOwnerHouse, HouseClass* pTargetHouse)
{
	if (pOwnerHouse && pTargetHouse)
	{
		if (pTargetHouse == pOwnerHouse)
			return this->AffectsOwner.Get(this->OwnerObject()->AffectsAllies);

		if (pOwnerHouse->IsAlliedWith(pTargetHouse) && pTargetHouse != pOwnerHouse)
			return this->OwnerObject()->AffectsAllies;

		return AffectsEnemies.Get();
	}

	return true;
}

bool WarheadTypeExt::ExtData::CanDealDamage(TechnoClass* pTechno)
{
	if (pTechno) {

		if (pTechno->GetTechnoType()->Immune)
			return false;

		if (EffectsRequireVerses.Get()) {
			auto nArmor = pTechno->GetTechnoType()->Armor;
			if (auto pExt = TechnoExt::GetExtData(pTechno))
				if (pExt->CurrentShieldType && pExt->GetShield() && pExt->GetShield()->IsActive())
					nArmor = pExt->CurrentShieldType->Armor;

			auto nVal = GeneralUtils::GetWarheadVersusArmor(OwnerObject(), nArmor);
			//if(IS_SAME_STR_(OwnerObject()->get_ID() , "NebulaWH"))
			//	Debug::Log("%s WH Calculating Damage Against %s : [%s] : [%fl] ! \n", OwnerObject()->get_ID() , pTechno->get_ID() , ArmorTypeClass::Array[(int)nArmor]->Name.data(), nWHVerses);
			return (fabs(nVal) >= 0.001);
		}
	}

	return true;
}

bool WarheadTypeExt::ExtData::CanDealDamage(TechnoClass* pTechno, int damageIn, int distanceFromEpicenter, int& DamageResult, bool effectsRequireDamage)
{
	auto nArmor = pTechno->GetTechnoType()->Armor;

	if (auto pExt = TechnoExt::GetExtData(pTechno))
		if (pExt->GetShield() && pExt->GetShield()->IsActive())
			nArmor = pExt->GetShield()->GetType()->Armor;

	if (damageIn > 0)
		DamageResult = MapClass::GetTotalDamage(damageIn, OwnerObject(), nArmor, distanceFromEpicenter);
	else
		DamageResult = -MapClass::GetTotalDamage(-damageIn, OwnerObject(), nArmor, distanceFromEpicenter);

	if (damageIn == 0)
	{
		return AllowZeroDamage;
	}
	else
	{
		if (EffectsRequireVerses)
		{
			if (MapClass::GetTotalDamage(RulesGlobal->MaxDamage, OwnerObject(), nArmor, 0) == 0.0)
			{
				return false;
			}

			if (effectsRequireDamage || EffectsRequireDamage)
			{
				return DamageResult != 0;
			}
		}
	}

	return true;

}

bool WarheadTypeExt::ExtData::CanTargetHouse(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (pHouse && pTarget) {
		return CanAffectHouse(pHouse, pTarget->GetOwningHouse());
	}

	return true;
}

void WarheadTypeExt::DetonateAt(WarheadTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, int damage)
{
	BulletTypeClass* pType = BulletTypeExt::GetDefaultBulletType();
	auto pBulletTypeExt = BulletTypeExt::GetExtData(pType);

	if (BulletClass* pBullet = pBulletTypeExt->CreateBullet(pTarget, pOwner,
		damage, pThis, 0,0,pThis->Bright))
	{
		const CoordStruct& coords = pTarget->GetCoords();

		pBullet->Limbo();
		pBullet->SetLocation(coords);
		pBullet->Explode(true);
		pBullet->UnInit();
	}
}

void WarheadTypeExt::DetonateAt(WarheadTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage)
{
	BulletTypeClass* pType = BulletTypeExt::GetDefaultBulletType();
	auto pBulletTypeExt = BulletTypeExt::GetExtData(pType);

	if (BulletClass* pBullet = pBulletTypeExt->CreateBullet(nullptr, pOwner,
		damage, pThis, 0 , 0 , pThis->Bright))
	{
		pBullet->Limbo();
		pBullet->SetLocation(coords);
		pBullet->Explode(true);
		pBullet->UnInit();
	}
}

// =============================
// load / save

void WarheadTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	// Miscs
	this->SpySat.Read(exINI, pSection, "SpySat");
	this->BigGap.Read(exINI, pSection, "BigGap");
	this->TransactMoney.Read(exINI, pSection, "TransactMoney");
	this->SplashList.Read(exINI, pSection, "SplashList");
	this->SplashList_PickRandom.Read(exINI, pSection, "SplashList.PickRandom");
	this->RemoveDisguise.Read(exINI, pSection, "RemoveDisguise");
	this->RemoveMindControl.Read(exINI, pSection, "RemoveMindControl");
	this->AnimList_PickRandom.Read(exINI, pSection, "AnimList.PickRandom");
	this->DecloakDamagedTargets.Read(exINI, pSection, "DecloakDamagedTargets");
	this->ShakeIsLocal.Read(exINI, pSection, "ShakeIsLocal");

	// Crits
	this->Crit_Chance.Read(exINI, pSection, "Crit.Chance");
	this->Crit_ApplyChancePerTarget.Read(exINI, pSection, "Crit.ApplyChancePerTarget");
	this->Crit_ExtraDamage.Read(exINI, pSection, "Crit.ExtraDamage");
	this->Crit_Warhead.Read(exINI, pSection, "Crit.Warhead");
	this->Crit_Affects.Read(exINI, pSection, "Crit.Affects");
	this->Crit_AnimList.Read(exINI, pSection, "Crit.AnimList");
	this->Crit_AnimList_PickRandom.Read(exINI, pSection, "Crit.AnimList.PickRandom");
	this->Crit_AnimOnAffectedTargets.Read(exINI, pSection, "Crit.AnimOnAffectedTargets");
	this->Crit_AffectBelowPercent.Read(exINI, pSection, "Crit.AffectBelowPercent");
	this->Crit_SuppressOnIntercept.Read(exINI, pSection, "Crit.SuppressWhenIntercepted");

	this->MindControl_Anim.Read(exINI, pSection, "MindControl.Anim");

	// Ares tags
	// http://ares-developers.github.io/Ares-docs/new/warheads/general.html
	this->AffectsEnemies.Read(exINI, pSection, "AffectsEnemies");
	this->AffectsOwner.Read(exINI, pSection, "AffectsOwner");
	this->EffectsRequireDamage.Read(exINI, pSection, "EffectsRequireDamage");
	this->EffectsRequireVerses.Read(exINI, pSection, "EffectsRequireVerses");
	this->AllowZeroDamage.Read(exINI, pSection, "AllowZeroDamage");

	// Shields
	this->Shield_Penetrate.Read(exINI, pSection, "Shield.Penetrate");
	this->Shield_Break.Read(exINI, pSection, "Shield.Break");
	this->Shield_BreakAnim.Read(exINI, pSection, "Shield.BreakAnim");
	this->Shield_HitAnim.Read(exINI, pSection, "Shield.HitAnim");
	this->Shield_BreakWeapon.Read(exINI, pSection, "Shield.BreakWeapon", true);
	this->Shield_AbsorbPercent.Read(exINI, pSection, "Shield.AbsorbPercent");
	this->Shield_PassPercent.Read(exINI, pSection, "Shield.PassPercent");
	this->Shield_Respawn_Duration.Read(exINI, pSection, "Shield.Respawn.Duration");
	this->Shield_Respawn_Amount.Read(exINI, pSection, "Shield.Respawn.Amount");
	this->Shield_Respawn_Rate_InMinutes.Read(exINI, pSection, "Shield.Respawn.Rate");
	this->Shield_Respawn_Rate = (int)(this->Shield_Respawn_Rate_InMinutes * 900);
	this->Shield_Respawn_ResetTimer.Read(exINI, pSection, "Shield.Respawn.RestartTimer");
	this->Shield_SelfHealing_Duration.Read(exINI, pSection, "Shield.SelfHealing.Duration");
	this->Shield_SelfHealing_Amount.Read(exINI, pSection, "Shield.SelfHealing.Amount");
	this->Shield_SelfHealing_Rate_InMinutes.Read(exINI, pSection, "Shield.SelfHealing.Rate");
	this->Shield_SelfHealing_Rate = (int)(this->Shield_SelfHealing_Rate_InMinutes * 900);
	this->Shield_SelfHealing_ResetTimer.Read(exINI, pSection, "Shield.SelfHealing.RestartTimer");
	this->Shield_AttachTypes.Read(exINI, pSection, "Shield.AttachTypes");
	this->Shield_RemoveTypes.Read(exINI, pSection, "Shield.RemoveTypes");
	this->Shield_ReplaceOnly.Read(exINI, pSection, "Shield.ReplaceOnly");
	this->Shield_ReplaceNonRespawning.Read(exINI, pSection, "Shield.ReplaceNonRespawning");
	this->Shield_InheritStateOnReplace.Read(exINI, pSection, "Shield.InheritStateOnReplace");
	this->Shield_MinimumReplaceDelay.Read(exINI, pSection, "Shield.MinimumReplaceDelay");
	this->Shield_AffectTypes.Read(exINI, pSection, "Shield.AffectTypes");

	this->NotHuman_DeathSequence.Read(exINI, pSection, "NotHuman.DeathSequence");
	this->ShakeIsLocal.Read(exINI, pSection, "ShakeIsLocal");

	// Transact
	this->Transact.Read(exINI, pSection, "Transact");

	this->TransactMoney_Ally.Read(exINI, pSection, "TransactMoney.Ally");
	this->TransactMoney_Enemy.Read(exINI, pSection, "TransactMoney.Enemy");
	this->Transact_AffectsEnemies.Read(exINI, pSection, "TransactMoney.AffectsEnemies");
	this->Transact_AffectsAlly.Read(exINI, pSection, "TransactMoney.AffectsAlly");
	this->Transact_AffectsOwner.Read(exINI, pSection, "TransactMoney.AffectsOwner");

	this->Transact_SpreadAmongTargets.Read(exINI, pSection, "Transact.SpreadAmongTargets");
	this->Transact_Experience_Value.Read(exINI, pSection, "Transact.Experience.Value");
	this->Transact_Experience_Source_Flat.Read(exINI, pSection, "Transact.Experience.Source.Flat");
	this->Transact_Experience_Source_Percent.Read(exINI, pSection, "Transact.Experience.Source.Percent");
	this->Transact_Experience_Source_Percent_CalcFromTarget.Read(exINI, pSection, "Transact.Experience.Source.Percent.CalcFromTarget");
	this->Transact_Experience_Target_Flat.Read(exINI, pSection, "Transact.Experience.Target.Flat");
	this->Transact_Experience_Target_Percent.Read(exINI, pSection, "Transact.Experience.Target.Percent");
	this->Transact_Experience_Target_Percent_CalcFromSource.Read(exINI, pSection, "Transact.Experience.Target.Percent.CalcFromSource");
	this->TransactMoney_Display.Read(exINI, pSection, "TransactMoney.Display");
	this->TransactMoney_Display_Houses.Read(exINI, pSection, "TransactMoney.Display.Houses");
	this->TransactMoney_Display_AtFirer.Read(exINI, pSection, "TransactMoney.Display.AtFirer");
	this->TransactMoney_Display_Offset.Read(exINI, pSection, "TransactMoney.Display.Offset");

	this->StealMoney.Read(exINI, pSection, "StealMoney.Amount");
	this->Steal_Display_Houses.Read(exINI, pSection, "StealMoney.Display.Houses");
	this->Steal_Display.Read(exINI, pSection, "StealMoney.Display");
	this->Steal_Display_Offset.Read(exINI, pSection, "StealMoney.Display.Offset");

	this->NotHuman_DeathAnim.Read(exINI, pSection, "NotHuman.DeahAnim");
	this->AllowDamageOnSelf.Read(exINI, pSection, "AllowDamageOnSelf");
	this->Debris_Conventional.Read(exINI, pSection, "Debris.Conventional");

	this->GattlingStage.Read(exINI, pSection, "TargetGattlingStage");
	this->GattlingRateUp.Read(exINI, pSection, "TargetGattlingRateUp");
	this->ReloadAmmo.Read(exINI, pSection, "TargetReloadAmmo");

#pragma region Otamaa

	this->SquidSplash.Read(exINI, pSection, "Squid.SplashAnims");
	this->TemporalExpiredAnim.Read(exINI, pSection, "Temporal.ExpiredAnim");
	this->TemporalDetachDamageFactor.Read(exINI, pSection, "Temporal.ExpiredDamageFactor");
	this->TemporalExpiredApplyDamage.Read(exINI, pSection, "Temporal.ExpiredApplyDamage");
	this->DebrisAnimTypes.Read(exINI, pSection, "DebrisAnims");
	this->Flammability.Read(exINI, pSection, "FlameChance");

	if (this->Launchs.empty())
	{
		for (size_t i = 0; ; ++i)
		{
			LauchSWData nData;
			if (!nData.Read(exINI, pSection, i))
				break;

			this->Launchs.push_back((nData));
		}
	}

	this->Parasite_DisableRocking.Read(exINI, pSection, "Parasite.DisableRocking");
	this->Parasite_GrappleAnimIndex.Read(exINI, pSection, "Parasite.GrappleAnim");

	auto ReadHitAnim = [this, &exINI, pSection](const char* pBaseKey, bool bAllocate = true)
	{
		char tempBuffer[2048];
		for (size_t i = 0; i < ArmorTypeClass::Array.size(); ++i)
		{
			Nullable<AnimTypeClass*> pAnimReaded;
			if (auto pArmor = ArmorTypeClass::Array[i].get())
			{
				_snprintf_s(tempBuffer, _TRUNCATE, "%s.%s", pBaseKey, pArmor->Name.data());
				pAnimReaded.Read(exINI, pSection, tempBuffer, bAllocate);
			}
			ArmorHitAnim[i] = pAnimReaded.Get(nullptr);
		}

		//remove invalid items!
		if (!ArmorHitAnim.empty()) {
			//remove invalid items to keep memory clean !
			for (auto const& [nArmor , Anim] : ArmorHitAnim) {
				if (!Anim)
					ArmorHitAnim.erase(nArmor);
			}
		}
	};

	ReadHitAnim("HitAnim");

	this->IsNukeWarhead.Read(exINI, pSection, "IsNukeWarhead");
	this->Remover.Read(exINI, pSection, "Remover");
	this->Remover_Anim.Read(exINI, pSection, "Remover.Anim");
	this->PermaMC.Read(exINI, pSection, "MindControl.Permanent");

#ifdef COMPILE_PORTED_DP_FEATURES_
	auto ReadHitTextData = [this, &exINI, pSection](const char* pBaseKey, bool bAllocate = true)
	{
		char tempBuffer[2048];
		for (size_t i = 0; i < ArmorTypeClass::Array.size(); ++i)
		{
			if (auto pArmor = ArmorTypeClass::Array[i].get())
			{
				_snprintf_s(tempBuffer, _TRUNCATE, "%s.%s.", pBaseKey, pArmor->Name.data());
				DamageTextPerArmor[i].Read(exINI, pSection, tempBuffer);
			}
		}
	};

	ReadHitTextData("DamageText");

#endif

#ifdef COMPILE_PORTED_DP_FEATURES
	this->PaintBallDuration.Read(exINI, pSection, "PaintBall.Duration");
	PaintBallData.Read(exINI, pSection);
#endif
#pragma endregion

}

template <typename T>
void WarheadTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->SpySat)
		.Process(this->BigGap)
		.Process(this->TransactMoney)
		.Process(this->TransactMoney_Ally)
		.Process(this->TransactMoney_Enemy)
		.Process(this->Transact_AffectsEnemies)
		.Process(this->Transact_AffectsAlly)
		.Process(this->Transact_AffectsOwner)

		.Process(this->TransactMoney_Display)
		.Process(this->TransactMoney_Display_Houses)
		.Process(this->TransactMoney_Display_AtFirer)
		.Process(this->TransactMoney_Display_Offset)

		.Process(this->SplashList)
		.Process(this->SplashList_PickRandom)
		.Process(this->RemoveDisguise)
		.Process(this->RemoveMindControl)
		.Process(this->AnimList_PickRandom)
		.Process(this->DecloakDamagedTargets)
		.Process(this->ShakeIsLocal)

		.Process(this->Crit_Chance)
		.Process(this->Crit_ApplyChancePerTarget)
		.Process(this->Crit_ExtraDamage)
		.Process(this->Crit_Warhead)
		.Process(this->Crit_Affects)
		.Process(this->Crit_AnimList)
		.Process(this->Crit_AnimList_PickRandom)
		.Process(this->Crit_AnimOnAffectedTargets)
		.Process(this->Crit_AffectBelowPercent)
		.Process(this->Crit_SuppressOnIntercept)

		.Process(this->MindControl_Anim)

		// Ares tags
		.Process(this->AffectsEnemies)
		.Process(this->AffectsOwner)
		.Process(this->EffectsRequireDamage)
		.Process(this->EffectsRequireVerses)
		.Process(this->AllowZeroDamage)

		.Process(this->Shield_Penetrate)
		.Process(this->Shield_Break)
		.Process(this->Shield_BreakAnim)
		.Process(this->Shield_HitAnim)
		.Process(this->Shield_BreakWeapon)
		.Process(this->Shield_AbsorbPercent)
		.Process(this->Shield_PassPercent)

		.Process(this->Shield_Respawn_Duration)
		.Process(this->Shield_Respawn_Amount)
		.Process(this->Shield_Respawn_Rate)
		.Process(this->Shield_Respawn_ResetTimer)
		.Process(this->Shield_SelfHealing_Duration)
		.Process(this->Shield_SelfHealing_Amount)
		.Process(this->Shield_SelfHealing_Rate)
		.Process(this->Shield_SelfHealing_ResetTimer)
		.Process(this->Shield_AttachTypes)
		.Process(this->Shield_RemoveTypes)
		.Process(this->Shield_ReplaceOnly)
		.Process(this->Shield_ReplaceNonRespawning)
		.Process(this->Shield_InheritStateOnReplace)
		.Process(this->Shield_MinimumReplaceDelay)
		.Process(this->Shield_AffectTypes)

		.Process(this->Transact)
		.Process(this->Transact_SpreadAmongTargets)
		.Process(this->Transact_Experience_Value)
		.Process(this->Transact_Experience_Source_Flat)
		.Process(this->Transact_Experience_Source_Percent)
		.Process(this->Transact_Experience_Source_Percent_CalcFromTarget)
		.Process(this->Transact_Experience_Target_Flat)
		.Process(this->Transact_Experience_Target_Percent)
		.Process(this->Transact_Experience_Target_Percent_CalcFromSource)

		.Process(this->NotHuman_DeathSequence)
		.Process(this->AllowDamageOnSelf)
		.Process(this->Debris_Conventional)

		.Process(NotHuman_DeathAnim)
		.Process(IsNukeWarhead)
		.Process(Remover)
		.Process(Remover_Anim)
		.Process(ArmorHitAnim)
		.Process(DebrisAnimTypes)
		.Process(SquidSplash)
		.Process(TemporalExpiredAnim)
		.Process(TemporalExpiredApplyDamage)
		.Process(TemporalDetachDamageFactor)
		.Process(Parasite_DisableRocking)
		.Process(Parasite_GrappleAnimIndex)
		.Process(Flammability)
		.Process(Launchs)
		.Process(PermaMC)

		.Process(StealMoney)
		.Process(Steal_Display_Houses)
		.Process(Steal_Display)
		.Process(Steal_Display_Offset)

		.Process(GattlingStage)
		.Process(GattlingRateUp)
		.Process(ReloadAmmo)

#ifdef COMPILE_PORTED_DP_FEATURES_
		.Process(DamageTextPerArmor)

#endif
#ifdef COMPILE_PORTED_DP_FEATURES
		.Process(PaintBallDuration)
#endif
		;
#ifdef COMPILE_PORTED_DP_FEATURES
		PaintBallData.Serialize(Stm);
#endif
}

void WarheadTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<WarheadTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void WarheadTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<WarheadTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool WarheadTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool WarheadTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}

// =============================
// container

WarheadTypeExt::ExtContainer::ExtContainer() : Container("WarheadTypeClass") { }

WarheadTypeExt::ExtContainer::~ExtContainer() = default;

void WarheadTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved)
{

}

// =============================
// container hooks

DEFINE_HOOK(0x75D1A9, WarheadTypeClass_CTOR, 0x7)
{
	GET(WarheadTypeClass*, pItem, EBP);

	WarheadTypeExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x75E5C8, WarheadTypeClass_SDDTOR, 0x6)
{
	GET(WarheadTypeClass*, pItem, ESI);

	WarheadTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x75E2C0, WarheadTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x75E0C0, WarheadTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(WarheadTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	WarheadTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x75E2AE, WarheadTypeClass_Load_Suffix, 0x7)
{
	WarheadTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x75E39C, WarheadTypeClass_Save_Suffix, 0x5)
{
	WarheadTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x75DEAF, WarheadTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x75DEA0, WarheadTypeClass_LoadFromINI, 0x5)
{
	GET(WarheadTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x150);

	WarheadTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
