#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Tiberium/Body.h>

#include <Utilities/Macro.h>

#include <CaptureManagerClass.h>
#include <SpawnManagerClass.h>
#include <InfantryClass.h>
#include <InfantryTypeClass.h>

#include <Misc/Ares/Hooks/Header.h>
#include <TextDrawing.h>

#pragma region pipDrawings

ASMJIT_PATCH(0x708CD9, TechnoClass_PipCount_GetTotalAmounts, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	const auto storange = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
	R->EAX((float)storange->GetAmounts());
	return 0x708CE4;
}

namespace Tiberiumpip
{
	struct PackedTibPipData
	{
		int value;
		int pipIdx;
	};

	int GetEmptyShapeIndex(bool isWeeder, TechnoTypeExtData* pTypeData)
	{

		if (isWeeder && pTypeData->Weeder_PipEmptyIndex.isset())
			return pTypeData->Weeder_PipEmptyIndex;
		else if (pTypeData->Tiberium_EmptyPipIdx.isset())
			return pTypeData->Tiberium_EmptyPipIdx;

		return 0;
	}

	int DrawFrames(
		bool IsWeeder,
		TechnoTypeExtData* pTypeData,
		std::vector<PackedTibPipData>& Amounts,
		const Iterator<int> orders)
	{
		for (size_t i = 0; i < (size_t)TiberiumClass::Array->Count; i++)
		{
			size_t index = i;
			if (i < orders.size() && orders[i] >= 0)
				index = orders[i];

			if (Amounts[index].value > 0)
			{
				--Amounts[index].value;
				return Amounts[index].pipIdx;
			}
		}

		return GetEmptyShapeIndex(IsWeeder, pTypeData);
	};

	int GetShapeIndex(int storageIndex, TechnoTypeExtData* pTypeData)
	{
		auto frames = pTypeData->Tiberium_PipIdx.GetElements(RulesExtData::Instance()->Pips_Tiberiums_Frames);
		const auto pTibExt = TiberiumExtContainer::Instance.Find(TiberiumClass::Array->Items[storageIndex]);

		return (size_t)storageIndex >= frames.size() || frames[storageIndex] < 0 ? pTibExt->PipIndex : frames[storageIndex];
	}

	void DrawTiberiumPip(TechnoClass* pTechno, TechnoTypeClass* pType, int nMax, SHPStruct* pShape, ConvertClass* pConvert, Point2D* nPoints, RectangleStruct* pRect, int nOffsetX, int nOffsetY)
	{
		if (!nMax)
			return;

		auto const nStorage = pType->Storage;
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto what = pTechno->WhatAmI();

		Point2D nOffs {};
		const auto pBuilding = what == BuildingClass::AbsID ? static_cast<BuildingClass*>(pTechno) : nullptr;
		const auto pUnit = what == UnitClass::AbsID ? static_cast<UnitClass*>(pTechno) : nullptr;

		auto pShape_toUse = pShape;
		if (pTypeExt->Tiberium_PipShapes)
		{
			pShape_toUse = pTypeExt->Tiberium_PipShapes;
		}

		ConvertClass* nPal = pConvert;
		if (auto pConv = pTypeExt->Tiberium_PipShapes_Palette.GetConvert())
		{
			nPal = pConv;
		}

		auto storage = &TechnoExtContainer::Instance.Find(pTechno)->TiberiumStorage;
		static std::vector<PackedTibPipData> Amounts(TiberiumClass::Array->Count);

		const bool isWeeder = pBuilding && pBuilding->Type->Weeder || pUnit && pUnit->Type->Weeder;

		for (size_t i = 0; i < Amounts.size(); i++)
		{
			int FrameIdx = 0;
			int amount = 0;

			if (pBuilding && pBuilding->Type->Weeder)
			{
				amount = int(pTechno->Owner->GetWeedStoragePercentage() * nMax + 0.5);
			}
			else
			{
				amount = int(storage->m_values[i] / nStorage * nMax + 0.5);
			}

			if (!isWeeder)
			{
				FrameIdx = GetShapeIndex(i, pTypeExt);
			}
			else
			{
				FrameIdx = pTypeExt->Weeder_PipIndex.Get(1);
			}

			Amounts[i] = { amount , FrameIdx };
		}

		static COMPILETIMEEVAL std::array<int, 4u> defOrder { {0, 2, 3, 1} };
		const auto displayOrders = RulesExtData::Instance()->Pips_Tiberiums_DisplayOrder.GetElements(make_iterator(&defOrder[0], 4u));

		for (int i = nMax; i; --i)
		{
			Point2D nPointHere { nOffs.X + nPoints->X  , nOffs.Y + nPoints->Y };
			CC_Draw_Shape(
			DSurface::Temp(),
			nPal,
			pShape_toUse,
			DrawFrames(isWeeder, pTypeExt, Amounts, displayOrders),
			&nPointHere,
			pRect,
			0x600,
			0,
			0,
			0,
			1000,
			0,
			0,
			0,
			0,
			0);

			nOffs.X += nOffsetX;
			nOffs.Y += nOffsetY;
		}
	}

}

