#pragma once

#include <algorithm>
#include <GeneralStructures.h>

// ═══════════════════════════════════════════════════════════════════════════
// FacingClass — smooth rotation toward a target direction
// ═══════════════════════════════════════════════════════════════════════════
//
//  This class does NOT store just one angle. It stores THREE things:
//    1. DesiredFacing  — where we WANT to point (the goal)
//    2. StartFacing    — where we were when the turn began (the starting point)
//    3. RotationTimer  — counts down from NumSteps to 0 as we sweep the arc
//    4. ROT            — turn rate (Raw used as a step-size per frame)
//
//  HOW SMOOTH ROTATION WORKS:
//
//    When Set_Desired() is called with a new target direction:
//      - StartFacing = current facing (snapshot of where we are now)
//      - DesiredFacing = new target
//      - Timer starts counting down from NumSteps (= angular_gap / ROT)
//
//    Each frame, Current() INTERPOLATES between StartFacing and DesiredFacing
//    based on how much time is left on the timer:
//
//      current = desired - (timeleft * gap / totalsteps)
//
//    When the timer hits 0, timeleft=0, so current = desired. Turn complete.
//
//  WHY THIS APPROACH?
//    It means you can call Current() at any point during the turn and get
//    a smooth in-between angle — no need to manually step the angle each frame.
//
//  ROT special case: if ROT.Raw == 0, turns are INSTANT (NumSteps returns 0,
//  timer never starts, Current() always returns DesiredFacing directly).

class FacingClass
{
public:
	// ── Constructors ────────────────────────────────────────────────────────

	// Default: facing North, no rotation rate (instant turns)
	COMPILETIMEEVAL explicit FacingClass() noexcept :
		DesiredFacing {}, StartFacing {}, RotationTimer {}, ROT {} { }

	// With a turn rate (higher = faster turning, max 127 per DirType clamping)
	// ROT is stored as a DirType so its Raw value = rate * 256
	COMPILETIMEEVAL explicit FacingClass(int rate) noexcept :
		DesiredFacing {}, StartFacing {}, RotationTimer {},
		ROT { (DirType)MinImpl(rate, 127) }
	{ }

	// Pre-set to a specific facing (no turn rate, instant)
	COMPILETIMEEVAL explicit FacingClass(const DirStruct& facing) noexcept :
		DesiredFacing { facing }, StartFacing {}, RotationTimer {}, ROT {} { }

	COMPILETIMEEVAL explicit FacingClass(DirType dir) noexcept :
		DesiredFacing { dir }, StartFacing {}, RotationTimer {}, ROT {} { }

	COMPILETIMEEVAL FacingClass(const FacingClass& another) noexcept
		: DesiredFacing { another.DesiredFacing }
		, StartFacing { another.StartFacing }
		, RotationTimer { another.RotationTimer }
		, ROT { another.ROT }
	{ }

	COMPILETIMEEVAL FacingClass& operator=(const FacingClass& another) noexcept
	{
		if (this != &another)
		{
			DesiredFacing = another.DesiredFacing;
			StartFacing = another.StartFacing;
			RotationTimer = another.RotationTimer;
			ROT = another.ROT;
		}
		return *this;
	}

	// ── Rotation state queries ──────────────────────────────────────────────

	// The signed angular gap between where we started and where we want to go
	// Signed short arithmetic handles wrap-around automatically (e.g. 350°→10° = +20°)
	COMPILETIMEEVAL short Difference_Raw() const
	{ return short(DesiredFacing.Raw) - short(StartFacing.Raw); }

	// Step size per frame (how many Raw units we sweep per tick)
	COMPILETIMEEVAL short turn_rate() const
	{ return short(this->ROT.Raw); }

	// Returns true while actively rotating toward a target
	COMPILETIMEEVAL bool Is_Rotating() const
	{ return turn_rate() > 0 && RotationTimer.GetTimeLeft(); }

