#include "AresData.h"

#include <ASMMacros.h>
#include <Phobos.h>
#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <tlhelp32.h>

#include <Utilities/Constructs.h>
#include <Utilities/Macro.h>

class TechnoClass;
class TechnoTypeClass;
class BuildingLightClass;

uintptr_t AresData::PhobosBaseAddress = 0x0;
uintptr_t AresData::AresBaseAddress = 0x0;
HMODULE AresData::AresDllHmodule = nullptr;
DWORD AresData::AresMemDelAddrFinal = 0x0;
DWORD AresData::AresMemAllocAddrFinal = 0x0;
AresData::Version AresData::AresVersionId = AresData::Version::Unknown;
bool AresData::CanUseAres = false;
DWORD AresData::AresFunctionOffsetsFinal[AresData::AresFunctionCount];
DWORD AresData::AresCustomPaletteReadFuncFinal[AresData::AresCustomPaletteReadCount];
DWORD AresData::AresStaticInstanceFinal[AresData::AresStaticInstanceCount];

bool EMPulse::IsDeactivationAdvisable(TechnoClass* Target)
{
	switch (Target->CurrentMission)
	{
	case Mission::Selling:
	case Mission::Construction:
	case Mission::Unload:
		return false;
	}

	return true;
}

uintptr_t AresData::GetModuleBaseAddress(const char* modName)
{
	return Patch::GetModuleBaseAddress(modName);
}

bool AresData::Init()
{
	AresBaseAddress = GetModuleBaseAddress(ARES_DLL_S);
	PhobosBaseAddress = GetModuleBaseAddress(PHOBOS_DLL_S);
	Debug::LogDeferred("[Phobos] Phobos base address: 0x%X.\n", PhobosBaseAddress);

	if (!AresBaseAddress)
	{
		Debug::LogDeferred("[Phobos] Failed to detect Ares. Disabling integration.\n");
		return false;
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
	switch ((*(PEHeaderPtr + 2)))
	{
	case AresTimestampBytes[Version::Ares30p]:
		AresVersionId = Version::Ares30p;
		CanUseAres = true;
		Debug::LogDeferred("[Phobos] Detected Ares 3.0p1.\n");
		break;
	default:
		Debug::LogDeferred("[Phobos] Detected a version of Ares that is not supported by this version of Phobos.\n");
		break;
	}

	if (CanUseAres && GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN, ARES_DLL, &AresDllHmodule))
	{
		for (int a = 0; a < AresData::AresFunctionCount; a++)
			AresData::AresFunctionOffsetsFinal[a] = AresData::AresBaseAddress + AresData::AresFunctionOffsets[a * AresData::AresVersionCount + AresVersionId];

		for (int b = 0; b < AresData::AresStaticInstanceCount; b++)
			AresData::AresStaticInstanceFinal[b] = AresData::AresBaseAddress + AresData::AresStaticInstanceTable[b * AresData::AresVersionCount + AresVersionId];

		for (int c = 0; c < AresData::AresCustomPaletteReadCount; c++)
			AresData::AresCustomPaletteReadFuncFinal[c] = AresData::AresBaseAddress + AresData::AAresCustomPaletteReadTable[c * AresData::AresVersionCount + AresVersionId];

		AresData::AresMemDelAddrFinal = AresData::AresBaseAddress + 0x0077FCA;
		AresData::AresMemAllocAddrFinal = AresData::AresBaseAddress + 0x077F9A;
	}

	return CanUseAres;
}

