#pragma once

#include <AnimTypeClass.h>
#include <Utilities/Container.h>
#include <Utilities/Enum.h>
#include <Utilities/TemplateDefB.h>

#include <New/Entity/LauchSWData.h>
#include <New/AnonymousType/Spawns.h>
//#include "AnimSpawnerDatas.h"

class AnimTypeExtData final
{
public:
	using base_type = AnimTypeClass;
	static constexpr size_t Canary = 0xEEECEEEE;
	//static constexpr size_t ExtOffset = 0x374;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	Valueable<PaletteManager*> Palette {}; //CustomPalette::PaletteMode::Temperate
	bool CreateUnit_Scatter { false };
	bool CreateUnit_AI_Scatter { false };
	bool MakeInfantry_Scatter { false };
	bool MakeInfantry_AI_Scatter { false };

#pragma region CreateUnit
	Valueable<UnitTypeClass*> CreateUnit {};
	Valueable<DirType> CreateUnit_Facing { DirType::North };
	Valueable<bool> CreateUnit_InheritDeathFacings { false };
	Valueable<bool> CreateUnit_InheritTurretFacings { false };
	Nullable<bool> CreateUnit_RemapAnim { };
	Valueable<bool> CreateUnit_RandomFacing { true };
	Valueable<Mission> CreateUnit_Mission { Mission::Guard };
	Nullable<Mission> CreateUnit_AI_Mission { };

	Nullable<OwnerHouseKind> CreateUnit_Owner {};
	Valueable<bool> CreateUnit_ConsiderPathfinding { false };
	Valueable<AnimTypeClass*> CreateUnit_SpawnAnim { nullptr };
	Valueable<bool> CreateUnit_AlwaysSpawnOnGround { true };
	Valueable<bool> CreateUnit_KeepOwnerIfDefeated { true };
	Valueable<bool> CreateUnit_SpawnParachutedInAir { false };
#pragma endregion

	Valueable<int> XDrawOffset { 0 };
	Valueable<int> HideIfNoOre_Threshold { 0 };
	Nullable<bool> Layer_UseObjectLayer {};
	Valueable<bool> UseCenterCoordsIfAttached { false };

	Valueable<WeaponTypeClass*> Weapon { nullptr };
	Valueable<WeaponTypeClass*> WeaponToCarry {};
	Valueable<bool> Warhead_Detonate { false };

	Valueable<int> Damage_Delay { 0 };
	Valueable<bool> Damage_DealtByInvoker { false };
	Valueable<bool> Damage_ApplyOnce { false };
	Valueable<bool> Damage_ConsiderOwnerVeterancy { true };
	Nullable<DamageDelayTargetFlag> Damage_TargetFlag {};

	Nullable<Mission> MakeInfantry_Mission {};
	Nullable<Mission> MakeInfantry_AI_Mission {};

	NullableVector <AnimTypeClass*> SplashList {};
	Valueable<bool> SplashIndexRandom { false };

	Nullable<AnimTypeClass*> WakeAnim {};
	Valueable<bool> ExplodeOnWater { false };

	ValueableVector<AnimTypeClass*> SpawnsMultiple {};
	Valueable<bool> SpawnsMultiple_Random { false };
	std::vector<int> SpawnsMultiple_amouts {};

	Valueable<double> ParticleRangeMin { 0.0 };
	Valueable<double> ParticleRangeMax { 0.0 };
	Nullable<int> ParticleChance {};
	Valueable<bool> SpawnParticleModeUseAresCode { true };

	std::vector<LauchSWData> Launchs {};

	Valueable<int> CraterDecreaseTiberiumAmount { 6 };
	Valueable<double> CraterChance { 0.5 };
	Nullable<bool> SpawnCrater { };
	Nullable<double> ScorchChance { };

	Valueable<bool> SpecialDraw { false };
	Valueable<bool> NoOwner { false };

	Valueable<int> Spawns_Delay { 0 };

	Valueable<double> ConcurrentChance { 0.0 };
	ValueableVector<AnimTypeClass*> ConcurrentAnim { };
	Nullable<OwnerHouseKind> MakeInfantryOwner {};
	Valueable<ParticleSystemTypeClass*> AttachedSystem {};

	bool IsInviso { false };

	Valueable<bool> RemapAnim { false };
	//AnimSpawnerDatas SpawnerDatas;

