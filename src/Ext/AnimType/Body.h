#pragma once

#include <AnimTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/Enum.h>
#include <Utilities/TemplateDefB.h>

#include <New/Type/PaletteManager.h>

#include <New/Entity/LauchSWData.h>
#include <New/Entity/CreateUnitTypeClass.h>

#include <Ext/ObjectType/Body.h>

struct ExternalFunction
{
	HMODULE Module {};   // +0x00
	FARPROC Proc {};     // +0x04
};

struct BlendFunctionLoader
{
	using EntryPtr = std::shared_ptr<ExternalFunction>;

	// ".\bin\color_blend.dll", built as a 0x15-byte literal at both call sites.
	// NOT configurable -- only the symbol name comes from INI.
	static constexpr std::string_view LibraryPath = ".\\bin\\color_blend.dll";

	static_assert(LibraryPath.size() == 0x15,
		"The literal is constructed with an explicit length of 0x15.");

	// FNV-1a, 32-bit. 100C1870 / 100C1887.
	static constexpr uint32_t FnvOffsetBasis = 0x811C9DC5u;
	static constexpr uint32_t FnvPrime = 0x01000193u;

	static constexpr uint32_t Hash(std::string_view symbol)
	{
		uint32_t hash = FnvOffsetBasis;

		for (const char c : symbol)
			hash = (hash ^ static_cast<uint8_t>(c)) * FnvPrime;

		return hash;
	}

	// VERIFY: the original's container is a static whose end sentinel is
	// dword_102A29AC. Point this at it once the address is known.
	static std::unordered_map<std::string, EntryPtr>& Cache();

	// sub_100C16D0 -- LoadLibraryA + GetProcAddress, and the insert on success.
	static EntryPtr LoadUncached(std::string_view library, const std::string& symbol);

	// Load_Something_ (.text:100C1850) -- cache probe, then LoadUncached.
	static EntryPtr Get(std::string_view library, const std::string& symbol);
};

class AnimTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = AnimTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "AnimTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "AnimTypeClass";
	static COMPILETIMEEVAL DWORD Canary = 0x6E62C573;

