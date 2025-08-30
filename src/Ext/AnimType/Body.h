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

public:

#pragma region ClassMembers
	CustomPalette Palette { CustomPalette::PaletteMode::Temperate }; //CustomPalette::PaletteMode::Temperate
	bool MakeInfantry_Scatter { false };
	bool MakeInfantry_AI_Scatter { false };
    std::unique_ptr<CreateUnitTypeClass> CreateUnitType{};
	Valueable<int> XDrawOffset { 0 };
	Valueable<int> HideIfNoOre_Threshold { 0 };
	Nullable<bool> Layer_UseObjectLayer {};
	Valueable<AttachedAnimPosition> AttachedAnimPosition { AttachedAnimPosition::Default };
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
	Valueable<bool> AltPalette_ApplyLighting { false };
	Valueable<bool> ExtraShadow { true };
	NullableIdx<VocClass> DetachedReport {};
	Valueable<int> AdditionalHeight {};
	NullableIdx<VocClass> AltReport {};
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
#pragma endregion

public:

	AnimTypeExtData(AnimTypeClass* pObj) : ObjectTypeExtData(pObj) {
		SpecialDraw = IS_SAME_STR_(this->Name(), GameStrings::Anim_RING1());
		IsInviso = IS_SAME_STR_(this->Name(), GameStrings::Anim_INVISO());
	}

	AnimTypeExtData(AnimTypeClass* pObj, noinit_t& nn) : ObjectTypeExtData(pObj, nn) { }

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

	virtual void SaveToStream(PhobosStreamWriter& Stm) const
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
	virtual bool WriteToINI(CCINIClass* pINI) const { }
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
			result = this->CreateUnitType->AIUnitMission;

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

	static void Clear()
	{
		Array.clear();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		return true;
	}

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

	virtual bool WriteDataToTheByteStream(AnimTypeExtData::base_type* key, IStream* pStm) { };
	virtual bool ReadDataFromTheByteStream(AnimTypeExtData::base_type* key, IStream* pStm) { };
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