	Valueable<bool> AltPalette_ApplyLighting { false };
	Valueable<bool> ExtraShadow { true };
	NullableIdx<VocClass> DetachedReport {};

	Valueable<int> AdditionalHeight {};
	NullableIdx<VocClass> AltReport {};

	Spawns SpawnsData {};

	Valueable<AffectedHouse> VisibleTo { AffectedHouse::All };
	Valueable<bool> VisibleTo_ConsiderInvokerAsOwner { false };
	Valueable<bool> RestrictVisibilityIfCloaked { false };
	Valueable<bool> DetachOnCloak { true };
	Nullable<int> Translucency_Cloaked {};

	Valueable<double> Translucent_Stage1_Percent { 0.2 };
	Nullable<int> Translucent_Stage1_Frame {};
	Valueable<TranslucencyLevel> Translucent_Stage1_Translucency { 25 };
	Valueable<double> Translucent_Stage2_Percent { 0.4 };
	Nullable<int> Translucent_Stage2_Frame {};
	Valueable<TranslucencyLevel> Translucent_Stage2_Translucency { 50 };
	Valueable<double> Translucent_Stage3_Percent { 0.6 };
	Nullable<int> Translucent_Stage3_Frame {};
	Valueable<TranslucencyLevel> Translucent_Stage3_Translucency { 75 };

	Nullable<int> CreateUnit_SpawnHeight {};

	Valueable<bool> ConstrainFireAnimsToCellSpots { true };
	Nullable<LandTypeFlags> FireAnimDisallowedLandTypes {};
	Nullable<bool> AttachFireAnimsToParent { false };
	Nullable<int> SmallFireCount {};
	ValueableVector<AnimTypeClass*> SmallFireAnims {};
	ValueableVector<double> SmallFireChances {};
	ValueableVector<double> SmallFireDistances {};
	Valueable<int> LargeFireCount { 1 };
	ValueableVector<AnimTypeClass*> LargeFireAnims {};
	ValueableVector<double> LargeFireChances {};
	ValueableVector<double> LargeFireDistances {};

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void Initialize();

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	void ValidateSpalshAnims();

	constexpr OwnerHouseKind GetAnimOwnerHouseKind()
	{
		if (this->CreateUnit && !this->CreateUnit_Owner.isset())
			return OwnerHouseKind::Victim;

		if (this->AttachedToObject->MakeInfantry > -1 && !this->MakeInfantryOwner.isset())
			return OwnerHouseKind::Invoker;

		if (this->CreateUnit_Owner.isset())
			return this->CreateUnit_Owner;

		if (this->MakeInfantryOwner.isset())
			return this->MakeInfantryOwner;

		return OwnerHouseKind::Invoker;
	}

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(AnimTypeExtData) -
			(4u //AttachedToObject
			 );
	}

	static void ProcessDestroyAnims(FootClass* pThis, TechnoClass* pKiller = nullptr , WarheadTypeClass* pWH = nullptr);
	static void CreateUnit_MarkCell(AnimClass* pThis);
	static void CreateUnit_Spawn(AnimClass* pThis);

	constexpr bool ScatterCreateUnit(bool IsAi) {
		return IsAi ? this->CreateUnit_AI_Scatter : this->CreateUnit_Scatter;
	}

	constexpr bool ScatterAnimToInfantry(bool IsAi) {
		return !IsAi ? this->MakeInfantry_Scatter : this->MakeInfantry_AI_Scatter;
	}

	constexpr  Mission GetCreateUnitMission(bool IsAi) {
		auto result = this->CreateUnit_Mission;

		if (IsAi && this->CreateUnit_AI_Mission.isset())
			result = this->CreateUnit_AI_Mission;

		return result;
	}

	constexpr Mission GetAnimToInfantryMission(bool IsAi) {
		auto result = this->MakeInfantry_Mission.Get(Mission::Hunt);

		if (IsAi && this->MakeInfantry_AI_Mission.isset())
			result = this->MakeInfantry_AI_Mission;

		return result;
	}

	void ValidateData();
private:
	template <typename T>
	void Serialize(T& Stm);

};

class AnimClass;
class AnimTypeExtContainer final : public Container<AnimTypeExtData>
{
public:
	static AnimTypeExtContainer Instance;

	//CONSTEXPR_NOCOPY_CLASSB(AnimTypeExtContainer, AnimTypeExtData, "AnimTypeClass");
};