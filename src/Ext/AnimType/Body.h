#pragma once

#include <AnimTypeClass.h>
#include <Utilities/Container.h>
#include <Utilities/Enum.h>
#include <Utilities/TemplateDefB.h>

#include <New/Entity/LauchSWData.h>
#include <New/Type/PaletteManager.h>

#include <New/AnonymousType/CreateUnitTypeClass.h>

#include <ext/ObjectType/Body.h>

class AnimTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = AnimTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "AnimTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "AnimTypeClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();


public:
#pragma region ClassMembers
	// ============================================================
	// Large aggregates (unknown internal alignment, place first)
	// ============================================================
	CustomPalette Palette;
	Animatable<TranslucencyLevel> Translucent_Keyframes;

	// ============================================================
	// 8-byte aligned: unique_ptr
	// ============================================================
	std::unique_ptr<CreateUnitTypeClass> CreateUnitType;

	// ============================================================
	// 24-byte aligned: vectors (group all together)
	// ============================================================
	NullableVector<AnimTypeClass*> SplashList;
	ValueableVector<AnimTypeClass*> SpawnsMultiple;
	std::vector<int> SpawnsMultiple_amouts;
	std::vector<LauchSWData> Launchs;
	ValueableVector<AnimTypeClass*> ConcurrentAnim;
	ValueableVector<AnimTypeClass*> SmallFireAnims;
	ValueableVector<double> SmallFireChances;
	ValueableVector<double> SmallFireDistances;
	ValueableVector<AnimTypeClass*> LargeFireAnims;
	ValueableVector<double> LargeFireChances;
	ValueableVector<double> LargeFireDistances;

	// ============================================================
	// Valueable<pointer> (8 bytes each)
	// ============================================================
	Valueable<WeaponTypeClass*> Weapon;
	Valueable<WeaponTypeClass*> WeaponToCarry;
	Valueable<ParticleSystemTypeClass*> AttachedSystem;

	// ============================================================
	// Nullable<pointer> (pointer + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<AnimTypeClass*> WakeAnim;

	// ============================================================
	// Valueable<double> (8 bytes each)
	// ============================================================
	Valueable<double> ParticleRangeMin;
	Valueable<double> ParticleRangeMax;
	Valueable<double> CraterChance;
	Valueable<double> ConcurrentChance;

	// ============================================================
	// Nullable<double> (double + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<double> ScorchChance;

	// ============================================================
	// Nullable with 4-byte inner types (int/enum + bool + padding ≈ 8 bytes)
	// ============================================================
	Nullable<int> YDrawOffset_BracketAdjust_Buildings;
	Nullable<int> ParticleChance;
	Nullable<int> Translucency_Cloaked;
	Nullable<int> SmallFireCount;
	Nullable<DamageDelayTargetFlag> Damage_TargetFlag;
	Nullable<Mission> MakeInfantry_Mission;
	Nullable<Mission> MakeInfantry_AI_Mission;
	Nullable<OwnerHouseKind> MakeInfantryOwner;
	Nullable<LandTypeFlags> FireAnimDisallowedLandTypes;

	// ============================================================
	// Nullable<bool> (bool + bool ≈ 2 bytes, but may pad to 4)
	// ============================================================
	Nullable<bool> Layer_UseObjectLayer;
	Nullable<bool> SpawnCrater;
	Nullable<bool> AttachFireAnimsToParent;

	// ============================================================
	// NullableIdx (likely int + bool ≈ 8 bytes)
	// ============================================================
	NullableIdx<VocClass> DetachedReport;
	NullableIdx<VocClass> AltReport;

	// ============================================================
	// Valueable<int> (4 bytes each)
	// ============================================================
	Valueable<int> XDrawOffset;
	Valueable<int> YDrawOffset_BracketAdjust;
	Valueable<int> HideIfNoOre_Threshold;
	Valueable<int> Damage_Delay;
	Valueable<int> CraterDecreaseTiberiumAmount;
	Valueable<int> Spawns_Delay;
	Valueable<int> AdditionalHeight;
	Valueable<int> CreateUnit_SpawnHeight;
	Valueable<int> LargeFireCount;
	Valueable<int> Damaging_Rate;

	// ============================================================
	// Valueable<enum> (4 bytes each, assuming 4-byte enums)
	// ============================================================
	Valueable<AttachedAnimPosition> AttachedAnimPosition;
	Valueable<AffectedHouse> VisibleTo;

	// ============================================================
	// Valueable<bool> (1 byte each, packed together)
	// ============================================================
	Valueable<bool> YDrawOffset_ApplyBracketHeight;
	Valueable<bool> YDrawOffset_InvertBracketShift;
	Valueable<bool> Warhead_Detonate;
	Valueable<bool> Damage_DealtByInvoker;
	Valueable<bool> Damage_ApplyOnce;
	Valueable<bool> Damage_ConsiderOwnerVeterancy;
	Valueable<bool> SplashIndexRandom;
	Valueable<bool> ExplodeOnWater;
	Valueable<bool> SpawnsMultiple_Random;
	Valueable<bool> SpawnParticleModeUseAresCode;
	Valueable<bool> SpecialDraw;
	Valueable<bool> NoOwner;
	Valueable<bool> RemapAnim;
	Valueable<bool> AltPalette_ApplyLighting;
	Valueable<bool> ExtraShadow;
	Valueable<bool> VisibleTo_ConsiderInvokerAsOwner;
	Valueable<bool> RestrictVisibilityIfCloaked;
	Valueable<bool> DetachOnCloak;
	Valueable<bool> ConstrainFireAnimsToCellSpots;
	Valueable<bool> Damaging_UseSeparateState;

	// ============================================================
	// Plain bool (1 byte each, at the very end)
	// ============================================================
	bool MakeInfantry_Scatter;
	bool MakeInfantry_AI_Scatter;
	bool IsInviso;
	// 23 Valueable<bool> + 3 plain bool = 26 bytes
	// Pads to 28 or 32 for alignment

