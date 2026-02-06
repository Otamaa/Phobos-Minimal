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

// ============================================================================
// Static member initialization
// ============================================================================
Point2D FakeLaserDrawClass::DrawCoords[8][2];
bool FakeLaserDrawClass::s_CoordsInitialized = false;

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
// InitializeDirectionCoords - One-time init of direction offset lookup table
//
// The table defines pixel offsets for 8 compass directions.
// Each direction has two offset vectors used for thickness expansion.
// Directions 0,4 and 2,6 are diagonal pairs; 1,5 and 3,7 are axis-aligned.
// ============================================================================
void FakeLaserDrawClass::_InitializeDirectionCoords()
{
	if (s_CoordsInitialized)
		return;

	// Direction 0: SW-NE diagonal
	DrawCoords[0][0] = { -1, -1 };
	DrawCoords[0][1] = {  1,  1 };
	// Direction 1: Vertical
	DrawCoords[1][0] = {  0, -1 };
	DrawCoords[1][1] = {  0,  1 };
	// Direction 2: NW-SE diagonal
	DrawCoords[2][0] = { -1,  1 };
	DrawCoords[2][1] = {  1, -1 };
	// Direction 3: Horizontal
	DrawCoords[3][0] = { -1,  0 };
	DrawCoords[3][1] = {  1,  0 };
	// Direction 4: SW-NE diagonal (same as 0)
	DrawCoords[4][0] = { -1, -1 };
	DrawCoords[4][1] = {  1,  1 };
	// Direction 5: Vertical (same as 1)
	DrawCoords[5][0] = {  0, -1 };
	DrawCoords[5][1] = {  0,  1 };
	// Direction 6: SE-NW diagonal (mirrored 2)
	DrawCoords[6][0] = {  1, -1 };
	DrawCoords[6][1] = { -1,  1 };
	// Direction 7: Horizontal (same as 3)
	DrawCoords[7][0] = { -1,  0 };
	DrawCoords[7][1] = {  1,  0 };

	s_CoordsInitialized = true;
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
// CalculateSmoothFalloff - Exponential falloff for thickness layers
//
// ORIGINAL: each layer halves (>>1) the color - harsh 50% steps
// NEW: smooth exponential falloff: (1 - step*layer) * (1-step)^layer
//
// This produces a much smoother transition from bright center to dim edges.
// ============================================================================
double FakeLaserDrawClass::_CalculateSmoothFalloff(int thickness, int currentLayer)
{
	if (thickness <= 1)
		return 1.0;

	const double falloffStep = 1.0 / static_cast<double>(thickness);
	const double falloffMult = GeneralUtils::SecsomeFastPow(1.0 - falloffStep, static_cast<size_t>(currentLayer));
	return (1.0 - falloffStep * static_cast<double>(currentLayer)) * falloffMult;
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
	if (Progress.Stage >= Duration)
	{
		// Remove from global array and delete
		const int idx = LaserDrawClass::Array->find(this);
		if (idx >= 0)
		{
			LaserDrawClass::Array->erase_at(idx);
		}
		GameDelete(this);
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

	for (int i = LaserDrawClass::Array->Count - 1; i >= 0; --i)
	{
		auto* pLaser = static_cast<FakeLaserDrawClass*>((*LaserDrawClass::Array)[i]);
		if (pLaser)
		{
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

	while (LaserDrawClass::Array->Count > 0)
	{
		auto* pLaser = (*LaserDrawClass::Array)[0];
		if (pLaser)
		{
			const int idx = LaserDrawClass::Array->find(pLaser);
			if (idx >= 0)
			{
				LaserDrawClass::Array->erase_at(idx);
			}
			GameDelete(pLaser);
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
	if (Duration <= 0)
		return;

#ifdef LASERDRAWDEBUG
	Debug::Log("[LaserDraw] DrawLaser @ %p (HouseColor=%d, Thickness=%d, Duration=%d, Stage=%d)\n",
		this, IsHouseColor, Thickness, Duration, Progress.Stage);
#endif

	if (IsHouseColor)
	{
		_DrawInHouseColor();
		return;
	}

	// Skip if blinking and currently in "off" state
	if (BlinkState)
		return;

	_InitializeDirectionCoords();

	// Calculate direction index from world coordinates
	const unsigned int direction = _CalculateDirectionIndex(Source, Target);

	// Convert world to screen coordinates
	Point2D ptSource = TacticalClass::Instance->CoordsToClient(Source);
	Point2D ptTarget = TacticalClass::Instance->CoordsToClient(Target);

	// Calculate Z-depths for depth sorting
	const int zSource = ZAdjust - Game::AdjustHeight(Source.Z) - 2;
	const int zTarget = -2 - Game::AdjustHeight(Target.Z);

	// Determine if outer color exists
	const bool hasOuterColor = (OuterColor.R != 0 || OuterColor.G != 0 || OuterColor.B != 0);

	// Calculate random spread for outer color
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

		int r = std::clamp(static_cast<int>(OuterColor.R) + spreadR, 0, 255);
		int g = std::clamp(static_cast<int>(OuterColor.G) + spreadG, 0, 255);
		int b = std::clamp(static_cast<int>(OuterColor.B) + spreadB, 0, 255);

		outerDrawColor = { static_cast<unsigned char>(r),
			static_cast<unsigned char>(g), static_cast<unsigned char>(b) };
		outerPacked = outerDrawColor.ToInit();
	}

	// Calculate intensity
	float intensity = 1.0f;
	if (Fades)
	{
		const int elapsed = Duration - Progress.Stage;
		const float delta = StartIntensity - EndIntensity;
		intensity = (delta * static_cast<float>(elapsed) / static_cast<float>(Duration))
			+ EndIntensity;
	}
	const int ratio = static_cast<int>(intensity * 255.0f);

	// Check if inner color has non-zero channels (for subtractive line)
	const bool hasRed = (InnerColor.R != 0);
	const bool hasGreen = (InnerColor.G != 0);
	const bool hasBlue = (InnerColor.B != 0);

	// Determine rendering quality
	const bool useHighQuality = RulesExtData::DetailsCurrentlyEnabled();

	// ====================================================================
	// Draw the laser based on detail level
	// ====================================================================
	if (useHighQuality)
	{
		if (Blinks)
		{
			// Subtractive drawing for blinking lasers
			ColorStruct innerCopy = InnerColor;
			DSurface::Temp->DrawSubtractiveLine_AZ(
				DSurface::ViewBounds(),
				ptSource, ptTarget,
				innerCopy,
				zSource, zTarget,
				false, true, true, true,
				intensity
			);

			// Draw outer color lines if present
			if (hasOuterColor)
			{
				const auto& off0 = DrawCoords[direction][0];
				const auto& off1 = DrawCoords[direction][1];

				Point2D outerSrc0 = { ptSource.X + off0.X, ptSource.Y + off0.Y };
				Point2D outerTgt0 = { ptTarget.X + off0.X, ptTarget.Y + off0.Y };

				ColorStruct outerCopy0 = outerDrawColor;
				DSurface::Temp->DrawSubtractiveLine_AZ(
					DSurface::ViewBounds(),
					outerSrc0, outerTgt0,
					outerCopy0,
					zSource, zTarget,
					false, true, true, true,
					intensity
				);

				Point2D outerSrc1 = { ptSource.X + off1.X, ptSource.Y + off1.Y };
				Point2D outerTgt1 = { ptTarget.X + off1.X, ptTarget.Y + off1.Y };

				ColorStruct outerCopy1 = outerDrawColor;
				DSurface::Temp->DrawSubtractiveLine_AZ(
					DSurface::ViewBounds(),
					outerSrc1, outerTgt1,
					outerCopy1,
					zSource, zTarget,
					false, true, true, true,
					intensity
				);
			}
		}
		else if (hasOuterColor)
		{
			// RGB multiplying draw with outer color
			const auto& off0 = DrawCoords[direction][0];
			const auto& off1 = DrawCoords[direction][1];

			Point2D outerSrc0 = { ptSource.X + off0.X, ptSource.Y + off0.Y };
			Point2D outerTgt0 = { ptTarget.X + off0.X, ptTarget.Y + off0.Y };

			DSurface::Temp->DrawRGBMultiplyingLine_AZ(
				&DSurface::ViewBounds(), &outerSrc0, &outerTgt0,
				&outerDrawColor, intensity, zSource, zTarget
			);

			Point2D outerSrc1 = { ptSource.X + off1.X, ptSource.Y + off1.Y };
			Point2D outerTgt1 = { ptTarget.X + off1.X, ptTarget.Y + off1.Y };

			DSurface::Temp->DrawRGBMultiplyingLine_AZ(
				&DSurface::ViewBounds(), &outerSrc1, &outerTgt1,
				&outerDrawColor, intensity, zSource, zTarget
			);

			// Draw inner color center line
			ColorStruct innerCopy = InnerColor;
			DSurface::Temp->DrawRGBMultiplyingLine_AZ(
				&DSurface::ViewBounds(), &ptSource, &ptTarget,
				&innerCopy, intensity, zSource, zTarget
			);
		}
		else
		{
			// Subtractive drawing for non-outer-color (fading) lasers
			ColorStruct innerFade = InnerColor;
			DSurface::Temp->DrawSubtractiveLine_AZ(
				DSurface::ViewBounds(),
				ptSource, ptTarget,
				innerFade,
				zSource, zTarget,
				false, hasRed, hasGreen, hasBlue,
				intensity
			);
		}
	}
	else
	{
		// Low quality: simple colored line
		const unsigned int innerPacked = ColorStruct(InnerColor.R, InnerColor.G, InnerColor.B).ToInit();

		DSurface::Temp->DrawLineColor_AZ(
			DSurface::ViewBounds(),
			ptSource, ptTarget,
			innerPacked, zSource, zTarget, false
		);

		if (hasOuterColor)
		{
			const auto& off0 = DrawCoords[direction][0];
			const auto& off1 = DrawCoords[direction][1];

			Point2D outerSrc0 = { ptSource.X + off0.X, ptSource.Y + off0.Y };
			Point2D outerTgt0 = { ptTarget.X + off0.X, ptTarget.Y + off0.Y };

			DSurface::Temp->DrawLineColor_AZ(
				DSurface::ViewBounds(),
				outerSrc0, outerTgt0,
				outerPacked, zSource, zTarget, false
			);

			Point2D outerSrc1 = { ptSource.X + off1.X, ptSource.Y + off1.Y };
			Point2D outerTgt1 = { ptTarget.X + off1.X, ptTarget.Y + off1.Y };

			DSurface::Temp->DrawLineColor_AZ(
				DSurface::ViewBounds(),
				outerSrc1, outerTgt1,
				outerPacked, zSource, zTarget, false
			);
		}
	}

	// ====================================================================
	// NEW: Draw additional thickness layers for multicolored lasers
	// Previously Thickness was only used for IsHouseColor
	// Now we apply thickness to the outer color with smooth falloff
	// ====================================================================
	if (Thickness > 1 && hasOuterColor && !Game::bDirect3DIsUseable)
	{
		const bool isDiagonal = (direction & 1) != 0;

		// Start from the base outer offset positions
		Point2D line1Start = ptSource;
		Point2D line1End = ptSource;
		Point2D line2Start = ptTarget;
		Point2D line2End = ptTarget;

		// Apply the base offset (layer 0 is the already-drawn outer lines)
		const auto& baseOff0 = DrawCoords[direction][0];
		const auto& baseOff1 = DrawCoords[direction][1];
		line1Start = { ptSource.X + baseOff0.X, ptSource.Y + baseOff0.Y };
		line1End = { ptSource.X + baseOff1.X, ptSource.Y + baseOff1.Y };
		line2Start = { ptTarget.X + baseOff0.X, ptTarget.Y + baseOff0.Y };
		line2End = { ptTarget.X + baseOff1.X, ptTarget.Y + baseOff1.Y };

		ColorStruct layerColor = outerDrawColor;

		for (int layer = 2; layer <= Thickness; ++layer)
		{
			// Expand the lines outward
			const auto& offsets = DrawCoords[direction];
			if (isDiagonal)
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

			// Calculate smooth falloff for this layer
			const double mult = _CalculateSmoothFalloff(Thickness, layer);
			layerColor.R = static_cast<unsigned char>(mult * outerDrawColor.R);
			layerColor.G = static_cast<unsigned char>(mult * outerDrawColor.G);
			layerColor.B = static_cast<unsigned char>(mult * outerDrawColor.B);

			// Check if color is too dim to continue
			const unsigned int threshold = useHighQuality ? 8u : 64u;
			if (layerColor.R < threshold && layerColor.G < threshold && layerColor.B < threshold)
				break;

			if (useHighQuality)
			{
				DSurface::Temp->DrawRGBMultiplyingLine_AZ(
					&DSurface::ViewBounds(), &line1Start, &line2Start,
					&layerColor, intensity, zSource, zTarget
				);
				DSurface::Temp->DrawRGBMultiplyingLine_AZ(
					&DSurface::ViewBounds(), &line1End, &line2End,
					&layerColor, intensity, zSource, zTarget
				);
			}
			else
			{
				ColorStruct adjustedColor = layerColor;
				static constexpr ColorStruct white { 255, 255, 255 };
				adjustedColor.Adjust(ratio, white);
				const unsigned int packed = adjustedColor.ToInit();

				DSurface::Temp->DrawLineColor_AZ(
					DSurface::ViewBounds(),
					line1Start, line2Start,
					packed, zSource, zTarget, false
				);
				DSurface::Temp->DrawLineColor_AZ(
					DSurface::ViewBounds(),
					line1End, line2End,
					packed, zSource, zTarget, false
				);
			}
		}
	}
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
	Debug::Log("[LaserDraw] DrawInHouseColor @ %p (Thickness=%d, Inner: R=%d G=%d B=%d)\n",
		this, Thickness, InnerColor.R, InnerColor.G, InnerColor.B);
#endif

	_InitializeDirectionCoords();

	// Calculate direction index
	const unsigned int direction = _CalculateDirectionIndex(Source, Target);

	// Convert world to screen coordinates
	Point2D ptSource = TacticalClass::Instance->CoordsToClient(Source);
	Point2D ptTarget = TacticalClass::Instance->CoordsToClient(Target);

	// Calculate Z-depths
	const int zSource = ZAdjust - Game::AdjustHeight(Source.Z) - 2;
	const int zTarget = -2 - Game::AdjustHeight(Target.Z);

	// Determine rendering quality
	const bool useHighQuality = RulesExtData::DetailsCurrentlyEnabled();

	// Calculate intensity
	float intensity = 1.0f;
	if (Fades)
	{
		const int elapsed = Duration - Progress.Stage;
		const float delta = StartIntensity - EndIntensity;
		intensity = (delta * static_cast<float>(elapsed) / static_cast<float>(Duration))
			+ EndIntensity;
	}
	const int ratio = static_cast<int>(intensity * 255.0f);

	// Prepare colors
	ColorStruct workingColor = _PrepareDrawColor();

	// Capture max color for smooth falloff calculations
	const ColorStruct maxColor = workingColor;

	// Store original inner color (used when IsSupported and layer == 1)
	const ColorStruct innerColor = InnerColor;

	// Initialize line endpoint pairs
	Point2D line1Start = ptSource;
	Point2D line1End = ptSource;
	Point2D line2Start = ptTarget;
	Point2D line2End = ptTarget;

	// Draw thickness layers
	const bool isDiagonal = (direction & 1) != 0;

	if (Thickness >= 1)
	{
		for (int layer = 1; layer <= Thickness; ++layer)
		{
			// Apply direction offset to expand lines outward
			const auto& offsets = DrawCoords[direction];

			if (isDiagonal)
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

			if (!Game::bDirect3DIsUseable)
			{
				// Draw the lines for this layer
				if (useHighQuality)
				{
					DSurface::Temp->DrawRGBMultiplyingLine_AZ(
						&DSurface::ViewBounds(),
						&line1Start, &line2Start,
						&workingColor, intensity,
						zSource, zTarget
					);
					DSurface::Temp->DrawRGBMultiplyingLine_AZ(
						&DSurface::ViewBounds(),
						&line1End, &line2End,
						&workingColor, intensity,
						zSource, zTarget
					);
				}
				else
				{
					ColorStruct rgbWork = workingColor;
					static constexpr ColorStruct white { 255, 255, 255 };
					rgbWork.Adjust(ratio, white);
					const unsigned int packed = rgbWork.ToInit();

					DSurface::Temp->DrawLineColor_AZ(
						DSurface::ViewBounds(),
						line1Start, line2Start,
						packed, zSource, zTarget, false
					);
					DSurface::Temp->DrawLineColor_AZ(
						DSurface::ViewBounds(),
						line1End, line2End,
						packed, zSource, zTarget, false
					);
				}

				// Color falloff for next layer
				if (IsSupported && layer == 1)
				{
					// First layer with AdjustColor: reset to inner color
					workingColor = innerColor;
				}
				else
				{
					// Smooth exponential falloff using maxColor as base
					const unsigned int threshold = useHighQuality ? 8u : 64u;
					const double mult = _CalculateSmoothFalloff(Thickness, layer);

					workingColor.R = static_cast<unsigned char>(mult * maxColor.R);
					workingColor.G = static_cast<unsigned char>(mult * maxColor.G);
					workingColor.B = static_cast<unsigned char>(mult * maxColor.B);

					// Check if too dim to continue
					if (workingColor.R < threshold &&
						workingColor.G < threshold &&
						workingColor.B < threshold)
					{
						break;
					}
				}
			}
		}
	}

	// Final rendering: D3D triangle path or software center line
	if (Game::bDirect3DIsUseable && DSurface::CD3DTriangleInstance() && ZBuffer::Instance)
	{
		const short zMax = ZBuffer::Instance->MaxValue;
		const short viewportY = static_cast<short>(DSurface::ViewBounds->Y);

		const int zValSource = zSource + zMax + ZBuffer::Instance->Area.Y
			- static_cast<short>(ptSource.Y) - viewportY;
		const int zValTarget = zTarget + zMax + ZBuffer::Instance->Area.Y
			- static_cast<short>(ptTarget.Y) - viewportY;

		const float szSource = static_cast<float>(zValSource & 0xFFFF) * 0.000015259022f;
		const float szTarget = static_cast<float>(zValTarget & 0xFFFF) * 0.000015259022f;

		const int red = (ratio * innerColor.R) >> 8;
		const int green = (ratio * innerColor.G) >> 8;
		const int blue = (ratio * innerColor.B) >> 8;

		CD3DTriangle tri1, tri2;
		tri1.Set_Color(red, green, blue);
		tri2.Set_Color(red, green, blue);

		tri1.Set_Coords(0, static_cast<float>(line1Start.X), static_cast<float>(line1Start.Y), szSource, 0.0f, 0.0f);
		tri1.Set_Coords(1, static_cast<float>(line1End.X), static_cast<float>(line1End.Y), szSource, 0.0f, 1.0f);
		tri1.Set_Coords(2, static_cast<float>(line2Start.X), static_cast<float>(line2Start.Y), szTarget, 1.0f, 0.0f);

		tri2.Set_Coords(0, static_cast<float>(line1End.X), static_cast<float>(line1End.Y), szSource, 0.0f, 1.0f);
		tri2.Set_Coords(1, static_cast<float>(line2End.X), static_cast<float>(line2End.Y), szTarget, 1.0f, 1.0f);
		tri2.Set_Coords(2, static_cast<float>(line2Start.X), static_cast<float>(line2Start.Y), szTarget, 1.0f, 0.0f);

		DSurface::CD3DTriangleInstance->Add(&tri1);
		DSurface::CD3DTriangleInstance->Add(&tri2);
	}
	else
	{
		// Software center line
		ColorStruct centerColor = innerColor;
		if (useHighQuality)
		{
			DSurface::Temp->DrawRGBMultiplyingLine_AZ(
				&DSurface::ViewBounds(),
				&ptSource, &ptTarget,
				&centerColor, intensity,
				zSource, zTarget
			);
		}
		else
		{
			static constexpr ColorStruct white { 255, 255, 255 };
			centerColor.Adjust(ratio, white);
			const unsigned int packed = centerColor.ToInit();

			DSurface::Temp->DrawLineColor_AZ(
				DSurface::ViewBounds(),
				ptSource, ptTarget,
				packed, zSource, zTarget, false
			);
		}
	}
}

// ============================================================================
// Hook: HouseClass::init_laser_color (0x50BA00)
// Replaces the original RGB normalization function
// ============================================================================
DEFINE_FUNCTION_JUMP(LJMP, 0x50BA00, FakeHouseClass::_InitLaserColor);

// ============================================================================
// Hook: Destroy_LaserDrawClassDVC (0x550000)
// Replaces the function that destroys all lasers in the global DVC
// ============================================================================
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

// ============================================================================
// Hook: LaserDrawClass::Draw_All (0x550240)
// Replaces the static __fastcall that iterates and draws all lasers
// ============================================================================
DEFINE_FUNCTION_JUMP(LJMP, 0x550240, FakeLaserDrawClass::_DrawAllLasers);

// ============================================================================
// Hook: LaserDrawClass::Draw (0x550260)
// Replaces the per-instance draw function (__thiscall)
// Now supports multicolored thickness (previously only IsHouseColor)
// ============================================================================
DEFINE_FUNCTION_JUMP(LJMP, 0x550260, FakeLaserDrawClass::_DrawLaser);

// ============================================================================
// Hook: LaserDrawClass::Draw_In_House_Color (0x5509F0)
// Replaces the house-color laser rendering (__thiscall)
// Uses smooth exponential falloff instead of harsh >>1 halving
// ============================================================================
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