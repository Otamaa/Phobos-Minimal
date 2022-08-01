#pragma once
#include <BuildingTypeClass.h>
#include <SuperWeaponTypeClass.h>

#include <Utilities/BaseClassTemplates.h>
#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <ExtraHeaders/DirClass.h>

struct BuildSpeedBonus : public SaveLoadBaseClassTemplate
{
	bool Enabled;
	double SpeedBonus_Aircraft;
	double SpeedBonus_Building;
	double SpeedBonus_Infantry;
	double SpeedBonus_Unit;
	ValueableVector<TechnoTypeClass*> AffectedType;

	BuildSpeedBonus() : Enabled { false }
		, SpeedBonus_Aircraft { 0.000 }
		, SpeedBonus_Building { 0.000 }
		, SpeedBonus_Infantry { 0.000 }
		, SpeedBonus_Unit { 0.000 }
		, AffectedType { }
	{}

	void Read(INI_EX& parser, const char* pSection)
	{
		Nullable<double> nBuff {};
		nBuff.Read(parser, pSection, "BuildSpeedBonus.Aircraft");

		if (nBuff.isset() && nBuff.Get() != 0.000)
		{
			Enabled = true;
			SpeedBonus_Aircraft = nBuff.Get();
		}

		nBuff.Reset();
		nBuff.Read(parser, pSection, "BuildSpeedBonus.Building");

		if (nBuff.isset() && nBuff.Get() != 0.000)
		{
			Enabled = true;
			SpeedBonus_Building = nBuff.Get();
		}

		nBuff.Reset();
		nBuff.Read(parser, pSection, "BuildSpeedBonus.Infantry");

		if (nBuff.isset() && nBuff.Get() != 0.000)
		{
			Enabled = true;
			SpeedBonus_Infantry = nBuff.Get();
		}

		nBuff.Reset();
		nBuff.Read(parser, pSection, "BuildSpeedBonus.Unit");

		if (nBuff.isset() && nBuff.Get() != 0.000)
		{
			Enabled = true;
			SpeedBonus_Unit = nBuff.Get();
		}

		if (Enabled)
			AffectedType.Read(parser, pSection, "BuildSpeedBonus.AffectedTypes");
	}

	virtual bool Load(PhobosStreamReader& stm, bool registerForChange)
	{
		return Serialize(stm);
	}
	virtual bool Save(PhobosStreamWriter& stm) const
	{
		return const_cast<BuildSpeedBonus*>(this)->Serialize(stm);
	}

private:
	template <typename T>
	bool Serialize(T& stm)
	{
		Debug::Log("Processing Items From BuildSpeedBonus ! \n");

		return stm
			.Process(Enabled)
			.Process(SpeedBonus_Aircraft)
			.Process(SpeedBonus_Building)
			.Process(SpeedBonus_Infantry)
			.Process(SpeedBonus_Unit)
			.Process(AffectedType)
			.Success();
	}
};

class BuildingTypeExt
{
public:
	static constexpr size_t Canary = 0x11111111;
	using base_type = BuildingTypeClass;

	class ExtData final : public Extension<BuildingTypeClass>
	{
	public:
		Valueable<AffectedHouse> PowersUp_Owner;
		ValueableVector<BuildingTypeClass*> PowersUp_Buildings;
		DynamicVectorClass<SuperWeaponTypeClass*> SuperWeapons;

		ValueableVector<BuildingTypeClass*> PowerPlantEnhancer_Buildings;
		Nullable<int> PowerPlantEnhancer_Amount;
		Nullable<float> PowerPlantEnhancer_Factor;

		DynamicVectorClass<Point2D> OccupierMuzzleFlashes;

		Valueable<bool> Refinery_UseStorage;

