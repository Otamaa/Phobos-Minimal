#pragma once
#include <TerrainTypeClass.h>

#include <Ext/ObjectType/Body.h>
#include <New/Type/PaletteManager.h>

class TerrainTypeExtData final : public ObjectTypeExtData
{
public:

	using base_type = TerrainTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "TerrainTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "TerrainTypeClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:

#pragma region ClassMember

	// ---- Visual / palette ----
	CustomPalette CustomPalette; //
	Nullable<ColorStruct> MinimapColor;

	// ---- Tiberium spawning ----
	Valueable<int> SpawnsTiberium_Type;
	Valueable<int> SpawnsTiberium_Range;
	Valueable<PartialVector2D<int>> SpawnsTiberium_GrowthStage;
	Valueable<PartialVector2D<int>> SpawnsTiberium_CellsPerAnim;
	Valueable<float> SpawnsTiberium_StageFalloff;
	ValueableIdx<ParticleTypeClass*> SpawnsTiberium_Particle;

	// ---- Destruction / damage ----
	Valueable<AnimTypeClass*> DestroyAnim;
	ValueableIdx<VocClass> DestroySound;
	Nullable<WarheadTypeClass*> Warhead;
	Nullable<int> Damage;
	Valueable<bool> AreaDamage;
	Valueable<int> Bounty;

	// ---- Terrain interaction ----
	Valueable<bool> IsPassable;
	Valueable<bool> CanBeBuiltOn;
	Valueable<int> CrushableLevel;

	// ---- Lighting ----
	Valueable<bool> LightEnabled;
	Nullable<int> LightVisibility;
	Nullable<double> LightIntensity;
	Nullable<double> LightRedTint;
	Nullable<double> LightGreenTint;
	Nullable<double> LightBlueTint;

	// ---- Animation state ----
	ValueableVector<AnimTypeClass*> AttachedAnim;
	Valueable<bool> HasDamagedFrames;
	Valueable<bool> HasCrumblingFrames;
	ValueableIdx<VocClass> CrumblingSound;
	Nullable<int> AnimationLength;
	NullableVector<AnimTypeClass*> TreeFires;

#pragma endregion

	TerrainTypeExtData(TerrainTypeClass* pObj) : ObjectTypeExtData(pObj),
		CustomPalette(CustomPalette::PaletteMode::Temperate),
		MinimapColor(),
		SpawnsTiberium_Type(-1),
		SpawnsTiberium_Range(1),
		SpawnsTiberium_GrowthStage({ 3, 0 }),
		SpawnsTiberium_CellsPerAnim({ 1, 0 }),
		SpawnsTiberium_StageFalloff(),
		SpawnsTiberium_Particle(-1),
		DestroyAnim(),
		DestroySound(-1),
		Warhead(),
		Damage(),
		AreaDamage(),
		Bounty(0),
		IsPassable(),
		CanBeBuiltOn(),
		CrushableLevel(0),
		LightEnabled(),
		LightVisibility(),
		LightIntensity(),
		LightRedTint(),
		LightGreenTint(),
		LightBlueTint(),
		AttachedAnim(),
		HasDamagedFrames(),
		HasCrumblingFrames(),
		CrumblingSound(-1),
		AnimationLength(),
		TreeFires()
	{
		this->AbsType = TerrainTypeClass::AbsID;
		this->Initialize();
	}

	TerrainTypeExtData(TerrainTypeClass* pObj, noinit_t nn) : ObjectTypeExtData(pObj, nn) { }

	virtual ~TerrainTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<TerrainTypeExtData*>(this)->ObjectTypeExtData::SaveToStream(Stm);
		const_cast<TerrainTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectTypeExtData::CalculateCRC(crc);
	}

	TerrainTypeClass* This() const { return reinterpret_cast<TerrainTypeClass*>(this->AttachedToObject); }
	const TerrainTypeClass* This_Const() const { return reinterpret_cast<const TerrainTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

	void Initialize();

public:

	int GetTiberiumGrowthStage();
	int GetCellsPerAnim();
	void PlayDestroyEffects(CoordStruct coords);

	COMPILETIMEEVAL int GetLightIntensity() const
	{
		auto const Intense = this->LightIntensity.Get(0.0);
		return (int)(Intense * 1000.0);
	}

	COMPILETIMEEVAL TintStruct GetLightTint() const
	{
		COMPILETIMEEVAL auto ToInt = [](double nInput)
			{ return std::clamp(((int)(nInput * 1000.0)), -2000, 2000); };

		return TintStruct
		{
			ToInt(this->LightRedTint.Get(1.0)),
			ToInt(this->LightGreenTint.Get(1.0)),
			ToInt(this->LightBlueTint.Get(1.0))
		};
	}

private:
	template <typename T>
	void Serialize(T& Stm);
public:

	static void Remove(TerrainClass* pTerrain);
};

class TerrainTypeExtContainer final : public Container<TerrainTypeExtData>
	, public ReadWriteContainerInterfaces<TerrainTypeExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "TerrainTypeExtContainer";
	using base_t = Container<TerrainTypeExtData>;
	using ext_t = TerrainTypeExtData;

public:
	static TerrainTypeExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

	virtual void LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(ext_t::base_type* key, CCINIClass* pINI);
};

class NOVTABLE FakeTerrainTypeClass : public TerrainTypeClass
{
public:

	bool _ReadFromINI(CCINIClass* pINI);


	TerrainTypeExtData* _GetExtData() {
		return *reinterpret_cast<TerrainTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeTerrainTypeClass) == sizeof(TerrainTypeClass), "Invalid Size !");