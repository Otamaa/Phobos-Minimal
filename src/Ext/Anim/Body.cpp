#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Helpers.h>
#include <Utilities/AnimHelpers.h>

#include <ParticleSystemClass.h>
#include <ColorScheme.h>
#include <SmudgeTypeClass.h>

//std::vector<CellClass*> AnimExt::AnimCellUpdater::Marked;
AnimExt::ExtContainer AnimExt::ExtMap;

void AnimExt::ExtData::InitializeConstants()
{
	CreateAttachedSystem();
}
void AnimExt::OnInit(AnimClass* pThis, CoordStruct* pCoord)
{
	if (!pThis->Type)
		return;

	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->ConcurrentChance.Get() >= 1.0 && !pTypeExt->ConcurrentAnim.empty()) {
		if (ScenarioClass::Instance->Random.RandomDouble() <= pTypeExt->ConcurrentChance.Get()) {
			auto const nIdx = ScenarioClass::Instance->Random.RandomFromMax(pTypeExt->ConcurrentAnim.size() - 1);

			if (auto pType = pTypeExt->ConcurrentAnim[nIdx]) {

				if (pType == pThis->Type)
					return;

				if (auto pAnim = GameCreate<AnimClass>(pType, pCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false))
					pAnim->Owner = pThis->GetOwningHouse();
			}
		}
	}

	//if (auto const& pSpawns = pExt->SpawnData) {
	//	pSpawns->OnInit(pCoord);
	//}
}

void AnimExt::OnMiddle_SpawnParticle(AnimClass* pThis, CellClass* pCell, Point2D nOffs)
{
	const auto pType = pThis->Type;
	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pType);

	if (pTypeExt->SpawnCrater.Get(pThis->GetHeight() < 30))
	{
		auto nCoord = pThis->GetCoords();
		if (!pType->Scorch || (pType->Crater && ScenarioClass::Instance->Random.RandomDouble() >= pTypeExt->CraterChance.Get()))
		{
			if (pType->Crater)
			{
				if (pTypeExt->CraterDecreaseTiberiumAmount.Get() > 0)
					pCell->ReduceTiberium(pTypeExt->CraterDecreaseTiberiumAmount.Get());

				if (pType->ForceBigCraters)
					SmudgeTypeClass::CreateRandomSmudgeFromTypeList(nCoord, 300, 300, true);
				else
					SmudgeTypeClass::CreateRandomSmudgeFromTypeList(nCoord, nOffs.X, nOffs.Y, false);
			}
		}
		else
		{
			const bool bSpawn = (pTypeExt->ScorchChance.isset()) ? (ScenarioClass::Instance->Random.RandomDouble() >= pTypeExt->ScorchChance.Get()) : true;

			if (bSpawn)
				SmudgeTypeClass::CreateRandomSmudge(nCoord, nOffs.X, nOffs.Y, false);
		}
	}
}

void AnimExt::OnExpired(AnimClass* pThis, bool LandIsWater, bool EligibleHeight)
{
	auto const pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	{
		TechnoClass* const pTechOwner = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker);
		auto const pOwner = !pThis->Owner && pTechOwner ? pTechOwner->Owner : pThis->Owner;

		if (!LandIsWater || EligibleHeight)
		{
			Helper::Otamaa::DetonateWarhead(Game::F2I(pThis->Type->Damage), pThis->Type->Warhead, pAnimTypeExt->Warhead_Detonate, pThis->Bounce.GetCoords(), pTechOwner, pOwner, pAnimTypeExt->Damage_ConsiderOwnerVeterancy.Get());

			if (auto const pExpireAnim = pThis->Type->ExpireAnim)
			{
				if (auto pAnim = GameCreate<AnimClass>(pExpireAnim, pThis->Bounce.GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -30, 0))
				{
					AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, pTechOwner, false);
				}
			}
		}
		else
		{
			if (!pAnimTypeExt->ExplodeOnWater)
			{
				if (auto pSplashAnim = Helper::Otamaa::PickSplashAnim(pAnimTypeExt->SplashList, pAnimTypeExt->WakeAnim.Get(RulesClass::Instance->Wake), pAnimTypeExt->SplashIndexRandom.Get(), pThis->Type->IsMeteor))
				{
					if (auto const pSplashAnimCreated = GameCreate<AnimClass>(pSplashAnim, pThis->GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, false))
					{
						AnimExt::SetAnimOwnerHouseKind(pSplashAnimCreated, pOwner, nullptr, pTechOwner, false);
					}
				}
			}
			else
			{
				auto const [bPlayWHAnim, nDamage] = Helper::Otamaa::DetonateWarhead(Game::F2I(pThis->Type->Damage), pThis->Type->Warhead, pAnimTypeExt->Warhead_Detonate, pThis->GetCoords(), pTechOwner, pOwner, pAnimTypeExt->Damage_ConsiderOwnerVeterancy.Get());
				if (bPlayWHAnim)
				{
					if (auto pSplashAnim = MapClass::SelectDamageAnimation(nDamage, pThis->Type->Warhead, pThis->GetCell()->LandType, pThis->GetCoords()))
					{
						if (auto const pSplashAnimCreated = GameCreate<AnimClass>(pSplashAnim, pThis->GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -30))
						{
							AnimExt::SetAnimOwnerHouseKind(pSplashAnimCreated, pOwner, nullptr, pTechOwner, false);
						}
					}
				}
			}
		}
	}
}

