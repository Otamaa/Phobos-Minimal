#include "AresData.h"

#include <ASMMacros.h>
#include <Phobos.h>
#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <tlhelp32.h>

#include <Utilities/Constructs.h>
#include <Utilities/Macro.h>

#include <Lib/nameof/nameof.h>

class TechnoClass;
class TechnoTypeClass;
class BuildingLightClass;

uintptr_t AresData::PhobosBaseAddress = 0x0;
uintptr_t AresData::AresBaseAddress = 0x0;
HMODULE AresData::AresDllHmodule = nullptr;
DWORD AresData::AresMemDelAddrFinal = 0x0;
DWORD AresData::AresMemAllocAddrFinal = 0x0;

enum FunctionIndices : int
{
	ConvertTypeToID = 0,
	SpawnSurvivorsID = 1,
	RecalculateStatID = 2,
	ReverseEngineerID = 3,
	GetInfActionOverObjectID = 4,
	SetMouseCursorActionID = 5, //0x058AB0
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

	DisableEMPAffectID = 36,
	CloakDisallowedID = 37,
	CloadAllowedID = 38,
	RemoveAEID = 39,

	FlyingStringsAddID = 40,
	CalculateBountyID = 41,
	SetSpotlightID = 42,

	IsPoweredID = 43,
	IsDriverKillableID = 44,
	KillDriverCoreID = 45,

	FireIronCurtainID = 46,

	RespondToFirewallID = 47,
	RequirementsMetID = 48,

	UpdateAcademyID = 49,
	SetSWCursorID = 50, //0x058AD0
	UpdateDisplayToID = 51,

	GetTurretWeaponIdxID = 52,
	CameoIsEliteID = 53,

	GetActionHijackID = 54,

	NetEvent_RespondToFirewall = 55,

	TechnoTypeExt_GetWeaponTypeID = 56,

	BuildingExt_SetFactoryPlans = 57,

	AttachedAffect_UpdateType = 58,
	count
};

// storage for absolute addresses of functions (module base + offset)
static DWORD AresFunctionOffsetsFinal[FunctionIndices::count];

#pragma region Caller
template<int idx, typename Tret, typename... TArgs>
struct AresStdcall
{
	static_assert(idx < FunctionIndices::count, "Index Out Of Bound !");
	using fp_type = Tret(__stdcall*)(TArgs...);
	decltype(auto) operator()(TArgs... args) const
	{
		return reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(args...);
	}
};

template<int idx, typename... TArgs>
struct AresStdcall<idx, void, TArgs...>
{
	static_assert(idx < FunctionIndices::count, "Index Out Of Bound !");
	using fp_type = void(__stdcall*)(TArgs...);
	decltype(auto) operator()(TArgs... args) const
	{
		reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(args...);
	}
};

template<int idx, typename Tret, typename... TArgs>
struct AresCdecl
{
	static_assert(idx < FunctionIndices::count, "Index Out Of Bound !");
	using fp_type = Tret(__cdecl*)(TArgs...);
	decltype(auto) operator()(TArgs... args) const
	{
		return reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(args...);
	}
};

template<int idx, typename... TArgs>
struct AresCdecl<idx, void, TArgs...>
{
	static_assert(idx < FunctionIndices::count, "Index Out Of Bound !");
	using fp_type = void(__cdecl*)(TArgs...);
	decltype(auto) operator()(TArgs... args) const
	{
		reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(args...);
	}
};

template<int idx, typename Tret, typename... TArgs>
struct AresFastcall
{
	static_assert(idx < FunctionIndices::count, "Index Out Of Bound !");
	using fp_type = Tret(__fastcall*)(TArgs...);
	decltype(auto) operator()(TArgs... args) const
	{
		return reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(args...);
	}
};

template<int idx, typename... TArgs>
struct AresFastcall<idx, void, TArgs...>
{
	static_assert(idx < FunctionIndices::count, "Index Out Of Bound !");
	using fp_type = void(__fastcall*)(TArgs...);
	decltype(auto) operator()(TArgs... args) const
	{
		reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(args...);
	}
};

template<int idx, typename Tret, typename TThis, typename... TArgs>
struct AresThiscall
{
	static_assert(idx < FunctionIndices::count, "Index Out Of Bound !");
	using fp_type = Tret(__fastcall*)(TThis, void*, TArgs...);
	decltype(auto) operator()(TThis pThis, TArgs... args) const
	{
		return reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(pThis, nullptr, args...);
	}
};

