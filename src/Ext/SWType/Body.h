#pragma once
#include <SuperWeaponTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class SuperClass;
class SWTypeExt
{
public:
	static constexpr size_t Canary = 0x11111111;
	using base_type = SuperWeaponTypeClass;
#ifndef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = 0xAC;
#endif

	class ExtData final : public Extension<SuperWeaponTypeClass>
	{
	public:

		Valueable<int> Money_Amount;
		Valueable<CSFText> UIDescription;
		Valueable<int> CameoPriority;
		ValueableVector<BuildingTypeClass*> LimboDelivery_Types;
		ValueableVector<int> LimboDelivery_IDs;
		ValueableVector<float> LimboDelivery_RollChances;
		Valueable<AffectedHouse> LimboKill_Affected;
		ValueableVector<int> LimboKill_IDs;
		Valueable<double> RandomBuffer;

		ValueableVector<ValueableVector<int>> LimboDelivery_RandomWeightsData;

		ValueableVector<TechnoTypeClass*> SW_Inhibitors;
		Valueable<bool> SW_AnyInhibitor;
		ValueableVector<TechnoTypeClass*> SW_Designators;
		Valueable<bool> SW_AnyDesignator;

		Valueable<double> SW_RangeMinimum;
		Valueable<double> SW_RangeMaximum;
		DWORD SW_RequiredHouses;
		DWORD SW_ForbiddenHouses;
		ValueableVector<BuildingTypeClass*> SW_AuxBuildings;
		ValueableVector<BuildingTypeClass*> SW_NegBuildings;

		Nullable<WarheadTypeClass*> Detonate_Warhead;
		Nullable<WeaponTypeClass*> Detonate_Weapon;
		Nullable<int> Detonate_Damage;


		#pragma region Otamaa
		Nullable<SHPStruct*> GClock_Shape;
		Nullable<int> GClock_Transculency;
		CustomPalette GClock_Palette;
		Valueable<bool> ChargeTimer;
		Valueable<bool> ChargeTimer_Backwards;
		#pragma endregion

		ExtData(SuperWeaponTypeClass* OwnerObject) : Extension<SuperWeaponTypeClass>(OwnerObject)
			, Money_Amount { 0 }
			, UIDescription {}
			, CameoPriority { 0 }
			, LimboDelivery_Types {}
			, LimboDelivery_IDs {}
			, LimboDelivery_RollChances {}
			, LimboKill_Affected { AffectedHouse::Owner }
			, LimboKill_IDs {}
			, RandomBuffer { 0.0 }
			, LimboDelivery_RandomWeightsData {}

			, SW_Inhibitors {}
			, SW_AnyInhibitor { false }
			, SW_Designators { }
			, SW_AnyDesignator { false }

			, SW_RangeMinimum { -1.0 }
			, SW_RangeMaximum { -1.0 }
			, SW_RequiredHouses { 0xFFFFFFFFu }
			, SW_ForbiddenHouses { 0u }
			, SW_AuxBuildings {}
			, SW_NegBuildings {}

			, Detonate_Warhead {}
			, Detonate_Weapon {}
			, Detonate_Damage {}

			, GClock_Shape { }
			, GClock_Transculency { }
			, GClock_Palette { }
			, ChargeTimer { false }
			, ChargeTimer_Backwards { false }
		{ }


		void FireSuperWeapon(SuperClass* pSW, HouseClass* pHouse,const CoordStruct& coords , bool IsCurrentPlayer);

		bool IsInhibitor(HouseClass* pOwner, TechnoClass* pTechno);
		bool HasInhibitor(HouseClass* pOwner, const CellStruct& Coords);
		bool IsInhibitorEligible(HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno);
		bool IsDesignator(HouseClass* pOwner, TechnoClass* pTechno) const;
		bool HasDesignator(HouseClass* pOwner, const CellStruct& coords) const;
		bool IsDesignatorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const;

		bool IsLaunchSiteEligible(const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const;
		bool IsLaunchSite(BuildingClass* pBuilding) const;
		std::pair<double, double> GetLaunchSiteRange(BuildingClass* pBuilding = nullptr) const;
		bool IsAvailable(HouseClass* pHouse) const;

		void ApplyDetonation(HouseClass* pHouse, const CoordStruct& coords);

		void LoadFromINIFile(CCINIClass* pINI);
		virtual ~ExtData() = default;
		// void InvalidatePointer(void* ptr, bool bRemoved) { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	private:
		void ApplyLimboDelivery(HouseClass* pHouse);
		void ApplyLimboKill(HouseClass* pHouse);

		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SWTypeExt
#ifndef ENABLE_NEWHOOKS
		,true,false,true
#endif
	>
	{
	public:
		ExtContainer();
		~ExtContainer();

		void InvalidatePointer(void* ptr, bool bRemoved);
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);


	static std::vector<int> WeightedRollsHandler(Valueable<double>& RandomBuffer, const ValueableVector<float>& rolls, const ValueableVector<ValueableVector<int>>& weights, size_t size)
	{
		bool rollOnce = false;
		size_t rollsSize = rolls.size();
		size_t weightsSize = weights.size();
		int index;
		std::vector<int> indices;

		// if no RollChances are supplied, do only one roll
		if (rollsSize == 0)
		{
			rollsSize = 1;
			rollOnce = true;
		}

		for (size_t i = 0; i < rollsSize; i++)
		{
			RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();
			if (!rollOnce && RandomBuffer > rolls[i])
				continue;

			// If there are more rolls than weight lists, use the last weight list
			size_t j = std::min(weightsSize - 1, i);
			index = GeneralUtils::ChooseOneWeighted(RandomBuffer, weights[j]);

			// If modder provides more weights than there are objects and we hit one of these, ignore it
			// otherwise add
			if (size_t(index) < size)
				indices.push_back(index);
		}

		return indices;
	}

	static bool Handled;
	static SuperClass* TempSuper;
};