#include <AlphaShapeClass.h>
#include <TacticalClass.h>

#include <ObjectClass.h>
#include <CellClass.h>
#include <BuildingClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>

#include <FootClass.h>

#include <Misc/AresData.h>

#include <Misc/PhobosGlobal.h>

void UpdateAlphaShape(ObjectClass* pSource)
{
	ObjectTypeClass* pSourceType = pSource->GetType();
	if (!pSourceType) {
		return;
	}

	const SHPStruct* pImage = pSourceType->AlphaImage;
	if (!pImage) {
		return;
	}

	CoordStruct XYZ;

	RectangleStruct ScreenArea = TacticalClass::Instance->VisibleArea();
	Point2D off = { ScreenArea.X - (pImage->Width / 2), ScreenArea.Y - (pImage->Height / 2) };
	Point2D xy;

	// for animations attached to the owner object, consider
	// the owner object as source, so the display is refreshed
	// whenever the owner object moves.
	auto pOwner = pSource;
	if (auto pAnim = specific_cast<AnimClass*>(pSource)) {
		if (pAnim->OwnerObject) {
			pOwner = pAnim->OwnerObject;
		}
	}

	if (auto pFoot = abstract_cast<FootClass*>(pOwner))
	{
		if (pFoot->LastMapCoords != pFoot->CurrentMapCoords)
		{
			// we moved - need to redraw the area we were in
			// alas, we don't have the precise XYZ we were in, only the cell we were last seen in
			// so we need to add the cell's dimensions to the dirty area just in case
			XYZ = CellClass::Cell2Coord(pFoot->LastMapCoords);
			Point2D xyTL, xyBR;
			TacticalClass::Instance->CoordsToClient(&XYZ, &xyTL);
			// because the coord systems are different - xyz is x/, y\, xy is x-, y|
			XYZ.X += 256;
			XYZ.Y += 256;
			TacticalClass::Instance->CoordsToClient(&XYZ, &xyBR);
			Point2D cellDimensions = xyBR - xyTL;
			xy = xyTL;
			xy.X += cellDimensions.X / 2;
			xy.Y += cellDimensions.Y / 2;
			xy += off;
			RectangleStruct Dirty = { xy.X - ScreenArea.X - cellDimensions.X,
				xy.Y - ScreenArea.Y - cellDimensions.Y, pImage->Width + cellDimensions.X * 2,
				pImage->Height + cellDimensions.Y * 2 };
			TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
		}
	}

	bool Inactive = pSource->InLimbo;

	if (auto pTechno = abstract_cast<TechnoClass*>(pSource)) {
		Inactive = Inactive || pTechno->Deactivated;
		if (pSource->WhatAmI() == BuildingClass::AbsID) {
			if (pTechno->GetCurrentMission() != Mission::Construction) {
				Inactive = Inactive || !pTechno->IsPowerOnline();
			}
		}
	}

	if (Inactive)
	{
		if (auto pAlpha = PhobosGlobal::Instance()->ObjectLinkedAlphas.get_or_default(pSource))
		{
			GameDelete<true, false>(pAlpha);
			// pSource is erased from map
		}
		return;
	}

	if (Unsorted::CurrentFrame % 2)
	{ // lag reduction - don't draw a new alpha every frame
		XYZ = pSource->GetCoords();
		TacticalClass::Instance->CoordsToClient(&XYZ, &xy);
		xy += off;
		++Unsorted::ScenarioInit;
		GameCreate<AlphaShapeClass>(pSource, xy.X, xy.Y);
		--Unsorted::ScenarioInit;
		//int Margin = 40;
		RectangleStruct Dirty = { xy.X - ScreenArea.X, xy.Y - ScreenArea.Y, pImage->Width, pImage->Height };
		TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
	}
}


DEFINE_OVERRIDE_HOOK(0x421730, AlphaShapeClass_SDDTOR, 8)
{
	GET(AlphaShapeClass*, pAlpha, ECX);
	PhobosGlobal::Instance()->ObjectLinkedAlphas.erase(pAlpha->AttachedTo);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x420960, AlphaShapeClass_CTOR, 5)
{
	GET_STACK(ObjectClass*, pSource, 0x4);
	GET(AlphaShapeClass*, pThis, ECX);

	if (auto pOldAlpha = PhobosGlobal::Instance()->ObjectLinkedAlphas.get_or_default(pSource))
	{
		GameDelete<true, false>(pOldAlpha);
	}

	PhobosGlobal::Instance()->ObjectLinkedAlphas[pSource] = pThis;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5F3D65, ObjectClass_DTOR, 6)
{
	GET(ObjectClass*, pThis, ESI);

	if (auto pAlpha = PhobosGlobal::Instance()->ObjectLinkedAlphas.get_or_default(pThis))
	{
		GameDelete<true, false>(pAlpha);
		// pThis is erased from map
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5F3E70, ObjectClass_Update_AlphaLight, 5)
{
	GET(ObjectClass*, pThis, ECX);
	UpdateAlphaShape(pThis);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x423B0B, AnimClass_Update_AlphaLight, 6)
{
	GET(AnimClass*, pThis, ESI);
	// flaming guys do the update via base class
	if (!pThis->Type->IsFlamingGuy)
	{
		UpdateAlphaShape(pThis);
	}

	return 0;
}