		Valueable<bool> Grinding_AllowAllies;
		Valueable<bool> Grinding_AllowOwner;
		ValueableVector<TechnoTypeClass*> Grinding_AllowTypes;
		ValueableVector<TechnoTypeClass*> Grinding_DisallowTypes;
		NullableIdx<VocClass> Grinding_Sound;
		Nullable<WeaponTypeClass*> Grinding_Weapon;
		Valueable<bool> Grinding_DisplayRefund;
		Valueable<AffectedHouse> Grinding_DisplayRefund_Houses;
		Valueable<Point2D> Grinding_DisplayRefund_Offset;

		Nullable<bool> PlacementPreview_Show;
		Nullable<Theater_SHPStruct*> PlacementPreview_Shape;
		Nullable<int> PlacementPreview_ShapeFrame;
		Valueable<CoordStruct> PlacementPreview_Offset;
		Valueable<bool> PlacementPreview_Remap;
		CustomPalette PlacementPreview_Palette;
		Nullable<int> PlacementPreview_TranslucentLevel;
#pragma region Otamaa
		NullableVector<AnimTypeClass*> DamageFireTypes;
		NullableVector<AnimTypeClass*> OnFireTypes;
		NullableVector<int> OnFireIndex;
		HealthOnFireData HealthOnfire;
		DynamicVectorClass<Point2D> DamageFire_Offs;

		Nullable<double> RepairRate;
		Nullable<int> RepairStep;

		Nullable<bool> PlayerReturnFire;

		Valueable<bool> PackupSound_PlayGlobal;
		Valueable<bool> DisableDamageSound;

		Nullable<float> BuildingOccupyDamageMult;
		Nullable<float> BuildingOccupyROFMult;

		Nullable<float> BuildingBunkerDamageMult;
		Nullable<float> BuildingBunkerROFMult;


		NullableIdx<VocClass> BunkerWallsUpSound;
		NullableIdx<VocClass> BunkerWallsDownSound;

		ValueableIdxVector<BuildingTypeClass> AIBuildInsteadPerDiff;

		PhobosMap<int, AnimTypeClass*> GarrisonAnim_idle;
		PhobosMap<int, AnimTypeClass*> GarrisonAnim_ActiveOne;
		PhobosMap<int, AnimTypeClass*> GarrisonAnim_ActiveTwo;
		PhobosMap<int, AnimTypeClass*> GarrisonAnim_ActiveThree;
		PhobosMap<int, AnimTypeClass*> GarrisonAnim_ActiveFour;

		CustomPalette PipShapes01Palette;
		Valueable<bool> PipShapes01Remap;

		Nullable<AnimTypeClass*> TurretAnim_LowPower;
		Nullable<AnimTypeClass*> TurretAnim_DamagedLowPower;

		Valueable<bool> BuildUp_UseNormalLIght;

		Valueable<bool> Power_DegradeWithHealth;

		Nullable<float> AutoSellTime;

		Valueable<bool> IsJuggernaut;

		Nullable<SHPStruct*> BuildingPlacementGrid_Shape;
		BuildSpeedBonus SpeedBonus;

		CustomPalette RubblePalette;

#pragma endregion
		ExtData(BuildingTypeClass* OwnerObject) : Extension<BuildingTypeClass>(OwnerObject)
			, PowersUp_Owner { AffectedHouse::Owner }
			, PowersUp_Buildings {}
			, PowerPlantEnhancer_Buildings {}
			, PowerPlantEnhancer_Amount {}
			, PowerPlantEnhancer_Factor {}
			, OccupierMuzzleFlashes()
			, Refinery_UseStorage { false }
			, Grinding_AllowAllies { false }
			, Grinding_AllowOwner { true }
			, Grinding_AllowTypes {}
			, Grinding_DisallowTypes {}
			, Grinding_Sound {}
			, Grinding_Weapon {}
			, Grinding_DisplayRefund { false }
			, Grinding_DisplayRefund_Houses { AffectedHouse::All }
			, Grinding_DisplayRefund_Offset { { 0,0 } }

