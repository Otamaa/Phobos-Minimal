#pragma once
#include <TerrainTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDefB.h>
#include <New/Type/PaletteManager.h>

class TerrainTypeExtData final
{
public:
	static COMPILETIMEEVAL size_t Canary = 0xBEE78007;
	using base_type = TerrainTypeClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
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

 	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void Initialize();
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

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

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(TerrainTypeExtData) -
			(4u //AttachedToObject
			 );
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

	//CONSTEXPR_NOCOPY_CLASSB(TerrainTypeExtContainer, TerrainTypeExtData, "TerrainTypeClass");
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