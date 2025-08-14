#pragma once
#include <HouseClass.h>
#include <TeamClass.h>

#include <Helpers/Macro.h>

#include <Utilities/OptionalStruct.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/VectorHelper.h>

#include <New/Entity/NewTiberiumStorageClass.h>
#include <New/Entity/TrackerClass.h>
#include <New/Type/TechTreeTypeClass.h>

#include <Misc/Defines.h>
#include <map>

#include <SuperWeaponTypeClass.h>

class TActionClass;

struct LauchData
{
	int LastFrame { Unsorted::CurrentFrame };
	int Count { 0 };

	COMPILETIMEEVAL void Update()
	{
		++Count;
		LastFrame = Unsorted::CurrentFrame();
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<LauchData*>(this)->Serialize(Stm); }

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(LastFrame)
			.Process(Count)
			.Success()
			//&& Stm.RegisterChange(this)
			; // announce this type
	}
};

struct TunnelData
{
	HelperedVector<FootClass*> Vector;
	int MaxCap;

	COMPILETIMEEVAL TunnelData() noexcept : Vector {}, MaxCap { 1 } { }
	COMPILETIMEEVAL ~TunnelData() noexcept = default;
	COMPILETIMEEVAL TunnelData(int MaxCap) noexcept : Vector {} , MaxCap { MaxCap } {}


	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<TunnelData*>(this)->Serialize(Stm); }

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Vector, true)
			.Process(MaxCap)
			.Success()
			//&& Stm.RegisterChange(this)
			; // announce this type
	}
};

struct PlacingBuildingStruct
{
	BuildingTypeClass* Type;
	BuildingTypeClass* DrawType;
	int Times;
	CDTimerClass Timer;
	CellStruct TopLeft;
};

//TODO : validate check
enum class BuildLimitStatus
{
	ReachedPermanently = -1, // remove cameo
	ReachedTemporarily = 0, // black out cameo
	NotReached = 1, // don't do anything
};

class HouseExtData final
{
public:
	static COMPILETIMEEVAL size_t Canary = 0x12345678;
	using base_type = HouseClass;

	static COMPILETIMEEVAL size_t ExtOffset = 0x16084;//ARES
	//static COMPILETIMEEVAL size_t ExtOffset = 0x16098;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	Nullable<bool> Degrades {};

	PhobosMap<BuildingTypeClass*, int> PowerPlantEnhancerBuildings {};
	PhobosMap<BuildingTypeClass*, int> Building_BuildSpeedBonusCounter {};
	PhobosMap<BuildingTypeClass*, int> Building_OrePurifiersCounter {};
	PhobosMap<BuildingTypeClass*, int> BattlePointsCollectors {};

	bool m_ForceOnlyTargetHouseEnemy { false };
	int ForceOnlyTargetHouseEnemyMode { -1 };

	BuildingClass* Factory_BuildingType { nullptr };
	BuildingClass* Factory_InfantryType { nullptr };
	BuildingClass* Factory_VehicleType { nullptr };
	BuildingClass* Factory_NavyType { nullptr };
	BuildingClass* Factory_AircraftType { nullptr };

	bool AllRepairEventTriggered { false };
	int LastBuildingTypeArrayIdx { -1 };

	Nullable<bool> RepairBaseNodes[3] { };

	//#817
	int LastBuiltNavalVehicleType { -1 };
	int ProducingNavalUnitTypeIndex { -1 };

	std::vector<LauchData> LaunchDatas {};
	bool CaptureObjectExecuted { false };
	CDTimerClass DiscoverEvaDelay {};
	std::vector<TunnelData> Tunnels {};
	DWORD Seed { 0 };

	int SWLastIndex { -1 };
	HelperedVector<SuperClass*> Batteries {};

	int AvaibleDocks { 0 };

	std::bitset<MaxHouseCount> StolenTech {};
	IndexBitfield<HouseClass*> RadarPersist {};
	std::set<HouseTypeClass*> FactoryOwners_GatheredPlansOf {};
	std::set<BuildingClass*> Academies {};
	std::set<BuildingClass*> TunnelsBuildings {};
	std::set<TechnoTypeClass*> Reversed {};
	std::set<TechnoClass*> OwnedCountedHarvesters {};

	bool Is_NavalYardSpied { false };
	bool Is_AirfieldSpied { false };
	bool Is_ConstructionYardSpied { false };
	int AuxPower { 0 };

