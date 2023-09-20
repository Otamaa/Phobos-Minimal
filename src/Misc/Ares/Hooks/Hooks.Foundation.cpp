#include <Ext/Building/Body.h>
#include <Ext/Building/Body.h>

#include <Misc/PhobosGlobal.h>

#include "Header.h"

DEFINE_OVERRIDE_HOOK(0x656584, RadarClass_GetFoundationShape, 6)
{
	GET(RadarClass*, pThis, ECX);
	GET(BuildingTypeClass*, pType, EAX);

	const auto fnd = pType->Foundation;
	const DynamicVectorClass<Point2D>* ret = fnd > Foundation::_0x0 ?
		&BuildingTypeExt::ExtMap.Find(pType)->FoundationRadarShape :
		&pThis->FoundationTypePixels[(int)fnd];

	R->EAX(ret);
	return 0x656595;
}

DEFINE_OVERRIDE_HOOK(0x6563B0, RadarClass_UpdateFoundationShapes_Custom, 5)
{
	// update each building type foundation
	for (auto pType : *BuildingTypeClass::Array)
	{
		BuildingTypeExt::ExtMap.Find(pType)->UpdateFoundationRadarShape();
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x568565, MapClass_AddContentAt_Foundation_OccupyHeight, 5)
{
	GET(BuildingClass*, pThis, EDI);
	GET(int, ShadowHeight, EBP);
	GET_STACK(CellStruct*, MainCoords, 0x8B4);

	auto const AffectedCells = CustomFoundation::GetCoveredCells(
		pThis, *MainCoords, ShadowHeight);

	std::for_each(AffectedCells->begin(), AffectedCells->end(), [](CellStruct const& cell) {
		if (auto pCell = MapClass::Instance->TryGetCellAt(cell)) {
			++pCell->OccupyHeightsCoveringMe;
		}
	});

	return 0x568697;
}

DEFINE_OVERRIDE_HOOK(0x568411, MapClass_AddContentAt_Foundation_P1, 0)
{
	GET(BuildingClass*, pThis, EDI);

	R->EBP(pThis->GetFoundationData(false));

	return 0x568432;
}

DEFINE_OVERRIDE_HOOK(0x568841, MapClass_RemoveContentAt_Foundation_P1, 0)
{
	GET(BuildingClass*, pThis, EDI);

	R->EBP(pThis->GetFoundationData(false));

	return 0x568862;
}

DEFINE_OVERRIDE_HOOK(0x568997, MapClass_RemoveContentAt_Foundation_OccupyHeight, 5)
{
	GET(BuildingClass*, pThis, EDX);
	GET(int, ShadowHeight, EBP);
	GET_STACK(CellStruct*, MainCoords, 0x8B4);

	auto const& AffectedCells = CustomFoundation::GetCoveredCells(
		pThis, *MainCoords, ShadowHeight);

	std::for_each(AffectedCells->begin(), AffectedCells->end(), [](CellStruct const& cell) {
		if (auto pCell = MapClass::Instance->TryGetCellAt(cell)) {
			--pCell->OccupyHeightsCoveringMe;
		}
	});

	return 0x568ADC;
}

DEFINE_OVERRIDE_HOOK(0x4A8C77, DisplayClass_ProcessFoundation1_UnlimitBuffer, 5)
{
	GET_STACK(CellStruct const*, Foundation, 0x18);
	GET(DisplayClass*, Display, EBX);

	DWORD Len = CustomFoundation::FoundationLength(Foundation);

	PhobosGlobal::Instance()->TempFoundationData1.assign(Foundation, Foundation + Len);

	Display->CurrentFoundation_Data = PhobosGlobal::Instance()->TempFoundationData1.data();

	auto const bounds = Display->FoundationBoundsSize(
		PhobosGlobal::Instance()->TempFoundationData1.data());

	R->Stack<CellStruct>(0x18, bounds);
	R->EAX<CellStruct*>(R->lea_Stack<CellStruct*>(0x18));

	return 0x4A8C9E;
}

DEFINE_OVERRIDE_HOOK(0x4A8DD7, DisplayClass_ProcessFoundation2_UnlimitBuffer, 5)
{
	GET_STACK(CellStruct const*, Foundation, 0x18);
	GET(DisplayClass*, Display, EBX);

	DWORD Len = CustomFoundation::FoundationLength(Foundation);

	PhobosGlobal::Instance()->TempFoundationData2.assign(Foundation, Foundation + Len);

	Display->CurrentFoundationCopy_Data = PhobosGlobal::Instance()->TempFoundationData2.data();

	auto const bounds = Display->FoundationBoundsSize(
		PhobosGlobal::Instance()->TempFoundationData2.data());

	R->Stack<CellStruct>(0x18, bounds);
	R->EAX<CellStruct*>(R->lea_Stack<CellStruct*>(0x18));

	return 0x4A8DFE;
}

DEFINE_OVERRIDE_HOOK(0x45ec90, BuildingTypeClass_GetFoundationWidth, 6)
{
	GET(BuildingTypeClass*, pThis, ECX);

	if (pThis->Foundation == BuildingTypeExt::CustomFoundation)
	{
		R->EAX(BuildingTypeExt::ExtMap.Find(pThis)->CustomWidth);
		return 0x45EC9D;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x45eca0, BuildingTypeClass_GetFoundationHeight, 6)
{
	GET(BuildingTypeClass*, pThis, ECX);

	if (pThis->Foundation == BuildingTypeExt::CustomFoundation)
	{
		bool bIncludeBib = (R->Stack8(0x4) != 0);

		int fH = BuildingTypeExt::ExtMap.Find(pThis)->CustomHeight;
		if (bIncludeBib && pThis->Bib)
		{
			++fH;
		}

		R->EAX(fH);
		return 0x45ECDA;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x45ECE0, BuildingTypeClass_GetMaxPips, 6)
{
	GET(BuildingTypeClass*, pThis, ECX);

	if (pThis->Foundation == BuildingTypeExt::CustomFoundation)
	{
		R->EAX(BuildingTypeExt::ExtMap.Find(pThis)->CustomWidth);
		return 0x45ECED;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x465550, BuildingTypeClass_GetFoundationOutline, 6)
{
	GET(BuildingTypeClass*, pThis, ECX);

	if (pThis->Foundation == BuildingTypeExt::CustomFoundation)
	{
		R->EAX(BuildingTypeExt::ExtMap.Find(pThis)->OutlineData.data());
		return 0x46556D;
	}

	return 0;
}

DEFINE_DISABLE_HOOK(0x465d4a, BuildingTypeClass_IsUndeployable_ares) //, 6)

DEFINE_OVERRIDE_HOOK(0x464AF0, BuildingTypeClass_GetSizeInLeptons, 6)
{
	GET(BuildingTypeClass*, pThis, ECX);
	if (pThis->Foundation == BuildingTypeExt::CustomFoundation)
	{
		GET_STACK(CoordStruct*, Coords, 0x4);
		const auto pData = BuildingTypeExt::ExtMap.Find(pThis);

		Coords->X = pData->CustomWidth * 256;
		Coords->Y = pData->CustomHeight * 256;
		Coords->Z = BuildingTypeClass::HeightInLeptons * pThis->Height;
		R->EAX(Coords);
		return 0x464B2C;
	}
	return 0;

}


DEFINE_OVERRIDE_HOOK(0x474DEE, INIClass_GetFoundation, 7)
{
	GET_STACK(const char*, Section, 0x2C);
	GET_STACK(const char*, Key, 0x30);
	LEA_STACK(const char*, Value, 0x8);

	if (CRT::strcmpi(Value, "Custom") != 0)
	{
		Debug::INIParseFailed(Section, Key, Value);
	}

	return 0;
}
