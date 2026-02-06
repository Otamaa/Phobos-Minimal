#include <Helpers/Macro.h>

#include <LaserDrawClass.h>
#include <GeneralStructures.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>
#include <Misc/PhobosGlobal.h>

//ColorStruct maxColor;
//
//ASMJIT_PATCH(0x550D1F, LaserDrawClass_DrawInHouseColor_Context_Set, 0x6)
//{
//	LEA_STACK(ColorStruct*, pColor, 0x14);
//	maxColor = *pColor;
//	return 0;
//}
//
////Enables proper laser thickness and falloff of it
//ASMJIT_PATCH(0x550F47, LaserDrawClass_DrawInHouseColor_BetterDrawing, 0x5) //0
//{
//	// Restore overridden code that's needed - Kerbiter
//	GET_STACK(bool, noQuickDraw, 0x13);
//	R->ESI(noQuickDraw ? 8u : 64u);
//
//	GET(LaserDrawClass*, pThis, EBX);
//	GET_STACK(int, currentThickness, 0x5C)
//
//	double mult = 1.0;
//	if (pThis->Thickness > 1) {
//		double falloffStep = 1.0 / pThis->Thickness;
//		double falloffMult = GeneralUtils::SecsomeFastPow(1.0 - falloffStep, currentThickness);
//		mult = (1.0 - falloffStep * currentThickness) * falloffMult;
//	}
//
//	R->EAX((unsigned int)(mult * maxColor.R));
//	R->ECX((unsigned int)(mult * maxColor.G));
//	R->EDX((unsigned int)(mult * maxColor.B));
//
//	return 0x550F9D;
//}

// ================================================================================
// LaserDrawClass::Draw_In_House_Color - Draws laser with house color and thickness
// ================================================================================

// Static coordinate offsets for 8 directions (initialized once)
static Point2D Draw_In_House_Color_coords[8][2];
static bool coords_initialized = false;

// ============================================================================
// [HOOK ADDITION] Global to store max color for falloff calculations
// Captured at 0x550D1F before thickness loop
// ============================================================================
static ColorStruct maxColor;

// --------------------------------------------------------------------------------
// Helper: Initialize direction offset table (unchanged)
// --------------------------------------------------------------------------------
static void Initialize_Direction_Coords()
{
	if (coords_initialized)
		return;

	Draw_In_House_Color_coords[0][0] = { -1, -1 };
	Draw_In_House_Color_coords[0][1] = { 1,  1 };
	Draw_In_House_Color_coords[1][0] = { 0, -1 };
	Draw_In_House_Color_coords[1][1] = { 0,  1 };
	Draw_In_House_Color_coords[2][0] = { -1,  1 };
	Draw_In_House_Color_coords[2][1] = { 1, -1 };
	Draw_In_House_Color_coords[3][0] = { -1,  0 };
	Draw_In_House_Color_coords[3][1] = { 1,  0 };
	Draw_In_House_Color_coords[4][0] = { -1, -1 };
	Draw_In_House_Color_coords[4][1] = { 1,  1 };
	Draw_In_House_Color_coords[5][0] = { 0, -1 };
	Draw_In_House_Color_coords[5][1] = { 0,  1 };
	Draw_In_House_Color_coords[6][0] = { 1, -1 };
	Draw_In_House_Color_coords[6][1] = { -1,  1 };
	Draw_In_House_Color_coords[7][0] = { -1,  0 };
	Draw_In_House_Color_coords[7][1] = { 1,  0 };

	coords_initialized = true;
}

// --------------------------------------------------------------------------------
// Helper: Calculate direction index (unchanged)
// --------------------------------------------------------------------------------
static unsigned int Calculate_Direction_Index(const CoordStruct& source, const CoordStruct& target)
{
	const double angle = Math::atan2(
		double(source.Y - target.Y),
		double(target.X - source.X)
	);
	const double adjusted = angle - Math::DEG90_AS_RAD;
	const int binary_angle = static_cast<int>(adjusted * Math::BINARY_ANGLE_MAGIC);
	return (((binary_angle >> 12) + 1) >> 1) & 7;
}