#ifdef _done

ASMJIT_PATCH(0x70A36E, TechnoClass_DrawPip_Ammo, 0x6)
{
	enum { SkipGameDrawing = 0x70A4EC };

	GET(TechnoClass*, pThis, ECX);
	LEA_STACK(RectangleStruct*, offset, STACK_OFFSET(0x74, -0x24));
	GET_STACK(RectangleStruct*, rect, STACK_OFFSET(0x74, 0xC));
	GET(int, pipWrap, EBX);
	GET_STACK(int, pipCount, STACK_OFFSET(0x74, -0x54));
	GET_STACK(int, maxPips, STACK_OFFSET(0x74, -0x60));
	GET(int, yOffset, ESI); //verticalSpacing
	GET_STACK(SHPStruct*, pDefault, 0x74 - 0x48);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	Point2D position = { offset->X + pTypeExt->AmmoPipOffset->X, offset->Y + pTypeExt->AmmoPipOffset->Y };
	ConvertClass* pConvert = pTypeExt->AmmoPip_Palette.GetConvert() ?
		pTypeExt->AmmoPip_Palette.GetConvert()
		: FileSystem::PALETTE_PAL();

	auto pSHApe = pTypeExt->AmmoPip_shape.Get(pDefault);

	if (pipWrap > 0)
	{
		int levels = maxPips / pipWrap - 1;

		for (int i = 0; i < pipWrap; i++)
		{
			int frame = pTypeExt->PipWrapAmmoPip;

			if (levels >= 0)
			{
				int counter = i + pipWrap * levels;
				int frameCounter = levels;
				bool calculateFrame = true;

				while (counter >= pThis->Ammo)
				{
					frameCounter--;
					counter -= pipWrap;

					if (frameCounter < 0)
					{
						calculateFrame = false;
						break;
					}
				}

				if (calculateFrame)
					frame = frameCounter + frame + 1;
			}

			position.X += offset->Width;
			position.Y += yOffset;

			DSurface::Temp->DrawSHP(pConvert, pSHApe,
				frame, &position, rect, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}
	else
	{
		int ammoFrame = pTypeExt->AmmoPip;
		int emptyFrame = pTypeExt->EmptyAmmoPip;

		for (int i = 0; i < maxPips; i++)
		{
			if (i >= pipCount && emptyFrame < 0)
				break;

			int frame = i >= pipCount ? emptyFrame : ammoFrame;
			position.X += offset->Width;
			position.Y += yOffset;

			DSurface::Temp->DrawSHP(pConvert, pSHApe,
				frame, &position, rect, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

	return SkipGameDrawing;
}

ASMJIT_PATCH(0x709B79, TechnoClass_DrawPip_Spawner, 0x6)
{
	enum { SkipGameDrawing = 0x709C27 };

	GET(TechnoClass*, pThis, EBP);
	GET(TechnoTypeClass*, pType, EAX);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (!pTypeExt->ShowSpawnsPips.Get(((int)pType->PipScale == 6)))
	{
		return SkipGameDrawing;
	}

	LEA_STACK(RectangleStruct*, offset, STACK_OFFSET(0x74, -0x24));
	GET_STACK(RectangleStruct*, rect, STACK_OFFSET(0x74, 0xC));
	//GET_STACK(SHPStruct*, shape, STACK_OFFSET(0x74, -0x58));
	GET_STACK(bool, isBuilding, STACK_OFFSET(0x74, -0x61));
	GET(int, maxSpawnsCount, EBX);

	ConvertClass* pPal = FileSystem::PALETTE_PAL();
	const auto pShape = isBuilding ?
		pTypeExt->PipShapes01.Get(FileSystem::PIPS_SHP()) :
		pTypeExt->PipShapes02.Get(FileSystem::PIPS_SHP());

	//if (isBuilding)
	//{
	//	const auto pBuildingTypeExt = BuildingTypeExtContainer::Instance.Find((BuildingTypeClass*)pType);
	//
	//	if (pBuildingTypeExt->PipShapes01Remap)
	//		pPal = pThis->GetRemapColour();
	//	else if (const auto pConvertData = pBuildingTypeExt->PipShapes01Palette)
	//		pPal = pConvertData->GetConvert<PaletteManager::Mode::Temperate>();
	//
	//}

	int currentSpawnsCount = pThis->SpawnManager->CountDockedSpawns();
	Point2D position { offset->X + pTypeExt->SpawnsPipOffset->X, offset->Y + pTypeExt->SpawnsPipOffset->Y };
	const Point2D size = pTypeExt->SpawnsPipSize.Get(
		isBuilding ?
		RulesExtData::Instance()->Pips_Generic_Buildings_Size :
		RulesExtData::Instance()->Pips_Generic_Size
	);

	for (int i = 0; i < maxSpawnsCount; i++)
	{
		int frame = i < currentSpawnsCount ? pTypeExt->SpawnsPip : pTypeExt->EmptySpawnsPip;

		DSurface::Temp->DrawSHP(pPal, pShape, frame,
			&position, rect, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

		position.X += size.X;
		position.Y += size.Y;
	}

	return SkipGameDrawing;
}

ASMJIT_PATCH(0x70A4FB, TechnoClass_DrawPip_SelfHealGain, 0x5)
{
	enum { SkipGameDrawing = 0x70A6C0 };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x74, -0x4));
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFS(0x74, -0xC));

	const auto pType = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	TechnoExtData::DrawSelfHealPips(pThis, pLocation, pBounds, FileSystem::PIPS_SHP(), FileSystem::PALETTE_PAL());

	return SkipGameDrawing;
}

ASMJIT_PATCH(0x70A1F6, TechnoClass_DrawPip_Tiberium, 0x6)
{
	struct __declspec(align(4)) PipDataStruct
	{
		Point2D nPos;
		int Int;
		int Number;
	};

	GET(TechnoClass* const, pThis, EBP);
	GET_STACK(PipDataStruct, pDatas, STACK_OFFS(0x74, 0x24));
	//GET_STACK(SHPStruct*, pShapes, STACK_OFFS(0x74, 0x68));
	GET_STACK(RectangleStruct*, pBound, STACK_OFFSET(0x74, 0xC));
	GET(int, nOffsetY, ESI);

	Tiberiumpip::DrawTiberiumPip(pThis, &pDatas.nPos, pBound, pDatas.Int, nOffsetY);
	//DrawCargoPips_NotBySize(pThis, pThis->GetTechnoType()->GetPipMax(), &pDatas.nPos, pBound, pDatas.Int, nOffsetY);
	return 0x70A340;
}

ASMJIT_PATCH(0x709C84, TechnoClass_DrawPip_Occupants, 0x6)
{
	struct DrawPipDataStruct
	{
		int nOccupantsCount; int Y; SHPStruct* pShape; int nMaxOccupants;
	};

	GET(BuildingClass*, pThis, EBP);
	GET(int, nOccupantIdx, EDI);
	GET(int, nOffset_X, EBX);
	GET_STACK(int, nOffset_Y, STACK_OFFS(0x74, 0x50));
	GET_STACK(DrawPipDataStruct, nPipDataStruct, STACK_OFFS(0x74, 0x60));
	GET_STACK(Point2D, nDrawOffset, STACK_OFFS(0x74, 0x24));
	GET_STACK(Point2D, nOffsetadd, STACK_OFFS(0x74, 0x1C));
	GET_STACK(RectangleStruct*, pRect, STACK_OFFS(0x74, -0xC));
	GET(int, nOffsetY_Increment, ESI);

	int nPipFrameIndex = 6;
	SHPStruct* pPipFile = nPipDataStruct.pShape;
	ConvertClass* pPalette = FileSystem::THEATER_PAL;

	if (nOccupantIdx < nPipDataStruct.nMaxOccupants)
	{
		if (auto const pInfantry = pThis->Occupants.Items[nOccupantIdx])
		{
			const auto pExt = TechnoTypeExtContainer::Instance.Find(pInfantry->Type);

			if (const auto pGarrisonPip = pExt->PipGarrison.Get(nullptr))
			{
				pPipFile = pGarrisonPip;
				nPipFrameIndex = std::clamp((int)pExt->PipGarrison_FrameIndex, 0, (int)pGarrisonPip->Frames);
				if (auto pConvert_c = pExt->PipGarrison_Palette.GetConvert())
					pPalette = pConvert_c;
			}
			else
			{
				nPipFrameIndex = (int)pInfantry->Type->OccupyPip;
			}
		}
	}

	Point2D nOffset { nOffset_X + nDrawOffset.X ,nDrawOffset.Y + nOffset_Y };
	if (pPipFile)
	{
		DSurface::Temp->DrawSHP(
			pPalette,
			pPipFile,
			nPipFrameIndex,
			&nOffset,
			pRect,
			BlitterFlags(0x600),
			0,
			0,
			ZGradient::None,
			1000,
			0,
			0,
			0,
			0,
			0);
	}

	++nOccupantIdx;
	nOffset_X += nOffsetadd.X;
	nOffset_Y += nOffsetY_Increment;

	// need to forward the value bacause it is needed for next loop
	R->EBX(nOffset_X);
	R->ECX(nOffset_Y);
	R->EDI(nOccupantIdx);
	R->EAX(nPipDataStruct.nOccupantsCount);

	return 0x709D11;
}

// the game specifically hides tiberium building pips. allow them, but
// take care they don't show up for the original game
ASMJIT_PATCH(0x709B4E, TechnoClass_DrawPip_SkipSkipTiberium, 6)
{
	GET(TechnoClass* const, pThis, EBP);

	bool showTiberium = true;
	if (const auto pBld = cast_to<BuildingClass*, false>(pThis))
	{
		if ((pBld->Type->Refinery || pBld->Type->ResourceDestination) && pBld->Type->Storage > 0)
		{
			// show only if this refinery uses storage. otherwise, the original
			// refineries would show an unused tiberium pip scale
			showTiberium = TechnoTypeExtContainer::Instance.Find(pBld->Type)->Refinery_UseStorage;
		}
	}

	return showTiberium ? 0x709B6E : 0x70A980;
}

ASMJIT_PATCH(0x709B2E, TechnoClass_DrawPip_Sizes, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	REF_STACK(int, pipWidth, STACK_OFFSET(0x74, -0x1C));

	const bool isBuilding = pThis->WhatAmI() == AbstractType::Building;

	Point2D size = Point2D::Empty;
	const auto pType = pThis->GetTechnoType();

	if (pType->PipScale == PipScale::Ammo)
	{
		size = TechnoTypeExtContainer::Instance.Find(pType)->AmmoPipSize.Get((isBuilding ?
			RulesExtData::Instance()->Pips_Ammo_Buildings_Size : RulesExtData::Instance()->Pips_Ammo_Size));
	}
	else
	{
		size = (isBuilding ? RulesExtData::Instance()->Pips_Generic_Buildings_Size : RulesExtData::Instance()->Pips_Generic_Size).Get();
	}

	pipWidth = size.X;
	R->ESI(size.Y);

	return 0;
}

ASMJIT_PATCH(0x709ACF, TechnoClass_DrawPip_PipShape1_A, 0x6)
{
	GET(TechnoClass* const, pThis, EBP);
	GET(SHPStruct*, pPipShape01, ECX);

	R->ECX(TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())
		->PipShapes01.Get(pPipShape01));

	return 0;
}

ASMJIT_PATCH(0x709AE3, TechnoClass_DrawPip_PipShape1_B, 0x6)
{
	GET(TechnoClass* const, pThis, EBP);
	GET(SHPStruct*, pPipShape01, EAX);

	R->EAX(TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())
		->PipShapes01.Get(pPipShape01));

	return 0;
}

ASMJIT_PATCH(0x709AF8, TechnoClass_DrawPip_PipShape2, 0x6)
{
	GET(TechnoClass* const, pThis, EBP);
	GET(SHPStruct*, pPipShape02, EBX);

	R->EBX(TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())
		->PipShapes02.Get(pPipShape02));

	return 0;
}
ASMJIT_PATCH(0x709D38, TechnoClass_DrawPip_Passengers, 7)
{
	GET(TechnoClass* const, pThis, EBP);
	GET(TechnoTypeClass*, pType, EAX);

	if (pType->PipScale != PipScale::Passengers)
		return 0x70A083;

	GET(int, nBracketPosDeltaY, ESI);
	GET_STACK(SHPStruct*, pShp, 0x1C);
	GET_STACK(RectangleStruct*, pRect, 0x80);
	GET_STACK(int, nPosX, 0x50);
	GET_STACK(int, nPosY, 0x54);
	GET_STACK(int, nBracketPosDeltaX, 0x58);

	Point2D nPos = { nPosX ,nPosY };
	int nForGunner = 0;
	bool fail = false;
	const auto pData = TunnelFuncs::PopulatePassangerPIPData(pThis, pType, fail);

	if (fail)
		return 0x70A083;

	for (auto nDPos = pData->begin(); nDPos != pData->end(); ++nDPos)
	{
		DSurface::Temp->DrawSHP
		(FileSystem::PALETTE_PAL,
			pShp,
			*nDPos,
			&nPos,
			pRect,
			BlitterFlags(0x600),
			0,
			0,
			ZGradient::Ground,
			1000,
			0,
			0,
			0,
			0,
			0
		);

		++nForGunner;
		const auto nXHere = nBracketPosDeltaX + nPos.X;
		const auto nYHere = nBracketPosDeltaY + nPos.Y;
		nPos.X += nBracketPosDeltaX;
		nPos.Y += nBracketPosDeltaY;

		if ((bool)nForGunner == pType->Gunner)
		{
			nPos.X += nXHere + nBracketPosDeltaX;
			nPos.Y += nYHere + nBracketPosDeltaY;
		}
	}

	return 0x70A4EC;
}

