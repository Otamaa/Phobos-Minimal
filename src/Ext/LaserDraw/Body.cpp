#include "Body.h"

#include <Helpers/Macro.h>
#include <TacticalClass.h>
#include <Surface.h>
#include <Drawing.h>
#include <Unsorted.h>
#include <Randomizer.h>
#include <HouseClass.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>
#include <Misc/PhobosGlobal.h>
#include <Ext/Rules/Body.h>
#include <Ext/House/Body.h>

#include <Ext/WarheadType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

// ============================================================================
// Behaviour toggles
//
// All divergences from vanilla are gated here so a single flip restores
// byte-faithful vanilla rendering for A/B comparison against the real binary.
// These are constexpr, so the disabled branches are eliminated by LTCG and
// cost nothing at runtime.
// ============================================================================
namespace LaserDrawConfig
{
	// --- Issue #4 -----------------------------------------------------------
	// After the IsSupported layer-1 reset (workingColor = InnerColor), re-base
	// the smooth-falloff reference colour to the reset value.
	//
	// false (vanilla shape): falloff keeps decaying from clamp(2*Inner), so
	//   layer 2 renders BRIGHTER than layer 1 and the beam never converges.
	//   This is the root cause of "supported lasers become really thick".
	// true: falloff decays from InnerColor after the reset, matching the
	//   monotonic brightness curve vanilla's >>1 halving produces.
	inline constexpr bool RebaseFalloffAfterSupportedReset = true;

	// --- Issue #5 -----------------------------------------------------------
	// _CalculateSmoothFalloff(T, L) evaluates to exactly 0.0 when L == T,
	// because of the (1 - L/T) term.
	//
	// In _DrawInHouseColor the falloff is computed AFTER drawing, so the zero
	// lands on an already-finished loop and is harmless.
	//
	// In _DrawLaser the falloff is computed BEFORE drawing, so the final layer
	// is always black and always trips the dim-out break. Net effect:
	// Thickness == 2 renders identically to Thickness == 1.
	//
	// true: offset the exponent by one in _DrawLaser so its first extra layer
	//   uses falloff(T, 1) — the same value _DrawInHouseColor's first falloff
	//   step uses. Keeps the two functions on one shared curve.
	inline constexpr bool FixFalloffFinalLayer = true;

	// --- Issue #6 -----------------------------------------------------------
	// Vanilla Draw hardcodes 1.0f as the intensity for the two OUTER
	// subtractive lines (push 3F800000h @ 0x55066F and @ 0x5506FF) and passes
	// the real fade intensity only to the INNER centre line (@ 0x55062A).
	//
	// true  (vanilla): outer glow holds full strength as the laser fades.
	// false (extension): outer glow fades with the beam. Softer, but the
	//   halo visibly detaches from the core on long Duration values.
	inline constexpr bool VanillaOuterLineIntensity = true;

	// --- Issue #7 -----------------------------------------------------------
	// EXTENSION: skip drawing entirely when both endpoints are fogged.
	// Vanilla Draw has no fog test — LaserDrawClass::Draw_All is the only
	// visibility gate. Purely an optimisation / fog-correctness addition.
	inline constexpr bool EnableFogOfWarCulling = true;

	// --- EXTENSION ----------------------------------------------------------
	// Vanilla Draw never reads __Thickness (verified: no [ebx+1Ch] access
	// anywhere in 0x550260-0x5509D1). Thickness is a house-colour-only feature.
	// Enabling this applies it to multicolour lasers as well.
	inline constexpr bool EnableNonHouseThickness = true;

	// EXTENSION: widen the INNER core too, not just the outer glow.
	//
	// Without this, increasing Thickness on a non-house laser fattens the halo
	// while the core stays exactly 1px — because vanilla's inner and outer are
	// separate draws and only the outer participates in the offset loop.
	// This is the "middle stays small" symptom.
	inline constexpr bool EnableNonHouseCoreThickness = true;

	// EXTENSION: apply non-house thickness under D3D as well.
	//
	// Vanilla's non-house path has no D3D triangle branch at all — it always
	// goes through software line draws. The thickness loop is therefore safe
	// to run under D3D, unlike the house-colour path where the loop's only
	// purpose under D3D is to widen the triangle quad.
	inline constexpr bool NonHouseThicknessUnderD3D = true;

	// 1 == current behaviour.
	inline constexpr int DefaultCoreThickness = 1;

	// Clamp ceiling. Guards against an INI typo producing a loop that draws
	// several hundred lines per laser per frame.
	inline constexpr int MaxCoreThickness = 32;
}

int FakeLaserDrawClass::_GetCoreThickness() const
{
	const int value = LaserDrawConfig::DefaultCoreThickness;

	return std::clamp(value, 1, LaserDrawConfig::MaxCoreThickness);
}

// ============================================================================
// Direction coordinate tables
//
// BUGFIX (issue #1): vanilla maintains TWO SEPARATE tables. The previous
// backport routed both _DrawLaser and _DrawInHouseColor through the
// house-colour table, which is wrong for four of the eight directions.
//
//   dir | Draw_Coords (non-house)   | House_Color_coords
//   ----+---------------------------+---------------------------
//    0  | ( 0,-1) ( 1, 0)           | (-1,-1) ( 1, 1)   <-- differs
//    1  | ( 0,-1) ( 0, 1)           | ( 0,-1) ( 0, 1)
//    2  | ( 1, 0) ( 0, 1)           | (-1, 1) ( 1,-1)   <-- differs
//    3  | (-1, 0) ( 1, 0)           | (-1, 0) ( 1, 0)
//    4  | (-1, 0) ( 0, 1)           | (-1,-1) ( 1, 1)   <-- differs
//    5  | ( 0,-1) ( 1, 0)           | ( 0,-1) ( 0, 1)   <-- differs
//    6  | ( 0,-1) (-1, 0)           | (-1, 1) ( 1,-1)   <-- differs
//    7  | (-1, 0) ( 1, 0)           | (-1, 0) ( 1, 0)
//
// The non-house table uses the two AXIS-ADJACENT pixels on diagonals, not a
// diagonal pair. Using the house table shifted every diagonal laser's glow
// by one pixel off-axis.
// ============================================================================
bool FakeLaserDrawClass::s_CoordsInitialized = false;

