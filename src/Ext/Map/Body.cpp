/*
// Backport of MapClass::RecalculateSubZones (0x584550–0x584E42).
// Ported from game binary — see pseudocode in TeamClass Pseudocode.instructions.md.

#include "Body.h"
#include <MapClass.h>
#include <Utilities/Macro.h>
#include <cstring> // std::memcpy

// ─── Helper function-pointer types (not MapClass methods) ─────────────────────

// sub_58AC70: game constructor for a DVC<SubzoneConnectionStruct> instance.
// Initialises the vtable pointer and zeroes all other fields.
using GameDVCSubzoneCtor_t = void (__thiscall*)(DummyDynamicVectorClass*);

// sub_58ABD0: initialises a SubzoneTrackingStruct entry's SubzoneConnections
// by moving (transferring ownership of) the items stored in a game-initialised
// temporary DVC produced by the flood-fill.
using SubzoneInitEntry_t = void (__thiscall*)(SubzoneTrackingStruct*, DummyDynamicVectorClass*);

// ─────────────────────────────────────────────────────────────────────────────

void __fastcall FakeMapClass::_RecalculateSubZones(MapClass* pThis, discard_t, CellStruct const& cell)
{
    if (!pThis->IsWithinUsableArea(cell, true))
        return;

    auto* self = reinterpret_cast<MapClass*>(pThis);
    bool needBigReset = false;

    // Process three hierarchical levels: 2 (8×8), 1 (4×4), 0 (2×2).
    for (int level = 2; level >= 0 && !needBigReset; --level)
    {
        // ── 3a: compute aligned block base ────────────────────────────────
        const int step  = 1 << (level + 1); // 8, 4, or 2
        const int baseX = static_cast<int>(static_cast<short>(cell.X))
                        - static_cast<int>(static_cast<short>(cell.X)) % step;
        const int baseY = static_cast<int>(static_cast<short>(cell.Y))
                        - static_cast<int>(static_cast<short>(cell.Y)) % step;

        // ── 3c: zone-ID collector used in the disconnect phase ─────────────
        DynamicVectorClass<WORD> tmpZoneIDs;
        tmpZoneIDs.CapacityIncrement = 10;

        // ── 3d: reset all hash-table bucket active counts ─────────────────
        HashTable<DWORD, SubzoneConnectionStruct>* ht = pThis->unknown_80[level];
        // Cast Buckets to DummyDynamicVectorClass* to avoid instantiating
        // DynamicVectorClass<HashObject<...>>, which would require operator== on HashObject.
        DummyDynamicVectorClass* const htBuckets =
            reinterpret_cast<DummyDynamicVectorClass*>(ht->Buckets);
        for (int i = 0; i < ht->BucketCount; ++i)
            htBuckets[i].ActiveCount = 0;

        // ── 3e: COLLECT + CLEAR pass ──────────────────────────────────────
        // Gather zone IDs currently assigned to cells in the block and wipe
        // them so they can be reassigned below.
        for (int cy = baseY; cy < baseY + step; ++cy)
        {
            for (int cx = baseX; cx < baseX + step; ++cx)
            {
                const CellStruct cs{ static_cast<short>(cx), static_cast<short>(cy) };
                const int pathIdx = pThis->GetCellPathIndex(cs);
                GlobalPassabilityData& gpd = pThis->LevelAndPassabilityStruct2pointer_70[pathIdx];

                const WORD zoneID = gpd.data[level];
                if (zoneID != 0 && !tmpZoneIDs.contains(zoneID))
                    tmpZoneIDs.push_back(zoneID);

                gpd.data[level] = 0;

                // Clamp before accessing CellLevelPassabilityStruct array.
                const int clampedIdx = (pathIdx < 0) ? 0
                    : (pathIdx >= pThis->ValidMapCellCount ? pThis->ValidMapCellCount - 1 : pathIdx);
                gpd.data[3] = pThis->LevelAndPassability[clampedIdx].ZoneArrayIndex;
            }
        }

        // ── 3f: DISCONNECT phase ──────────────────────────────────────────
        // Remove back-references into the zones we are about to rebuild and
        // reset their own connection counts.
        DynamicVectorClass<SubzoneTrackingStruct>& stv = (&pThis->SubzoneTracking1)[level];
        for (int ti = tmpZoneIDs.Count - 1; ti >= 0; --ti)
        {
            const WORD zid = tmpZoneIDs.Items[ti];
            SubzoneTrackingStruct& zone = stv.Items[zid];

            for (int ci = zone.SubzoneConnections.Count - 1; ci >= 0; --ci)
            {
                const DWORD neighborID = zone.SubzoneConnections.Items[ci].NeighborSubzoneIndex;
                SubzoneTrackingStruct& neighbor = stv.Items[neighborID];
                for (int ni = neighbor.SubzoneConnections.Count - 1; ni >= 0; --ni)
                {
                    if (neighbor.SubzoneConnections.Items[ni].NeighborSubzoneIndex
                            == static_cast<DWORD>(zid))
                    {
                        neighbor.SubzoneConnections.erase_at(ni);
                        break;
                    }
                }
            }
            // Reset count directly — avoids game-vtable dispatch (game vtable[3] is Clear,
            // which would also free Items, but we only want to mark the list as empty).
            zone.SubzoneConnections.Count = 0;
        }

        // ── 3g: SCAN + ASSIGN phase ───────────────────────────────────────
        // Walk every cell in the block; for each unvisited passable cell,
        // grow the SubzoneTracking array by one slot and flood-fill all
        // reachable passable cells from it.
        DWORD a4a = (&pThis->unknown_74)[level];
        for (int cy = baseY; cy < baseY + step && !needBigReset; ++cy)
        {
            for (int cx = baseX; cx < baseX + step && !needBigReset; ++cx)
            {
                const CellStruct cs{ static_cast<short>(cx), static_cast<short>(cy) };
                if (!pThis->IsWithinUsableArea(cs, true))
                    continue;

                const int pathIdx = pThis->GetCellPathIndex(cs);
                CellLevelPassabilityStruct& lp = pThis->LevelAndPassability[pathIdx];
                if (lp.CellPassability == 7) // impassable
                    continue;

                GlobalPassabilityData& gpd = pThis->LevelAndPassabilityStruct2pointer_70[pathIdx];
                if (gpd.data[level] != 0) // already assigned by a prior flood-fill
                    continue;

                // Grow SubzoneTracking using raw allocation to avoid triggering
                // constructors that would overwrite the game vtable in existing entries.
                const int newIdx = stv.Count;
                if (newIdx >= stv.Capacity)
                {
                    if (!stv.IsAllocated)
                    {
                        needBigReset = true;
                        break;
                    }
                    const int newCap = stv.Capacity + stv.CapacityIncrement;
                    if (newCap == 0)
                    {
                        needBigReset = true;
                        break;
                    }
                    auto* newItems = static_cast<SubzoneTrackingStruct*>(
                        YRMemory::Allocate(static_cast<size_t>(newCap) * sizeof(SubzoneTrackingStruct)));
                    if (!newItems)
                    {
                        needBigReset = true;
                        break;
                    }
                    if (stv.Items)
                    {
                        std::memcpy(newItems, stv.Items,
                            static_cast<size_t>(stv.Capacity) * sizeof(SubzoneTrackingStruct));
                    }
                    YRMemory::Deallocate(stv.Items);
                    stv.Items    = newItems;
                    stv.Capacity = newCap;
                }
                ++stv.Count;
                SubzoneTrackingStruct* newEntry = &stv.Items[newIdx];

                // Initialise a temporary DVC via the game constructor so that the
                // flood-fill can use the correct game vtable when it pushes results.
                DummyDynamicVectorClass tempConns{};
                reinterpret_cast<GameDVCSubzoneCtor_t>(0x58AC70)(&tempConns);

                // Flood-fill: marks all connected passable cells with a4a as their
                // zone ID and populates tempConns with outgoing SubzoneConnections.
                self->SubzoneFloodFill_5824A0(&lp, level, static_cast<int>(a4a), &tempConns, nullptr);

                // Transfer tempConns into newEntry->SubzoneConnections (game function
                // also stamps the correct game vtable into the new slot).
                reinterpret_cast<SubzoneInitEntry_t>(0x58ABD0)(newEntry, &tempConns);

                // Populate the remaining fields of the new SubzoneTrackingStruct.
                newEntry->ParentZoneIndex  = (level == 2)
                    ? static_cast<WORD>(0)
                    : gpd.data[level + 1];
                newEntry->MovementCostType = static_cast<DWORD>(
                    static_cast<unsigned char>(lp.CellPassability));
                newEntry->unknown_dword_20 = static_cast<DWORD>(
                    static_cast<short>(cx) / 4 + 130 * (static_cast<short>(cy) / 4) + 131);
                newEntry->SubzoneConnections.CapacityIncrement = 16;

                ++a4a;
                // Overflow guard: if the zone counter wraps to a multiple of 36
                // the slot maths breaks — fall back to a full reset.
                if (a4a * 36u == 0)
                {
                    needBigReset = true;
                    break;
                }
            }
        }

        if (!needBigReset)
        {
            (&pThis->unknown_74)[level] = a4a;

            // ── 3h: ZONE CONNECTIONS phase ────────────────────────────────
            // Re-evaluate every ZoneConnectionClass entry whose from- or to-cell
            // lies inside the block we just rebuilt.
            for (int zi = pThis->ZoneConnections.Count - 1; zi >= 0; --zi)
            {
                ZoneConnectionClass& zc = pThis->ZoneConnections.Items[zi];
                if (!zc.unknown_bool_08)
                    continue;

                const bool fromInBlock =
                    (zc.FromMapCoords.X >= baseX && zc.FromMapCoords.X < baseX + step
                  && zc.FromMapCoords.Y >= baseY && zc.FromMapCoords.Y < baseY + step);
                const bool toInBlock   =
                    (zc.ToMapCoords.X   >= baseX && zc.ToMapCoords.X   < baseX + step
                  && zc.ToMapCoords.Y   >= baseY && zc.ToMapCoords.Y   < baseY + step);

                if (fromInBlock || toInBlock)
                    self->SubzoneUpdateZoneConn_582D70(&zc, level);
            }

            // ── 3i: CROSS-CONNECTIONS phase ───────────────────────────────
            // Rebuild SubzoneConnections from the hash table that was populated
            // by SubzoneUpdateZoneConn_582D70.
            for (int i = 0; i < ht->BucketCount; ++i)
            {
                DummyDynamicVectorClass& bucket = htBuckets[i];
                if (bucket.ActiveCount <= 0)
                    continue;

                // HashObject<DWORD, SubzoneConnectionStruct> layout (12 bytes):
                //   [+0] Key (DWORD hash key)
                //   [+4] Value.NeighborSubzoneIndex: packed pair (lo16=src, hi16=dst)
                //   [+8] Value.ConnectionPenaltyFlag
                auto* items = reinterpret_cast<HashObject<DWORD, SubzoneConnectionStruct>*>(
                    bucket.Vector_Item);
                for (int j = 0; j < bucket.ActiveCount; ++j)
                {
                    const HashObject<DWORD, SubzoneConnectionStruct>& obj = items[j];
                    const WORD srcZone = static_cast<WORD>(obj.Value.NeighborSubzoneIndex & 0xFFFFu);
                    const WORD dstZone = static_cast<WORD>(obj.Value.NeighborSubzoneIndex >> 16);

                    stv.Items[srcZone].SubzoneConnections.push_back(
                        { static_cast<DWORD>(dstZone), obj.Value.ConnectionPenaltyFlag });
                    stv.Items[dstZone].SubzoneConnections.push_back(
                        { static_cast<DWORD>(srcZone), obj.Value.ConnectionPenaltyFlag });
                }
            }
        }
    } // end for (level)

    if (needBigReset)
    {
        // BigReset: clear all three levels and trigger a full reinitialisation
        // of each SubzoneTracking array via the original game routine.
        for (int lv = 2; lv >= 0; --lv)
        {
            (&pThis->SubzoneTracking1)[lv].Count = 0;
            self->SubzoneReinit_581F90(lv);
        }
    }
    else
    {
        // Step 4: update ParentZoneIndex for every cell in the enclosing 8×8 block.
        const int baseX8 = static_cast<int>(static_cast<short>(cell.X))
                         - static_cast<int>(static_cast<short>(cell.X)) % 8;
        const int baseY8 = static_cast<int>(static_cast<short>(cell.Y))
                         - static_cast<int>(static_cast<short>(cell.Y)) % 8;
        const int stride = pThis->MapRect.Width + pThis->MapRect.Height + 1;

        for (int cy = baseY8; cy < baseY8 + 8; ++cy)
        {
            for (int cx = baseX8; cx < baseX8 + 8; ++cx)
            {
                const CellStruct cs{ static_cast<short>(cx), static_cast<short>(cy) };
                if (!pThis->IsWithinUsableArea(cs, true))
                    continue;

                int idx = static_cast<int>(static_cast<short>(cx))
                        + stride * static_cast<int>(static_cast<short>(cy));
                idx = (idx < 0) ? 0
                    : (idx >= pThis->ValidMapCellCount ? pThis->ValidMapCellCount - 1 : idx);

                const GlobalPassabilityData* ptr = &pThis->LevelAndPassabilityStruct2pointer_70[idx];
                for (int lv = 0; lv < 2; ++lv)
                {
                    (&pThis->SubzoneTracking1)[lv].Items[ptr->data[lv]].ParentZoneIndex
                        = ptr->data[lv + 1];
                }
            }
        }
    }

    // Invalidate any cached A* paths that referenced the modified subzone data.
    AStarPathFinderClass::Instance->AStarClass__Clear_Pointers();
}

DEFINE_FUNCTION_JUMP(CALL, 0x424EF3, FakeMapClass::_RecalculateSubZones);
DEFINE_FUNCTION_JUMP(CALL, 0x442053, FakeMapClass::_RecalculateSubZones);
DEFINE_FUNCTION_JUMP(CALL, 0x480992, FakeMapClass::_RecalculateSubZones);
DEFINE_FUNCTION_JUMP(CALL, 0x4809FE, FakeMapClass::_RecalculateSubZones);
DEFINE_FUNCTION_JUMP(CALL, 0x480F1C, FakeMapClass::_RecalculateSubZones);
DEFINE_FUNCTION_JUMP(CALL, 0x48A312, FakeMapClass::_RecalculateSubZones);
DEFINE_FUNCTION_JUMP(CALL, 0x4FD02E, FakeMapClass::_RecalculateSubZones);
DEFINE_FUNCTION_JUMP(CALL, 0x58141F, FakeMapClass::_RecalculateSubZones);
DEFINE_FUNCTION_JUMP(CALL, 0x581B31, FakeMapClass::_RecalculateSubZones);
DEFINE_FUNCTION_JUMP(CALL, 0x586AA2, FakeMapClass::_RecalculateSubZones);
DEFINE_FUNCTION_JUMP(CALL, 0x5FC742, FakeMapClass::_RecalculateSubZones);
DEFINE_FUNCTION_JUMP(CALL, 0x71CA51, FakeMapClass::_RecalculateSubZones);
DEFINE_FUNCTION_JUMP(CALL, 0x74EA10, FakeMapClass::_RecalculateSubZones);

DEFINE_FUNCTION_JUMP(LJMP, 0x584550, FakeMapClass::_RecalculateSubZones)
*/