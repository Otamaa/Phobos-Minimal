#include "Body.h"

DEFINE_HOOK(0x464A21, BuildingTypeClass_ReadFromINI_Dock, 0x5)
{
	GET(BuildingTypeClass*, pThis, EBP);
	GET(int, nIDX, ESI);

	if (pThis->Helipad)
	{
		char key[0x40] = { '\0' };
		sprintf_s(key, "DockingPoseDir%d", nIDX);
		BuildingTypeExt::ExtMap.Find(pThis)->DockPoseDir.emplace_back(CCINIClass::INI_Art->ReadInteger(pThis->ImageFile, key, 0));
	}

	return 0x0;
}

static DirStruct GetPoseDir(BuildingClass* pBld, AircraftClass* pAir, int nDefault)
{
	int nDIr = nDefault;

	if (pBld)
	{
		const int nIdx = pBld->FindLinkIndex(pAir);
		const auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
		if (!pExt->DockPoseDir.empty() && nIdx != -1)
			nDIr = abs(pExt->DockPoseDir.at(nIdx));
	}

	if (nDIr <= 7)
	{
		DirStruct nDirRet { static_cast<DirType>(nDIr << 5) };
		return nDirRet;
	}
	else
	{
		DirStruct nDirRet { static_cast<int>(nDIr) * 255 };
		return nDirRet;
	}
}

DEFINE_HOOK(0x446FA2, BuildingClass_GrandOpening_PoseDir, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	GET(AircraftClass*, pAir, ESI);
	pThis->SendCommand(RadioCommand::RequestTether, pAir);
	pAir->SecondaryFacing.Set_Current(GetPoseDir(pThis, pAir, 0));
	return 0x446FB0;
}

DEFINE_HOOK(0x687AF4, CCINIClass_InitializeStuffOnMap_AdjustAircrafts, 0x5)
{
	std::for_each(AircraftClass::Array->begin(), AircraftClass::Array->end(), [](AircraftClass* const pThis)
 {
	 if (pThis && pThis->Type->AirportBound)
	 {
		 if (auto pCell = pThis->GetCell())
		 {
			 if (auto pBuilding = pCell->GetBuilding())
			 {
				 if (pBuilding->Type->Helipad)
				 {
					 pBuilding->SendCommand(RadioCommand::RequestLink, pThis);
					 pBuilding->SendCommand(RadioCommand::RequestTether, pThis);
					 pThis->SetLocation(pBuilding->GetDockCoords(pThis));
					 pThis->DockedTo = pBuilding;
					 pThis->SecondaryFacing.Set_Current(GetPoseDir(pBuilding, pThis, 0));
					 if (pThis->GetHeight() > 0)
						 pThis->Tracker_4134A0();
				 }
			 }
		 }
	 }
	});

	return 0x0;
}

#ifndef checkradio
DEFINE_HOOK(0x444014, BuildingClass_ExitObject_PoseDir_A, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pAir, ECX);

	pThis->SendCommand(RadioCommand::RequestLink, pAir);
	pThis->SendCommand(RadioCommand::RequestTether, pAir);
	pAir->SetLocation(pThis->GetDockCoords(pAir));
	pAir->DockedTo = pThis;
	pAir->SecondaryFacing.Set_Current(GetPoseDir(pThis, pAir, 0));
	return 0x444053;
}
#else
DEFINE_HOOK(0x444053, BuildingClass_ExitObject_PoseDir_A, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pAir, EBP);
	pAir->SecondaryFacing.Set_Current(GetPoseDir(pThis, pAir));
	return 0x0;
}
#endif