template<int idx, typename TThis, typename... TArgs>
struct AresThiscall<idx, void, TThis, TArgs...>
{
	static_assert(idx < FunctionIndices::count, "Index Out Of Bound !");
	using fp_type = void(__fastcall*)(TThis, void*, TArgs...);
	void operator()(TThis pThis, TArgs... args) const
	{
		reinterpret_cast<fp_type>(AresFunctionOffsetsFinal[idx])(pThis, nullptr, args...);
	}
};
#pragma endregion

DWORD AresData::AresCustomPaletteReadFuncFinal[AresData::AresCustomPaletteReadCount];
DWORD AresData::AresStaticInstanceFinal[AresData::AresStaticInstanceCount];

//bool EMPulse::IsDeactivationAdvisable(TechnoClass* Target)
//{
//	switch (Target->CurrentMission)
//	{
//	case Mission::Selling:
//	case Mission::Construction:
//	case Mission::Unload:
//		return false;
//	}
//
//	return true;
//}

bool AresData::Init()
{
	const auto& ares = Patch::ModuleDatas[ARES_DLL_S];
	AresDllHmodule = ares.Handle;
	AresBaseAddress = ares.BaseAddr;
	PhobosBaseAddress = Patch::ModuleDatas[PHOBOS_DLL_S].BaseAddr;

	// find offset of PE header
	const int PEHeaderOffset = *(DWORD*)(AresBaseAddress + 0x3c);
	// find absolute address of PE header
	const DWORD* PEHeaderPtr = (DWORD*)(AresBaseAddress + PEHeaderOffset);

	// read the timedatestamp at 0x8 offset
	switch ((*(PEHeaderPtr + 2)))
	{
	case 0x61daa114:
		Debug::LogDeferred("[Phobos] Detected Ares 3.0p1.\n");
		break;
	default:
		Debug::LogDeferred("[Phobos] Detected Version of Ares that is not supported.\n");
		return false;
	}

	// offsets of function addresses for each AresVersion
	static constexpr DWORD AresFunctionOffsets[FunctionIndices::count] =
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

		0x063550, //EMPulse_DisableEmpaffect
		0x044520, //TechnoExt CloakDisallowed

		0x044470, //TechnoExt CloadAllowed

		0x059580, //TechnoExt::RemoveAE
		0x044F70, //TechnoExt::FlyingStringsAdd
		0x043D10, //TechnoExt::CalculateBounty
		0x046F90, //TechnoExt::SetSpotlight

		0x046060, //IsPowered
		0x043FA0, //IsDriverKillable
		0x046240, //TechnoExt::KillDriverCore

		0x03A970, //TeamExt_FireIC
		0x023010, //HouseExt::RespondToFirewall
		0x022A70, //RequirementsMet
		0x0231E0, //UpdateAcademy

		0x058AD0, //MapSWAction

		0x013760, //BuildingExt::UpdateDosplayTo

		0x003E870, //TechnoTypeExt::GetTurretWeaponIdx
		0x003E210, //TechnoTypeExt::CameoIsElite
		0x0045B60, //TechnoExt::GetActionHijack
		0x0023010, //AresEvent::Handle::RespondToFirewall

		0x003E810, //TechnoTypeExt::GetWeapon
		0x00454B0, //BuildingExt::SetFactoryPlans

		0x0059690, //AttachedAffect_UpdateType
	};

	static constexpr DWORD AAresCustomPaletteReadTable[AresCustomPaletteReadCount] = {
		0x005B59, //Ares UI
		0x00A544, //AnimTypeExt
		0x029CF1, //ParticleTypeExt
		0x03456B, //SWTypeExt
		0x03F0AB, //TechnoTypeExt
	};

	static constexpr DWORD AresStaticInstanceTable[AresStaticInstanceCount] = {
		0x0C2B84, //ParticleSystemExt
		0x0C2DD0, //WeaponTypeExt
		0x0C2C50, //SWTypeExt

		0x0C2A00, //Debug::bTrackParserErrors

		0x0C2A54, //HouseExt::FSW

		0x0C3100, //std::vector<const char*>

		0x09C934, //Default SWAiTargetingData

		0x0C1134, //SW_Firewall::FirewallType

		0x0C2C7C, //SWTypeExt::CurrentSWType

		0x0C2E2C, //EboltColors1
		0x0C2E30, //EboltColors2
		0x0C2E34, //EboltColors3

		0x0C2E14, //Ebolt - Ext map

		0x0C1C64, //Ares_Shuttingdown
	};

	for (int a = 0; a < FunctionIndices::count; a++)
		AresFunctionOffsetsFinal[a] = AresData::AresBaseAddress + AresFunctionOffsets[a];

	for (int b = 0; b < AresData::AresStaticInstanceCount; b++)
		AresData::AresStaticInstanceFinal[b] = AresData::AresBaseAddress + AresStaticInstanceTable[b];

	for (int c = 0; c < AresData::AresCustomPaletteReadCount; c++)
		AresData::AresCustomPaletteReadFuncFinal[c] = AresData::AresBaseAddress + AAresCustomPaletteReadTable[c];

	AresData::AresMemDelAddrFinal = AresData::AresBaseAddress + 0x0077FCA;
	AresData::AresMemAllocAddrFinal = AresData::AresBaseAddress + 0x077F9A;

	return true;
}