#endif

// Helper structure for drawing state
struct Spacing
{
	int Horizontal;
	int Vertical;
};

struct PipDrawState
{
	Point2D pos;
	union
	{
		Spacing spacings;
		Point2D spacigns_;
	};
};

// Helper structure for pip drawing
struct PipDrawInfo
{
	SHPStruct* shape;
	ConvertClass* convert;
	int count;
	int* pipTypes;
	int maxPips;
};

// Constants for common blitter flags
constexpr BlitterFlags PIP_BLITTER_FLAGS = BlitterFlags::bf_400 | BlitterFlags::Centered;
constexpr BlitterFlags PIP_BLITTER_FLAGS_DARKEN = PIP_BLITTER_FLAGS | BlitterFlags::Darken;

// Forward declarations for helper functions
static int CalculateAmmoBarFrame(int levels, int i, int pipWrap, int curammo, int defaultEmpty);
static void DrawSinglePip(Point2D* position, ConvertClass* pConvert, SHPStruct* shape, int frameIndex, RectangleStruct* clipRect, BlitterFlags flags = PIP_BLITTER_FLAGS);
static void DrawSpawnPips(TechnoClass* techno, bool isBuilding, TechnoTypeClass* technoType, PipDrawState* drawState, PipDrawInfo* pipInfo, RectangleStruct* clipRect);
static void DrawBuildingOccupants(BuildingClass* building, PipDrawState* drawState, PipDrawInfo* pipInfo, RectangleStruct* clipRect);
static void DrawAmmoPip(TechnoClass* techno, bool isBuilding, SHPStruct* pipShape, PipDrawState* drawState, PipDrawInfo* pipInfo, RectangleStruct* clipRect);
static void DrawMindControlPip(TechnoClass* techno, PipDrawState* drawState, PipDrawInfo* pipInfo, RectangleStruct* clipRect);
static void DrawPassengerPips(TechnoClass* techno, TechnoTypeClass* technoType, PipDrawState* drawState, PipDrawInfo* pipInfo, RectangleStruct* clipRect);
static void DrawGroupNumber(TechnoClass* techno, AbstractType unitType, Point2D* position, RectangleStruct* clipRect);

