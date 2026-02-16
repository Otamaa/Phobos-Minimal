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
 * VTable layout (from .rdata):
 *
 *   Main vtable (IPersistStream + C++) at 0x7F50CC:
 *     +0x00  QueryInterface    __stdcall (COM)
 *     +0x04  AddRef            __stdcall (COM)
 *     +0x08  Release           __stdcall (COM)
 *     +0x0C  GetClassID        __stdcall (IPersist)
 *     +0x10  IsDirty           __stdcall (IPersistStream)
 *     +0x14  Load              __stdcall (IPersistStream)
 *     +0x18  Save              __stdcall (IPersistStream)
 *     +0x1C  GetSizeMax        __stdcall (IPersistStream)
 *     +0x20  ~destructor       __thiscall (C++)
 *     +0x24  Size              __thiscall (C++)
 *     +0x28  vt_entry_28       __thiscall (C++) -> 0x719BF0 (ProcessTimerCompletion)
 *     +0x2C  IsStill           __thiscall (C++) -> 0x718090
 *
 *   ILocomotion vtable at 0x7F5000:
 *     +0x10  Is_Moving         __stdcall -> 0x718080
 *     +0x14  Destination       __stdcall -> 0x7180A0
 *     +0x40  Process           __stdcall -> 0x7192F0
 *     +0x44  Move_To           __stdcall -> 0x718100
 *     +0x48  Stop_Moving       __stdcall -> 0x718230
 *     +0x4C  Do_Turn           __stdcall -> 0x7192C0
 *     +0x74  In_Which_Layer    __stdcall -> 0x719E20
 *     +0x9C  Mark_All_Occ_Bits __stdcall -> 0x71A090
 *     +0xB0  Clear_Coords      __stdcall
 *
 *   IPiggyback vtable at 0x7F4FDC:
 *     +0x00  QueryInterface    __stdcall
 *     +0x04  AddRef            __stdcall
 *     +0x08  Release           __stdcall
 *     +0x0C  Begin_Piggyback   __stdcall
 *     +0x10  End_Piggyback     __stdcall
 *     +0x14  Is_Ok_To_End      __stdcall
 *     +0x18  Piggyback_CLSID   __stdcall
 *     +0x1C  Is_Piggybacking   __stdcall
 *
 * ACTIVE hooks (LJMP - replace entire function):
 *   Internal __thiscall functions (no vtable or main vtable C++ entries):
 *   - 0x718260 (InternalMark)           - Not in any vtable, __thiscall
 *   - 0x7187A0 (Unwarp)                 - Not in any vtable, __thiscall
 *   - 0x718B70 (ComputeDestination)     - Not in any vtable, __thiscall
 *   - 0x719BF0 (vt_entry_28)            - Main vtable +0x28, __thiscall
 *   - 0x718090 (IsStill)                - Main vtable +0x2C, __thiscall
 *
 * ACTIVE VTABLE hooks:
 *   - 0x7F5040: Process (ILocomotion +0x40) -> Hook_Process
 *     Replaces the original Process function entirely.
 *     Existing ASMJIT_PATCHes inside the original function (Hooks.Teleport.cpp)
 *     become dead code but are kept as a safety net.
 *
 * REFERENCE implementations (not hooked):
 *   ILocomotion vtable functions (__stdcall via COM dispatch):
 *   - Move_To, Stop_Moving, Do_Turn, Mark_All_Occupation_Bits
 *   - Is_Moving, Destination, In_Which_Layer, GetClassID
 *   - ProcessTimerCompletion
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
 *   - 0x719CBC (Hooks.BugFixes.cpp)       - Inside Load (not hooked)
 *   - 0x719F17 (Hooks.BugFixes.cpp)       - End_Piggyback shared across locos
 *
 * Dead code (inside VTABLE-replaced Process, kept as safety net):
 *   - 0x7193F6, 0x719742, 0x719827, 0x71997B, 0x7197DF, 0x719BD9
 *     (Hooks.Teleport.cpp) - Modify original Process code, unreachable via VTABLE
 *   - 0x7196BB (Hooks.Transport.cpp) - Inside Process, unreachable via VTABLE
 */
class FakeTeleportLocomotionClass
{
public:
	// Default coordinate value used to check if coords have been initialized
	// Maps to TeleportLocoCoord::default in the original game
	static const CoordStruct DefaultCoords;

	// =====================================================
	// Active hook entry points (__thiscall via __fastcall)
	// All of these are either internal functions (not in any vtable)
	// or C++ virtuals in the main vtable (NOT ILocomotion/IPiggyback COM vtables).
	// =====================================================