public:
#pragma region ClassMembers
	// ============================================================
	// Large aggregates (unknown internal alignment, place first)
	// ============================================================
	CustomPalette Palette { CustomPalette::PaletteMode::Temperate };
	Animatable<TranslucencyLevel> Translucent_Keyframes {};

	// ============================================================
	// 8-byte aligned: unique_ptr
	// ============================================================
	std::unique_ptr<CreateUnitTypeClass> CreateUnitType {};

	// ============================================================
	// 24-byte aligned: vectors (group all together)
	// ============================================================
	NullableVector<AnimTypeClass*> SplashList {};
	ValueableVector<AnimTypeClass*> SpawnsMultiple {};
	std::vector<int> SpawnsMultiple_amouts {};
	std::vector<LauchSWData> Launchs {};
	ValueableVector<AnimTypeClass*> ConcurrentAnim {};
	ValueableVector<AnimTypeClass*> SmallFireAnims {};
	ValueableVector<double> SmallFireChances {};
	ValueableVector<double> SmallFireDistances {};
	ValueableVector<AnimTypeClass*> LargeFireAnims {};
	ValueableVector<double> LargeFireChances {};
	ValueableVector<double> LargeFireDistances {};

	// ============================================================
	// Valueable<pointer> (8 bytes each)
	// ============================================================
	Valueable<WeaponTypeClass*> Weapon {};
	Valueable<WeaponTypeClass*> WeaponToCarry {};
	Valueable<ParticleSystemTypeClass*> AttachedSystem {};

	// ============================================================
	// Nullable<pointer> (pointer + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<AnimTypeClass*> WakeAnim {};

	// ============================================================
	// Valueable<double> (8 bytes each)
	// ============================================================
	Valueable<double> ParticleRangeMin {};
	Valueable<double> ParticleRangeMax {};
	Valueable<double> CraterChance { 0.5 };
	Valueable<double> ConcurrentChance {};

	// ============================================================
	// Nullable<double> (double + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<double> ScorchChance {};

	// ============================================================
	// Nullable with 4-byte inner types (int/enum + bool + padding ≈ 8 bytes)
	// ============================================================
	Nullable<int> YDrawOffset_BracketAdjust_Buildings {};
	Nullable<int> ParticleChance {};
	Nullable<int> Translucency_Cloaked { };
	Nullable<int> SmallFireCount {};
	Nullable<DamageDelayTargetFlag> Damage_TargetFlag {};
	Nullable<Mission> MakeInfantry_Mission {};
	Nullable<Mission> MakeInfantry_AI_Mission {};
	Nullable<OwnerHouseKind> MakeInfantryOwner {};
	Nullable<LandTypeFlags> FireAnimDisallowedLandTypes {};

	// ============================================================
	// Nullable<bool> (bool + bool ≈ 2 bytes, but may pad to 4)
	// ============================================================
	Nullable<bool> Layer_UseObjectLayer {};
	Nullable<bool> SpawnCrater {};
	Nullable<bool> AttachFireAnimsToParent {};
	Nullable<bool> TheaterPalette {};
	// ============================================================
	// NullableIdx (likely int + bool ≈ 8 bytes)
	// ============================================================
	NullableIdx<VocClass> DetachedReport {};
	NullableIdx<VocClass> AltReport {};

	// ============================================================
	// Valueable<int> (4 bytes each)
	// ============================================================
	Valueable<int> XDrawOffset {};
	Valueable<int> YDrawOffset_BracketAdjust {};
	Valueable<int> HideIfNoOre_Threshold {};
	Valueable<int> Damage_Delay {};
	Valueable<int> CraterDecreaseTiberiumAmount { 6 };
	Valueable<int> Spawns_Delay {};
	Valueable<int> AdditionalHeight {};
	Valueable<int> CreateUnit_SpawnHeight { -1 };
	Valueable<int> LargeFireCount { 1 };
	Valueable<int> Damaging_Rate { -1 };
	Valueable<int> Tiled_Interval { 0 };
	// ============================================================
	// Valueable<enum> (4 bytes each, assuming 4-byte enums)
	// ============================================================
	Valueable<AttachedAnimPosition> AttachedAnimPosition { AttachedAnimPosition::Default };
	Valueable<AffectedHouse> VisibleTo { AffectedHouse::All };

	// ============================================================
	// Valueable<bool> (1 byte each, packed together)
	// ============================================================
	Valueable<bool> YDrawOffset_ApplyBracketHeight {};
	Valueable<bool> YDrawOffset_InvertBracketShift {};
	Valueable<bool> Warhead_Detonate {};
	Nullable<bool> Damage_DealtByInvoker {};
	Valueable<bool> Damage_ApplyOnce {};
	Nullable<bool> Damage_ConsiderOwnerVeterancy { };
	Valueable<bool> SplashIndexRandom {};
	Valueable<bool> ExplodeOnWater {};
	Valueable<bool> SpawnsMultiple_Random {};
	Valueable<bool> SpawnParticleModeUseAresCode { false };
	Valueable<bool> SpecialDraw {};
	Valueable<bool> NoOwner {};
	Valueable<bool> RemapAnim {};
	Valueable<bool> AltPalette_ApplyLighting {};
	Valueable<bool> ExtraShadow { true };
	Valueable<bool> VisibleTo_ConsiderInvokerAsOwner {};
	Valueable<bool> RestrictVisibilityIfCloaked {};
	Valueable<bool> DetachOnCloak { true };
	Valueable<bool> ConstrainFireAnimsToCellSpots { true };
	Valueable<bool> Damaging_UseSeparateState {};

	// ============================================================
	// Plain bool (1 byte each, at the very end)
	// ============================================================
	bool MakeInfantry_Scatter {};
	bool MakeInfantry_AI_Scatter {};
	bool IsInviso { };
	// 23 Valueable<bool> + 3 plain bool = 26 bytes
	// Pads to 28 or 32 for alignment

	// CORRECTION: read as a dedicated "wants alpha mask" flag in the first pass
	// over the mask hooks. It is FXLightEnable -- it sits in a run with the
	// three bools above, all set by adjacent details::ReadBool calls. The four
	// AnimClass_Draw_SetMaskBuffer hooks gate on the FX light flag because the
	// mask feeds that pass.
	Valueable<bool>           FXLightEnable { false };                   // 0x077
	Valueable<int>			  FXLightMaxFrame { 0 };
	Valueable<int>			  FXLightIntensity { 0 };
	Valueable<int>	          FXLightSecondary { 0 };
	ValueableVector<int>      FXLightFrames {};

	// NOTE: fixed char buffers, exactly what this tree is trying to eliminate.
	// A symbol name over 63 characters is truncated by details::ReadString and
	// then quietly fails to resolve.
	PhobosFixedString<0x40>           BlendFunctionName {};              // 0x078
	PhobosFixedString<0x40>           FullReplaceBlendFunctionName {};   // 0x0B8

	BlendFunctionLoader::EntryPtr BlendFunction {};              // 0x0F8
	BlendFunctionLoader::EntryPtr FullReplaceBlendFunction {};   // 0x100

