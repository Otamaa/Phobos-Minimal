#pragma once
#include <TerrainTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/Macro.h>

class TerrainTypeExt
{
public:
	static constexpr size_t Canary = 0xBEE78007;
	using base_type = TerrainTypeClass;
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
#endif

	class ExtData final : public TExtension<TerrainTypeClass>
	{
	public:
		CustomPalette CustomPalette;
		Valueable<int> SpawnsTiberium_Type;
		Valueable<int> SpawnsTiberium_Range;
		Valueable<Point2D> SpawnsTiberium_GrowthStage;
		Valueable<Point2D> SpawnsTiberium_CellsPerAnim;
		Nullable<AnimTypeClass*> DestroyAnim;
		NullableIdx<VocClass> DestroySound;
		Nullable<ColorStruct> MinimapColor;

		Valueable<bool> IsPassable;
		Valueable<bool> CanBeBuiltOn;

		#pragma region Otamaa
		Nullable<int> LightVisibility;
		Nullable<double> LightIntensity;
		Nullable<double> LightRedTint;
		Nullable<double> LightGreenTint;
		Nullable<double> LightBlueTint;

		ValueableVector<AnimTypeClass*> AttachedAnim;
		Nullable<WarheadTypeClass*> Warhead;
		Nullable<int> Damage;
		Valueable<bool> AreaDamage;
		#pragma endregion

		ExtData(TerrainTypeClass* OwnerObject) : TExtension<TerrainTypeClass>(OwnerObject)
			, CustomPalette { CustomPalette::PaletteMode::Temperate }
			, SpawnsTiberium_Type { 0 }
			, SpawnsTiberium_Range { 1 }
			, SpawnsTiberium_GrowthStage { { 3, 0 } }
			, SpawnsTiberium_CellsPerAnim { { 1, 0 } }

			, DestroyAnim {}
			, DestroySound {}
			, MinimapColor {}
			, IsPassable { false }
			, CanBeBuiltOn { false }

			, LightVisibility { 0 }
			, LightIntensity { }
			, LightRedTint { }
			, LightGreenTint { }
			, LightBlueTint { }
			, AttachedAnim { }
			, Warhead { }
			, Damage { }
			, AreaDamage { false }

		{ }

		virtual ~ExtData() = default;

		void LoadFromINIFile(CCINIClass* pINI);

		// void InvalidatePointer(void* ptr, bool bRemoved) {}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
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
			{ return Math::clamp(((int)(nInput * 1000.0)), -2000, 2000); };

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

	class ExtContainer final : public TExtensionContainer<TerrainTypeExt
#ifdef ENABLE_NEWHOOKS
		, true , true , true
#endif
	> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void Remove(TerrainClass* pTerrain);
};