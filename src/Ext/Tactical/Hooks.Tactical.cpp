#include "Body.h"

#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Commands/ShowTeamLeader.h>
#include <Commands/ToggleRadialIndicatorDrawMode.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/Techno/Body.h>

#include <TerrainTypeClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>

#include <Conversions.h>

#include <AlphaShapeClass.h>
#include <InfantryClass.h>
#include <AircraftClass.h>
#include <TeamClass.h>
#include <VeinholeMonsterClass.h>
#include <IonBlastClass.h>

#include <Commands/ToggleSuperTimers.h>

static COMPILETIMEEVAL void ShakeScreen(GScreenClass* pScreen)
{
	/**
	 *   TibSun style.
	 */

	if (pScreen->ScreenShakeX >= 0)
	{
		if (pScreen->ScreenShakeX > 0)
		{
			pScreen->ScreenShakeX = pScreen->ScreenShakeX - 1;
		}
	}
	else
	{
		pScreen->ScreenShakeX = pScreen->ScreenShakeX + 1;
	}

	if (pScreen->ScreenShakeY >= 0)
	{
		if (pScreen->ScreenShakeY > 0)
		{
			pScreen->ScreenShakeY = pScreen->ScreenShakeY - 1;
		}
	}
	else
	{
		pScreen->ScreenShakeY = pScreen->ScreenShakeY + 1;
	}
}

ASMJIT_PATCH(0x4F4BB9, GSCreenClass_AI_ShakescreenMode, 0x5)
{

	GET(GScreenClass*, pThis, ECX);

	if (RulesExtData::Instance()->ShakeScreenUseTSCalculation)
	{
		ShakeScreen(pThis);
		return 0x4F4BEF;
	}

	return 0x0;
}

ASMJIT_PATCH(0x6D912B, TacticalClass_Render_BuildingInLimboDeliveryA, 0x9)
{
	enum { Draw = 0x0, DoNotDraw = 0x6D9159 };

	GET(TechnoClass* const, pTechno, ESI);

	if (pTechno->WhatAmI() == BuildingClass::AbsID)
	{
		if (BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pTechno))->LimboID >= 0)
		{
			return DoNotDraw;
		}
	}

	return Draw;
}

ASMJIT_PATCH(0x6D966A, TacticalClass_Render_BuildingInLimboDeliveryB, 0x9)
{
	enum { Draw = 0x0, DoNotDraw = 0x6D978F };

	GET(TechnoClass* const, pTechno, EBX);

	if (pTechno->WhatAmI() == BuildingClass::AbsID)
	{
		if (BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pTechno))->LimboID >= 0)
		{
			return DoNotDraw;
		}
	}

	return Draw;
}

//hook at Tactical_super_lines_circles
ASMJIT_PATCH(0x6DBE35, TacticalClass_DrawLinesOrCircles, 0x9)
{
	ObjectClass** items = !ToggleRadialIndicatorDrawModeClass::ShowForAll ? ObjectClass::CurrentObjects->Items : (ObjectClass**)TechnoClass::Array->Items;
	const int count = !ToggleRadialIndicatorDrawModeClass::ShowForAll ? ObjectClass::CurrentObjects->Count : TechnoClass::Array->Count;

	if (count <= 0)
		return 0x6DBE74;

	ObjectClass** items_end = &items[count];

	for (ObjectClass** walk = items; walk != items_end; ++walk)
	{
		if (*walk)
		{
			if (auto pObjType = (*walk)->GetType())
			{
				if (pObjType->HasRadialIndicator)
				{
					(*walk)->DrawRadialIndicator(1);
				}
				//else if (auto pTechno = flag_cast_to<TechnoClass*>(*walk))
				//{
				//	auto pTypeExt = TechnoTypeExtContainer::Instance.Find((TechnoTypeClass*)(pObjType));

				//	if (pTypeExt->DesignatorRange > 0)
				//	{
				//		if (HouseClass::IsCurrentPlayerObserver()
				//		|| pTechno->Owner->ControlledByCurrentPlayer())
				//		{
				//			int nRadius = pTypeExt->DesignatorRange;
				//			const auto Color = pTypeExt->RadialIndicatorColor.Get(pTechno->Owner->Color);

				//			if (Color != ColorStruct::Empty)
				//			{
				//				auto nCoord = pTechno->GetCoords();
				//				FakeTacticalClass::__DrawRadialIndicator(false, true, nCoord, Color, (nRadius * 1.0f), false, true);
				//			}
				//		}
				//	}
				//}
			}
		}
	}

	return 0x6DBE74;
}

