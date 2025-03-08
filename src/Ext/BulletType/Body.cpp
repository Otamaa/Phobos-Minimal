#include "Body.h"

#include <Ext/AnimType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/Macro.h>

BulletTypeClass* BulletTypeExtData::GetDefaultBulletType() {
	if(!RulesExtData::Instance()->DefautBulletType)
		RulesExtData::Instance()->DefautBulletType = BulletTypeClass::Find(DEFAULT_STR2);

	return RulesExtData::Instance()->DefautBulletType;
}

CoordStruct BulletTypeExtData::CalculateInaccurate(BulletTypeClass* pBulletType) {
	if (pBulletType->Inaccurate)
	{
		const auto pTypeExt = BulletTypeExtContainer::Instance.Find(pBulletType);
		const int scatterMax = pTypeExt->BallisticScatterMax.isset() ? (int)(pTypeExt->BallisticScatterMax.Get()) : RulesClass::Instance()->BallisticScatter;
		const int scatterMin = pTypeExt->BallisticScatterMin.isset() ? (int)(pTypeExt->BallisticScatterMin.Get()) : (scatterMax / 2);

		const double random = ScenarioClass::Instance()->Random.RandomRanged(scatterMin, scatterMax);
		const double theta = ScenarioClass::Instance()->Random.RandomDouble() * Math::TwoPi;

		CoordStruct offset
		{
			static_cast<int>(random * Math::cos(theta)),
			static_cast<int>(random * Math::sin(theta)),
			0
		};
		return offset;
	}

	return CoordStruct::Empty;
}

const ConvertClass* BulletTypeExtData::GetBulletConvert()
{
	if(!this->ImageConvert.empty())
		return this->ImageConvert;
	else
	{
		ConvertClass* pConvert = nullptr;
		if (const auto pAnimType = AnimTypeClass::Find(this->AttachedToObject->ImageFile)) {
			if(const auto pConvertData = AnimTypeExtContainer::Instance.Find(pAnimType)->Palette){
				pConvert = pConvertData->GetConvert<PaletteManager::Mode::Temperate>();
			}
		}

		this->ImageConvert = pConvert;
		return pConvert;
	}
}

BulletClass* BulletTypeExtData::CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, WeaponTypeClass* pWeapon, bool addDamage, bool SetWeaponType) const
{
	if (auto pBullet = this->CreateBullet(pTarget, pOwner, pWeapon->Damage, pWeapon->Warhead,
		pWeapon->Speed, WeaponTypeExtContainer::Instance.Find(pWeapon)->GetProjectileRange(), pWeapon->Bright || pWeapon->Warhead->Bright, addDamage))
	{
		if(SetWeaponType)
			pBullet->SetWeaponType(pWeapon);

		return pBullet;
	}

	return nullptr;
}

BulletClass* BulletTypeExtData::CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner, WeaponTypeClass* pWeapon) const
{
	if (auto pBullet = this->CreateBullet(pTarget, pOwner, pWeapon->Damage, pWeapon->Warhead,
		pWeapon->Speed, WeaponTypeExtContainer::Instance.Find(pWeapon)->GetProjectileRange(), pWeapon->Bright || pWeapon->Warhead->Bright, true))
	{
		pBullet->SetWeaponType(pWeapon);
		return pBullet;
	}

	return nullptr;
}

BulletClass* BulletTypeExtData::CreateBullet(AbstractClass* pTarget, TechnoClass* pOwner,
	int damage, WarheadTypeClass* pWarhead, int speed, int range, bool bright, bool addDamage) const
{
	damage = (int)(TechnoExtData::GetDamageMult(pOwner , damage , !addDamage));

	auto pBullet = this->AttachedToObject->CreateBullet(pTarget, pOwner, damage, pWarhead, speed, bright);

	if (pBullet)
	{
		pBullet->Range = range;
	}

	return pBullet;
}

// =============================
// load / save

void BulletTypeExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->AttachedToObject;
	auto pArtInI = &CCINIClass::INI_Art;

	const char* pSection = pThis->ID;
	const char* pArtSection = (!pThis->ImageFile || !pThis->ImageFile[0]) ? pSection : pThis->ImageFile;

	this->ImageConvert.clear();
	bool trailReaded = false;

	if (!parseFailAddr)
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
		this->Shrapnel_Chance.Read(exINI, pSection, "Shrapnel.Chance");
		this->Shrapnel_UseWeaponTargeting.Read(exINI, pSection, "Shrapnel.UseWeaponTargeting");

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
		this->RetargetOwner.Read(exINI, pSection, "RetargetSelf");
		this->RetargetSelf_Probability.Read(exINI, pSection, "RetargetSelf.Probability");
		this->Splits.Read(exINI, pSection, "Splits");
		this->AroundTarget.Read(exINI, pSection, "AroundTarget");
		this->AirburstWeapons.Read(exINI, pSection, "AirburstWeapons");
		this->Airburst_RandomClusters.Read(exINI, pSection, "Airburst.RandomClusters");
		this->Airburst_UseCluster.Read(exINI, pSection, "Airburst.UseCluster");

		this->Splits_Range.Read(exINI, pSection, "Splits.TechnoRange");
		this->Splits_Range.Read(exINI, pSection, "Splits.TargetingDistance");

		this->Splits_TargetCellRange.Read(exINI, pSection, "Splits.TargetCellRange");
		this->Splits_UseWeaponTargeting.Read(exINI, pSection, "Splits.UseWeaponTargeting");
		this->AirburstWeapon_ApplyFirepowerMult.Read(exINI, pSection, "AirburstWeapon.ApplyFirepowerMult");

		this->Splits_RandomCellUseHarcodedRange.Read(exINI, pSection, "Splits.RandomCellUseHardcodedRange");
		this->Splits_TargetingUseVerses.Read(exINI, pSection, "Splits.TargetingUseVerses");
		this->Splits_FillRemainingClusterWithRandomcells.Read(exINI, pSection, "Splits.FillRemainingClusterWihRandomCells");

		this->Cluster_Scatter_Min.Read(exINI, pSection, "ClusterScatter.Min");
		this->Cluster_Scatter_Max.Read(exINI, pSection, "ClusterScatter.Max");

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
		this->Parachuted.Read(exINI, pSection, "Parachuted");
		this->AnimLength.Read(exINI, pThis->ID, "AnimLength");
		this->Arcing_AllowElevationInaccuracy.Read(exINI, pSection, "Arcing.AllowElevationInaccuracy");
		this->AttachedSystem.Read(exINI, pSection, "AttachedSystem");
		this->ReturnWeapon.Read(exINI, pSection, "ReturnWeapon" , true);

		if (pThis->Inviso) {
			trailReaded = true;
			this->LaserTrail_Types.Read(exINI, pSection, "LaserTrail.Types");
			this->Trails.Read(exINI, pSection, false);
		}
	}

	if (pArtInI && pArtInI->GetSection(pArtSection)){
		INI_EX exArtINI(pArtInI);

		//LineTrailData::LoadFromINI(this->LineTrailData, exArtINI, pArtSection);
		this->Parachute.Read(exArtINI, pArtSection, GameStrings::Parachute());
		if(!trailReaded) {
			this->LaserTrail_Types.Read(exArtINI, pArtSection, "LaserTrail.Types");
			this->Trails.Read(exArtINI, pArtSection, false);
		}
	}
}

