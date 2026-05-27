#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/House/Body.h>

#include <Utilities/Macro.h>

#include <CaptureManagerClass.h>
#include <SpawnManagerClass.h>
#include <InfantryClass.h>
#include <InfantryTypeClass.h>

#include <TextDrawing.h>

#include <Misc/PhobosGlobal.h>

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
	TechnoTypeClass* technoTypeClass = GET_TECHNOTYPE(techno);
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

	{
		if (isBuilding) {
			DrawBuildingOccupants((BuildingClass*)techno, &drawState, &pipInfo, clipRect);
		}

		switch (technoTypeClass->PipScale)
		{
		default: { break; }
		case PipScale::Ammo: {
			DrawAmmoPip(techno, isBuilding, pips2_SHP, &drawState, &pipInfo, clipRect);
			break;
		}
		case PipScale::Tiberium: {
			bool showTiberium = true;

			if (isBuilding)
			{
				const auto pBld = static_cast<BuildingClass*>(techno);

				if ((pBld->Type->Refinery || pBld->Type->ResourceDestination) && pBld->Type->Storage > 0)
				{
					// show only if this refinery uses storage. otherwise, the original
					// refineries would show an unused tiberium pip scale
					showTiberium = pTypeExt->Refinery_UseStorage;
				}
			}

			if (showTiberium) {
				Tiberiumpip::DrawTiberiumPip(techno, technoTypeClass, pipInfo.maxPips, pipInfo.shape, pipInfo.convert, &drawState.pos, clipRect, drawState.spacigns_.X, drawState.spacigns_.Y);
			}
			break;
		}
		case PipScale::MindControl: {
			DrawMindControlPip(techno, &drawState, &pipInfo, clipRect);
			break;
		}
		case PipScale::Passengers: {
			if (technoTypeClass->Passengers > 0)
				DrawPassengerPips(techno, technoTypeClass, &drawState, &pipInfo, clipRect);

			break;
		}
		case PipScale::Power: {
			//none
		}
		case PipScale(6):
		{
			DrawSpawnPips(techno, isBuilding, technoTypeClass, &drawState, &pipInfo, clipRect);
			break;
		}
		}
	}

	// Draw self-heal indicator
	TechnoExtData::DrawSelfHealPips(techno, &drawState.pos, clipRect, pips_SHP, pipInfo.convert);

	// Draw info tip for non-buildings
	// techno other than buildings has no extra info atm
	// moved to different state
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
	TechnoTypeClass* technoType = GET_TECHNOTYPE(techno);
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

	if (!HouseExtData::PopulatePassangerPIPData(techno, technoType, maxPips))
	{
		return;
	}

	if (PhobosGlobal::Instance()->PipDatas.empty())
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

		DrawSinglePip(&gunnerPos, pipInfo->convert, pipInfo->shape, PhobosGlobal::Instance()->PipDatas[0], clipRect);
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

		DrawSinglePip(&pipPos, pipInfo->convert, pipInfo->shape, PhobosGlobal::Instance()->PipDatas[i], clipRect);
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
		Drawing::GetTextDimensions(&textRect, numberText.data(), textPos, TextPrintType::FullShadow | TextPrintType::Efnt, Point2D(2, -2));

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
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E26F4, FakeTechnoClass::__Draw_Pips)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E430C, FakeTechnoClass::__Draw_Pips)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E90E4, FakeTechnoClass::__Draw_Pips)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB4A8, FakeTechnoClass::__Draw_Pips)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4DB0, FakeTechnoClass::__Draw_Pips)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F60C0, FakeTechnoClass::__Draw_Pips)
#pragma endregion
