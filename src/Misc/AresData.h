#pragma once
#include <Windows.h>

#include <YRPPCore.h>
#include <GeneralDefinitions.h>

class TechnoClass;
class TechnoTypeClass;
class FootClass;
class BuildingClass;
class InfantryClass;

typedef int (__cdecl *CallHook)(REGISTERS* R);

#define GetAresTechnoExt(var) (void*)(*(uintptr_t*)((char*)var + 0x154))
#define GetAresBuildingExt(var)  (void*)(*(uintptr_t*)((char*)var +0x71C))

struct AresData
{
	enum FunctionIndices
	{
		ConvertTypeToID = 0,
		SpawnSurvivorsID = 1,
		RecalculateStatID = 2,
		ReverseEngineerID = 3,
		GetInfActionOverObjectID = 4,
		SetMouseCursorActionID = 5 ,
	};

	enum Version
	{
		Unknown = -1,
		Ares30 = 0,
		Ares30p,
	};

	static HMODULE AresDllHmodule;
	static uintptr_t AresBaseAddress;
	static uintptr_t PhobosBaseAddress;

	// number of Ares functions we use
	static constexpr int AresFunctionCount = 6;
	// number of Ares versions we support
	static constexpr int AresVersionCount = 2;

	// timestamp bytes for each version
	static constexpr DWORD AresTimestampBytes[AresData::AresVersionCount] =
	{
		0x5fc37ef6,	// 3.0
		0x61daa114, // 3.0p
	};

	// offsets of function addresses for each version
	static constexpr DWORD AresFunctionOffsets[AresData::AresVersionCount * AresData::AresFunctionCount] =
	{
		0x043650, 0x044130,	// ConvertTypeTo
		0x0464C0, 0x047030, // TechnoExt::SpawnSurvivors
		0x0 , 0x046C10, //TechnoExt::RecalculateStat
		0x0 , 0x013390, // static BuildingExt::ReverseEngineer 
		0x0 , 0x025DF0,
		0x0 , 0x058AB0,
	};

	// storage for absolute addresses of functions (module base + offset)
	static DWORD AresFunctionOffsetsFinal[AresData::AresFunctionCount];
	// numeric id of currently used version, zero-indexed, -1 is unknown or missing
	static int AresVersionId;
	// is Ares detected and version known?
	static bool CanUseAres;

	static uintptr_t GetModuleBaseAddress(const char* modName);
	static void Init();
	static void UnInit();

	// here be known Ares functions
	static bool ConvertTypeTo(TechnoClass* pFoot, TechnoTypeClass* pConvertTo);
	static void SpawnSurvivors(FootClass* const pThis, TechnoClass* const pKiller, const bool Select, const bool IgnoreDefenses);
	static void RecalculateStat(TechnoClass* const pTechno);
	static bool ReverseEngineer(BuildingClass* const pBld, TechnoTypeClass* const pTechnoType);
	static Action GetInfActionOverObject(InfantryClass* const pThis, BuildingClass* const pBld);
	static void SetMouseCursorAction(size_t CursorIdx, Action nAction, bool bShrouded);

	static int NOINLINE CallAresBuildingClass_Infiltrate(REGISTERS* R);
	static int NOINLINE CallAresArmorType_FindIndex(REGISTERS* R);

	template<int idx, typename Tret, typename... TArgs>
	struct AresStdcall
	{
		using fp_type = Tret(__stdcall*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			return reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename... TArgs>
	struct AresStdcall<idx, void, TArgs...>
	{
		using fp_type = void(__stdcall*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename Tret, typename... TArgs>
	struct AresCdecl
	{
		using fp_type = Tret(__cdecl*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			return reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename... TArgs>
	struct AresCdecl<idx, void, TArgs...>
	{
		using fp_type = void(__cdecl*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename Tret, typename... TArgs>
	struct AresFastcall
	{
		using fp_type = Tret(__fastcall*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			return reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename... TArgs>
	struct AresFastcall<idx, void, TArgs...>
	{
		using fp_type = void(__fastcall*)(TArgs...);
		decltype(auto) operator()(TArgs... args) const
		{
			reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(args...);
		}
	};

	template<int idx, typename Tret, typename TThis, typename... TArgs>
	struct AresThiscall
	{
		using fp_type = Tret(__fastcall*)(TThis, void*, TArgs...);
		decltype(auto) operator()(TThis pThis, TArgs... args) const
		{
			return reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(pThis, nullptr, args...);
		}
	};

	template<int idx, typename TThis, typename... TArgs>
	struct AresThiscall<idx, void, TThis, TArgs...>
	{
		using fp_type = void(__fastcall*)(TThis, void*, TArgs...);
		void operator()(TThis pThis, TArgs... args) const
		{
			reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(pThis, nullptr, args...);
		}
	};
};