	/**
	 * 0x718260 - Internal mark/collision handler (NOT in any vtable)
	 * Original: bool __thiscall (TeleportLocomotionClass*, int x, int y, int z, int mark)
	 * Handles collision detection and cell occupation management during teleportation.
	 * [Improvement] Integrates: 0x718275, 0x7184CE, 0x7185DA, 0x71872C, 0x7187DA
	 */
	static bool __fastcall Hook_InternalMark(
		TeleportLocomotionClass* pThis, void* edx_unused,
		int destX, int destY, int destZ, int mark);

	/**
	 * 0x7187A0 - Unwarp/landing handler (NOT in any vtable)
	 * Original: void __thiscall (TeleportLocomotionClass*, CoordStruct)
	 * Handles consequences of teleporting into a cell (water, iron curtain, sinking).
	 * [Improvement] Integrates: 0x7187DA, 0x7188F2, 0x718871
	 */
	static void __fastcall Hook_Unwarp(
		TeleportLocomotionClass* pThis, void* edx_unused,
		int coordX, int coordY, int coordZ);

	/**
	 * 0x718B70 - Compute/validate destination (NOT in any vtable)
	 * Original: bool __thiscall (TeleportLocomotionClass*, CoordStruct*)
	 * Validates and computes the destination for teleportation with pathfinding.
	 * [Improvement] Integrates: 0x718F1E, 0x7190B0
	 * [Improvement] Better infantry sub-location and alternate cell finding
	 *
	 * IMPORTANT: Original takes CoordStruct* (4-byte pointer), NOT individual ints.
	 * Using 3 ints here would cause ret 12 vs caller's push 4 = stack corruption.
	 */
	static bool __fastcall Hook_ComputeDestination(
		TeleportLocomotionClass* pThis, void* edx_unused,
		CoordStruct* pCoord);

	/**
	 * 0x719BF0 - vt_entry_28 / ProcessTimerCompletion
	 * Main vtable at 0x7F50CC + 0x28 = 0x7F50F4, __thiscall
	 * YRpp: virtual void vt_entry_28(DWORD dwUnk)
	 * Called after chrono timer expires to handle post-teleport idle behavior.
	 */
	static void __fastcall Hook_ProcessTimerCompletion(
		TeleportLocomotionClass* pThis, void* edx_unused,
		DWORD dwUnk);

	/**
	 * 0x718090 - IsStill
	 * Main vtable at 0x7F50CC + 0x2C = 0x7F50F8, __thiscall
	 * YRpp: virtual bool IsStill()
	 * Returns true if the unit is NOT currently moving/teleporting.
	 */
	static bool __fastcall Hook_IsStill(
		TeleportLocomotionClass* pThis, void* edx_unused);

	// =====================================================
	// Reference implementations (NOT hooked)
	// ILocomotion vtable entries use __stdcall via COM dispatch.
	// Main vtable IPersistStream entries also use __stdcall.
	// Cannot safely LJMP these; use VTABLE patches if needed:
	//   DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5000 + offset, func) for ILocomotion
	//   DEFINE_FUNCTION_JUMP(VTABLE, 0x7F50CC + offset, func) for main vtable COM
	// =====================================================

	// 0x718100 - ILocomotion vtable +0x44, __stdcall
	static void __fastcall Hook_MoveTo(
		TeleportLocomotionClass* pThis, void* edx_unused,
		int coordX, int coordY, int coordZ);

	// 0x7192F0 - ILocomotion vtable +0x40, __stdcall
	// ACTIVE: Hooked via DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5040, ...)
	// Receives ILocomotion* (COM this) on stack; static_cast adjusts to base class.
	static bool __stdcall Hook_Process(ILocomotion* pLoco);

	// 0x718230 - ILocomotion vtable +0x48, __stdcall
	static void __fastcall Hook_StopMoving(
		TeleportLocomotionClass* pThis, void* edx_unused);

	// 0x7192C0 - ILocomotion vtable +0x4C, __stdcall
	static bool __fastcall Hook_DoTurn(
		TeleportLocomotionClass* pThis, void* edx_unused,
		DirStruct dir);

	// 0x71A090 - ILocomotion vtable +0x9C, __stdcall
	static void __fastcall Hook_MarkAllOccupationBits(
		TeleportLocomotionClass* pThis, void* edx_unused,
		int mark);

	// 0x718080 - ILocomotion vtable +0x10, __stdcall
	static bool __fastcall Hook_IsMoving(
		TeleportLocomotionClass* pThis, void* edx_unused);

	// 0x7180A0 - ILocomotion vtable +0x14, __stdcall
	static CoordStruct* __fastcall Hook_Destination(
		TeleportLocomotionClass* pThis, void* edx_unused,
		CoordStruct* pOutBuffer);

	// 0x719E20 - ILocomotion vtable +0x74, __stdcall
	static Layer __fastcall Hook_InWhichLayer(
		TeleportLocomotionClass* pThis, void* edx_unused);

	// 0x719C60 - Main vtable +0x0C, __stdcall (IPersist COM)
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
