#include "Body.h"

DEFINE_HOOK(0x464A21, BuildingTypeClass_ReadFromINI_Dock, 0x5)
{
	GET(BuildingTypeClass*, pThis, EBP);
	GET(int, nIDX, ESI);

	if (!pThis->Helipad)
		return 0x0;

	if (auto pExt = BuildingTypeExt::ExtMap.Find(pThis))
	{
		std::string nData = "DockingPoseDir";
		nData += std::to_string(nIDX);
		pExt->DockPoseDir[nIDX] = CCINIClass::INI_Art->ReadInteger(pThis->ImageFile, nData.c_str(), 0);
	}


	return 0x0;
}

static DirStruct GetPoseDir(BuildingClass* pBld, AircraftClass* pAir)
{
	const int nIdx = pBld->FindLinkIndex(pAir);
	const auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
	const int nDIr = pExt->DockPoseDir.get_or_default(nIdx, RulesGlobal->PoseDir);

	DirType nRes = DirType::North;
	switch (nDIr)
	{
	case 0:
		nRes = DirType::North; break;
	case 1:
		nRes = DirType::NorthEast; break;
	case 2:
		nRes = DirType::East; break;
	case 3:
		nRes = DirType::SouthEast; break;
	case 4:
		nRes = DirType::South; break;
	case 5:
		nRes = DirType::SouthWest; break;
	case 6:
		nRes = DirType::West; break;
	case 7:
		nRes = DirType::NorthWest; break;
	default:
	{
		DirStruct nDirRet { static_cast<int>(nDIr) * 255 };
		return nDirRet;
	}
	}

	DirStruct nDirRet { nRes };
	return nDirRet;
}

static DirStruct GetDefaultPoseDir()
{
	DirType nRes = DirType::North;
	switch (RulesGlobal->PoseDir)
	{
	case 0:
		nRes = DirType::North; break;
	case 1:
		nRes = DirType::NorthEast; break;
	case 2:
		nRes = DirType::East; break;
	case 3:
		nRes = DirType::SouthEast; break;
	case 4:
		nRes = DirType::South; break;
	case 5:
		nRes = DirType::SouthWest; break;
	case 6:
		nRes = DirType::West; break;
	case 7:
		nRes = DirType::NorthWest; break;
	default:
	{
		DirStruct nDirRet { static_cast<int>(RulesGlobal->PoseDir) * 255 };
		return nDirRet;
	}
	}

	DirStruct nDirRet { nRes };
	return nDirRet;
}

DEFINE_HOOK(0x446FA2, BuildingClass_GrandOpening_PoseDir, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	GET(AircraftClass*, pAir, ESI);
	pThis->SendCommand(RadioCommand::RequestTether, pAir);
	pAir->SecondaryFacing.Set_Current(GetPoseDir(pThis, pAir));
	return 0x446FB0;
}

DEFINE_HOOK(0x44404D, BuildingClass_ExitObject_PoseDir_A, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pAir, EBP);
	pAir->SecondaryFacing.Set_Current(GetPoseDir(pThis, pAir));
	return 0x0;
}

// the aircraft doesnt do radio contact to the building when ION Strom Active ?
// what ?
DEFINE_HOOK(0x443FD8, BuildingClass_ExitObject_PoseDir_B, 0x8)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pAir, EBP);

	if (R->AL())
	{
		pAir->DockedTo = pThis;
		pAir->SecondaryFacing.Set_Current(GetPoseDir(pThis, pAir));
		return 0x443FE0;
	}

	return 0x444EDE;
}

DEFINE_HOOK(0x41B760, IFlyControl_LandDirection, 0x6)
{
	//GET(IFlyControl*, pFly, EDI);
	//GET(AircraftClass*, pThis, ESI);
	GET_STACK(IFlyControl*, pFly, 0x4);
	auto const pThis = static_cast<AircraftClass*>(pFly);


	int nDir = RulesGlobal->PoseDir;
	//we can get building from GetRadioContact , but i prefer this way
	if (auto pBld = specific_cast<BuildingClass*>(pThis->DockedTo))
	{
		const auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
		nDir = (pExt->DockPoseDir.get_or_default(pBld->FindLinkIndex(pThis), RulesGlobal->PoseDir));

	}
	else if (pThis->HasAnyLink())
	{
		auto const pLink = pThis->GetRadioContact();
		auto const nRAW = static_cast<short>(pLink->PrimaryFacing.Current().Raw);

		//if (pLink->WhatAmI() == AbstractType::Building)
		//{
		//	auto pBld = static_cast<BuildingClass*>(pLink);
		//	const auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
		//	nDir = (pExt->DockPoseDir.get_or_default(pBld->FindLinkIndex(pThis), (((((nRAW >> 12) + 1) >> 1) & 7))));
		//}
		//else {
		nDir = (((((nRAW >> 12) + 1) >> 1) & 7));
		//}

	}

	// something override the facing ? , not sure
	//return 0x41B78D;

	R->EAX(nDir);
	return 0x41B7B4;
}


