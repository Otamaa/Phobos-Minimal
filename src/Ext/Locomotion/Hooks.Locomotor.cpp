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
#include <Locomotor/DriveLocomotionClass.h>
#include <Locomotor/ShipLocomotionClass.h>

#include <Misc/DamageArea.h>

class NOVTABLE FakeDriveLocomotionClass final : DriveLocomotionClass
{
public:

	bool __stdcall _Is_Moving_Now()
	{
		if (!this->Owner || !this->Owner->IsAlive)
			return false;

		if (this->Owner->PrimaryFacing.Is_Rotating())
			return true;

		return this->Is_Moving()
			&& this->HeadToCoord.IsValid()
			&& this->Owner->GetCurrentSpeed() > 0;
	}
};


ASMJIT_PATCH(0x4B050B, DriveLocomotionClass_Process_Cargo, 0x5)
{
	GET(ILocomotion* const, pILoco, ESI);

	const auto pLoco = static_cast<DriveLocomotionClass* const>(pILoco);

	if (const auto pFoot = pLoco->LinkedTo)
	{
		if (const auto pTrans = pFoot->Transporter)
		{
			R->EAX(pTrans->GetCell());
			return 0x4B0516;
		}
	}

	return 0x0;
}

ASMJIT_PATCH(0x4B07CA, DriveLocomotionClass_Process_WakeAnim, 0x5)
{
	GET(ILocomotion* const, pLoco, ESI);
	const auto pDrive = static_cast<DriveLocomotionClass* const>(pLoco);
	const auto pTypeExt = GET_TECHNOTYPEEXT(pDrive->LinkedTo);
	TechnoExtData::PlayAnim(pTypeExt->Wake.Get(RulesClass::Instance->Wake), pDrive->LinkedTo);
	return 0x4B0828;
}

ASMJIT_PATCH(0x69FE92, ShipLocomotionClass_Process_WakeAnim, 0x5)
{
	GET(ILocomotion* const, pLoco, ESI);
	const auto pShip = static_cast<ShipLocomotionClass* const>(pLoco);
	const auto pTypeExt = GET_TECHNOTYPEEXT(pShip->LinkedTo);
	TechnoExtData::PlayAnim(pTypeExt->Wake.Get(RulesClass::Instance->Wake), pShip->LinkedTo);
	return 0x69FEF0;
}

ASMJIT_PATCH(0x75AC93, WalkLocomotionClass_Process_Wake, 0x6)
{
	if (!RulesExtData::Instance()->WalkLocomotorMakesWake)
		return 0;

	GET(ILocomotion* const, pThis, ESI);

	const auto pLinkedTo = static_cast<LocomotionClass*>(pThis)->LinkedTo;
	const auto pTypeExt = GET_TECHNOTYPEEXT(pLinkedTo);

	if (pThis->Is_Moving_Now() && !(Unsorted::CurrentFrame % 10) && !pLinkedTo->OnBridge && pLinkedTo->GetCell()->LandType == LandType::Water)
	{
		TechnoExtData::PlayAnim(pTypeExt->Wake.Get(RulesClass::Instance->Wake), pLinkedTo);
	}

	return 0;
}

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

Point2D *__stdcall JumpjetLoco_ILoco_Shadow_Point(ILocomotion* iloco, Point2D *pPoint)
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