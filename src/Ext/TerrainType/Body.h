#pragma once
#include <TerrainTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/Macro.h>

class TerrainTypeExt
{
public:
	class ExtData final : public Extension<TerrainTypeClass>
	{
	public:
		static constexpr size_t Canary = 0xBEE78007;
		using base_type = TerrainTypeClass;

	public:
		Valueable<PaletteManager*> CustomPalette { }; //CustomPalette::PaletteMode::Temperate
		Valueable<int> SpawnsTiberium_Type { 0 };
		Valueable<int> SpawnsTiberium_Range { 1 };
		Valueable<PartialVector2D<int>> SpawnsTiberium_GrowthStage { { 3, 0 } };
		Valueable<PartialVector2D<int>> SpawnsTiberium_CellsPerAnim{ { 1, 0 } };
		Nullable<AnimTypeClass*> DestroyAnim { };
		NullableIdx<VocClass> DestroySound { };
		Nullable<ColorStruct> MinimapColor { };

		Valueable<bool> IsPassable { false };
		Valueable<bool> CanBeBuiltOn { false };
		
		Valueable<int> CrushableLevel;

		Valueable<bool> LightEnabled { false };
		Nullable<int> LightVisibility { };
		Nullable<double> LightIntensity { };
		Nullable<double> LightRedTint { };
		Nullable<double> LightGreenTint { };
		Nullable<double> LightBlueTint { };

		ValueableVector<AnimTypeClass*> AttachedAnim { };
		Nullable<WarheadTypeClass*> Warhead { };
		Nullable<int> Damage { };
		Valueable<bool> AreaDamage  { false };

		Valueable<int> Bounty { 0 };

		ExtData(TerrainTypeClass* OwnerObject) : 
			Extension<TerrainTypeClass>(OwnerObject)
			, CrushableLevel { OwnerObject->Crushable ? 10 : 0 }
		{ }

		virtual ~ExtData() override = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void Initialize() ;
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

		int GetTiberiumGrowthStage();
		int GetCellsPerAnim();

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

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TerrainTypeExt::ExtData> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static void Remove(TerrainClass* pTerrain);
};