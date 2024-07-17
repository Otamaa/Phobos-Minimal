#include "Body.h"
#include <Ext/AnimType/Body.h>

// Draw Tiled !
#ifndef UsePhobosOne
//ConvertClass* Convert = nullptr;
//DEFINE_HOOK(0x42366D, AnimClass_DrawIt_Tiled_FetchBeforeChange, 0x6) {
//	GET_STACK(ConvertClass*, pConvert, 0x110 - 0xE0);
//	Convert = pConvert;
//	return 0x0;
//
//}

DEFINE_HOOK(0x4236F0, AnimClass_DrawIt_Tiled_Palette, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	if (const auto pCustom = AnimTypeExtContainer::Instance.Find(pThis->Type)->Palette) {
		R->EDX(pCustom->GetConvert<PaletteManager::Mode::Temperate>());
		return 0x4236F6;
	}

	R->EDX(FileSystem::ANIM_PAL());
	return 0x4236F6;
}
#else
DEFINE_HOOK(0x4236A7, AnimClass_Draw_Tiled_CustomPalette, 0x6) //was A
{
	GET(AnimClass* const, pThis, ESI);
	GET(int, nY_Loc, EDI);
	GET(int, nYadd_Loc, EBP);
	GET(BlitterFlags, nFlags, EBX);
	GET_STACK(int, nFrame, STACK_OFFS(0x110, 0xE4));
	LEA_STACK(Point2D*, nPoint, STACK_OFFS(0x110, 0xE0));
	GET_STACK(int, nSHPHeight, STACK_OFFS(0x110, 0xF0));
	GET_STACK(SHPStruct*, pShp, STACK_OFFS(0x110, 0xE8));
	GET_STACK(int, nX_Loc, STACK_OFFS(0x110, 0xD0));
	GET_STACK(int, nTintColor, STACK_OFFS(0x110, 0xF4));
	GET_STACK(int, nBrightness, STACK_OFFS(0x110, 0xD8));

	const auto pTypeExt = AnimTypeExtContainer::Instance.TryFind(pThis->Type);
	if (!pTypeExt)
		return 0x0;

	if (!pShp)
		return 0x42371B;

	const auto pPal = pTypeExt->Palette.GetOrDefaultConvert(FileSystem::ANIM_PAL());
	auto Y_Doffs = pThis->Type->YDrawOffset;

	for (; nYadd_Loc >= 0;)
	{
		nPoint->X = nX_Loc;
		nPoint->Y = nYadd_Loc + Y_Doffs;

		DSurface::Temp->DrawSHP(
		pPal,
		pShp,
		nFrame,
		nPoint,
		&Drawing::SurfaceDimensions_Hidden(),
		nFlags,
		0,
		nY_Loc,
		ZGradient::Deg90,
		nBrightness,
		nTintColor,
		0,
		0,
		0,
		0);

		nYadd_Loc -= nSHPHeight;
		nY_Loc -= nSHPHeight + (nSHPHeight / 2);
		nPoint->Y = nYadd_Loc + Y_Doffs;
	}
#ifdef Other_Impl_AnimPal
	do
	{
		DSurface::Temp->DrawSHP(
			pPal,
			pShp,
			nFrame,
			nPoint,
			&Drawing::SurfaceDimensions_Hidden(),
			nFlags,
			0,
			nY_Loc,
			ZGradient::Deg90,
			nBrightness,
			nTintColor,
			0,
			0,
			0,
			0);

		nYadd_Loc -= nSHPHeight;
		nY_Loc -= nSHPHeight + nSHPHeight / 2;
		nPoint->Y = nYadd_Loc + Y_Doffs;
	}
	while (nYadd_Loc >= 0);
#endif
	return 0x42371B;
}
#endif

#ifdef OVERRIDE_SHP_DRAWING

