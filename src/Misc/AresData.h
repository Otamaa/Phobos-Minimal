#pragma once
#include <Windows.h>

#include <YRPPCore.h>
#include <GeneralDefinitions.h>

class TechnoClass;
class TechnoTypeClass;
class FootClass;
class BuildingClass;
class InfantryClass;
class HouseClass;
class SuperClass;
struct VoxelStruct;
typedef int (__cdecl *CallHook)(REGISTERS* R);

#define GetAresTechnoExt(var) (void*)(*(uintptr_t*)((char*)var + 0x154))
#define GetAresBuildingExt(var)  (void*)(*(uintptr_t*)((char*)var + 0x71C))
#define GetAresHouseExt(var)  (void*)(*(uintptr_t*)((char*)var + 0x16084))
#define GetAresBuildingTypeExt(var) (void*)(*(uintptr_t*)((char*)var + 0xE24))
#define GetAresTechnoTypeExt(var) (void*)(*(uintptr_t*)((char*)var + 0x2FC))


struct AresData
{
	enum FunctionIndices
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
	};

	enum Version
	{
		Unknown = -1,
		Ares30p = 0,
	};

	static HMODULE AresDllHmodule;
	static uintptr_t AresBaseAddress;
	static uintptr_t PhobosBaseAddress;

	// number of Ares functions we use
	static constexpr int AresFunctionCount = 15;
	// number of Ares versions we support
	static constexpr int AresVersionCount = 1;
	//number of static instance
	static constexpr int AresStaticInstanceCount = 1;

	// timestamp bytes for each version
	static constexpr DWORD AresTimestampBytes[AresData::AresVersionCount] =
	{
		0x61daa114, // 3.0p
	};

	static constexpr DWORD AresStaticInstanceTable[AresStaticInstanceCount] = {
		0x0C2B84, //ParticleSystemExt
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

	};

	// storage for absolute addresses of functions (module base + offset)
	static DWORD AresFunctionOffsetsFinal[AresData::AresFunctionCount];
	static DWORD AresStaticInstanceFinal[AresData::AresStaticInstanceCount];
	// numeric id of currently used version, zero-indexed, -1 is unknown or missing
	static Version AresVersionId;
	// is Ares detected and version known?
	static bool CanUseAres;

	static uintptr_t GetModuleBaseAddress(const char* modName);
	static void Init();
	static void UnInit();

	template<int idx, typename Tret, typename... TArgs>
	struct AresStdcall
	{
		using fp_type = Tret(__stdcall*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			return reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename... TArgs>
	struct AresStdcall<idx, void, TArgs...>
	{
		using fp_type = void(__stdcall*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename Tret, typename... TArgs>
	struct AresCdecl
	{
		using fp_type = Tret(__cdecl*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			return reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename... TArgs>
	struct AresCdecl<idx, void, TArgs...>
	{
		using fp_type = void(__cdecl*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename Tret, typename... TArgs>
	struct AresFastcall
	{
		using fp_type = Tret(__fastcall*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			return reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename... TArgs>
	struct AresFastcall<idx, void, TArgs...>
	{
		using fp_type = void(__fastcall*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename Tret, typename TThis, typename... TArgs>
	struct AresThiscall
	{
		using fp_type = Tret(__fastcall*)(TThis, void*, TArgs...);
		decltype(auto) operator()(TThis pThis, TArgs... args) const
		{
			return reinterpret_cast<fp_type>(AresData::AresFunctionOffsetsFinal[idx])(pThis, nullptr, args...);
		}
	};

	template<int idx, typename TThis, typename... TArgs>
	struct AresThiscall<idx, void, TThis, TArgs...>
	{
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
	static void TechnoTransferAffects(TechnoClass* const pFrom, TechnoClass* const pTo);
	static bool IsGenericPrerequisite(TechnoTypeClass* const pThis);
	static int GetSelfHealAmount(TechnoClass* const pTechno);

	static int NOINLINE CallAresBuildingClass_Infiltrate(REGISTERS* R);
	static int NOINLINE CallAresArmorType_FindIndex(REGISTERS* R);

};