void __fastcall FakeTechnoClass::__Draw_Pips(TechnoClass* techno, discard_t, Point2D* position, Point2D* unused, RectangleStruct* clipRect)
{
	// Cache frequently accessed values
	const AbstractType technoType = techno->WhatAmI();
	const bool isBuilding = (technoType == AbstractType::Building);
	const bool isInfantry = (technoType == AbstractType::Infantry);
	TechnoTypeClass* technoTypeClass = techno->GetTechnoType();
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(technoTypeClass);
	SHPStruct* pips_SHP = pTypeExt->PipShapes01.Get(FileSystem::PIPS_SHP());
	SHPStruct* pips2_SHP = pTypeExt->PipShapes02.Get(FileSystem::PIPS2_SHP());

	Point2D cur_pos {
		.X = isBuilding ? position->X + 6 : position->X - 5,
		.Y = isBuilding ? position->Y - 1 : position->Y
	};

	PipDrawState drawState {
		.pos = cur_pos ,
		.spacigns_ = (isBuilding ? RulesExtData::Instance()->Pips_Generic_Buildings_Size : RulesExtData::Instance()->Pips_Generic_Size).Get()
	};

	PipDrawInfo pipInfo {
		.shape = isBuilding ? pips_SHP : pips2_SHP,
		.convert = FileSystem::PALETTE_PAL(),
		.count = techno->GetPipFillLevel(),
		.maxPips = technoTypeClass->GetPipMax()
	};

	// Adjust for infantry
	if (isInfantry)
	{
		drawState.pos.X += 11;
	}

	// Draw spawns (drones, etc.)
	if (!isBuilding || technoTypeClass->PipScale != PipScale::Tiberium)
	{
		DrawSpawnPips(techno, isBuilding, technoTypeClass, &drawState, &pipInfo, clipRect);

		bool showTiberium = true;
		// Draw building occupants
		if (isBuilding)
		{
			const auto pBld = static_cast<BuildingClass*>(techno);

			if ((pBld->Type->Refinery || pBld->Type->ResourceDestination) && pBld->Type->Storage > 0)
			{
				// show only if this refinery uses storage. otherwise, the original
				// refineries would show an unused tiberium pip scale
				showTiberium = pTypeExt->Refinery_UseStorage;
			}

			DrawBuildingOccupants((BuildingClass*)techno, &drawState, &pipInfo, clipRect);
		}

		// Draw various pip types based on MaxPassengers
		if (technoTypeClass->Passengers <= 0)
		{
			switch (technoTypeClass->PipScale)
			{
			case PipScale::Ammo:
				DrawAmmoPip(techno, isBuilding, pips2_SHP, &drawState, &pipInfo, clipRect);
				break;
			case PipScale::Tiberium:
				if (showTiberium)
				{
					Tiberiumpip::DrawTiberiumPip(techno, technoTypeClass, pipInfo.maxPips, pipInfo.shape, pipInfo.convert, &drawState.pos, clipRect, drawState.spacigns_.X, drawState.spacigns_.Y);
				}
				break;
			case PipScale::MindControl:
				DrawMindControlPip(techno, &drawState, &pipInfo, clipRect);
				break;
			default:
				break;
			}
		}
		else
		{
			DrawPassengerPips(techno, technoTypeClass, &drawState, &pipInfo, clipRect);
		}

		// Draw self-heal indicator
		TechnoExtData::DrawSelfHealPips(techno, &drawState.pos, clipRect, pips_SHP, pipInfo.convert);

		// Draw info tip for non-buildings
		// techno other than buildings has no extra info atm
		//if (!isBuilding) {
		//    Point2D infoPos {
		//        .X = position->X - 10,
		//        .Y = position->Y + 10
		//    };
		//    techno->DrawExtraInfo(&infoPos, position, clipRect);
		//}

		// Draw group number
		DrawGroupNumber(techno, technoType, position, clipRect);
	}
}

