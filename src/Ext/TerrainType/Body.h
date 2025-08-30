#pragma once
#include <TerrainTypeClass.h>

#include <Ext/ObjectType/Body.h>
#include <New/Type/PaletteManager.h>

class TerrainTypeExtData final : public ObjectTypeExtData
{
public:

	using base_type = TerrainTypeClass;

public:

#pragma region ClassMember

	CustomPalette CustomPalette { CustomPalette::PaletteMode::Temperate }; //
	Valueable<int> SpawnsTiberium_Type { -1 };
	Valueable<int> SpawnsTiberium_Range { 1 };
	Valueable<PartialVector2D<int>> SpawnsTiberium_GrowthStage { { 3, 0 } };
	Valueable<PartialVector2D<int>> SpawnsTiberium_CellsPerAnim { { 1, 0 } };
	Valueable<AnimTypeClass*> DestroyAnim { nullptr };
	ValueableIdx<VocClass> DestroySound { -1 };
	Nullable<ColorStruct> MinimapColor { };

	Valueable<bool> IsPassable { false };
	Valueable<bool> CanBeBuiltOn { false };

	Valueable<int> CrushableLevel {};

	Valueable<bool> LightEnabled { false };
	Nullable<int> LightVisibility { };
	Nullable<double> LightIntensity { };
	Nullable<double> LightRedTint { };
	Nullable<double> LightGreenTint { };
	Nullable<double> LightBlueTint { };

	ValueableVector<AnimTypeClass*> AttachedAnim { };
	Nullable<WarheadTypeClass*> Warhead { };
	Nullable<int> Damage { };
	Valueable<bool> AreaDamage { false };

	Valueable<int> Bounty { 0 };

	Valueable<bool> HasDamagedFrames { false };
	Valueable<bool> HasCrumblingFrames { false };
	ValueableIdx<VocClass> CrumblingSound { -1 };
	Nullable<int> AnimationLength {};

	NullableVector<AnimTypeClass*> TreeFires {};
	ValueableIdx<ParticleTypeClass> SpawnsTiberium_Particle { -1 };
#pragma endregion

	TerrainTypeExtData(TerrainTypeClass* pObj) : ObjectTypeExtData(pObj) { this->Initialize(); }
	TerrainTypeExtData(TerrainTypeClass* pObj, noinit_t& nn) : ObjectTypeExtData(pObj, nn) { }

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

	virtual void SaveToStream(PhobosStreamWriter& Stm) const
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

	virtual AircraftTypeClass* This() const override { return reinterpret_cast<AircraftTypeClass*>(this->ObjectTypeExtData::This()); }
	virtual const AircraftTypeClass* This_Const() const override { return reinterpret_cast<const AircraftTypeClass*>(this->ObjectTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { }

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

	virtual bool WriteDataToTheByteStream(TerrainTypeExtData::base_type* key, IStream* pStm) { };
	virtual bool ReadDataFromTheByteStream(TerrainTypeExtData::base_type* key, IStream* pStm) { };
};

class NOVTABLE FakeTerrainTypeClass : public TerrainTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	TerrainTypeExtData* _GetExtData() {
		return *reinterpret_cast<TerrainTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeTerrainTypeClass) == sizeof(TerrainTypeClass), "Invalid Size !");