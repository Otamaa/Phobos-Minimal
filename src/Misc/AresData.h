#pragma once
#include <Windows.h>

#include <YRPPCore.h>
#include <GeneralDefinitions.h>
#include <Memory.h>

#include <ArrayClasses.h>
#include <vector>

#include <CoordStruct.h>
#include <ScriptActionNode.h>

#include <Utilities/Enum.h>

class AbstractClass;
class TechnoClass;
class TechnoTypeClass;
class FootClass;
class BuildingClass;
class BuildingTypeClass;
class InfantryClass;
class HouseClass;
class SuperClass;
class SuperWeaponTypeClass;
struct VoxelStruct;
class ConvertClass;
class BulletTypeClass;
class WarheadTypeClass;
class BuildingLightClass;
class BSurface;
class TeamClass;
typedef int (__cdecl *CallHook)(REGISTERS* R);

class PoweredUnitClass
{
public:
	TechnoClass* Techno;
	int LastScan;
	bool Powered;
};

static_assert(sizeof(PoweredUnitClass) == 0xC, "Invalid Size !");

struct DataBundleDetonate
{
	CoordStruct* pCoord;
	void* ptr;
};

struct AEData
{
	DWORD* ArrFirst;
	DWORD* ArrLast;
	DWORD* ArrEnd;
	int Delay;
	bool NeedToRecreateAnim;
};

class JammerClass
{
	int LastScan;							//!< Frame number when the last scan was performed.
	TechnoClass* AttachedToObject;			//!< Pointer to game object this jammer is on
	bool Registered;						//!< Did I jam anything at all? Used to skip endless unjam calls.
};

class cPrismForwarding
{
	void* Owner;  //Ares BuildingExtData
	DynamicVectorClass<cPrismForwarding*> Senders;		//the prism towers that are forwarding to this one
	cPrismForwarding* SupportTarget;			//what tower am I sending to?
	int PrismChargeDelay;					//current delay charge
	double ModifierReserve;					//current modifier reservoir
	int DamageReserve;
};

struct EMPulse
{
	static bool IsDeactivationAdvisable(TechnoClass* Target);
};

//struct ALIGN(4) OptionalStructDummy
//{
//	bool Value;
//	bool HasValue;
//};
//static_assert(sizeof(OptionalStructDummy) == 2, "Invalid Size !");

// TODO : Allocation and deallocation for Ares vector and pointer stuffs
// becarefull when doing operation that involve those , it can result on undefine behaviour

//TODO : port these
#define GetAresTechnoExt(var) (void*)(*(uintptr_t*)((char*)var + 0x154))
#define GetAresBuildingExt(var)  (void*)(*(uintptr_t*)((char*)var + 0x71C))
#define GetAresHouseExt(var)  (void*)(*(uintptr_t*)((char*)var + 0x16084))
#define GetAresHouseTypeExt(var)  (void*)(*(uintptr_t*)((char*)var + 0xC4))
#define GetAresBuildingTypeExt(var) (void*)(*(uintptr_t*)((char*)var + 0xE24))
#define GetAresTechnoTypeExt(var) (void*)(*(uintptr_t*)((char*)var + 0x2FC))
#define GetAresBulletTypeExt(var) (void*)(*(uintptr_t*)((char*)var + 0x2C4))
#define GetAresAresWarheadTypeExt(var) (void*)(*(uintptr_t*)((char*)var + 0x1CC))

// TechnoExt
#define GetDisableWeaponTimer(techno) (*(CDTimerClass*)(((char*)GetAresTechnoExt(techno)) + 0x50))
#define Is_DriverKilled(techno) (*(bool*)(((char*)GetAresTechnoExt(techno)) + 0x9C))
#define Is_SurvivorsDone(techno) (*(bool*)(((char*)GetAresTechnoExt(techno)) + 0x9B))
#define Is_Operated(techno)  (*(bool*)(((char*)GetAresTechnoExt(techno)) + 0x9D))
#define HijackerHealth(techno) (*(int*)(((char*)GetAresTechnoExt(techno)) + 0x68))
#define HijackerVeterancy(techno) (*(float*)(((char*)GetAresTechnoExt(techno)) + 0x70))
#define HijackerOwner(techno) (*(HouseClass**)(((char*)GetAresTechnoExt(techno)) + 0x6C))
#define GetSonarTimer(techno) (*(CDTimerClass*)(((char*)GetAresTechnoExt(techno)) + 0x44))
#define GetSelfHealingCombatTimer(techno) (*(CDTimerClass*)(((char*)GetAresTechnoExt(techno)) + 0x5C))
#define Ares_TemporalWeapon(techno) (*(BYTE*)(((char*)GetAresTechnoExt(techno)) + 0xA))
#define Ares_ParasiteWeapon(techno) (*(BYTE*)(((char*)GetAresTechnoExt(techno)) + 0xB))