void FakeLaserDrawClass::_InitializeDirectionCoords()
{
	if (s_CoordsInitialized)
		return;

	// ------------------------------------------------------------------
	// House-colour table — Draw_In_House_Color_coords @ 0xABC7F8
	// Init block @ 0x550A0D .. 0x550B19
	// ------------------------------------------------------------------
	HouseCoords[0][0] = { -1, -1 };   // 0x550A15 / 0x550A1F
	HouseCoords[0][1] = { 1,  1 };   // 0x550A27 / 0x550A2C
	HouseCoords[1][0] = { 0, -1 };   // 0x550A37 / 0x550A41
	HouseCoords[1][1] = { 0,  1 };   // 0x550A3C / 0x550A5E
	HouseCoords[2][0] = { -1,  1 };   // 0x550A4F / 0x550A64
	HouseCoords[2][1] = { 1, -1 };   // 0x550A56 / 0x550A77
	HouseCoords[3][0] = { -1,  0 };   // 0x550A6D / 0x550A87
	HouseCoords[3][1] = { 1,  0 };   // 0x550A7F / 0x550A8D
	HouseCoords[4][0] = { -1, -1 };   // 0x550A96 / 0x550AA0
	HouseCoords[4][1] = { 1,  1 };   // 0x550AA8 / 0x550AAD
	HouseCoords[5][0] = { 0, -1 };   // 0x550AB8 / 0x550AC2
	HouseCoords[5][1] = { 0,  1 };   // 0x550ABD / 0x550AD7

	// BUGFIX (issue #2): previously { 1,-1 } / { -1, 1 } and commented as
	// "mirrored 2". Raw assembly shows direction 6 is IDENTICAL to direction 2.
	//   0x550AD0  mov [coords.X+60h], -1     -> [6][0].X = -1
	//   0x550ADD  mov [coords.Y+60h],  1     -> [6][0].Y =  1
	//   0x550AE3  mov [coords.X+68h],  1     -> [6][1].X =  1
	//   0x550AEE  mov [coords.Y+68h], -1     -> [6][1].Y = -1
	// The sign flip broke the even-direction alternating branch of the
	// thickness loop, expanding into the opposite quadrant.
	HouseCoords[6][0] = { -1,  1 };
	HouseCoords[6][1] = { 1, -1 };

	HouseCoords[7][0] = { -1,  0 };   // 0x550AF4 / 0x550B0E
	HouseCoords[7][1] = { 1,  0 };   // 0x550B14 / 0x550B19

	// ------------------------------------------------------------------
	// Non-house table — Draw_Coords @ 0xABC738
	// Init block @ 0x550295 .. 0x5503A0
	// Vanilla indexes this flat as Draw_Coords[2*dir] and [2*dir+1];
	// reshaped to [dir][slot] here.
	// ------------------------------------------------------------------
	OuterCoords[0][0] = { 0, -1 };   // flat[0]  0x5502A4 / 0x55029C
	OuterCoords[0][1] = { 1,  0 };   // flat[1]  0x5502B7 / 0x5502AE
	OuterCoords[1][0] = { 0, -1 };   // flat[2]  0x5502C9 / 0x5502BE
	OuterCoords[1][1] = { 0,  1 };   // flat[3]  0x5502CE / 0x5502E4
	OuterCoords[2][0] = { 1,  0 };   // flat[4]  0x5502D5 / 0x5502F6
	OuterCoords[2][1] = { 0,  1 };   // flat[5]  0x5502DC / 0x550309
	OuterCoords[3][0] = { -1,  0 };   // flat[6]  0x5502EA / 0x550318
	OuterCoords[3][1] = { 1,  0 };   // flat[7]  0x5502FC / 0x55031E
	OuterCoords[4][0] = { -1,  0 };   // flat[8]  0x55030F / 0x550324
	OuterCoords[4][1] = { 0,  1 };   // flat[9]  0x55032A / 0x55033E
	OuterCoords[5][0] = { 0, -1 };   // flat[10] 0x55032F / 0x55034E
	OuterCoords[5][1] = { 1,  0 };   // flat[11] 0x550344 / 0x55035E
	OuterCoords[6][0] = { 0, -1 };   // flat[12] 0x550354 / 0x55036F
	OuterCoords[6][1] = { -1,  0 };   // flat[13] 0x550364 / 0x55038F
	OuterCoords[7][0] = { -1,  0 };   // flat[14] 0x550375 / 0x550395
	OuterCoords[7][1] = { 1,  0 };   // flat[15] 0x55039B / 0x5503A0

	s_CoordsInitialized = true;
}

// ============================================================================
// Smooth falloff
//
// DIFF: replaces vanilla's per-layer >>1 halving with a smoothed curve.
// Vanilla: c[L] = c[0] >> L      (exponential, halves every layer)
// Here:    c[L] = c[0] * (1 - L/T) * (1 - 1/T)^L
//
// The linear (1 - L/T) term guarantees the outermost layer reaches zero, so
// the beam always terminates cleanly instead of stopping wherever the 8/64
// dim-out threshold happens to land.
//
// NOTE: returns exactly 0.0 at currentLayer == thickness. Callers that
// evaluate the falloff BEFORE drawing must offset by one — see
// LaserDrawConfig::FixFalloffFinalLayer.
// ============================================================================
double FakeLaserDrawClass::_CalculateSmoothFalloff(int thickness, int currentLayer)
{
	if (thickness <= 1)
		return 1.0;

	const double falloffStep = 1.0 / static_cast<double>(thickness);
	const double falloffMult = GeneralUtils::SecsomeFastPow(
		1.0 - falloffStep, static_cast<size_t>(currentLayer));

	return (1.0 - falloffStep * static_cast<double>(currentLayer)) * falloffMult;
}

// ============================================================================
// Colour preparation
//
// BUGFIX (issue #3): vanilla keeps TWO distinct colours alive through
// Draw_In_House_Color, and the previous backport collapsed them into one.
//
//   v60 (Full)    @ 0x550C90 / 0x550CB4 — the prepared colour
//                 IsSupported ? clamp(2 * InnerColor) : InnerColor
//                 Consumed by the D3D triangle Set_Color (@ 0x55102A) and by
//                 the software centre line (@ 0x5511E5).
//
//   v56 (Working) @ 0x550CA1 / 0x550CCA — the thickness-loop start colour
//                 IsSupported ? Full : (InnerColor >> 1)
//
// The previous backport fed raw InnerColor to both the D3D colour and the
// centre line, so IsSupported beams rendered at 1x instead of the intended
// clamped 2x. Prism support beams looked dim under every renderer.
// ============================================================================
FakeLaserDrawClass::PreparedColors FakeLaserDrawClass::_PrepareDrawColors() const
{
	PreparedColors out {};

	if (IsSupported)
	{
		// 0x550C55 .. 0x550C90: shl 1, clamp each channel to 0xFF.
		out.Full.R = static_cast<unsigned char>(std::min(2 * static_cast<int>(InnerColor.R), 255));
		out.Full.G = static_cast<unsigned char>(std::min(2 * static_cast<int>(InnerColor.G), 255));
		out.Full.B = static_cast<unsigned char>(std::min(2 * static_cast<int>(InnerColor.B), 255));

		// 0x550CA1 / 0x550CA6: working colour starts AT the doubled value.
		out.Working = out.Full;
	}
	else
	{
		// 0x550CAA .. 0x550CD2: full = raw inner, working = inner >> 1.
		out.Full = InnerColor;

		out.Working.R = static_cast<unsigned char>(InnerColor.R >> 1);
		out.Working.G = static_cast<unsigned char>(InnerColor.G >> 1);
		out.Working.B = static_cast<unsigned char>(InnerColor.B >> 1);
	}

	return out;
}