bool AnimExt::DealDamageDelay(AnimClass* pThis)
{
	if (pThis->Type->Damage <= 0.0 || pThis->HasExtras)
		return false;

	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	int delay = pTypeExt->Damage_Delay.Get();
	TechnoClass* const pInvoker = AnimExt::GetTechnoInvoker(pThis, pTypeExt->Damage_DealtByInvoker);

	int damageMultiplier = 1;

	if (pThis->OwnerObject && pThis->OwnerObject->WhatAmI() == AbstractType::Terrain)
		damageMultiplier = 5;

	bool adjustAccum = false;
	double damage = 0;
	int appliedDamage = 0;

	if (pTypeExt->Damage_ApplyOnce.Get()) // If damage is to be applied only once per animation loop
	{
		if (pThis->Animation.Value == max(delay - 1, 1))
			appliedDamage = static_cast<int>(int_round(pThis->Type->Damage)) * damageMultiplier;
		else
			return false;
	}
	else if (delay <= 0 || pThis->Type->Damage < 1.0) // If Damage.Delay is less than 1 or Damage is a fraction.
	{
		adjustAccum = true;
		damage = damageMultiplier * pThis->Type->Damage + pThis->Accum;
		pThis->Accum = damage;

		// Deal damage if it is at least 1, otherwise accumulate it for later.
		if (damage >= 1.0)
			appliedDamage = static_cast<int>(int_round(damage));
		else
			return false;
	}
	else
	{
		// Accum here is used as a counter for Damage.Delay, which cannot deal fractional damage.
		damage = pThis->Accum + 1.0;
		pThis->Accum = damage;

		if (damage < delay)
			return false;

		// Use Type->Damage as the actually dealt damage.
		appliedDamage = static_cast<int>((pThis->Type->Damage)) * damageMultiplier;
	}

	if (appliedDamage <= 0 || pThis->IsPlaying)
		return false;

	// Store fractional damage if needed, or reset the accum if hit the Damage.Delay counter.
	if (adjustAccum)
		pThis->Accum = damage - appliedDamage;
	else
		pThis->Accum = 0.0;

	const auto nCoord = pThis->GetCoords();
	auto const nDamageResult = static_cast<int>(appliedDamage *
		TechnoExt::GetDamageMult(pInvoker, !pTypeExt->Damage_ConsiderOwnerVeterancy.Get()));

	if (auto const pWeapon = pTypeExt->Weapon.Get(nullptr))
	{
		AbstractClass* pTarget = AnimExt::GetTarget(pThis);
		WeaponTypeExt::DetonateAt(pWeapon, nCoord, pTarget, pInvoker, nDamageResult);
	}
	else
	{
		auto const pWarhead = pThis->Type->Warhead ? pThis->Type->Warhead :
			!pTypeExt->IsInviso ? RulesClass::Instance->FlameDamage2 : RulesClass::Instance->C4Warhead;

		const auto pOwner = pThis->Owner ? pThis->Owner : pInvoker ? pInvoker->GetOwningHouse() : nullptr;

		if (pTypeExt->Warhead_Detonate.Get())
		{
			AbstractClass* pTarget = AnimExt::GetTarget(pThis);
			WarheadTypeExt::DetonateAt(pWarhead, pTarget, nCoord, pInvoker, nDamageResult);
		}
		else
			MapClass::DamageArea(nCoord, nDamageResult, pInvoker, pWarhead, pWarhead->Tiberium, pOwner);
		//MapClass::FlashbangWarheadAt(nDamageResult, pWarhead, nCoord);
	}

	return true;
}

