#include "Body.h"

#include <UnitClass.h>
#include <Utilities/Macro.h>

#include <BitFont.h>
#include <New/Entity/FlyingStrings.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/House/Body.h>


DEFINE_HOOK(0x739717, UnitClass_TryToDeploy_Transfer, 0x8)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	if (R->AL())
	{
		if (pUnit->Type->DeployToFire && pUnit->Target)
			pStructure->LastTarget = pUnit->Target;

		BuildingExt::ExtMap.Find(pStructure)->DeployedTechno = true;

		return 0x73971F;	
	}

	return 0x739A6E;
}

//DEFINE_HOOK(0x7396D2, UnitClass_TryToDeploy_Transfer, 0x5)
//{
//	GET(UnitClass*, pUnit, EBP);
//	GET(BuildingClass*, pStructure, EBX);
//
//	if (pUnit->Type->DeployToFire && pUnit->Target)
//		pStructure->LastTarget = pUnit->Target;
//
//	BuildingExt::ExtMap.Find(pStructure)->DeployedTechno = true;
//
//	return 0;
//}

DEFINE_HOOK(0x449ADA, BuildingClass_MissionConstruction_DeployToFireFix, 0x6) //was 0
{
	GET(BuildingClass*, pThis, ESI);

	if (BuildingExt::ExtMap.Find(pThis)->DeployedTechno && pThis->LastTarget)
	{
		pThis->Target = pThis->LastTarget;
		pThis->QueueMission(Mission::Attack, false);
	}
	else
	{
		pThis->QueueMission(Mission::Guard, false);
	}

	return 0x449AE8;
}

DEFINE_HOOK(0x43FE73, BuildingClass_AI_FlyingStrings, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	if (Unsorted::CurrentFrame % 15 != 0)
		return 0;

	auto const pExt = BuildingExt::ExtMap.Find(pThis);
	{
		if (pExt->AccumulatedGrindingRefund)
		{
			const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
			const int refundAmount = pExt->AccumulatedGrindingRefund;
			const bool isPositive = refundAmount > 0;
			const auto color = isPositive ? ColorStruct { 0, 255, 0 } : ColorStruct { 255, 0, 0 };
			wchar_t moneyStr[0x20];
			swprintf_s(moneyStr, L"%s$%d", isPositive ? L"+" : L"-", std::abs(refundAmount));
			auto coords = pThis->GetRenderCoords();
			int width = 0, height = 0;
			BitFont::Instance->GetTextDimension(moneyStr, &width, &height, 120);

			Point2D pixelOffset = Point2D::Empty;
			pixelOffset += pTypeExt->Grinding_DisplayRefund_Offset;
			pixelOffset.X -= width / 2;
			coords.Z += 104 * pThis->Type->Height;

			if (auto const pCell = MapClass::Instance->TryGetCellAt(coords)) {
				if(!pCell->IsFogged() && !pCell->IsShrouded()) {
					if(pThis->VisualCharacter( 0,HouseClass::CurrentPlayer()) != VisualType::Hidden ) {
						FlyingStrings::Add(moneyStr, coords, color, pixelOffset);
					}
				}
			}

			pExt->AccumulatedGrindingRefund = 0;
		}
	}

	return 0;
}

DEFINE_HOOK(0x44224F, BuildingClass_ReceiveDamage_DamageSelf, 0x5)
{
	enum { SkipCheck = 0x442268 };

	REF_STACK(args_ReceiveDamage const, receiveDamageArgs, STACK_OFFS(0x9C, -0x4));

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(receiveDamageArgs.WH);
	return pWHExt->AllowDamageOnSelf.isset() && pWHExt->AllowDamageOnSelf.Get() ? SkipCheck : 0x0;
}

DEFINE_HOOK(0x4506D4, BuildingClass_UpdateRepair_Campaign, 0x6)
{
	enum { GoRepair = 0x4506F5, SkipRepair = 0x450813 };
	GET(BuildingClass*, pThis, ESI);
	//GET(HouseClass*, pHouse, ECX);

	if (pThis->BeingProduced && SessionClass::Instance->GameMode == GameMode::Campaign)
		if (HouseExt::ExtMap.Find(pThis->Owner)->RepairBaseNodes[GameOptionsClass::Instance->Difficulty])
			return GoRepair;

	return 0x0;
}