// ============================================================================
// DrawInHouseColor - Draw house-color laser with thickness (backported)
//
// Backported from LaserDrawClass::Draw_In_House_Color with smooth falloff.
// This is the function that handles IsHouseColor=true and IsSingleColor lasers.
//
// Changes from original:
// - Smooth exponential falloff replaces harsh >>1 halving
// - Uses CalculateSmoothFalloff for gradual color reduction per layer
// - Captures maxColor before the thickness loop for falloff base
// ============================================================================
void FakeLaserDrawClass::_DrawInHouseColor()
{
#ifdef LASERDRAWDEBUG
	Debug::Log("[LaserDraw] DrawInHouseColor @ %p (Thickness=%d, Supported=%d, Inner: R=%d G=%d B=%d)\n",
		this, Thickness, static_cast<int>(IsSupported),
		InnerColor.R, InnerColor.G, InnerColor.B);
#endif

	_InitializeDirectionCoords();

	const unsigned int direction = _CalculateDirectionIndex(Source, Target);

	Point2D ptSource = TacticalClass::Instance->CoordsToClient(Source);
	Point2D ptTarget = TacticalClass::Instance->CoordsToClient(Target);

	// 0x550BA7 .. 0x550BC6
	const int zSource = ZAdjust - Game::AdjustHeight(Source.Z) - 2;
	const int zTarget = -2 - Game::AdjustHeight(Target.Z);

	// DIFF: vanilla gates on (CurrentFPS >= GetMinFrameRate() &&
	// Options.DetailLevel != 0) @ 0x550BCA. Unified into the shared helper at
	// the user's request; the FPS component is expected to live there.
	const bool useHighQuality = FakeRulesClass::DetailsCurrentlyEnabled();

	// 0x550BEA .. 0x550C2C
	float intensity = 1.0f;

	if (Fades)
	{
		const int elapsed = Duration - Progress.Stage;
		const float delta = StartIntensity - EndIntensity;
		intensity = (delta * static_cast<float>(elapsed) / static_cast<float>(Duration)) + EndIntensity;
	}

	const int ratio = static_cast<int>(intensity * 255.0f);

	// BUGFIX (issue #3): keep both colours, see _PrepareDrawColors.
	const PreparedColors colors = _PrepareDrawColors();
	ColorStruct workingColor = colors.Working;

	// Reference colour the smooth falloff decays from.
	// BUGFIX (issue #4): this is re-based after the IsSupported layer-1 reset
	// so layers 2+ cannot come out brighter than layer 1.
	ColorStruct falloffBase = colors.Working;

	// 0x550CD4 .. 0x550D06: all four endpoints seeded from the two screen
	// points before any offset is applied.
	Point2D line1Start = ptSource;
	Point2D line1End = ptSource;
	Point2D line2Start = ptTarget;
	Point2D line2End = ptTarget;

	// NOTE: (direction & 1) selects directions 1/3/5/7, which are the
	// AXIS-ALIGNED pairs, not the diagonals. Vanilla's branch test is
	// `sz[0] != 0.0` where sz[0] = direction & 1 (@ 0x550D25), and the
	// non-zero branch adds BOTH components. Naming corrected from the previous
	// backport's inverted `isDiagonal`; control flow is unchanged.
	const bool addsBothAxes = (direction & 1) != 0;

	const bool d3dActive = Game::bDirect3DIsUseable.get();

	// EXTENSION: clamped to Thickness here — on this path the core is a subset
	// of the same loop, so a core wider than the beam is meaningless. On the
	// non-house path the two are genuinely independent and no clamp applies.
	const int coreThickness = std::clamp(_GetCoreThickness(), 1, std::max(1, Thickness));

	if (Thickness >= 1)
	{
		for (int layer = 1; layer <= Thickness; ++layer)
		{
			const auto& offsets = HouseCoords[direction];

			// 0x550D35 .. 0x550E21
			if (addsBothAxes)
			{
				line1Start.X += offsets[0].X;
				line1Start.Y += offsets[0].Y;
				line1End.X += offsets[1].X;
				line1End.Y += offsets[1].Y;
				line2Start.X += offsets[0].X;
				line2Start.Y += offsets[0].Y;
				line2End.X += offsets[1].X;
				line2End.Y += offsets[1].Y;
			}
			else if (layer & 1)
			{
				line1Start.X += offsets[0].X;
				line1End.Y += offsets[1].Y;
				line2Start.X += offsets[0].X;
				line2End.Y += offsets[1].Y;
			}
			else
			{
				line1Start.Y += offsets[0].Y;
				line1End.X += offsets[1].X;
				line2Start.Y += offsets[0].Y;
				line2End.X += offsets[1].X;
			}

			// 0x550E25: under D3D the loop runs for its GEOMETRY side effects
			// only. No drawing, no dim-out break — the four endpoints simply
			// accumulate, and Thickness becomes the width of the triangle quad
			// emitted below. Removing this would render house lasers 1px wide
			// under D3D.
			if (d3dActive)
				continue;

			if (useHighQuality)
			{
				// 0x550E3A / 0x550E64
				DSurface::Temp->DrawRGBMultiplyingLine_AZ(
					DSurface::ViewBounds.operator->(),
					&line1Start, &line2Start,
					&workingColor, intensity,
					zSource, zTarget);

				DSurface::Temp->DrawRGBMultiplyingLine_AZ(
					DSurface::ViewBounds.operator->(),
					&line1End, &line2End,
					&workingColor, intensity,
					zSource, zTarget);
			}
			else
			{
				// 0x550E8F .. 0x550F44
				ColorStruct adjusted = workingColor;
				static constexpr ColorStruct white { 255, 255, 255 };
				adjusted.Adjust(ratio, white);
				const unsigned int packed = adjusted.ToInit();

				DSurface::Temp->DrawLineColor_AZ(
					DSurface::ViewBounds(),
					line1Start, line2Start,
					packed, zSource, zTarget, false);

				DSurface::Temp->DrawLineColor_AZ(
					DSurface::ViewBounds(),
					line1End, line2End,
					packed, zSource, zTarget, false);
			}

			// 0x550F47: IsSupported drops back to the undoubled inner colour
			// after the first (bright) layer.
			if (IsSupported && layer == 1)
			{
				workingColor = InnerColor;

				// BUGFIX (issue #4): without this the falloff keeps decaying
				// from clamp(2*Inner), so layer 2 renders brighter than the
				// layer 1 we just drew and the dim-out break is postponed for
				// several extra layers. This is what makes supported lasers
				// balloon in thickness.
				if constexpr (LaserDrawConfig::RebaseFalloffAfterSupportedReset)
					falloffBase = workingColor;

				continue;
			}

			// EXTENSION: CoreThickness flat top.
			//
			// Hold the working colour undecayed for the first coreThickness
			// layers, so the beam has a solid centre of a controllable width
			// instead of decaying from layer 1. Generalises the IsSupported
			// mechanic above from a hardcoded single layer to N.
			//
			// coreThickness == 1 skips this entirely and the curve is
			// unchanged from the previous revision.
			if (layer < coreThickness)
				continue;

			// 0x550F6A .. 0x550FB9
			// DIFF: vanilla halves (>>1); smooth curve substituted.
			//
			// The falloff exponent is rebased so decay begins at the first
			// post-core layer rather than at absolute layer 1 — otherwise a
			// wide core would start its decay already part-way down the curve
			// and the beam would terminate early.
			const int decayLayer = layer - (coreThickness - 1);
			const double mult = _CalculateSmoothFalloff(Thickness - (coreThickness - 1), decayLayer);

			workingColor.R = static_cast<unsigned char>(mult * falloffBase.R);
			workingColor.G = static_cast<unsigned char>(mult * falloffBase.G);
			workingColor.B = static_cast<unsigned char>(mult * falloffBase.B);

			// 0x550F91 .. 0x550FA7: threshold is 8 under high quality, 64
			// otherwise (neg/sbb/and 0FFFFFFC8h/add 40h).
			const unsigned int threshold = useHighQuality ? 8u : 64u;

			if (workingColor.R < threshold && workingColor.G < threshold && workingColor.B < threshold)
				break;
		}
	}

	// ------------------------------------------------------------------
	// 0x550FD5: D3D triangle pair, or the software centre line.
	// ------------------------------------------------------------------
	if (d3dActive && DSurface::CD3DTriangleInstance() && ZBuffer::Instance.get())
	{
		// 0x550FFF .. 0x55108F
		// ZBuffer layout confirmed: Area @ +0x00 (so Area.Y @ +0x04),
		// MaxValue @ +0x24. Vanilla performs the add in 16-bit registers, but
		// the whole expression is masked to 0xFFFF afterwards and subtraction's
		// low 16 bits depend only on the operands' low 16 bits, so the wider
		// arithmetic here is equivalent.
		const int zMax = ZBuffer::Instance->MaxValue;
		const int areaY = ZBuffer::Instance->Area.Y;
		const int viewportY = DSurface::ViewBounds->Y;

		const int zValSource = zSource + zMax + areaY - ptSource.Y - viewportY;
		const int zValTarget = zTarget + zMax + areaY - ptTarget.Y - viewportY;

		// fild is a qword load with the high dword explicitly zeroed
		// (@ 0x551003 / 0x551012), so the masked value is unsigned.
		const float szSource = static_cast<float>(zValSource & 0xFFFF) * 0.000015259022f;
		const float szTarget = static_cast<float>(zValTarget & 0xFFFF) * 0.000015259022f;

		// BUGFIX (issue #3): reads the PREPARED colour (v60), not raw
		// InnerColor. Under IsSupported this is clamp(2 * InnerColor).
		const unsigned char red = static_cast<unsigned char>((ratio * colors.Full.R) >> 8);
		const unsigned char green = static_cast<unsigned char>((ratio * colors.Full.G) >> 8);
		const unsigned char blue = static_cast<unsigned char>((ratio * colors.Full.B) >> 8);

		CD3DTriangle tri1, tri2;
		tri1.Set_Color(red, green, blue);
		tri2.Set_Color(red, green, blue);

		// 0x5510B1 .. 0x551198 — vertex order and UVs verified call-by-call.
		tri1.Set_Coords(0, static_cast<float>(line1Start.X), static_cast<float>(line1Start.Y), szSource, 0.0f, 0.0f);
		tri1.Set_Coords(1, static_cast<float>(line1End.X), static_cast<float>(line1End.Y), szSource, 0.0f, 1.0f);
		tri1.Set_Coords(2, static_cast<float>(line2Start.X), static_cast<float>(line2Start.Y), szTarget, 1.0f, 0.0f);

		tri2.Set_Coords(0, static_cast<float>(line1End.X), static_cast<float>(line1End.Y), szSource, 0.0f, 1.0f);
		tri2.Set_Coords(1, static_cast<float>(line2End.X), static_cast<float>(line2End.Y), szTarget, 1.0f, 1.0f);
		tri2.Set_Coords(2, static_cast<float>(line2Start.X), static_cast<float>(line2Start.Y), szTarget, 1.0f, 0.0f);

		DSurface::CD3DTriangleInstance->Add(&tri1);
		DSurface::CD3DTriangleInstance->Add(&tri2);

		return;
	}

	// BUGFIX (issue #3): centre line also uses the prepared colour.
	ColorStruct centerColor = colors.Full;

	if (useHighQuality)
	{
		// 0x5511D6
		DSurface::Temp->DrawRGBMultiplyingLine_AZ(
			DSurface::ViewBounds.operator->(),
			&ptSource, &ptTarget,
			&centerColor, intensity,
			zSource, zTarget);
	}
	else
	{
		// 0x55120D
		static constexpr ColorStruct white { 255, 255, 255 };
		centerColor.Adjust(ratio, white);

		DSurface::Temp->DrawLineColor_AZ(
			DSurface::ViewBounds(),
			ptSource, ptTarget,
			centerColor.ToInit(), zSource, zTarget, false);
	}
}

