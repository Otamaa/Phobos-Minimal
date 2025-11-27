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
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:
#pragma region ClassMembers
	CustomPalette Palette;
	bool MakeInfantry_Scatter;
	bool MakeInfantry_AI_Scatter;
	std::unique_ptr<CreateUnitTypeClass> CreateUnitType;
	Valueable<int> XDrawOffset;
	Valueable<bool> YDrawOffset_ApplyBracketHeight;
	Valueable<bool> YDrawOffset_InvertBracketShift;
	Valueable<int> YDrawOffset_BracketAdjust;
	Nullable<int> YDrawOffset_BracketAdjust_Buildings;
	Valueable<int> HideIfNoOre_Threshold;
	Nullable<bool> Layer_UseObjectLayer;
	Valueable<AttachedAnimPosition> AttachedAnimPosition;
	Valueable<WeaponTypeClass*> Weapon;
	Valueable<WeaponTypeClass*> WeaponToCarry;
	Valueable<bool> Warhead_Detonate;
	Valueable<int> Damage_Delay;
	Valueable<bool> Damage_DealtByInvoker;
	Valueable<bool> Damage_ApplyOnce;
	Valueable<bool> Damage_ConsiderOwnerVeterancy;
	Nullable<DamageDelayTargetFlag> Damage_TargetFlag;
	Nullable<Mission> MakeInfantry_Mission;
	Nullable<Mission> MakeInfantry_AI_Mission;
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
	Valueable<bool> SpawnParticleModeUseAresCode;
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
	Nullable<OwnerHouseKind> MakeInfantryOwner;
	Valueable<ParticleSystemTypeClass*> AttachedSystem;
	bool IsInviso;
	Valueable<bool> RemapAnim;
	Valueable<bool> AltPalette_ApplyLighting;
	Valueable<bool> ExtraShadow;
	NullableIdx<VocClass> DetachedReport;
	Valueable<int> AdditionalHeight;
	NullableIdx<VocClass> AltReport;
	Valueable<AffectedHouse> VisibleTo;
	Valueable<bool> VisibleTo_ConsiderInvokerAsOwner;
	Valueable<bool> RestrictVisibilityIfCloaked;
	Valueable<bool> DetachOnCloak;
	Nullable<int> Translucency_Cloaked;
	Animatable<TranslucencyLevel> Translucent_Keyframes;
	Valueable<int> CreateUnit_SpawnHeight;
	Valueable<bool> ConstrainFireAnimsToCellSpots;
	Nullable<LandTypeFlags> FireAnimDisallowedLandTypes;
	Nullable<bool> AttachFireAnimsToParent;
	Nullable<int> SmallFireCount;
	ValueableVector<AnimTypeClass*> SmallFireAnims;
	ValueableVector<double> SmallFireChances;
	ValueableVector<double> SmallFireDistances;
	Valueable<int> LargeFireCount;
	ValueableVector<AnimTypeClass*> LargeFireAnims;
	ValueableVector<double> LargeFireChances;
	ValueableVector<double> LargeFireDistances;
	Valueable<bool> Damaging_UseSeparateState;
	Valueable<int> Damaging_Rate;
#pragma endregion

public:
	AnimTypeExtData(AnimTypeClass* pObj)
		: ObjectTypeExtData(pObj),
		Palette(CustomPalette::PaletteMode::Temperate),
		MakeInfantry_Scatter(false),
		MakeInfantry_AI_Scatter(false),
		CreateUnitType(nullptr),
		XDrawOffset(0),
		YDrawOffset_ApplyBracketHeight(false),
		YDrawOffset_InvertBracketShift(false),
		YDrawOffset_BracketAdjust(0),
		YDrawOffset_BracketAdjust_Buildings(),
		HideIfNoOre_Threshold(0),
		Layer_UseObjectLayer(),
		AttachedAnimPosition(AttachedAnimPosition::Default),
		Weapon(nullptr),
		WeaponToCarry(nullptr),
		Warhead_Detonate(false),
		Damage_Delay(0),
		Damage_DealtByInvoker(false),
		Damage_ApplyOnce(false),
		Damage_ConsiderOwnerVeterancy(true),
		Damage_TargetFlag(),
		MakeInfantry_Mission(),
		MakeInfantry_AI_Mission(),
		SplashList(),
		SplashIndexRandom(false),
		WakeAnim(),
		ExplodeOnWater(false),
		SpawnsMultiple(),
		SpawnsMultiple_Random(false),
		SpawnsMultiple_amouts(),
		ParticleRangeMin(0.0),
		ParticleRangeMax(0.0),
		ParticleChance(),
		SpawnParticleModeUseAresCode(true),
		Launchs(),
		CraterDecreaseTiberiumAmount(6),
		CraterChance(0.5),
		SpawnCrater(),
		ScorchChance(),
		SpecialDraw(IS_SAME_STR_(pObj->ID, GameStrings::Anim_RING1())),
		NoOwner(false),
		Spawns_Delay(0),
		ConcurrentChance(0.0),
		ConcurrentAnim(),
		MakeInfantryOwner(),
		AttachedSystem(nullptr),
		IsInviso(IS_SAME_STR_(pObj->ID, GameStrings::Anim_INVISO())),
		RemapAnim(false),
		AltPalette_ApplyLighting(false),
		ExtraShadow(true),
		DetachedReport(),
		AdditionalHeight(0),
		AltReport(),
		VisibleTo(AffectedHouse::All),
		VisibleTo_ConsiderInvokerAsOwner(false),
		RestrictVisibilityIfCloaked(false),
		DetachOnCloak(true),
		Translucency_Cloaked(),
		Translucent_Keyframes(),
		CreateUnit_SpawnHeight(-1),
		ConstrainFireAnimsToCellSpots(true),
		FireAnimDisallowedLandTypes(),
		AttachFireAnimsToParent(),
		SmallFireCount(),
		SmallFireAnims(),
		SmallFireChances(),
		SmallFireDistances(),
		LargeFireCount(1),
		LargeFireAnims(),
		LargeFireChances(),
		LargeFireDistances(),
		Damaging_UseSeparateState(false),
		Damaging_Rate(-1)
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

	virtual AnimTypeClass* This() const override { return reinterpret_cast<AnimTypeClass*>(this->ObjectTypeExtData::This()); }
	virtual const AnimTypeClass* This_Const() const override { return reinterpret_cast<const AnimTypeClass*>(this->ObjectTypeExtData::This_Const()); }

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
	static void ProcessDestroyAnims(FootClass* pThis, TechnoClass* pKiller = nullptr, WarheadTypeClass* pWH = nullptr);
	static void CreateUnit_MarkCell(AnimClass* pThis);
	static void CreateUnit_Spawn(AnimClass* pThis);

private:
	template <typename T>
	void Serialize(T& Stm);

};

class AnimClass;
class AnimTypeExtContainer final : public Container<AnimTypeExtData>
{
public:
	static AnimTypeExtContainer Instance;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}
};

class NOVTABLE FakeAnimTypeClass : public AnimTypeClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);


	bool _ReadFromINI(CCINIClass* pINI);

	AnimTypeExtData* _GetExtData() {
		return *reinterpret_cast<AnimTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeAnimTypeClass) == sizeof(AnimTypeClass), "Invalid Size !");