void AresData::UnInit()
{
	if (!AresBaseAddress)
		return;

	//if (CanUseAres)
	//	FreeLibrary(AresDllHmodule);
}

//bool AresData::ConvertTypeTo(TechnoClass* pFoot, TechnoTypeClass* pConvertTo)
//{
//	return AresStdcall<ConvertTypeToID, bool, TechnoClass*, TechnoTypeClass*>()(pFoot, pConvertTo);
//}

//void AresData::SpawnSurvivors(FootClass* const pThis, TechnoClass* const pKiller, const bool ISelect, const bool IgnoreDefenses)
//{
//	AresStdcall<SpawnSurvivorsID, void, FootClass*, TechnoClass*, bool, bool>()(pThis, pKiller, ISelect, IgnoreDefenses);
//}

//void AresData::RecalculateStat(TechnoClass* const pTechno)
//{
//	AresThiscall<RecalculateStatID, void,  void*>()((void*)pTechno->align_154);
//}

//bool AresData::ReverseEngineer(BuildingClass* const pBld, TechnoTypeClass* const pTechnoType)
//{
//	return AresStdcall<ReverseEngineerID, bool, BuildingClass* , TechnoTypeClass*>()(pBld , pTechnoType);
//}
//
//Action AresData::GetInfActionOverObject(InfantryClass* const pInf, BuildingClass* const pBld)
//{
//	return AresStdcall<GetInfActionOverObjectID, Action, InfantryClass*  , BuildingClass*>()(pInf, pBld);
//}

void AresData::SetMouseCursorAction(size_t CursorIdx, Action nAction, bool bShrouded)
{
	AresStdcall<SetMouseCursorActionID, void, size_t, Action, bool>()(CursorIdx , nAction , bShrouded);
}

//CanBuildResult AresData::PrereqValidate(HouseClass* const pHouse, TechnoTypeClass* const pItem, bool const buildLimitOnly, bool const includeQueued)
//{
//	return AresStdcall<HouseCanBuildID, CanBuildResult, HouseClass* , TechnoTypeClass*, bool , bool>()(pHouse, pItem , buildLimitOnly , includeQueued);
//}

//bool AresData::SW_Activate(SuperClass* pSuper, CellStruct cell, bool isPlayer)
//{
//	return AresStdcall<SWActivateID, bool, SuperClass*, CellStruct, bool>()(pSuper, cell, isPlayer);
//}

//void AresData::TechnoExt_ExtData_DepositTiberium(TechnoClass* const pTechno, float const amount, float const bonus, int const idxType)
//{
//	AresThiscall<DepositTiberiumID, void, void* , float , float , int>()((void*)pTechno->align_154,amount , bonus , idxType);
//}
//
//void AresData::HouseExt_ExtData_ApplyAcademy(HouseClass* const pThis, TechnoClass* const pTarget, AbstractType Type)
//{
//	AresThiscall<ApplyAcademyID, void, void*, TechnoClass*, AbstractType>()(GetAresHouseExt(pThis), pTarget, Type);
//}

//VoxelStruct* AresData::GetTurretsVoxel(TechnoTypeClass* const pThis, int index)
//{
//	return AresThiscall<GetTurretVXLDataID, VoxelStruct*, void*, int>()(GetAresTechnoTypeExt(pThis), index);
//}