// ============================================================================
// DrawLaser - Main draw function (backported from LaserDrawClass::Draw)
//
// MAJOR CHANGE: Now supports Thickness for non-house-color (multicolored)
// lasers too! Previously Thickness was only respected for IsHouseColor.
//
// For multicolored lasers with Thickness > 1, we draw the outer color
// layers with thickness, similar to house-color behavior.
//
// Flow:
// 1. If Duration <= 0, skip
// 2. If IsHouseColor, delegate to DrawInHouseColor
// 3. Otherwise, draw the multicolored laser:
//    a. Calculate direction, screen coords, z-depths
//    b. Compute random spread for outer color
//    c. Draw based on detail level (high/low quality)
//    d. NEW: Apply thickness layers for outer color lines
// ============================================================================
void FakeLaserDrawClass::_DrawLaser()
{
	// 0x550268
	if (Duration <= 0)
		return;

	// EXTENSION (issue #7): vanilla Draw performs no fog test.
	if constexpr (LaserDrawConfig::EnableFogOfWarCulling)
	{
		if (ScenarioClass::Instance->SpecialFlags.StructEd.FogOfWar)
		{
			auto const pMap = MapClass::Instance();

			if (pMap->IsLocationFogged(Source) && pMap->IsLocationFogged(Target))
				return;
		}
	}

#ifdef LASERDRAWDEBUG
	Debug::Log("[LaserDraw] DrawLaser @ %p (HouseColor=%d, Thickness=%d, Duration=%d, Stage=%d)\n",
		this, IsHouseColor, Thickness, Duration, Progress.Stage);
#endif

	// 0x550274
	if (IsHouseColor)
	{
		_DrawInHouseColor();
		return;
	}

	// 0x5503AE: DontDraw — blink "off" phase.
	if (BlinkState)
		return;

	_InitializeDirectionCoords();

	const unsigned int direction = _CalculateDirectionIndex(Source, Target);

	Point2D ptSource = TacticalClass::Instance->CoordsToClient(Source);
	Point2D ptTarget = TacticalClass::Instance->CoordsToClient(Target);

	// 0x550435 .. 0x550457
	const int zSource = ZAdjust - Game::AdjustHeight(Source.Z) - 2;
	const int zTarget = -2 - Game::AdjustHeight(Target.Z);

	// 0x550459 .. 0x550483
	const bool hasOuterColor = (OuterColor.R != 0 || OuterColor.G != 0 || OuterColor.B != 0);

	// 0x55048F .. 0x5505AE
	ColorStruct outerDrawColor {};
	unsigned int outerPacked = 0;

	if (hasOuterColor)
	{
		const int spreadR = Random2Class::NonCriticalRandomNumber->RandomRanged(
			-static_cast<int>(OuterSpread.R), static_cast<int>(OuterSpread.R));
		const int spreadG = Random2Class::NonCriticalRandomNumber->RandomRanged(
			-static_cast<int>(OuterSpread.G), static_cast<int>(OuterSpread.G));
		const int spreadB = Random2Class::NonCriticalRandomNumber->RandomRanged(
			-static_cast<int>(OuterSpread.B), static_cast<int>(OuterSpread.B));

		// sets/dec/and clamps negatives to 0, then cmp 0FFh clamps the top.
		const int r = std::clamp(static_cast<int>(OuterColor.R) + spreadR, 0, 255);
		const int g = std::clamp(static_cast<int>(OuterColor.G) + spreadG, 0, 255);
		const int b = std::clamp(static_cast<int>(OuterColor.B) + spreadB, 0, 255);

		outerDrawColor = {
			static_cast<unsigned char>(r),
			static_cast<unsigned char>(g),
			static_cast<unsigned char>(b)
		};

		outerPacked = outerDrawColor.ToInit();
	}

	// 0x5505E8 .. 0x55060D
	float intensity = 1.0f;

	if (Fades)
	{
		const int elapsed = Duration - Progress.Stage;
		const float delta = StartIntensity - EndIntensity;
		intensity = (delta * static_cast<float>(elapsed) / static_cast<float>(Duration)) + EndIntensity;
	}

	const int ratio = static_cast<int>(intensity * 255.0f);

	// 0x5505C4 .. 0x5505E4: per-channel non-zero flags feed the subtractive
	// line's three bool arguments.
	const bool hasRed = (InnerColor.R != 0);
	const bool hasGreen = (InnerColor.G != 0);
	const bool hasBlue = (InnerColor.B != 0);

	// 0x550611: vanilla gates on Options.DetailLevel alone here — the
	// GetMinFrameRate() test exists ONLY in Draw_In_House_Color.
	// DIFF: unified helper used in both, per user request.
	const bool useHighQuality = FakeRulesClass::DetailsCurrentlyEnabled();

	// BUGFIX (issue #1): non-house path uses Draw_Coords, not the house table.
	const auto& baseOffsets = OuterCoords[direction];

	// Issue #6: vanilla pushes a literal 1.0f for the outer subtractive lines.
	const float outerIntensity = LaserDrawConfig::VanillaOuterLineIntensity ? 1.0f : intensity;

	// ------------------------------------------------------------------
	// Base draw — three lines, exactly as vanilla.
	// ------------------------------------------------------------------
	if (useHighQuality)
	{
		if (Blinks)
		{
			// 0x55062A: inner centre line gets the REAL intensity.
			ColorStruct innerCopy = InnerColor;
			DSurface::Temp->DrawSubtractiveLine_AZB(
				DSurface::ViewBounds(),
				ptSource, ptTarget,
				innerCopy,
				zSource, zTarget,
				false, true, true, true,
				intensity);

			if (hasOuterColor)
			{
				// 0x550664 / 0x5506D5 — both push 3F800000h.
				Point2D outerSrc0 { ptSource.X + baseOffsets[0].X, ptSource.Y + baseOffsets[0].Y };
				Point2D outerTgt0 { ptTarget.X + baseOffsets[0].X, ptTarget.Y + baseOffsets[0].Y };

				ColorStruct outerCopy0 = outerDrawColor;
				DSurface::Temp->DrawSubtractiveLine_AZB(
					DSurface::ViewBounds(),
					outerSrc0, outerTgt0,
					outerCopy0,
					zSource, zTarget,
					false, true, true, true,
					outerIntensity);

				Point2D outerSrc1 { ptSource.X + baseOffsets[1].X, ptSource.Y + baseOffsets[1].Y };
				Point2D outerTgt1 { ptTarget.X + baseOffsets[1].X, ptTarget.Y + baseOffsets[1].Y };

				ColorStruct outerCopy1 = outerDrawColor;
				DSurface::Temp->DrawSubtractiveLine_AZB(
					DSurface::ViewBounds(),
					outerSrc1, outerTgt1,
					outerCopy1,
					zSource, zTarget,
					false, true, true, true,
					outerIntensity);
			}
		}
		else if (hasOuterColor)
		{
			// 0x55074B .. 0x550841 — RGB-multiplying, all three at `intensity`.
			Point2D outerSrc0 { ptSource.X + baseOffsets[0].X, ptSource.Y + baseOffsets[0].Y };
			Point2D outerTgt0 { ptTarget.X + baseOffsets[0].X, ptTarget.Y + baseOffsets[0].Y };

			DSurface::Temp->DrawRGBMultiplyingLine_AZ(
				DSurface::ViewBounds.operator->(), &outerSrc0, &outerTgt0,
				&outerDrawColor, intensity, zSource, zTarget);

			Point2D outerSrc1 { ptSource.X + baseOffsets[1].X, ptSource.Y + baseOffsets[1].Y };
			Point2D outerTgt1 { ptTarget.X + baseOffsets[1].X, ptTarget.Y + baseOffsets[1].Y };

			DSurface::Temp->DrawRGBMultiplyingLine_AZ(
				DSurface::ViewBounds.operator->(), &outerSrc1, &outerTgt1,
				&outerDrawColor, intensity, zSource, zTarget);

			ColorStruct innerCopy = InnerColor;
			DSurface::Temp->DrawRGBMultiplyingLine_AZ(
				DSurface::ViewBounds.operator->(), &ptSource, &ptTarget,
				&innerCopy, intensity, zSource, zTarget);
		}
		else
		{
			// 0x55084C — per-channel flags instead of hardcoded true.
			ColorStruct innerFade = InnerColor;
			DSurface::Temp->DrawSubtractiveLine_AZB(
				DSurface::ViewBounds(),
				ptSource, ptTarget,
				innerFade,
				zSource, zTarget,
				false, hasRed, hasGreen, hasBlue,
				intensity);
		}
	}
	else
	{
		// 0x55088B: raw shift-pack, NO Adjust(ratio, white) on this path.
		const unsigned int innerPacked =
			ColorStruct(InnerColor.R, InnerColor.G, InnerColor.B).ToInit();

		DSurface::Temp->DrawLineColor_AZ(
			DSurface::ViewBounds(),
			ptSource, ptTarget,
			innerPacked, zSource, zTarget, false);

		if (hasOuterColor)
		{
			Point2D outerSrc0 { ptSource.X + baseOffsets[0].X, ptSource.Y + baseOffsets[0].Y };
			Point2D outerTgt0 { ptTarget.X + baseOffsets[0].X, ptTarget.Y + baseOffsets[0].Y };

			DSurface::Temp->DrawLineColor_AZ(
				DSurface::ViewBounds(),
				outerSrc0, outerTgt0,
				outerPacked, zSource, zTarget, false);

			Point2D outerSrc1 { ptSource.X + baseOffsets[1].X, ptSource.Y + baseOffsets[1].Y };
			Point2D outerTgt1 { ptTarget.X + baseOffsets[1].X, ptTarget.Y + baseOffsets[1].Y };

			DSurface::Temp->DrawLineColor_AZ(
				DSurface::ViewBounds(),
				outerSrc1, outerTgt1,
				outerPacked, zSource, zTarget, false);
		}
	}

	// ------------------------------------------------------------------
	// EXTENSION: thickness for non-house lasers.
	//
	// Vanilla Draw never reads __Thickness — there is no [ebx+1Ch] access
	// anywhere in 0x550260-0x5509D1. Everything below has no vanilla
	// counterpart.
	// ------------------------------------------------------------------
	if constexpr (!LaserDrawConfig::EnableNonHouseThickness)
		return;

	// EXTENSION: NOT clamped to Thickness.
	//
	// On this path Thickness only ever affected the outer glow, so a fat core
	// with no glow (CoreThickness = 6, Thickness = 1) is a legitimate and
	// useful configuration. The loop below runs to whichever is larger.
	const int coreThickness = LaserDrawConfig::EnableNonHouseCoreThickness
		? _GetCoreThickness()
		: 1;

	const int layerCount = std::max(Thickness, coreThickness);

	if (layerCount <= 1)
		return;

	// EXTENSION: vanilla's non-house path has no D3D triangle branch at all —
	// it always goes through software line draws — so running the loop under
	// D3D is safe here. The previous backport gated on !D3DIsUseable, which
	// made thickness a no-op for the majority of players and produced the
	// "thickness only affects HouseColor lasers" symptom.
	if (!LaserDrawConfig::NonHouseThicknessUnderD3D && Game::bDirect3DIsUseable.get())
		return;

	const bool widenCore = LaserDrawConfig::EnableNonHouseCoreThickness && coreThickness > 1;

	if (!hasOuterColor && !widenCore)
		return;

	const bool addsBothAxes = (direction & 1) != 0;

	// Outer glow endpoints start at the base offset positions already drawn.
	Point2D outer1Start { ptSource.X + baseOffsets[0].X, ptSource.Y + baseOffsets[0].Y };
	Point2D outer1End { ptSource.X + baseOffsets[1].X, ptSource.Y + baseOffsets[1].Y };
	Point2D outer2Start { ptTarget.X + baseOffsets[0].X, ptTarget.Y + baseOffsets[0].Y };
	Point2D outer2End { ptTarget.X + baseOffsets[1].X, ptTarget.Y + baseOffsets[1].Y };

	// EXTENSION: separate endpoint set for the core, seeded at the UNOFFSET
	// screen points. Without this the core stays exactly 1px no matter how
	// high Thickness goes, because inner and outer are separate draws in the
	// non-house path and only the outer participates in the offset loop.
	Point2D core1Start = ptSource;
	Point2D core1End = ptSource;
	Point2D core2Start = ptTarget;
	Point2D core2End = ptTarget;

	ColorStruct layerColor = outerDrawColor;
	const unsigned int threshold = useHighQuality ? 8u : 64u;

	for (int layer = 2; layer <= layerCount; ++layer)
	{
		const auto& offsets = baseOffsets;

		if (addsBothAxes)
		{
			outer1Start.X += offsets[0].X;
			outer1Start.Y += offsets[0].Y;
			outer1End.X += offsets[1].X;
			outer1End.Y += offsets[1].Y;
			outer2Start.X += offsets[0].X;
			outer2Start.Y += offsets[0].Y;
			outer2End.X += offsets[1].X;
			outer2End.Y += offsets[1].Y;

			core1Start.X += offsets[0].X;
			core1Start.Y += offsets[0].Y;
			core1End.X += offsets[1].X;
			core1End.Y += offsets[1].Y;
			core2Start.X += offsets[0].X;
			core2Start.Y += offsets[0].Y;
			core2End.X += offsets[1].X;
			core2End.Y += offsets[1].Y;
		}
		else if (layer & 1)
		{
			outer1Start.X += offsets[0].X;
			outer1End.Y += offsets[1].Y;
			outer2Start.X += offsets[0].X;
			outer2End.Y += offsets[1].Y;

			core1Start.X += offsets[0].X;
			core1End.Y += offsets[1].Y;
			core2Start.X += offsets[0].X;
			core2End.Y += offsets[1].Y;
		}
		else
		{
			outer1Start.Y += offsets[0].Y;
			outer1End.X += offsets[1].X;
			outer2Start.Y += offsets[0].Y;
			outer2End.X += offsets[1].X;

			core1Start.Y += offsets[0].Y;
			core1End.X += offsets[1].X;
			core2Start.Y += offsets[0].Y;
			core2End.X += offsets[1].X;
		}

		// EXTENSION: widen the core for the first coreThickness layers at full
		// inner colour, so the beam reads as a solid rod rather than a shell.
		// No falloff is applied here — the core is deliberately flat; the glow
		// below is what fades.
		if (widenCore && layer <= coreThickness)
		{
			ColorStruct coreColor = InnerColor;

			if (useHighQuality)
			{
				DSurface::Temp->DrawRGBMultiplyingLine_AZ(
					DSurface::ViewBounds.operator->(), &core1Start, &core2Start,
					&coreColor, intensity, zSource, zTarget);
				DSurface::Temp->DrawRGBMultiplyingLine_AZ(
					DSurface::ViewBounds.operator->(), &core1End, &core2End,
					&coreColor, intensity, zSource, zTarget);
			}
			else
			{
				const unsigned int corePacked = coreColor.ToInit();

				DSurface::Temp->DrawLineColor_AZ(
					DSurface::ViewBounds(), core1Start, core2Start,
					corePacked, zSource, zTarget, false);
				DSurface::Temp->DrawLineColor_AZ(
					DSurface::ViewBounds(), core1End, core2End,
					corePacked, zSource, zTarget, false);
			}
		}

		// The glow stops at Thickness even when the core runs longer, so a
		// CoreThickness > Thickness configuration produces a fat solid beam
		// with a thin halo rather than extending the halo to match.
		if (!hasOuterColor || layer > Thickness)
			continue;

		// BUGFIX (issue #5): the falloff is evaluated BEFORE drawing on this
		// path, so passing `layer` directly makes the final layer exactly
		// black and trips the dim-out break without ever drawing it — which
		// made Thickness == 2 identical to Thickness == 1. Offsetting by one
		// puts this loop on the same curve _DrawInHouseColor uses, where the
		// first applied falloff step is _CalculateSmoothFalloff(T, 1).
		const int falloffLayer = LaserDrawConfig::FixFalloffFinalLayer ? (layer - 1) : layer;
		const double mult = _CalculateSmoothFalloff(Thickness, falloffLayer);

		layerColor.R = static_cast<unsigned char>(mult * outerDrawColor.R);
		layerColor.G = static_cast<unsigned char>(mult * outerDrawColor.G);
		layerColor.B = static_cast<unsigned char>(mult * outerDrawColor.B);

		if (layerColor.R < threshold && layerColor.G < threshold && layerColor.B < threshold)
			break;

		if (useHighQuality)
		{
			DSurface::Temp->DrawRGBMultiplyingLine_AZ(
				DSurface::ViewBounds.operator->(), &outer1Start, &outer2Start,
				&layerColor, intensity, zSource, zTarget);
			DSurface::Temp->DrawRGBMultiplyingLine_AZ(
				DSurface::ViewBounds.operator->(), &outer1End, &outer2End,
				&layerColor, intensity, zSource, zTarget);
		}
		else
		{
			ColorStruct adjusted = layerColor;
			static constexpr ColorStruct white { 255, 255, 255 };
			adjusted.Adjust(ratio, white);
			const unsigned int packed = adjusted.ToInit();

			DSurface::Temp->DrawLineColor_AZ(
				DSurface::ViewBounds(), outer1Start, outer2Start,
				packed, zSource, zTarget, false);
			DSurface::Temp->DrawLineColor_AZ(
				DSurface::ViewBounds(), outer1End, outer2End,
				packed, zSource, zTarget, false);
		}
	}
}