	int KeepAliveCount { 0 };
	int KeepAliveBuildingCount { 0 };

	NewTiberiumStorageClass TiberiumStorage {};

	//TrackerClass BuiltAircraftTypes {};
	//TrackerClass BuiltInfantryTypes {};
	//TrackerClass BuiltUnitTypes {};
	//TrackerClass BuiltBuildingTypes {};
	//TrackerClass KilledAircraftTypes {};
	//TrackerClass KilledInfantryTypes {};
	//TrackerClass KilledUnitTypes {};
	//TrackerClass KilledBuildingTypes {};
	//TrackerClass CapturedBuildings {};
	//TrackerClass CollectedCrates {};

	OptionalStruct<TechTreeTypeClass*, true> SideTechTree {};
	CDTimerClass CombatAlertTimer {};
	std::set<BuildingClass*> RestrictedFactoryPlants {};
	CDTimerClass AISellAllDelayTimer {};

	HelperedVector<UnitClass*> OwnedDeployingUnits {};
	PlacingBuildingStruct Common {};
	PlacingBuildingStruct Combat {};

	CDTimerClass AISuperWeaponDelayTimer {};

	// Factories that exist but don't count towards multiple factory bonus.
	int NumAirpads_NonMFB { 0 };
	int NumBarracks_NonMFB { 0 };
	int NumWarFactories_NonMFB { 0 };
	int NumConYards_NonMFB { 0 };
	int NumShipyards_NonMFB { 0 };

	PhobosMap<SuperClass*, std::vector<SuperClass*>> SuspendedEMPulseSWs {};

