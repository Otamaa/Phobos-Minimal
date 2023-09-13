#include <Ext/Building/Body.h>
#include <Ext/Building/Body.h>

#include <Misc/PhobosGlobal.h>

DWORD FoundationLength(CellStruct const* const pFoundation)
{
	auto pFCell = pFoundation;
	while (*pFCell != BuildingTypeExt::FoundationEndMarker)
	{
		++pFCell;
	}

	// include the end marker
	return static_cast<DWORD>(std::distance(pFoundation, pFCell + 1));
}

const std::vector<CellStruct>& GetCoveredCells(
	BuildingClass* const pThis, CellStruct const mainCoords,
	int const shadowHeight)
{
	auto const pFoundation = pThis->GetFoundationData(false);
	auto const len = FoundationLength(pFoundation);

	PhobosGlobal::Instance()->TempCoveredCellsData.clear();
	PhobosGlobal::Instance()->TempCoveredCellsData.reserve(len * shadowHeight);

	auto pFCell = pFoundation;

	while (*pFCell != BuildingTypeExt::FoundationEndMarker)
	{
		auto actualCell = mainCoords + *pFCell;
		for (auto i = shadowHeight; i > 0; --i)
		{
			PhobosGlobal::Instance()->TempCoveredCellsData.push_back(actualCell);
			--actualCell.X;
			--actualCell.Y;
		}
		++pFCell;
	}

	std::sort(PhobosGlobal::Instance()->TempCoveredCellsData.begin(),
			  PhobosGlobal::Instance()->TempCoveredCellsData.end(),
		[](const CellStruct& lhs, const CellStruct& rhs) -> bool
 {
	 return lhs.X > rhs.X || lhs.X == rhs.X && lhs.Y > rhs.Y;
		});

	auto const it = std::unique(
		PhobosGlobal::Instance()->TempCoveredCellsData.begin(),
		PhobosGlobal::Instance()->TempCoveredCellsData.end());

	PhobosGlobal::Instance()->TempCoveredCellsData.erase(it, PhobosGlobal::Instance()->TempCoveredCellsData.end());

	return PhobosGlobal::Instance()->TempCoveredCellsData;
}

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
	for (auto const& pType : *BuildingTypeClass::Array)
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

	auto const& AffectedCells = GetCoveredCells(
		pThis, *MainCoords, ShadowHeight);

	auto& Map = MapClass::Instance;

	for (auto const& cell : AffectedCells)
	{
		if (auto pCell = Map->TryGetCellAt(cell))
		{
			++pCell->OccupyHeightsCoveringMe;
		}
	}

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

	auto const& AffectedCells = GetCoveredCells(
		pThis, *MainCoords, ShadowHeight);

	auto& Map = MapClass::Instance;

	for (auto const& cell : AffectedCells)
	{
		if (auto const pCell = Map->TryGetCellAt(cell))
		{
			if (pCell->OccupyHeightsCoveringMe > 0)
			{
				--pCell->OccupyHeightsCoveringMe;
			}
		}
	}

	return 0x568ADC;
}

DEFINE_OVERRIDE_HOOK(0x4A8C77, DisplayClass_ProcessFoundation1_UnlimitBuffer, 5)
{
	GET_STACK(CellStruct const*, Foundation, 0x18);
	GET(DisplayClass*, Display, EBX);

	DWORD Len = FoundationLength(Foundation);

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

	DWORD Len = FoundationLength(Foundation);

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

DEFINE_OVERRIDE_HOOK(0x465d4a, BuildingTypeClass_IsUndeployable, 6)
{
	GET(BuildingTypeClass*, pThis, ECX);

	if (pThis->Foundation != BuildingTypeExt::CustomFoundation)
		return 0;

	const auto pData = BuildingTypeExt::ExtMap.Find(pThis);
	R->EAX(pData->CustomHeight == 1 && pData->CustomWidth == 1);
	return 0x465D6D;
}

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
