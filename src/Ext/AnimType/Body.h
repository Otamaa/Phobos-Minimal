#pragma once

#include <AnimTypeClass.h>

#include <Utilities/Enum.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDefB.h>

#include <New/Entity/LauchSWData.h>
//#include "AnimSpawnerDatas.h"

class AnimTypeExt
{
public:
	class ExtData final : public Extension<AnimTypeClass>
	{
	public:
		using base_type = AnimTypeClass;
		static constexpr size_t Canary = 0xEEEEEEEE;
		//static constexpr size_t ExtOffset = 0x374;

	public:
		Valueable<PaletteManager*> Palette; //CustomPalette::PaletteMode::Temperate
		Valueable<UnitTypeClass*> CreateUnit;
		Valueable<DirType> CreateUnit_Facing;
		Valueable<bool> CreateUnit_InheritDeathFacings;
		Valueable<bool> CreateUnit_InheritTurretFacings;
		Valueable<bool> CreateUnit_RemapAnim;
		Valueable<bool> CreateUnit_RandomFacing;
		Valueable<Mission> CreateUnit_Mission;
		Valueable<OwnerHouseKind> CreateUnit_Owner;
		Valueable<bool> CreateUnit_ConsiderPathfinding;
		Nullable<AnimTypeClass*> CreateUnit_SpawnAnim;
		Valueable<bool> CreateUnit_AlwaysSpawnOnGround;
		Valueable<bool> CreateUnit_KeepOwnerIfDefeated;

		Valueable<int> XDrawOffset;
		Valueable<int> HideIfNoOre_Threshold;
		Nullable<bool> Layer_UseObjectLayer;
		Valueable<bool> UseCenterCoordsIfAttached;
		Nullable<WeaponTypeClass*> Weapon;
		Valueable<int> Damage_Delay;
		Valueable<bool> Damage_DealtByInvoker;
		Valueable<bool> Damage_ApplyOnce;
		Valueable<bool> Damage_ConsiderOwnerVeterancy;
		Valueable<DamageDelayTargetFlag> Damage_TargetFlag;
		Nullable<Mission> MakeInfantry_Mission;

		Valueable<bool> Warhead_Detonate;

		#pragma region Otamaa
		NullableVector <AnimTypeClass*> SplashList;
		Valueable<bool> SplashIndexRandom;

		Nullable<AnimTypeClass*> WakeAnim;
		Valueable<bool> ExplodeOnWater;

		ValueableVector<AnimTypeClass*> SpawnsMultiple;
		Valueable<bool> SpawnsMultiple_Random;
		std::vector<int> SpawnsMultiple_amouts;

		Valueable<double> ParticleRangeMin;
		Valueable<double> ParticleRangeMax;
		Nullable<int> ParticleChance;

		std::vector<LauchSWData> Launchs;

		Valueable<int> CraterDecreaseTiberiumAmount;
		Valueable<double> CraterChance;
		Nullable<bool> SpawnCrater;
		Nullable<double> ScorchChance;
		Valueable<bool> SpecialDraw;
		Valueable<bool> NoOwner;

		Valueable<int> Spawns_Delay;

		Valueable<double> ConcurrentChance;
		ValueableVector<AnimTypeClass*> ConcurrentAnim;
		Valueable<bool> ShouldFogRemove;
		Valueable<OwnerHouseKind> MakeInfantryOwner;
	    #pragma endregion
		Valueable<ParticleSystemTypeClass*> AttachedSystem;
		bool IsInviso;

		//AnimSpawnerDatas SpawnerDatas;
		ExtData(AnimTypeClass* OwnerObject) : Extension<AnimTypeClass>(OwnerObject)
			, Palette { }
			, CreateUnit { }
			, CreateUnit_Facing { DirType::North }
			, CreateUnit_InheritDeathFacings { false }
			, CreateUnit_InheritTurretFacings { false }
			, CreateUnit_RemapAnim { false }
			, CreateUnit_RandomFacing { true }
			, CreateUnit_Mission { Mission::Guard }
			, CreateUnit_Owner { OwnerHouseKind::Victim }
			, CreateUnit_ConsiderPathfinding { false }
			, CreateUnit_SpawnAnim { }
			, CreateUnit_AlwaysSpawnOnGround { true }
			, CreateUnit_KeepOwnerIfDefeated { true }

			, XDrawOffset { 0 }
			, HideIfNoOre_Threshold { 0 }
			, Layer_UseObjectLayer {}
			, UseCenterCoordsIfAttached { false }
			, Weapon {}
			, Damage_Delay { 0 }
			, Damage_DealtByInvoker { false }
			, Damage_ApplyOnce{ false }
			, Damage_ConsiderOwnerVeterancy { true }
			, Damage_TargetFlag { DamageDelayTargetFlag::Cell }
			, Warhead_Detonate { false }

			, SplashList {}
			, SplashIndexRandom { false }
			, WakeAnim {}
			, ExplodeOnWater { false }

			, SpawnsMultiple {}
			, SpawnsMultiple_Random { false }
			, SpawnsMultiple_amouts {}

			, ParticleRangeMin { 0.0 }
			, ParticleRangeMax { 0.0 }
			, ParticleChance {}

			, Launchs {}

			, CraterDecreaseTiberiumAmount { 6 }
			, CraterChance { 0.5 }
			, SpawnCrater { }
			, ScorchChance { }
			, SpecialDraw { false }
			, NoOwner { false }
			, Spawns_Delay { 0 }

			, ConcurrentChance { 0.0 }
			, ConcurrentAnim { }
			, ShouldFogRemove { true }
			, MakeInfantryOwner { OwnerHouseKind::Invoker }
			, AttachedSystem {}
			, IsInviso { false }

			//, SpawnerDatas {}
		{ }

		virtual ~ExtData() override = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void Initialize();

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

		void ValidateSpalshAnims();
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<AnimTypeExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static void ProcessDestroyAnims(FootClass* pThis, TechnoClass* pKiller = nullptr);
	static OwnerHouseKind SetMakeInfOwner(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim);
	static void CreateUnit_MarkCell(AnimClass* pThis);
	static void CreateUnit_Spawn(AnimClass* pThis);
};