	int ForceEnemyIndex { -1 };
	int BattlePoints {};

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(HouseExtData) -
			(4u //AttachedToObject
			 );
	}

	TechTreeTypeClass* GetTechTreeType();

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void InitializeConstant();

	void UpdateVehicleProduction();

	void UpdateShotCount(SuperWeaponTypeClass* pFor);
	void UpdateShotCountB(SuperWeaponTypeClass* pFor);
	COMPILETIMEEVAL LauchData GetShotCount(SuperWeaponTypeClass* pFor){
		if ((size_t)pFor->ArrayIndex < this->LaunchDatas.size())
			return this->LaunchDatas[pFor->ArrayIndex];

		return {};
	}

	//void AddToLimboTracking(TechnoTypeClass* pTechnoType);
	//void RemoveFromLimboTracking(TechnoTypeClass* pTechnoType);
	//int CountOwnedPresentAndLimboed(TechnoTypeClass* pTechnoType);

	void UpdateAcademy(BuildingClass* pAcademy, bool added);
	void ApplyAcademy(TechnoClass* pTechno, AbstractType considerAs) const;
	void ApplyAcademyWithoutMutexCheck(TechnoClass* pTechno, AbstractType considerAs) const;

	void UpdateNonMFBFactoryCounts(AbstractType rtti, bool remove, bool isNaval);
	int GetFactoryCountWithoutNonMFB(AbstractType rtti, bool isNaval);

	int GetForceEnemyIndex();
	void SetForceEnemy(int EnemyIndex);

	void UpdateBattlePoints(int modifier);
	bool AreBattlePointsEnabled();
	bool CanTransactBattlePoints(int amount);
	int CalculateBattlePoints(TechnoClass* pTechno);
	int CalculateBattlePoints(TechnoTypeClass* pTechno, HouseClass* pOwner);

	bool ReverseEngineer(TechnoClass* Victim);

	static SuperClass* IsSuperAvail(int nIdx, HouseClass* pHouse);

	static int ActiveHarvesterCount(HouseClass* pThis);
	static int TotalHarvesterCount(HouseClass* pThis);
	static CellClass* GetEnemyBaseGatherCell(HouseClass* pTargetHouse, HouseClass* pCurrentHouse, const CoordStruct& defaultCurrentCoords, SpeedType speedTypeZone, int extraDistance = 0);

	// Some non playable countries will set SideIndex to -1
	static SideClass* GetSide(HouseClass* pHouse)
	{
		if (!pHouse)
			return nullptr;

		return SideClass::Array->GetItemOrDefault(pHouse->SideIndex);
	}

	static HouseClass* FindFirstCivilianHouse();
	static HouseClass* FindSpecial();
	static HouseClass* FindNeutral();
	static HouseClass* GetHouseKind(OwnerHouseKind const& kind, bool allowRandom, HouseClass* pDefault, HouseClass* pInvoker = nullptr, HouseClass* pVictim = nullptr);
	static HouseClass* GetSlaveHouse(SlaveReturnTo const& kind, HouseClass* pKiller, HouseClass* pVictim);

	//
	static int GetSurvivorDivisor(HouseClass* pHouse);
	static InfantryTypeClass* GetCrew(HouseClass* pHouse);
	static InfantryTypeClass* GetEngineer(HouseClass* pHouse);
	static InfantryTypeClass* GetTechnician(HouseClass* pHouse);
	static InfantryTypeClass* GetDisguise(HouseClass* pHouse);
	static AircraftTypeClass* GetParadropPlane(HouseClass* pHouse);
	static AircraftTypeClass* GetSpyPlane(HouseClass* pHouse);
	static UnitTypeClass* GetHunterSeeker(HouseClass* pHouse);
	static AnimTypeClass* GetParachuteAnim(HouseClass* pHouse);
	static bool GetParadropContent(HouseClass* pHouse, Iterator<TechnoTypeClass*>& Types, Iterator<int>& Num);
	//
	static void ForceOnlyTargetHouseEnemy(HouseClass* pThis, int mode);

	static bool IsObserverPlayer();
	static bool IsObserverPlayer(HouseClass* pCur);

	static bool PrerequisitesMet(HouseClass* const pThis, TechnoTypeClass* const pItem);
	static bool PrerequisitesMet(HouseClass* pThis, int* items , int size);

	static void UpdateAutoDeathObjects();
	static void UpdateTransportReloaders();

	//static bool HasGenericPrerequisite(int idx, const Iterator<BuildingTypeClass*>& ownedBuildingTypes);
	//static int FindGenericPrerequisite(const char* id);
	static int GetHouseIndex(int param, TeamClass* pTeam, TActionClass* pTAction);

	static bool IsDisabledFromShell(HouseClass* pHouse, BuildingTypeClass const* pItem);

	static size_t FindOwnedIndex(HouseClass* pHouse, int idxParentCountry, Iterator<TechnoTypeClass const*> items, size_t start = 0);

	template <typename T>
	static size_t FindBuildableIndex(HouseClass* pHouse, int idxParentCountry, Iterator<T const*> items, size_t start = 0)
	{
		for (size_t i = start; i < items.size(); ++i)
		{
			if (!items[i])
				continue;

			if (pHouse->CanExpectToBuild(items[i], idxParentCountry)) {

				if COMPILETIMEEVAL (T::AbsID == BuildingTypeClass::AbsID) {
					if (HouseExtData::IsDisabledFromShell(pHouse, (const BuildingTypeClass*)items[i])) {
						continue;
					}
				}

				return i;
			}
		}

		return items.size();
	}

	template <typename T>
	static T* FindOwned(HouseClass* pHouse, int const idxParent, Iterator<T*> const items, size_t const start = 0)
	{
		size_t const index = FindOwnedIndex(pHouse, idxParent, items, start);
		return index < items.size() ? items[index] : nullptr;
	}

	template <typename T>
	static T* FindBuildable(HouseClass* pHouse, int const idxParent, Iterator<T*> const items, size_t const start = 0)
	{
		size_t const index = FindBuildableIndex<T>(pHouse, idxParent, items, start);
		return index < items.size() ? items[index] : nullptr;
	}

	static int CountOwnedNowTotal(HouseClass const* pHouse, TechnoTypeClass* pItem);
	static signed int BuildLimitRemaining(HouseClass const* pHouse, TechnoTypeClass* pItem);
	static BuildLimitStatus CheckBuildLimit(HouseClass const* pHouse, TechnoTypeClass* pItem, bool includeQueued);
	static int BuildBuildingLimitRemaining(HouseClass* pHouse, BuildingTypeClass* pItem);
	static int CheckBuildingBuildLimit(HouseClass* pHouse, BuildingTypeClass* pItem, bool const includeQueued);
	static int CountOwnedIncludeDeploy(const HouseClass* pThis, const TechnoTypeClass* pItem);

	static std::vector<int> GetBuildLimitGroupLimits(HouseClass* pHouse,TechnoTypeClass* pType);
	static CanBuildResult BuildLimitGroupCheck(HouseClass* pThis, TechnoTypeClass* pItem, bool buildLimitOnly, bool includeQueued);
	static int QueuedNum(const HouseClass* pHouse, const TechnoTypeClass* pType);
	static void RemoveProduction(const HouseClass* pHouse, const TechnoTypeClass* pType, int num);
	static bool ReachedBuildLimit(HouseClass* pHouse, TechnoTypeClass* pType, bool ignoreQueued);

	static bool ShouldDisableCameo(HouseClass* pThis, TechnoTypeClass* pType);

	static TunnelData* GetTunnelVector(HouseClass* pHouse, size_t nTunnelIdx);
	static TunnelData* GetTunnelVector(BuildingTypeClass* pBld, HouseClass* pHouse);

	static void UpdateFactoryPlans(BuildingClass* pBld);

	static bool CheckFactoryOwners(HouseClass* pHouse, TechnoTypeClass* pItem);
	static CanBuildResult PrereqValidate(HouseClass* pHouse, TechnoTypeClass* pItem, bool buildLimitOnly, bool includeQueued);
	static std::pair<NewFactoryState, BuildingClass*> HasFactory(
	HouseClass* pHouse,
	TechnoTypeClass* pType,
	bool bSkipAircraft,
	bool requirePower,
	bool bCheckCanBuild,
	bool b7);

	static RequirementStatus RequirementsMet(HouseClass* pHouse, TechnoTypeClass* pItem);
	static void UpdateAcademy(HouseClass* pHouse, BuildingClass* pAcademy, bool added);
	static void ApplyAcademy(HouseClass* pHouse, TechnoClass* pTechno, AbstractType considerAs);

	static void IncremetCrateTracking(HouseClass* pHouse, Powerup type);
	static void InitializeTrackers(HouseClass* pHouse);

	static bool IsMutualAllies(HouseClass const* pThis , HouseClass const* pHouse);

	float GetRestrictedFactoryPlantMult(TechnoTypeClass* pTechnoType) const;