//VoxelStruct* AresData::GetBarrelsVoxel(TechnoTypeClass* const pThis, int index)
//{
//	return AresThiscall<GetBarrelsVoxelID, VoxelStruct*, void*, int>()(GetAresTechnoTypeExt(pThis), index);
//}

void AresData::TechnoTransferAffects(TechnoClass* const pFrom, TechnoClass* const pTo)
{
	AresStdcall<TechnoTransferAffectsID, void, TechnoClass*, TechnoClass*>()(pFrom, pTo);
}

//bool AresData::IsGenericPrerequisite(TechnoTypeClass* const pThis)
//{
//	return AresThiscall<IsGenericPrerequisiteID, bool, void*>()(GetAresTechnoTypeExt(pThis));
//}
//
//int AresData::GetSelfHealAmount(TechnoClass* const pTechno)
//{
//	return AresThiscall<GetSelfHealAmountID, int, void*>()((void*)pTechno->align_154);
//}

//bool AresData::IsOperated(TechnoClass* const pTechno)
//{
//	return AresThiscall<IsOperatedID, bool, void*>()(GetAresTechnoExt(pTechno));
//}

//ConvertClass* AresData::GetBulletTypeConvert(BulletTypeClass* pThis)
//{
//	return AresThiscall<BulletTypeExtGetConvertID, ConvertClass*, void*>()(GetAresBulletTypeExt(pThis));
//}

//void AresData::WarheadTypeExt_ExtData_ApplyKillDriver(WarheadTypeClass* pThis, TechnoClass* const pAttacker, TechnoClass* const pVictim)
//{
//	AresThiscall<ApplyKillDriverID, ConvertClass*, void* , TechnoClass* , TechnoClass*>()(GetAresAresWarheadTypeExt(pThis) , pAttacker , pVictim);
//}

void AresData::MouseCursorTypeLoadDefault()
{
	AresStdcall<MouseCursorTypeLoadDefaultID, void>()();
}

//AresFactoryStateRet* AresData::HouseExt_HasFactory(AresFactoryStateRet* nBuff, HouseClass const* const Owner, TechnoTypeClass const* const pType, bool bSkipAircraft, bool bRequirePower, bool bCheckCanBuild, bool a7)
//{
//	return AresStdcall<HouseExtHasFactoryID, AresFactoryStateRet*  , AresFactoryStateRet* , HouseClass const*, TechnoTypeClass const*, bool, bool , bool , bool>()(nBuff, Owner, pType , bSkipAircraft , bRequirePower , bCheckCanBuild , a7);
//}
//
//int AresData::HouseExt_GetBuildLimitRemaining(HouseClass const* const Owner, TechnoTypeClass const* const pType)
//{
//	return AresStdcall<HouseExtGetBuildLimitRemainingID, int , HouseClass const*, TechnoTypeClass const*>()(Owner, pType);
//}

//int AresData::CallAresBuildingClass_Infiltrate(REGISTERS* R)
//{
//	if (AresDllHmodule != NULL) {
//		CallHook Data = (CallHook)GetProcAddress(AresDllHmodule, "BuildingClass_Infiltrate");
//		if (Data != NULL) {
//			return (Data)(R);
//		}
//	}
//
//	return -1;
//}

//int NOINLINE AresData::CallAresArmorType_FindIndex(REGISTERS* R)
//{
//	if (AresDllHmodule != NULL)
//	{
//		CallHook Data = (CallHook)GetProcAddress(AresDllHmodule, "ArmorType_FindIndex");
//		if (Data != NULL)
//		{
//			return (Data)(R);
//		}
//	}
//
//	return-1;
//}

//std::vector<FootClass*>* AresData::GetTunnelArray(BuildingTypeClass* const pBld, HouseClass* const pOwner)
//{
//	return AresThiscall<GetTunnelArrayID, std::vector<FootClass*>*, void* , HouseClass* const>()(GetAresBuildingTypeExt(pBld) , pOwner);
//}

void AresData::UpdateAEData(AresTechnoExt::AEData* const pAE)
{
	AresThiscall<UpdateAEDataID, void, AresTechnoExt::AEData* const>()(pAE);
}

