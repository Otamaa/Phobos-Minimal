#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>
#include <Strsafe.h>

#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/SWType/Body.h>

#include <New/Type/ArmorTypeClass.h>

#include <Locomotor/JumpjetLocomotionClass.h>
#include <Locomotor/HoverLocomotionClass.h>

#include <Misc/DamageArea.h>

#include "Header.h"

//ASMJIT_PATCH(0x7079A1, TechnoClass_Detach_HunterSeeker, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//	GET(void*, comparator, EBP);
//
//	if (!pThis->GetTechnoType()->HunterSeeker || (pThis->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
//		return 0x0;
//
//	if (pThis->Target == comparator)
//		((FootClass*)pThis)->Locomotor->Acquire_Hunter_Seeker_Target();
//
//	return 0x0;
//}
/*

*/

// ASMJIT_PATCH(0x514DFE, HoverLocomotionClass_ILocomotion_MoveTo_DeployToLand, 0x7)
// {
// 	GET(HoverLocomotionClass const* const, pLoco, ESI);
// 	const auto pFoot = !pLoco->Owner ? pLoco->LinkedTo : pLoco->Owner;

// 	if (pFoot->GetTechnoType()->DeployToLand)
// 		pFoot->NeedsRedraw = true;

// 	return 0;
// }

ASMJIT_PATCH(0x4B99A2, DropshipLoadout_WriteUnit, 0xA)
{
	GET(TechnoTypeClass*, pType, ESI);

	GET_STACK(bool, Available, STACK_OFFS(0x164, -0x8));

	LEA_STACK(Point2D*, BaseCoords, STACK_OFFS(0x164, 0x14C));
	LEA_STACK(Point2D*, AltCoords, STACK_OFFS(0x164, 0x144));

	COMPILETIMEEVAL size_t StringLen = 256;

	wchar_t pName[StringLen];
	wchar_t pArmor[StringLen];
	wchar_t pArmament[StringLen];
	wchar_t pCost[StringLen];

	StringCchPrintfW(pName, StringLen, L"Name: %hs", pType->Name);

	if (Available)
	{
		StringCchPrintfW(pCost, StringLen, L"Cost: %d", pType->GetCost());
	}
	else
	{
		StringCchPrintfW(pCost, StringLen, L"Cost: N/A");
	}

	if (auto pPrimary = pType->Weapon[0].WeaponType)
	{
		StringCchPrintfW(pArmament, StringLen, L"Armament: %hs", pPrimary->Name);
	}
	else
	{
		StringCchPrintfW(pArmament, StringLen, L"Armament: NONE");
	}

	if (const auto& pArmorType = ArmorTypeClass::FindFromIndexFix(static_cast<unsigned int>(pType->Armor)))
	{
		StringCchPrintfW(pArmor, StringLen, L"Armor: %hs", pArmorType->Name.data());
	}
	else
	{
		StringCchPrintfW(pArmor, StringLen, L"Armor: UNKNOWN");
	}

	auto Color = ColorScheme::Find(Available ? GameStrings::Green() : GameStrings::Red(), 1);

	auto pSurface = DSurface::Hidden();
	RectangleStruct pSurfaceRect = pSurface->Get_Rect();
	Point2D Coords = *BaseCoords;
	Coords.X += 450;
	Coords.Y += 300;

	PrintUnicode(AltCoords, pName, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	Coords.Y += 15;
	PrintUnicode(AltCoords, pArmament, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	Coords.Y += 15;
	PrintUnicode(AltCoords, pArmor, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	Coords.Y += 15;
	PrintUnicode(AltCoords, pCost, pSurface, &pSurfaceRect, &Coords, Color, 0, 70);

	return 0x4B9BBF;
}

ASMJIT_PATCH(0x4B9A4A, DropshipLoadout_PrintArmor, 7)
{
	R->EAX(ArmorTypeClass::Array[R->EDX()]->Name.data());
	return 0x4B9A51;
}

// ASMJIT_PATCH(0x514A21, HoverLocomotionClass_ILocomotion_Process_DeployToLand, 9)
// {
// 	GET(ILocomotion*, ILoco, ESI);

// 	auto const bIsMovingNow = ILoco->Is_Moving_Now();
// 	R->AL(bIsMovingNow);
// 	auto const pOwner = static_cast<HoverLocomotionClass*>(ILoco)->Owner;

// 	if (pOwner->InAir)
// 	{
// 		auto const pType = pOwner->GetTechnoType();
// 		if (pType->DeployToLand)
// 		{
// 			auto pCell = pOwner->GetCell();
// 			auto nLand = pCell->LandType;
// 			if ((nLand == LandType::Beach || nLand == LandType::Water) && !pCell->ContainsBridge())
// 			{
// 				pOwner->InAir = false;
// 				pOwner->QueueMission(Mission::Guard, true);
// 			}

// 			if (bIsMovingNow)
// 			{
// 				ILoco->Stop_Moving();
// 				pOwner->SetDestination(nullptr, true);
// 			}

// 			if (pType->DeployingAnim)
// 			{
// 				auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
// 				const int nDeployDirVal = pTypeExt->DeployDir.isset() ? (int)pTypeExt->DeployDir.Get() << 13 : RulesClass::Instance->DeployDir << 8;
// 				DirStruct nDeployDir(nDeployDirVal);

// 				if (pOwner->PrimaryFacing.Current() != nDeployDir) {
// 					pOwner->PrimaryFacing.Set_Desired(nDeployDir);
// 				}
// 			}

// 			if (pOwner->GetHeight() <= 0)
// 			{
// 				pOwner->InAir = false;
// 				ILoco->Mark_All_Occupation_Bits(0);
// 			}
// 		}
// 	}

// 	return 0x514A2A;
// }

// ASMJIT_PATCH(0x54C767, JumpjetLocomotionClass_State4_54C550_DeployDir, 6)
// {
// 	GET(JumpjetLocomotionClass*, pLoco, ESI);
// 	auto const pOwner = pLoco->LinkedTo;
// 	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pOwner->GetTechnoType());
// 	const int nDeployDirVal = pTypeExt->DeployDir.isset() ? (int)pTypeExt->DeployDir.Get() << 13 : RulesClass::Instance->DeployDir << 8;
//
// 	DirStruct nDeployDir(nDeployDirVal);
//
// 	if (pLoco->Facing.Current() != nDeployDir)
// 		pLoco->Facing.Set_Desired(nDeployDir);
//
// 	return 0x54C7A3;
// }

Point2D *__stdcall JumpjetLoco_ILoco_Shadow_Point(ILocomotion * iloco, Point2D *pPoint)
{
	const auto pLoco = static_cast<JumpjetLocomotionClass*>(iloco);
	const auto pThis = pLoco->LinkedTo;
	const auto pCell = MapClass::Instance->GetCellAt(pThis->Location);

	auto height = pThis->Location.Z - MapClass::Instance->GetCellFloorHeight(pThis->Location);

	// Vanilla GetHeight check OnBridge flag, which can not work on jumpjet
	// Here, we simulate the drawing of an airplane for altitude calculation
	if (pCell->ContainsBridge()
		&& ((pCell->Flags & CellFlags::BridgeDir) && pCell->GetNeighbourCell(FacingType::North)->ContainsBridge()
			|| !(pCell->Flags & CellFlags::BridgeDir) && pCell->GetNeighbourCell(FacingType::West)->ContainsBridge()))
	{
		height -= Unsorted::BridgeHeight;
	}

	pPoint->X = 0;
	pPoint->Y = Game::AdjustHeight(height) ;
	return pPoint;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7ECD98, JumpjetLoco_ILoco_Shadow_Point);