void LaserDrawClassExtData::Clear()
{
	Array.clear();
}

void LaserDrawClassExtData::PointerExpired(void* ptr, bool removed)
{
	for (auto& _item : Array) {
		if (_item) {
			_item->InvalidatePointer((AbstractClass*)ptr, removed);
		}
	}
}

// ============================================================================
// Static member initialization
// ============================================================================
Point2D FakeLaserDrawClass::DrawCoords[8][2];

// ============================================================================
// FakeHouseClass::_InitLaserColor
// Backported from HouseClass::init_laser_color (0x50BA00-0x50BC90)
//
// Reads this->Color, normalizes it via NormalizeLaserColor, and stores
// the result in this->LaserColor.
//
// Note: The original function at 0x50BA00 has ftol_safe patches at
// 0x50BC66/0x50BC75/0x50BC80 and sqrtd patches at 0x50BA64/0x50BBA8
// that become dead code when this LJMP replaces the entire function.
// ============================================================================
void FakeHouseClass::_InitLaserColor()
{
#ifdef LASERDRAWDEBUG
	Debug::Log("[LaserDraw] _InitLaserColor called for HouseClass @ %p (Color: R=%d G=%d B=%d)\n",
		this, this->Color.R, this->Color.G, this->Color.B);
#endif

	this->LaserColor = FakeLaserDrawClass::_NormalizeLaserColor(this->Color);

#ifdef LASERDRAWDEBUG
	Debug::Log("[LaserDraw] _InitLaserColor result: R=%d G=%d B=%d\n",
		this->LaserColor.R, this->LaserColor.G, this->LaserColor.B);
#endif
}