#define AresGarrisonedIn(techno) (*(BuildingClass**)(((char*)GetAresTechnoExt(techno)) + 0xC))
#define AresEMPSparkleAnim(techno) (*(AnimClass**)(((char*)GetAresTechnoExt(techno)) + 0x10))
#define AresBuildingLight(techno) (*(BuildingLightClass**)(((char*)GetAresTechnoExt(techno)) + 0x38))

#define AresEMPLastMission(techno) (*(Mission*)(((char*)GetAresTechnoExt(techno)) + 0x14))

#define TechnoValueAmount(techno) (*(int*)(((char*)GetAresTechnoExt(techno)) + 0xA8))

#define AE_ArmorMult(techno) (*(double*)(((char*)GetAresTechnoExt(techno)) + 0x88))
#define AE_Cloak(techno) (*(bool*)(((char*)GetAresTechnoExt(techno)) + 0x98))
#define AE_FirePowerMult(techno) (*(double*)(((char*)GetAresTechnoExt(techno)) + 0x80))
#define AE_SpeedMult(techno) (*(double*)(((char*)GetAresTechnoExt(techno)) + 0x90))
#define AE_ROF(techno) (*(double*)(((char*)GetAresTechnoExt(techno)) + 0x78))

#define GetAEData(techno) (*(AEData*)(((char*)GetAresTechnoExt(techno)) + 0x20))

#define TakeVehicleMode(techno) (*(bool*)(((char*)GetAresTechnoExt(techno)) + 0xB0))
#define AltOccupy_HasValue(techno)  (*(bool*)(((char*)GetAresTechnoExt(techno)) + 0x9A)) 
#define AltOccupy_Value(techno)  (*(bool*)(((char*)GetAresTechnoExt(techno)) + 0x99)) 

#define EMPulseTarget(techno) (*(AbstractClass**)(((char*)GetAresTechnoExt(techno)) + 0xA4))
//#define AttachedSuperWeapon(techno) (*(SuperClass**)(((char*)GetAresTechnoExt(techno)) + 0xA0))
#define GetCloakSkipTimer(techno) (*(CDTimerClass*)(((char*)GetAresTechnoExt(techno)) + 0x44))

// HouseExt
#define Is_NavalYardSpied(var) (*(bool*)((char*)GetAresHouseExt(var) + 0x48))
#define Is_AirfieldSpied(var) (*(bool*)((char*)GetAresHouseExt(var) + 0x49))
#define Is_BuildingProductionSpied(var) (*(bool*)((char*)GetAresHouseExt(var) + 0x4A))
#define RadarPresist(var) (*(IndexBitfield<HouseClass*>*)((char*)GetAresHouseExt(var) + 0x44))
#define StolenTechnoType(var) (*(std::bitset<32>*)((char*)GetAresHouseExt(var) + 0x40))
#define AuxPower(var) (*(int*)((char*)GetAresHouseExt(var) + 0xC))
#define BatteryCount(var) (*(int*)((char*)GetAresHouseExt(var) + 0x10))
#define KeepAlivesCount(var) (*(int*)((char*)GetAresHouseExt(var) + 0x18))
#define KeepAlivesBuildingCount(var) (*(int*)((char*)GetAresHouseExt(var) + 0x1C))