void AnimExt::OnMiddle(AnimClass* pThis)
{
	const auto pType = pThis->Type;
	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pType);

	{
		auto pAnimTypeExt = pTypeExt;
		const auto pObject = AnimExt::GetTechnoInvoker(pThis, pTypeExt->Damage_DealtByInvoker.Get());
		const auto pHouse = !pThis->Owner && pObject ? pObject->Owner : pThis->Owner;
		const auto nCoord = pThis->GetCoords();

		Helper::Otamaa::SpawnMultiple(
			pAnimTypeExt->SpawnsMultiple,
			pAnimTypeExt->SpawnsMultiple_amouts,
			nCoord, pObject, pHouse, pAnimTypeExt->SpawnsMultiple_Random.Get());

		if (pType->SpawnsParticle != -1)
		{
			const auto pParticleType = ParticleTypeClass::Array.get()->GetItem(pType->SpawnsParticle);

			if (pType->NumParticles > 0 && pParticleType)
			{
				for (int i = 0; i < pType->NumParticles; ++i)
				{
					CoordStruct nDestCoord = CoordStruct::Empty;
					if (pAnimTypeExt->ParticleChance.isset() ?
						(ScenarioClass::Instance->Random.RandomFromMax(99) < abs(pAnimTypeExt->ParticleChance.Get())) : true)
					{
						nDestCoord = Helper::Otamaa::GetRandomCoordsInsideLoops(pAnimTypeExt->ParticleRangeMin.Get(), pAnimTypeExt->ParticleRangeMax.Get(), nCoord, i);
						ParticleSystemClass::Instance->SpawnParticle(pParticleType, &nDestCoord);
					}
				}
			}
		}
		if (!pTypeExt->Launchs.empty())
		{
			for (auto const& nLauch : pTypeExt->Launchs)
			{
				if (nLauch.LaunchWhat)
				{
					Helpers::Otamaa::LauchSW(
						nLauch.LaunchWhat, pHouse,
						nCoord, nLauch.LaunchWaitcharge,
						nLauch.LaunchResetCharge,
						nLauch.LaunchGrant,
						nLauch.LaunchGrant_RepaintSidebar,
						nLauch.LaunchGrant_OneTime,
						nLauch.LaunchGrant_OnHold,
						nLauch.LaunchSW_Manual,
						nLauch.LaunchSW_IgnoreInhibitors,
						nLauch.LaunchSW_IgnoreDesignators,
						nLauch.LauchSW_IgnoreMoney
					);
				}
			}
		}
	}
}

AbstractClass* AnimExt::GetTarget(AnimClass* pThis)
{
	auto const pType = pThis->Type;
	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pType);

	switch (pTypeExt->Damage_TargetFlag.Get())
	{
	case DamageDelayTargetFlag::Cell:
		return  pThis->GetCell();
	case DamageDelayTargetFlag::AttachedObject:
	{
		if (pThis->AttachedBullet)
		{
			return pThis->AttachedBullet->Owner;
		}
		else
		{
			if (auto const pBullet = specific_cast<BulletClass*>(pThis->OwnerObject))
				return pBullet->Owner;
			else
				return pThis->OwnerObject;
		}
	}
	case DamageDelayTargetFlag::Invoker:
	{
		return AnimExt::ExtMap.Find(pThis)->Invoker;
	}
	}

	return nullptr;
}

bool AnimExt::ExtData::InvalidateIgnorable(void* const ptr) const
{
	auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
	switch (abs)
	{
	case AbstractType::Building:
	case AbstractType::Infantry:
	case AbstractType::Unit:
	case AbstractType::Aircraft:
	case AbstractType::ParticleSystem:
		return false;
	}

	return true;
}

void AnimExt::ExtData::InvalidatePointer(void* const ptr, bool bRemoved)
{
	if (this->InvalidateIgnorable(ptr))
		return;

	AnnounceInvalidPointer(this->Invoker, ptr);
	AnnounceInvalidPointer(this->ParentBuilding, ptr);
	AnnounceInvalidPointer(this->AttachedSystem, ptr);
}

// =============================
// load / save

