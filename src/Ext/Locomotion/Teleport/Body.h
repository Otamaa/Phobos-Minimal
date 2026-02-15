#pragma once

#include <Locomotor/TeleportLocomotionClass.h>
#include <FootClass.h>
#include <MapClass.h>
#include <CellClass.h>
#include <RulesClass.h>
#include <Unsorted.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Rules/Body.h>

/**
 * FakeTeleportLocomotionClass
 *
 * Complete backport of key TeleportLocomotionClass internal functions.
 * Replaces the original game code with equivalent C++ implementations
 * while integrating existing Phobos/Ares hooks and improvements.
 *
 * ACTIVE hooks (LJMP - replace entire function):
 *   - 0x718260 (InternalMark)           - Collision detection and cell occupation management
 *   - 0x7187A0 (Unwarp)                 - Post-teleport landing/damage handling
 *   - 0x718B70 (ComputeDestination)     - Destination validation and pathfinding
 *
 * These three internal __thiscall functions are safely hooked via LJMP because
 * they are called directly by the game engine, NOT through the ILocomotion COM vtable.
 *
 * REFERENCE implementations (not hooked, ILocomotion vtable functions):
 *   - Move_To, Stop_Moving, Do_Turn, Mark_All_Occupation_Bits
 *   - Is_Moving, IsStill, Destination, In_Which_Layer, GetClassID
 *   - Process, ProcessTimerCompletion
 *
 * Active conflicting hooks integrated into our backport (become dead code):
 *   - 0x718275 (Hooks.Locomotor.cpp)      - Already commented out
 *   - 0x7184CE (Spawner/Bugfixes.cpp)     - Already commented out
 *   - 0x7185DA (Spawner/Bugfixes.cpp)     - Already commented out
 *   - 0x71872C (Hooks.BugFixes.cpp)       - Inside InternalMark range, becomes dead
 *   - 0x7187DA (Hooks.Unit.cpp)           - Inside Unwarp range, becomes dead
 *   - 0x7188F2 (Hooks.Unit.cpp)           - Inside Unwarp range, becomes dead
 *   - 0x718871 (Hooks.Unit.cpp)           - Inside Unwarp range, becomes dead
 *   - 0x718F1E/0x7190B0 (Hooks.Transport) - Inside ComputeDestination, becomes dead
 *
 * Non-conflicting hooks that remain active:
 *   - 0x71810D (Hooks.Unit.cpp)           - Inside Move_To (not hooked)
 *   - 0x7196BB (Hooks.Transport.cpp)      - Inside Process (not hooked)
 *   - 0x719CBC (Hooks.BugFixes.cpp)       - Inside Load (not hooked)
 *   - 0x719F17 (Hooks.BugFixes.cpp)       - End_Piggyback shared across locos
 *   - 0x7193F6 etc. (Hooks.Teleport.cpp)  - Inside Process (not hooked)
 */
class FakeTeleportLocomotionClass
{
public:
	// Default coordinate value used to check if coords have been initialized
	// Maps to TeleportLocoCoord::default in the original game
	static const CoordStruct DefaultCoords;

	// =====================================================
	// Active hook entry points (__thiscall via __fastcall)
	// =====================================================

	/**
	 * 0x718260 - Internal mark/collision handler
	 * Original: bool __thiscall (TeleportLocomotionClass*, int x, int y, int z, int mark)
	 * Handles collision detection and cell occupation management during teleportation.
	 * [Improvement] Integrates: 0x718275, 0x7184CE, 0x7185DA, 0x71872C, 0x7187DA
	 */
	static bool __fastcall Hook_InternalMark(
		TeleportLocomotionClass* pThis, void* edx_unused,
		int destX, int destY, int destZ, int mark);

	/**
	 * 0x7187A0 - Unwarp/landing handler
	 * Original: void __thiscall (TeleportLocomotionClass*, CoordStruct)
	 * Handles consequences of teleporting into a cell (water, iron curtain, sinking).
	 * [Improvement] Integrates: 0x7187DA, 0x7188F2, 0x718871
	 */
	static void __fastcall Hook_Unwarp(
		TeleportLocomotionClass* pThis, void* edx_unused,
		int coordX, int coordY, int coordZ);