template <typename T>
void BulletTypeExtData::Serialize(T& Stm)
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
		.Process(this->Shrapnel_Chance)
		.Process(this->Shrapnel_UseWeaponTargeting)
		.Process(this->SubjectToLand)
		.Process(this->SubjectToLand_Detonate)
		.Process(this->SubjectToWater)
		.Process(this->SubjectToWater_Detonate)
		.Process(this->AAOnly)
		.Process(this->SubjectToSolid)
		.Process(this->Solid_Level)

		.Process(this->Cluster_Scatter_Min)
		.Process(this->Cluster_Scatter_Max)
		.Process(this->Interceptable_DeleteOnIntercept)
		.Process(this->Interceptable_WeaponOverride)

		.Process(this->Parachute)
		.Process(this->MissileROTVar)
		.Process(this->MissileSafetyAltitude)
		.Process(this->Splits)
		.Process(this->RetargetAccuracy)
		.Process(this->RetargetOwner)
		.Process(this->RetargetSelf_Probability)
		.Process(this->AirburstSpread)
		.Process(this->AroundTarget)
		.Process(this->AirburstWeapons)
		.Process(this->Airburst_UseCluster)
		.Process(this->Airburst_RandomClusters)
		.Process(this->Splits_Range)
		.Process(this->Splits_RandomCellUseHarcodedRange)
		.Process(this->Splits_TargetCellRange)
		.Process(this->Splits_UseWeaponTargeting)
		.Process(this->AirburstWeapon_ApplyFirepowerMult)
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
		//.Process(this->LineTrailData)
		.Process(this->BallisticScatterMin)
		.Process(this->BallisticScatterMax)
		.Process(this->ImageConvert)
		.Process(this->Parachuted)
		.Process(this->AnimLength)
		.Process(this->Arcing_AllowElevationInaccuracy)
		.Process(this->AttachedSystem)
		.Process(this->ReturnWeapon)
		;

	this->Trails.Serialize(Stm);

	PhobosTrajectoryType::ProcessFromStream(Stm, this->TrajectoryType);
}

// =============================
// container

BulletTypeExtContainer BulletTypeExtContainer::Instance;

bool BulletTypeExtContainer::Load(BulletTypeClass* key, IStream* pStm)
{
	// this really shouldn't happen
	if (!key)
	{
		//Debug::LogInfo("[LoadKey] Attempted for a null pointer! WTF!");
		return false;
	}

	auto ptr = BulletTypeExtContainer::Instance.Map.get_or_default(key);

	if (!ptr) {
		ptr = BulletTypeExtContainer::Instance.Map.insert_unchecked(key, this->AllocateUnchecked(key));
	}

	this->ClearExtAttribute(key);
	this->SetExtAttribute(key, ptr);

	PhobosByteStream loader { 0 };
	if (loader.ReadBlockFromStream(pStm))
	{
		PhobosStreamReader reader { loader };
		if (reader.Expect(BulletTypeExtData::Canary)
			&& reader.RegisterChange(ptr))
		{
			ptr->LoadFromStream(reader);
			if (reader.ExpectEndOfBlock())
				return true;
		}
	}

	return false;
}
// =============================
// container hooks

DEFINE_HOOK(0x46BDD9, BulletTypeClass_CTOR, 0x5)
{
	GET(BulletTypeClass*, pItem, EAX);
	//BulletTypeExtContainer::Instance.Allocate(pItem);

	auto ptr = BulletTypeExtContainer::Instance.Map.get_or_default(pItem);

	if (!ptr) {
		ptr = BulletTypeExtContainer::Instance.Map.insert_unchecked(pItem,
			  BulletTypeExtContainer::Instance.AllocateUnchecked(pItem));
	}

	BulletTypeExtContainer::Instance.SetExtAttribute(pItem, ptr);
	return 0;
}

DEFINE_HOOK(0x46C8B6, BulletTypeClass_SDDTOR, 0x6)
{
	GET(BulletTypeClass*, pItem, ESI);
	auto extData = BulletTypeExtContainer::Instance.GetExtAttribute(pItem);
	BulletTypeExtContainer::Instance.ClearExtAttribute(pItem);
	BulletTypeExtContainer::Instance.Map.erase(pItem);
	if(extData)
		DLLCallDTOR(extData);
	return 0;
}

#include <Misc/Hooks.Otamaa.h>

HRESULT __stdcall FakeBulletTypeClass::_Load(IStream* pStm)
{

	BulletTypeExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->BulletTypeClass::Load(pStm);

	if (SUCCEEDED(res))
		BulletTypeExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeBulletTypeClass::_Save(IStream* pStm, bool clearDirty)
{

	BulletTypeExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->BulletTypeClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		BulletTypeExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E495C, FakeBulletTypeClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4960, FakeBulletTypeClass::_Save)

DEFINE_HOOK_AGAIN(0x46C429, BulletTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x46C41C, BulletTypeClass_LoadFromINI, 0xA)
{
	GET(BulletTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x90);
	BulletTypeExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x46C429);

	return 0;
}