ASMJIT_PATCH(0x420F75, AlphaLightClass_UpdateScreen_ShouldDraw, 5)
{
	GET(AlphaShapeClass*, pAlpha, ECX);

	bool shouldDraw = !pAlpha->IsObjectGone;

	if(shouldDraw && pAlpha->AttachedTo) {
		const auto table = VTable::Get(pAlpha->AttachedTo);
		if (table == InfantryClass::vtable || table == UnitClass::vtable || table == AircraftClass::vtable || table == BuildingClass::vtable) {
			shouldDraw =
				((TechnoClass*)pAlpha->AttachedTo)->VisualCharacter(VARIANT_TRUE, ((TechnoClass*)pAlpha->AttachedTo)->Owner) == VisualType::Normal
			&& !((TechnoClass*)pAlpha->AttachedTo)->Disguised;
		}
	}

	return shouldDraw ? 0x420F80 : 0x42132A;

}

ASMJIT_PATCH(0x4210AC, AlphaLightClass_UpdateScreen_Header, 5)
{
	GET(AlphaShapeClass*, pAlpha, EDX);
	GET(SHPStruct *, pImage, ECX);

	if (const auto pTechno = flag_cast_to <TechnoClass*>(pAlpha->AttachedTo))
	{
		unsigned int idx = 0;
		if (pImage->Frames > 0) {
			const int countFrames = Conversions::Int2Highest(pImage->Frames);
			const DirStruct PrimaryFacing = pTechno->PrimaryFacing.Current();
			idx = (PrimaryFacing.Raw >> (16 - countFrames));
		}

		R->Stack(0x0, idx);
	}

	return 0;
}

ASMJIT_PATCH(0x4211AC, AlphaLightClass_UpdateScreen_Body, 8)
{
	GET_STACK(int, AlphaLightIndex, STACK_OFFS(0xDC, 0xB4));
	GET_STACK(SHPStruct*, pImage, STACK_OFFS(0xDC, 0x6C));

	const auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];

	if(const auto pTechno = flag_cast_to<TechnoClass*>(pAlpha->AttachedTo)) {
		unsigned int idx = 0;
		if (pImage->Frames > 0) {
			const int countFrames = Conversions::Int2Highest(pImage->Frames);
			const DirStruct PrimaryFacing = pTechno->PrimaryFacing.Current();
			idx = (PrimaryFacing.Raw >> (16 - countFrames));
		}

		R->Stack(0x0, idx);
	}

	return 0;
}

ASMJIT_PATCH(0x42146E, TacticalClass_UpdateAlphasInRectangle_Header, 5)
{
	GET(int, AlphaLightIndex, EBX);
	GET(RectangleStruct*, buffer, EDX);
	GET(SHPStruct*, pImage, EDI);

	const auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];
	unsigned int idx = 0;

	if (const auto pTechno = flag_cast_to<TechnoClass*>(pAlpha->AttachedTo))  {
		if (pImage->Frames > 0) {
			const int countFrames = Conversions::Int2Highest(pImage->Frames);
			const DirStruct PrimaryFacing = pTechno->PrimaryFacing.Current();
			idx = (PrimaryFacing.Raw >> (16 - countFrames));
		}
	}

	R->EAX(pImage->GetFrameBounds_ptr(buffer, idx));
	return 0x421478;
}