void AresData::RemoveAEType(AresTechnoExt::AEData* const pAE, TechnoTypeClass* pNewType)
{
	AresThiscall<AttachedAffect_UpdateType, void, AresTechnoExt::AEData* const , TechnoTypeClass*>()(pAE , pNewType);
}

//void AresData::JammerClassUnjamAll(AresTechnoExt::JammerClass* const pJamm)
//{
//	AresThiscall<JammerclassUnjamAllID, void, AresTechnoExt::JammerClass* const>()(pJamm);
//}

//void AresData::CPrismRemoveFromNetwork(cPrismForwarding* const pThis, bool bCease)
//{
//	AresThiscall<CPrismRemoveFromNetworkID, void, cPrismForwarding* const , bool>()(pThis , bCease);
//}

//void  AresData::applyIonCannon(WarheadTypeClass* pWH, CoordStruct* pTarget){
//	AresThiscall<applyIonCannonID , void, void* , CoordStruct*>()(GetAresAresWarheadTypeExt(pWH) , pTarget);
//}
//
//bool AresData::applyPermaMC(WarheadTypeClass* pWH, HouseClass* pOwner, AbstractClass* pTarget) {
//	return AresThiscall<applyPermaMCID, bool, void*, HouseClass*, AbstractClass*>()(GetAresAresWarheadTypeExt(pWH), pOwner , pTarget);
//}
//
//void AresData::applyIC(WarheadTypeClass* pWH, CoordStruct* pTarget, HouseClass* pOwner, int Damage) {
//	AresThiscall<applyICID, void, void*, CoordStruct* , HouseClass*, int>()(GetAresAresWarheadTypeExt(pWH), pTarget , pOwner, Damage);
//}

//void AresData::applyEMP(WarheadTypeClass* pWH, CoordStruct* pTarget, TechnoClass* pOwner) {
//	AresThiscall<applyEMPID, void, void*, CoordStruct* , TechnoClass*>()(GetAresAresWarheadTypeExt(pWH), pTarget , pOwner);
//}

void AresData::applyAE(WarheadTypeClass* pWH, CoordStruct* pTarget, HouseClass* pOwner) {
	AresThiscall<applyAEID, void, void*, CoordStruct* , HouseClass*>()(GetAresAresWarheadTypeExt(pWH), pTarget , pOwner);
}

// void AresData::EvalRaidStatus(BuildingClass* pBuilding)
// {
// 	AresThiscall<EvalRaidStatusID, void, void*>()(GetAresBuildingExt(pBuilding));
// }

//bool AresData::IsActiveFirestormWall(BuildingClass* pBuilding, HouseClass* pOwner)
//{
//	return AresStdcall<IsActiveFirestormWallID, bool, BuildingClass* , HouseClass* >()(pBuilding , pOwner);
//}

//bool AresData::ImmolateVictim(BuildingClass* pBuilding, FootClass* pTarget, bool Destroy)
//{
//	return AresThiscall<ImmolateVictimID, bool, void*, FootClass*, bool>()(GetAresBuildingExt(pBuilding), pTarget , Destroy);
//}

//void AresData::DisableEMPEffect(TechnoClass* pTechno)
//{
//	AresStdcall<DisableEMPAffectID, void, TechnoClass*>()(pTechno);
//}

//bool AresData::CloakDisallowed(TechnoClass* pTechno, bool allowPassive)
//{
//	return AresThiscall<CloakDisallowedID, bool, void*, bool>()((void*)pTechno->align_154, allowPassive);
//}

//bool AresData::CloakAllowed(TechnoClass* pTechno)
//{
//	return AresThiscall<CloadAllowedID, bool, void*>()((void*)pTechno->align_154);
//}

bool AresData::RemoveAE(AresTechnoExt::AEData* pAE)
{
	return AresThiscall<RemoveAEID, bool , AresTechnoExt::AEData*>()(pAE);
}

void AresData::FlyingStringsAdd(TechnoClass* pTech, bool bSomething)
{
	AresThiscall<FlyingStringsAddID, void, void*, bool>()((void*)pTech->align_154, bSomething);
}

//void AresData::CalculateBounty(TechnoClass* pThis, TechnoClass* pKiller)
//{
//	AresThiscall <CalculateBountyID, void, void*, TechnoClass*>()((void*)pThis->align_154, pKiller);
//}

