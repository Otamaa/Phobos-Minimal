#pragma once
#include <HouseClass.h>
#include <TeamClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/VectorHelper.h>
#include <map>

struct LauchData
{
	int LastFrame { Unsorted::CurrentFrame };
	int Count { 0 };

	void Update() {
		++Count;
		LastFrame = Unsorted::CurrentFrame();
	}
};

struct TunnelData
{
	std::vector<FootClass*> Vector {};
	int MaxCap { 1 };
};

//TODO : validate check
enum class BuildLimitStatus {
	ReachedPermanently = -1, // remove cameo
	ReachedTemporarily = 0, // black out cameo
	NotReached = 1, // don't do anything
};

class HouseExt
{
public:
	static std::vector<int> AIProduction_CreationFrames;
	static std::vector<int> AIProduction_Values;
	static std::vector<int> AIProduction_BestChoices;
	static std::vector<int> AIProduction_BestChoicesNaval;

	static int LastGrindingBlanceUnit;
	static int LastGrindingBlanceInf;
	static int LastHarvesterBalance;
	static int LastSlaveBalance;

	static CDTimerClass CloakEVASpeak;
	static CDTimerClass SubTerraneanEVASpeak;

	class ExtData final : public Extension<HouseClass>
	{
	public:
		static constexpr size_t Canary = 0x11111111;
		using base_type = HouseClass;
		static constexpr size_t ExtOffset = 0x16098;

	public:

		PhobosMap<BuildingTypeClass*, int> PowerPlantEnhancerBuildings {};
		PhobosMap<BuildingTypeClass*, int> Building_BuildSpeedBonusCounter {};

		bool ForceOnlyTargetHouseEnemy { false };
		int ForceOnlyTargetHouseEnemyMode { -1 };

		BuildingClass* Factory_BuildingType { nullptr };
		BuildingClass* Factory_InfantryType { nullptr };
		BuildingClass* Factory_VehicleType { nullptr };
		BuildingClass* Factory_NavyType { nullptr };
		BuildingClass* Factory_AircraftType { nullptr };

		bool AllRepairEventTriggered { false };
		int LastBuildingTypeArrayIdx { -1 };

		bool RepairBaseNodes[3] { false };

		std::vector<TeamClass*> ActiveTeams {};

		//#817
		int LastBuiltNavalVehicleType { -1 };
		int ProducingNavalUnitTypeIndex { -1 };

		//#830
		PhobosMap<TechnoClass* , KillMethod> AutoDeathObjects {};

		std::vector<LauchData> LaunchDatas {};
		bool CaptureObjectExecuted { false };
		CDTimerClass DiscoverEvaDelay {};
		std::vector<TunnelData> Tunnels {};
		DWORD Seed { 0 };

		int SWLastIndex { -1 };
		HelperedVector<SuperClass*> Batteries {};
		HelperedVector<HouseTypeClass*> Factories_HouseTypes {};
		HelperedVector<TechnoClass*> LimboTechno {};

		int AvaibleDocks { 0 };

		ExtData(HouseClass* OwnerObject) : Extension<HouseClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;

		void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
		static bool InvalidateIgnorable(AbstractClass* ptr);

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);

		void UpdateVehicleProduction();
		void UpdateAutoDeathObjects();

		void UpdateShotCount(SuperWeaponTypeClass* pFor);
		void UpdateShotCountB(SuperWeaponTypeClass* pFor);
		LauchData GetShotCount(SuperWeaponTypeClass* pFor);

		//void AddToLimboTracking(TechnoTypeClass* pTechnoType);
		//void RemoveFromLimboTracking(TechnoTypeClass* pTechnoType);
		//int CountOwnedPresentAndLimboed(TechnoTypeClass* pTechnoType);

		static SuperClass* IsSuperAvail(int nIdx , HouseClass* pHouse);
	private:
		bool UpdateHarvesterProduction();

		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<HouseExt::ExtData> {
	public:
		ExtContainer();
		~ExtContainer();

		static bool LoadGlobals(PhobosStreamReader& Stm);
		static bool SaveGlobals(PhobosStreamWriter& Stm);
		void Clear()
		{
			AIProduction_CreationFrames.clear();
			AIProduction_Values.clear();
			AIProduction_BestChoices.clear();
			AIProduction_BestChoicesNaval.clear();

			LastGrindingBlanceUnit = 0;
			LastGrindingBlanceInf = 0;
			LastHarvesterBalance = 0;
			LastSlaveBalance = 0;

			CloakEVASpeak.Stop();
			SubTerraneanEVASpeak.Stop();
		}

	};