// --------------------------------------------------------------------------------
// Helper: Calculate intensity ratio (unchanged)
// --------------------------------------------------------------------------------
static int Calculate_Intensity_Ratio(const LaserDrawClass* laser)
{
	float intensity = 1.0f;

	if (laser->Fades && laser->Duration > 0)
	{
		const int elapsed = laser->Duration - laser->Progress.Stage;
		const float delta = laser->StartIntensity - laser->EndIntensity;
		intensity = (delta * elapsed / static_cast<float>(laser->Duration)) + laser->EndIntensity;
	}

	return static_cast<int>(intensity * 255.0f);
}

// --------------------------------------------------------------------------------
// Helper: Prepare draw color (unchanged)
// --------------------------------------------------------------------------------

static ColorStruct Prepare_Draw_Color(const LaserDrawClass* laser)
{
	if (laser->IsSupported) {
		unsigned int r = static_cast<unsigned int>(laser->InnerColor.R) * 2;
		unsigned int g = static_cast<unsigned int>(laser->InnerColor.G) * 2;
		unsigned int b = static_cast<unsigned int>(laser->InnerColor.B) * 2;

		return {
		static_cast<unsigned char>(std::min(r, 255u)),
		static_cast<unsigned char>(std::min(g, 255u)),
		static_cast<unsigned char>(std::min(b, 255u))
		};
	} else {
		return { laser->InnerColor.R >> 1  , laser->InnerColor.G >> 1 , laser->InnerColor.B >> 1 };
	}

}

// --------------------------------------------------------------------------------
// Helper: Apply direction offset (unchanged)
// --------------------------------------------------------------------------------
static void Apply_Direction_Offset(
	Point2D& pt1_a, Point2D& pt1_b,
	Point2D& pt2_a, Point2D& pt2_b,
	unsigned int direction,
	int iteration,
	bool is_diagonal)
{
	const auto& offsets = Draw_In_House_Color_coords[direction];

	if (is_diagonal)
	{
		pt1_a.X += offsets[0].X;
		pt1_a.Y += offsets[0].Y;
		pt1_b.X += offsets[1].X;
		pt1_b.Y += offsets[1].Y;
		pt2_a.X += offsets[0].X;
		pt2_a.Y += offsets[0].Y;
		pt2_b.X += offsets[1].X;
		pt2_b.Y += offsets[1].Y;
	}
	else if (iteration & 1)
	{
		pt1_a.X += offsets[0].X;
		pt1_b.Y += offsets[1].Y;
		pt2_a.X += offsets[0].X;
		pt2_b.Y += offsets[1].Y;
	}
	else
	{
		pt1_a.Y += offsets[0].Y;
		pt1_b.X += offsets[1].X;
		pt2_a.Y += offsets[0].Y;
		pt2_b.X += offsets[1].X;
	}
}

// ============================================================================
// [HOOK ADDITION] Smooth exponential falloff calculation
// Replaces the harsh halving (>>1) with gradual falloff
// ============================================================================
static COMPILETIMEEVAL double Calculate_Smooth_Falloff(int thickness, int current_layer)
{
	if (thickness <= 1)
		return 1.0;

	const double falloff_step = 1.0 / thickness;
	const double falloff_mult = GeneralUtils::SecsomeFastPow(1.0 - falloff_step, current_layer);
	return (1.0 - falloff_step * current_layer) * falloff_mult;
}

// ============================================================================
// [HOOK MODIFIED] Check color threshold
// ORIGINAL: checked after halving
// NEW: threshold still used but colors calculated differently
// ============================================================================
static bool Is_Color_Below_Threshold(const unsigned char* color, bool use_high_quality)
{
	// [HOOK] ESI is set to threshold: use_high_quality ? 8 : 64
	const unsigned int threshold = use_high_quality ? 8u : 64u;

	// Note: Original checked (color[i] >> 1) < threshold
	// With hook, we compare the calculated values directly
	return (color[0] < threshold) &&
		(color[1] < threshold) &&
		(color[2] < threshold);
}

//=================================================================================
#include <TacticalClass.h>
#include <Ext/Rules/Body.h>
#include <Surface.h>

