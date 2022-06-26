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
	using base_type = HouseClass;
	class ExtData final : public Extension<HouseClass>
	{
	public:
		std::map<BuildingTypeExt::ExtData*, int> BuildingCounter;
		CounterClass OwnedLimboBuildingTypes;
		std::map<BuildingTypeExt::ExtData*, int> Building_BuildSpeedBonusCounter;
		DynamicVectorClass<BuildingClass*> HouseAirFactory;
		bool ForceOnlyTargetHouseEnemy;
		int ForceOnlyTargetHouseEnemyMode;

		ExtData(HouseClass* OwnerObject) : Extension<HouseClass>(OwnerObject)
			, BuildingCounter {}
			, OwnedLimboBuildingTypes {}
			, Building_BuildSpeedBonusCounter {}
			, HouseAirFactory { }
			, ForceOnlyTargetHouseEnemy { false }
			, ForceOnlyTargetHouseEnemyMode { -1 }
		{ }

		virtual ~ExtData() = default;

		//virtual void Initialize() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {
			HouseAirFactory.Remove(reinterpret_cast<BuildingClass*>(ptr));
		}

		virtual void Uninitialize() override {
			BuildingCounter.clear();
			OwnedLimboBuildingTypes.Clear();
			Building_BuildSpeedBonusCounter.clear();
			HouseAirFactory.Clear();
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<HouseExt> {
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
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

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static int CountOwnedLimbo(HouseClass* pThis, BuildingTypeClass const* const pItem);

	static int ActiveHarvesterCount(HouseClass* pThis);
	static int TotalHarvesterCount(HouseClass* pThis);
	static HouseClass* FindCivilianSide();
	static HouseClass* FindSpecial();
	static HouseClass* FindNeutral();
	static HouseClass* GetHouseKind(OwnerHouseKind const& kind, bool allowRandom, HouseClass* pDefault, HouseClass* pInvoker = nullptr, HouseClass* pVictim = nullptr);
	static void HouseExt::ForceOnlyTargetHouseEnemy(HouseClass* pThis, int mode);
};