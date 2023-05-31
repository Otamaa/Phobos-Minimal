#include "Body.h"

#include <Ext/AnimType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/Macro.h>

const Leptons BulletTypeExt::DefaultBulletScatterMin = Leptons { 256 };
const Leptons BulletTypeExt::DefaultBulletScatterMax = Leptons { 512 };

double BulletTypeExt::GetAdjustedGravity(BulletTypeClass* pType)
{
	auto const nGravity = BulletTypeExt::ExtMap.Find(pType)->Gravity.Get(static_cast<double>(RulesClass::Instance->Gravity));
	return pType->Floater ? nGravity * 0.5 : nGravity;
}

BulletTypeClass* BulletTypeExt::GetDefaultBulletType(const char* pBullet)
{
	BulletTypeClass* pType = nullptr;

	if (pBullet)
	{
		pType = BulletTypeClass::Find(pBullet);
	}
	else
	{
		pType = BulletTypeClass::Find(DEFAULT_STR2);
	}

	//an dummy bullet , huh
	return pType;
}

const ConvertClass* BulletTypeExt::ExtData::GetBulletConvert()
{
	if(!this->ImageConvert.empty())
		return  this->ImageConvert;
	else
	{
		ConvertClass* pConvert = nullptr;
		if (const auto pAnimType = AnimTypeClass::Find(this->Get()->ImageFile)) {
			if(const auto pConvertData = AnimTypeExt::ExtMap.Find(pAnimType)->Palette){
				pConvert = pConvertData->GetConvert<PaletteManager::Mode::Temperate>();
			}
		}

		this->ImageConvert = pConvert;
		return pConvert;
	}
}

bool BulletTypeExt::ExtData::HasSplitBehavior()
{
	// behavior in FS: Splits defaults to Airburst.
	return this->Get()->Airburst || this->Splits.Get();
}

BulletClass* BulletTypeExt::ExtData::CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, WeaponTypeClass* pWeapon) const
{
	if (auto pBullet = this->CreateBullet(pTarget, pOwner, pWeapon->Damage, pWeapon->Warhead,
		pWeapon->Speed, WeaponTypeExt::ExtMap.Find(pWeapon)->GetProjectileRange(), pWeapon->Bright || pWeapon->Warhead->Bright, true))
	{
		pBullet->SetWeaponType(pWeapon);
		return pBullet;
	}

	return nullptr;
}

BulletClass* BulletTypeExt::ExtData::CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner,
	int damage, WarheadTypeClass* pWarhead, int speed, int range, bool bright, bool addDamage) const
{
	if (addDamage)
		damage = (int)(damage * TechnoExt::GetDamageMult(pOwner));

	auto pBullet = this->Get()->CreateBullet(pTarget, pOwner, damage, pWarhead, speed, bright);

	if (pBullet)
	{
		pBullet->Range = range;
	}

	return pBullet;
}

void  BulletTypeExt::ExtData::Uninitialize()
{

}
// =============================
// load / save

void BulletTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->Get();
	auto pArtInI = &CCINIClass::INI_Art;

	const char* pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;
	this->ImageConvert.clear();

	if (pINI->GetSection(pSection))
	{
		INI_EX exINI(pINI);

		this->Health.Read(exINI, pSection, GameStrings::Strength());
		this->Armor.Read(exINI, pSection, GameStrings::Armor());
		this->Interceptable.Read(exINI, pSection, "Interceptable");
		this->Gravity.Read(exINI, pSection, GameStrings::Gravity());
		this->Gravity_HeightFix.Read(exINI, pSection, "Gravity.HeightFix");
		PhobosTrajectoryType::CreateType(this->TrajectoryType, pINI, pSection, "Trajectory");
		this->Shrapnel_AffectsGround.Read(exINI, pSection, "Shrapnel.AffectsGround");
		this->Shrapnel_AffectsBuildings.Read(exINI, pSection, "Shrapnel.AffectsBuildings");

		// Code Disabled , #816 , Bullet/Hooks.obstacles.cpp
		this->SubjectToLand.Read(exINI, pSection, "SubjectToLand");
		this->SubjectToLand_Detonate.Read(exINI, pSection, "SubjectToLand.Detonate");
		this->SubjectToWater.Read(exINI, pSection, "SubjectToWater");
		this->SubjectToWater_Detonate.Read(exINI, pSection, "SubjectToWater.Detonate");
		this->AAOnly.Read(exINI, pSection, "AAOnly");
		this->SubjectToSolid.Read(exINI, pSection, "SubjectToBuildings");
		this->Solid_Level.Read(exINI, pSection, "SolidLevel");

		//
#pragma region Otamaa
		this->MissileROTVar.Read(exINI, pSection, GameStrings::MissileROTVar());
		this->MissileSafetyAltitude.Read(exINI, pSection, GameStrings::MissileSafetyAltitude());
#pragma endregion

		this->AirburstSpread.Read(exINI, pSection, "AirburstSpread");
		this->RetargetAccuracy.Read(exINI, pSection, "RetargetAccuracy");
		this->RetargetOwner.Read(exINI, pSection, "RetargetOwner");
		this->Splits.Read(exINI, pSection, "Splits");
		this->AroundTarget.Read(exINI, pSection, "AroundTarget");
		this->AirburstWeapons.Read(exINI, pSection, "AirburstWeapons");

		this->Splits_Range.Read(exINI, pSection, "Splits.TechnoRange");
		this->Splits_RandomCellUseHarcodedRange.Read(exINI, pSection, "Splits.RandomCellUseHardcodedRange");
		this->Splits_TargetingUseVerses.Read(exINI, pSection, "Splits.TargetingUseVerses");
		this->Splits_FillRemainingClusterWithRandomcells.Read(exINI, pSection, "Splits.FillRemainingClusterWihRandomCells");

		this->Cluster_Scatter_Min.Read(exINI, pSection, "ClusterScatter.Min");
		this->Cluster_Scatter_Max.Read(exINI, pSection, "ClusterScatter.Max");

		// Ares 0.7
		this->BallisticScatter_Min.Read(exINI, pSection, "BallisticScatter.Min");
		this->BallisticScatter_Max.Read(exINI, pSection, "BallisticScatter.Max");

		this->Interceptable_DeleteOnIntercept.Read(exINI, pSection, "Interceptable.DeleteOnIntercept");
		this->Interceptable_WeaponOverride.Read(exINI, pSection, "Interceptable.WeaponOverride" , true);

		this->BallisticScatterMin.Read(exINI, pSection, "BallisticScatter.Min");
		this->BallisticScatterMax.Read(exINI, pSection, "BallisticScatter.Max");

		this->PreExplodeRange.Read(exINI, pSection, "PreExplodeRange");
		this->Trajectory_Speed.Read(exINI, pSection, "Trajectory.Speed");
		this->Proximity_Range.Read(exINI, pSection, "Proximity.Range");
		this->IsScalable.Read(exINI, pSection, GameStrings::Scalable());

		//code disabled , unfinished
		this->BounceAmount.Read(exINI, pSection, "Bounce.Amount");
		this->BounceHitWeapon.Read(exINI, pSection, "Bounce.HitWeapon", true);
		this->BounceOnTerrain.Read(exINI, pSection, "Bounce.OnTerrain");
		this->BounceOnBuilding.Read(exINI, pSection, "Bounce.OnBuilding");
		this->BounceOnInfantry.Read(exINI, pSection, "Bounce.OnInfantry");
		this->BounceOnVehicle.Read(exINI, pSection, "Bounce.OnVehicle");
		//

		this->SubjectToTrenches.Read(exINI, pSection, "SubjectToTrenches");
	}

	if (pArtInI && pArtInI->GetSection(pArtSection)){
		INI_EX exArtINI(pArtInI);

		this->LaserTrail_Types.Read(exArtINI, pArtSection, "LaserTrail.Types");
		//LineTrailData::LoadFromINI(this->LineTrailData, exArtINI, pArtSection);
#pragma region Otamaa

		this->Parachute.Read(exArtINI, pArtSection, GameStrings::Parachute());
#ifdef COMPILE_PORTED_DP_FEATURES
		this->Trails.Read(exArtINI, pArtSection, false);
#endif
#pragma endregion
	}
}