// Draw spawn manager pips (for carriers, etc.)
static void DrawSpawnPips(TechnoClass* techno, bool isBuilding, TechnoTypeClass* technoType, PipDrawState* drawState, PipDrawInfo* pipInfo, RectangleStruct* clipRect)
{
	int spawnCount = technoType->SpawnsNumber;

	if (spawnCount <= 0)
	{
		return;
	}

	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(technoType);

	if (!pTypeExt->ShowSpawnsPips.Get(((int)technoType->PipScale == 6)))
	{
		return;
	}

	int currentSpawnsCount = techno->SpawnManager->CountDockedSpawns();
	const Point2D size = pTypeExt->SpawnsPipSize.Get(drawState->spacigns_);

	for (int i = 0; i < spawnCount; i++)
	{
		Point2D pipPos {
			.X = drawState->pos.X + i * size.X,
			.Y = drawState->pos.Y + i * size.Y
		};

		pipPos += pTypeExt->SpawnsPipOffset;

		DrawSinglePip(&pipPos, pipInfo->convert, pipInfo->shape,
			i < currentSpawnsCount ? pTypeExt->SpawnsPip : pTypeExt->EmptySpawnsPip,
			clipRect);

	}

}

// Draw building occupant pips
static void DrawBuildingOccupants(BuildingClass* building, PipDrawState* drawState, PipDrawInfo* pipInfo, RectangleStruct* clipRect)
{
	int maxOccupants = building->Type->MaxNumberOccupants;

	if (!building->Type->ShowOccupantPips || maxOccupants <= 0)
	{
		return;
	}

	int currentOccupants = building->GetOccupantCount();

	SHPStruct* pPipFile = pipInfo->shape;
	ConvertClass* pPalette = pipInfo->convert;

	for (int i = 0; i < maxOccupants; i++)
	{

		PipIndex frameIndex = PipIndex::PersonEmpty;
		if (i < currentOccupants)
		{
			if (auto const pInfantry = building->Occupants.Items[i])
			{

				const auto pIfnExt = TechnoTypeExtContainer::Instance.Find(pInfantry->Type);

				if (const auto pGarrisonPip = pIfnExt->PipGarrison.Get(nullptr))
				{
					pPipFile = pGarrisonPip;
					frameIndex = (PipIndex)std::clamp((int)pIfnExt->PipGarrison_FrameIndex, 0, (int)pGarrisonPip->Frames);
					if (auto pConvert_c = pIfnExt->PipGarrison_Palette.GetConvert())
						pPalette = pConvert_c;
				}
				else
				{
					frameIndex = pInfantry->Type->OccupyPip;
				}
			}
		}

		Point2D pipPos {
			.X = drawState->pos.X + i * drawState->spacings.Horizontal,
			.Y = drawState->pos.Y + i * drawState->spacings.Vertical
		};

		DrawSinglePip(&pipPos, pPalette, pPipFile, (int)frameIndex, clipRect);
	}
}

