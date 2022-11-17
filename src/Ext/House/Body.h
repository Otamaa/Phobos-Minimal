#pragma once
#include <HouseClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/BuildingType/Body.h>

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
		{ }

		virtual ~ExtData() = default;

		void InvalidatePointer(void* ptr, bool bRemoved);
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void InitializeConstants();
		void LoadFromINIFile(CCINIClass* pINI);

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
				return false;
			default:
				return true;
			}
		}
	};

	static ExtContainer ExtMap;
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
};