// void AresData::SetSpotlight(TechnoClass* pThis, BuildingLightClass* pSpotlight)
// {
// 	AresThiscall<SetSpotlightID, void, void*, BuildingLightClass*>()((void*)pThis->align_154, pSpotlight);
// }

//bool AresData::IsPowered(TechnoClass* pThis)
//{
//	return AresThiscall<IsPoweredID, bool, void*>()(GetAresTechnoExt(pThis));
//}

//bool AresData::IsDriverKillable(TechnoClass* pThis, double tresh)
//{
//	return AresThiscall<IsDriverKillableID, bool, void* , double>()((void*)pThis->align_154, tresh);
//}

//bool AresData::KillDriverCore(TechnoClass* pThis, HouseClass* pToHouse, TechnoClass* pKiller, bool removeVet)
//{
//	return AresThiscall<KillDriverCoreID, bool, void* ,HouseClass*, TechnoClass* , bool>()((void*)pThis->align_154, pToHouse , pKiller , removeVet);
//}

//void AresData::FireIronCurtain(TeamClass* pTeam, ScriptActionNode* pNode, bool ntrhd)
//{
//	AresStdcall<KillDriverCoreID, void, TeamClass*, ScriptActionNode* , bool>()(pTeam , pNode , ntrhd);
//}

//void AresData::RespondToFirewall(HouseClass* pHouse, bool Active)
//{
//	AresThiscall<RespondToFirewallID, void, void*, bool>()(GetAresHouseExt(pHouse), Active);
//}

//RequirementStatus AresData::RequirementsMet(HouseClass* pHouse, TechnoTypeClass* pTech)
//{
//	return AresStdcall<RequirementsMetID, RequirementStatus, HouseClass*, TechnoTypeClass*>()(pHouse, pTech);
//}
//
//void AresData::UpdateAcademy(HouseClass* pThis, TechnoClass* pTechno, bool bAdded)
//{
//	AresThiscall<UpdateAcademyID, void, void*  , TechnoClass*, bool>()(GetAresHouseExt(pThis), pTechno , bAdded);
//}
//
//void* AresData::Ares_SWType_ExtMap_Find(SuperWeaponTypeClass* pType)
//{
//	return AresThiscall<ExtMapFindID, void*, DWORD , SuperWeaponTypeClass*>()(AresStaticInstanceFinal[2] ,pType);
//}

void AresData::SetSWMouseCursorAction(size_t CursorIdx,  bool bShrouded , int nAction)
{
	AresStdcall<SetSWCursorID, void, size_t , bool , int>()(CursorIdx, bShrouded, nAction);
}

//void AresData::BuildingExt_UpdateDisplayTo(BuildingClass* pFor)
//{
//	AresStdcall<UpdateDisplayToID, void, BuildingClass*>()(pFor);
//}

//int* AresData::TechnoTypeExt_GetTurretWeaponIdx(TechnoTypeClass* pThis, int idx)
//{
//	return AresThiscall<GetTurretWeaponIdxID, int*, void*, int>()(GetAresTechnoTypeExt(pThis) , idx);
//}

//bool AresData::TechnoTypeExt_CameoIsElite(TechnoTypeClass* pThis, HouseClass* Owner)
//{
//	return AresThiscall<CameoIsEliteID, bool, void*, HouseClass*>()(GetAresTechnoTypeExt(pThis), Owner);
//}

//Action AresData::TechnoExt_GetActionHijack(TechnoClass* pThis, TechnoClass* pTarget)
//{
//	return AresThiscall<GetActionHijackID, Action, void*, TechnoClass*>()((void*)pThis->align_154, pTarget);
//}
//

//void AresData::AresNetEvent_Handlers_RespondToFirewallToggle(HouseClass* pFor, bool Activate)
//{
//	AresThiscall<NetEvent_RespondToFirewall, void, void*, bool>()(GetAresHouseExt(pFor), Activate);
//}

//WeaponStruct* AresData::GetWeapon(TechnoTypeClass* pType, int idx, bool Elite) {
//	return AresThiscall<TechnoTypeExt_GetWeaponTypeID, WeaponStruct*, void*, int , bool>()(GetAresTechnoTypeExt(pType), idx, Elite);
//}

//void AresData::SetFactoryPlans(BuildingClass* pBld) {
//	AresStdcall< BuildingExt_SetFactoryPlans, void, BuildingClass*>()(pBld);
//}