#pragma once
#include <HouseClass.h>
#include <TeamClass.h>

#include <Helpers/Macro.h>

#include <Utilities/OptionalStruct.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/VectorHelper.h>
#include <Utilities/VectorSet.h>
#include <Utilities/PlacingBuildingStruct.h>

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

protected:

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

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Vector)
			.Process(MaxCap)
			.Success()
			//&& Stm.RegisterChange(this)
			; // announce this type
	}
};

//TODO : validate check
enum class BuildLimitStatus
{
	ReachedPermanently = -1, // remove cameo
	ReachedTemporarily = 0, // black out cameo
	NotReached = 1, // don't do anything
};

struct ProductionData
{
	std::vector<int> CreationFrames;
	std::vector<int> Values;
	std::vector<int> BestChoices;

	bool Load(PhobosStreamReader& stm, bool registerForChange)
	{
		return this->Serialize(stm);
	}
	bool Save(PhobosStreamWriter& stm) const
	{
		return const_cast<ProductionData*>(this)->Serialize(stm);
	}

private:
	template <typename T>
	bool Serialize(T& stm)
	{
		return stm
			.Process(this->CreationFrames)
			.Process(this->Values)
			.Process(this->BestChoices)
			.Success();
	}
};

class HouseExtData final : public AbstractExtended
{
public:
	using base_type = HouseClass;
	static COMPILETIMEEVAL const char* ClassName = "HouseExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "HouseClass";
	
	

public:

#pragma region ClassMembers
	// ============================================================
	// Large aggregates (unknown internal alignment, place first)
	// ============================================================
	PhobosFixedString<0x18> Name {};
	NewTiberiumStorageClass TiberiumStorage {};
	PlacingBuildingStruct Common {};
	PlacingBuildingStruct Combat {};
	std::bitset<MaxHouseCount> StolenTech {};
	IndexBitfield<HouseClass*> RadarPersist {};
	std::array<Nullable<bool>, 3u> RepairBaseNodes {};
	std::array<ProductionData, 3u> Productions {};

	// ============================================================
	// PhobosMap (likely contains std::unordered_map or similar)
	// ============================================================
	PhobosMap<BuildingTypeClass*, int> PowerPlantEnhancerBuildings {};
	PhobosMap<BuildingTypeClass*, int> Building_BuildSpeedBonusCounter {};
	PhobosMap<BuildingTypeClass*, int> Building_OrePurifiersCounter {};
	PhobosMap<BuildingTypeClass*, int> BattlePointsCollectors {};
	PhobosMap<SuperClass*, std::vector<SuperClass*>> SuspendedEMPulseSWs {};

	// ============================================================
	// VectorSet (likely vector-based, ~24+ bytes each)
	// ============================================================
	VectorSet<HouseTypeClass*> FactoryOwners_GatheredPlansOf {};
	VectorSet<BuildingClass*> Academies {};
	VectorSet<BuildingClass*> TunnelsBuildings {};
	VectorSet<TechnoTypeClass*> Reversed {};
	VectorSet<TechnoClass*> OwnedCountedHarvesters {};
	VectorSet<BuildingClass*> RestrictedFactoryPlants {};
	VectorSet<BuildingClass*> PowerPlantEnhancers {};
	// ============================================================
	// Vectors (24 bytes each)
	// ============================================================
	std::vector<LauchData> LaunchDatas {};
	std::vector<TunnelData> Tunnels {};
	HelperedVector<SuperClass*> Batteries {};
	HelperedVector<UnitClass*> OwnedDeployingUnits {};
	std::vector<int> BestChoicesNaval {};
	std::vector<int> AITriggers_ValidList {};

	// ============================================================
	// OptionalStruct (pointer + bool + padding ≈ 16 bytes)
	// ============================================================
	OptionalStruct<TechTreeTypeClass*, true> SideTechTree {};

	// ============================================================
	// 8-byte aligned: Pointers
	// ============================================================
	BuildingClass* Factory_BuildingType { nullptr };
	BuildingClass* Factory_InfantryType { nullptr };
	BuildingClass* Factory_VehicleType { nullptr };
	BuildingClass* Factory_NavyType { nullptr };
	BuildingClass* Factory_AircraftType { nullptr };

	// ============================================================
	// CDTimerClass (group together)
	// ============================================================
	CDTimerClass DiscoverEvaDelay {};
	CDTimerClass CombatAlertTimer {};
	CDTimerClass AISellAllDelayTimer {};
	CDTimerClass AISuperWeaponDelayTimer {};

	// ============================================================
	// Nullable<bool> (bool + bool ≈ 2-4 bytes)
	// ============================================================
	Nullable<bool> Degrades {};

