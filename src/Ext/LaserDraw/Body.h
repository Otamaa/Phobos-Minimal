#pragma once

#include <LaserDrawClass.h>
#include <CoordStruct.h>
#include <ColorStruct.h>
#include <GeneralStructures.h>

#include <Utilities/Enum.h>
#include <Utilities/VectorHelper.h>

class AbstractClass;
class TechnoClass;
class ObjectClass;
class TechnoTypeClass;
class PhobosStreamReader;
class PhobosStreamWriter;
class LaserDrawClassExtData
{
public:
	using base_type = LaserDrawClass;
	static COMPILETIMEEVAL const char* ClassName = "LaserDrawClassExt";
	static COMPILETIMEEVAL const char* BaseClassName = "LaserDrawClass";

	static HelperedVector<LaserDrawClassExtData*> Array;

public:

	LaserDrawClassExtData() { Array.push_back(this); }
	~LaserDrawClassExtData() { Array.remove(this); }

public:

	LaserDrawClass* AttachedToObject { nullptr };
	
	// --- position tracking -------------------------------------------------
	TechnoClass* Shooter { nullptr };
	ObjectClass* TrackedTarget { nullptr };
	TechnoTypeClass* OriginalType { nullptr };

	CoordStruct SavedOffset { CoordStruct::Empty };
	CoordStruct LocalFLH { CoordStruct::Empty };

	int WeaponIndex { 0 };
	int FrozenBurstIndex { 0 };

	PositionFollow FollowMode { PositionFollow::None };
	bool StopOnFirerConvert { false };

	// A record with neither end attached can never move the beam again.
	bool IsInert() const { return !this->Shooter && !this->TrackedTarget; }

	bool TracksShooter() const { return this->Shooter && (this->FollowMode & PositionFollow::Firer); }
	bool TracksTarget() const { return this->TrackedTarget && (this->FollowMode & PositionFollow::Target); }

	// ORIG: LaserTrackerClass::Assign / SetLaserTrackingData
	void AssignTracking(TechnoClass* pShooter, AbstractClass* pTarget,
		int weaponIdx, PositionFollow mode, bool ignoreShooter);

	// ORIG: LaserTrackerClass::Update
	void UpdateTracking();

	// ORIG: LaserTrackerClass::Remove
	void ResetTracking();

	// -----------------------------------------------------------------------
	// Transient context, valid only between TechnoClass::CreateLaser entry
	// (0x6FD210) and the LaserDrawClass allocation site (0x6FD446).
	//
	// ORIG: LaserRT::Shooter / Target / WeaponIndex / IgnoreShooter
	//       (+ SavedLocalFLH / SavedBurstIndex - see .cpp, both were dead)
	// -----------------------------------------------------------------------
	struct PendingContext
	{
		TechnoClass* Shooter { nullptr };
		AbstractClass* Target { nullptr };
		int WeaponIndex { 0 };

		// Sticky across a whole CreateLaser call; set by the shrapnel wrapper.
		bool IgnoreShooter { false };

		void Reset()
		{
			this->Shooter = nullptr;
			this->Target = nullptr;
			this->WeaponIndex = 0;
			// IgnoreShooter is intentionally NOT reset here: it is owned by the
			// caller that raised it (RAII guard below).
		}
	};

	static PendingContext Pending;

	// RAII replacement for the manual `IgnoreShooter = true; ...; = false;` pair.
	class IgnoreShooterScope final
	{
	public:
		IgnoreShooterScope() : Previous(Pending.IgnoreShooter)
		{
			Pending.IgnoreShooter = true;
		}

		~IgnoreShooterScope()
		{
			Pending.IgnoreShooter = this->Previous;
		}

		IgnoreShooterScope(const IgnoreShooterScope&) = delete;
		IgnoreShooterScope& operator = (const IgnoreShooterScope&) = delete;

	private:
		bool Previous;
	};

	// Frozen-burst FLH evaluation (restores CurrentBurstIndex on the way out).
	CoordStruct GetFrozenWorldFLH(TechnoClass* pShooter) const;

