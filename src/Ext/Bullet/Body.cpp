#include "Body.h"

#include <Ext/RadSite/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include "Trajectories/PhobosTrajectory.h"

#include <Utilities/Macro.h>

BulletExt::ExtContainer BulletExt::ExtMap;

BulletExt::ExtData::~ExtData()
{
	GameDelete<true>(Trajectory);
}

void BulletExt::ExtData::Uninitialize()
{


}

void BulletExt::ExtData::InitializeConstants() {

	this->LaserTrails.reserve(1);
#ifdef COMPILE_PORTED_DP_FEATURES
	this->Trails.reserve(1);
#endif
	//Type is not initialize here , wtf
}

void BulletExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved) {
	AnnounceInvalidPointer(Owner , ptr);

	if (Trajectory)
		Trajectory->InvalidatePointer(ptr, bRemoved);
 }

void BulletExt::ExtData::ApplyRadiationToCell(CellStruct const& Cell, int Spread, int RadLevel)
{
	auto pThis = this->Get();
	auto pWeapon = pThis->GetWeaponType();
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	auto pRadType = pWeaponExt->RadType;

		auto const it = std::find_if(RadSiteClass::Array_Constant->begin(), RadSiteClass::Array_Constant->end(),
			[=](auto const pSite) {

				auto const pRadExt = RadSiteExt::ExtMap.Find(pSite);
				if (pRadExt->Type != pRadType)
					return false;

				if (Map[pSite->BaseCell] != Map[Cell])
					return false;

				if (Spread != pSite->Spread)
					return false;

				if (pWeapon != pRadExt->Weapon)
					return false;

				if (pRadExt->TechOwner && pThis->Owner)
					return pRadExt->TechOwner == pThis->Owner;

				return true;
			});

		if (it != RadSiteClass::Array_Constant->end()) {
			if ((*it)->GetRadLevel() + RadLevel >= pRadType->GetLevelMax()) {
				RadLevel = pRadType->GetLevelMax() - (*it)->GetRadLevel();
			}

			auto const pRadExt = RadSiteExt::ExtMap.Find((*it));
			// Handle It
			pRadExt->Add(RadLevel);
			return;
		}

	RadSiteExt::CreateInstance(Cell, Spread, RadLevel, pWeaponExt, pThis->Owner);
}

void BulletExt::ExtData::InitializeLaserTrails(BulletTypeExt::ExtData* pTypeExt)
{
	auto pThis = Get();

	if (LaserTrails.size() || pThis->Type->Inviso)
		return;

	if (pTypeExt) {
		const auto pOwner = pThis->Owner ? pThis->Owner->Owner : (Owner ? Owner : HouseExt::FindCivilianSide());

		for (auto const& idxTrail: pTypeExt->LaserTrail_Types) {
			if (auto pLaserType = LaserTrailTypeClass::Array[idxTrail].get()) {
				LaserTrails.push_back(
					std::make_unique<LaserTrailClass>(pLaserType, pOwner->LaserColor));
			}
		}
	}
}

