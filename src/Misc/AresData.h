#pragma once
#include <Windows.h>

#include <YRPPCore.h>
#include <GeneralDefinitions.h>
#include <Memory.h>

#include <ArrayClasses.h>
#include <vector>

#include <CoordStruct.h>

class AbstractClass;
class TechnoClass;
class TechnoTypeClass;
class FootClass;
class BuildingClass;
class BuildingTypeClass;
class InfantryClass;
class HouseClass;
class SuperClass;
struct VoxelStruct;
class ConvertClass;
class BulletTypeClass;
class WarheadTypeClass;
class BuildingLightClass;
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

// TODO : Allocation and deallocation for Ares vector and pointer stuffs
// becarefull when doing operation that involve those , it can result on undefine behaviour

//TODO : port these
#define GetAresTechnoExt(var) (void*)(*(uintptr_t*)((char*)var + 0x154))
#define GetAresBuildingExt(var)  (void*)(*(uintptr_t*)((char*)var + 0x71C))
#define GetAresHouseExt(var)  (void*)(*(uintptr_t*)((char*)var + 0x16084))
#define GetAresBuildingTypeExt(var) (void*)(*(uintptr_t*)((char*)var + 0xE24))
#define GetAresTechnoTypeExt(var) (void*)(*(uintptr_t*)((char*)var + 0x2FC))
#define GetAresBulletTypeExt(var) (void*)(*(uintptr_t*)((char*)var + 0x2C4))
#define GetAresAresWarheadTypeExt(var) (void*)(*(uintptr_t*)((char*)var + 0x1CC))

// TechnoExt
//#define GetDisableWeaponTimer(techno) (*(CDTimerClass*)(((char*)GetAresTechnoExt(var)) + 0x50))
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

#define AE_ArmorMult(techno) (*(double*)(((char*)GetAresTechnoExt(techno)) + 0x88))
#define AE_Cloak(techno) (*(bool*)(((char*)GetAresTechnoExt(techno)) + 0x98))
#define AE_FirePowerMult(techno) (*(double*)(((char*)GetAresTechnoExt(techno)) + 0x80))
#define AE_SpeedMult(techno) (*(double*)(((char*)GetAresTechnoExt(techno)) + 0x90))

#define GetAEData(techno) (*(AEData*)(((char*)GetAresTechnoExt(techno)) + 0x20))

// HouseExt
#define Is_NavalYardSpied(var) (*(bool*)((char*)GetAresHouseExt(var) + 0x48))
#define Is_AirfieldSpied(var) (*(bool*)((char*)GetAresHouseExt(var) + 0x49))
#define Is_BuildingProductionSpied(var) (*(bool*)((char*)GetAresHouseExt(var) + 0x4A))
#define RadarPresist(var) (*(IndexBitfield<HouseClass*>*)((char*)GetAresHouseExt(var) + 0x44))
#define ReverseEngineeredTechnoType(var) (*(std::vector<TechnoTypeClass*>*)((char*)GetAresHouseExt(var) + 0x34))
#define StolenTechnoType(var) (*(std::bitset<32>*)((char*)GetAresHouseExt(var) + 0x40))


// BldTypeExt
#define Is_FirestromWall(techno) (*(bool*)((char*)GetAresBuildingTypeExt(techno) + 0x5D))
#define Is_Passable(techno) (*(bool*)((char*)GetAresBuildingTypeExt(techno) + 0x5E))
#define TunnelIdx(var) (*(int*)(((char*)GetAresBuildingTypeExt(var)) + 0x244))

//
#define Ares_AboutToChronoshift(var) (*(bool*)(((char*)GetAresBuildingExt(var)) + 0xD))