struct DummyStructForDrawing
{
	static void Callme(
		Surface* dest_surface,
		ConvertClass* drawer,
		SHPReference* shape,
		int shapenum,
		Point2D* xy,
		RectangleStruct* rect1,
		BlitterFlags flags,
		int drawerval,
		int height_offset,
		int somearrayindex,
		int intensity,
		int useotherblitterset,
		SHPStruct* z_shape,
		int z_shape_frame,
		int shape_x_offset,
		int shape_y_offset)
	{
		SHPReference* v16 = nullptr;
		BSurface* v35 = nullptr;
		RectangleStruct z_shape_rect { 0,0,0,0 };

		if (shape)
		{
			if (shape->Type == -1)
			{
				if (!shape->Loaded)
				{
					shape->Load();
				}
				v16 = shape->AsReference();
			}
			if (v16)
			{
				RectangleStruct nDummy = dest_surface->Get_Rect();
				auto area_rect = *rect1;
				area_rect = Drawing::Intersect(area_rect, nDummy);

				if (area_rect.Width > 0 && area_rect.Height > 0)
				{
					drawer->CurrentZRemap = drawerval;
					int xpos = xy->X;
					int ypos = xy->Y;
					RectangleStruct shape_rect = v16->GetFrameBounds(shapenum);
					auto v21 = v16->GetPixels(shapenum);
					auto height = v16->Height;
					auto v23 = v21;
					auto width = v16->Width;
					MemoryBuffer Buffer { v23 ,shape_rect.Height * shape_rect.Width };
					BSurface shape_surface { shape_rect.Width, shape_rect.Height,1, &Buffer };

					if (z_shape)
					{
						z_shape_rect = z_shape->GetFrameBounds(z_shape_frame);
						MemoryBuffer nMemBuffer { z_shape_rect.Height * z_shape_rect.Width };
						v35 = GameCreate<BSurface>(z_shape_rect.Width, z_shape_rect.Height, 1, &nMemBuffer);
					}

					if ((BYTE(flags) & 2) != 0)
					{
						xpos += width / -2;
						ypos += height / -2;
					}

					if (z_shape)
					{
						shape_x_offset -= width / 2 - shape_rect.X;
						shape_y_offset -= height / 2 - shape_rect.Y;
					}

					xpos += shape_rect.X;
					ypos += shape_rect.Y;
					shape_rect.X = 0;
					shape_rect.Y = 0;
					int intersect_rect_height = 0;
					RectangleStruct reused_rect {};

					if (!z_shape)
						goto noZShapeFound;
					{
						shape_x_offset += z_shape_rect.X;
						shape_y_offset += z_shape_rect.Y;
						reused_rect.X = -shape_x_offset;
						reused_rect.Y = -shape_y_offset;
						reused_rect.Width = z_shape_rect.Width;
						reused_rect.Height = z_shape_rect.Height;
						auto intersect_rect = Drawing::Intersect(shape_rect, reused_rect);
						shape_rect.X = intersect_rect.X;
						shape_rect.Y = intersect_rect.Y;
						shape_rect.Width = intersect_rect.Width;
						intersect_rect_height = intersect_rect.Height;
						shape_rect.Height = intersect_rect_height;
					}

					if (shape_rect.Width > 0 && intersect_rect_height > 0)
					{
					noZShapeFound:
						if (drawerval)
						{
							flags |= (BlitterFlags)16;
						}

						if (v16->HasCompression(shapenum))
						{

							if (auto blitter = drawer->Select_RLE_Blitter(flags))
							{
								reused_rect.X = xpos;
								reused_rect.Y = ypos;
								reused_rect.Width = shape_rect.Width;
								reused_rect.Height = shape_rect.Height;
								Point2D ShapePoint = { shape_rect.X, shape_rect.Y };
								Point2D AreaPoint = { area_rect.X, area_rect.Y };

								Buffer_To_RLE_Surface_With_Z_Shape(
									dest_surface,
									&AreaPoint,
									&reused_rect,
									&shape_surface,
									&ShapePoint,
									&shape_rect,
									blitter,
									height_offset,
									somearrayindex,
									intensity,
									0,
									v35,
									shape_x_offset,
									shape_y_offset,
									useotherblitterset);
							}
						}
						else
						{
							if (auto blitter = drawer->Select_Blitter(flags))
							{
								reused_rect.Y = 0;
								reused_rect.X = 0;
								z_shape_rect.X = xpos;
								z_shape_rect.Y = ypos;
								z_shape_rect.Width = shape_rect.Width;
								z_shape_rect.Height = shape_rect.Height;
								reused_rect.Width = shape_surface.Width;
								reused_rect.Height = shape_surface.Height;

								Buffer_To_Surface_with_LastArg(
									dest_surface,
									&area_rect,
									&z_shape_rect,
									&shape_surface,
									&reused_rect,
									&shape_rect,
									blitter,
									height_offset,
									somearrayindex,
									intensity,
									useotherblitterset,
									0);
							}
						}
					}

					if (v35)
					{
						GameDelete(v35);
					}

					shape_surface.DestroyBuffer();
				}
			}
		}

	}
};

