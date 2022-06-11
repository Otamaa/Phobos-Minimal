#include "Body.h"

#include <Ext/RadSite/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

template<> const DWORD TExtension<BulletClass>::Canary = 0x2A2A2A2A;
BulletExt::ExtContainer BulletExt::ExtMap;

void BulletExt::ExtData::InitializeConstants() {
	//Type is not initialize here , wtf
}

void BulletExt::ExtData::ApplyRadiationToCell(CellStruct const& Cell, int Spread, int RadLevel)
{
	auto pThis = this->OwnerObject();
	auto pWeapon = pThis->GetWeaponType();
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	auto pRadType = pWeaponExt->RadType;

	if (RadSiteExt::Array.Count > 0) {
		auto const it = std::find_if(RadSiteExt::Array.begin(), RadSiteExt::Array.end(),
			[=](RadSiteExt::ExtData* const pSite) {
				if (pSite->Type != pRadType)
					return false;

				if (pSite->OwnerObject()->BaseCell != Cell)
					return false;

				if (Spread != pSite->OwnerObject()->Spread)
					return false;

				if (pWeapon != pSite->Weapon)
					return false;

				return true;
			});

		if (it != RadSiteExt::Array.end()) {
			if ((*it)->OwnerObject()->GetRadLevel() + RadLevel >= pRadType->GetLevelMax()) {
				RadLevel = pRadType->GetLevelMax() - (*it)->OwnerObject()->GetRadLevel();
			}

			// Handle It
			(*it)->Add(RadLevel);
			return;
		}
	}

	RadSiteExt::CreateInstance(Cell, Spread, RadLevel, pWeaponExt, pThis->Owner);
}

void BulletExt::ExtData::InitializeLaserTrails(BulletTypeExt::ExtData* pTypeExt)
{
	auto pThis = OwnerObject();

	if (LaserTrails.size() || pThis->Type->Inviso)
		return;

	if (pTypeExt) {
		auto pOwner = pThis->Owner ? pThis->Owner->Owner : (Owner ? Owner : HouseExt::FindCivilianSide());
		auto OwnerLaserColor = pOwner->LaserColor;

		for (auto const& idxTrail: pTypeExt->LaserTrail_Types) {
			if (auto pLaserType = LaserTrailTypeClass::Array[idxTrail].get()) {
				LaserTrails.push_back(
					std::make_unique<LaserTrailClass>(pLaserType, OwnerLaserColor));
			}
		}
	}
}

void BulletExt::InterceptBullet(BulletClass* pThis, TechnoClass* pSource, WeaponTypeClass* pWeapon)
{
	if (!pThis || !pSource || !pWeapon)
		return;

	auto const pExt = BulletExt::GetExtData(pThis);
	auto const pTypeExt = pExt->TypeExt;

	if (!pExt || !pTypeExt)
		return;

	pExt->InterceptedStatus = InterceptedStatus::Targeted;

	bool canAffect = false;
	bool isIntercepted = false;

	if ((int)pTypeExt->Armor >= 0)
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
		auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pSource->GetTechnoType());
		if (!pTechnoTypeExt)
			return;

		auto const pWeaponOverride = pTechnoTypeExt->Interceptor_WeaponOverride.Get(pTypeExt->Interceptable_WeaponOverride.Get(nullptr));
		bool detonate = !pTechnoTypeExt->Interceptor_DeleteOnIntercept.Get(pTypeExt->Interceptable_DeleteOnIntercept);

		pExt->Intercepted_Detonate = detonate;

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
				TrailsManager::CleanUp(pExt->OwnerObject());
				TrailsManager::Construct(pExt->OwnerObject());
#endif
			}
		}

		if (isIntercepted) {
			if (!pTechnoTypeExt->Interceptor_KeepIntact)
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
		.Process(this->BrightCheckDone)
		.Process(this->Owner)
#ifdef COMPILE_PORTED_DP_FEATURES
		.Process(this->Trails)
#endif
		;

	this->Trajectory = PhobosTrajectory::ProcessFromStream(Stm, this->Trajectory);
}

void BulletExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	//Extension<BulletClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BulletExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	//Extension<BulletClass>::SaveToStream(Stm);
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

BulletExt::ExtContainer::ExtContainer() : TExtensionContainer("BulletClass") { }
BulletExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4664BA, BulletClass_CTOR, 0x5)
{
	GET(BulletClass*, pItem, ESI);

	//BulletExt::ExtMap.FindOrAllocate(pItem);
	ExtensionWrapper::GetWrapper(pItem)->CreateExtensionObject<BulletExt::ExtData>(pItem);

	return 0;
}

DEFINE_HOOK(0x4665E9, BulletClass_DTOR, 0xA)
{
	GET(BulletClass*, pItem, ESI);
	//BulletExt::ExtMap.Remove(pItem);
	ExtensionWrapper::GetWrapper(pItem)->DestoryExtensionObject();

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

	if (auto pExt = BulletExt::GetExtData(pThis))
		pExt->InvalidatePointer(target, all);

	return pThis->NextAnim == target ? 0x4685C6 :0x4685CC;
}