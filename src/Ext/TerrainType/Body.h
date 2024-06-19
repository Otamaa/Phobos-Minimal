#pragma once
#include <TerrainTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/Macro.h>

class TerrainTypeExtData final
{
public:
	static constexpr size_t Canary = 0xBEE78007;
	using base_type = TerrainTypeClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	Valueable<PaletteManager*> CustomPalette { }; //CustomPalette::PaletteMode::Temperate
	Valueable<int> SpawnsTiberium_Type { 0 };
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
	NullableIdx<VocClass> CrumblingSound {};
	Nullable<int> AnimationLength {};

	TerrainTypeExtData()  noexcept = default;
	~TerrainTypeExtData() noexcept = default;

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void Initialize();
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	int GetTiberiumGrowthStage();
	int GetCellsPerAnim();
	void PlayDestroyEffects(CoordStruct coords);

	int GetLightIntensity() const
	{
		auto const Intense = this->LightIntensity.Get(0.0);
		return (int)(Intense * 1000.0);
	}

	TintStruct GetLightTint() const
	{
		constexpr auto ToInt = [](double nInput)
			{ return std::clamp(((int)(nInput * 1000.0)), -2000, 2000); };

		return TintStruct
		{
			ToInt(this->LightRedTint.Get(1.0)),
			ToInt(this->LightGreenTint.Get(1.0)),
			ToInt(this->LightBlueTint.Get(1.0))
		};
	}

	constexpr FORCEINLINE static size_t size_Of()
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

	CONSTEXPR_NOCOPY_CLASSB(TerrainTypeExtContainer, TerrainTypeExtData, "TerrainTypeClass");
};