#pragma endregion

public:
	AnimTypeExtData(AnimTypeClass* pObj)
		: ObjectTypeExtData(pObj)
		// Large aggregates
		, Palette(CustomPalette::PaletteMode::Temperate)
		, Translucent_Keyframes()
		// unique_ptr
		, CreateUnitType(nullptr)
		// Vectors
		, SplashList()
		, SpawnsMultiple()
		, SpawnsMultiple_amouts()
		, Launchs()
		, ConcurrentAnim()
		, SmallFireAnims()
		, SmallFireChances()
		, SmallFireDistances()
		, LargeFireAnims()
		, LargeFireChances()
		, LargeFireDistances()
		// Valueable<pointer>
		, Weapon(nullptr)
		, WeaponToCarry(nullptr)
		, AttachedSystem(nullptr)
		// Nullable<pointer>
		, WakeAnim()
		// Valueable<double>
		, ParticleRangeMin(0.0)
		, ParticleRangeMax(0.0)
		, CraterChance(0.5)
		, ConcurrentChance(0.0)
		// Nullable<double>
		, ScorchChance()
		// Nullable<int/enum>
		, YDrawOffset_BracketAdjust_Buildings()
		, ParticleChance()
		, Translucency_Cloaked()
		, SmallFireCount()
		, Damage_TargetFlag()
		, MakeInfantry_Mission()
		, MakeInfantry_AI_Mission()
		, MakeInfantryOwner()
		, FireAnimDisallowedLandTypes()
		// Nullable<bool>
		, Layer_UseObjectLayer()
		, SpawnCrater()
		, AttachFireAnimsToParent()
		// NullableIdx
		, DetachedReport()
		, AltReport()
		// Valueable<int>
		, XDrawOffset(0)
		, YDrawOffset_BracketAdjust(0)
		, HideIfNoOre_Threshold(0)
		, Damage_Delay(0)
		, CraterDecreaseTiberiumAmount(6)
		, Spawns_Delay(0)
		, AdditionalHeight(0)
		, CreateUnit_SpawnHeight(-1)
		, LargeFireCount(1)
		, Damaging_Rate(-1)
		// Valueable<enum>
		, AttachedAnimPosition(AttachedAnimPosition::Default)
		, VisibleTo(AffectedHouse::All)
		// Valueable<bool>
		, YDrawOffset_ApplyBracketHeight(false)
		, YDrawOffset_InvertBracketShift(false)
		, Warhead_Detonate(false)
		, Damage_DealtByInvoker(false)
		, Damage_ApplyOnce(false)
		, Damage_ConsiderOwnerVeterancy(true)
		, SplashIndexRandom(false)
		, ExplodeOnWater(false)
		, SpawnsMultiple_Random(false)
		, SpawnParticleModeUseAresCode(true)
		, SpecialDraw(IS_SAME_STR_(pObj->ID, GameStrings::Anim_RING1()))
		, NoOwner(false)
		, RemapAnim(false)
		, AltPalette_ApplyLighting(false)
		, ExtraShadow(true)
		, VisibleTo_ConsiderInvokerAsOwner(false)
		, RestrictVisibilityIfCloaked(false)
		, DetachOnCloak(true)
		, ConstrainFireAnimsToCellSpots(true)
		, Damaging_UseSeparateState(false)
		// Plain bool
		, MakeInfantry_Scatter(false)
		, MakeInfantry_AI_Scatter(false)
		, IsInviso(IS_SAME_STR_(pObj->ID, GameStrings::Anim_INVISO()))
	{
		this->AbsType = AnimTypeClass::AbsID;
	}

	AnimTypeExtData(AnimTypeClass* pObj, noinit_t nn) : ObjectTypeExtData(pObj, nn) { }

	virtual ~AnimTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectTypeExtData::Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->ObjectTypeExtData::Internal_SaveToStream(Stm);
		const_cast<AnimTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectTypeExtData::CalculateCRC(crc);
	}

	AnimTypeClass* This() const { return reinterpret_cast<AnimTypeClass*>(this->AttachedToObject); }
	const AnimTypeClass* This_Const() const { return reinterpret_cast<const AnimTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const {  return true; }
public:

	void ValidateSpalshAnims();

	OwnerHouseKind GetAnimOwnerHouseKind()
	{
		if(this->CreateUnitType){
			return this->CreateUnitType->Owner.Get(OwnerHouseKind::Victim);
		}

		if (This()->MakeInfantry > -1) {
			return this->MakeInfantryOwner.Get(OwnerHouseKind::Invoker);
		}

		return OwnerHouseKind::Invoker;
	}

	bool ScatterCreateUnit(bool IsAi) {
		return IsAi ? this->CreateUnitType->AI_Scatter : this->CreateUnitType->Scatter;
	}

	bool ScatterAnimToInfantry(bool IsAi) {
		return !IsAi ? this->MakeInfantry_Scatter : this->MakeInfantry_AI_Scatter;
	}

	Mission GetCreateUnitMission(bool IsAi) {
		auto result = this->CreateUnitType->UnitMission;
		if (IsAi && this->CreateUnitType->AIUnitMission.isset())
			result = this->CreateUnitType->AIUnitMission.Get();

		return result;
	}

	Mission GetAnimToInfantryMission(bool IsAi) {
		auto result = this->MakeInfantry_Mission.Get(Mission::Hunt);

		if (IsAi && this->MakeInfantry_AI_Mission.isset())
			result = this->MakeInfantry_AI_Mission;

		return result;
	}

	void ValidateData();

public:
	static void ProcessDestroyAnims(FootClass* pThis, HouseClass* pKiller = nullptr, WarheadTypeClass* pWH = nullptr);
	static void CreateUnit_MarkCell(AnimClass* pThis);
	static void CreateUnit_Spawn(AnimClass* pThis);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class AnimClass;
class AnimTypeExtContainer final : public Container<AnimTypeExtData>
	, public ReadWriteContainerInterfaces<AnimTypeExtData>
{
public:

	static COMPILETIMEEVAL const char* ClassName = "AnimTypeExtContainer";

public:
	static AnimTypeExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

	virtual void LoadFromINI(AnimTypeClass* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(AnimTypeClass* key, CCINIClass* pINI);
};

class NOVTABLE FakeAnimTypeClass : public AnimTypeClass
{
public:

	bool _ReadFromINI(CCINIClass* pINI);

	AnimTypeExtData* _GetExtData() {
		return *reinterpret_cast<AnimTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeAnimTypeClass) == sizeof(AnimTypeClass), "Invalid Size !");