void AresData::UnInit()
{
	if (!AresBaseAddress)
		return;

	//if (CanUseAres)
	//	FreeLibrary(AresDllHmodule);
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

VoxelStruct* AresData::GetBarrelsVoxel(TechnoTypeClass* const pThis, int index)
{
	return AresThiscall<GetBarrelsVoxelID, VoxelStruct*, void*, int>()(GetAresTechnoTypeExt(pThis), index);
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

bool AresData::IsOperated(TechnoClass* const pTechno)
{
	return AresThiscall<IsOperatedID, bool, void*>()(GetAresTechnoExt(pTechno));
}

ConvertClass* AresData::GetBulletTypeConvert(BulletTypeClass* pThis)
{
	return AresThiscall<BulletTypeExtGetConvertID, ConvertClass*, void*>()(GetAresBulletTypeExt(pThis));
}

void AresData::WarheadTypeExt_ExtData_ApplyKillDriver(WarheadTypeClass* pThis, TechnoClass* const pAttacker, TechnoClass* const pVictim)
{
	AresThiscall<ApplyKillDriverID, ConvertClass*, void* , TechnoClass* , TechnoClass*>()(GetAresAresWarheadTypeExt(pThis) , pAttacker , pVictim);
}

void AresData::MouseCursorTypeLoadDefault()
{
	AresStdcall<MouseCursorTypeLoadDefaultID, void>()();
}

AresFactoryStateRet* AresData::HouseExt_HasFactory(AresFactoryStateRet* nBuff, HouseClass const* const Owner, TechnoTypeClass const* const pType, bool bSkipAircraft, bool bRequirePower, bool bCheckCanBuild, bool a7)
{
	return AresStdcall<HouseExtHasFactoryID, AresFactoryStateRet*  , AresFactoryStateRet* , HouseClass const*, TechnoTypeClass const*, bool, bool , bool , bool>()(nBuff, Owner, pType , bSkipAircraft , bRequirePower , bCheckCanBuild , a7);
}

int AresData::HouseExt_GetBuildLimitRemaining(HouseClass const* const Owner, TechnoTypeClass const* const pType)
{
	return AresStdcall<HouseExtGetBuildLimitRemainingID, int , HouseClass const*, TechnoTypeClass const*>()(Owner, pType);
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

std::vector<FootClass*>* AresData::GetTunnelArray(BuildingTypeClass* const pBld, HouseClass* const pOwner)
{
	return AresThiscall<GetTunnelArrayID, std::vector<FootClass*>*, void* , HouseClass* const>()(GetAresBuildingTypeExt(pBld) , pOwner);
}

void AresData::UpdateAEData(AEData* const pAE)
{
	AresThiscall<UpdateAEDataID, void, AEData* const>()(pAE);
}

void AresData::JammerClassUnjamAll(JammerClass* const pJamm)
{
	AresThiscall<JammerclassUnjamAllID, void, JammerClass* const>()(pJamm);
}

void AresData::CPrismRemoveFromNetwork(cPrismForwarding* const pThis, bool bCease)
{
	AresThiscall<CPrismRemoveFromNetworkID, void, cPrismForwarding* const , bool>()(pThis , bCease);
}

void  AresData::applyIonCannon(WarheadTypeClass* pWH, CoordStruct* pTarget){
	AresThiscall<applyIonCannonID , void, void* , CoordStruct*>()(GetAresAresWarheadTypeExt(pWH) , pTarget);
}

bool AresData::applyPermaMC(WarheadTypeClass* pWH, HouseClass* pOwner, AbstractClass* pTarget) {
	return AresThiscall<applyPermaMCID, bool, void*, HouseClass*, AbstractClass*>()(GetAresAresWarheadTypeExt(pWH), pOwner , pTarget);
}

void AresData::applyIC(WarheadTypeClass* pWH, CoordStruct* pTarget, HouseClass* pOwner, int Damage) {
	AresThiscall<applyICID, void, void*, CoordStruct* , HouseClass*, int>()(GetAresAresWarheadTypeExt(pWH), pTarget , pOwner, Damage);
}

void AresData::applyEMP(WarheadTypeClass* pWH, CoordStruct* pTarget, TechnoClass* pOwner) {
	AresThiscall<applyEMPID, void, void*, CoordStruct* , TechnoClass*>()(GetAresAresWarheadTypeExt(pWH), pTarget , pOwner);
}

void AresData::applyAE(WarheadTypeClass* pWH, CoordStruct* pTarget, HouseClass* pOwner) {
	AresThiscall<applyAEID, void, void*, CoordStruct* , HouseClass*>()(GetAresAresWarheadTypeExt(pWH), pTarget , pOwner);
}

void AresData::EvalRaidStatus(BuildingClass* pBuilding)
{
	AresThiscall<EvalRaidStatusID, void, void*>()(GetAresBuildingExt(pBuilding));
}

bool AresData::IsActiveFirestormWall(BuildingClass* pBuilding, HouseClass* pOwner)
{
	return AresThiscall<IsActiveFirestormWallID, bool, BuildingClass* , HouseClass* >()(pBuilding , pOwner);
}

bool AresData::ImmolateVictim(BuildingClass* pBuilding, FootClass* pTarget, bool Destroy)
{
	return AresThiscall<ImmolateVictimID, bool, void*, FootClass*, bool>()(GetAresBuildingExt(pBuilding), pTarget , Destroy);
}

void AresData::DisableEMPEffect(TechnoClass* pTechno)
{
	AresStdcall<DisableEMPAffectID, void, TechnoClass*>()(pTechno);
}

bool AresData::CloakDisallowed(TechnoClass* pTechno, bool allowPassive)
{
	return AresThiscall<CloakDisallowedID, bool, void*, bool>()(GetAresTechnoExt(pTechno), allowPassive);
}

bool AresData::CloakAllowed(TechnoClass* pTechno)
{
	return AresThiscall<CloadAllowedID, bool, void*>()(GetAresTechnoExt(pTechno));
}

bool AresData::RemoveAE(AEData* pAE)
{
	return AresThiscall<RemoveAEID, bool , AEData*>()(pAE);
}

void AresData::FlyingStringsAdd(TechnoClass* pTech, bool bSomething)
{
	AresThiscall<FlyingStringsAddID, void, void*, bool>()(GetAresTechnoExt(pTech), bSomething);
}

void AresData::CalculateBounty(TechnoClass* pThis, TechnoClass* pKiller)
{
	AresThiscall <CalculateBountyID, void, void*, TechnoClass*>()(GetAresTechnoExt(pThis), pKiller);
}

void AresData::SetSpotlight(TechnoClass* pThis, BuildingLightClass* pSpotlight)
{
	AresThiscall<SetSpotlightID, void, void*, BuildingLightClass*>()(GetAresTechnoExt(pThis), pSpotlight);
}

bool AresData::IsPowered(TechnoClass* pThis)
{
	return AresThiscall<IsPoweredID, bool, void*>()(GetAresTechnoExt(pThis));
}

bool AresData::IsDriverKillable(TechnoClass* pThis, double tresh)
{
	return AresThiscall<IsDriverKillableID, bool, void* , double>()(GetAresTechnoExt(pThis), tresh);
}

bool AresData::KillDriverCore(TechnoClass* pThis, HouseClass* pToHouse, TechnoClass* pKiller, bool removeVet)
{
	return AresThiscall<KillDriverCoreID, bool, void* ,HouseClass*, TechnoClass* , bool>()(GetAresTechnoExt(pThis), pToHouse , pKiller , removeVet);
}

void AresData::FireIronCurtain(TeamClass* pTeam, ScriptActionNode* pNode, bool ntrhd)
{
	AresStdcall<KillDriverCoreID, void, TeamClass*, ScriptActionNode* , bool>()(pTeam , pNode , ntrhd);
}

void AresData::RespondToFirewall(HouseClass* pHouse, bool Active)
{
	AresThiscall<RespondToFirewallID, void, void*, bool>()(GetAresHouseExt(pHouse), Active);
}