// PipScale 5: Simple filled/empty pips with overload indicator
static void DrawMindControlPip(TechnoClass* techno, PipDrawState* drawState, PipDrawInfo* pipInfo, RectangleStruct* clipRect)
{
	for (int i = 0; i < pipInfo->maxPips; i++)
	{
		Point2D pipPos {
			.X = drawState->pos.X + i * drawState->spacings.Horizontal,
			.Y = drawState->pos.Y + i * drawState->spacings.Vertical
		};

		int frameIndex = (i < pipInfo->count) ? 1 : 0;
		DrawSinglePip(&pipPos, pipInfo->convert, pipInfo->shape, frameIndex, clipRect);
	}

	// Draw overload indicator if applicable
	if (CaptureManagerClass* captureManager = techno->CaptureManager)
	{
		bool overloadState = false;
		if (captureManager->IsOverloading(&overloadState))
		{
			Point2D overloadPos {
				.X = drawState->pos.X + pipInfo->maxPips * drawState->spacings.Horizontal,
				.Y = drawState->pos.Y + pipInfo->maxPips * drawState->spacings.Vertical
			};

			DrawSinglePip(&overloadPos, pipInfo->convert, pipInfo->shape, 4 - (int)overloadState, clipRect);
		}
	}
}

// PipScale 1: Ammo display
//using PIPS2_SHP exclusively
static void DrawAmmoPip(TechnoClass* techno, bool isBuilding, SHPStruct* pipShape, PipDrawState* drawState, PipDrawInfo* pipInfo, RectangleStruct* clipRect)
{
	TechnoTypeClass* technoType = techno->GetTechnoType();
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(technoType);

	const Point2D pipSize = pTypeExt->AmmoPipSize.Get((isBuilding ?
		RulesExtData::Instance()->Pips_Ammo_Buildings_Size : RulesExtData::Instance()->Pips_Ammo_Size));

	int pipWrap = technoType->PipWrap;
	ConvertClass* pConvert = pTypeExt->AmmoPip_Palette.GetConvert() ?
		pTypeExt->AmmoPip_Palette.GetConvert()
		: pipInfo->convert;

	auto pSHApe = pTypeExt->AmmoPip_shape.Get(pipShape);

	if (pipWrap > 0)
	{
		int levels = pipInfo->maxPips / pipWrap - 1;

		for (int i = 0; i < pipWrap; i++)
		{
			int frame = CalculateAmmoBarFrame(levels, i, pipWrap, techno->Ammo, pTypeExt->PipWrapAmmoPip);

			Point2D pipPos {
					.X = drawState->pos.X + i * pipSize.X,
					.Y = drawState->pos.Y + i * pipSize.Y - 3
			};

			pipPos += pTypeExt->AmmoPipOffset;

			DrawSinglePip(&pipPos, pConvert, pSHApe, frame, clipRect);
		}
	}
	else
	{
		int remaining = pipInfo->count;
		int ammoFrame = pTypeExt->AmmoPip;
		int emptyFrame = pTypeExt->EmptyAmmoPip;

		for (int i = 0; i < pipInfo->maxPips && remaining > 0; i++)
		{

			int frame = i >= pipInfo->count ? emptyFrame : ammoFrame;

			Point2D pipPos {
				.X = drawState->pos.X + i * drawState->spacigns_.X,
				.Y = drawState->pos.Y + i * drawState->spacigns_.Y - 3
			};

			DrawSinglePip(&pipPos, pConvert, pSHApe, frame, clipRect);
			remaining--;
		}
	}
}

