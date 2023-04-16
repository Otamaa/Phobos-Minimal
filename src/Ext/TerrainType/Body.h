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

	class ExtData final : public Extension<TerrainTypeClass>
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
		
		Valueable<int> CrushableLevel;
		#pragma region Otamaa
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
		#pragma endregion

		Valueable<int> Bounty;

		ExtData(TerrainTypeClass* OwnerObject) : Extension<TerrainTypeClass>(OwnerObject)
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
			, CrushableLevel { OwnerObject->Crushable ? 10 : 0 }

			, LightEnabled { false }
			, LightVisibility { 0 }
			, LightIntensity { }
			, LightRedTint { }
			, LightGreenTint { }
			, LightBlueTint { }
			, AttachedAnim { }
			, Warhead { }
			, Damage { }
			, AreaDamage { false }
			, Bounty { 0 }

		{ }

		virtual ~ExtData() override = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void InitializeConstants() override;
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

	class ExtContainer final : public Container<TerrainTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void Remove(TerrainClass* pTerrain);
};