			, PlacementPreview_Show {}
			, PlacementPreview_Shape {}
			, PlacementPreview_ShapeFrame {}
			, PlacementPreview_Offset { {0,-15,1} }
			, PlacementPreview_Remap { true }
			, PlacementPreview_Palette {}
			, PlacementPreview_TranslucentLevel {}

			, DamageFireTypes {}
			, OnFireTypes {}
			, OnFireIndex {}
			, HealthOnfire {}

			, RepairRate {}
			, RepairStep {}

			, PlayerReturnFire {}

			, PackupSound_PlayGlobal { false }
			, DisableDamageSound { false }

			, BuildingOccupyDamageMult {}
			, BuildingOccupyROFMult {}

			, BuildingBunkerDamageMult {}
			, BuildingBunkerROFMult {}

			, BunkerWallsUpSound { }
			, BunkerWallsDownSound { }

			, AIBuildInsteadPerDiff {}

			, GarrisonAnim_idle {}
			, GarrisonAnim_ActiveOne {}
			, GarrisonAnim_ActiveTwo {}
			, GarrisonAnim_ActiveThree {}
			, GarrisonAnim_ActiveFour {}

			, PipShapes01Palette { CustomPalette::PaletteMode::Temperate }
			, PipShapes01Remap { false }

			, TurretAnim_LowPower {}
			, TurretAnim_DamagedLowPower {}
			, BuildUp_UseNormalLIght { false }
			, Power_DegradeWithHealth { true }
			, IsJuggernaut { false }
			, BuildingPlacementGrid_Shape { }
			, SpeedBonus {}
			, RubblePalette { CustomPalette::PaletteMode::Temperate }
		{ }

		virtual ~ExtData() = default;
		virtual size_t Size() const { return sizeof(*this); }
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InitializeConstants() override;
		virtual void CompleteInitialization();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BuildingTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
		virtual bool Load(BuildingTypeClass* pThis, IStream* pStm) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static int GetEnhancedPower(BuildingClass* pBuilding, HouseClass* pHouse);
	static double GetExternalFactorySpeedBonus(TechnoClass* pWhat);
	static double GetExternalFactorySpeedBonus(TechnoClass* pWhat , HouseClass* pOwner);
	static bool CanUpgrade(BuildingClass* pBuilding, BuildingTypeClass* pUpgradeType, HouseClass* pUpgradeOwner);
	static int GetUpgradesAmount(BuildingTypeClass* pBuilding, HouseClass* pHouse);

	static const DirClass DefaultJuggerFacing;

	template<auto  T>
	struct BunkerSound
	{
		constexpr void operator ()(BuildingClass* pThis) {
			Handle(pThis, std::bool_constant<T>::type());
		}

	private:
		void Handle(BuildingClass* pThis, std::true_type) noexcept
		{
			auto nSound = RulesGlobal->BunkerWallsUpSound;

			if (const auto TypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type)) {
				nSound = TypeExt->BunkerWallsUpSound.Get(nSound);
			}

			if (nSound != -1)
				VocClass::PlayAt(nSound, pThis->Location);
		}

		void Handle(BuildingClass* pThis, std::false_type) noexcept
		{
			auto nSound = RulesGlobal->BunkerWallsDownSound;

			if (const auto TypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type)) {
				nSound = TypeExt->BunkerWallsDownSound.Get(nSound);
			}

			if (nSound != -1)
				VocClass::PlayAt(nSound, pThis->Location);
		}
	};

	static int GetBuildingAnimTypeIndex(BuildingClass* pThis, const BuildingAnimSlot& nSlot, const char* pDefault);
	static bool __fastcall IsFactory(BuildingClass* pThis, void* _);
	static void __fastcall DrawPlacementGrid(Surface* Surface, ConvertClass* Pal, SHPStruct* SHP, int FrameIndex, const Point2D* const Position, const RectangleStruct* const Bounds, BlitterFlags Flags, int Remap, int ZAdjust, ZGradient ZGradientDescIndex,	int Brightness,	int TintColor, SHPStruct* ZShape, int ZShapeFrame, int XOffset,	int YOffset);
};