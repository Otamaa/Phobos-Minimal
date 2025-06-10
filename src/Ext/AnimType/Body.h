#pragma once

#include <AnimTypeClass.h>
#include <Utilities/Container.h>
#include <Utilities/Enum.h>
#include <Utilities/TemplateDefB.h>

#include <New/Entity/LauchSWData.h>
#include <New/Type/PaletteManager.h>

#include <New/AnonymousType/CreateUnitTypeClass.h>

//#include <New/AnonymousType/Spawns.h>
//#include "AnimSpawnerDatas.h"

class AnimTypeExtData final
{
public:
	using base_type = AnimTypeClass;
	static COMPILETIMEEVAL size_t Canary = 0xEEECEEEE;
	//static COMPILETIMEEVAL size_t ExtOffset = 0x374;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	CustomPalette Palette { CustomPalette::PaletteMode::Temperate }; //CustomPalette::PaletteMode::Temperate

	bool MakeInfantry_Scatter { false };
	bool MakeInfantry_AI_Scatter { false };

#pragma region CreateUnit
    std::unique_ptr<CreateUnitTypeClass> CreateUnitType{};
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

	//Spawns SpawnsData {};

	Valueable<AffectedHouse> VisibleTo { AffectedHouse::All };
	Valueable<bool> VisibleTo_ConsiderInvokerAsOwner { false };
	Valueable<bool> RestrictVisibilityIfCloaked { false };
	Valueable<bool> DetachOnCloak { true };
	Nullable<int> Translucency_Cloaked {};

	Animatable<TranslucencyLevel> Translucent_Keyframes {};

	Valueable<int> CreateUnit_SpawnHeight { -1 };

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

	Valueable<bool> Damaging_UseSeparateState {};
	Valueable<int> Damaging_Rate { -1 };

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void Initialize();

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	void ValidateSpalshAnims();

	COMPILETIMEEVAL OwnerHouseKind GetAnimOwnerHouseKind()
	{
		if(this->CreateUnitType){
			return this->CreateUnitType->Owner.Get(OwnerHouseKind::Victim);
		}

		if (this->AttachedToObject->MakeInfantry > -1) {
			return this->MakeInfantryOwner.Get(OwnerHouseKind::Invoker);
		}

		return OwnerHouseKind::Invoker;
	}

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(AnimTypeExtData) -
			(4u //AttachedToObject
			 );
	}

	static void ProcessDestroyAnims(FootClass* pThis, TechnoClass* pKiller = nullptr , WarheadTypeClass* pWH = nullptr);
	static void CreateUnit_MarkCell(AnimClass* pThis);
	static void CreateUnit_Spawn(AnimClass* pThis);

	COMPILETIMEEVAL bool ScatterCreateUnit(bool IsAi) {
		return IsAi ? this->CreateUnitType->AI_Scatter : this->CreateUnitType->Scatter;
	}

	COMPILETIMEEVAL bool ScatterAnimToInfantry(bool IsAi) {
		return !IsAi ? this->MakeInfantry_Scatter : this->MakeInfantry_AI_Scatter;
	}

	COMPILETIMEEVAL  Mission GetCreateUnitMission(bool IsAi) {
		auto result = this->CreateUnitType->UnitMission;
		if (IsAi && this->CreateUnitType->AIUnitMission.isset())
			result = this->CreateUnitType->AIUnitMission;

		return result;
	}

	COMPILETIMEEVAL Mission GetAnimToInfantryMission(bool IsAi) {
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

class NOVTABLE FakeAnimTypeClass : public AnimTypeClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	AnimTypeExtData* _GetExtData() {
		return *reinterpret_cast<AnimTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeAnimTypeClass) == sizeof(AnimTypeClass), "Invalid Size !");