// Calculate the ammo bar frame for wrapped display
static int CalculateAmmoBarFrame(int levels, int i, int pipWrap, int curammo, int defaultEmpty)
{
	int frameIndex = defaultEmpty; // Default empty/neutral frame

	if (levels >= 0)
	{
		int counter = i + pipWrap * levels;
		int frameCounter = levels;
		bool calculateFrame = true;

		while (counter >= curammo)
		{
			frameCounter--;
			counter -= pipWrap;

			if (frameCounter < 0)
			{
				calculateFrame = false;
				break;
			}
		}

		if (calculateFrame)
			frameIndex = frameCounter + defaultEmpty + 1;
	}

	return frameIndex;
}

// Draw passenger pips (for APCs, etc.)
static void DrawPassengerPips(TechnoClass* techno, TechnoTypeClass* technoType, PipDrawState* drawState, PipDrawInfo* pipInfo, RectangleStruct* clipRect)
{
	int maxPips = pipInfo->maxPips;

	if (maxPips <= 0)
	{
		return;
	}

	if (!TunnelFuncs::PopulatePassangerPIPData(techno, technoType, maxPips))
	{
		return;
	}

	if (TunnelFuncs::PipDatas.empty())
	{
		return;
	}

	int startIndex = 0;
	Point2D offset = { 0, 0 };

	if (technoType->Gunner)
	{
		Point2D gunnerPos {
			.X = drawState->pos.X,
			.Y = drawState->pos.Y
		};

		DrawSinglePip(&gunnerPos, pipInfo->convert, pipInfo->shape, TunnelFuncs::PipDatas[0], clipRect);
		startIndex = 1;
		offset.X = 2 * drawState->spacigns_.X;
		offset.Y = 2 * drawState->spacigns_.Y;
	}

	for (int i = startIndex; i < maxPips; i++)
	{
		Point2D pipPos {
			.X = drawState->pos.X + offset.X + (i - startIndex) * drawState->spacigns_.X,
			.Y = drawState->pos.Y + offset.Y + (i - startIndex) * drawState->spacigns_.Y
		};

		DrawSinglePip(&pipPos, pipInfo->convert, pipInfo->shape, TunnelFuncs::PipDatas[i], clipRect);
	}
}