// BldTypeExt
#define Is_FirestromWall(techno) (*(bool*)((char*)GetAresBuildingTypeExt(techno) + 0x5D))
#define Is_Passable(techno) (*(bool*)((char*)GetAresBuildingTypeExt(techno) + 0x5E))
#define TunnelIdx(var) (*(int*)(((char*)GetAresBuildingTypeExt(var)) + 0x244))
#define Is_Academy(var) (*(bool*)((char*)GetAresBuildingTypeExt(var) + 0x138))
//
#define Is_CurrentlyRaided(var) (*(bool*)(((char*)GetAresBuildingExt(var)) + 0x8))
#define FreeUnitDone(var) (*(bool*)(((char*)GetAresBuildingExt(var)) + 0xC))
#define Ares_AboutToChronoshift(var) (*(bool*)(((char*)GetAresBuildingExt(var)) + 0xD))
#define Is_FromSW(var) (*(bool*)(((char*)GetAresBuildingExt(var)) + 0xE))
#define ToggalePowerHasPower(var) (*(bool*)(((char*)GetAresBuildingExt(var)) +0x51))
//
#define GetSelfHealingDleayAmount(var) (*(int*)(((char*)GetAresTechnoTypeExt(var)) + 0x4A8))
#define GetNoSpawnAlt(var) (*(VoxelStruct**)(((char*)GetAresTechnoTypeExt(var)) + 0x1E0))
#define GetCursorDeploy(var) (*(int*)(((char*)GetAresTechnoTypeExt(var)) + 0x590))
#define GetCursorNoDeploy(var) (*(int*)(((char*)GetAresTechnoTypeExt(var)) + 0x594))
#define GetCursorMove(var) (*(int*)(((char*)GetAresTechnoTypeExt(var)) + 0x5A0))
#define GetCursorNoMove(var) (*(int*)(((char*)GetAresTechnoTypeExt(var)) + 0x5A4))
#define GetCursorEnter(var) (*(int*)(((char*)GetAresTechnoTypeExt(var)) + 0x598))
#define GetCursorNoEnter(var) (*(int*)(((char*)GetAresTechnoTypeExt(var)) + 0x59C))
#define GetCameoPCXSurface(var) (*(BSurface**)(((char*)GetAresTechnoTypeExt(var)) + 0x228))
#define GetCameoSHPConvert(var) (*(ConvertClass**)(((char*)GetAresTechnoTypeExt(var)) + 0x120))
//
#define Is_MaliciousWH(wh) (*(bool*)((((char*)GetAresAresWarheadTypeExt(wh)) + 0x75)))
#define GetSonarDur(wh) (*(int*)(((char*)GetAresAresWarheadTypeExt(wh)) + 0xF8))
#define GetDisableWeaponDur(wh) (*(int*)(((char*)GetAresAresWarheadTypeExt(wh)) + 0xFC))
#define GetFlashDuration(wh) (*(int*)(((char*)GetAresAresWarheadTypeExt(wh)) + 0x100))

enum class NewFactoryState
{
	NoFactory = 0, // there is no factory building for this
	NotFound = 1, //
	Unpowered = 2, //
	Available_Alternative = 3, // Ares 3.0
	Available_Primary = 4 // Ares 3.0
};


struct AresFactoryStateRet
{
	NewFactoryState state;
	BuildingClass* Factory;
};

struct AresData
{
	static HMODULE AresDllHmodule;
	static uintptr_t AresBaseAddress;
	static uintptr_t PhobosBaseAddress;
	static DWORD AresMemDelAddrFinal;
	static DWORD AresMemAllocAddrFinal;

	//number of static instance
	static constexpr int AresStaticInstanceCount = 9;
	//number of call for `CustomPalette::ReadFromINI`
	static constexpr int AresCustomPaletteReadCount = 5;

	static DWORD AresCustomPaletteReadFuncFinal[AresCustomPaletteReadCount];
	static DWORD AresStaticInstanceFinal[AresData::AresStaticInstanceCount];

	static bool Init();
	static void UnInit();

	// here be known Ares functions
	static bool ConvertTypeTo(TechnoClass* pFoot, TechnoTypeClass* pConvertTo);
	static void SpawnSurvivors(FootClass* const pThis, TechnoClass* const pKiller, const bool Select, const bool IgnoreDefenses);
	static void RecalculateStat(TechnoClass* const pTechno);
	static bool ReverseEngineer(BuildingClass* const pBld, TechnoTypeClass* const pTechnoType);
	static Action GetInfActionOverObject(InfantryClass* const pInf, BuildingClass* const pBld);
	static void SetMouseCursorAction(size_t CursorIdx, Action nAction, bool bShrouded);
	static CanBuildResult PrereqValidate(HouseClass* const pHouse, TechnoTypeClass* const pItem, bool const buildLimitOnly, bool const includeQueued);
	static bool SW_Activate(SuperClass* pSuper, CellStruct cell, bool isPlayer);
	static void TechnoExt_ExtData_DepositTiberium(TechnoClass* const pTechno, float const amount, float const bonus, int const idxType);
	static void HouseExt_ExtData_ApplyAcademy(HouseClass* const pThis, TechnoClass* const pTarget, AbstractType Type);
	static VoxelStruct* GetTurretsVoxel(TechnoTypeClass* const pThis, int index);
	static VoxelStruct* GetBarrelsVoxel(TechnoTypeClass* const pThis, int index);
	static void TechnoTransferAffects(TechnoClass* const pFrom, TechnoClass* const pTo);
	static bool IsGenericPrerequisite(TechnoTypeClass* const pThis);
	static int GetSelfHealAmount(TechnoClass* const pTechno);
	static bool IsOperated(TechnoClass* const pTechno);

