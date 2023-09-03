#include "Hooks.OtamaaBugFix.h"
#include <SpecificStructures.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <New/Entity/FlyingStrings.h>

//DEFINE_HOOK(0x730F1C, ObjectClass_StopCommand, 0x5)
//{
//	GET(ObjectClass*, pObject, ESI);
//
//	if (auto pTechno = generic_cast<TechnoClass*>(pObject)) {
//		if (auto pManager = pTechno->SpawnManager) {
//			if (!pManager->SpawnType->MissileSpawn && pManager->Status != SpawnManagerStatus::Idle && pManager->Target)
//				pManager->ResetTarget();
//		}
//	}
//
//	return 0;
//}

#ifdef ENABLE_NEWHOOKS
//TODO : retest for desync
DEFINE_HOOK(0x442A2A, BuildingClass_ReceiveDamage_RotateVsAircraft, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	GET(RulesClass*, pRules, ECX);

	if (pThis && pThis->Type)
	{
		if (auto const pStructureExt = BuildingTypeExt::ExtMap.Find(pThis->Type))
		{
			R->AL(pStructureExt->PlayerReturnFire.Get(pRules->PlayerReturnFire));
			return 0x442A30;
		}
	}

	return 0x0;
}
#endif

//size
//#ifdef ENABLE_NEWHOOKS
//Crash
//DEFINE_HOOK(0x444014, AircraftClass_ExitObject_DisableRadioContact, 0x5)
//{
//	enum { SkipAllSet = 0x444053, Nothing = 0x0 };
//
//	//GET(BuildingClass*, pBuilding, ESI);
//	GET(AircraftClass*, pProduct, EBP);
//
//	if (pProduct) {
//		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pProduct->Type);
//		if (pTypeExt && !pProduct->Type->AirportBound && pTypeExt->NoAirportBound_DisableRadioContact.Get()) {
//			return SkipAllSet;
//		}
//	}
//
//	return Nothing;
//}

//#endif
//
//DEFINE_HOOK(0x415302, AircraftClass_MissionUnload_IsDropship, 0x8)
//{
//	GET(AircraftClass*, pThis, ESI);
//
//	if (pThis->Destination) {
//		if (pThis->Type->IsDropship) {
//			CellStruct nCell = CellStruct::Empty;
//			if (pThis->SelectNavalTargeting(pThis->Destination) != 11) {
//				if (auto pTech = pThis->Destination->AsTechno()) {
//					auto nCoord = pTech->GetCoords();
//					nCell = CellClass::Coord2Cell(nCoord);
//
//					if (nCell != CellStruct::Empty) {
//						if (auto pCell = Map[nCell]) {
//							for (auto pOccupy = pCell->FirstObject;
//								pOccupy;
//								pOccupy = pOccupy->NextObject) {
//								if (pOccupy->WhatAmI() == AbstractType::Building) {
//									auto pGoodLZ = pThis->GoodLandingZone();
//									pThis->SetDestination(pGoodLZ, true);
//								}
//								else {
//									nCoord = pThis->GetCoords();
//									pOccupy->Scatter(nCoord, true, true);
//								}
//							}
//						}
//					}
//				}
//			}
//		} else {
//			return 0x41531B;
//		}
//	}
//
//	return 0x41530C;
//}

//DEFINE_HOOK(0x4CD8C9, FlyLocomotionClass_Movement_AI_DisableTSExp, 0x9)
//{
//	GET(FootClass*, pFoot, EDX);
//	auto const& pTypeExt = TechnoTypeExt::ExtMap.Find(pFoot->GetTechnoType());
//	return pTypeExt->Disable_C4WarheadExp.Get() ? 0x4CD9C0 : 0x0;
//}

//#ifndef ENABLE_NEWHOOKS
//template<bool CheckNotHuman = true>
//static Iterator<AnimTypeClass*> GetDeathBodies(InfantryTypeClass* pThisType, const ValueableVector<AnimTypeClass*>& nOverrider)
//{
//	if (!nOverrider.empty())
//		return nOverrider;
//
//	if (pThisType->DeadBodies.Count > 0)
//	{
//		return pThisType->DeadBodies;
//	}
//
//	if constexpr (CheckNotHuman)
//	{
//		if (!pThisType->NotHuman)
//		{
//			return RulesGlobal->DeadBodies;
//		}
//	}
//
//	return { };
//}
//
//DEFINE_HOOK(0x520BCC, InfantryClass_DoingAI_FetchDoType, 0x6)
//{
//	GET(InfantryClass*, pThis, ESI);
//	GET(int, nDoType, EAX);
//	InfantryExt::ExtMap.Find(pThis)->CurrentDoType = nDoType;
//	return 0x0;
//}

