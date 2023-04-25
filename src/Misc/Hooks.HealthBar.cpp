
#include <Utilities/Macro.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Utilities/Helpers.h>
#include <Utilities/Cast.h>

DEFINE_HOOK(0x709ACF, TechnoClass_DrawPip_PipShape1_A, 0x6)
{
	GET(TechnoClass*, pThis, EBP);
	GET(SHPStruct*, pPipShape01, ECX);

	R->ECX(TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->PipShapes01.Get(pPipShape01));

	return 0;
}

DEFINE_HOOK(0x709AE3, TechnoClass_DrawPip_PipShape1_B, 0x6)
{
	GET(TechnoClass*, pThis, EBP);
	GET(SHPStruct*, pPipShape01, EAX);

	R->EAX(TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->PipShapes01.Get(pPipShape01));

	return 0;
}

DEFINE_HOOK(0x709AF8, TechnoClass_DrawPip_PipShape2, 0x6)
{
	GET(TechnoClass*, pThis, EBP);
	GET(SHPStruct*, pPipShape02, EBX);

	R->EBX(TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->PipShapes02.Get(pPipShape02));

	return 0;
}

//6F6759 , EDX ,6F675F
DEFINE_HOOK(0x6F6722, TechnoClass_DrawHealth_Building_PipFile_B, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	const auto pThisExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	R->EDX(pThisExt->PipShapes01.Get(FileSystem::PIPS_SHP()));

	return 0x6F6728;
}

DEFINE_HOOK(0x6F6759, TechnoClass_DrawHealth_Building_PipFile_B_pal, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	const auto pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
	ConvertClass* nPal = nullptr;

	if (pBuildingTypeExt->PipShapes01Remap)
		nPal = pThis->GetRemapColour();
	else
		nPal = pBuildingTypeExt->PipShapes01Palette.GetOrDefaultConvert(FileSystem::THEATER_PAL());

	R->EDX(nPal);
	return 0x6F675F;
}

DEFINE_HOOK(0x6F66B3, TechnoClass_DrawHealth_Building_PipFile_A, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(SHPReference*, pDefaultPip, EAX);

	const auto pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
	ConvertClass* nPal = nullptr;

	if (pBuildingTypeExt->PipShapes01Remap)
		nPal = pThis->GetRemapColour();
	else
		nPal = pBuildingTypeExt->PipShapes01Palette.GetOrDefaultConvert(FileSystem::THEATER_PAL());

	//PipShapes01Palette
	R->EDX(nPal);//
	R->EAX(pBuildingTypeExt->Type->PipShapes01.Get(pDefaultPip));

	return 0x6F66B9;
}

namespace DrawHeathData
{
	void DrawNumber(const TechnoClass* const pThis, Point2D* pLocation, RectangleStruct* pBounds)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (!pTypeExt->HealthNumber_Show.Get() || !pThis->IsAlive)
			return;

		auto const pShpGreen = pTypeExt->HealthNumber_SHP.Get(nullptr);

		if (!pShpGreen)
			return;

		Point2D nCurrentDistance { 0,0 };
		auto const nLocation = *pLocation;
		auto const pType = pThis->GetTechnoType();
		int XOffset = 0;
		int	YOffset = 0;

		auto GetFrame = [pThis, pShpGreen](char nInput)
		{
			int nFrameResult = -1;
			int nInputToFrame = -1;

			switch (nInput)
			{
			case (*" "):
				nInputToFrame = 12;//blank frame
				break;
			case (*"%"):
				nInputToFrame = 10;
				break;
			case (*"/"):
				nInputToFrame = 11;
				break;
			default:
				nInputToFrame = nInput - 48;
				break;
			}

			int const nFrame_Total = std::clamp((int)pShpGreen->Frames, 12 + 1, 36 + 1);
			nFrameResult = nInputToFrame;

			// blank frames on the end (+1) !
			// 0  1  2  3  4  5  6  7  8  9 10 11

			// 12 13 14 15 16 17 18 19 20 21 22	23
			if (nFrame_Total == 25 && (pThis->IsYellowHP() || pThis->IsRedHP()))
			{
				nFrameResult = nInputToFrame + 12;
			}
			// 24 25 26 27 28 29 30 31 32 33 34 35
			else if (nFrame_Total == 37)
			{
				if (!pThis->IsGreenHP())
				{
					if (!(nInputToFrame == 12) && pThis->IsYellowHP())
						nFrameResult = nInputToFrame + 12;

					nFrameResult = nInputToFrame + 24;
				}
			}

			return nFrameResult;
		};

