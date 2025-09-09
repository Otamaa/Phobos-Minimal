#pragma once
#include <TerrainTypeClass.h>

#include <Ext/ObjectType/Body.h>
#include <New/Type/PaletteManager.h>

class TerrainTypeExtData final : public ObjectTypeExtData
{
public:

	using base_type = TerrainTypeClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:
#pragma region ClassMember
	CustomPalette CustomPalette; //
	Valueable<int> SpawnsTiberium_Type;
	Valueable<int> SpawnsTiberium_Range;
	Valueable<PartialVector2D<int>> SpawnsTiberium_GrowthStage;
	Valueable<PartialVector2D<int>> SpawnsTiberium_CellsPerAnim;
	Valueable<AnimTypeClass*> DestroyAnim;
	ValueableIdx<VocClass> DestroySound;
	Nullable<ColorStruct> MinimapColor;
	Valueable<bool> IsPassable;
	Valueable<bool> CanBeBuiltOn;
	Valueable<int> CrushableLevel;
	Valueable<bool> LightEnabled;
	Nullable<int> LightVisibility;
	Nullable<double> LightIntensity;
	Nullable<double> LightRedTint;
	Nullable<double> LightGreenTint;
	Nullable<double> LightBlueTint;
	ValueableVector<AnimTypeClass*> AttachedAnim;
	Nullable<WarheadTypeClass*> Warhead;
	Nullable<int> Damage;
	Valueable<bool> AreaDamage;
	Valueable<int> Bounty;
	Valueable<bool> HasDamagedFrames;
	Valueable<bool> HasCrumblingFrames;
	ValueableIdx<VocClass> CrumblingSound;
	Nullable<int> AnimationLength;
	NullableVector<AnimTypeClass*> TreeFires;
	ValueableIdx<ParticleTypeClass*> SpawnsTiberium_Particle;
#pragma endregion

	TerrainTypeExtData(TerrainTypeClass* pObj) : ObjectTypeExtData(pObj),
		CustomPalette(CustomPalette::PaletteMode::Temperate),
		SpawnsTiberium_Type(-1),
		SpawnsTiberium_Range(1),
		SpawnsTiberium_GrowthStage({ 3, 0 }),
		SpawnsTiberium_CellsPerAnim({ 1, 0 }),
		DestroyAnim(nullptr),
		DestroySound(-1),
		IsPassable(false),
		CanBeBuiltOn(false),
		CrushableLevel(0),
		LightEnabled(false),
		AreaDamage(false),
		Bounty(0),
		HasDamagedFrames(false),
		HasCrumblingFrames(false),
		CrumblingSound(-1),
		SpawnsTiberium_Particle(-1)
	{
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

	virtual TerrainTypeClass* This() const override { return reinterpret_cast<TerrainTypeClass*>(this->ObjectTypeExtData::This()); }
	virtual const TerrainTypeClass* This_Const() const override { return reinterpret_cast<const TerrainTypeClass*>(this->ObjectTypeExtData::This_Const()); }

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
{
public:
	static TerrainTypeExtContainer Instance;

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

class NOVTABLE FakeTerrainTypeClass : public TerrainTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	bool _ReadFromINI(CCINIClass* pINI);


	TerrainTypeExtData* _GetExtData() {
		return *reinterpret_cast<TerrainTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeTerrainTypeClass) == sizeof(TerrainTypeClass), "Invalid Size !");