//TODO : retest for desync
//DEFINE_HOOK(0x520BE5, InfantryClass_DoingAI_DeadBodies, 0x6)
//{
//	GET(InfantryClass* const, pThis, ESI);
//
//	auto const Iter = GetDeathBodies(pThis->Type, {});
//	constexpr auto Die = [](int x) { return x - 11; };
//
//	if (!Iter.empty()) {
//		int nIdx = 0;
//
//		if(InfantryTypeExt::ExtMap.Find(pThis->Type)->DeathBodies_UseDieSequenceAsIndex.Get()){
//			auto pInfExt = InfantryExt::ExtMap.Find(pThis);
//			nIdx = Die(pInfExt->CurrentDoType);
//			if (nIdx > Iter.size())
//				nIdx = Iter.size();
//		} else {
//			nIdx = ScenarioGlobal->Random.RandomFromMax(Iter.size() - 1);
//		}
//
//		if (AnimTypeClass* pSelected = Iter.at(nIdx)) {
//			if (const auto pAnim = GameCreate<AnimClass>(pSelected, pThis->GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0)) {
//				AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);
//			}
//		}
//	}
//
//    pThis->UnInit();
//	return 0x520E52;
//}

#endif

//#include "TestTurrent.h"
//
//std::vector<std::pair<ImageDatas, ImageDatas>> TTypeEXt::AdditionalTurrents { };

//#include <JumpjetLocomotionClass.h>
//#include <Ext/Building/Body.h>
//
//static int GetHeight(CellClass* pCell, const Nullable<int>& nOffset)
//{
//	int nHeight = 0;
//	if (auto pBuilding = pCell->GetBuilding())
//	{
//	   auto pBldExt = BuildingExt::ExtMap.Find(pBuilding);
//
//		CoordStruct vCoords = { 0, 0, 0 };
//		if(!pBldExt->LimboID != -1)
//			pBuilding->Type->Dimension2(&vCoords);
//
//		nHeight = vCoords.Z;
//	}
//	else
//	{
//		if (pCell->FindTechnoNearestTo({ 0,0 }, 0, 0))
//			nHeight = 85;
//	}
//
//	if (nOffset.isset()) {
//		nHeight -= nOffset.Get();
//		if (nHeight < 0)
//			nHeight = 0;
//	}
//
//	int nCellHeight = pCell->GetFloorHeight({ 128,128 });
//	if (pCell->ContainsBridge())
//		nCellHeight += 416;
//
//	return nHeight + nCellHeight;
//}
//
//DEFINE_HOOK(0x54D820, JumpJetLocomotionClass_UpdateHeightAboveObject, 0x6)
//{
//	GET(JumpjetLocomotionClass*, pLoco, ECX);
//
//	auto pOwner = pLoco->LinkedTo;
//	auto pCell = Map.GetCellAt(pOwner->Location);
//	int nHeight = GetHeight(pCell, {});
//
//	if (pLoco->__currentSpeed <= 0.0)
//	{
//		R->EAX(nHeight);
//	}
//	else
//	{
//		auto nFace = pLoco->Facing.Current().GetFacing<8>();
//		auto nAdj = AdjacentCoord[nFace];
//		auto nRes = CoordStruct { pOwner->Location.X + nAdj.X , pOwner->Location.Y + nAdj.Y };
//		auto pCell2 = Map.GetCellAt(nRes);
//		int nHeight2 = GetHeight(pCell2, {});
//		if (nHeight2 <= nHeight)
//			nHeight2 = (nHeight + nHeight2) / 2;
//
//		R->EAX(nHeight2);
//	}
//
//	return 0x54D917;
//}