ASMJIT_PATCH(0x42152C, TacticalClass_UpdateAlphasInRectangle_Body, 8)
{
	GET_STACK(int, AlphaLightIndex, STACK_OFFS(0xA4, 0x78));
	GET(SHPStruct*, pImage, ECX);

	const auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];
	if (const auto pTechno = flag_cast_to<TechnoClass*>(pAlpha->AttachedTo)) {
		if (pImage->Frames > 0) {
			const int countFrames = Conversions::Int2Highest(pImage->Frames);
			const DirStruct PrimaryFacing = pTechno->PrimaryFacing.Current();
			R->ESP((PrimaryFacing.Raw >> (16 - countFrames)));
		}
	}

	return 0;
}

ASMJIT_PATCH(0x421371, TacticalClass_UpdateAlphasInRectangle_ShouldDraw, 5)
{
	GET(int, AlphaLightIndex, EBX);
	auto pAlpha = AlphaShapeClass::Array->Items[AlphaLightIndex];

	bool shouldDraw = !pAlpha->IsObjectGone;

	if (shouldDraw && pAlpha->AttachedTo) {
		const auto table = VTable::Get(pAlpha->AttachedTo);
		if (table == InfantryClass::vtable || table == UnitClass::vtable || table == AircraftClass::vtable || table == BuildingClass::vtable ) {
			shouldDraw = ((TechnoClass*)pAlpha->AttachedTo)->IsAlive && ((TechnoClass*)pAlpha->AttachedTo)->VisualCharacter(VARIANT_TRUE, ((TechnoClass*)pAlpha->AttachedTo)->Owner) == VisualType::Normal &&
				!((TechnoClass*)pAlpha->AttachedTo)->Disguised;
		}
		//else if (table == AnimClass::vtable || table == ParticleClass::vtable || table == VoxelAnimClass::vtable) {
		//	Debug::LogInfo("Alpha[%x - %d] Attached to [%s - %s] with state [%s]", pAlpha, AlphaLightIndex,
		//		pAlpha->AttachedTo->GetType()->ID,
		//		pAlpha->AttachedTo->GetThisClassName()
		//	, !pAlpha->AttachedTo->IsAlive ? "Dead" : "Alive"
		//	);
		//}
	}

	return shouldDraw ? 0 : 0x421694;
}

/*******************************************************************************
	* Cohen-Sutherland Line Clipping Algorithm
	* Cleaned up version
	******************************************************************************/

	/*
	 * Build bits that indicate which end points lie outside the clipping rectangle.
	 * Quick checks against these flag bits will speed the clipping process.
	 */
constexpr inline int CODE_INSIDE = 0;  // 0000
constexpr inline int CODE_LEFT = 1;    // 0001
constexpr inline int CODE_RIGHT = 2;   // 0010
constexpr inline int CODE_BOTTOM = 4;  // 0100
constexpr inline int CODE_TOP = 8;     // 1000

/***********************************************************************************************
 * Compute_Out_Code
 *
 * Compute the bit code for a point (x, y) using the clip rectangle.
 * Bounded diagonally by (xmin, ymin), and (xmax, ymax).
 *
 * INPUT:   x     -- X coordinate of point
 *          y     -- Y coordinate of point
 *          rect  -- Clipping rectangle
 *
 * OUTPUT:  OutCode indicating which boundaries the point is outside of
 ***********************************************************************************************/
int __forceinline Compute_Out_Code(double x, double y, RectangleStruct* rect)
{
	int code = CODE_INSIDE;

	int right_edge = rect->X + rect->Width;
	if (x >= right_edge)
	{
		code |= CODE_RIGHT;
	}
	else if (x < rect->X)
	{
		code |= CODE_LEFT;
	}

	int bottom_edge = rect->Y + rect->Height;
	if (y >= bottom_edge)
	{
		code |= CODE_BOTTOM;
	}
	else if (y < rect->Y)
	{
		code |= CODE_TOP;
	}

	return code;
}

