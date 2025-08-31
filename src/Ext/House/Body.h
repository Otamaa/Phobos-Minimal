#pragma once
#include <HouseClass.h>
#include <TeamClass.h>

#include <Helpers/Macro.h>

#include <Utilities/OptionalStruct.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/VectorHelper.h>
#include <Utilities/VectorSet.h>

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

class HouseExtData final : public AbstractExtended
{
public:
	using base_type = HouseClass;

public:

#pragma region ClassMembers
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
	int LastBuiltNavalVehicleType { -1 };
	int ProducingNavalUnitTypeIndex { -1 };
	std::vector<LauchData> LaunchDatas {};
	bool CaptureObjectExecuted { false };
	CDTimerClass DiscoverEvaDelay {};
	std::vector<TunnelData> Tunnels {};
	int SWLastIndex { -1 };
	HelperedVector<SuperClass*> Batteries {};
	int AvaibleDocks { 0 };
	std::bitset<MaxHouseCount> StolenTech {};
	IndexBitfield<HouseClass*> RadarPersist {};
	VectorSet<HouseTypeClass*> FactoryOwners_GatheredPlansOf {};
	VectorSet<BuildingClass*> Academies {};
	VectorSet<BuildingClass*> TunnelsBuildings {};
	VectorSet<TechnoTypeClass*> Reversed {};
	VectorSet<TechnoClass*> OwnedCountedHarvesters {};
	bool Is_NavalYardSpied { false };
	bool Is_AirfieldSpied { false };
	bool Is_ConstructionYardSpied { false };
	int AuxPower { 0 };
	int KeepAliveCount { 0 };
	int KeepAliveBuildingCount { 0 };
	NewTiberiumStorageClass TiberiumStorage {};
	OptionalStruct<TechTreeTypeClass*, true> SideTechTree {};
	CDTimerClass CombatAlertTimer {};
	VectorSet<BuildingClass*> RestrictedFactoryPlants {};
	CDTimerClass AISellAllDelayTimer {};
	HelperedVector<UnitClass*> OwnedDeployingUnits {};
	PlacingBuildingStruct Common {};
	PlacingBuildingStruct Combat {};
	CDTimerClass AISuperWeaponDelayTimer {};
	int NumAirpads_NonMFB { 0 };
	int NumBarracks_NonMFB { 0 };
	int NumWarFactories_NonMFB { 0 };
	int NumConYards_NonMFB { 0 };
	int NumShipyards_NonMFB { 0 };
	PhobosMap<SuperClass*, std::vector<SuperClass*>> SuspendedEMPulseSWs {};
	int ForceEnemyIndex { -1 };
	int BattlePoints {};

	struct ProductionData {
		std::vector<int> CreationFrames {};
		std::vector<int> Values {};
		std::vector<int> BestChoices {};

		bool Load(PhobosStreamReader& stm, bool registerForChange) {
			return this->Serialize(stm);
		}

		bool Save(PhobosStreamWriter& stm) const {
			return const_cast<ProductionData*>(this)->Serialize(stm);
		}

	private:
		template <typename T>
		bool Serialize(T& stm) {
			return stm
				.Process(this->CreationFrames)
				.Process(this->Values)
				.Process(this->BestChoices)
				.Success();
		}

	}; std::array<ProductionData , 3u> Productions {};

	std::vector<int> BestChoicesNaval {};
#pragma endregion


	HouseExtData(HouseClass* pObj) : AbstractExtended(pObj) {

		TiberiumStorage.m_values.resize(TiberiumClass::Array->Count);
	}

	HouseExtData(HouseClass* pObj, noinit_t& nn) : AbstractExtended(pObj, nn) { }

	virtual ~HouseExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractExtended::Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<HouseExtData*>(this)->AbstractExtended::Internal_SaveToStream(Stm);
		const_cast<HouseExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const { }

	virtual HouseClass* This() const override { return reinterpret_cast<HouseClass*>(this->AbstractExtended::This()); }
	virtual const HouseClass* This_Const() const override { return reinterpret_cast<const HouseClass*>(this->AbstractExtended::This_Const()); }

public:

	bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);

	void GetUnitTypeToProduce();
	int GetAircraftTypeToProduce();
	int GetInfantryTypeToProduce();

	TechTreeTypeClass* GetTechTreeType();

	void UpdateShotCount(SuperWeaponTypeClass* pFor);
	void UpdateShotCountB(SuperWeaponTypeClass* pFor);

	COMPILETIMEEVAL LauchData GetShotCount(SuperWeaponTypeClass* pFor){
		if ((size_t)pFor->ArrayIndex < this->LaunchDatas.size())
			return this->LaunchDatas[pFor->ArrayIndex];

		return {};
	}

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
	float GetRestrictedFactoryPlantMult(TechnoTypeClass* pTechnoType) const;

public:

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

private:
	bool UpdateHarvesterProduction();

	template <typename T>
	void Serialize(T& Stm);

public:
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
public:
	static HouseExtContainer Instance;

	static HouseClass* Civilian;
	static HouseClass* Special;
	static HouseClass* Neutral;
	static SideClass* CivilianSide;

	static PhobosMap<HouseClass*, VectorSet<TeamClass*>> HousesTeams;

	static void Clear();
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved);

	virtual bool WriteDataToTheByteStream(HouseExtData::base_type* key, IStream* pStm) { return true;  };
	virtual bool ReadDataFromTheByteStream(HouseExtData::base_type* key, IStream* pStm) { return true;  };
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
		return *reinterpret_cast<HouseExtData**>(((DWORD)this) + 0x18);
	}

	HouseTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<HouseTypeExtData**>(((DWORD)this->Type) + 0x18);
	}
};
static_assert(sizeof(FakeHouseClass) == sizeof(HouseClass), "Invalid Size !");