// ============================================================================
// CalculateDirectionIndex - Get direction index (0-7) from source/target
//
// Computes angle between source and target in world coordinates, then
// converts to a binary angle and maps to one of 8 compass directions.
// ============================================================================
unsigned int FakeLaserDrawClass::_CalculateDirectionIndex(
	const CoordStruct& source, const CoordStruct& target)
{
	const double angle = Math::atan2(
		static_cast<double>(source.Y - target.Y),
		static_cast<double>(target.X - source.X)
	);
	const double adjusted = angle - Math::DEG90_AS_RAD;
	const int binaryAngle = static_cast<int>(adjusted * Math::BINARY_ANGLE_MAGIC);
	return (((static_cast<unsigned int>(binaryAngle) >> 12) + 1) >> 1) & 7;
}

// ============================================================================
// CalculateIntensityRatio - Compute intensity as 0-255 ratio
//
// If fading is enabled, interpolates between StartIntensity and EndIntensity
// based on current progress. Returns 255 * intensity.
// ============================================================================
int FakeLaserDrawClass::_CalculateIntensityRatio() const
{
	float intensity = 1.0f;

	if (Fades)
	{
		const int elapsed = Duration - Progress.Stage;
		const float delta = StartIntensity - EndIntensity;
		intensity = (delta * static_cast<float>(elapsed) / static_cast<float>(Duration))
			+ EndIntensity;
	}

	return static_cast<int>(intensity * 255.0f);
}

// ============================================================================
// PrepareDrawColor - Prepare the color for house-color drawing
//
// If IsSupported (AdjustColor): doubles each channel (clamped to 255)
// Otherwise: halves each channel (>>1)
// ============================================================================
ColorStruct FakeLaserDrawClass::_PrepareDrawColor() const
{
	if (IsSupported)
	{
		const unsigned int r = static_cast<unsigned int>(InnerColor.R) * 2;
		const unsigned int g = static_cast<unsigned int>(InnerColor.G) * 2;
		const unsigned int b = static_cast<unsigned int>(InnerColor.B) * 2;
		return {
			static_cast<unsigned char>(std::min(r, 255u)),
			static_cast<unsigned char>(std::min(g, 255u)),
			static_cast<unsigned char>(std::min(b, 255u))
		};
	}
	else
	{
		return {
			static_cast<unsigned char>(InnerColor.R >> 1),
			static_cast<unsigned char>(InnerColor.G >> 1),
			static_cast<unsigned char>(InnerColor.B >> 1)
		};
	}
}

// ============================================================================
// NormalizeLaserColor - Backported from HouseClass::init_laser_color
//
// Takes a raw laser color and normalizes it to create a bright, saturated
// version suitable for laser rendering. Two-pass normalization:
//   Pass 1: Normalize to magnitude 240, zero out weak channels (<96)
//   Pass 2: Normalize again to magnitude 240
//
// Bug fix: The pseudocode has a decompiler artifact with v5/v6 flags
// (checking if Blue > 255 via carry flags). We replace this with a
// proper comparison: if (blue > 255.0) blue = 255.0;
// ============================================================================
ColorStruct FakeLaserDrawClass::_NormalizeLaserColor(const ColorStruct& input)
{
	double r = static_cast<double>(input.R);
	double g = static_cast<double>(input.G);
	double b = static_cast<double>(input.B);

	// Pass 1: Normalize to magnitude 240
	double mag = Math::sqrt(r * r + g * g + b * b);

	if (mag == 0.0)
	{
		r = 255.0;
		g = 255.0;
		b = 255.0;
	}
	else
	{
		r = r * 240.0 / mag;
		if (r > 255.0)
			r = 255.0;

		g = g * 240.0 / mag;
		if (g > 255.0)
			g = 255.0;

		b = b * 240.0 / mag;
		if (b > 255.0)
			b = 255.0;

		// Zero out weak channels (threshold: 96)
		if (r < 96.0)
			r = 0.0;
		if (g < 96.0)
			g = 0.0;
		if (b < 96.0)
			b = 0.0;
	}

	// Pass 2: Re-normalize after zeroing weak channels
	mag = Math::sqrt(r * r + g * g + b * b);

	if (mag == 0.0)
	{
		r = 255.0;
		g = 255.0;
		b = 255.0;
	}
	else
	{
		r = r * 240.0 / mag;
		if (r > 255.0)
			r = 255.0;

		g = g * 240.0 / mag;
		if (g > 255.0)
			g = 255.0;

		b = b * 240.0 / mag;
		if (b > 255.0)
			b = 255.0;
	}

	return {
		static_cast<unsigned char>(static_cast<int>(r)),
		static_cast<unsigned char>(static_cast<int>(g)),
		static_cast<unsigned char>(static_cast<int>(b))
	};
}