		char nBuffer[0x100];

		if (!pTypeExt->HealthNumber_Percent.Get())
			IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "%d/%d", pThis->Health, pThis->GetTechnoType()->Strength);
		else
			IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "%d%s", (int)(pThis->GetHealthPercentage() * 100.0), "%");

		auto const bIsBuilding = Is_Building(pThis);

		{
			// coord calculation is not really right !
			if (bIsBuilding)
			{
				auto const pBuilding = static_cast<const BuildingClass*>(pThis);
				auto const pBldType = pBuilding->Type;
				CoordStruct nDimension { 0,0,0 };
				auto const nLocTemp = nLocation;
				pBldType->Dimension2(&nDimension);
				CoordStruct nDimension2 { -nDimension.X / 2,nDimension.Y / 2,nDimension.Z / 2 };
				Point2D nDest {};
				TacticalClass::Instance->CoordsToScreen(&nDest, &nDimension2);

				XOffset = nDest.X + nLocTemp.X + pTypeExt->Healnumber_Offset.Get().X + 2;
				YOffset = nDest.Y + nLocTemp.Y + pTypeExt->Healnumber_Offset.Get().Y + pType->PixelSelectionBracketDelta;
			}
			else
			{
				XOffset = nLocation.X + pTypeExt->Healnumber_Offset.Get().X + 2 - 15;
				YOffset = nLocation.Y + pTypeExt->Healnumber_Offset.Get().Y + pType->PixelSelectionBracketDelta - 33;
			}
		}

		Point2D nDistanceFactor = pTypeExt->Healnumber_Decrement.Get(bIsBuilding ? Point2D { 10,-5 } : Point2D { 5 ,0 });

		for (auto const& nCurrentData : nBuffer)
		{
			if (nCurrentData == *"\0")
				break;
			Point2D nOffset { nCurrentDistance.X + XOffset,YOffset + nCurrentDistance.Y };

			{
				auto const nFrameIndex = GetFrame(nCurrentData);

				if ((nFrameIndex >= 0) && Phobos::Otamaa::ShowHealthPercentEnabled)
				{
					DSurface::Temp->DrawSHP
					(FileSystem::PALETTE_PAL,
						pShpGreen,
						nFrameIndex,
						&nOffset,
						pBounds,
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
				}
			}

			nCurrentDistance.Y += nDistanceFactor.Y;
			nCurrentDistance.X += nDistanceFactor.X;
		}
	}

	void DrawBar(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBound)
	{
		auto const&[pType , pOwner]= TechnoExt::GetDisguiseType(pThis, false, true);
		LightConvertClass* pTechConvert = pThis->GetRemapColour();
		const bool bIsInfantry = Is_Infantry(pThis);
		bool IsDisguised = false;

		if (pThis->IsDisguised() && !pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer)) {
			IsDisguised = true;
		}

		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		const auto pPipsShape = pTypeExt->HealthBarSHP.Get(FileSystem::PIPS_SHP());
		const auto pPipsShapeSelected = pTypeExt->HealthBarSHP_Selected.Get(FileSystem::PIPBRD_SHP());
		const auto pPalette = pTypeExt->HealthbarRemap.Get() && pTechConvert ? pTechConvert :
			pTypeExt->HealthBarSHP_Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL());

		Point2D nLocation = *pLocation;
		nLocation += pTypeExt->HealthBarSHP_PointOffset.Get();
		const auto nBracketDelta = pType->PixelSelectionBracketDelta + pTypeExt->HealthBarSHPBracketOffset.Get();
		Point2D nPoint { 0,0 };
		const Point2D DrawOffset { 2,0 };

		if (pThis->IsSelected)
		{
			nPoint.X = nLocation.X + (bIsInfantry ? 11 : 1);
			nPoint.Y = nLocation.Y + nBracketDelta - (bIsInfantry ? 25 : 26);

			DSurface::Temp->DrawSHP(pPalette, pPipsShapeSelected, (bIsInfantry ? 1 : 0), &nPoint, pBound, BlitterFlags(0xE00), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
			TechnoExt::DrawSelectBrd(pThis, pType, bIsInfantry ? 8 : 17, pLocation, pBound, bIsInfantry, IsDisguised);
		}

		const int nOffsetX = (bIsInfantry ? -5 : -15);
		const int nLength = (bIsInfantry ? 8 : 17);
		const int nYDelta = nBracketDelta - (bIsInfantry ? 24 : 25);
		const int nDraw = pThis->IsAlive ? std::clamp((int)(std::round(pThis->GetHealthPercentage() * nLength)), 1, nLength) : 0;
		CoordStruct const nHealthFrame = pTypeExt->HealthBarSHP_HealthFrame.Get();
		int nHealthFrameResult = nHealthFrame.Y; //Green

		if (pThis->IsYellowHP())
			nHealthFrameResult = nHealthFrame.Z; //Yellow

		if (pThis->IsRedHP() || pThis->Health <= 0 || !pThis->IsAlive)
			nHealthFrameResult = nHealthFrame.X;//Red

		for (int i = 0; i < nDraw; ++i)
		{
			nPoint.Y = nYDelta + nLocation.Y + DrawOffset.Y * i;
			nPoint.X = nOffsetX + nLocation.X + DrawOffset.X * i;
			DSurface::Temp->DrawSHP(pPalette, pPipsShape, nHealthFrameResult, &nPoint, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

	int DrawBar_PipAmount(TechnoClass* pThis, int iLength)
	{
		const auto passed = Unsorted::CurrentFrame - pThis->IronCurtainTimer.StartTime;
		const auto left = static_cast<double>(pThis->IronCurtainTimer.TimeLeft - passed);
		const auto nTime = (left / pThis->IronCurtainTimer.TimeLeft);

		return std::clamp((int)std::round(nTime * iLength), 0, iLength);
	}

	void DrawBar_Building(TechnoClass* pThis, int iLength, Point2D* pLocation, RectangleStruct* pBound, int frame, int empty_frame, int bracket_delta)
	{
		CoordStruct vCoords = { 0, 0, 0 };
		pThis->GetTechnoType()->Dimension2(&vCoords);
		Point2D vPos2 = { 0, 0 };
		CoordStruct vCoords2 = { -vCoords.X / 2, vCoords.Y / 2,vCoords.Z };
		TacticalClass::Instance->CoordsToScreen(&vPos2, &vCoords2);

		Point2D vLoc = *pLocation;
		vLoc.X -= 5;
		vLoc.Y -= 3 + bracket_delta;

		Point2D vPos = { 0, 0 };

		const int iTotal = DrawBar_PipAmount(pThis, iLength);

		if (iTotal > 0)
		{
			int frameIdx, deltaX, deltaY;
			for (frameIdx = iTotal, deltaX = 0, deltaY = 0;
				frameIdx;
				frameIdx--, deltaX += 4, deltaY -= 2)
			{
				vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
				vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;

				DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
					frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
			}
		}

		if (iTotal < iLength)
		{
			int frameIdx, deltaX, deltaY;
			for (frameIdx = iLength - iTotal, deltaX = 4 * iTotal, deltaY = -2 * iTotal;
				frameIdx;
				frameIdx--, deltaX += 4, deltaY -= 2)
			{
				vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
				vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;
				DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
					empty_frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
			}
		}
	}

	void DrawdBar_Other(TechnoClass* pThis, int iLength, Point2D* pLocation, RectangleStruct* pBound, int aframe, int bracket_delta)
	{
		const auto pPipsShape = FileSystem::PIPS_SHP();
		const auto pPipsShapeSelected = FileSystem::PIPBRD_SHP();

		Point2D vPos = { 0,0 };
		Point2D nLoc = *pLocation;
		Point2D vLoc = *pLocation;
		int frame, XOffset, YOffset;
		YOffset = pThis->GetTechnoType()->PixelSelectionBracketDelta + bracket_delta;
		vLoc.Y -= 5;

		if (iLength == 8)
		{
			vPos.X = vLoc.X + 11;
			vPos.Y = vLoc.Y - 25 + YOffset;
			frame = pPipsShapeSelected->Frames > 2 ? 3 : 1;
			XOffset = -5;
			YOffset -= 24;
		}
		else
		{
			vPos.X = vLoc.X + 1;
			vPos.Y = vLoc.Y - 26 + YOffset;
			frame = pPipsShapeSelected->Frames > 2 ? 2 : 0;
			XOffset = -15;
			YOffset -= 25;
		}

		if (pThis->IsSelected)
		{
			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, pPipsShapeSelected,
				frame, &vPos, pBound, BlitterFlags(0xE00), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}

		const auto iTotal = DrawBar_PipAmount(pThis, iLength);

		for (int i = 0; i < iTotal; ++i)
		{
			vPos.X = nLoc.X + XOffset + 2 * i;
			vPos.Y = nLoc.Y + YOffset;

			DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, pPipsShape,
				aframe, &vPos, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

	void DrawIronCurtaindBar(TechnoClass* pThis, int iLength, Point2D* pLocation, RectangleStruct* pBound)
	{
		if (pThis->IsIronCurtained())
		{
			if (Is_Building(pThis))
				DrawBar_Building(pThis, iLength, pLocation, pBound, pThis->ForceShielded != 1 ? 2 : 3, 0, 0);
			else
				DrawdBar_Other(pThis, iLength, pLocation, pBound, 18, 0);
		}
	}

	void DrawIronTemporalEraseDelayBarAndNumber(TechnoClass* pThis, int iLength, Point2D* pLocation, RectangleStruct* pBound)
	{
		auto icur = pThis->TemporalTargetingMe->WarpRemaining;
		auto imax = pThis->TemporalTargetingMe->GetWarpPerStep();
		//draw it here ? 0x71A88D
	}

}

DEFINE_HOOK(0x6F65D1, TechnoClass_DrawdBar_Building, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, iLength, EBX);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x4C, -0x4));
	GET_STACK(RectangleStruct*, pBound, STACK_OFFS(0x4C, -0x8));

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (const auto pShieldData = pExt->Shield.get()) {
		if (pShieldData->IsAvailable())
			pShieldData->DrawShieldBar(iLength, pLocation, pBound);
	}

	DrawHeathData::DrawNumber(pThis, pLocation, pBound);
	//DrawHeathData::DrawIronCurtaindBar(pThis, iLength, pLocation, pBound);
	return 0;
}

DEFINE_HOOK(0x6F683C, TechnoClass_DrawBar_Foot, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x4C, -0x4));
	GET_STACK(RectangleStruct*, pBound, STACK_OFFS(0x4C, -0x8));


	const int iLength = Is_Infantry(pThis) ? 8 : 17;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (const auto pShieldData = pExt->Shield.get()) {
		if (pShieldData->IsAvailable()) {
			pShieldData->DrawShieldBar(iLength, pLocation, pBound);
		}
	}

	DrawHeathData::DrawBar(pThis, pLocation, pBound);
	DrawHeathData::DrawNumber(pThis, pLocation, pBound);
	//DrawHeathData::DrawIronCurtaindBar(pThis, iLength, pLocation, pBound);

	return 0x6F6A58;
}