class NOVTABLE FakeLaserDrawClaass : public LaserDrawClass {
public:
	void __DrawInHouseColor();
};
// ================================================================================
// Main Function: Draw_In_House_Color (with hook modifications marked)
// ================================================================================
void FakeLaserDrawClaass::__DrawInHouseColor()
{
	Initialize_Direction_Coords();

	// ------------------------------------------------------------------------
	// Calculate direction index from angle
	// ------------------------------------------------------------------------
	const unsigned int direction = Calculate_Direction_Index(Source, Target);

	// ------------------------------------------------------------------------
	// Convert world coordinates to screen pixels
	// ------------------------------------------------------------------------
	Point2D pt_source = TacticalClass::Instance->CoordsToClient(Source);
	Point2D pt_target = TacticalClass::Instance->CoordsToClient(Target);

	// ------------------------------------------------------------------------
	// Calculate Z-depths
	// ------------------------------------------------------------------------
	const int z_source = ZAdjust - Game::AdjustHeight(Source.Z) - 2;
	const int z_target = -2 - Game::AdjustHeight(Target.Z);

	// ------------------------------------------------------------------------
	// Determine rendering quality
	// ------------------------------------------------------------------------
	const bool use_high_quality = RulesExtData::DetailsCurrentlyEnabled();

	// ------------------------------------------------------------------------
	// Calculate intensity and ratio
	// ------------------------------------------------------------------------
	const int ratio = Calculate_Intensity_Ratio(this);
	float intensity = 1.0f;

	if (Fades && Duration > 0)
	{
		const int elapsed = Duration - Progress.Stage;
		const float delta = StartIntensity - EndIntensity;
		intensity = (delta * elapsed / static_cast<float>(Duration)) + EndIntensity;
	}

	// ------------------------------------------------------------------------
	// Prepare colors
	// ------------------------------------------------------------------------
	ColorStruct draw_info = Prepare_Draw_Color(this);
	ColorStruct working_color = draw_info;

	// ========================================================================
	// [HOOK 0x550D1F] Capture max color for falloff calculations
	// This happens just before the thickness loop starts
	// ========================================================================
	maxColor = working_color;

	// Store original inner color
	ColorStruct inner_color = InnerColor;

	// ------------------------------------------------------------------------
	// Initialize line endpoint pairs
	// ------------------------------------------------------------------------
	Point2D line1_start = pt_source;
	Point2D line1_end = pt_source;
	Point2D line2_start = pt_target;
	Point2D line2_end = pt_target;

	// ------------------------------------------------------------------------
	// Draw thickness layers
	// ------------------------------------------------------------------------
	const bool is_diagonal = (direction & 1) != 0;

	if (Thickness >= 1)
	{
		for (int layer = 1; layer <= Thickness; ++layer)
		{
			Apply_Direction_Offset(
				line1_start, line1_end,
				line2_start, line2_end,
				direction, layer, is_diagonal
			);

			if (!Game::bDirect3DIsUseable())
			{
				// Draw the lines for this layer
				if (use_high_quality)
				{
					DSurface::Temp->DrawRGBMultiplyingLine_AZ(
						&DSurface::ViewBounds(),
						&line1_start, &line2_start,
						&working_color, static_cast<int>(intensity),
						z_source, z_target
					);
					DSurface::Temp->DrawRGBMultiplyingLine_AZ(
						&DSurface::ViewBounds(),
						&line1_end, &line2_end,
						&working_color, static_cast<int>(intensity),
						z_source, z_target
					);
				}
				else
				{
					ColorStruct rgb_work = working_color;
					static constexpr ColorStruct white { 255, 255, 255 };
					rgb_work.Adjust(ratio, white);

					const unsigned int packed = rgb_work.ToInit();

					DSurface::Temp->DrawLineColor_AZ(
						DSurface::ViewBounds(),
						line1_start, line2_start,
						packed, z_source, z_target, 0
					);
					DSurface::Temp->DrawLineColor_AZ(
						DSurface::ViewBounds(),
						line1_end, line2_end,
						packed, z_source, z_target, 0
					);
				}

				// ============================================================
				// [HOOK 0x550F47] Color falloff calculation - MODIFIED
				// ============================================================
				// ORIGINAL CODE:
				// if (AdjustColor && layer == 1)
				// {
				//     working_color[0] = inner_color[0];
				//     working_color[1] = inner_color[1];
				//     working_color[2] = inner_color[2];
				// }
				// else
				// {
				//     // Check threshold BEFORE halving in original
				//     unsigned int threshold = use_high_quality ? 8 : 64;
				//     if ((working_color[0] >> 1) < threshold &&
				//         (working_color[1] >> 1) < threshold &&
				//         (working_color[2] >> 1) < threshold)
				//     {
				//         break;  // Color too dim, stop drawing layers
				//     }
				//     // Harsh halving - each layer is 50% of previous
				//     working_color[0] >>= 1;
				//     working_color[1] >>= 1;
				//     working_color[2] >>= 1;
				// }

				// NEW CODE (with hook):
				// Smooth exponential falloff using captured maxColor
				{
					// [HOOK] Set threshold: ESI = use_high_quality ? 8 : 64
					const unsigned int threshold = use_high_quality ? 8u : 64u;

					// [HOOK] Calculate smooth falloff multiplier
					const double mult = Calculate_Smooth_Falloff(Thickness, layer);

					// [HOOK] Apply multiplier to original maxColor (not previous layer)
					working_color.R = static_cast<unsigned char>(mult * maxColor.R);
					working_color.G = static_cast<unsigned char>(mult * maxColor.G);
					working_color.B = static_cast<unsigned char>(mult * maxColor.B);

					// [HOOK] Jumps to 0x550F9D which is the threshold comparison
					// Still check if color is too dim to continue
					if (working_color.R < threshold &&
						working_color.G < threshold &&
						working_color.B < threshold)
					{
						break;
					}
				}
			}
		}
	}

	// ------------------------------------------------------------------------
	// Final rendering pass (unchanged)
	// ------------------------------------------------------------------------
	if (Game::bDirect3DIsUseable() && DSurface::CD3DTriangleInstance() && ZBuffer::Instance)
	{
		// D3D triangle rendering...
		const short z_max = ZBuffer::Instance->MaxValue;
		const short viewport_y = static_cast<short>(DSurface::ViewBounds->Y);

		const int z_val_source = z_source + z_max + ZBuffer::Instance->Area.Y
			- static_cast<short>(pt_source.Y) - viewport_y;
		const int z_val_target = z_target + z_max + ZBuffer::Instance->Area.Y
			- static_cast<short>(pt_target.Y) - viewport_y;

		const float sz_source = static_cast<float>(z_val_source & 0xFFFF) * 0.000015259022f;
		const float sz_target = static_cast<float>(z_val_target & 0xFFFF) * 0.000015259022f;

		const int red = (ratio * inner_color.R) >> 8;
		const int green = (ratio * inner_color.G) >> 8;
		const int blue = (ratio * inner_color.B) >> 8;

		CD3DTriangle tri1, tri2;
		tri1.Set_Color(red, green, blue);
		tri2.Set_Color(red, green, blue);

		tri1.Set_Coords(0, static_cast<float>(line1_start.X), static_cast<float>(line1_start.Y), sz_source, 0.0f, 0.0f);
		tri1.Set_Coords(1, static_cast<float>(line1_end.X), static_cast<float>(line1_end.Y), sz_source, 0.0f, 1.0f);
		tri1.Set_Coords(2, static_cast<float>(line2_start.X), static_cast<float>(line2_start.Y), sz_target, 1.0f, 0.0f);

		tri2.Set_Coords(0, static_cast<float>(line1_end.X), static_cast<float>(line1_end.Y), sz_source, 0.0f, 1.0f);
		tri2.Set_Coords(1, static_cast<float>(line2_end.X), static_cast<float>(line2_end.Y), sz_target, 1.0f, 1.0f);
		tri2.Set_Coords(2, static_cast<float>(line2_start.X), static_cast<float>(line2_start.Y), sz_target, 1.0f, 0.0f);

		DSurface::CD3DTriangleInstance->Add(&tri1);
		DSurface::CD3DTriangleInstance->Add(&tri2);
	}
	else
	{
		// Software center line
		if (use_high_quality)
		{
			DSurface::Temp->DrawRGBMultiplyingLine_AZ(
				&DSurface::ViewBounds(),
				&pt_source, &pt_target,
				&inner_color, static_cast<int>(intensity),
				z_source, z_target
			);
		}
		else
		{
			ColorStruct rgb_work = inner_color;
			ColorStruct white { 255, 255, 255 };
			rgb_work.Adjust(ratio, white);

			const unsigned int packed = rgb_work.ToInit();

			DSurface::Temp->DrawLineColor_AZ(
				DSurface::ViewBounds(),
				pt_source, pt_target,
				packed, z_source, z_target, 0
			);
		}
	}
}

DEFINE_FUNCTION_JUMP(CALL, 0x55027B, FakeLaserDrawClaass::__DrawInHouseColor);