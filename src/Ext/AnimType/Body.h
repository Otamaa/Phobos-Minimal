#pragma once

#include <AnimTypeClass.h>

#include <Utilities/Container.Otamaa.h>
#include <Utilities/Enum.h>
#include <Utilities/Container.h>
#include <Utilities/Template.h>

#include <New/Entity/LauchSWData.h>

class AnimTypeExt
{
public:
	using base_type = AnimTypeClass;

	class ExtData final : public Extension<AnimTypeClass>
	{
	public:
		CustomPalette Palette;
		Valueable<UnitTypeClass*> CreateUnit;
		Valueable<unsigned short> CreateUnit_Facing;
		Valueable<bool> CreateUnit_InheritDeathFacings;
		Valueable<bool> CreateUnit_InheritTurretFacings;
		Valueable<bool> CreateUnit_RemapAnim;
		Valueable<bool> CreateUnit_RandomFacing;
		Valueable<Mission> CreateUnit_Mission;
		Valueable<OwnerHouseKind> CreateUnit_Owner;
		Valueable<bool> CreateUnit_ConsiderPathfinding;
		Valueable<int> XDrawOffset;
		Valueable<int> HideIfNoOre_Threshold;
		Nullable<bool> Layer_UseObjectLayer;
		Valueable<bool> UseCenterCoordsIfAttached;
		Nullable<WeaponTypeClass*> Weapon;
		Valueable<int> Damage_Delay;
		Valueable<bool> Damage_DealtByInvoker;
		Valueable<bool> Damage_ApplyOnce;
		Valueable<bool> Damage_ConsiderOwnerVeterancy;
		Valueable<bool> Warhead_Detonate;

		#pragma region Otamaa
		NullableVector <AnimTypeClass*> SplashList;
		Valueable<bool> SplashIndexRandom;

		Nullable<AnimTypeClass*> WakeAnim;
		Valueable<bool> ExplodeOnWater;

		ValueableVector<AnimTypeClass*> SpawnsMultiple;
		Valueable<bool> SpawnsMultiple_Random;
		DynamicVectorClass<int> SpawnsMultiple_amouts;

		Valueable<double> ParticleRangeMin;
		Valueable<double> ParticleRangeMax;
		Nullable<int> ParticleChance;

		std::vector<LauchSWData> Launchs;

		Valueable<double> CraterChance;
		Nullable<bool> SpawnCrater;
		Nullable<double> ScorchChance;
		Valueable<bool> SpecialDraw;
		Valueable<bool> NoOwner;
	    #pragma endregion
		ExtData(AnimTypeClass* OwnerObject) : Extension<AnimTypeClass>(OwnerObject)
			, Palette { CustomPalette::PaletteMode::Temperate }
			, CreateUnit { }
			, CreateUnit_Facing { 0 }
			, CreateUnit_InheritDeathFacings { false }
			, CreateUnit_InheritTurretFacings { false }
			, CreateUnit_RemapAnim { false }
			, CreateUnit_RandomFacing { true }
			, CreateUnit_Mission { Mission::Guard }
			, CreateUnit_Owner { OwnerHouseKind::Victim }
			, CreateUnit_ConsiderPathfinding { false }
			, XDrawOffset { 0 }
			, HideIfNoOre_Threshold { 0 }
			, Layer_UseObjectLayer {}
			, UseCenterCoordsIfAttached { false }
			, Weapon {}
			, Damage_Delay { 0 }
			, Damage_DealtByInvoker { false }
			, Damage_ApplyOnce{ false }
			, Damage_ConsiderOwnerVeterancy { true }
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

			, CraterChance { 0.5 }
			, SpawnCrater { }
			, ScorchChance { }
			, SpecialDraw { false }
			, NoOwner { false }
		{ }

		virtual ~ExtData() = default;
		//virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override { } //Init After INI Read
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void InitializeConstants() override {
			auto pObj = OwnerObject();
			SpecialDraw = IS_SAME_STR_(pObj->get_ID(), RING1_NAME);
			Launchs.reserve(2);
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void CleanUp()
		{
			SpawnsMultiple_amouts.Clear();
		}

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<AnimTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static const void ProcessDestroyAnims(UnitClass* pThis, TechnoClass* pKiller = nullptr);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
