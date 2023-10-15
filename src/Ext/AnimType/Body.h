#pragma once

#include <AnimTypeClass.h>

#include <Utilities/Enum.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDefB.h>

#include <New/Entity/LauchSWData.h>
//#include "AnimSpawnerDatas.h"

class AnimTypeExtData final
{
public:
	using base_type = AnimTypeClass;
	static constexpr size_t Canary = 0xEEEEEEEE;
	//static constexpr size_t ExtOffset = 0x374;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	Valueable<PaletteManager*> Palette {}; //CustomPalette::PaletteMode::Temperate

#pragma region CreateUnit
	Valueable<UnitTypeClass*> CreateUnit {};
	Valueable<DirType> CreateUnit_Facing { DirType::North };
	Valueable<bool> CreateUnit_InheritDeathFacings { false };
	Valueable<bool> CreateUnit_InheritTurretFacings { false };
	Nullable<bool> CreateUnit_RemapAnim { };
	Valueable<bool> CreateUnit_RandomFacing { true };
	Valueable<Mission> CreateUnit_Mission { Mission::Guard };
	Nullable<OwnerHouseKind> CreateUnit_Owner {};
	Valueable<bool> CreateUnit_ConsiderPathfinding { false };
	Nullable<AnimTypeClass*> CreateUnit_SpawnAnim { };
	Valueable<bool> CreateUnit_AlwaysSpawnOnGround { true };
	Valueable<bool> CreateUnit_KeepOwnerIfDefeated { true };
#pragma endregion

	Valueable<int> XDrawOffset { 0 };
	Valueable<int> HideIfNoOre_Threshold { 0 };
	Nullable<bool> Layer_UseObjectLayer {};
	Valueable<bool> UseCenterCoordsIfAttached { false };

	Nullable<WeaponTypeClass*> Weapon {};
	Valueable<bool> Warhead_Detonate { false };

	Valueable<int> Damage_Delay { 0 };
	Valueable<bool> Damage_DealtByInvoker { false };
	Valueable<bool> Damage_ApplyOnce { false };
	Valueable<bool> Damage_ConsiderOwnerVeterancy { true };
	Nullable<DamageDelayTargetFlag> Damage_TargetFlag {};

	Nullable<Mission> MakeInfantry_Mission {};

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

	AnimTypeExtData(AnimTypeClass* OwnerObject) noexcept
	{
		AttachedToObject = OwnerObject;
	}

	~AnimTypeExtData() noexcept = default;

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void Initialize();

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	void ValidateSpalshAnims();

	OwnerHouseKind GetAnimOwnerHouseKind();

	static void ProcessDestroyAnims(FootClass* pThis, TechnoClass* pKiller = nullptr);
	static void CreateUnit_MarkCell(AnimClass* pThis);
	static void CreateUnit_Spawn(AnimClass* pThis);

private:
	template <typename T>
	void Serialize(T& Stm);

private:
	AnimTypeExtData(const AnimTypeExtData&) = delete;
	AnimTypeExtData& operator = (const AnimTypeExtData&) = delete;
	AnimTypeExtData& operator = (AnimTypeExtData&&) = delete;
};

class AnimTypeExtContainer final : public Container<AnimTypeExtData>
{
public:
	static AnimTypeExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(AnimTypeExtContainer, AnimTypeExtData, "AnimTypeClass");
};