// ============================================================================
// UpdateLaser - Per-laser AI update (backported from 0x550080)
//
// Advances the progress timer. When the timer expires, Stage increments by
// Step. Handles blinking (toggling BlinkState). When Stage >= Duration, the
// laser is expired and destroyed.
//
// This replaces the manually-inlined timer logic from the pseudocode with
// the existing ProgressTimer::Update() which does the same thing.
// ============================================================================
void FakeLaserDrawClass::_UpdateLaser()
{
#ifdef LASERDRAWDEBUG
	Debug::Log("[LaserDraw] UpdateLaser @ %p (Stage=%d, Duration=%d, Blinks=%d)\n",
		this, Progress.Stage, Duration, Blinks);
#endif

	// Advance the progress timer
	// ProgressTimer::Update() checks if timer expired, increments Stage by Step
	Progress.Update();

	// Handle blinking: toggle BlinkState each update
	if (Blinks)
	{
		BlinkState = !BlinkState;
	}

	// Check if laser has expired
	if (Progress.Stage >= Duration) {
		this->LaserDrawClass::~LaserDrawClass();
		GameDelete<false , false>(this);
	}
}

// ============================================================================
// UpdateAllLasers - Static AI for all lasers (backported from LaserDrawClass::AI)
//
// Iterates backwards through the array (because lasers may delete themselves)
// and calls UpdateLaser on each.
// ============================================================================
void FakeLaserDrawClass::_UpdateAllLasers()
{
#ifdef LASERDRAWDEBUG
	Debug::Log("[LaserDraw] UpdateAllLasers: %d lasers\n", LaserDrawClass::Array->Count);
#endif

	for (int i = LaserDrawClass::Array->Count - 1; i >= 0; --i) {
		if (auto* pLaser = static_cast<FakeLaserDrawClass*>((*LaserDrawClass::Array)[i])) {
			LaserDrawClassExtData::GetExtData(pLaser)->UpdateTracking();
			pLaser->_UpdateLaser();
		}
	}
}

// ============================================================================
// DestroyAllLasers - Destroy all lasers in global array
//
// Backported from Destroy_LaserDrawClassDVC. Removes and deletes all lasers.
// ============================================================================
void FakeLaserDrawClass::_DestroyAllLasers()
{
#ifdef LASERDRAWDEBUG
	Debug::Log("[LaserDraw] DestroyAllLasers: destroying %d lasers\n", LaserDrawClass::Array->Count);
#endif

	while (LaserDrawClass::Array->Count > 0) {
		if (auto* pLaser = (*LaserDrawClass::Array)[0]) {
			pLaser->LaserDrawClass::~LaserDrawClass();
			GameDelete<false, false>(pLaser);
		}
	}
}

// ============================================================================
// DrawAllLasers - Draw all lasers (replacement for DrawAll at 0x550240)
//
// Iterates backwards and calls DrawLaser on each laser.
// ============================================================================
void __fastcall FakeLaserDrawClass::_DrawAllLasers()
{
#ifdef LASERDRAWDEBUG
	static int s_drawFrame = 0;
	if ((s_drawFrame++ % 300) == 0) // Log every ~10 seconds at 30fps
	{
		Debug::Log("[LaserDraw] DrawAllLasers: %d lasers active\n", LaserDrawClass::Array->Count);
	}
#endif

	for (int i = LaserDrawClass::Array->Count - 1; i >= 0; --i)
	{
		auto* pLaser = static_cast<FakeLaserDrawClass*>((*LaserDrawClass::Array)[i]);
		if (pLaser)
		{
			pLaser->_DrawLaser();
		}
	}
}

//// ============================================================================
//// Hook: HouseClass::init_laser_color (0x50BA00)
//// Replaces the original RGB normalization function
//// ============================================================================
DEFINE_FUNCTION_JUMP(LJMP, 0x50BA00, FakeHouseClass::_InitLaserColor);
DEFINE_FUNCTION_JUMP(CALL, 0x6880E6, FakeHouseClass::_InitLaserColor);
DEFINE_FUNCTION_JUMP(CALL, 0x6881E7, FakeHouseClass::_InitLaserColor);

// ============================================================================
// Hook: Destroy_LaserDrawClassDVC (0x550000)
// Replaces the function that destroys all lasers in the global DVC
// ============================================================================
DEFINE_FUNCTION_JUMP(CALL, 0x534949, FakeLaserDrawClass::_DestroyAllLasers);
DEFINE_FUNCTION_JUMP(LJMP, 0x550000, FakeLaserDrawClass::_DestroyAllLasers);

// ============================================================================
// Hook: LaserDrawClass per-laser update (0x550080)
// Replaces the per-instance AI update (__thiscall)
// ============================================================================
DEFINE_FUNCTION_JUMP(LJMP, 0x550080, FakeLaserDrawClass::_UpdateLaser);

// ============================================================================
// Hook: LaserDrawClass::AI (0x550150)
// Replaces the static function that iterates all lasers and updates them
// ============================================================================
DEFINE_FUNCTION_JUMP(LJMP, 0x550150, FakeLaserDrawClass::_UpdateAllLasers);
DEFINE_FUNCTION_JUMP(CALL, 0x55B5C3, FakeLaserDrawClass::_UpdateAllLasers);

// ============================================================================
// Hook: LaserDrawClass::Draw_All (0x550240)
// Replaces the static __fastcall that iterates and draws all lasers
// ============================================================================
DEFINE_FUNCTION_JUMP(LJMP, 0x550240, FakeLaserDrawClass::_DrawAllLasers);
DEFINE_FUNCTION_JUMP(CALL, 0x6D4669, FakeLaserDrawClass::_DrawAllLasers);

//// ============================================================================
//// Hook: LaserDrawClass::Draw (0x550260)
//// Replaces the per-instance draw function (__thiscall)
//// Now supports multicolored thickness (previously only IsHouseColor)
//// ============================================================================
DEFINE_FUNCTION_JUMP(CALL, 0x550256, FakeLaserDrawClass::_DrawLaser);
DEFINE_FUNCTION_JUMP(LJMP, 0x550260, FakeLaserDrawClass::_DrawLaser);


// ============================================================================
// Hook: LaserDrawClass::Draw_In_House_Color (0x5509F0)
// Replaces the house-color laser rendering (__thiscall)
// Uses smooth exponential falloff instead of harsh >>1 halving
// ============================================================================
DEFINE_FUNCTION_JUMP(CALL, 0x55027B, FakeLaserDrawClass::_DrawInHouseColor);
DEFINE_FUNCTION_JUMP(LJMP, 0x5509F0, FakeLaserDrawClass::_DrawInHouseColor);

#ifdef LASERDRAWDEBUG
// Debug: confirm hook registration at load time
struct LaserDrawDebugInit
{
	LaserDrawDebugInit()
	{
		Debug::Log("[LaserDraw] Backported laser system loaded.\n");
		Debug::Log("[LaserDraw] Hooks active:\n");
		Debug::Log("[LaserDraw]   0x50BA00 -> FakeHouseClass::_InitLaserColor\n");
		Debug::Log("[LaserDraw]   0x550000 -> DestroyAllLasers\n");
		Debug::Log("[LaserDraw]   0x550080 -> UpdateLaser\n");
		Debug::Log("[LaserDraw]   0x550150 -> UpdateAllLasers\n");
		Debug::Log("[LaserDraw]   0x550240 -> DrawAllLasers\n");
		Debug::Log("[LaserDraw]   0x550260 -> DrawLaser\n");
		Debug::Log("[LaserDraw]   0x5509F0 -> DrawInHouseColor\n");
		Debug::Log("[LaserDraw]   CTOR/DTOR not hooked (0x54FE60/0x54FFB0)\n");
		Debug::Log("[LaserDraw]   DVC/VC functions skipped (use YRpp templates)\n");
		Debug::Log("[LaserDraw]   Multicolored thickness support enabled.\n");
	}
};
static LaserDrawDebugInit s_laserDrawDebugInit;
#endif

