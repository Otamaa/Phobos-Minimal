#include "Body.h"

#include <TacticalClass.h>

#include <BuildingClass.h>
#include <HouseClass.h>
#include <Ext/Rules/Body.h>
#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
//201803642
DEFINE_HOOK(0x460285, BuildingTypeClass_LoadFromINI_Muzzle, 0x6)
{
	enum { Skip = 0x460388, Read = 0x460299 };

	GET(BuildingTypeClass*, pThis, EBP);

	// Restore overriden instructions
	R->Stack(STACK_OFFS(0x368, 0x358), 0);
	R->EDX(0);

	// Disable Vanilla Muzzle flash when MaxNumberOccupants is 0 or more than 10
	return !pThis->MaxNumberOccupants || pThis->MaxNumberOccupants > 10
		? Skip : Read;
}

DEFINE_HOOK(0x44043D, BuildingClass_AI_Temporaled_Chronosparkle_MuzzleFix, 0x8)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if(	auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType)){
		if (pType->MaxNumberOccupants > 10) {
			GET(int, nFiringIndex, EBX);
			R->EAX(&pTypeExt->OccupierMuzzleFlashes[nFiringIndex]);
		}
	}
	return 0;
}

DEFINE_HOOK(0x45387A, BuildingClass_FireOffset_Replace_MuzzleFix, 0xA)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if(auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType)){
		if (pType->MaxNumberOccupants > 10) {
			R->EDX(&pTypeExt->OccupierMuzzleFlashes[pThis->FiringOccupantIndex]);
		}
	}

	return 0;
}

DEFINE_HOOK(0x458623, BuildingClass_KillOccupiers_Replace_MuzzleFix, 0x7)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if(auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType)){
		if (pType->MaxNumberOccupants > 10) {
			GET(int, nFiringIndex, EDI);
			R->ECX(&pTypeExt->OccupierMuzzleFlashes[nFiringIndex]);
		}
	}
	return 0;
}

DEFINE_HOOK(0x6D528A, TacticalClass_DrawPlacement_PlacementPreview, 0x6)
{
	if (auto const pBuilding = specific_cast<BuildingClass*>(DisplayClass::Instance->CurrentBuilding))
	{
		if (auto const pType = pBuilding->Type)
		{
			auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pType);

			if (pTypeExt &&
				pTypeExt->PlacementPreview_Show.Get(RulesExt::Global()->Building_PlacementPreview.Get(Phobos::Config::EnableBuildingPlacementPreview)))
			{
				SHPStruct* Selected = nullptr;
				bool bBuildupPresent = false;

				if (!pTypeExt->PlacementPreview_Shape.isset())
				{
					if (pType->LoadBuildup())
					{
						bBuildupPresent = true;
						Selected = pType->LoadBuildup();
					}
					else
					{
						Selected = pType->GetImage();
					}
				}
				else{
					Selected = pTypeExt->PlacementPreview_Shape.Get(nullptr);
				}

				auto const pImage = Selected;

				if (!pImage)
					return 0x0;

				CellStruct const nDisplayCell = Make_Global<CellStruct>(0x88095C);
				CellStruct const nDisplayCell_Offset = Make_Global<CellStruct>(0x880960);
				auto const pCell = MapClass::Instance->TryGetCellAt(nDisplayCell + nDisplayCell_Offset);

				if (!pCell)
					return 0x0;

				auto const nFrame = Math::clamp(pTypeExt->PlacementPreview_ShapeFrame.Get(bBuildupPresent ? ((pImage->Frames / 2) - 1) : 0), 0, (int)pImage->Frames);
				auto const nHeight = pCell->GetFloorHeight({ 0,0 });
				auto const&[nOffsetX , nOffsetY , nOffsetZ] = pTypeExt->PlacementPreview_Offset.Get();
				Point2D nPoint { 0,0 };
				TacticalClass::Instance->CoordsToClient(CellClass::Cell2Coord(pCell->MapCoords, nHeight + nOffsetZ), &nPoint);
				nPoint.X += nOffsetX;
				nPoint.Y += nOffsetY;
				auto const nFlag = BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass | EnumFunctions::GetTranslucentLevel(pTypeExt->PlacementPreview_TranslucentLevel.Get(RulesExt::Global()->BuildingPlacementPreview_TranslucentLevel.Get()));
				auto nREct = DSurface::Temp()->Get_Rect();
				nREct.Height -= 32;
				auto const pPalette = pTypeExt->PlacementPreview_Remap.Get() ? pBuilding->GetDrawer() : pTypeExt->PlacementPreview_Palette.GetOrDefaultConvert(FileSystem::UNITx_PAL());

				DSurface::Temp()->DrawSHP(pPalette, pImage, nFrame, &nPoint, &nREct, nFlag,
					0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
			}
		}
	}

	return 0x0;
}

//Make Building placement Grid tranparent
static void __fastcall CellClass_Draw_It_Shape(const args_DrawSHP& nArgs)
{
	auto nFlag = nArgs.Flags | EnumFunctions::GetTranslucentLevel(RulesExt::Global()->PlacementGrid_TranslucentLevel.Get());

	CC_Draw_Shape(nArgs.Surface, nArgs.Pal, nArgs.SHP, nArgs.FrameIndex, nArgs.Position, nArgs.Bounds, nFlag, nArgs.Remap, nArgs.ZAdjust,
		nArgs.ZGradientDescIndex, nArgs.Brightness, nArgs.TintColor, nArgs.ZShape, nArgs.ZShapeFrame, nArgs.XOffset, nArgs.YOffset);
}

DEFINE_POINTER_CALL(0x47EFB4, &CellClass_Draw_It_Shape);