//int __cdecl UnitClass_GetFireError_AIDeployFire(REGISTERS* R)
//{
//
//	bool DeployToFire_RequiresAIOnly = false;
//	GET(InfantryClass*, v1, ESI);
//	auto v2 = v1->Type;
//	if (!DeployToFire_RequiresAIOnly || v1->Owner->IsControlledByCurrentPlayer())
//	{
//		if (!*(v2 + 3602) || !v1->CanDeployNow())
//			return 0x7410B7;
//
//		return !Map.GetCellAt(v1->Location)->IsBuildable() ? 0x7410A8 :0x7410B7;
//	}
//	else
//	{
//		auto v22 = v1->GetCoords();
//		auto v7 =  Map.GetCellAt(v22);
//		auto v8 = *(v7 + 236);
//		auto v9 = !v7->IsBuildable() && v8 != 2 && v8 != 6;
//		if (*(v3 + 989))
//		{
//			if (!v1->f.t.cargo.CargoHold)
//				return 0x7410B7;
//			if (!v9)
//			{
//				MapClass::NearByLocation(
//				  0x87F7E8,
//				  &cell1,
//				  &cell2,
//				  SPEED_FOOT,
//				  -1,
//				  MZONE_NORMAL,
//				  0,
//				  1,
//				  1,
//				  0,
//				  0,
//				  0,
//				  0,
//				  &cellstrunct::empty,
//				  0,
//				  0);
//				cell2 = cell1;
//				if (cell1.X || cell1.Y)
//				{
//					v12 = cell1.X + (cell1.Y << 9);
//					if (v12 > 0x3FFFF || (v13 = *(MEMORY[0x87F924] + 4 * v12)) == 0)
//					{
//						v13 = 11263056;
//						MEMORY[0xABDC74] = cell1;
//					}
//					(v1->f.t.r.m.o.a.vftable->Stop_Driver)(v1);
//					v1->f.t.r.m.o.a.vftable->t.Assign_Target(v1, 0);
//					v1->f.t.r.m.o.a.vftable->t.Assign_Destination(v1, v13, 1);
//					v1->f.t.r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(v1, MISSION_MOVE, 0);
//					v1->f.t.r.m.o.a.vftable->t.r.m.Commence(v1);
//					v1->f.t.r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(v1, MISSION_UNLOAD, 0);
//				}
//			}
//			v14 = v1->f.t.r.m.o.a.vftable->CanDeployNow(v1);
//			v15 = 0x7410B7;
//			if (v14)
//				v15 = 0x7410A8;
//			result = v15;
//		}
//		else
//		{
//			if (!v9)
//				return 0x7410B7;
//			v10 = !v1->f.t.r.m.o.a.vftable->CanDeployNow(v1);
//			result = 7606440;
//			if (v10)
//				return 0x7410B7;
//		}
//	}
//	return result;
//}
//#include <Ext/WeaponType/Body.h>
//
//DEFINE_HOOK(0x4A730D , DiskLaserClass_Update_CalculateFacing, 0x6)
//{
//	GET(DiskLaserClass*, pThis, ESI);
//	GET(int, nFace, EDX);
//
//	auto pExt = WeaponTypeExt::ExtMap.Find(pThis->Weapon);
//
//	if (!pExt->DiskLaser_FiringOffset.isset())
//		return 0x0;
//
//	auto nOffset = (pExt->DiskLaser_FiringOffset.Get().Y - pExt->DiskLaser_FiringOffset.Get().X) >> 3;
//	pThis->DrawOffset.X = ((((pExt->DiskLaser_FiringOffset.Get().Y - pExt->DiskLaser_FiringOffset.Get().X) >> 4)
//					+ (nOffset -1) & (((nFace >> 11) + 1) >> 1)))
//					% nOffset;
//
//	return 0x4A7329;
//}
//
//DEFINE_HOOK(0x4A748E , DiskLaserClass_Update_CalculatePoint, 0x5)
//{
//	GET(DiskLaserClass*, pThis, ESI);
//
//	auto pExt = WeaponTypeExt::ExtMap.Find(pThis->Weapon);
//
//	if (!pExt->DiskLaser_FiringOffset.isset())
//		return 0x0;
//
//	auto nOffset = (pExt->DiskLaser_FiringOffset.Get().Y - pExt->DiskLaser_FiringOffset.Get().X) >> 3;
//
//	LEA_STACK(CoordStruct*, pCoord, 0x44);
//
//	auto nFLH = pThis->Owner->GetFLH(pCoord, 0, CoordStruct::Empty);
//
//	R->Stack(0x38, nFLH->X);
//	R->Stack(0x3C, nFLH->Y);
//	R->EBP(nFLH->Z);
//	R->ECX(pThis->DrawOffset.Y);
//
//	R->EBX((pThis->DrawOffset.X + nOffset - pThis->DrawOffset.Y) % nOffset);
//	R->EDX((nOffset - pThis->DrawOffset.Y + pThis->DrawOffset.X - 1) % nOffset);
//	R->EAX((pThis->DrawOffset.X + pThis->DrawOffset.Y) % nOffset);
//	R->EDI((pThis->DrawOffset.Y + pThis->DrawOffset.X + 1) % nOffset);
//
//	return 0x4A7507;
//}