template <typename T>
void AnimExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Something)
		.Process(this->DeathUnitFacing)
		.Process(this->DeathUnitTurretFacing)
		.Process(this->Invoker)
		.Process(this->AttachedSystem)
		.Process(this->OwnerSet)
		//.Process(this->SpawnData)
		.Process(this->ParentBuilding)
		;
}

void AnimExt::ExtData::CreateAttachedSystem()
{
	const auto pThis = this->Get();
	const auto pData = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (!pData || !pData->AttachedSystem || this->AttachedSystem)
		return;
	auto nLoc = pThis->Location;

	if (pData->AttachedSystem->BehavesLike == BehavesLike::Smoke)
		nLoc.Z += 100;

	if (auto const pSystem = GameCreate<ParticleSystemClass>(pData->AttachedSystem.Get(), nLoc, pThis->GetCell(), pThis, CoordStruct::Empty, pThis->GetOwningHouse()))
		this->AttachedSystem = pSystem;
}

void AnimExt::ExtData::DeleteAttachedSystem()
{
	if (!this->AttachedSystem)
		return;

	this->AttachedSystem->Owner = nullptr;
	this->AttachedSystem->UnInit();
	this->AttachedSystem = nullptr;
}

//Modified from Ares
const bool AnimExt::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner)
{
	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);
	if (pTypeExt->NoOwner)
		// no we dont set the owner
	// this return also will prevent Techno `Invoker` to be set !
		return false;

	if (pTypeExt->CreateUnit.Get())
	{
		if (const auto newOwner = HouseExt::GetHouseKind(pTypeExt->CreateUnit_Owner.Get(), true, defaultToVictimOwner ? pVictim : nullptr, pInvoker, pVictim))
		{
			pAnim->SetHouse(newOwner);

			if (pTypeExt->CreateUnit_RemapAnim.Get() && !newOwner->Defeated)
				pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;

			return true;//yes
		}
	}

	if (!pAnim->Owner && pInvoker)
		pAnim->SetHouse(pInvoker);

	return true; //yes return true
}

const bool AnimExt::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, TechnoClass* pTechnoInvoker, bool defaultToVictimOwner)
{
	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);
	if (pTypeExt->NoOwner)
		// no we dont set the owner
		// this return also will prevent Techno `Invoker` to be set !
		return false;

	AnimExt::ExtMap.Find(pAnim)->Invoker = pTechnoInvoker;

	if (pTypeExt->CreateUnit.Get())
	{
		if (const auto newOwner = HouseExt::GetHouseKind(pTypeExt->CreateUnit_Owner.Get(), true, defaultToVictimOwner ? pVictim : nullptr, pInvoker, pVictim))
		{
			pAnim->SetHouse(newOwner);

			if (pTypeExt->CreateUnit_RemapAnim.Get() && !newOwner->Defeated)
				pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;

			return true;//yes
		}
	}

	if (!pAnim->Owner && pInvoker)
		pAnim->SetHouse(pInvoker);

	return true; //yes return true
}

//Modified from Ares
//const bool AnimExt::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner)
//{
//	auto const pExt = AnimTypeExt::ExtMap.Find(pAnim->Type);
//
//	if (!pExt->NoOwner)
//	{
//		if (!pExt->CreateUnit.Get())
//		{
//			if (!pAnim->Owner && pInvoker)
//				pAnim->SetHouse(pInvoker);
//
//			return true; //yes return true
//		}
//		else
//		{
//			if (auto const newOwner = HouseExt::GetHouseKind(pExt->CreateUnit_Owner.Get(), true, defaultToVictimOwner ? pVictim : nullptr, pInvoker, pVictim))
//			{
//
//				pAnim->SetHouse(newOwner);
//				AnimExt::ExtMap.Find(pAnim)->OwnerSet = true;
//
//				if (pExt->CreateUnit_RemapAnim.Get() && !newOwner->Defeated)
//					pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;
//
//				return true;//yes
//			}
//		}
//	}
//
//	// no we dont set the owner
//	// this return also will prevent Techno `Invoker` to be set !
//	return false;
//}