LaserDrawClassExtData::PendingContext LaserDrawClassExtData::Pending {};
HelperedVector<LaserDrawClassExtData*> LaserDrawClassExtData::Array;

CoordStruct LaserDrawClassExtData::GetFrozenWorldFLH(TechnoClass* pShooter) const
{
	const int savedBurstIndex = pShooter->CurrentBurstIndex;
	pShooter->CurrentBurstIndex = this->FrozenBurstIndex;
	const CoordStruct worldFLH = pShooter->GetFLH(this->WeaponIndex, this->LocalFLH.X, this->LocalFLH.Y, this->LocalFLH.Z);
	pShooter->CurrentBurstIndex = savedBurstIndex;
	return worldFLH;
}

CoordStruct LaserDrawClassExtData::ResolveLocalFLH(TechnoClass* pShooter, int weaponIdx)
{
	auto [flhFound, localFLH] = TechnoExtData::GetBurstFLH(pShooter, weaponIdx);

	if (!flhFound)
	{
		// BUGFIX: original did `pShooter->GetWeapon(weaponIdx)->FLH` with no null guard.
		if (const auto pWeaponStruct = pShooter->GetWeapon(weaponIdx))
			localFLH = pWeaponStruct->FLH;
		else
			localFLH = CoordStruct::Empty;
	}

	// SUSPECT: the old hook at 0x6FD210 additionally mirrored Y for odd burst
	// indices (`if (SavedBurstIndex % 2 != 0) FLH.Y = -FLH.Y;`), but that value
	// was written into LaserRT::SavedLocalFLH and then thrown away at 0x6FD446
	// via std::exchange without ever being read. SetLaserTrackingData always
	// recomputed the FLH itself, unmirrored. Behaviour preserved as-is
	// (unmirrored) because that is what actually ran. TechnoClass::GetFLH is
	// believed to apply burst mirroring internally, so mirroring here would
	// double-flip - VERIFY against 0x6F3300 before "fixing" this.
	return localFLH;
}

bool LaserDrawClassExtData::ResolveStopOnFirerConvert(TechnoClass* pShooter, int weaponIdx)
{
	const auto pWeaponStruct = pShooter->GetWeapon(weaponIdx);
	const auto pWeapon = pWeaponStruct ? pWeaponStruct->WeaponType : nullptr;

	// SUSPECT: vanilla path returned false (not the Rules default) when the
	// weapon slot was empty. Preserved verbatim.
	if (!pWeapon)
		return false;

	return WeaponTypeExtContainer::Instance.Find(pWeapon)->LaserPositionUpdate_StopOnFirerConvert
		.Get(FakeRulesClass::Instance()->LaserPositionUpdate_StopOnFirerConvert);
}

// ===========================================================================
// Record lifetime
// ===========================================================================

void LaserDrawClassExtData::DetachShooter()
{
	this->Shooter = nullptr;
	this->OriginalType = nullptr;
}

void LaserDrawClassExtData::ResetTracking()
{
	this->Shooter = nullptr;
	this->TrackedTarget = nullptr;
	this->OriginalType = nullptr;
	this->SavedOffset = CoordStruct::Empty;
	this->LocalFLH = CoordStruct::Empty;
	this->WeaponIndex = 0;
	this->FrozenBurstIndex = 0;
	this->FollowMode = PositionFollow::None;
	this->StopOnFirerConvert = false;
}

void LaserDrawClassExtData::AssignTracking(TechnoClass* pShooter, AbstractClass* pTarget,
	int weaponIdx, PositionFollow mode, bool ignoreShooter)
{
	const auto pLaser = this->AttachedToObject;

	if (!pLaser)
		return;

	if (ignoreShooter)
		pShooter = nullptr;

	// Garrisonable buildings move their firing origin per occupant slot, so a
	// frozen FLH would desync from the muzzle - firer follow is dropped.
	if (const auto pBuilding = cast_to<BuildingClass*>(pShooter))
	{
		if (pBuilding->Type->MaxNumberOccupants > 0)
			mode &= ~PositionFollow::Firer;
	}

	// Always wipe any prior record first - the ext outlives individual
	// assignments and the engine may re-issue on the same laser.
	this->ResetTracking();

	this->WeaponIndex = weaponIdx;
	this->FollowMode = mode;

	// DIFF (perf only): the old SetLaserTrackingData resolved FLH / burst index /
	// StopOnFirerConvert unconditionally and then discarded them unless Firer was
	// set. Same result, fewer ExtMap lookups.
	if (pShooter && (mode & PositionFollow::Firer))
	{
		this->Shooter = pShooter;
		this->LocalFLH = LaserDrawClassExtData::ResolveLocalFLH(pShooter, weaponIdx);

		if (pShooter->CurrentBurstIndex % 2 != 0)
			this->LocalFLH.Y = -this->LocalFLH.Y;

		this->FrozenBurstIndex = pShooter->CurrentBurstIndex;
		this->StopOnFirerConvert = LaserDrawClassExtData::ResolveStopOnFirerConvert(pShooter, weaponIdx);

		if (this->StopOnFirerConvert)
			this->OriginalType = pShooter->GetTechnoType();

		this->SavedOffset = pLaser->Source - this->GetFrozenWorldFLH(pShooter);
	}

	if (mode & PositionFollow::Target)
		this->TrackedTarget = flag_cast_to<ObjectClass*>(pTarget);

	// DIFF: no "don't store inert records" special case any more. The ext exists
	// either way, so an inert record costs one IsInert() test per frame instead
	// of a map insert/erase pair.
}

// ===========================================================================
// Per-frame update
// ===========================================================================
void LaserDrawClassExtData::UpdateTracking()
{
	if (this->IsInert())
		return;

	const auto pLaser = this->AttachedToObject;

	if (!pLaser)
		return;

	if (this->Shooter && this->StopOnFirerConvert && this->OriginalType)
	{
		if (this->Shooter->GetTechnoType() != this->OriginalType)
			this->DetachShooter();
	}

	if (const auto pShooter = this->Shooter)
		pLaser->Source = this->GetFrozenWorldFLH(pShooter) + this->SavedOffset;

	if (const auto pTarget = this->TrackedTarget)
		pLaser->Target = pTarget->GetTargetCoords();
}

void LaserDrawClassExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	if (this->Shooter == ptr) {
		this->Shooter = nullptr;
		this->OriginalType = nullptr;
	}

	if (this->TrackedTarget == ptr)
		this->TrackedTarget = nullptr;
}

ASMJIT_PATCH(0x54FFA4, LaserDrawClass_CTOR, 0x7)
{
	GET(LaserDrawClass*, pLaser, ESI);

	auto pLaserExt = new LaserDrawClassExtData();
		 pLaserExt->AttachedToObject = pLaser;
		 pLaser->SetPadToPtr(pLaserExt);

	return 0x0;
}

ASMJIT_PATCH(0x54FFB0, LaserDrawClass_DTOR, 7)
{
	GET(LaserDrawClass*, pLaser, ECX);

	if (auto pLaserExt = (LaserDrawClassExtData*)pLaser->GetPptrFromPad())
		delete pLaserExt;

	pLaser->CleanPad();

	return 0;
}

ASMJIT_PATCH(0x5501D7 , LaserDrawClass_Remove_InlineDTOR, 0x5) {
	GET(LaserDrawClass*, pLaser, ESI);
	pLaser->LaserDrawClass::~LaserDrawClass();
	GameDelete<false, false>(pLaser);
	return R->Origin() + 0x51;
}ASMJIT_PATCH_AGAIN(0x5500EF ,LaserDrawClass_Remove_InlineDTOR, 0x5)

ASMJIT_PATCH(0x550016 ,LaserDrawClass_Remove_InlineDTOR_BB, 0x6 ){
	GET(LaserDrawClass*, pLaser, ESI);
	pLaser->LaserDrawClass::~LaserDrawClass();
	GameDelete<false, false>(pLaser);
	return R->Origin() + 0x52;
}