#pragma endregion

public:
	AnimTypeExtData(AnimTypeClass* pObj) : ObjectTypeExtData(pObj)
	{
		this->AbsType = AnimTypeClass::AbsID;
		this->SpecialDraw = IS_SAME_STR_(pObj->ID, GameStrings::Anim_RING1());
		this->IsInviso = IS_SAME_STR_(pObj->ID, GameStrings::Anim_INVISO());
	}

	AnimTypeExtData() = default;

	virtual ~AnimTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override
	{
		this->ObjectTypeExtData::InvalidatePointer(ptr, bRemoved, type);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->ObjectTypeExtData::SaveToStream(Stm);
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
			result = this->CreateUnitType->AIUnitMission.Fetch();

		return result;
	}

	Mission GetAnimToInfantryMission(bool IsAi) {
		auto result = this->MakeInfantry_Mission.Get(Mission::Hunt);

		if (IsAi && this->MakeInfantry_AI_Mission.isset())
			result = this->MakeInfantry_AI_Mission.Fetch();

		return result;
	}

	void ValidateData();

public:
	static void ProcessDestroyAnims(TechnoClass* pThis, HouseClass* pKiller = nullptr, WarheadTypeClass* pWH = nullptr);
	static void CreateUnit_MarkCell(AnimClass* pThis);
	static void CreateUnit_Spawn(AnimClass* pThis);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class AnimClass;
class AnimTypeExtContainer final : public Container<AnimTypeExtData>
	, public ReadWriteContainerInterfaces<AnimTypeExtData>
	, public ContainerSaveLoad<AnimTypeExtContainer, AnimTypeExtData>
{
public:

	static COMPILETIMEEVAL const char* ClassName = "AnimTypeExtContainer";

public:
	static AnimTypeExtContainer Instance;

	virtual bool SaveGlobal(PhobosStreamWriter& stm) { return true; }
	virtual bool LoadGlobal(PhobosStreamReader& stm) { return true; }
	virtual void LoadFromINI(AnimTypeClass* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(AnimTypeClass* key, CCINIClass* pINI);
};

class NOVTABLE FakeAnimTypeClass : public AnimTypeClass
{
public:
	HRESULT __stdcall __Load(IStream* pStm);
	HRESULT __stdcall __Save(IStream* pStm, BOOL fClearDirty);

	bool _ReadFromINI(CCINIClass* pINI);

	AnimTypeExtData* _GetExtData() {
		return *reinterpret_cast<AnimTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeAnimTypeClass) == sizeof(AnimTypeClass), "Invalid Size !");