	static ExtContainer ExtMap;

	static int ActiveHarvesterCount(HouseClass* pThis);
	static int TotalHarvesterCount(HouseClass* pThis);
	static CellClass* GetEnemyBaseGatherCell(HouseClass* pTargetHouse, HouseClass* pCurrentHouse, const CoordStruct& defaultCurrentCoords, SpeedType speedTypeZone, int extraDistance = 0);

	// Some non playable countries will set SideIndex to -1
	static SideClass* HouseExt::GetSide(HouseClass* pHouse) {
		if (!pHouse)
			return nullptr;

		return SideClass::Array->GetItemOrDefault(pHouse->SideIndex);
	}

	static HouseClass* FindCivilianSide();
	static HouseClass* FindSpecial();
	static HouseClass* FindNeutral();
	static HouseClass* GetHouseKind(OwnerHouseKind const& kind, bool allowRandom, HouseClass* pDefault, HouseClass* pInvoker = nullptr, HouseClass* pVictim = nullptr);
	static HouseClass* GetSlaveHouse(SlaveReturnTo const& kind, HouseClass* pKiller, HouseClass* pVictim);
	static signed int PrereqValidate(HouseClass const* const pHouse, TechnoTypeClass const* const pItem,bool const buildLimitOnly, bool const includeQueued)
	{
		return 1;
	}

	//
	static int GetSurvivorDivisor(HouseClass* pHouse);
	static InfantryTypeClass* GetCrew(HouseClass* pHouse);
	static InfantryTypeClass* GetEngineer(HouseClass* pHouse);
	static InfantryTypeClass* GetTechnician(HouseClass* pHouse);
	static AircraftTypeClass* GetParadropPlane(HouseClass* pHouse);
	static AircraftTypeClass* GetSpyPlane(HouseClass* pHouse);
	static UnitTypeClass* GetHunterSeeker(HouseClass* pHouse);
	static AnimTypeClass* GetParachuteAnim(HouseClass* pHouse);
	static bool GetParadropContent(HouseClass* pHouse , Iterator<TechnoTypeClass*>& Types, Iterator<int>& Num);
	//
	static void ForceOnlyTargetHouseEnemy(HouseClass* pThis, int mode);

	static bool IsObserverPlayer();
	static bool IsObserverPlayer(HouseClass* pCur);

	static bool PrerequisitesMet(HouseClass* const pThis, TechnoTypeClass* const pItem);
	//static bool HasGenericPrerequisite(int idx, const Iterator<BuildingTypeClass*>& ownedBuildingTypes);
	//static int FindGenericPrerequisite(const char* id);
	static int GetHouseIndex(int param, TeamClass* pTeam, TActionClass* pTAction);

	static bool IsDisabledFromShell(HouseClass* pHouse, BuildingTypeClass const* pItem);

	static size_t FindOwnedIndex(HouseClass* pHouse, int idxParentCountry, Iterator<TechnoTypeClass const*> items, size_t start = 0);
	static size_t FindBuildableIndex(HouseClass* pHouse, int idxParentCountry, Iterator<TechnoTypeClass const*> items, size_t start = 0);

	template <typename T>
	static T* FindOwned(HouseClass* pHouse, int const idxParent, Iterator<T*> const items, size_t const start = 0)
	{
		auto const index = FindOwnedIndex(pHouse, idxParent, items, start);
		return index < items.size() ? items[index] : nullptr;
	}

	template <typename T>
	static T* FindBuildable(HouseClass* pHouse, int const idxParent, Iterator<T*> const items, size_t const start = 0)
	{
		auto const index = FindBuildableIndex(pHouse, idxParent, items, start);
		return index < items.size() ? items[index] : nullptr;
	}

	static int CountOwnedNowTotal(HouseClass const* pHouse, TechnoTypeClass* pItem);
	static signed int BuildLimitRemaining(HouseClass const* pHouse, TechnoTypeClass* pItem);
	static BuildLimitStatus CheckBuildLimit(HouseClass const* pHouse, TechnoTypeClass* pItem, bool includeQueued);


	static TunnelData* GetTunnelVector(HouseClass* pHouse, size_t nTunnelIdx);
	static TunnelData* GetTunnelVector(BuildingTypeClass* pBld, HouseClass* pHouse);
};