//DEFINE_HOOK(0x4CFCF0, FlyLocomotionClass_InWhichLayer, 0x5)
//{
//	GET_STACK(ILocomotion*, pILoco, 0x4);
//	auto pLoco = static_cast<FlyLocomotionClass*>(pILoco);
//	auto const pLinked = static_cast<AircraftClass*>(pLoco->Owner);
//	auto nHeight = pLinked->GetHeight();
//
//	if (pLinked->Type->AirportBound &&  nHeight > 0 && pLinked->RadioLinks.IsAllocated) {
//		if (auto pCell = pLinked->GetCell()) {
//			if (auto pBld = specific_cast<BuildingClass*>(pLinked->RadioLinks[0])) {
//				if (pCell->GetBuilding() == pBld && pBld->FindLinkIndex(pLinked) >= 0) {
//					nHeight -= abs(pBld->Type->DockingOffsets[pBld->FindLinkIndex(pLinked)].Z);
//				}
//			}
//		}
//	}
//
//	R->EAX(nHeight > 0 ? Layer::Top : Layer::Ground);
//	return 0x4CFD0F;
//}

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
//		if (pAircraft->Type->AirportBound && nCurHeight > 0 && pAircraft->RadioLinks.IsAllocated) {
//
//			auto pCell = pThis->GetCell();
//			auto pBld = specific_cast<BuildingClass*>(pAircraft->RadioLinks[0]);
//
//			if (pBld && pCell && pBld == pCell->GetBuilding()) {
//				const auto nIDx = pBld->FindLinkIndex(pAircraft);
//				if (nIDx >= 0)
//				{
//					nCurHeight -= abs(pBld->Type->DockingOffsets[nIDx].Z);
//				}
//			}
//		}
//
//		Debug::Log("[%s]Plane Height %d \n", pAircraft, nCurHeight);
//	}
//
//	R->EAX(nCurHeight);
//	return 0x5F5F91;
//}
//
//DEFINE_HOOK(0x4CF922, FlyLocomotionClass_DrawPoint_AddOffset, 0x5)
//{
//	GET(ILocomotion*, pThis, EDI);
//	auto pLoco = static_cast<FlyLocomotionClass*>(pThis);
//	GET(int, nY, EBP);
//
//	if (auto pAircraft = specific_cast<AircraftClass*>(pLoco->LinkedTo))
//	{
//		if (pAircraft->Type->AirportBound && pAircraft->RadioLinks.IsAllocated && !pAircraft->GetHeight())
//		{
//
//			auto pCell = pAircraft->GetCell();
//			auto pBld = specific_cast<BuildingClass*>(pAircraft->RadioLinks[0]);
//
//			if (pBld && pCell && pBld == pCell->GetBuilding())
//			{
//				const auto nIDx = pBld->FindLinkIndex(pAircraft);
//				if (nIDx >= 0)
//				{
//					nY += abs(pBld->Type->DockingOffsets[nIDx].Z);
//				}
//			}
//		}
//
//		Debug::Log("[%s]Plane Height %d \n", pAircraft, nY);
//	}
//
//	R->EBP(nY);
//	return 0x0;
//}

// the aircraft doesnt do radio contact to the building when ION Strom Active ?
// what ?
DEFINE_HOOK(0x443FD8, BuildingClass_ExitObject_PoseDir_B, 0x8)
{
	enum { RetCreationFail  = 0x444EDE, RetCreationSucceeded = 0x443FE0 };

	GET(BuildingClass*, pThis, ESI);
	GET(AircraftClass*, pAir, EBP);

	if (R->AL())
	{
		pAir->DockedTo = pThis;
		pAir->SecondaryFacing.Set_Current(GetPoseDir(pThis, pAir, 0));

		if (pAir->GetHeight() > 0)
			AircraftClass::AircraftTracker_4134A0(pAir);

		return RetCreationSucceeded;
	}

	return RetCreationFail;
}

DEFINE_HOOK(0x41B780, IFlyControl_LandDirection_InRadioContact , 0x5)
{
	enum { SetFromCurrent = 0x41B7A7, SetFromValue = 0x41B7BC};

	//GET_STACK(IFlyControl*, pFly, 0x4);
	GET(AircraftClass*, pAircraft, ESI);
	GET(TechnoClass*, pContact, EAX);

	if (const auto pBld = specific_cast<BuildingClass*>(pContact)) {

		const auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
		const auto nIdx = pBld->FindLinkIndex(pAircraft);

		if (!pExt->DockPoseDir.empty() && nIdx != -1) {
			R->EAX(abs(pExt->DockPoseDir.at(nIdx)));
			return SetFromValue; //we return here , similar to Rules->PoseDir 
		}
	}

	//original game code , this can be return 0 , but i prefer doing it this way !
	auto const pCurFacing = pContact->PrimaryFacing.Current();
	R->EAX(&pCurFacing.Raw); // this will be depointer later 
	return SetFromCurrent;
}

//DEFINE_HOOK(0x41B793, IFlyControl_LandDirection_SomeCond, 0x8)
//{
//	GET(bool, bSomething, EAX);
//
//	if (!bSomething) {
//		R->EAX(abs(RulesGlobal->PoseDir));
//		return 0x41B7BC;
//	}
//
//	return 0x41B797;
//}

DEFINE_HOOK(0x41B7B7, IFlyControl_LandDirection_PoseDir, 0x5)
{
	R->EAX(abs(RulesGlobal->PoseDir));
	return 0x41B7BC;
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

//DEFINE_HOOK(0x444971, BuildingClass_ExitObject_AircraftClass_AircraftTracker, 0x5)
//{
//	GET(AircraftClass*, pProduct, EBP);
//
//	if (pProduct && pProduct->Type->AirportBound  && pProduct->GetHeight() > 0)
//		AircraftClass::AircraftTracker_4134A0(pProduct);
//
//	return 0x0;
//}
