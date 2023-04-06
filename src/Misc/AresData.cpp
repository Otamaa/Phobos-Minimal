#include "AresData.h"

#include <ASMMacros.h>
#include <Phobos.h>
#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <tlhelp32.h>

class TechnoClass;
class TechnoTypeClass;

uintptr_t AresData::PhobosBaseAddress = 0x0;
uintptr_t AresData::AresBaseAddress = 0x0;
HMODULE AresData::AresDllHmodule = nullptr;
AresData::Version AresData::AresVersionId = AresData::Version::Unknown;
bool AresData::CanUseAres = false;
DWORD AresData::AresFunctionOffsetsFinal[AresData::AresFunctionCount];

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

uintptr_t AresData::GetModuleBaseAddress(const char* modName)
{
	return Patch::GetModuleBaseAddress(modName);
}

void AresData::Init()
{
	AresBaseAddress = GetModuleBaseAddress(ARES_DLL_S);
	PhobosBaseAddress = GetModuleBaseAddress(PHOBOS_DLL_S);
	Debug::LogDeferred("[Phobos] Phobos base address: 0x%X.\n", PhobosBaseAddress);

	if (!AresBaseAddress)
	{
		Debug::LogDeferred("[Phobos] Failed to detect Ares. Disabling integration.\n");
		return;
	}
	else
	{
		Debug::LogDeferred("[Phobos] Ares base address: 0x%X.\n", AresBaseAddress);
	}

	// find offset of PE header
	const int PEHeaderOffset = *(DWORD*)(AresBaseAddress + 0x3c);
	// find absolute address of PE header
	const DWORD* PEHeaderPtr = (DWORD*)(AresBaseAddress + PEHeaderOffset);
	// read the timedatestamp at 0x8 offset
	const DWORD TimeDateStamp = *(PEHeaderPtr + 2);
	switch (TimeDateStamp)
	{
	case AresTimestampBytes[Version::Ares30p]:
		AresVersionId = Version::Ares30p;
		CanUseAres = true;
		Debug::LogDeferred("[Phobos] Detected Ares 3.0p1.\n");
		break;
	default:
		Debug::LogDeferred("[Phobos] Detected a version of Ares that is not supported by Phobos. Disabling integration.\n");
		break;
	}

	if (CanUseAres && GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN, ARES_DLL, &AresDllHmodule))
	{
		for (int i = 0; i < AresData::AresFunctionCount; i++)
			AresData::AresFunctionOffsetsFinal[i] = AresData::AresBaseAddress + AresData::AresFunctionOffsets[i * AresData::AresVersionCount + AresVersionId];
	}
}

void AresData::UnInit()
{
	if (!AresBaseAddress)
		return;

	if (CanUseAres)
		FreeLibrary(AresDllHmodule);
}

bool AresData::ConvertTypeTo(TechnoClass* pFoot, TechnoTypeClass* pConvertTo)
{
	return AresStdcall<ConvertTypeToID, bool, TechnoClass*, TechnoTypeClass*>()(pFoot, pConvertTo);
}

void AresData::SpawnSurvivors(FootClass* const pThis, TechnoClass* const pKiller, const bool ISelect, const bool IgnoreDefenses)
{
	AresStdcall<SpawnSurvivorsID, void, FootClass*, TechnoClass*, bool, bool>()(pThis, pKiller, ISelect, IgnoreDefenses);
}

void AresData::RecalculateStat(TechnoClass* const pTechno)
{
	AresThiscall<RecalculateStatID, void,  void*>()(GetAresTechnoExt(pTechno));
}

bool AresData::ReverseEngineer(BuildingClass* const pBld, TechnoTypeClass* const pTechnoType)
{
	return AresStdcall<ReverseEngineerID, bool, BuildingClass* , TechnoTypeClass*>()(pBld , pTechnoType);
}

Action AresData::GetInfActionOverObject(InfantryClass* const pInf, BuildingClass* const pBld)
{
	return AresStdcall<GetInfActionOverObjectID, Action, InfantryClass*  , BuildingClass*>()(pInf, pBld);
}

void AresData::SetMouseCursorAction(size_t CursorIdx, Action nAction, bool bShrouded)
{
	AresStdcall<SetMouseCursorActionID, void, size_t, Action, bool>()(CursorIdx , nAction , bShrouded);
}

CanBuildResult AresData::PrereqValidate(HouseClass* const pHouse, TechnoTypeClass* const pItem, bool const buildLimitOnly, bool const includeQueued)
{
	return AresStdcall<HouseCanBuildID, CanBuildResult, HouseClass* , TechnoTypeClass*, bool , bool>()(pHouse, pItem , buildLimitOnly , includeQueued);
}

bool AresData::SW_Activate(SuperClass* pSuper, CellStruct cell, bool isPlayer)
{
	return AresStdcall<SWActivateID, bool, SuperClass*, CellStruct, bool>()(pSuper, cell, isPlayer);
}

void AresData::TechnoExt_ExtData_DepositTiberium(TechnoClass* const pTechno, float const amount, float const bonus, int const idxType)
{
	AresThiscall<DepositTiberiumID, void, void* , float , float , int>()(GetAresTechnoExt(pTechno) ,amount , bonus , idxType);
}

void AresData::HouseExt_ExtData_ApplyAcademy(HouseClass* const pThis, TechnoClass* const pTarget, AbstractType Type)
{
	AresThiscall<ApplyAcademyID, void, void*, TechnoClass*, AbstractType>()(GetAresHouseExt(pThis), pTarget, Type);
}

VoxelStruct* AresData::GetTurretsVoxel(TechnoTypeClass* const pThis, int index)
{
	return AresThiscall<GetTurretVXLDataID, VoxelStruct*, void*, int>()(GetAresTechnoTypeExt(pThis), index);
}

void AresData::TechnoTransferAffects(TechnoClass* const pFrom, TechnoClass* const pTo)
{
	AresStdcall<TechnoTransferAffectsID, void, TechnoClass*, TechnoClass*>()(pFrom, pTo);
}

bool AresData::IsGenericPrerequisite(TechnoTypeClass* const pThis)
{
	return AresThiscall<IsGenericPrerequisiteID, bool, void*>()(GetAresTechnoTypeExt(pThis));
}

int AresData::GetSelfHealAmount(TechnoClass* const pTechno)
{
	return AresThiscall<GetSelfHealAmountID, int, void*>()(GetAresTechnoExt(pTechno));
}

int AresData::CallAresBuildingClass_Infiltrate(REGISTERS* R)
{
	if (AresDllHmodule != NULL) {
		CallHook Data = (CallHook)GetProcAddress(AresDllHmodule, "BuildingClass_Infiltrate");
		if (Data != NULL) {
			return (Data)(R);
		}
	}

	return -1;
}

int NOINLINE AresData::CallAresArmorType_FindIndex(REGISTERS* R)
{
	if (AresDllHmodule != NULL)
	{
		CallHook Data = (CallHook)GetProcAddress(AresDllHmodule, "ArmorType_FindIndex");
		if (Data != NULL)
		{
			return (Data)(R);
		}
	}
	
	return-1;
}