	// Drops only the shooter half.
	void DetachShooter();

	void InvalidatePointer(void* ptr, bool bRemoved);

	static CoordStruct ResolveLocalFLH(TechnoClass* pShooter, int weaponIdx);
	static bool ResolveStopOnFirerConvert(TechnoClass* pShooter, int weaponIdx);

	static LaserDrawClassExtData* GetExtData(LaserDrawClass* pLaser) { return (LaserDrawClassExtData*)pLaser->GetPptrFromPad(); }
public:

	static void Clear();
	static void PointerExpired(void* ptr, bool removed);

};
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
protected:
	FakeLaserDrawClass() = delete;
	 ~FakeLaserDrawClass() = delete;
public:
	// colours alive and the previous backport collapsed them into one.
	//
	//   Full    = v60 @ 0x550C90 / 0x550CB4
	//             IsSupported ? clamp(2 * InnerColor) : InnerColor
	//             Feeds the D3D triangle colour and the software centre line.
	//
	//   Working = v56 @ 0x550CA1 / 0x550CCA
	//             IsSupported ? Full : (InnerColor >> 1)
	//             Starting colour for the thickness loop.
// ------------------------------------------------------------------
	struct PreparedColors
	{
		ColorStruct Full {};
		ColorStruct Working {};
	};

	// ------------------------------------------------------------------
	// Direction offset tables.
	//
	// BUGFIX (issue #1): vanilla maintains TWO tables, not one. Routing both
	// draw paths through the house-colour table shifted the outer glow off
	// axis on every diagonal direction (0, 2, 4, 5, 6).
	// ------------------------------------------------------------------

	// Draw_In_House_Color_coords @ 0xABC7F8 — used by _DrawInHouseColor.
	// Renamed from DrawCoords to make the pairing explicit at every call site.
	static inline Point2D HouseCoords[8][2] {};

	// Draw_Coords @ 0xABC738 — used by _DrawLaser. New; did not exist in the
	// previous backport.
	static inline Point2D OuterCoords[8][2] {};

	static bool s_CoordsInitialized;

	// Initialize the direction offset lookup table
	static void _InitializeDirectionCoords();

	// Smooth exponential falloff for thickness layers
	// Replaces the harsh >>1 (50% per layer) with gradual falloff
	static double _CalculateSmoothFalloff(int thickness, int currentLayer);

	// Signature CHANGED: was `ColorStruct _PrepareDrawColor()`. Now returns
	// both colours. Every existing call site needs updating — a single-colour
	// return cannot express the IsSupported case correctly.
	PreparedColors _PrepareDrawColors() const;

	// Replacement for LaserDrawClass::Draw_In_House_Color (0x5509F0-0x5512B5)
	// Draws house-color (single-color) lasers with smooth thickness falloff
	void _DrawInHouseColor();

	// Replacement for LaserDrawClass::Draw (0x550260-0x5509D2)
	// Now supports thickness for multicolored lasers too
	void _DrawLaser();


	// ========================================================================
	// Backported drawing functions
	// ========================================================================



	// Replacement for LaserDrawClass::Draw_All (0x550240-0x550260, static __fastcall)
	// Iterates all lasers and draws them
	static void __fastcall _DrawAllLasers();

	// ========================================================================
	// Backported lifecycle functions
	// ========================================================================

	// Per-laser AI update (0x550080-0x550145, __thiscall)
	// Advances progress timer, handles blinking, destroys expired lasers
	void _UpdateLaser();

	int _GetCoreThickness() const;

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


	// Calculate direction index from source/target world coordinates
	static unsigned int _CalculateDirectionIndex(const CoordStruct& source, const CoordStruct& target);

	// Calculate intensity ratio (0-255) based on fade progress
	int _CalculateIntensityRatio() const;

	// Prepare the draw color (doubled if IsSupported, halved otherwise)
	ColorStruct _PrepareDrawColor() const;


};