DEFINE_HOOK(0x4AED70, Game_DrawSHP, 0x0)
{
	GET(Surface*, dest_surface, ECX);
	GET(ConvertClass*, drawer, EDX);
	GET_STACK(SHPReference*, shape, 0x4);
	GET_STACK(int, shapenum, 0x8);
	GET_STACK(Point2D*, xy, 0xC);
	GET_STACK(RectangleStruct*, rect1, 0x10);
	GET_STACK(BlitterFlags, flags, 0x14);
	GET_STACK(int, drawerval, 0x18);
	GET_STACK(int, height_offset, 0x1C);
	GET_STACK(int, somearrayindex, 0x20);
	GET_STACK(int, intensity, 0x24);
	GET_STACK(int, useotherblitterset, 0x28);
	GET_STACK(SHPStruct*, z_shape, 0x2C);
	GET_STACK(int, z_shape_frame, 0x30);
	GET_STACK(int, shape_x_offset, 0x34);
	GET_STACK(int, shape_y_offset, 0x38);

	DummyStructForDrawing::Callme(dest_surface, drawer, shape, shapenum, xy, rect1, flags, drawerval, height_offset, somearrayindex, intensity, useotherblitterset, z_shape, z_shape_frame
		, shape_x_offset, shape_y_offset);

	return 0x4AF292;
}
#endif

#include <Ext/AnimType/Body.h>

DEFINE_HOOK(0x423061, AnimClass_Draw_Visibility, 0x6)
{
	enum { SkipDrawing = 0x4238A3 };

	GET(AnimClass* const, pThis, ESI);

	if(!HouseClass::IsCurrentPlayerObserver()) {
		auto const pTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);
		auto pTechno = generic_cast<TechnoClass*>(pThis->OwnerObject);
		HouseClass* const pCurrentHouse = HouseClass::CurrentPlayer;

		if (pTypeExt->RestrictVisibilityIfCloaked
			&& pTechno && (pTechno->CloakState == CloakState::Cloaked || pTechno->CloakState == CloakState::Cloaking)
			&& !pTechno->Owner->IsAlliedWith(pCurrentHouse))
		{
			auto const pCell = pTechno->GetCell();

			if (pCell && !pCell->Sensors_InclHouse(pCurrentHouse->ArrayIndex))
				return SkipDrawing;
		}

		auto pOwner = pThis->OwnerObject ? pThis->OwnerObject->GetOwningHouse() : pThis->Owner;

		if (pTypeExt->VisibleTo_ConsiderInvokerAsOwner)
		{
			auto const pExt = AnimExtContainer::Instance.Find(pThis);

			if (pExt->Invoker)
				pOwner = pExt->Invoker->Owner;
		}

		AffectedHouse visibilityFlags = pTypeExt->VisibleTo;

		if (!EnumFunctions::CanTargetHouse(visibilityFlags, pCurrentHouse, pOwner))
			return SkipDrawing;
	}

	return 0;
}

DEFINE_HOOK(0x423183, AnimClass_DrawIt_CloakTranslucency, 0x6)
{
	enum { SkipGameCode = 0x423189 };

	GET(AnimClass*, pThis, ESI);

	auto const pTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);

	if (!pTypeExt->DetachOnCloak && pTypeExt->Translucency_Cloaked.isset()) {
		if (auto const pTechno = abstract_cast<TechnoClass*>(pThis->OwnerObject)) {
			if (pTechno->CloakState == CloakState::Cloaked || pTechno->CloakState == CloakState::Cloaking) {
				R->EAX(pTypeExt->Translucency_Cloaked.Get());
				return SkipGameCode;
			}
		}
	}

	return 0;
}