//DEFINE_HOOK(0x4CFA07, FlyLocomotionClass_LandingFacing, 0x5)
//{
//	GET(ILocomotion*, pILoco, ESI);
//	auto pLoco = static_cast<FlyLocomotionClass*>(pILoco);
//	auto pAir = static_cast<AircraftClass*>(pLoco->LinkedTo);
//
//	if (pAir->DockedTo){
//		auto const nPoseDir = GetPoseDir(static_cast<BuildingClass*>(pAir->DockedTo), pAir);
//		R->EAX(&nPoseDir);
//	}
//	else
//	{
//		auto const nPoseDir = pAir->SecondaryFacing.Desired();
//		R->EAX(&nPoseDir);
//	}
//
//	return 0x4CFA1A;
//}

//DEFINE_HOOK(0x419FCD, AircraftClass_MI_Enter_Facing, 0x6)
//{
//	GET(AircraftClass*, pThis, ESI);
//	GET(int, nX, EAX);
//	GET(int, nY, EDI);
//	GET(int, nZ, EBX);
//
//	R->Stack(0x14, R->EDI());
//
//	if (auto pBld = specific_cast<BuildingClass*>(pThis->DockedTo)) {
//		pThis->SecondaryFacing.Set_Current(GetPoseDir(pBld, pThis));
//	}
//
//	CoordStruct nCell = { nX , nY , nZ };
//	pThis->SetLocation(nCell);
//
//	return 0x419FE4;
//}

//
//DEFINE_HOOK(0x4CE76B, FlyLocomotionClass_PrepareLanding_Facing, 0x5)
//{
//	GET(AircraftClass*, pThis, ESI);
//	REF_STACK(DirStruct, nFacingStorage, 0x1C);
//
//	if (auto pBld = specific_cast<BuildingClass*>(pThis->DockedTo)) {
//		nFacingStorage = GetPoseDir(pBld, pThis);
//	}
//	else {
//		nFacingStorage = pThis->PrimaryFacing.Desired();
//	}
//
//	R->EAX(&nFacingStorage);
//
//	return 0x4CE77E;
//}
//
//DEFINE_HOOK(0x4CF320, FlyLocomotionClass_PrepareLanding_Facing_B, 0x5)
//{
//	GET(AircraftClass*, pThis, ESI);
//
//	DirStruct nDir { DirType::North };
//
//	if (auto pBld = specific_cast<BuildingClass*>(pThis->DockedTo)) {
//		nDir = GetPoseDir(pBld, pThis);
//	}
//
//	R->Stack(0x50, nDir);
//
//	return 0x4CF325;
//}

//void __declspec(naked) _IFlyControl_LandDirection_RET()
//{
//	POP_REG(EDI);
//	POP_REG(ESI);
//	JMP(0x41B7C1);
//}

//DEFINE_HOOK(0x4177E6, IFlyControl_LandingAltitude_CountForBuildingZOffs, 0x8)
//{
//	//GET(IFlyControl*, pFly, EBX);
//	//auto const pThis = static_cast<AircraftClass*>(pFly);
//	GET(AircraftClass*, pThis, ESI);
//	GET(int, nAltitude, EBP);
//
//	int nLandingAltitude = 0;
//	if(pThis->RadioLinks.IsAllocated && pThis->RadioLinks.Capacity > 0){
//		if (auto pBld = specific_cast<BuildingClass*>(pThis->RadioLinks.Items[0])) {
//			auto nIDx = pBld->FindLinkIndex(pThis);
//			if (nIDx != -1) {
//				nLandingAltitude += abs(pBld->Type->DockingOffsets[nIDx].Z);
//			}
//		}
//	}
//
//	nAltitude += nLandingAltitude;
//	//R->EAX(nLandingAltitude);
//	//return 0x41B755;
//
//	return 0x0;
//}

//void __declspec(naked) _IFlyControl_LandingAltitude_CountForBuildingZOffs_RET()
//{
//	POP_REG(EDI);
//	POP_REG(ESI);
//	POP_REG(EBX);
//	JMP(0x41B756);
//}

