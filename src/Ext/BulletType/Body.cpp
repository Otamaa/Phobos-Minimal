#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>

BulletTypeExt::ExtContainer BulletTypeExt::ExtMap;
BulletTypeExt::ExtData::~ExtData()
{
	if (TrajectoryType)
		GameDelete(TrajectoryType);
}

double BulletTypeExt::GetAdjustedGravity(BulletTypeClass* pType)
{
	auto const pData = BulletTypeExt::GetExtData(pType);
	auto const nGravity = pData->Gravity.Get(static_cast<double>(RulesClass::Instance->Gravity));
	return pType->Floater ? nGravity * 0.5 : nGravity;
}

BulletTypeExt::ExtData* BulletTypeExt::GetExtData(BulletTypeExt::base_type* pThis)
{
	return ExtMap.Find(pThis);
}

BulletTypeClass* BulletTypeExt::GetDefaultBulletType()
{
	return BulletTypeClass::FindOrAllocate(NONE_STR);
}

void BulletTypeExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved) {
	if (TrajectoryType)
		TrajectoryType->InvalidatePointer(ptr, bRemoved);
}

bool BulletTypeExt::ExtData::HasSplitBehavior()
{
	// behavior in FS: Splits defaults to Airburst.
	return this->Get()->Airburst || this->Splits.Get();
}

BulletClass* BulletTypeExt::ExtData::CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, WeaponTypeClass* pWeapon) const
{
	auto pExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	return this->CreateBullet(pTarget, pOwner, pWeapon->Damage, pWeapon->Warhead,
		pWeapon->Speed, pExt->GetProjectileRange(), pWeapon->Bright || pWeapon->Warhead->Bright);

}

BulletClass* BulletTypeExt::ExtData::CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner,
	int damage, WarheadTypeClass* pWarhead, int speed, int range, bool bright) const
{
	auto pBullet = this->Get()->CreateBullet(pTarget, pOwner, static_cast<int>(damage * TechnoExt::GetDamageMult(pOwner)), pWarhead, speed, bright);

	if (pBullet) {
		pBullet->Range = range;
	}

	return pBullet;
}

void  BulletTypeExt::ExtData::Uninitialize() {

}
// =============================
// load / save

void BulletTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->Get();
	auto pArtInI = &CCINIClass::INI_Art;

	const char *pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	INI_EX exArtINI(pArtInI);

	this->Health.Read(exINI, pSection, "Strength");
	this->Armor.Read(exINI, pSection, "Armor");
	this->Interceptable.Read(exINI, pSection, "Interceptable");
	this->Gravity.Read(exINI, pSection, "Gravity");
	this->Gravity_HeightFix.Read(exINI, pSection, "Gravity.HeightFix");
	PhobosTrajectoryType::CreateType(this->TrajectoryType, pINI, pSection, "Trajectory");
	this->Shrapnel_AffectsGround.Read(exINI, pSection, "Shrapnel.AffectsGround");
	this->Shrapnel_AffectsBuildings.Read(exINI, pSection, "Shrapnel.AffectsBuildings");

	#pragma region Otamaa
	this->MissileROTVar.Read(exINI, pSection, "MissileROTVar");
	this->MissileSafetyAltitude.Read(exINI, pSection, "MissileSafetyAltitude");
	#pragma endregion

	this->AirburstSpread.Read(exINI, pSection, "AirburstSpread");
	this->RetargetAccuracy.Read(exINI, pSection, "RetargetAccuracy");
	this->RetargetOwner.Read(exINI, pSection, "RetargetOwner");
	this->Splits.Read(exINI, pSection, "Splits");
	this->AroundTarget.Read(exINI, pSection, "AroundTarget");
	this->AirburstWeapons.Read(exINI, pSection, "AirburstWeapons");

	this->Splits_Range.Read(exINI, pSection, "Splits.TechnoRange");
	this->Splits_RandomCellUseHarcodedRange.Read(exINI, pSection, "Splits.RandomCellUseHardcodedRange");
	this->Splits_TargetingUseVerses.Read(exINI, pSection ,"Splits.TargetingUseVerses");

	if (!pArtInI->GetSection(pArtSection))
		pArtSection = pSection;

	this->LaserTrail_Types.Read(exArtINI, pArtSection, "LaserTrail.Types");

	this->Cluster_Scatter_Min.Read(exINI, pSection, "ClusterScatter.Min");
	this->Cluster_Scatter_Max.Read(exINI, pSection, "ClusterScatter.Max");

	// Ares 0.7
	this->BallisticScatter_Min.Read(exINI, pSection, "BallisticScatter.Min");
	this->BallisticScatter_Max.Read(exINI, pSection, "BallisticScatter.Max");

	this->Interceptable_DeleteOnIntercept.Read(exINI, pSection, "Interceptable.DeleteOnIntercept");
	this->Interceptable_WeaponOverride.Read(exINI, pSection, "Interceptable.WeaponOverride", true);

