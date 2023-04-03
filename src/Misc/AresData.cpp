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
int AresData::AresVersionId = AresData::Version::Unknown;
bool AresData::CanUseAres = false;
DWORD AresData::AresFunctionOffsetsFinal[AresData::AresFunctionCount];

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
	case AresTimestampBytes[Version::Ares30]:
		AresVersionId = Version::Ares30;
		CanUseAres = true;
		Debug::LogDeferred("[Phobos] Detected Ares 3.0.\n");
		break;
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