	// Are we currently turning LEFT (counter-clockwise, i.e. gap is negative)?
	COMPILETIMEEVAL bool Is_Rotating_L() const
	{ return Is_Rotating() && Difference_Raw() < 0; }

	// Are we currently turning RIGHT (clockwise, i.e. gap is positive)?
	COMPILETIMEEVAL bool Is_Rotating_G() const
	{ return Is_Rotating() && Difference_Raw() > 0; }

	// ── Facing control ──────────────────────────────────────────────────────

	// Start rotating toward a new target direction
	// Returns false if we're already pointing there (no work needed)
	COMPILETIMEEVAL bool Set_Desired(const DirStruct& facing)
	{
		if (DesiredFacing == facing)
			return false;  // already there, nothing to do

		StartFacing = Current();   // snapshot current position as rotation start
		DesiredFacing = facing;      // record where we want to end up

		if (turn_rate() > 0)
			RotationTimer.Start(NumSteps()); // countdown = gap/rate frames

		return true;
	}

	// Teleport instantly to a direction (no gradual rotation, resets timer)
	COMPILETIMEEVAL bool Set_Current(const DirStruct& facing)
	{
		bool ret = Current() != facing;
		if (ret)
		{
			DesiredFacing = facing;
			StartFacing = facing;  // start == desired → no interpolation
		}
		RotationTimer.Start(0);      // timer = 0 → immediately expired
		return ret;
	}

	// ── Facing queries ──────────────────────────────────────────────────────

	// Where we WANT to end up (may differ from Current() during a turn)
	COMPILETIMEEVAL DirStruct Desired() const
	{ return DesiredFacing; }

	// Where we ARE RIGHT NOW, accounting for in-progress rotation
	//
	// Math:  current = desired - (stepsLeft * totalGap / totalSteps)
	//
	// When timer just started: stepsLeft ≈ totalSteps → current ≈ startFacing ✓
	// When timer at zero:      stepsLeft = 0          → current = desired     ✓
	//
	// offset: "pretend N more frames have passed than actually have"
	// (used by Next() to peek one frame ahead for smooth animation)
	COMPILETIMEEVAL DirStruct Current(int offset = 0) const
	{
		DirStruct ret = this->DesiredFacing;

		if (Is_Rotating())
		{
			short diff = Difference_Raw();   // total angular gap (signed)
			short num_steps = short(NumSteps());  // how many frames the full turn takes

			if (num_steps > 0)
			{
				int steps_left = RotationTimer.GetTimeLeft() - offset;
				// Subtract the "remaining arc" from desired to get current position
				ret.Raw = unsigned short(
					((short)ret.Raw - steps_left * diff / num_steps)
				);
			}
		}

		return ret;
	}

	static COMPILETIMEEVAL DirStruct FORCEDINLINE Current(FacingClass* facing, int offset)
	{ return facing->Current(offset); }

	// One frame ahead of Current() — useful for "where will I be next tick?"
	COMPILETIMEEVAL DirStruct Next()
	{ return Current(1); }

	// The total angular arc of the current rotation as a DirStruct
	COMPILETIMEEVAL DirStruct Difference() const
	{ return DirStruct { short(DesiredFacing.Raw) - short(StartFacing.Raw) }; }

	// Change turn rate mid-flight (clamps to 127 like the constructor)
	COMPILETIMEEVAL void Set_ROT(int rate)
	{ ROT.SetDir((DirType)MinImpl(rate, 127)); }

private:
	// Total number of frames needed to complete the current rotation
	// = |angular gap in Raw| / ROT.Raw
	// If ROT is zero (instant turn), returns 0 to avoid division by zero
	int NumSteps() const
	{ return ROT.Raw != 0 ? Math::abs(Difference_Raw()) / ROT.Raw : 0; }

public:
	DirStruct DesiredFacing;  // Target angle (where we want to face)
	DirStruct StartFacing;    // Angle when the current rotation began
	CDTimerClass RotationTimer; // Counts down from NumSteps to 0
	DirStruct ROT;            // Turn rate (Raw used directly as step size)
};