#pragma region Otamaa
	this->BounceAmount.Read(exArtINI, pArtSection, "Bounce.Amount");
	this->BounceHitWeapon.Read(exArtINI, pArtSection, "Bounce.HitWeapon" , true);
	this->BounceOnTerrain.Read(exArtINI, pArtSection, "Bounce.OnTerrain");
	this->BounceOnBuilding.Read(exArtINI, pArtSection, "Bounce.OnBuilding");
	this->BounceOnInfantry.Read(exArtINI, pArtSection, "Bounce.OnInfantry");
	this->BounceOnVehicle.Read(exArtINI, pArtSection, "Bounce.OnVehicle");
	this->Parachute.Read(exArtINI, pArtSection, "Parachute");
	this->PreExplodeRange.Read(exINI, pSection, "PreExplode.Range");
	this->Trajectory_Speed.Read(exINI, pSection, "Trajectory.Speed");

#ifdef COMPILE_PORTED_DP_FEATURES
	this->Trails.Read(exArtINI, pArtSection, false);
#endif
	#pragma endregion

}

template <typename T>
void BulletTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Health)
		.Process(this->Armor)
		.Process(this->Interceptable)
		.Process(this->LaserTrail_Types)
		.Process(this->Gravity)
		.Process(this->Gravity_HeightFix)
		.Process(this->Shrapnel_AffectsGround)
		.Process(this->Shrapnel_AffectsBuildings)
		.Process(this->Cluster_Scatter_Min)
		.Process(this->Cluster_Scatter_Max)
		.Process(this->BallisticScatter_Min)
		.Process(this->BallisticScatter_Max)
		.Process(this->Interceptable_DeleteOnIntercept)
		.Process(this->Interceptable_WeaponOverride)

		.Process(this->Parachute)
		.Process(this->MissileROTVar)
		.Process(this->MissileSafetyAltitude)
		.Process(this->Splits)
		.Process(this->RetargetAccuracy)
		.Process(this->RetargetOwner)
		.Process(this->AirburstSpread)
		.Process(this->AroundTarget)
		.Process(this->AirburstWeapons)
		.Process(this->Splits_Range)
		.Process(this->Splits_RandomCellUseHarcodedRange)
		.Process(this->Splits_TargetingUseVerses)
		.Process(this->BounceAmount)
		.Process(this->BounceHitWeapon)
		.Process(this->BounceOnTerrain)
		.Process(this->BounceOnBuilding)
		.Process(this->BounceOnInfantry)
		.Process(this->BounceOnVehicle)
		.Process(this->PreExplodeRange)
		.Process(this->Trajectory_Speed)
		;
#ifdef COMPILE_PORTED_DP_FEATURES
	this->Trails.Serialize(Stm);
#endif
	this->TrajectoryType = PhobosTrajectoryType::ProcessFromStream(Stm, this->TrajectoryType);
}

void BulletTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BulletTypeClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void BulletTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BulletTypeClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void BulletTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) {}

bool BulletTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool BulletTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

BulletTypeExt::ExtContainer::ExtContainer() : Container("BulletTypeClass") { }
BulletTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x46BDD9, BulletTypeClass_CTOR, 0x5)
{
	GET(BulletTypeClass*, pItem, EAX);
#ifdef ENABLE_NEWHOOKS
	BulletTypeExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	BulletTypeExt::ExtMap.FindOrAllocate(pItem);
#endif
	return 0;
}

DEFINE_HOOK(0x46C8B6, BulletTypeClass_SDDTOR, 0x6)
{
	GET(BulletTypeClass*, pItem, ESI);
	BulletTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x46C730, BulletTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x46C6A0, BulletTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BulletTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x46C722, BulletTypeClass_Load_Suffix, 0x4)
{
	BulletTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46C74A, BulletTypeClass_Save_Suffix, 0x3)
{
	BulletTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x46C429, BulletTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x46C41C, BulletTypeClass_LoadFromINI, 0xA)
{
	GET(BulletTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x90);

	BulletTypeExt::ExtMap.LoadFromINI(pItem,pINI);
	pItem->Strength = 0;
	pItem->Armor = Armor::None;

	return 0;
}