TechnoClass* AnimExt::GetTechnoInvoker(AnimClass* pThis, bool DealthByOwner)
{
	if (!DealthByOwner)
		return nullptr;

	auto const pExt = AnimExt::ExtMap.Find(pThis);
	if (pExt && pExt->Invoker)
		return pExt->Invoker;

	if (pThis->OwnerObject)
	{
		switch (pThis->OwnerObject->WhatAmI())
		{
		case AbstractType::Building:
		case AbstractType::Unit:
		case AbstractType::Infantry:
		case AbstractType::Aircraft:
			return static_cast<TechnoClass*>(pThis->OwnerObject);
		case AbstractType::Bullet:
			return static_cast<BulletClass*>(pThis->OwnerObject)->Owner;
		}
	}

	if (auto const pBullet = pThis->AttachedBullet)
		return pBullet->Owner;

	return nullptr;
}

Layer __fastcall AnimExt::GetLayer_patch(AnimClass* pThis, void* _)
{
	if (!pThis->OwnerObject)
		return pThis->Type ? pThis->Type->Layer : Layer::Air;

	const auto pExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (!pExt || !pExt->Layer_UseObjectLayer.isset())
		return Layer::Ground;

	if (pExt->Layer_UseObjectLayer.Get())
	{
		if (auto const pFoot = generic_cast<FootClass*>(pThis->OwnerObject))
		{
			if (auto const pLocomotor = pFoot->Locomotor.get())
				return pLocomotor->In_Which_Layer();
		}
		else if (auto const pBullet = specific_cast<BulletClass*>(pThis->OwnerObject))
			return pBullet->InWhichLayer();

		return pThis->OwnerObject->ObjectClass::InWhichLayer();
	}

	return pThis->Type ? pThis->Type->Layer : Layer::Air;
}

bool AnimExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		//.Process(AnimCellUpdater::Marked)
		.Success();
}

bool AnimExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		//.Process(AnimCellUpdater::Marked)
		.Success();
}

// =============================
// container
void AnimExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<AnimClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AnimExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<AnimClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

AnimExt::ExtContainer::ExtContainer() : Container("AnimClass") { }
AnimExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

//DEFINE_HOOK_AGAIN(0x4226F0, AnimClass_AltExt_CTOR, 0x6)
//DEFINE_HOOK_AGAIN(0x4228D2, AnimClass_AltExt_CTOR, 0x5)

//Only Extend Anim that Has "Type" Pointer
DEFINE_HOOK(0x422131, AnimClass_CTOR, 0x6)
{
	GET(AnimClass*, pItem, ESI);
	AnimExt::ExtMap.Allocate(pItem);
	return 0;
}

#ifdef ENABLE_NEWHOOKS
DEFINE_HOOK(0x426590, AnimClass_SDDTOR, 0x8)
{
	GET(AnimClass* const, pItem, ECX);
	GET_STACK(char, nFlag, 0x4);

	pItem->DestroyPointer();
	if ((nFlag & 1) != 0)
		YRMemory::Deallocate(pItem);

	AnimExt::ExtMap.Remove(pItem);

	R->EAX(pItem);
	return 0x4265AB;
}
#else
DEFINE_HOOK(0x422A59, AnimClass_DTOR, 0x6)
{
	GET(AnimClass* const, pItem, ESI);
	AnimExt::ExtMap.Remove(pItem);
	return 0;
}
#endif

DEFINE_HOOK_AGAIN(0x425280, AnimClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x4253B0, AnimClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(AnimClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	AnimExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK_AGAIN(0x425391, AnimClass_Load_Suffix, 0x7)
DEFINE_HOOK_AGAIN(0x4253A2, AnimClass_Load_Suffix, 0x7)
DEFINE_HOOK(0x425358, AnimClass_Load_Suffix, 0x7)
{
	AnimExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x4253FF, AnimClass_Save_Suffix, 0x5)
{
	AnimExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x425164, AnimClass_Detach, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	GET(void*, target, EDI);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	if (auto pAnimExt = AnimExt::ExtMap.Find(pThis))
	{
		pAnimExt->InvalidatePointer(target, all);
	}

	R->EBX(0);
	return pThis->OwnerObject == target && target ? 0x425174 : 0x4251A3;
}

//remove from CRC
//DEFINE_JUMP(LJMP, 0x42543A, 0x425448)
//
//DEFINE_HOOK_AGAIN(0x421EF4, AnimClass_CTOR_setD0, 0x6)
//DEFINE_HOOK(0x42276D, AnimClass_CTOR_setD0, 0x6)
//{
//	GET(AnimClass*, pThis, ESI);
//	pThis->unknown_D0 = 0;
//	return R->Origin() + 0x6;
//}
