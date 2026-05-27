#pragma once

#include <LaserDrawClass.h>
#include <CoordStruct.h>
#include <ColorStruct.h>
#include <GeneralStructures.h>

// ============================================================================
// FakeLaserDrawClass - Backported and improved laser rendering
//
// Original address map:
//   0x54FE60-0x54FFAB  LaserDrawClass::LaserDrawClass (CTOR) - NOT HOOKABLE
//   0x54FFB0-0x54FFFC  LaserDrawClass::~LaserDrawClass (DTOR) - NOT HOOKABLE
//   0x550000-0x550074  Destroy_LaserDrawClassDVC (static free fn)
//   0x550080-0x550145  LaserDrawClass update (per-laser AI, __thiscall)
//   0x550150-0x550236  LaserDrawClass::AI (static, iterates all lasers)
//   0x550240-0x550260  LaserDrawClass::Draw_All (static __fastcall)
//   0x550260-0x5509D2  LaserDrawClass::Draw (__thiscall)
//   0x5509F0-0x5512B5  LaserDrawClass::Draw_In_House_Color (__thiscall)
//
// DVC/VC functions at 0x5512D0-0x5515E0 are NOT backported — already
// implemented as templates in YRpp/DynamicVectorClass.h and VectorClass.h
//
// CTOR (0x54FE60) and DTOR (0x54FFB0) cannot use DEFINE_FUNCTION_JUMP.
//
// HouseClass::init_laser_color (0x50BA00-0x50BC90) is backported as
// FakeHouseClass::_InitLaserColor in src/Ext/House/Body.h
// ============================================================================
class NOVTABLE FakeLaserDrawClass : public LaserDrawClass
{
public:
	// ========================================================================
	// Backported drawing functions
	// ========================================================================

	// Replacement for LaserDrawClass::Draw_In_House_Color (0x5509F0-0x5512B5)
	// Draws house-color (single-color) lasers with smooth thickness falloff
	void _DrawInHouseColor();

	// Replacement for LaserDrawClass::Draw (0x550260-0x5509D2)
	// Now supports thickness for multicolored lasers too
	void _DrawLaser();

	// Replacement for LaserDrawClass::Draw_All (0x550240-0x550260, static __fastcall)
	// Iterates all lasers and draws them
	static void __fastcall _DrawAllLasers();

	// ========================================================================
	// Backported lifecycle functions
	// ========================================================================

	// Per-laser AI update (0x550080-0x550145, __thiscall)
	// Advances progress timer, handles blinking, destroys expired lasers
	void _UpdateLaser();

	// Static AI that updates all lasers (0x550150-0x550236)
	static void _UpdateAllLasers();

	// Destroys all lasers in the global array (0x550000-0x550074)
	static void _DestroyAllLasers();

	// ========================================================================
	// Helper: Normalize laser color for house color display
	// Backported from HouseClass::init_laser_color (0x50BA00-0x50BC90)
	// Normalizes RGB to create a bright, saturated version for laser rendering
	// ========================================================================
	static ColorStruct _NormalizeLaserColor(const ColorStruct& input);

	// ========================================================================
	// Internal helpers
	// ========================================================================
private:
	// Direction coordinate table (8 directions x 2 offsets)
	static Point2D DrawCoords[8][2];
	static bool s_CoordsInitialized;

	// Initialize the direction offset lookup table
	static void _InitializeDirectionCoords();

	// Calculate direction index from source/target world coordinates
	static unsigned int _CalculateDirectionIndex(const CoordStruct& source, const CoordStruct& target);

	// Calculate intensity ratio (0-255) based on fade progress
	int _CalculateIntensityRatio() const;

	// Prepare the draw color (doubled if IsSupported, halved otherwise)
	ColorStruct _PrepareDrawColor() const;

	// Smooth exponential falloff for thickness layers
	// Replaces the harsh >>1 (50% per layer) with gradual falloff
	static double _CalculateSmoothFalloff(int thickness, int currentLayer);
};
