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
 * This class provides a reimplementation of the internal TeleportLocomotionClass helper function
 * at 0x718260. This is NOT the public Mark_All_Occupation_Bits vtable function, but rather
 * an internal helper that takes explicit destination coordinates.
 *
 * Function signature: bool __thiscall TeleportLocomotionClass::internal_mark(int x, int y, int z, int mark)
 *
 * The original function at 0x718260 handles:
 *   - When mark != 0: Finalizing teleport - clearing old occupation, setting new occupation
 *   - When mark == 0: Starting teleport - collision detection, finding valid destination
 *
 * The function is called during the teleportation process to manage cell occupation bits
 * and handle unit collisions at the destination.
 */
class FakeTeleportLocomotionClass
{
public:
	// Default coordinate value used to check if LastCoords has been initialized
	// In the original game, this is a static constant at TeleportLocoCoord::default
	static constexpr CoordStruct DefaultCoords { -1, -1, -1 };

	/**
	 * Mark_All_Occupation_Bits - Main teleportation occupation bit handler
	 *
	 * @param pLoco - The TeleportLocomotionClass instance
	 * @param destCoord - The destination coordinates (passed as X, Y, Z)
	 * @param mark - The mark type:
	 *               0 = Starting teleport (check collisions, find valid spot)
	 *               non-0 = Finishing teleport (finalize position, set occupation)
	 *
	 * @return true if occupation bits were set successfully
	 *         false if a new destination was calculated (caller should retry)
	 */
	static bool Mark_All_Occupation_Bits(
		TeleportLocomotionClass* pLoco,
		const CoordStruct& destCoord,
		int mark);

	/**
	 * TeleportLocomotionClass_InternalMark - Wrapper matching original __thiscall signature
	 * This is the actual function that gets hooked at 0x718260
	 *
	 * Original signature: bool __thiscall (TeleportLocomotionClass* this, int x, int y, int z, int mark)
	 * Return: 1 if occupation bits were set, 0 if destination was changed
	 */
	static bool __fastcall TeleportLocomotionClass_InternalMark(
		TeleportLocomotionClass* pThis,
		void* edx_unused,
		int destX,
		int destY,
		int destZ,
		int mark);

private:
	/**
	 * ClearOccupyBit - Helper to clear occupation at a location
	 * Calls the linked unit's UnmarkAllOccupationBits
	 */
	static void ClearOccupyBit(FootClass* pLinkedTo, const CoordStruct& coords);

	/**
	 * SetOccupyBit - Helper to set occupation at a location
	 * Calls the linked unit's MarkAllOccupationBits
	 */
	static void SetOccupyBit(FootClass* pLinkedTo, const CoordStruct& coords);

	/**
	 * HandleDestinationCollisions - Process collisions at teleport destination
	 *
	 * Handles various collision scenarios:
	 * - Infantry vs Infantry at same coordinates: target takes damage
	 * - Iron curtained units: teleporting unit takes damage
	 * - Non-foot objects: need to find alternate location
	 * - Regular foot units: they take damage
	 *
	 * @param pLoco - The locomotor
	 * @param destCoord - Destination coordinates
	 * @param pDestCell - The destination cell
	 *
	 * @return true if destination is blocked and alternate location needed
	 */
	static bool HandleDestinationCollisions(
		TeleportLocomotionClass* pLoco,
		const CoordStruct& destCoord,
		CellClass* pDestCell);

	/**
	 * FindAlternateLocation - Find a nearby valid cell when destination is blocked
	 *
	 * @param pLoco - The locomotor
	 * @param destCoord - Original destination
	 * @param pSourceCell - The source cell
	 * @param sourceCellHasBridge - Whether source cell has a bridge
	 *
	 * @return true if alternate location was found and ChronoDestCoords updated
	 */
	static bool FindAlternateLocation(
		TeleportLocomotionClass* pLoco,
		const CoordStruct& destCoord,
		CellClass* pSourceCell,
		bool sourceCellHasBridge);

	/**
	 * FinalizeDestination - Complete the teleportation and set final position
	 *
	 * @param pLoco - The locomotor
	 */
	static void FinalizeDestination(TeleportLocomotionClass* pLoco);

	/**
	 * IsDefaultCoords - Check if coordinates match the default uninitialized value
	 */
	static bool IsDefaultCoords(const CoordStruct& coords);

	/**
	 * AdjustMovementZone - Convert special movement zones to normal ones
	 * Fly -> Normal, Destroyer -> Normal, AmphibiousDestroyer -> Amphibious
	 */
	static MovementZone AdjustMovementZone(MovementZone mzone);
};