	// ============================================================
	// 4-byte aligned: int
	// ============================================================
	int ForceOnlyTargetHouseEnemyMode { -1 };
	int LastBuildingTypeArrayIdx { -1 };
	int LastBuiltNavalVehicleType { -1 };
	int ProducingNavalUnitTypeIndex { -1 };
	int SWLastIndex { -1 };
	int AvaibleDocks { 0 };
	int AuxPower { 0 };
	int KeepAliveCount { 0 };
	int KeepAliveBuildingCount { 0 };
	int NumAirpads_NonMFB { 0 };
	int NumBarracks_NonMFB { 0 };
	int NumWarFactories_NonMFB { 0 };
	int NumConYards_NonMFB { 0 };
	int NumShipyards_NonMFB { 0 };
	int ForceEnemyIndex { -1 };
	int BattlePoints {};
	int TeamDelay { -1 };

	// ============================================================
	// 1-byte aligned: bool (packed together at the end)
	// ============================================================
	bool m_ForceOnlyTargetHouseEnemy {};
	bool AllRepairEventTriggered {};
	bool CaptureObjectExecuted {};
	bool Is_NavalYardSpied {};
	bool Is_AirfieldSpied {};
	bool Is_ConstructionYardSpied {};
	bool FreeRadar {};
	bool ForceRadar {};
	bool PlayerAutoRepair {};
	// 9 bools = 9 bytes, pads to 12 for 4-byte alignment
#pragma endregion

public:
	HouseExtData(HouseClass* pObj) : AbstractExtended(pObj)
	{
		TiberiumStorage.m_values.resize(TiberiumClass::Array->Count);
		this->Name = pObj->Type->ID;
		this->AbsType = HouseClass::AbsID;
	}

	HouseExtData(HouseClass* pObj, noinit_t nn) : AbstractExtended(pObj, nn) { }

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

	HouseClass* This() const { return reinterpret_cast<HouseClass*>(this->AttachedToObject); }
	const HouseClass* This_Const() const { return reinterpret_cast<const HouseClass*>(this->AttachedToObject); }

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
		const int idx = pHouse->SideIndex == -1 ? pHouse->Type->SideIndex : pHouse->SideIndex;
		return SideClass::Array->operator[](idx);
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
	static bool PrerequisitesMet(HouseClass* pThis, const Iterator<int> items);

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
	static bool ShouldDisableCameo(HouseClass* pThis, TechnoTypeClass* pType, bool AdditionalCheks);

	static bool CheckShouldDisableDefensesCameo(HouseClass* pHouse, TechnoTypeClass* pType);

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

	static bool CheckBasePlanSanity(HouseClass* const pThis);
	static void UpdateTogglePower(HouseClass* pThis);
	static bool UpdateAnyFirestormActive(bool const lastChange);
	static void SetFirestormState(HouseClass* pHouse, bool const active);
	static void FormulateTypeList(std::vector<TechnoTypeClass*>& types, TechnoTypeClass** items, int count, int houseidx);
	static std::vector<TechnoTypeClass*> GetTypeList();
	static int GetTotalCost(const Nullable<int>& fixed);

	static bool FindSameTunnel(BuildingClass* pTunnel);
	static void KillFootClass(FootClass* pFoot, TechnoClass* pKiller);
	static void DestroyTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, TechnoClass* pKiller);
	static void EnterTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, FootClass* pFoot);
	static bool CanEnterTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, FootClass* pEnterer);
	static bool PopulatePassangerPIPData(TechnoClass* pThis, TechnoTypeClass* pType, int pipMax);
	static std::pair<bool, FootClass*> UnlimboOne(std::vector<FootClass*>* pVector, BuildingClass* pTunnel, DWORD Where);
	static bool UnloadOnce(FootClass* pFoot, BuildingClass* pTunnel, bool silent = false);
	static void HandleUnload(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel);

private:
	bool UpdateHarvesterProduction();

	template <typename T>
	void Serialize(T& Stm);

};

class HouseExtContainer final : public Container<HouseExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "HouseExtContainer";
	using base_t = Container<HouseExtData>;

public:
	static HouseExtContainer Instance;

public:

	HouseClass* Civilian;
	HouseClass* Special;
	HouseClass* Neutral;
	SideClass* CivilianSide;
	PhobosMap<TechnoClass*, KillMethod> AutoDeathObjects;
	HelperedVector<TechnoClass*> LimboTechno;

	int LastGrindingBlanceUnit;
	int LastGrindingBlanceInf;
	int LastHarvesterBalance;
	int LastSlaveBalance;

	CDTimerClass CloakEVASpeak;
	CDTimerClass SubTerraneanEVASpeak;
	bool IsAnyFirestormActive;

public:

	virtual bool LoadAll(const PhobosStreamReader& stm) { return true; }
	virtual bool SaveAll(PhobosStreamWriter& stm){ return true; }

	virtual void Clear();
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
	void _Attacked(BuildingClass* source, WarheadTypeClass* warhead);

	// Backported from HouseClass::init_laser_color (0x50BA00-0x50BC90)
	// Normalizes this->Color into this->LaserColor for laser rendering
	void _InitLaserColor();

	HouseExtData* _GetExtData() {
		return *reinterpret_cast<HouseExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	HouseTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<HouseTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeHouseClass) == sizeof(HouseClass), "Invalid Size !");