	/**
	 * 0x718B70 - Compute/validate destination
	 * Original: bool __thiscall (TeleportLocomotionClass*, CoordStruct*)
	 * Validates and computes the destination for teleportation with pathfinding.
	 * [Improvement] Integrates: 0x718F1E, 0x7190B0
	 * [Improvement] Better infantry sub-location and alternate cell finding
	 */
	static bool __fastcall Hook_ComputeDestination(
		TeleportLocomotionClass* pThis, void* edx_unused,
		int coordX, int coordY, int coordZ);

	// =====================================================
	// Reference implementations (NOT hooked - ILocomotion vtable functions)
	// These use __stdcall calling convention via COM interface dispatch.
	// Kept for documentation and potential future VTABLE hooks.
	// =====================================================

	// 0x718100 - ILocomotion::Move_To
	static void __fastcall Hook_MoveTo(
		TeleportLocomotionClass* pThis, void* edx_unused,
		int coordX, int coordY, int coordZ);

	// 0x7192F0 - ILocomotion::Process
	static bool __fastcall Hook_Process(
		TeleportLocomotionClass* pThis, void* edx_unused);

	// 0x719BF0 - Timer completion / post-teleport idle
	static void __stdcall Hook_ProcessTimerCompletion(
		TeleportLocomotionClass* pThis);

	// 0x718230 - ILocomotion::Stop_Moving
	static void __fastcall Hook_StopMoving(
		TeleportLocomotionClass* pThis, void* edx_unused);

	// 0x7192C0 - ILocomotion::Do_Turn
	static bool __fastcall Hook_DoTurn(
		TeleportLocomotionClass* pThis, void* edx_unused,
		DirStruct dir);

	// 0x71A090 - ILocomotion::Mark_All_Occupation_Bits
	static void __fastcall Hook_MarkAllOccupationBits(
		TeleportLocomotionClass* pThis, void* edx_unused,
		int mark);

	// 0x718080 - ILocomotion::Is_Moving
	static bool __fastcall Hook_IsMoving(
		TeleportLocomotionClass* pThis, void* edx_unused);

	// 0x718090 - IsStill virtual
	static bool __fastcall Hook_IsStill(
		TeleportLocomotionClass* pThis, void* edx_unused);

	// 0x7180A0 - ILocomotion::Destination
	static CoordStruct* __fastcall Hook_Destination(
		TeleportLocomotionClass* pThis, void* edx_unused,
		CoordStruct* pOutBuffer);

	// 0x719E20 - ILocomotion::In_Which_Layer
	static Layer __fastcall Hook_InWhichLayer(
		TeleportLocomotionClass* pThis, void* edx_unused);

	// 0x719C60 - IPersist::GetClassID
	static HRESULT __fastcall Hook_GetClassID(
		TeleportLocomotionClass* pThis, void* edx_unused,
		CLSID* pClassID);

private:
	// =====================================================
	// Internal helper methods
	// =====================================================

	static bool IsDefaultCoords(const CoordStruct& coords);
	static void ClearOccupyBit(FootClass* pLinkedTo, const CoordStruct& coords);
	static void SetOccupyBit(FootClass* pLinkedTo, const CoordStruct& coords);

	/** [Improvement] Integrated from Hooks.Transport.cpp 0x718F1E */
	static MovementZone AdjustMovementZone(MovementZone mzone);

	/** [Improvement] Integrates: 0x718275, 0x7187DA (self-crush, Chronoshift_Crushable, ChronoInfantryCrush) */
	static bool HandleDestinationCollisions(
		TeleportLocomotionClass* pLoco, const CoordStruct& destCoord, CellClass* pDestCell);

	/** [Improvement] Integrates: 0x7185DA (fallback), 0x7184CE (cell fix) */
	static bool FindAlternateLocation(
		TeleportLocomotionClass* pLoco, const CoordStruct& destCoord,
		CellClass* pSourceCell, bool sourceCellHasBridge);

	static void FinalizeDestination(TeleportLocomotionClass* pLoco);

	/** [Improvement] Integrates: 0x718871 (SinkOrSwim) */
	static bool CanFloatOnWater(TechnoTypeClass* pType, FootClass* pUnit);

	static void SinkUnit(FootClass* pUnit);
	static void PlayChronoSound(FootClass* pUnit, int typeSound, int rulesSound);
};