//DEFINE_HOOK(0x41B751, IFlyControl_LandingAltitude_CountForBuildingZOffs, 0x5)
//{
	//GET(IFlyControl*, pFly, EBX);
	//auto const pThis = static_cast<AircraftClass*>(pFly);

	//int nLandingAltitude = 0;
	//if (auto pBld = specific_cast<BuildingClass*>(pThis->DockedTo)) {
	//	auto nIDx = pBld->FindLinkIndex(pThis);
	//	if (nIDx != -1)
	//	{
	//		nLandingAltitude += abs(pBld->Type->DockingOffsets[nIDx].Z);
	//	}
	//}
//
//	R->EAX(nLandingAltitude);
//	return (int)_IFlyControl_LandingAltitude_CountForBuildingZOffs_RET;
//}

//DEFINE_HOOK(0x419F71, AircraftClass_MI_Enter_DockingCoords, 0x5)
//{
//	GET(AircraftClass*, pThis, ESI);
//	GET(CoordStruct*, pDockingCoord, EAX);
//	auto const nLoc = pThis->Location;
//
//	auto nDistanceX = Math::clamp(pDockingCoord->X - nLoc.X, -5, 5);
//	auto nDistanceY = Math::clamp(pDockingCoord->Y - nLoc.Y, -5, 5);
//	auto nDistanceZ = Math::clamp(pDockingCoord->Z - nLoc.Z, -5, 5);
//
//	CoordStruct nFinalCoord = { pThis->Location.X + nDistanceX , pThis->Location.Y + nDistanceY, pThis->Location.Z + nDistanceZ };
//	pThis->SetLocation(nFinalCoord);
//
//	return 0x419FE4;
//}
//
//DEFINE_HOOK(0x41B70F, IFlyControl_LandingAltitude_CountForBuildingZOffsA, 0xA)
//{
//	GET(BuildingClass*, pBld, EDI);
//	GET(IFlyControl*, pFly, EBX);
//	auto const pThis = static_cast<AircraftClass*>(pFly);
//
//	if (R->AL()) {
//		auto nIDx = pBld->FindLinkIndex(pThis);
//		if (nIDx != -1) {
//			R->EAX(abs(pBld->Type->DockingOffsets[nIDx].Z));
//		}
//
//		return 0x41B751;
//	}
//
//	return 0x41B713;
//}
//
//DEFINE_HOOK(0x41B705, IFlyControl_LandingAltitude_CountForBuildingZOffsB, 0xA)
//{
//	if (R->AL()) {
//		R->EAX(0);
//		return 0x41B751;
//
//	}
//
//	return 0x41B709;
//}
//
//DEFINE_HOOK(0x41B735, IFlyControl_LandingAltitude_CountForBuildingZOffsC, 0x7)
//{
//	if (R->EAX()) {
//		return 0x41B739;
//	}
//
//	R->EAX(0);
//	return 0x41B751;
//}
//
//DEFINE_HOOK(0x41B742, IFlyControl_LandingAltitude_CountForBuildingZOffsD, 0x5)
//{
//	if (R->AL()) {
//		return 0x41B746;
//	}
//
//	R->EAX(0);
//	return 0x41B751;
//
//}
////remove xor eax eax
//DEFINE_JUMP(LJMP, 0x41B753, 0x41B755);

//DEFINE_HOOK(0x5F5F40, ObjectClass_GetHeight_Aircraft, 0x5)
//{
//	GET(ObjectClass*, pThis, ECX);
//
//	int nCurHeight = pThis->Location.Z - Map.GetCellFloorHeight(pThis->Location);
//
//	if (pThis->OnBridge)
//		nCurHeight -= 416;
//
//	if (auto pAircraft = specific_cast<AircraftClass*>(pThis)) {
//		if(pAircraft->Type->AirportBound && pAircraft->DockedTo) {
//			if(auto pCell = pThis->GetCell()) {
//				if(auto pBld = pCell->GetBuilding()) {
//					if(pAircraft->DockedTo == pBld){
//					const auto nIDx = pBld->FindLinkIndex(pAircraft);
//						if (nIDx != -1 && nCurHeight > 0) {
//							nCurHeight -= abs(pBld->Type->DockingOffsets[nIDx].Z);
//						}
//					}
//				}
//			}
//		}
//	}
//
//	R->EAX(nCurHeight);
//	return 0x5F5F91;
//}