void BulletExt::InterceptBullet(BulletClass* pThis, TechnoClass* pSource, WeaponTypeClass* pWeapon)
{
	auto const pExt = BulletExt::ExtMap.Find(pThis);
	auto const pTypeExt = pExt->TypeExt;
	bool canAffect = false;
	bool isIntercepted = false;

	if (pTypeExt->Armor.isset())
	{
		double versus = GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pTypeExt->Armor);

		if (((fabs(versus) >= 0.001)))
		{
			canAffect = true;
			pExt->CurrentStrength -= static_cast<int>(pWeapon->Damage * versus * TechnoExt::GetDamageMult(pSource));

			if (pExt->CurrentStrength <= 0)
				isIntercepted = true;
			else
				pExt->InterceptedStatus = InterceptedStatus::None;
		}
	}
	else
	{
		canAffect = true;
		isIntercepted = true;
	}

	if (canAffect)
	{
		bool bKeepIntact = true;
		if (pSource) {
			auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pSource->GetTechnoType());
			bKeepIntact = pTechnoTypeExt->Interceptor_KeepIntact.Get();
			auto const pWeaponOverride = pTechnoTypeExt->Interceptor_WeaponOverride.Get(pTypeExt->Interceptable_WeaponOverride.Get(nullptr));
			pExt->Intercepted_Detonate = !pTechnoTypeExt->Interceptor_DeleteOnIntercept.Get(pTypeExt->Interceptable_DeleteOnIntercept);

			if (pWeaponOverride)
			{
				bool replaceType = pTechnoTypeExt->Interceptor_WeaponReplaceProjectile;
				bool cumulative = pTechnoTypeExt->Interceptor_WeaponCumulativeDamage;

				pThis->WeaponType = pWeaponOverride;
				pThis->Health = cumulative ? pThis->Health + pWeaponOverride->Damage : pWeaponOverride->Damage;
				pThis->WH = pWeaponOverride->Warhead;
				pThis->Bright = pThis->WeaponType->Bright || pThis->WH->Bright;
				pThis->Speed = pWeaponOverride->Speed;

				if (replaceType && pWeaponOverride->Projectile != pThis->Type)
				{
					auto pNewProjTypeExt = BulletTypeExt::ExtMap.Find(pWeaponOverride->Projectile);

					if (!pNewProjTypeExt)
					{
						Debug::Log("Failed to find BulletTypeExt For [%s] ! \n", pWeaponOverride->Projectile->get_ID());
						return;
					}

					pExt->TypeExt = pNewProjTypeExt;

					pThis->Type = pWeaponOverride->Projectile;

					if (pExt->LaserTrails.size()) {
						pExt->LaserTrails.clear();
						pExt->InitializeLaserTrails(pNewProjTypeExt);
					}

#ifdef COMPILE_PORTED_DP_FEATURES
					TrailsManager::CleanUp(pExt->Get());
					TrailsManager::Construct(pExt->Get());
#endif

					//LineTrailExt::DeallocateLineTrail(pThis);
					//LineTrailExt::ConstructLineTrails(pThis);
				}
			}
		}

		if (isIntercepted && !bKeepIntact) {
			pExt->InterceptedStatus = InterceptedStatus::Intercepted;
		}
	}
}

// =============================
// load / save

template <typename T>
void BulletExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->TypeExt)
		.Process(this->CurrentStrength)
		.Process(this->IsInterceptor)
		.Process(this->InterceptedStatus)
		.Process(this->Intercepted_Detonate)
		.Process(this->LaserTrails)
		.Process(this->SnappedToTarget)
		.Process(this->BrightCheckDone)
		.Process(this->Owner)
		.Process(this->Bouncing)
		.Process(this->LastObject)
		.Process(this->BounceAmount)
		.Process(this->BulletDir)
#ifdef COMPILE_PORTED_DP_FEATURES
		.Process(this->Trails)
#endif
		;

	this->Trajectory = PhobosTrajectory::ProcessFromStream(Stm, this->Trajectory);
}

void BulletExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BulletClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void BulletExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BulletClass>::Serialize(Stm);
	this->Serialize(Stm);
}

//void BulletExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

bool BulletExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool BulletExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

BulletExt::ExtContainer::ExtContainer() : Container("BulletClass") { }
BulletExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4664BA, BulletClass_CTOR, 0x5)
{
	GET(BulletClass*, pItem, ESI);

#ifndef ENABLE_NEWEXT
	BulletExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	BulletExt::ExtMap.FindOrAllocate(pItem);
#endif

	return 0;
}

DEFINE_HOOK(0x4665E9, BulletClass_DTOR, 0xA)
{
	GET(BulletClass*, pItem, ESI);
	BulletExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x46AFB0, BulletClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x46AE70, BulletClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BulletClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK_AGAIN(0x46AF97, BulletClass_Load_Suffix, 0x7)
DEFINE_HOOK(0x46AF9E, BulletClass_Load_Suffix, 0x7)
{
	BulletExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46AFC4, BulletClass_Save_Suffix, 0x3)
{
	BulletExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x4685BE, BulletClass_Detach, 0x6)
{
	GET(BulletClass*, pThis, ESI);
	GET(void*, target, EDI);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	if (auto pExt = BulletExt::ExtMap.Find(pThis))
		pExt->InvalidatePointer(target, all);

	return pThis->NextAnim == target ? 0x4685C6 :0x4685CC;
}

static void __fastcall BulletClass_AnimPointerExpired(BulletClass* pThis, void* _, AnimClass* pTarget)
{
	pThis->ObjectClass::AnimPointerExpired(pTarget);
}

DEFINE_JUMP(VTABLE, 0x7E4744, GET_OFFSET(BulletClass_AnimPointerExpired))