	static ConvertClass* GetBulletTypeConvert(BulletTypeClass* pThis);
	static void WarheadTypeExt_ExtData_ApplyKillDriver(WarheadTypeClass* pThis, TechnoClass* const pAttacker, TechnoClass* const pVictim);
	static void MouseCursorTypeLoadDefault();

	static AresFactoryStateRet* HouseExt_HasFactory(AresFactoryStateRet* nBuff, HouseClass const* const Owner, TechnoTypeClass const* const pType, bool bSkipAircraft, bool bRequirePower, bool bCheckCanBuild, bool a7);
	static int HouseExt_GetBuildLimitRemaining(HouseClass const* const Owner, TechnoTypeClass const* const pType);
	static int NOINLINE CallAresBuildingClass_Infiltrate(REGISTERS* R);
	static int NOINLINE CallAresArmorType_FindIndex(REGISTERS* R);

	static std::vector<FootClass*>* GetTunnelArray(BuildingTypeClass* const pBld, HouseClass* const pOwner);
	static void UpdateAEData(AEData* const pAE);
	static void JammerClassUnjamAll(JammerClass* const pJamm);
	static void CPrismRemoveFromNetwork(cPrismForwarding* const pThis , bool bCease);

	static void applyIonCannon(WarheadTypeClass* pWH, CoordStruct* pTarget);
	static bool applyPermaMC(WarheadTypeClass* pWH, HouseClass* pOwner, AbstractClass* pTarget);
	static void applyIC(WarheadTypeClass* pWH, CoordStruct* pTarget, HouseClass* pOwner, int Damage);
	static void applyEMP(WarheadTypeClass* pWH, CoordStruct* pTarget, TechnoClass* pOwner);
	static void applyAE(WarheadTypeClass* pWH, CoordStruct* pTarget, HouseClass* pOwner);

	static void EvalRaidStatus(BuildingClass* pBuilding);
	static bool IsActiveFirestormWall(BuildingClass* pBuilding , HouseClass* pOwner);
	static bool ImmolateVictim(BuildingClass* pBuilding , FootClass* pTarget , bool Destroy);

	static void DisableEMPEffect(TechnoClass* pTechno);
	static bool CloakDisallowed(TechnoClass* pTechno, bool allowPassive);
	static bool CloakAllowed(TechnoClass* pTechno);

	static bool RemoveAE(AEData* pAE);

	static void FlyingStringsAdd(TechnoClass* pTech, bool bSomething);
	static void CalculateBounty(TechnoClass* pThis, TechnoClass* pKiller);
	static void SetSpotlight(TechnoClass* pThis , BuildingLightClass* pSpotlight);
	static bool IsPowered(TechnoClass* pThis);
	static bool IsDriverKillable(TechnoClass* pThis, double tresh);
	static bool KillDriverCore(TechnoClass* pThis, HouseClass* pToHouse, TechnoClass* pKiller, bool removeVet);
	static void FireIronCurtain(TeamClass* pTeam, ScriptActionNode* pNode , bool ntrhd);
	static void RespondToFirewall(HouseClass* pHouse, bool Active);
	static int RequirementsMet(HouseClass* pHouse , TechnoTypeClass* pTech);
	static void UpdateAcademy(HouseClass* pThis, TechnoClass* pTechno, bool bAdded);

	static void* Ares_SWType_ExtMap_Find(SuperWeaponTypeClass* pType);
	static void SetSWMouseCursorAction(size_t CursorIdx, bool bShrouded, int nAction);
};

namespace AresMemory
{
	// the ares operator delete
	static inline NAKED void __cdecl AresDeallocate(const void* mem)
	{
		JMP(AresData::AresMemDelAddrFinal);
	}

	// the ares operator new
	static inline NAKED void* __cdecl AresAllocate(size_t sz)
	{
		JMP(AresData::AresMemAllocAddrFinal);
	}

	static NOINLINE void* AresAllocateChecked(size_t sz)
	{
		return AresAllocate(sz);
	}

	template <typename T>
	struct AresAllocator
	{
		using value_type = T;

		constexpr AresAllocator() noexcept = default;

		template <typename U>
		constexpr AresAllocator(const AresAllocator<U>&) noexcept { }

		constexpr bool operator == (const AresAllocator&) const noexcept { return true; }
		constexpr bool operator != (const AresAllocator&) const noexcept { return false; }