template <typename T>
void BulletTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Health)
		.Process(this->Armor)
		.Process(this->Interceptable)
		.Process(this->LaserTrail_Types)
		.Process(this->Gravity)
		.Process(this->Gravity_HeightFix)
		.Process(this->Shrapnel_AffectsGround)
		.Process(this->Shrapnel_AffectsBuildings)

		.Process(this->SubjectToLand)
		.Process(this->SubjectToLand_Detonate)
		.Process(this->SubjectToWater)
		.Process(this->SubjectToWater_Detonate)
		.Process(this->AAOnly)
		.Process(this->SubjectToSolid)
		.Process(this->Solid_Level)

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
		.Process(this->Splits_FillRemainingClusterWithRandomcells)
		.Process(this->BounceAmount)
		.Process(this->BounceHitWeapon)
		.Process(this->BounceOnTerrain)
		.Process(this->BounceOnBuilding)
		.Process(this->BounceOnInfantry)
		.Process(this->BounceOnVehicle)
		.Process(this->SubjectToTrenches)
		.Process(this->PreExplodeRange)
		.Process(this->Trajectory_Speed)
		.Process(this->Proximity_Range)
		.Process(this->IsScalable)
		.Process(this->LineTrailData)
		.Process(this->BallisticScatterMin)
		.Process(this->BallisticScatterMax)
		.Process(this->ImageConvert)
		;
#ifdef COMPILE_PORTED_DP_FEATURES
	this->Trails.Serialize(Stm);
#endif
	PhobosTrajectoryType::ProcessFromStream(Stm, this->TrajectoryType);
}

// =============================
// container
BulletTypeExt::ExtContainer BulletTypeExt::ExtMap;
BulletTypeExt::ExtContainer::ExtContainer() : Container("BulletTypeClass") { }
BulletTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x46BDD9, BulletTypeClass_CTOR, 0x5)
{
	GET(BulletTypeClass*, pItem, EAX);
	BulletTypeExt::ExtMap.Allocate(pItem);
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

// Before : 0x46C722 , 0x4
// After : 46C70F , 0x6
DEFINE_HOOK(0x46C70F, BulletTypeClass_Load_Suffix, 0x6)
{
	GET(BulletTypeClass*, pThis, ESI);

	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->ShrapnelWeapon);
	BulletTypeExt::ExtMap.LoadStatic();

	return 0x46C720;
}

// Before : 0x46C74A , 0x3
// After : 46C744 , 0x6
DEFINE_HOOK(0x46C744, BulletTypeClass_Save_Suffix, 0x6)
{
	GET(HRESULT, nRes, EAX);

	if (SUCCEEDED(nRes))
	{
		nRes = 0;
		BulletTypeExt::ExtMap.SaveStatic();
	}

	return 0x46C74A;
}

DEFINE_HOOK_AGAIN(0x46C429, BulletTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x46C41C, BulletTypeClass_LoadFromINI, 0xA)
{
	GET(BulletTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x90);

	BulletTypeExt::ExtMap.LoadFromINI(pItem, pINI , R->Origin() == 0x46C429);

	return 0;
}
