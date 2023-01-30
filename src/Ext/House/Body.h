#pragma once
#include <HouseClass.h>
#include <TeamClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <map>

class HouseExt
{
public:
	static constexpr size_t Canary = 0x11111111;
	using base_type = HouseClass;
#ifndef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = 0x16098;
#endif

	class ExtData final : public Extension<HouseClass>
	{
	public:
		PhobosMap<BuildingTypeClass*, int> BuildingCounter;
		CounterClass OwnedLimboBuildingTypes;
		PhobosMap<BuildingTypeClass*, int> Building_BuildSpeedBonusCounter;
		std::vector<BuildingClass*> HouseAirFactory;
		bool ForceOnlyTargetHouseEnemy;
		int ForceOnlyTargetHouseEnemyMode;

		//DWORD RandomNumber;

		BuildingClass* Factory_BuildingType;
		BuildingClass* Factory_InfantryType;
		BuildingClass* Factory_VehicleType;
		BuildingClass* Factory_NavyType;
		BuildingClass* Factory_AircraftType;

		bool AllRepairEventTriggered;
		int LastBuildingTypeArrayIdx;

		bool RepairBaseNodes[3];

		std::vector<TeamClass*> ActiveTeams;

		//#817
		int LastBuiltNavalVehicleType;
		int ProducingNavalUnitTypeIndex;

		//#830
		std::vector<TechnoClass*> AutoDeathObjects;

		ExtData(HouseClass* OwnerObject) : Extension<HouseClass>(OwnerObject)
			, BuildingCounter {}
			, OwnedLimboBuildingTypes {}
			, Building_BuildSpeedBonusCounter {}
			, HouseAirFactory { }
			, ForceOnlyTargetHouseEnemy { false }
			, ForceOnlyTargetHouseEnemyMode { -1 }
			//, RandomNumber { 0 }

			, Factory_BuildingType { nullptr }
			, Factory_InfantryType { nullptr }
			, Factory_VehicleType { nullptr }
			, Factory_NavyType { nullptr }
			, Factory_AircraftType { nullptr }

			, AllRepairEventTriggered { false }
			, LastBuildingTypeArrayIdx { -1 }
			, RepairBaseNodes { false,false,false }

			, ActiveTeams { }

			, LastBuiltNavalVehicleType { -1 }
			, ProducingNavalUnitTypeIndex { -1 }

			, AutoDeathObjects {}
		{ }

		virtual ~ExtData() = default;

		void InvalidatePointer(void* ptr, bool bRemoved);
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void InitializeConstants();
		void LoadFromINIFile(CCINIClass* pINI);

		void UpdateVehicleProduction();
		void UpdateAutoDeathObjects();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<HouseExt
		,true ,true ,true
	> {
	public:
		ExtContainer();
		~ExtContainer();

		bool InvalidateExtDataIgnorable(void* const ptr) const
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Building:
			case AbstractType::Team:
				return false;
			default:
				return true;
			}
		}
	};

	static ExtContainer ExtMap;
	static int LastGrindingBlanceUnit;
	static int LastGrindingBlanceInf;
	static int LastHarvesterBalance;
	static int LastSlaveBalance;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static int CountOwnedLimbo(HouseClass* pThis, BuildingTypeClass const* const pItem);

	static int ActiveHarvesterCount(HouseClass* pThis);
	static int TotalHarvesterCount(HouseClass* pThis);
	static HouseClass* FindCivilianSide();
	static HouseClass* FindSpecial();
	static HouseClass* FindNeutral();
	static HouseClass* GetHouseKind(OwnerHouseKind const& kind, bool allowRandom, HouseClass* pDefault, HouseClass* pInvoker = nullptr, HouseClass* pVictim = nullptr);
	static HouseClass* GetSlaveHouse(SlaveReturnTo const& kind, HouseClass* pKiller, HouseClass* pVictim);
	static signed int PrereqValidate(HouseClass const* const pHouse, TechnoTypeClass const* const pItem,bool const buildLimitOnly, bool const includeQueued)
	{
		return 1;
	}
	static void ForceOnlyTargetHouseEnemy(HouseClass* pThis, int mode);

	static bool IsObserverPlayer();
	static bool IsObserverPlayer(HouseClass* pCur);

	static bool PrerequisitesMet(HouseClass* const pThis, TechnoTypeClass* const pItem, const Iterator<BuildingTypeClass*>& ownedBuildingTypes);
	static bool HasGenericPrerequisite(int idx, const Iterator<BuildingTypeClass*>& ownedBuildingTypes);
	static int FindGenericPrerequisite(const char* id);
	static int GetHouseIndex(int param, TeamClass* pTeam, TActionClass* pTAction);

	static bool IsDisabledFromShell(
		HouseClass const* pHouse, BuildingTypeClass const* pItem);

	static size_t FindOwnedIndex(
	HouseClass const* pHouse, int idxParentCountry,
	Iterator<TechnoTypeClass const*> items, size_t start = 0);

	static size_t FindBuildableIndex(
		HouseClass const* pHouse, int idxParentCountry,
		Iterator<TechnoTypeClass const*> items, size_t start = 0);

	template <typename T>
	static T* FindOwned(
		HouseClass const* const pHouse, int const idxParent,
		Iterator<T*> const items, size_t const start = 0)
	{
		auto const index = FindOwnedIndex(pHouse, idxParent, items, start);
		return index < items.size() ? items[index] : nullptr;
	}

	template <typename T>
	static T* FindBuildable(
		HouseClass const* const pHouse, int const idxParent,
		Iterator<T*> const items, size_t const start = 0)
	{
		auto const index = FindBuildableIndex(pHouse, idxParent, items, start);
		return index < items.size() ? items[index] : nullptr;
	}

	static std::vector<int> AIProduction_CreationFrames;
	static std::vector<int> AIProduction_Values;
	static std::vector<int> AIProduction_BestChoices;
	static std::vector<int> AIProduction_BestChoicesNaval;
};