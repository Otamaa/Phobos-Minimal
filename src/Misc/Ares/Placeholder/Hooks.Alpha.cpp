#include <AlphaShapeClass.h>
#include <TacticalClass.h>

#include <ObjectClass.h>
#include <CellClass.h>
#include <BuildingClass.h>

#include <Ext/Anim/Body.h>

#include <FootClass.h>

#include <Misc/AresData.h>

#include <Misc/PhobosGlobal.h>

/*
*	Otamaa : Nedd storage for `AlphaShapeClass` pointer for later check
*			 ObjectClass 0x28 seems used by something also
*			 Ext cant be implemented without changing everything too ..
*/
#pragma warning( push )
#pragma warning (disable : 4245)
#pragma warning (disable : 4838)
//DEFINE_DISABLE_HOOK(0x5F3D65, ObjectClass_DTOR_ares)
//DEFINE_DISABLE_HOOK(0x421730, AlphaShapeClass_SDDTOR_ares)
//DEFINE_DISABLE_HOOK(0x420960, AlphaShapeClass_CTOR_ares)
#pragma warning( pop )

DEFINE_OVERRIDE_HOOK(0x420960, AlphaShapeClass_CTOR, 5)
{
	GET_STACK(ObjectClass*, pSource, 0x4);
	GET(AlphaShapeClass*, pAlpha, ECX);

	if (auto pOldAlpha = PhobosGlobal::Instance()->ObjectLinkedAlphas.get_or_default(pSource))
	{
		GameDelete(pOldAlpha);
		// pSource is erased from map
	}

	PhobosGlobal::Instance()->ObjectLinkedAlphas[pSource] = pAlpha;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x421730, AlphaShapeClass_SDDTOR, 8)
{
	GET(AlphaShapeClass*, pAlpha, ECX);
	PhobosGlobal::Instance()->ObjectLinkedAlphas.erase(pAlpha->AttachedTo);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5F3D65, ObjectClass_DTOR, 6)
{
	GET(ObjectClass*, pThis, ESI);

	if (auto pAlpha = PhobosGlobal::Instance()->ObjectLinkedAlphas.get_or_default(pThis)) {
		GameDelete(pAlpha);
		// pThis is erased from map
	}

	return 0;
}

void NOINLINE UpdateObjectAlpha(ObjectClass* pSource)
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
	const RectangleStruct& ScreenArea = TacticalClass::Instance->VisibleArea();
	Point2D off = { ScreenArea.X - (pImage->Width / 2), ScreenArea.Y - (pImage->Height / 2) };
	Point2D xy;

	if (AlphaShapeClass* pAlpha_ = PhobosGlobal::Instance()->ObjectLinkedAlphas.get_or_default(pSource))
	{
		if (pSource->InLimbo || !pSource->IsAlive)
		{
			GameDelete(pAlpha_);
			return;
		}

		// for animations attached to the owner object, consider
		// the owner object as source, so the display is refreshed
		// whenever the owner object moves.
		auto pOwner = pSource;
		if (auto pAnim = specific_cast<AnimClass*>(pSource))
		{
			if (pAnim->OwnerObject)
			{
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

		if (auto pTechno = abstract_cast<TechnoClass*>(pSource))
		{
			if(pTechno->Deactivated)
			{
				GameDelete(pAlpha_);
				return;
			}

			if (auto pBld = specific_cast<BuildingClass*>(pSource))
			{
				if (pBld->GetCurrentMission() != Mission::Construction && !pBld->IsPowerOnline())
				{
					GameDelete(pAlpha_);
					return;
				}
			}
		}


	}
	else
	{
		if (pSource->InLimbo || !pSource->IsAlive)
			return;

		if ((Unsorted::CurrentFrame % 2))
		{ // lag reduction - don't draw a new alpha every frame
			XYZ = pSource->GetCoords();
			TacticalClass::Instance->CoordsToClient(&XYZ, &xy);
			xy += off;
			++Unsorted::ScenarioInit;
			GameCreate<AlphaShapeClass>(pSource, xy);
			--Unsorted::ScenarioInit;
			//int Margin = 40;
			RectangleStruct Dirty = { xy.X - ScreenArea.X, xy.Y - ScreenArea.Y, pImage->Width, pImage->Height };
			TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
		}
	}
}

DEFINE_OVERRIDE_HOOK(0x5F3E70, ObjectClass_Update_AlphaLight, 5)
{
	GET(ObjectClass*, pThis, ECX);
	UpdateObjectAlpha(pThis);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x423B0B, AnimClass_Update_AlphaLight, 6)
{
	GET(AnimClass*, pThis, ESI);
	// flaming guys do the update via base class
	if (!pThis->Type->IsFlamingGuy)
	{
		UpdateObjectAlpha(pThis);
	}

	return 0;
}