		[[nodiscard]] T* allocate(size_t n) {
        	if (n > std::numeric_limits<size_t>::max() / sizeof(T))
            	throw std::bad_array_new_length();

        		if (auto p = static_cast<T*>(AresAllocate(n * sizeof(T)))) {
            		return p;
       	 		}

       	 	throw std::bad_alloc();
   		}

		void destroy(T* const ptr) const noexcept
		{
			std::destroy_at(ptr);
		}

		void deallocate(T* const ptr, size_t count) noexcept
		{
			AresDeallocate(ptr);
		}

		template<class U, class... Args>
		void construct(U* p, Args&&... args)  const noexcept
		{
			std::construct_at(p, std::forward<Args>(args)...);
		}

	};

	template <typename T, typename... TArgs>
	static inline T* Create(TArgs&&... args)
	{
		static_assert(std::is_constructible<T, TArgs...>::value, "Cannot construct T from TArgs.");

		AresAllocator<T> alloc;
		return Memory::Create<T>(alloc, std::forward<TArgs>(args)...);
	}

	template<bool calldtor = false, bool check = true, typename T>
	static inline void Delete(T* ptr)
	{
		AresAllocator<T> alloc;
		Memory::Delete<calldtor>(std::bool_constant<check>::type(), alloc, ptr);
	}

	template<bool check = true, typename T>
	static inline void CallDTOR(T* ptr)
	{
		AresAllocator<T> alloc;
		Memory::Delete<true>(std::bool_constant<check>::type(), alloc, ptr);
	}

}

struct AresDeleter
{
	template <typename T>
	void operator ()(T* ptr) noexcept
	{
		if (ptr)
		{
			AresMemory::Delete(ptr);
		}
	}
};

struct AresDeleterNoCheck
{
	template <typename T>
	void operator ()(T* ptr) noexcept
	{
		AresMemory::Delete(ptr);
	}
};

struct AresDTORCaller
{
	template <typename T>
	void operator ()(T* ptr) noexcept
	{
		AresMemory::CallDTOR(ptr);
	}
};

#define Debug_bTrackParseErrors (*((bool*)(AresData::AresStaticInstanceFinal[3])))
#define ActiveSFW (*((SuperClass**)(AresData::AresStaticInstanceFinal[4])))
#define EvaTypes (*((std::vector<const char*, AresMemory::AresAllocator<const char*>>*)(AresData::AresStaticInstanceFinal[5])))
#define PoweredUnitUptr(techno) (*(PoweredUnitClass**)(((char*)GetAresTechnoExt(techno)) + 0x1C))
#define RadarJammerUptr(techno) (*(JammerClass**)(((char*)GetAresTechnoExt(techno)) + 0x18))
#define RegisteredJammers(techno) (*(PhobosMap<TechnoClass*, bool, AresMemory::AresAllocator<std::pair<TechnoClass*,bool>>>*)(((char*)GetAresBuildingExt(techno)) + 0x40))
#define PrimsForwardingPtr(techno) (*(cPrismForwarding*)(((char*)GetAresBuildingExt(techno)) + 0x10))
#define GetGunnerName(var) (*(std::vector<CSFText,AresMemory::AresAllocator<CSFText>>*)(((char*)GetAresTechnoTypeExt(var)) + 0xC8))
#define GetPilotTypeVec(var) (*(std::vector<InfantryTypeClass*,AresMemory::AresAllocator<InfantryTypeClass*>>*)(((char*)GetAresTechnoTypeExt(var)) + 0x8))
#define ReverseEngineeredTechnoType(var) (*(std::vector<TechnoTypeClass*,AresMemory::AresAllocator<TechnoTypeClass*>>*)((char*)GetAresHouseExt(var) + 0x34))
#define VeteranBuildings(var) (*(std::vector<BuildingTypeClass*,AresMemory::AresAllocator<BuildingTypeClass*>>*)((char*)GetAresHouseTypeExt(var) + 0x15C))
#define OverpoweredBuildingType(var) (*(std::vector<BuildingTypeClass*,AresMemory::AresAllocator<BuildingTypeClass*>>*)((char*)GetAresHouseExt(var) + 0x7C))
//#define GetDefaultTargetingArrayValue() (*((std::array<const AITargetingModeInfo , (size_t)SuperWeaponAITargetingMode::count>*)(AresData::AresStaticInstanceFinal[6])))
#define SW_Firewall_Type (*((SuperWeaponType*)(AresData::AresStaticInstanceFinal[7])))
#define Ares_CurrentSWType (*((SuperWeaponTypeClass**)(AresData::AresStaticInstanceFinal[8])))