// Draw group number indicator
static void DrawGroupNumber(TechnoClass* techno, AbstractType unitType, Point2D* position, RectangleStruct* clipRect)
{
	int group = techno->Group;

	if (group >= 0 && group <= 9)
	{
		int displayNumber = (group + 1) % 10;
		int offsetY = (unitType == AbstractType::Infantry) ? -36 : -33;

		static fmt::basic_memory_buffer<wchar_t, 10> numberText;
		numberText.clear();

		fmt::format_to(std::back_inserter(numberText), L"{}", displayNumber);
		numberText.push_back(L'\0');

		Point2D textPos {
			.X = position->X - 4,
			.Y = position->Y + offsetY - 3
		};

		// Get text dimensions
		RectangleStruct textRect;
		Drawing::GetTextDimensions(&textRect, numberText.data(), textPos, TextPrintType::FullShadow | TextPrintType::Efnt, 2, -2);

		// Clip to drawing area
		RectangleStruct clippedRect = *clipRect;
		if (textRect.Width <= 0 || textRect.Height <= 0 || clipRect->Width <= 0 || clipRect->Height <= 0)
		{
			return;
		}

		if (clippedRect.X < textRect.X)
		{
			clippedRect.Width += clippedRect.X - textRect.X;
			clippedRect.X = textRect.X;
		}
		if (clippedRect.Width < 1) return;

		if (clippedRect.Y < textRect.Y)
		{
			clippedRect.Height += clippedRect.Y - textRect.Y;
			clippedRect.Y = textRect.Y;
		}
		if (clippedRect.Height < 1) return;

		if (clippedRect.X + clippedRect.Width > textRect.X + textRect.Width)
		{
			clippedRect.Width = textRect.X + textRect.Width - clippedRect.X;
		}
		if (clippedRect.Width < 1) return;

		if (clippedRect.Y + clippedRect.Height > textRect.Y + textRect.Height)
		{
			clippedRect.Height = textRect.Y + textRect.Height - clippedRect.Y;
		}
		if (clippedRect.Height < 1) return;

		// Draw background box
		RectangleStruct boxRect {
			.X = clippedRect.X - 1,
			.Y = clippedRect.Y,
			.Width = clippedRect.Width + 1,
			.Height = clippedRect.Height
		};

		// Get house color
		int color = techno->Owner->Color.ToInit();

		DSurface::Temp->Fill_Rect(boxRect, 0);
		DSurface::Temp->Draw_Rect(boxRect, color);

		// Draw text
		TextDrawing::Simple_Text_Print_Wide(numberText.data(), DSurface::Temp, clipRect, &textPos, color, 0, TextPrintType::FullShadow | TextPrintType::Efnt);
	}
}

// Helper to draw a single pip
static void DrawSinglePip(Point2D* position, ConvertClass* pConvert, SHPStruct* shape, int frameIndex, RectangleStruct* clipRect, BlitterFlags flags)
{
	DSurface::Temp->DrawSHP(
		pConvert,
		shape,
		frameIndex,
		position,
		clipRect,
		flags,
		0, 0, 0,
		1000,
		0, 0, 0, 0, 0
	);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x709A90, FakeTechnoClass::__Draw_Pips)
#pragma endregion