/***********************************************************************************************
 * Clip_Line
 *
 * Cohen-Sutherland line clipping algorithm implementation.
 * Clips a line segment to fit within the specified rectangle.
 *
 * INPUT:   pt1   -- First point of line segment (modified in place)
 *          pt2   -- Second point of line segment (modified in place)
 *          rect  -- Clipping rectangle
 *
 * OUTPUT:  true if line segment intersects rectangle, false otherwise
 *
 * NOTES:   Based on algorithm from "Computer Graphics: Principles and Practice in C"
 *          Modified version of: https://en.wikipedia.org/wiki/Cohen–Sutherland_algorithm
 ***********************************************************************************************/
bool __fastcall Clip_Line(Point2D& point1, Point2D& point2, RectangleStruct& rect)
{
	int outcode0 = Compute_Out_Code(point1.X, point1.Y, &rect);
	int outcode1 = Compute_Out_Code(point2.X, point2.Y, &rect);

	double x0 = point1.X;
	double y0 = point1.Y;
	double x1 = point2.X;
	double y1 = point2.Y;

	while (true)
	{
		// Trivial accept
		if (outcode0 == CODE_INSIDE && outcode1 == CODE_INSIDE)
		{
			point1.X = (int)x0; point1.Y = (int)y0;
			point2.X = (int)x1; point2.Y = (int)y1;
			return true;
		}

		// Trivial reject
		if (outcode0 & outcode1)
			return false;

		// Choose endpoint outside rect
		int outcodeOut = (outcode0 != CODE_INSIDE) ? outcode0 : outcode1;

		double x = 0.0;
		double y = 0.0;

		// Find intersection
		if (outcodeOut & CODE_TOP)           // above clip window
		{
			double dy = y1 - y0;
			if (dy == 0.0) return false; // horizontal line outside
			double slope_y = (x1 - x0) / dy;
			y = rect.Y;
			x = x0 + (y - y0) * slope_y;
		}
		else if (outcodeOut & CODE_BOTTOM)   // below clip window
		{
			double dy = y1 - y0;
			if (dy == 0.0) return false;
			double slope_y = (x1 - x0) / dy;
			y = rect.Y + rect.Height - 1;
			x = x0 + (y - y0) * slope_y;
		}
		else if (outcodeOut & CODE_RIGHT)    // to the right of clip window
		{
			double dx = x1 - x0;
			if (dx == 0.0) return false; // vertical line outside
			double slope_x = (y1 - y0) / dx;
			x = rect.X + rect.Width - 1;
			y = y0 + (x - x0) * slope_x;
		}
		else if (outcodeOut & CODE_LEFT)     // to the left of clip window
		{
			double dx = x1 - x0;
			if (dx == 0.0) return false;
			double slope_x = (y1 - y0) / dx;
			x = rect.X;
			y = y0 + (x - x0) * slope_x;
		}
		else
		{
			// Safety net: outcodeOut has no directional bits? -> break
			return false;
		}

		// Move the outside point to intersection and recalc code
		if (outcodeOut == outcode0)
		{
			x0 = x; y0 = y;
			outcode0 = Compute_Out_Code(x0, y0, &rect);
		}
		else
		{
			x1 = x; y1 = y;
			outcode1 = Compute_Out_Code(x1, y1, &rect);
		}
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x7BC2B0, Clip_Line)

DEFINE_FUNCTION_JUMP(CALL, 0x436123, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x43617B, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4BBD72, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4BC933, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4BE060, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4BEBA6, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4BF7CD, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4BFDFF, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4C0EAA, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4C27D0, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4C28F4, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4DC694, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x63D8A3, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6595F6, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6598F5, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x659DA0, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x660272, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6605EB, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6DAC2B, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6DACD2, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6DAD1B, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6DB0E1, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6FFB70, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x704BE9, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x704DD4, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x704E14, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x70516F, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x70552E, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x705772, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x7BA689, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x7BAC06, Clip_Line)

ASMJIT_PATCH(0x6B0B81, SlaveManagerClass_FreeSlaves_dead, 0x5)
{
	GET(TechnoClass*, pTech, ESI);

	return pTech->IsAlive ? 0x0 : 0x6B0C0B;
}