//
#define GetSelfHealingDleayAmount(var) (*(int*)(((char*)GetAresTechnoTypeExt(var)) + 0x4A8))
#define GetGunnerName(var) (*(std::vector<CSFText>*)(((char*)GetAresTechnoTypeExt(var)) + 0xC8))
#define GetNoSpawnAlt(var) (*(VoxelStruct**)(((char*)GetAresTechnoTypeExt(var)) + 0x1E0))
#define GetCursorDeploy(var) (*(int*)(((char*)GetAresTechnoTypeExt(var)) + 0x590))
#define GetCursorNoDeploy(var) (*(int*)(((char*)GetAresTechnoTypeExt(var)) + 0x594))
#define GetCursorMove(var) (*(int*)(((char*)GetAresTechnoTypeExt(var)) + 0x5A0))
#define GetCursorNoMove(var) (*(int*)(((char*)GetAresTechnoTypeExt(var)) + 0x5A4))
#define GetCursorEnter(var) (*(int*)(((char*)GetAresTechnoTypeExt(var)) + 0x598))
#define GetCursorNoEnter(var) (*(int*)(((char*)GetAresTechnoTypeExt(var)) + 0x59C))
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
	enum FunctionIndices : int
	{
		ConvertTypeToID = 0,
		SpawnSurvivorsID = 1,
		RecalculateStatID = 2,
		ReverseEngineerID = 3,
		GetInfActionOverObjectID = 4,
		SetMouseCursorActionID = 5,
		HouseCanBuildID = 6,
		SWActivateID = 7,
		DepositTiberiumID = 8,
		ApplyAcademyID = 9,
		GetTurretVXLDataID = 10,
		TechnoTransferAffectsID = 11,
		IsGenericPrerequisiteID = 12,
		GetSelfHealAmountID = 13,
		ExtMapFindID = 14,
		BulletTypeExtGetConvertID = 15,
		ApplyKillDriverID = 16,
		MouseCursorTypeLoadDefaultID = 17,
		HouseExtHasFactoryID = 18,
		HouseExtGetBuildLimitRemainingID = 19,

		CustomPaletteReadFronINIID = 20,
		GetBarrelsVoxelID = 21,
		IsOperatedID = 22,
		GetTunnelArrayID = 23,
		UpdateAEDataID = 24,
		JammerclassUnjamAllID = 25,
		CPrismRemoveFromNetworkID = 26,


		//WHextfunc 
		applyIonCannonID = 27, //52790 , pWHExt , CoordStruct*
		applyPermaMCID = 28, //53980 , pWHExt , AbstractClass*
		applyICID = 29, // 53540 , pWHExt , HouseClass* , int
		applyEMPID = 30, // 53520 , pWHExt , CoordStruct* , TechnoClass*
		applyAEID = 31, // 533B0 , pWHExt , CoordStruct* , TechnoClass*
		applyOccupantDamageID = 32, // 53940 , BulletClass* 

		EvalRaidStatusID = 33,
		IsActiveFirestormWallID = 34, //BuildingExt  IsActiveFirestormWall BuildingClass* HouseClass*
		ImmolateVictimID = 35, //BuildingExt  ImmolateVictim BuildingClassExt* FootClass* bool
		count
	};

	enum Version
	{
		Unknown = -1,
		Ares30p = 0,

		vcount
	};

	static HMODULE AresDllHmodule;
	static uintptr_t AresBaseAddress;
	static uintptr_t PhobosBaseAddress;
	static DWORD AresMemDelAddrFinal;
	static DWORD AresMemAllocAddrFinal;

	// number of Ares functions we use
	static constexpr int AresFunctionCount = FunctionIndices::count;
	// number of Ares versions we support
	static constexpr int AresVersionCount = Version::vcount;
	//number of static instance
	static constexpr int AresStaticInstanceCount = 4;
	//number of call for `CustomPalette::ReadFromINI`
	static constexpr int AresCustomPaletteReadCount = 5;


	// timestamp bytes for each version
	static constexpr DWORD AresTimestampBytes[AresData::AresVersionCount] =
	{
		0x61daa114, // 3.0p
	};

	static constexpr DWORD AresStaticInstanceTable[AresStaticInstanceCount] = {
		0x0C2B84, //ParticleSystemExt
		0x0C2DD0, //WeaponTypeExt

		0x0C2A00, //Debug::bTrackParserErrors

		0x0C2A54, //HouseExt::FSW
	};

	// offsets of function addresses for each version
	static constexpr DWORD AresFunctionOffsets[AresData::AresVersionCount * AresData::AresFunctionCount] =
	{
		0x044130,	// ConvertTypeTo
		0x047030, // TechnoExt::SpawnSurvivors
		0x046C10, //TechnoExt::ExtData::RecalculateStat
		0x013390, // static BuildingExt::ReverseEngineer 
		0x025DF0, // static  GetInfActionOverObject
		0x058AB0, // static MouseCursor::SetAction
		0x022580, // static HouseExt::canBuild
		0x032A00, // static SWTypeExt::Activate
		0x044D10, // TechnoExt::ExtData::DepositTiberium
		0x0211D0, // HouseExt::ExtData::ApplyAcademy
		0x03E7C0, // TechnoTypeExt::ExtData::GetTurretsVoxel
		0x0465F0, // TechnoExt::ExtData::TechnoTransferAffects
		0x03E950, // TechnoTypeExt::ExtData::IsGenericPrerequisite
		0x0459F0, // TechnoExt::ExtData::GetSelfHealAmount
		0x058900, // ExtMap.Find
		0x019A50, // BulletTypeExtGetConvert
		0x0537F0, // WhExt::ApplyKillDriver
		0x007100, // MouseCursorTypeLoadDefault
		0x0217C0, // HouseExtHasFactory
		0x0212F0, // HouseExtGetBuildLimitRemaining
		0x077210, // CustomPaletteReadFronINI
		0x03E510, // GetBarrelsVoxel
		0x045FB0, // IsOperated
		0x00DA30, // GetTunnelArray
		0x059650, // UpdateAEData
		0x0693D0, // JammerClassUnjamAll
		0x0192A0, // CPrismRemoveFromNetwork

		0x052790, // applyIonCannon , pWHExt , CoordStruct*
		0x053980, // applyPermaMC , pWHExt , AbstractClass*
		0x053540, // applyIC ,  pWHExt , HouseClass* , int
		0x053520, // applyEMP , pWHExt , CoordStruct* , TechnoClass*
		0x0533B0, // applyAE , pWHExt , CoordStruct* , TechnoClass*
		0x053940, // applyOccupantDamage , BulletClass* 

		0x013C50, //BuildingExt  EvalRaidStatus
		0x012D00, //BuildingExt  IsActiveFirestormWall BuildingClass* HouseClass*
		0x0124C0, //BuildingExt  ImmolateVictim BuildingClassExt* FootClass* bool
	};

	static constexpr DWORD AAresCustomPaletteReadTable[AresCustomPaletteReadCount] = {
		0x005B59, //Ares UI
		0x00A544, //AnimTypeExt
		0x029CF1, //ParticleTypeExt
		0x03456B, //SWTypeExt
		0x03F0AB, //TechnoTypeExt

	};

	// storage for absolute addresses of functions (module base + offset)
	static DWORD AresFunctionOffsetsFinal[AresData::AresFunctionCount];
	static DWORD AresCustomPaletteReadFuncFinal[AresCustomPaletteReadCount];
	static DWORD AresStaticInstanceFinal[AresData::AresStaticInstanceCount];
	// numeric id of currently used version, zero-indexed, -1 is unknown or missing
	static Version AresVersionId;
	// is Ares detected and version known?
	static bool CanUseAres;

	static uintptr_t GetModuleBaseAddress(const char* modName);
	static bool Init();
	static void UnInit();

	template<int idx, typename Tret, typename... TArgs>
	struct AresStdcall
	{
		static_assert(idx < AresData::AresFunctionCount, "Index Out Of Bound !");
		using fp_type = Tret(__stdcall*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			return reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename... TArgs>
	struct AresStdcall<idx, void, TArgs...>
	{
		static_assert(idx < AresData::AresFunctionCount, "Index Out Of Bound !");
		using fp_type = void(__stdcall*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename Tret, typename... TArgs>
	struct AresCdecl
	{
		static_assert(idx < AresData::AresFunctionCount, "Index Out Of Bound !");
		using fp_type = Tret(__cdecl*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			return reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename... TArgs>
	struct AresCdecl<idx, void, TArgs...>
	{
		static_assert(idx < AresData::AresFunctionCount, "Index Out Of Bound !");
		using fp_type = void(__cdecl*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename Tret, typename... TArgs>
	struct AresFastcall
	{
		static_assert(idx < AresData::AresFunctionCount, "Index Out Of Bound !");
		using fp_type = Tret(__fastcall*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			return reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename... TArgs>
	struct AresFastcall<idx, void, TArgs...>
	{
		static_assert(idx < AresData::AresFunctionCount, "Index Out Of Bound !");
		using fp_type = void(__fastcall*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename Tret, typename TThis, typename... TArgs>
	struct AresThiscall
	{
		static_assert(idx < AresData::AresFunctionCount, "Index Out Of Bound !");
		using fp_type = Tret(__fastcall*)(TThis, void*, TArgs...);
		decltype(auto) operator()(TThis pThis, TArgs... args) const
		{
			return reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(pThis, nullptr, args...);
		}
	};

	template<int idx, typename TThis, typename... TArgs>
	struct AresThiscall<idx, void, TThis, TArgs...>
	{
		static_assert(idx < AresData::AresFunctionCount, "Index Out Of Bound !");
		using fp_type = void(__fastcall*)(TThis, void*, TArgs...);
		void operator()(TThis pThis, TArgs... args) const
		{
			reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(pThis, nullptr, args...);
		}
	};

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

		T* allocate(const size_t count) const noexcept
		{
			return static_cast<T*>(AresAllocateChecked(count * sizeof(T)));
		}

		void destroy(T* const ptr) const noexcept
		{
			std::destroy_at(ptr);
		}

		void deallocate(T* const ptr, size_t count) const noexcept
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

#define Debug_bTrackParseErrors (*(reinterpret_cast<bool*>(AresData::AresStaticInstanceFinal[2])))
#define ActiveSFW (*(reinterpret_cast<SuperClass**>(AresData::AresStaticInstanceFinal[3])))
#define PoweredUnitUptr(techno) (*(PoweredUnitClass**)(((char*)GetAresTechnoExt(techno)) + 0x1C))
#define RadarJammerUptr(techno) (*(JammerClass**)(((char*)GetAresTechnoExt(techno)) + 0x18))
#define RegisteredJammers(techno) (*(PhobosMap<TechnoClass*, bool>*)(((char*)GetAresBuildingExt(techno)) + 0x40))
#define PrimsForwardingPtr(techno) (*(cPrismForwarding*)(((char*)GetAresBuildingExt(techno)) + 0x10))

