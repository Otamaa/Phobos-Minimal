#include <Ext/Building/Body.h>
#include <Ext/Building/Body.h>

#include <Misc/PhobosGlobal.h>

#include "Header.h"

DEFINE_DISABLE_HOOK(0x465d4a, BuildingTypeClass_IsUndeployable_ares) //, 6)

DEFINE_DISABLE_HOOK(0x43bcbd, BuildingClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x43c022, BuildingClass_DTOR_ares)
DEFINE_DISABLE_HOOK(0x453e20, BuildingClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x45417e, BuildingClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x454190, BuildingClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x454244, BuildingClass_Save_Suffix_ares)

DEFINE_DISABLE_HOOK(0x45e50c, BuildingTypeClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x45e707, BuildingTypeClass_DTOR_ares)
DEFINE_DISABLE_HOOK(0x464a49, BuildingTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x464a56, BuildingTypeClass_LoadFromINI_ares)
DEFINE_DISABLE_HOOK(0x465010, BuildingTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x4652ed, BuildingTypeClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x465300, BuildingTypeClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x46536a, BuildingTypeClass_Save_Suffix_ares)

#ifndef ENABLE_FOUNDATIONHOOK

//since we loading game reaally early , fixup some of the stuffs
DEFINE_HOOK(0x465201, BuildingTypeClass_LoadFromStream_Foundation, 0x6)
{
	GET(BuildingTypeClass*, pThis, ESI);

	pThis->ToTile = 0;
	auto pExt = BuildingTypeExtContainer::Instance.Find(pThis);

	if (pExt->IsCustom && pExt->CustomWidth > 0 && pExt->CustomHeight > 0) {
		
		// if there's custom data, assign it
		pThis->FoundationData = pExt->CustomData.data();
		pThis->FoundationOutside = pExt->OutlineData.data();
	}
	else
	{
		pThis->FoundationData = BuildingTypeClass::FoundationlinesData[(int)pThis->Foundation].Datas;
		pThis->FoundationOutside = BuildingTypeClass::FoundationOutlinesData[(int)pThis->Foundation].Datas;
	}

	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->ToOverlay);
	return 0x465239;
}

DEFINE_OVERRIDE_HOOK(0x45eca0, BuildingTypeClass_GetFoundationHeight, 6)
{
	GET(BuildingTypeClass*, pThis, ECX);

	if (pThis->Foundation == BuildingTypeExtData::CustomFoundation) {
		const bool bIncludeBib = (R->Stack8(0x4) != 0);
		R->EAX(BuildingTypeExtContainer::Instance.Find(pThis)->CustomHeight + (bIncludeBib && pThis->Bib));
		return 0x45ECDA;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x656584, RadarClass_GetFoundationShape, 6)
{
	GET(RadarClass*, pThis, ECX);
	GET(BuildingTypeClass*, pType, EAX);

	const auto fnd = pType->Foundation;
	DWORD* ret = nullptr;

	if (fnd == BuildingTypeExtData::CustomFoundation) {
		const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pType);
		ret = reinterpret_cast<DWORD*>(&pTypeExt->FoundationRadarShape);
	} else if(fnd >= Foundation::_1x1 && fnd <= Foundation::_0x0) {
		ret = reinterpret_cast<DWORD*>(pThis->FoundationTypePixels + (int)fnd);
	} else {
		ret = reinterpret_cast<DWORD*>(pThis->FoundationTypePixels + (int)Foundation::_2x2);
	}

	R->EAX(ret);
	return 0x656595;
}

DEFINE_OVERRIDE_HOOK(0x6563B0, RadarClass_UpdateFoundationShapes_Custom, 5)
{
	// update each building type foundation
	for (auto pType : *BuildingTypeClass::Array)
	{
		BuildingTypeExtContainer::Instance.Find(pType)->UpdateFoundationRadarShape();
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

	for(const auto& cell : *AffectedCells) {
		if (auto pCell = MapClass::Instance->TryGetCellAt(cell)) {
			++pCell->OccupyHeightsCoveringMe;
		}
	}

	return 0x568697;
}

DEFINE_OVERRIDE_HOOK(0x568411, MapClass_AddContentAt_Foundation_P1, 6)
{
	GET(BuildingClass*, pThis, EDI);
	R->EBP(pThis->GetFoundationData(false));
	return 0x568432;
}

DEFINE_OVERRIDE_HOOK(0x568841, MapClass_RemoveContentAt_Foundation_P1, 6)
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

	auto const AffectedCells = CustomFoundation::GetCoveredCells(
		pThis, *MainCoords, ShadowHeight);

	for (const auto& cell : *AffectedCells) {
		if (auto pCell = MapClass::Instance->TryGetCellAt(cell)) {
			if (pCell->OccupyHeightsCoveringMe > 0)
				--pCell->OccupyHeightsCoveringMe;
		}
	}

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

	if (pThis->Foundation == BuildingTypeExtData::CustomFoundation)
	{
		R->EAX(BuildingTypeExtContainer::Instance.Find(pThis)->CustomWidth);
		return 0x45EC9D;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x45ECE0, BuildingTypeClass_GetMaxPips, 6)
{
	GET(BuildingTypeClass*, pThis, ECX);

	if (pThis->Foundation == BuildingTypeExtData::CustomFoundation)
	{
		R->EAX(BuildingTypeExtContainer::Instance.Find(pThis)->CustomWidth);
		return 0x45ECED;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x465550, BuildingTypeClass_GetFoundationOutline, 6)
{
	GET(BuildingTypeClass*, pThis, ECX);

	if (pThis->Foundation == BuildingTypeExtData::CustomFoundation)
	{
		R->EAX(BuildingTypeExtContainer::Instance.Find(pThis)->OutlineData.data());
		return 0x46556D;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x464AF0, BuildingTypeClass_GetSizeInLeptons, 6)
{
	GET(BuildingTypeClass*, pThis, ECX);
	if (pThis->Foundation == BuildingTypeExtData::CustomFoundation)
	{
		GET_STACK(CoordStruct*, Coords, 0x4);
		const auto pData = BuildingTypeExtContainer::Instance.Find(pThis);

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

	if (!IS_SAME_STR_(Value, "Custom")) {
		Debug::INIParseFailed(Section, Key, Value);
	}

	return 0;
}
#endif