private:
	bool UpdateHarvesterProduction();

	template <typename T>
	void Serialize(T& Stm);

public:
	static std::vector<int> AIProduction_CreationFrames;
	static std::vector<int> AIProduction_Values;
	static std::vector<int> AIProduction_BestChoices;
	static std::vector<int> AIProduction_BestChoicesNaval;
	static PhobosMap<TechnoClass*, KillMethod> AutoDeathObjects;
	static HelperedVector<TechnoClass*> LimboTechno;

	static int LastGrindingBlanceUnit;
	static int LastGrindingBlanceInf;
	static int LastHarvesterBalance;
	static int LastSlaveBalance;

	static CDTimerClass CloakEVASpeak;
	static CDTimerClass SubTerraneanEVASpeak;

	static bool IsAnyFirestormActive;
};

class HouseExtContainer final : public Container<HouseExtData>
{
//public:
//	CONSTEXPR_NOCOPY_CLASSB(HouseExtContainer, HouseExtData, "HouseClass");
public:
	static HouseExtContainer Instance;

	static HouseClass* Civilian;
	static HouseClass* Special;
	static HouseClass* Neutral;
	static std::unordered_map<HouseClass*, std::set<TeamClass*>> HousesTeams;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	void Clear();
};

class HouseTypeExtData;
class NOVTABLE FakeHouseClass : public HouseClass
{
public:
	bool _IsAlliedWith(HouseClass* pOther);
	void _Detach(AbstractClass* target, bool all);
	int _Expert_AI();
	void _GiveTiberium(float amout , int type);
	bool _IsIonCannonEligibleTarget(TechnoClass* pTechno) const;
	void _UpdateAngerNodes(int score_add, HouseClass* pHouse);
	void _AITryFireSW();
	void _BlowUpAll();
	void _BlowUpAllBuildings();
	void _UpdateRadar();
	void _UpdateSpySat();

	HouseExtData* _GetExtData() {
		return *reinterpret_cast<HouseExtData**>(((DWORD)this) + HouseExtData::ExtOffset);
	}

	HouseTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<HouseTypeExtData**>(((DWORD)this->Type) + 0xC4);
	}
};
static_assert(sizeof(FakeHouseClass) == sizeof(HouseClass), "Invalid Size !");
