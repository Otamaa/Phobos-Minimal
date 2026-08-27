#include "Body.h"

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <FootClass.h>
#include <TechnoClass.h>
#include <Memory.h>
#include <MapClass.h>
#include <HouseClass.h>
#include <TubeClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <Misc/PhobosGlobal.h>
#include <Utilities/Patch.h>

#include <Ext/Techno/Body.h>

#include <CellSpread.h>

#include <algorithm>
#include <bit>

#include <cstdint>

#ifdef _InitialVersion_Enabled
// struct TurnTrackType
// {
// 	int32_t LeftTrack;
// 	int32_t RightTrack;
// 	int32_t field_8;
// 	int32_t field_C;
// 	DirType direction;
// 	int32_t field_14;
// };

// // Main table (indices 0-59)
// constexpr std::array<TurnTrackType, 60> DriveClass_TrackControl = { {
// 		// 0-7
// 		{1,  0,  0, 0, DirType::Min,         0},
// 		{3,  7,  0, 0, DirType::NorthEast,   8},
// 		{4,  9,  0, 0, DirType::East,        8},
// 		{0,  0,  0, 0, DirType::SouthEast,   0},
// 		{0,  0,  0, 0, DirType::South,       0},
// 		{0,  0,  0, 0, DirType::SouthWest,   0},
// 		{4,  9,  0, 0, DirType::West,       10},
// 		{3,  7,  0, 0, DirType::NorthWest,  10},

// 		// 8-15
// 		{6,  8,  0, 0, DirType::Min,        15},
// 		{2,  0,  0, 0, DirType::NorthEast,   0},
// 		{6,  8,  0, 0, DirType::East,        8},
// 		{5, 10,  0, 0, DirType::SouthEast,   8},
// 		{0,  0,  0, 0, DirType::South,       0},
// 		{0,  0,  0, 0, DirType::SouthWest,   0},
// 		{0,  0,  0, 0, DirType::West,        0},
// 		{5, 10,  0, 0, DirType::NorthWest,  15},

// 		// 16-23
// 		{4,  9,  0, 0, DirType::Min,        15},
// 		{3,  7,  0, 0, DirType::NorthEast,  15},
// 		{1,  0,  0, 0, DirType::East,        3},
// 		{3,  7,  0, 0, DirType::SouthEast,  11},
// 		{4,  9,  0, 0, DirType::South,      11},
// 		{0,  0,  0, 0, DirType::SouthWest,   0},
// 		{0,  0,  0, 0, DirType::West,        0},
// 		{0,  0,  0, 0, DirType::NorthWest,   0},

// 		// 24-31
// 		{0,  0,  0, 0, DirType::Min,         0},
// 		{5, 10,  0, 0, DirType::NorthEast,  12},
// 		{6,  8,  0, 0, DirType::East,       12},
// 		{2,  0,  0, 0, DirType::SouthEast,   4},
// 		{6,  8,  0, 0, DirType::South,      11},
// 		{5, 10,  0, 0, DirType::SouthWest,  11},
// 		{0,  0,  0, 0, DirType::West,        0},
// 		{0,  0,  0, 0, DirType::NorthWest,   0},

// 		// 32-39
// 		{0,  0,  0, 0, DirType::Min,         0},
// 		{0,  0,  0, 0, DirType::NorthEast,   0},
// 		{4,  9,  0, 0, DirType::East,       12},
// 		{3,  7,  0, 0, DirType::SouthEast,  12},
// 		{1,  0,  0, 0, DirType::South,       4},
// 		{3,  7,  0, 0, DirType::SouthWest,  14},
// 		{4,  9,  0, 0, DirType::West,       14},
// 		{0,  0,  0, 0, DirType::NorthWest,   0},

// 		// 40-47
// 		{0,  0,  0, 0, DirType::Min,         0},
// 		{0,  0,  0, 0, DirType::NorthEast,   0},
// 		{0,  0,  0, 0, DirType::East,        0},
// 		{5, 10,  0, 0, DirType::SouthEast,   9},
// 		{6,  8,  0, 0, DirType::South,       9},
// 		{2,  0,  0, 0, DirType::SouthWest,   1},
// 		{6,  8,  0, 0, DirType::West,       14},
// 		{5, 10,  0, 0, DirType::NorthWest,  14},

// 		// 48-55
// 		{4,  9,  0, 0, DirType::Min,        13},
// 		{0,  0,  0, 0, DirType::NorthEast,   0},
// 		{0,  0,  0, 0, DirType::East,        0},
// 		{0,  0,  0, 0, DirType::SouthEast,   0},
// 		{4,  9,  0, 0, DirType::South,       9},
// 		{3,  7,  0, 0, DirType::SouthWest,   9},
// 		{1,  0,  0, 0, DirType::West,        1},
// 		{3,  7,  0, 0, DirType::NorthWest,  13},

// 		// 56-59
// 		{6,  8,  0, 0, DirType::Min,        13},
// 		{5, 10,  0, 0, DirType::NorthEast,  13},
// 		{0,  0,  0, 0, DirType::East,        0},
// 		{0,  0,  0, 0, DirType::SouthEast,   0},
// 	} };

// // Extra continuation entries (indices 60-73)
// constexpr std::array<TurnTrackType, 14> DriveClass_TrackControl_Extra = { {
// 	{0,  0,  0, 0, DirType::South,       0},
// 	{0,  0,  0, 0, DirType::SouthEast,   0},
// 	{0,  0,  0, 0, DirType::South,       0},
// 	{5, 10,  0, 0, DirType::SouthWest,  10},
// 	{6,  8,  0, 0, DirType::West,       10},
// 	{2,  0,  0, 0, DirType::NorthWest,   2},
// 	{11, 11, 0, 0, DirType::SouthWest,   0},
// 	{12, 12, 0, 0, DirType::SouthWest,   0},
// 	{13, 13, 0, 0, DirType::SouthWest,   0},
// 	{14, 14, 0, 0, DirType::NorthEast,   0},
// 	{14, 14, 0, 0, DirType::SouthEast,   4},
// 	{14, 14, 0, 0, DirType::SouthWest,   1},
// 	{14, 14, 0, 0, DirType::NorthWest,   2},
// 	{15, 15, 0, 0, DirType::West,        0},
// } };

// ===========================================================================
// File-local tables and helpers — unchanged from the previous pass except
// where noted. None of these touch the class layout.
// ===========================================================================

// -----------------------------------------------------------------------
// Stamp a flat cell index as visited and record its movement cost.
// useAlt selects the bridge (alt) layer vs ground layer arrays.
// Consolidates the 6+ VisitCounts/AltVisitCounts stamp pairs in
// Find_Path_Regular and Init.
//
// DIFF: holds raw pointers borrowed from the unique_ptr members. It is a
//       short-lived view, constructed per Find_Path_Regular call — it must
//       never outlive a Reset().
// -----------------------------------------------------------------------
struct VisitStamper
{
	int* VisitCounts;
	int* AltVisitCounts;
	float* Distances;
	float* AltDistances;
	int   SearchID;

	void Stamp(int flatIdx, float cost, bool useAlt) const
	{
		if (useAlt)
		{
			this->VisitCounts[flatIdx] = this->SearchID;
			this->Distances[flatIdx] = cost;
			return;
		}

		this->AltVisitCounts[flatIdx] = this->SearchID;
		this->AltDistances[flatIdx] = cost;
	}

	bool IsVisited(int flatIdx, bool useAlt) const
	{
		return useAlt
			? (this->VisitCounts[flatIdx] == this->SearchID)
			: (this->AltVisitCounts[flatIdx] == this->SearchID);
	}

	float GetDist(int flatIdx, bool useAlt) const
	{
		return useAlt ? this->Distances[flatIdx] : this->AltDistances[flatIdx];
	}
};

static COMPILETIMEEVAL constant_ptr<float, 0x7E3794> _pathfind_adjusment {};

// Cost multiplier per pass/layer index (a5).
// flt_81870C[0..6] = {1.0, 1000.0, 1.0, 1.0, 60.0, 20.0, 8.0}
// VERIFY: indices 3..6 — only 0/1/2 used by AStarPostProcessType; rest may be
//         used by other callers. Array size confirmed as 7 entries from segment dump.
static COMPILETIMEEVAL float MoveCostMultiplier [8] =
{
	1.0f,     // ASTAR_PASS_0
	1000.0f,  // ASTAR_PASS_1
	1.0f,     // ASTAR_PASS_2
	1.0f,     // VERIFY: usage unknown
	60.0f,    // VERIFY: usage unknown
	20.0f,    // VERIFY: usage unknown
	8.0f,     // VERIFY: usage unknown
	10000.0 ,
};

// Direction-to-neighbor-index LUT for non-bridge cells.
// dword_7E3710[0..7] = {-2,-2,0,1,1,1,0,-2}
// Used when CellClass::Bitfield2 bit 0x800 is clear.
// VERIFY: naming — "non-bridge neighbor offset table"
static COMPILETIMEEVAL int NeighborIndexTable_Flat[8] =
{
	-2, -2, 0, 1, 1, 1, 0, -2
};

// Direction-to-neighbor-index LUT for bridge cells.
// dword_7E3730[0..7] = {0,-1024,-1024,-1024,0,512,512,512}
// Used when CellClass::Bitfield2 bit 0x800 is set.
// VERIFY: naming — "bridge neighbor offset table"; large values suggest zone/buffer indexing
static COMPILETIMEEVAL int NeighborIndexTable_Bridge[8] =
{
	0, -1024, -1024, -1024, 0, 512, 512, 512
};

// Scalar float constants confirmed from asm bit patterns:
// flt_7E37B4 = 2.0f  (both-sides-bridge multiplier)
// flt_7E37B8 = 10.0f (no-bridge penalty)
// flt_7E37BC = 4.0f  (tunnel/wall cost multiplier, bit 0x40000)
static COMPILETIMEEVAL float BridgeBothSidesMultiplier = 2.0f;
static COMPILETIMEEVAL float NoBridgeMultiplier = 10.0f;
static COMPILETIMEEVAL float TunnelCostMultiplier = 4.0f;

static COMPILETIMEEVAL int CellAdjacencyDirectionLUT[9] = {
	3,   // (-1,-1) SW
	4,   // (-1, 0) S
	5,   // (-1,+1) SE
	2,   // ( 0,-1) W
   -1,   // ( 0, 0) same cell
	6,   // ( 0,+1) E
	1,   // (+1,-1) NE
	0,   // (+1, 0) N
	7,   // (+1,+1) NW
};

COMPILETIMEEVAL std::array<int, 8> dword_7E3774 = {
	-512, -511, 1, 513, 512, 511, -1, -513
};

COMPILETIMEEVAL std::array<float, 8> const adjust_81872C = {
	0.001f,
	0.0049999999f,
	0.0020000001f,
	0.0060000001f,
	0.003f,
	0.0070000002f,
	0.0040000002f,
	0.0080000004f
};

COMPILETIMEEVAL double dbl_7E37C0 = std::bit_cast<double>(0x3FF024DD2F1A9FBEULL); //1.009
COMPILETIMEEVAL reference<int, 0x89C2DC> const RegionSize {};
COMPILETIMEEVAL reference<PathType, 0x89A2D8> const outpath {};
COMPILETIMEEVAL reference<CellStruct, 0x89A324, 0x7D0> const MainOverlap {};
COMPILETIMEEVAL reference<int, 0x89A304, 8> const adjust_89A304 {};

ObjectClass* GetCellObj(CellClass* pCell, bool alt)
{
	return alt ? pCell->AltObject : pCell->FirstObject;
}

void ToggleBit40000(unsigned int* bitfield)
{
	*bitfield ^= (*bitfield ^ ~*bitfield) & 0x40000;
}

CellClass** CellArrayPtr(int x, int y)
{
	return &MapClass::Instance->Cells.Items[y * 512 + x];
}

int ChebyshevDist(CellStruct a, CellStruct b)
{
	return std::max(
		Math::abs(static_cast<int>(a.X) - static_cast<int>(b.X)),
		Math::abs(static_cast<int>(a.Y) - static_cast<int>(b.Y)));
}

// -----------------------------------------------------------------------
// Resolve one movement step from currentPos in direction dir.
// dir==8 → tunnel lookup; otherwise → AdjacentCell offset.
// Consolidates ResolveStep() and the tube_42D490 duplicate (which also
// had a bug: Items[facing] should be Items[tubeIdx]).
// -----------------------------------------------------------------------
static inline CellStruct ResolveStep(CellStruct currentPos, int dir)
{
	if (dir != 8)
		return currentPos + CellSpread::AdjacentCell[dir & 7];

	const int tubeIdx = static_cast<int>(
		MapClass::Instance->GetCellAt(currentPos)->TubeIndex);

	if (tubeIdx == -1)
		return { 0, 0 };

	// BUGFIX: vanilla tube_42D490 incorrectly used Items[facing] here.
	return TubeClass::Array->Items[tubeIdx]->ExitCell;
}

// -----------------------------------------------------------------------
// Walk the HierarchyBuffer back-pointer chain.
// BufferDelta == -1 is the root sentinel.
// bufferBase must be reinterpret_cast<char*>(HierarchyBuffer.get()).
// -----------------------------------------------------------------------
static inline AStarQueueNodeHierarchical* HierarchyParent(
	char* bufferBase,
	AStarQueueNodeHierarchical* node)
{
	return reinterpret_cast<AStarQueueNodeHierarchical*>(
		bufferBase + node->BufferDelta * 16);
}

static inline bool IsHierarchyRoot(AStarQueueNodeHierarchical* node)
{
	return node->BufferDelta == -1;
}

// ===========================================================================
// PhobosAStarPathFinderClass
// ===========================================================================

// -----------------------------------------------------------------------
// ctor
// DIFF: 8 raw `new` calls collapse into make_unique in the init list. The
//       three ZoneIndices reassignments, the PassabilityData memset and the
//       PassabilityCounts loop are gone — the members self-initialise.
//
// NOTE: make_unique value-initialises, which matches vanilla here — the old
//       `new AStarWorkPathStructHeap()` had a defaulted ctor, so the paren
//       form already zero-filled all ~2.5 MB. If that startup memset ever
//       shows up in a profile, switch to make_unique_for_overwrite and set
//       ActiveCount to 0 by hand; both arenas are bump-allocated and never
//       read before write.
// -----------------------------------------------------------------------
PhobosAStarPathFinderClass::PhobosAStarPathFinderClass()
	: unknown_byte_0 { 0 }
	, FindBridgeDir { false }
	, unknown_byte_2 { 0 }
	, CanFindPath { true }
	, PathCostFactor { 1.0f }
	, IsAlt { true }
	, PathNodeBuffer { std::make_unique<AStarWorkPathStructDataHeap>() }
	, PathQueueBuffer { std::make_unique<AStarWorkPathStructHeap>() }
	, PathQueue { std::make_unique<PathQueueType>(PathQueueCapacity) }
	, VisitCounts { nullptr }
	, AltVisitCounts { nullptr }
	, AltDistances { nullptr }
	, Distances { nullptr }
	, SearchID { -1 }
	, FinderSpeedType { static_cast<SpeedType>(-1) }
	, StartLevel { 0 }
	, EndLevel { 0 }
	, IsSearching { true }
	, FindMode { ASTAR_PASS_0 }
	, LevelVisitedMarkers { }
	, OpenSetMarkers { }
	, GCostArray { }
	, HierarchyBuffer { std::make_unique<AStarQueueNodeHierarchical[]>(HierarchyCapacity) }
	, HierarchyQueue { std::make_unique<HierarchyQueueType>(HierarchyCapacity) }
	, PathLength { -1 }
	, CellStructBuffer { 0, 0 }
	, ZoneIndices { }
	, PassabilityData { }
	, PassabilityCounts { }
{
	// VisitCounts / AltVisitCounts / Distances / AltDistances stay null until
	// Reset() sizes them. LevelVisitedMarkers / OpenSetMarkers / GCostArray
	// stay null until AllocZoneArrays() sizes them. Both match vanilla.
}

// -----------------------------------------------------------------------
// dtor
// DIFF: all 14 delete pairs and the manual ZoneIndices destruction loop are
//       gone. Nothing left to do.
//
// NOTE: vanilla deleted Distances before AltDistances; destruction is now in
//       reverse-declaration order. Both are plain buffers with no
//       cross-references, so the order was never observable.
// -----------------------------------------------------------------------
PhobosAStarPathFinderClass::~PhobosAStarPathFinderClass() = default;

// -----------------------------------------------------------------------
// __Find_Path_Hierarchical
// DIFF: HierarchyParent() / IsHierarchyRoot() replace the two inline
//       back-pointer chain walks (phase-1 mark + phase-2 store).
// DIFF: the 8-line blocked-pair reverse scan collapses into
//       ZoneIndexList::Contains().
// -----------------------------------------------------------------------
bool PhobosAStarPathFinderClass::__Find_Path_Hierarchical(
	CellStruct* startCell,
	CellStruct* destCell,
	MovementZone mzone,
	FootClass* foot)
{
	GlobalPassabilityData* globalPassabilityArray =
		MapClass::Instance->LevelAndPassabilityStruct2pointer_70;

	const int   mzoneIndex = static_cast<int>(mzone);
	auto& moveZoneArray = MapClass::MovementAdjustArray[mzoneIndex];
	const float* gameAdjustments = _pathfind_adjusment();

	double      threatAvoidance = 0.0;
	HouseClass* ownerHouse = nullptr;
	bool        useThreatAvoidance = false;

	if (foot)
	{
		threatAvoidance = foot->GetThreatAvoidanceCoefficient();
		ownerHouse = foot->Owner;

		if (threatAvoidance > 0.00001)
			useThreatAvoidance = true;
	}

	const int startZoneRaw = MapClass::Instance->MapClass_zone_56D3F0(startCell);
	const int destZoneRaw = MapClass::Instance->MapClass_zone_56D3F0(destCell);

	// DIFF: .get() — the arena is a unique_ptr<T[]> now.
	char* const hierarchyBase = reinterpret_cast<char*>(this->HierarchyBuffer.get());

	for (int level = 2; level >= 0; --level)
	{
		this->HierarchyQueue->Clear();

		const unsigned short startZone = globalPassabilityArray[startZoneRaw].data[level];
		const unsigned short destZone = globalPassabilityArray[destZoneRaw].data[level];

		const bool isTopLevel = (level == 2);

		// DIFF: .get() — these are unique_ptr<T[]> slots.
		int* parentLevelVisited = isTopLevel ? nullptr : this->LevelVisitedMarkers[level + 1].get();
		int* visitedArray = this->LevelVisitedMarkers[level].get();
		int* openSetArray = this->OpenSetMarkers[level].get();
		float* costArray = this->GCostArray[level].get();

		visitedArray[startZone] = this->SearchID;
		visitedArray[destZone] = this->SearchID;

		if (startZone == destZone)
		{
			if (level == 0)
			{
				this->HierarchyBuffer[0].Number = 0;
				this->HierarchyBuffer[0].Index = static_cast<DWORD>(startZone);
			}

			this->PassabilityData[level].Indices[0] = static_cast<unsigned short>(startZone);
			this->PassabilityCounts[level] = 1;
			continue;
		}

		AStarQueueNodeHierarchical* firstElement = this->HierarchyBuffer.get();
		firstElement->BufferDelta = -1;
		firstElement->Index = static_cast<DWORD>(startZone);
		firstElement->Score = 0.0f;
		firstElement->Number = 0;

		if (!this->HierarchyQueue->Push(firstElement))
			return false;

		int elementCount = 1;
		int bufferOffset = 16;   // 1 * sizeof(AStarQueueNodeHierarchical)

		openSetArray[startZone] = this->SearchID;
		costArray[startZone] = 0.0f;

		AStarQueueNodeHierarchical* currentElement = this->HierarchyQueue->Pop();

		if (!currentElement)
			return false;

		const bool noBlockedPairs = this->ZoneIndices[level].IsEmpty();
		auto& subzoneTrackingArray = SubzoneTrackingStruct::Array[level];

		while (true)
		{
			const int currentNode = static_cast<int>(currentElement->Index);

			if (currentNode == static_cast<int>(destZone))
				break;

			SubzoneTrackingStruct& currentSubzone = subzoneTrackingArray.Items[currentNode];
			auto& connections = currentSubzone.SubzoneConnections;
			const int neighborCount = connections.Count;

			for (int n = 0; n < neighborCount; ++n)
			{
				SubzoneConnectionStruct& connection = connections.Items[n];
				const int  neighborNode = static_cast<int>(connection.NeighborSubzoneIndex);
				const char connectionFlag = static_cast<char>(connection.ConnectionPenaltyFlag);

				SubzoneTrackingStruct& neighborSubzone = subzoneTrackingArray.Items[neighborNode];
				const unsigned short parentZone = neighborSubzone.ParentZoneIndex;
				const int            movementType = static_cast<int>(neighborSubzone.MovementCostType);

				int threatCost = 0;

				if (useThreatAvoidance)
				{
					const int rawThreat = MapClass::Instance->subZone_585F40(
						ownerHouse, level, currentNode, neighborNode);
					threatCost = static_cast<int>(static_cast<double>(rawThreat) * threatAvoidance);
				}

				const float connectionPenalty = connectionFlag ? 0.001f : 0.0f;
				const float newCost = gameAdjustments[movementType]
					+ currentElement->Score
					+ static_cast<float>(threatCost)
					+ connectionPenalty;

				if (openSetArray[neighborNode] == this->SearchID
				 && !(costArray[neighborNode] > static_cast<double>(newCost)))
					continue;

				if (!isTopLevel
				 && parentLevelVisited[parentZone] != this->SearchID
				 && movementType != 1)
					continue;

				if (moveZoneArray[movementType] != 1)
					continue;

				// DIFF: replaces the reverse hand-rolled scan over Items[].
				if (!noBlockedPairs)
				{
					unsigned short lo = static_cast<unsigned short>(neighborNode);
					unsigned short hi = static_cast<unsigned short>(currentNode);

					if (lo < hi)
						std::swap(lo, hi);

					const int pairKey = static_cast<int>(lo) | (static_cast<int>(hi) << 16);

					if (this->ZoneIndices[level].Contains(pairKey))
						continue;
				}

				// Allocate new node from the bump arena.
				// The bound is Push() below failing at HierarchyCapacity — see
				// the MinHeap comment in the header before touching that.
				AStarQueueNodeHierarchical* newElement =
					reinterpret_cast<AStarQueueNodeHierarchical*>(hierarchyBase + bufferOffset);

				// BufferDelta: byte offset of parent >> 4 (sizeof == 16)
				newElement->BufferDelta = static_cast<int>(
					(reinterpret_cast<char*>(currentElement) - hierarchyBase) >> 4);
				newElement->Index = static_cast<DWORD>(neighborNode);
				newElement->Score = newCost;
				newElement->Number = currentElement->Number + 1;

				if (!this->HierarchyQueue->Push(newElement))
					continue;

				openSetArray[neighborNode] = this->SearchID;
				costArray[neighborNode] = newCost;
				++elementCount;
				bufferOffset += 16;
			}

			currentElement = this->HierarchyQueue->Pop();

			if (!currentElement)
			{
				Debug::Log("[A* Hierarchical] No path at level %d: zones %u->%u\n",
					level, startZone, destZone);
				return false;
			}
		}

		if (!currentElement)
			return false;

		// Phase 1: walk back and mark visited.
		for (auto* walkNode = currentElement;
			 !IsHierarchyRoot(walkNode);
			 walkNode = HierarchyParent(hierarchyBase, walkNode))
		{
			visitedArray[walkNode->Index] = this->SearchID;
		}

		// Phase 2: store reversed path in PassabilityData.
		const int pathLength = currentElement->Number + 1;
		this->PassabilityCounts[level] = pathLength;

		int   pathIdx = pathLength - 1;
		auto* storeNode = currentElement;

		// DIFF: Indices is std::array now — .data() for the raw walk.
		short* resultPtr = reinterpret_cast<short*>(
			this->PassabilityData[level].Indices.data() + pathIdx);

		while (pathIdx > 0)
		{
			*resultPtr = static_cast<short>(storeNode->Index);
			--resultPtr;
			storeNode = HierarchyParent(hierarchyBase, storeNode);
			--pathIdx;
		}

		this->PassabilityData[level].Indices[0] =
			static_cast<unsigned short>(storeNode->Index);
	}

	return true;
}

// -----------------------------------------------------------------------
// Calc_Float
// No structural changes — helpers already used via AStarHelpers.h.
// -----------------------------------------------------------------------
double PhobosAStarPathFinderClass::Calc_Float(
	CellClass** arg0,
	CellClass** a3,
	int         a4,
	Move         a5,
	FootClass* a6) const
{
	CellClass* targetCell = *a3;
	CellClass* sourceCell = *arg0;

	float cost = MoveCostMultiplier [(int)a5];

	if (a5 == Move::MovingBlock)
	{
		ObjectClass* occupier = GetCellObj(targetCell, a4);
		int          chainDepth = 0;
		bool         chainBroken = false;

		if (!this->FindMode)
		{
			while (occupier)
			{
				if ((occupier->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
				{
					chainBroken = true;
					break;
				}

				FootClass* pFootOccupy = (FootClass*)occupier;
				unsigned int facingIndex;

				if (pFootOccupy->SpeedPercentage == 0.0)
				{
					facingIndex = pFootOccupy->PathDirections[0];

					if (facingIndex == static_cast<unsigned int>(-1))
						break;
				}
				else
				{
					DirStruct dir = pFootOccupy->PrimaryFacing.Current();
					facingIndex = (((static_cast<unsigned int>(dir.Raw) >> 12) + 1) >> 1) & 7u;
				}

				CellStruct occupierCell = occupier->GetMapCoords();
				CellStruct nextCell =
				{
					static_cast<short>(CellSpread::AdjacentCell[facingIndex & 7u].X + occupierCell.X),
					static_cast<short>(occupierCell.Y + CellSpread::AdjacentCell[facingIndex & 7u].Y)
				};

				CellClass* nextCellPtr = MapClass::Instance->GetCellAt(nextCell);
				const bool usesAlt = nextCellPtr->ContainsBridge()
					&& (pFootOccupy->OnBridge
						|| (pFootOccupy->GetCell()->Level - nextCellPtr->Level > 2));

				occupier = GetCellObj(nextCellPtr, usesAlt);

				if (++chainDepth >= 10)
				{
					chainBroken = true;
					break;
				}
			}
		}
		else
		{
			chainBroken = true;
		}

		if (chainBroken || this->FindMode)
			cost = 4.0f;

		if (this->FindMode == 2)
			cost = 1000.0f;
	}

	if (targetCell->UINTFlags & 0x40000)
		cost *= TunnelCostMultiplier;

	if (!a4 || !this->FindBridgeDir)
		return cost;

	const int dx = targetCell->MapCoords.X - sourceCell->MapCoords.X;
	const int dy = targetCell->MapCoords.Y - sourceCell->MapCoords.Y;
	const int dirIndex = CellAdjacencyDirectionLUT[3 * dy + dx + 4];

	CellClass* neighborA;
	CellClass* neighborB;

	if (targetCell->UINTFlags & 0x800)
	{
		neighborA = a3[NeighborIndexTable_Bridge[dirIndex]];
		neighborB = a3[NeighborIndexTable_Bridge[(dirIndex - 4) & 7]];
	}
	else
	{
		neighborA = a3[NeighborIndexTable_Flat[dirIndex]];
		neighborB = a3[NeighborIndexTable_Flat[(dirIndex - 4) & 7]];
	}

	if (neighborA->ContainsBridge())
		return NoBridgeMultiplier * cost;

	const float multiplier = neighborB->ContainsBridge() ? BridgeBothSidesMultiplier : 1.0f;
	return multiplier * cost;
}

// -----------------------------------------------------------------------
// Calc_sqrt — bump-allocates one node out of each arena.
// DIFF: PathQueueBuffer/PathNodeBuffer are unique_ptr; Nodes is std::array.
//       Indexing syntax is unchanged.
// -----------------------------------------------------------------------
AStarWorkPathStruct* PhobosAStarPathFinderClass::Calc_sqrt(
	AStarWorkPathStruct* parentNode,
	CellClass** a3,
	CellStruct* goalCell,
	float                a5)
{

	if (this->PathQueueBuffer->ActiveCount >= this->PathQueueBuffer->Nodes.size()
	 || this->PathNodeBuffer->ActiveCount >= this->PathNodeBuffer->Nodes.size())
	{
		Debug::Log("[A*] node arena exhausted\n");
		return nullptr;
	}

	AStarWorkPathStruct* newQueueNode = &this->PathQueueBuffer->Nodes[this->PathQueueBuffer->ActiveCount++];
	AStarWorkPathStructNode* newPathNode = &this->PathNodeBuffer->Nodes[this->PathNodeBuffer->ActiveCount++];

	newPathNode->Cells = a3;

	if (parentNode)
	{
		newPathNode->Prev = parentNode->Data;

		CellClass* targetCell = *a3;
		CellClass* parentFirst = *parentNode->Data->Cells;
		const int  targetLevel = targetCell->Level;

		newPathNode->CellLevel = targetLevel;

		if (targetCell->ContainsBridge())
		{
			const bool parentIsBridge = parentFirst->ContainsBridge();

			if (parentIsBridge)
			{
				if (parentNode->Data->CellLevel == parentFirst->Level + 4)
					newPathNode->CellLevel = targetLevel + 1;
			}
			else
			{
				const int delta = targetLevel - parentNode->Data->CellLevel + 3;

				if (Math::abs(delta) <= 1)
					newPathNode->CellLevel = newPathNode->CellLevel + 4;
			}
		}
	}
	else
	{
		newPathNode->Prev = nullptr;
		newPathNode->CellLevel = this->StartLevel;
	}

	newQueueNode->Data = newPathNode;

	if (parentNode)
	{
		newQueueNode->MovementCost = a5 + parentNode->MovementCost;
		newQueueNode->PathLength = parentNode->PathLength + 1;
	}
	else
	{
		newQueueNode->MovementCost = 0.0f;
		newQueueNode->PathLength = 1;
	}

	CellClass* targetCell = *a3;
	const int dx = Math::abs(static_cast<int>(targetCell->MapCoords.X) - static_cast<int>(goalCell->X));
	const int dy = Math::abs(static_cast<int>(targetCell->MapCoords.Y) - static_cast<int>(goalCell->Y));

	newQueueNode->PathCost = static_cast<float>(Math::sqrt(
		static_cast<double>(dx * dx + dy * dy))) + newQueueNode->MovementCost;

	return newQueueNode;
}

// -----------------------------------------------------------------------
// Init
// DIFF: the GCostArray zero-fill no longer puns float* to int*. 0.0f has the
//       same bit pattern, so write floats.
// -----------------------------------------------------------------------
void PhobosAStarPathFinderClass::Init()
{
	this->PathNodeBuffer->ActiveCount = 0;
	this->PathQueueBuffer->ActiveCount = 0;

	if (this->PathQueue)
		this->PathQueue->Clear();

	if (this->HierarchyQueue)
		this->HierarchyQueue->Clear();

	++this->SearchID;

	if (this->SearchID != 0)
		return;

	{
		const int regionArea = RegionSize() * RegionSize();

		for (int k = regionArea - 1; k >= 0; --k)
		{
			this->VisitCounts[k + 1] = 0;
			this->AltVisitCounts[k + 1] = 0;
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		const int count = MapClass::Instance->SubzoneCounts[i];

		for (int k = count - 1; k >= 0; --k)
		{
			this->LevelVisitedMarkers[i][k] = 0;
			this->OpenSetMarkers[i][k] = 0;
			this->GCostArray[i][k] = 0.0f;
		}
	}

	++this->SearchID;
}

// -----------------------------------------------------------------------
// IsVisited — trivial, unchanged.
// -----------------------------------------------------------------------
bool PhobosAStarPathFinderClass::IsVisited(int index, bool useAlt) const
{
	return useAlt
		? (this->VisitCounts[index] == this->SearchID)
		: (this->AltVisitCounts[index] == this->SearchID);
}

// -----------------------------------------------------------------------
// CellStruct_helper_distance — unchanged.
// -----------------------------------------------------------------------
int __fastcall PhobosAStarPathFinderClass::CellStruct_helper_distance(CellStruct* a1, CellStruct* a2)
{
	static COMPILETIMEEVAL int InvalidAdjacency = 8;

	const int dy = a2->Y - a1->Y;

	if (Math::abs(dy) > 1)
		return InvalidAdjacency;

	const int dx = a2->X - a1->X;

	if (Math::abs(dx) > 1)
		return InvalidAdjacency;

	return CellAdjacencyDirectionLUT[3 * dy + dx + 4];
}

// -----------------------------------------------------------------------
// Get_Path — unchanged structurally.
// -----------------------------------------------------------------------
PathType* PhobosAStarPathFinderClass::Get_Path(AStarWorkPathStruct* work_path, int* moves)
{
	outpath->Cost = static_cast<int>(work_path->PathCost);
	outpath->Length = work_path->PathLength;
	outpath->field_10 = 0;
	outpath->LastOverlap = { 0, 0 };
	outpath->Command = moves;
	outpath->Overlap = reinterpret_cast<CellStruct*>(MainOverlap());

	int* const overlap = reinterpret_cast<int*>(MainOverlap());

	AStarWorkPathStructNode* curNode = work_path->Data;
	AStarWorkPathStructNode* parentNode = work_path->Data->Prev;

	for (int i = work_path->PathLength - 2; i >= 0; --i)
	{
		if (parentNode)
		{
			overlap[i] = parentNode->CellLevel;

			CellStruct fromCell = (*curNode->Cells)->MapCoords;
			CellStruct toCell = (*parentNode->Cells)->MapCoords;
			moves[i] = this->CellStruct_helper_distance(&fromCell, &toCell);
		}

		curNode = curNode->Prev;
		parentNode = parentNode ? parentNode->Prev : nullptr;
	}

	moves[work_path->PathLength - 1] = -1;
	outpath->Start = (*curNode->Cells)->MapCoords;

	if (!outpath->Cost)
		outpath->Cost = 1;

	return outpath.operator->();
}

// -----------------------------------------------------------------------
// Reset
// DIFF: delete + operator new -> make_unique.
//
// BUG (vanilla): Init() writes VisitCounts[1 .. RegionSize^2], but vanilla
//       allocated exactly RegionSize^2 elements — the last write ran one int
//       past the end on every SearchID wrap. The +1 below fixes it. Drop it
//       only if you want the original corruption back.
//
// NOTE: make_unique value-initialises; vanilla left these as garbage and
//       relied on SearchID stamping. Use make_unique_for_overwrite if the
//       memset matters — nothing reads before writing.
// -----------------------------------------------------------------------
void PhobosAStarPathFinderClass::Reset(RectangleStruct* rect)
{
	RegionSize = rect->Width + rect->Height + 1;

	const std::size_t cellCount =
		static_cast<std::size_t>(RegionSize()) * RegionSize() + 1;

	this->AltVisitCounts = std::make_unique<int[]>(cellCount);
	this->VisitCounts = std::make_unique<int[]>(cellCount);
	this->Distances = std::make_unique<float[]>(cellCount);
	this->AltDistances = std::make_unique<float[]>(cellCount);

	adjust_89A304[0] = -RegionSize();
	adjust_89A304[1] = 1 - RegionSize();
	adjust_89A304[2] = 1;
	adjust_89A304[3] = RegionSize() + 1;
	adjust_89A304[4] = RegionSize();
	adjust_89A304[5] = RegionSize() - 1;
	adjust_89A304[6] = -1;
	adjust_89A304[7] = -1 - RegionSize();
}

// -----------------------------------------------------------------------
// Get_Occupier — unchanged structurally.
// -----------------------------------------------------------------------
FootClass* PhobosAStarPathFinderClass::Get_Occupier(CellStruct* pos, int level) const
{
	const int worldX = (static_cast<int>(pos->X) << 8) + 128;
	const int worldY = (static_cast<int>(pos->Y) << 8) + 128;
	const int worldZ = level * Unsorted::LevelHeight;

	for (int dy = -2; dy < 3; ++dy)
	{
		for (int dx = -2; dx < 3; ++dx)
		{
			CellStruct searchCell = { static_cast<short>(pos->X + dx), static_cast<short>(pos->Y + dy) };
			CellClass* cell = MapClass::Instance->GetCellAt(searchCell);

			const int levelDelta = Math::abs(static_cast<int>(cell->Level) - level);
			ObjectClass* occupier = GetCellObj(cell, (cell->ContainsBridge() && levelDelta > 2));

			while (occupier)
			{
				if (auto pFoot = flag_cast_to<FootClass*>(occupier))
				{
					CoordStruct coord { worldX, worldY, worldZ };

					if (pFoot->Locomotor->Is_Moving_Here(coord))
						return pFoot;
				}

				occupier = occupier->NextObject;
			}
		}
	}

	return nullptr;
}

// -----------------------------------------------------------------------
// Process_Paths — unchanged structurally.
// -----------------------------------------------------------------------
bool PhobosAStarPathFinderClass::Process_Paths(TechnoClass* techno)
{
	if (!this->CanFindPath)
		return false;

	CellStruct curPos = techno->GetMapCoords();
	CellClass* curCell = MapClass::Instance->GetCellAt(curPos);

	DirStruct facingDir = techno->PrimaryFacing.Current();
	const unsigned int facingIdx =
		(((static_cast<unsigned int>(facingDir.Raw) >> 12) + 1) >> 1) & 7u;

	CellStruct frontPos = curPos + CellSpread::AdjacentCell[facingIdx];
	CellClass* frontCell = MapClass::Instance->GetCellAt(frontPos);

	const int levelDelta = Math::abs(
		static_cast<int>(curCell->Level) - static_cast<int>(frontCell->Level));
	const bool useAlt = frontCell->ContainsBridge()
		&& (levelDelta > 3 || techno->OnBridge);

	ObjectClass* occupier = GetCellObj(frontCell, useAlt);

	if (!occupier)
	{
		occupier = this->Get_Occupier(
			&frontCell->MapCoords,
			static_cast<int>(frontCell->Level) + (useAlt ? 4 : 0));
	}

	TechnoTypeClass* myType = techno->GetTechnoType();
	bool             foundBlocker = false;

	while (occupier)
	{
		const AbstractType kind = occupier->WhatAmI();

		if (kind != AbstractType::Unit && kind != AbstractType::Infantry)
		{
			occupier = occupier->NextObject;
			continue;
		}

		CellStruct       occupierPos = ((FootClass*)occupier)->CurrentMapCoords;
		TechnoTypeClass* occupierType = occupier->GetTechnoType();

		const bool shouldYield =
			(this->FindMode == 2)
			|| (myType != occupierType
				&& myType->Speed > occupierType->Speed
				&& MapClass::Instance->IsWithinUsableArea(occupierPos, true));

		if (!shouldYield)
		{
			occupier = occupier->NextObject;
			continue;
		}

		bool pathValid = false;

		if (kind == AbstractType::Unit)
		{
			pathValid = (((FootClass*)occupier)->PathDirections[0] != -1
					  && ((FootClass*)occupier)->PathDirections[1] != -1);
		}
		else
		{
			pathValid = (((FootClass*)occupier)->PathDirections[0] != -1
					  && ((FootClass*)occupier)->PathDirections[1] != -1
					  && ((FootClass*)occupier)->PathDirections[2] != -1);
		}

		if (!pathValid)
		{
			occupier = occupier->NextObject;
			continue;
		}

		foundBlocker = true;

		int* pathStep = ((FootClass*)occupier)->PathDirections;
		int  stepCount = 0;

		while (stepCount < 24 && *pathStep != -1)
		{
			occupierPos = ResolveStep(occupierPos, *pathStep);

			CellClass* stepCell = MapClass::Instance->GetCellAt(occupierPos);
			ToggleBit40000(&stepCell->UINTFlags);
			++pathStep;
			++stepCount;
		}

		occupier = occupier->NextObject;
	}

	if (!foundBlocker && this->FindMode == 1)
	{
		this->FindMode = ASTAR_PASS_0;
		return true;
	}

	for (int dy = -2; dy < 3; ++dy)
	{
		for (int dx = -2; dx < 3; ++dx)
		{
			CellStruct neighborPos =
			{
				static_cast<short>(frontCell->MapCoords.X + dx),
				static_cast<short>(frontCell->MapCoords.Y + dy)
			};
			CellClass* neighbor = MapClass::Instance->GetCellAt(neighborPos);

			if (!neighbor->OccupationFlags)
				continue;

			if (neighbor->MapCoords == curPos)
				continue;

			ToggleBit40000(&neighbor->UINTFlags);
		}
	}

	ToggleBit40000(&frontCell->UINTFlags);
	return false;
}

// -----------------------------------------------------------------------
// tube_42D490
// BUGFIX: original used Items[facing] — should be Items[tubeIdx].
// DIFF: delegates to ResolveStep(), eliminating the duplicate.
// -----------------------------------------------------------------------
CellStruct* __fastcall PhobosAStarPathFinderClass::tube_42D490(CellStruct* a1, CellStruct* a2, int facing)
{
	*a1 = ResolveStep(*a2, facing);
	return a1;
}

// -----------------------------------------------------------------------
// Tube_Crap — ResolveStep() replaces both local copies.
// -----------------------------------------------------------------------
int PhobosAStarPathFinderClass::Tube_Crap(
	FootClass* techno,
	int* dirArray,
	int* levelArray,
	int         pathLen,
	int         lookAhead,
	CellStruct* posPtr)
{
	const int firstDir = dirArray[0];
	const int lastDir = dirArray[pathLen];
	int midDir = (firstDir + lastDir) >> 1;

	if (midDir + 1 != lastDir && midDir + 1 != firstDir)
		midDir = 0;

	if (firstDir == 8 || lastDir == 8)
	{
		const int  totalSteps = pathLen + lookAhead;
		CellStruct pos = *posPtr;

		for (int i = 0; i < totalSteps; ++i)
			pos = ResolveStep(pos, dirArray[i]);

		*posPtr = pos;
		return totalSteps;
	}

	CellStruct a1a = *posPtr;

	if (pathLen < lookAhead)
	{
		lookAhead = pathLen;
	}
	else if (lookAhead < pathLen)
	{
		const int skipCount = pathLen - lookAhead;

		for (int i = 0; i < skipCount; ++i)
			a1a = ResolveStep(a1a, dirArray[i]);
	}

	const double threatFactor = techno->GetThreatAvoidanceCoefficient();
	HouseClass* house = techno->Owner;

	while (lookAhead > 0)
	{
		const int scanCount = 2 * lookAhead;
		int       scanLevel = levelArray[pathLen + lookAhead - scanCount];
		bool      blocked = false;

		CellStruct scanPos = a1a + CellSpread::AdjacentCell[midDir & 7];
		CellClass* scanCell = MapClass::Instance->GetCellAt(scanPos);

		for (int s = scanCount; s > 0; --s)
		{
			if (techno->IsCellOccupied(scanCell, midDir, scanLevel, nullptr, true) == Move::OK
			 && !(scanCell->UINTFlags & 0x40000))
			{
				const int threat = MapClass::Instance->GetThreatPosed(a1a, house);
				blocked = static_cast<double>(threat) * threatFactor >= 1.0;
			}
			else
			{
				blocked = true;
			}

			scanPos += CellSpread::AdjacentCell[midDir & 7];
			scanCell = MapClass::Instance->GetCellAt(scanPos);

			const int newLevel = static_cast<int>(scanCell->Level);
			bool keepBridgeLevel = false;

			if (scanLevel - newLevel == 4)
			{
				scanLevel = newLevel + 4;
				keepBridgeLevel = scanCell->ContainsBridge();
			}

			if (!keepBridgeLevel)
				scanLevel = newLevel;

			if (blocked)
				break;
		}

		if (!blocked)
		{
			const int fillStart = pathLen - lookAhead;
			const int fillCount = 2 * lookAhead;

			if (fillCount > 0)
				std::fill_n(dirArray + fillStart, fillCount, midDir);

			CellStruct pos = *posPtr;
			const int  tailLen = pathLen - lookAhead;

			for (int i = 0; i < tailLen; ++i)
				pos = ResolveStep(pos, dirArray[i]);

			*posPtr = pos;
			return tailLen;
		}

		a1a += CellSpread::AdjacentCell[firstDir & 7];
		--lookAhead;
	}

	{
		CellStruct pos = *posPtr;

		for (int i = 0; i < pathLen; ++i)
			pos = ResolveStep(pos, dirArray[i]);

		*posPtr = pos;
	}

	return pathLen;
}

// -----------------------------------------------------------------------
// Process_Moves — unchanged structurally.
// -----------------------------------------------------------------------
void PhobosAStarPathFinderClass::Process_Moves(PathType* path, FootClass* techno)
{
	int* moves = path->Command;
	int* overlap = reinterpret_cast<int*>(path->Overlap);
	CellStruct start = path->Start;

	const int totalLen = path->Length - 1;

	if (totalLen <= 0)
		return;

	int prevDir = -1;
	int        runLen = 0;
	int        baseIdx = 0;
	int        diagRunLen = 0;
	int        scanOffset = 0;
	bool       inDiagRun = false;
	int expectedDir = -1;

	CellStruct curPos = start;
	CellStruct endCell = start;

	const auto UpdateEndCell = [&](int dir)
		{
			endCell = ResolveStep(endCell, (dir == 8) ? 8 : ((int)dir & 7));
		};

	while (true)
	{
		const bool pastEnd = (diagRunLen + scanOffset >= totalLen)
			|| (baseIdx + runLen >= totalLen);

		if (pastEnd)
		{
			if (inDiagRun)
			{
				this->Tube_Crap(techno,
					(int*)&moves[baseIdx],
					&overlap[baseIdx],
					runLen, diagRunLen, &curPos);
			}

			return;
		}

		if (inDiagRun)
		{
			if (moves[diagRunLen + scanOffset] == expectedDir)
			{
				++diagRunLen;
				continue;
			}

			baseIdx += this->Tube_Crap(techno,
				(int*)&moves[baseIdx],
				&overlap[baseIdx],
				runLen, diagRunLen, &curPos);

			runLen = 1;
			inDiagRun = false;

			const int nextDir = moves[baseIdx] & 7;
			endCell = curPos + CellSpread::AdjacentCell[(int)nextDir];

			prevDir = moves[baseIdx];
			continue;
		}

		const int curDir = moves[baseIdx + runLen];
		const int        delta = ((int)curDir - (int)prevDir) & 7;

		if (curDir == prevDir)
		{
			++runLen;
		}
		else if ((delta == 2 || delta == 6)
			  && prevDir != -1
			  && prevDir != 8
			  && curDir != 8)
		{
			inDiagRun = true;
			expectedDir = curDir;
			diagRunLen = 1;
			scanOffset = baseIdx + runLen;
		}
		else
		{
			baseIdx += runLen;
			runLen = 1;
			prevDir = ((int)curDir & 1) ? curDir : -1;
			curPos = endCell;
		}

		UpdateEndCell(curDir);
	}
}

// -----------------------------------------------------------------------
// Adj_Cell — unchanged structurally.
// -----------------------------------------------------------------------
void PhobosAStarPathFinderClass::Adj_Cell(
	int* dirArray,
	int         startIdx,
	int         minIdx,
	int* outIdx,
	CellStruct* posPtr)
{
	if (startIdx < minIdx)
	{
		*outIdx = minIdx;
		return;
	}

	CellStruct startPos = *posPtr;
	CellStruct walkPos = startPos;

	short accX = 0;
	short accY = 0;
	int   maxDist = 0;
	bool  peaked = false;

	int   curIdx = startIdx;
	int* curPtr = &dirArray[startIdx];

	while (curIdx >= minIdx)
	{
		if (*curPtr == -2)
		{
			--curIdx;
			--curPtr;
			continue;
		}

		const int revDir = (*curPtr - 4) & 7;
		const int adjX = CellSpread::AdjacentCell[revDir].X;
		const int adjY = CellSpread::AdjacentCell[revDir].Y;

		const short newAccX = static_cast<short>(accX + adjX);
		const short newAccY = static_cast<short>(accY + adjY);
		const short newWalkX = static_cast<short>(walkPos.X + adjX);
		const short newWalkY = static_cast<short>(walkPos.Y + adjY);

		const int distX = Math::abs(static_cast<int>(newAccX));
		const int distY = Math::abs(static_cast<int>(newAccY));
		const int dist = std::max(distX, distY);

		if (dist > maxDist)
		{
			if (peaked)
			{
				*outIdx = curIdx + 1;
				const int fwdDir = (revDir - 4) & 7;
				*posPtr =
				{
					static_cast<short>(newWalkX + CellSpread::AdjacentCell[fwdDir].X),
					static_cast<short>(newWalkY + CellSpread::AdjacentCell[fwdDir].Y)
				};
				return;
			}

			maxDist = dist;
		}
		else
		{
			peaked = true;
		}

		accX = newAccX;
		accY = newAccY;
		walkPos = { newWalkX, newWalkY };
		startPos = walkPos;
		--curIdx;
		--curPtr;
	}

	*outIdx = minIdx;
	*posPtr = startPos;
}

// -----------------------------------------------------------------------
// Is_Cell_In_Vector
// DIFF: the reverse hand-rolled scan collapses into ZoneIndexList::Contains.
//       The CellStruct::UnPack round-trip is gone — the key was only ever
//       compared as a packed int.
// -----------------------------------------------------------------------
bool PhobosAStarPathFinderClass::Is_Cell_In_Vector(unsigned int a, unsigned int b, int vectorNum) const
{
	if (b < a)
		std::swap(a, b);

	const int packedKey = static_cast<int>(((b & 0xFFFFu) << 16) | (a & 0xFFFFu));
	return this->ZoneIndices[vectorNum].Contains(packedKey);
}

// -----------------------------------------------------------------------
// UpdateZoneVector
// DIFF: Indices is std::array now — bind by reference instead of a raw ptr.
// -----------------------------------------------------------------------
void PhobosAStarPathFinderClass::UpdateZoneVector(unsigned int zoneValue, int vectorIdx)
{
	const int count = this->PassabilityCounts[vectorIdx];

	if (count <= 1)
	{
		this->IsSearching = false;
		return;
	}

	const auto& data = this->PassabilityData[vectorIdx].Indices;
	int         foundIdx = -1;

	for (int i = 0; i < count; ++i)
	{
		if (data[i] == static_cast<uint16_t>(zoneValue))
		{
			foundIdx = i;
			break;
		}
	}

	if (foundIdx < 0)
	{
		this->IsSearching = false;
		return;
	}

	unsigned int zoneA;
	unsigned int zoneB;

	if (foundIdx == count - 1)
	{
		zoneA = data[foundIdx - 1];
		zoneB = data[foundIdx];
	}
	else
	{
		zoneA = data[foundIdx];
		zoneB = data[foundIdx + 1];
	}

	this->Add_Cell_To_Vector(zoneA, zoneB, vectorIdx);

	auto& subzoneTrackingArray = SubzoneTrackingStruct::Array[vectorIdx];
	SubzoneTrackingStruct& nodeA = subzoneTrackingArray.Items[zoneA];
	SubzoneTrackingStruct& nodeB = subzoneTrackingArray.Items[zoneB];

	const int countA = nodeA.SubzoneConnections.Count - 1;

	for (int i = countA; i >= 0; --i)
	{
		const uint16_t entryZone = static_cast<uint16_t>(
			nodeA.SubzoneConnections[i].NeighborSubzoneIndex);

		if (entryZone == static_cast<uint16_t>(zoneB))
			continue;

		const int countB = nodeB.SubzoneConnections.Count - 1;

		for (int j = countB; j >= 0; --j)
		{
			if (static_cast<uint16_t>(
				nodeB.SubzoneConnections[j].NeighborSubzoneIndex) == entryZone)
			{
				this->Add_Cell_To_Vector(entryZone, zoneA, vectorIdx);
			}
		}
	}
}

// -----------------------------------------------------------------------
// Add_Cell_To_Vector
// DIFF: push_back on ZoneIndexList. Packing is unchanged — the CellStruct is
//       a 4-byte key, not a coordinate.
// -----------------------------------------------------------------------
void PhobosAStarPathFinderClass::Add_Cell_To_Vector(unsigned int a, unsigned int b, int vectorNum)
{
	if (a == b)
		return;

	if (b < a)
		std::swap(a, b);

	this->ZoneIndices[vectorNum].push_back(CellStruct::UnPack((b << 16) | (a & 0xFFFFu)));
}

// -----------------------------------------------------------------------
// Generate_Moves — unchanged structurally.
// -----------------------------------------------------------------------
bool PhobosAStarPathFinderClass::Generate_Moves(
	int* moves,
	int         capacity,
	CellStruct* startCell,
	CellStruct* delta,
	FootClass* techno,
	int* levelPtr,
	bool        tolerateThreats)
{
	static COMPILETIMEEVAL double kThreatAvoidanceThreshold = 0.00001;
	static COMPILETIMEEVAL double kCellThreatThreshold = 0.01;

	const int dx = static_cast<int>(delta->X);
	const int dy = static_cast<int>(delta->Y);

	int diagDir = (dx < 0) ? (dy >= 0 ? 5 : 7) : (dy >= 0 ? 3 : 1);
	int cardDir = ((dx - dy) > 0) ? ((dx + dy) > 0 ? 2 : 0) : ((dx + dy) > 0 ? 4 : 6);

	const int absDx = Math::abs(dx);
	const int absDy = Math::abs(dy);
	int diagSteps = std::min(absDx, absDy);
	int straightSteps = std::max(absDx, absDy) - diagSteps;

	const double threatFactor = techno->GetThreatAvoidanceCoefficient();
	HouseClass* house = techno->Owner;
	const bool  checkThreats = (threatFactor > kThreatAvoidanceThreshold);

	int retryCount = 0;

	while (true)
	{
		int  threatCount = 0;
		bool blocked = false;

		if (diagSteps == 0)
		{
			std::swap(diagDir, cardDir);
			std::swap(diagSteps, straightSteps);

			if (++retryCount >= 2)
				return false;

			continue;
		}

		CellStruct curPos = *startCell;
		int        curLevel = *levelPtr;

		const auto UpdateLevel = [&](CellClass* cell)
			{
				const int cellLevel = static_cast<int>(cell->Level);

				if (curLevel - cellLevel == 4)
				{
					curLevel = cellLevel + 4;

					if (cell->ContainsBridge())
						return;
				}

				curLevel = cellLevel;
			};

		const auto IsBlocked = [&](CellClass* cell, int dir) -> bool
			{
				if (checkThreats)
				{
					const int threat = MapClass::Instance->GetThreatPosed(curPos, house);

					if (static_cast<double>(threat) * threatFactor >= kCellThreatThreshold)
						++threatCount;
				}

				if (techno->IsCellOccupied(cell, dir, curLevel, nullptr, false) != Move::OK)
					return true;

				if (cell->UINTFlags & 0x40000)
					return true;

				if (threatCount > 3)
					return true;

				if (!tolerateThreats && threatCount > 0)
					return true;

				return false;
			};

		{
			int remaining = diagSteps;

			while (remaining > 0 && !blocked)
			{
				const int dirIdx = diagDir & 7;
				curPos =
				{
					static_cast<short>(curPos.X + CellSpread::AdjacentCell[dirIdx].X),
					static_cast<short>(curPos.Y + CellSpread::AdjacentCell[dirIdx].Y)
				};

				CellClass* cell = MapClass::Instance->GetCellAt(curPos);
				blocked = IsBlocked(cell, diagDir);
				UpdateLevel(cell);
				--remaining;
			}
		}

		if (!blocked && straightSteps > 0)
		{
			int remaining = straightSteps;

			while (remaining > 0 && !blocked)
			{
				const int dirIdx = cardDir & 7;
				curPos += CellSpread::AdjacentCell[dirIdx];

				CellClass* cell = MapClass::Instance->GetCellAt(curPos);
				blocked = IsBlocked(cell, cardDir);
				UpdateLevel(cell);
				--remaining;
			}
		}

		if (blocked)
		{
			std::swap(diagDir, cardDir);
			std::swap(diagSteps, straightSteps);

			if (++retryCount >= 2)
				return false;

			continue;
		}

		int writeIdx = 0;

		if (diagSteps > 0)
		{
			std::fill_n(moves, diagSteps, diagDir);
			writeIdx += diagSteps;
		}

		if (straightSteps > 0)
		{
			std::fill_n(moves + writeIdx, straightSteps, cardDir);
			writeIdx += straightSteps;
		}

		const int remainder = capacity - writeIdx;

		if (remainder > 0)
			std::fill_n(moves + writeIdx, remainder, -2);

		return true;
	}
}

// -----------------------------------------------------------------------
// Attempt — unchanged structurally.
// -----------------------------------------------------------------------
unsigned int PhobosAStarPathFinderClass::Attempt(
	CellStruct* startPos,
	CellStruct* destPos,
	FootClass* foot,
	bool         bridge1,
	bool         bridge2,
	MovementZone mzone)
{
	this->IsSearching = true;
	this->Init();

	for (auto& zoneList : this->ZoneIndices)
		zoneList.clear();

	CellClass* startCell = MapClass::Instance->GetCellAt(startPos);
	CellClass* destCell = MapClass::Instance->GetCellAt(destPos);

	CellStruct subStart;
	CellStruct subDest;
	MapClass::Instance->Subzone_bridgecheck_583180(&subStart, startCell, bridge1);
	MapClass::Instance->Subzone_bridgecheck_583180(&subDest, destCell, bridge2);

	MovementZone resolvedMzone = mzone;

	if (mzone == MovementZone::None)
	{
		resolvedMzone = foot
			? foot->GetTechnoType()->MovementZone
			: MovementZone::Normal;
	}

	if (!this->__Find_Path_Hierarchical(&subDest, &subStart, resolvedMzone, foot))
		return 0x7FFFFFFFu;

	const int baseDist = ChebyshevDist(*startPos, *destPos);
	const int passCount0 = this->PassabilityCounts[0];
	int       penalty = passCount0 * 2 - 2;

	const auto TrySubzone = [&](CellClass* cell, CellStruct* endpoint,
								uint16_t entry, CellStruct* fallback) -> bool
		{
			CellStruct result;
			MapClass::Instance->Subzone_583820(&result, cell, 0, entry);

			if (result == CellStruct::Empty)
			{
				if (!fallback)
					return false;

				result = *fallback;
			}

			if (result != CellStruct::Empty)
				penalty += ChebyshevDist(result, *endpoint);

			return (result != CellStruct::Empty);
		};

	bool bridge2Done = false;

	if (bridge2)
	{
		if (bridge1)
		{
			const ZoneType zoneA = MapClass::Instance->GetMovementZoneType(destPos, MovementZone::AmphibiousDestroyer, 0);
			const ZoneType zoneB = MapClass::Instance->GetMovementZoneType(startPos, MovementZone::AmphibiousDestroyer, 0);

			if (zoneA == zoneB && zoneA != ZoneType::None)
				return static_cast<unsigned int>(baseDist);
		}

		if (passCount0 >= 4)
		{
			const uint16_t entry = this->PassabilityData[0].Indices[passCount0 - 2];

			if (TrySubzone(destCell, destPos, entry, nullptr))
				bridge2Done = true;
		}

		if (!bridge2Done)
		{
			const uint16_t entry = this->PassabilityData[0].Indices[passCount0 - 1];
			TrySubzone(destCell, destPos, entry, &subDest);
		}
	}

	if (bridge1)
	{
		bool bridge1Done = false;

		if (passCount0 >= 4)
		{
			const uint16_t entry = this->PassabilityData[0].Indices[1];

			if (TrySubzone(startCell, startPos, entry, nullptr))
				bridge1Done = true;
		}

		if (!bridge1Done)
		{
			const uint16_t entry = this->PassabilityData[0].Indices[0];
			TrySubzone(startCell, startPos, entry, &subStart);
		}
	}

	return static_cast<unsigned int>(std::max(penalty, baseDist));
}

// -----------------------------------------------------------------------
// Fill_DVector
//
// DO NOT convert tempVec to std::vector. Subzone_5840C0 is vanilla code at a
// hardcoded address that writes into the DynamicVectorClass layout directly —
// handing it a std::vector corrupts the stack.
// -----------------------------------------------------------------------
void PhobosAStarPathFinderClass::Fill_DVector(FootClass* techno)
{
	const int startZoneIdx = MapClass::Instance->MapClass_zone_56D3F0(&this->CellStructBuffer);
	GlobalPassabilityData& startZoneStruct =
		MapClass::Instance->LevelAndPassabilityStruct2pointer_70[startZoneIdx];

	for (int passIdx = 0; passIdx < 3; ++passIdx)
	{
		const uint16_t startZoneValue = startZoneStruct.data[passIdx];

		DynamicVectorClass<uint16_t> tempVec {};   // vanilla ABI — keep
		CellClass* cell = MapClass::Instance->GetCellAt(this->CellStructBuffer);

		if (MapClass::Instance->Subzone_5840C0(cell, passIdx, &tempVec, techno))
		{
			const int cellZoneIdx = MapClass::Instance->MapClass_zone_56D3F0(&this->CellStructBuffer);
			const uint16_t cellZoneValue =
				MapClass::Instance->LevelAndPassabilityStruct2pointer_70[cellZoneIdx].data[passIdx];

			this->UpdateZoneVector(static_cast<unsigned int>(cellZoneValue), passIdx);
			continue;
		}

		for (int i = tempVec.Count - 1; i >= 0; --i)
		{
			this->Add_Cell_To_Vector(
				static_cast<unsigned int>(tempVec[i]),
				static_cast<unsigned int>(startZoneValue),
				passIdx);
		}
	}
}

// -----------------------------------------------------------------------
// AllocZoneArrays
// DIFF: delete + operator new + fill_n collapse into make_unique, which
//       value-initialises — the explicit zero-fill is redundant now.
// -----------------------------------------------------------------------

void PhobosAStarPathFinderClass::AllocZoneArrays(AStarPathFinderClass* pOriginal)
{
	for (int i = 0; i < 3; ++i) {
		const std::size_t count = (size_t)MapClass::Instance->SubzoneCounts[i];

		this->LevelVisitedMarkers[i] = std::make_unique<int[]>(count);
		this->OpenSetMarkers[i] = std::make_unique<int[]>(count);
		this->GCostArray[i] = std::make_unique<float[]>(count);
	}
}

// -----------------------------------------------------------------------
// Find_Path
// DIFF: ZoneIndices clear loop uses a range-for.
// -----------------------------------------------------------------------
PathType* PhobosAStarPathFinderClass::Find_Path(
	CellStruct* start,
	CellStruct* dest,
	FootClass* techno,
	int* moves,
	int                  maxCount,
	MovementZone         mzoneOverride,
	AStarPostProcessType findModeOverride)
{
	this->IsSearching = true;
	this->Init();

	for (auto& zoneList : this->ZoneIndices)
		zoneList.clear();

	this->FindMode = findModeOverride;

	CellClass* startCell = MapClass::Instance->GetCellAt(start);
	CellClass* destCell = MapClass::Instance->GetCellAt(dest);

	const MovementZone resolvedMzone = (mzoneOverride == MovementZone::None)
		? techno->GetTechnoType()->MovementZone
		: mzoneOverride;

	const int startZone = MapClass::Instance->GetMapZone(start, resolvedMzone, techno->OnBridge);

	const MovementZone destMzone = (mzoneOverride == MovementZone::None)
		? techno->GetTechnoType()->MovementZone
		: mzoneOverride;

	const bool destIsBridge = (destCell->UINTFlags >> 8) & 1;
	const int  destZone = MapClass::Instance->GetMapZone(dest, destMzone, destIsBridge);

	CellStruct subStart;
	CellStruct subDest;
	MapClass::Instance->Subzone_bridgecheck_583180(&subStart, startCell, techno->OnBridge);
	MapClass::Instance->Subzone_bridgecheck_583180(&subDest, destCell, destIsBridge);

	if (mzoneOverride == MovementZone::None)
		mzoneOverride = techno->GetTechnoType()->MovementZone;

	// VERIFY: infantry tunnel locomotion COM query — partial, see original for full detail
	if (techno->WhatAmI() == AbstractType::Infantry)
	{
		auto* infType = ((InfantryClass*)techno)->Type;

		if (infType->JumpJet)
		{
			mzoneOverride = MovementZone::Infantry;
			/*auto& loco = techno->Locomotor;

			static COMPILETIMEEVAL GUID CLSID_IPersist = {
				0x0000010C, 0x0000, 0x0000,
				{ 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
			};

			IPersistStream* pStm;
			loco.QueryInterface(CLSID_IPersist, (void**)&pStm);

			ULARGE_INTEGER _size;

			if (pStm->GetSizeMax(&_size))
				pStm->Release();*/
		}
	}

	bool useHierarchical = false;
	{
		const bool isTrain = techno->GetTechnoType()->IsTrain;
		const bool isLocked = techno->IsInPlayfield;
		const bool allowedToLeave = techno->IsAllowedToLeaveMap();
		const bool startInRadar = MapClass::Instance->IsWithinUsableArea(subStart, true);
		const bool destInRadar = MapClass::Instance->IsWithinUsableArea(subDest, true);

		useHierarchical = !isTrain && isLocked && !allowedToLeave
			&& startInRadar && destInRadar;
	}

	if (startZone == destZone)
	{
		if (useHierarchical
		 && !this->__Find_Path_Hierarchical(&subStart, &subDest,
			 static_cast<MovementZone>(mzoneOverride), techno))
		{
			Debug::Log("Hierarchical findpath failure: (%d,%d) to (%d, %d)\n",
				subStart.X, subStart.Y, subDest.X, subDest.Y);
			useHierarchical = false;
		}
	}
	else if (useHierarchical)
	{
		return nullptr;
	}

	const int maxRetries = (maxCount == -1) ? 5 : 1;
	int       retryCount = 0;
	PathType* result = nullptr;

	while (true)
	{
		if (!useHierarchical && (subStart.X != subDest.X || subStart.Y != subDest.Y))
		{
			Debug::Log("Warning.  A* without HS: (%d,%d) to (%d, %d)\n",
				subStart.X, subStart.Y, subDest.X, subDest.Y);
		}

		result = this->Find_Path_Regular(start, dest, techno, moves, maxCount, useHierarchical);

		if (result || !useHierarchical)
			break;

		{
			const int cheb = std::max(
				Math::abs(static_cast<int>(start->Y) - static_cast<int>(dest->Y)),
				Math::abs(static_cast<int>(start->X) - static_cast<int>(dest->X)));

			if (cheb > 1)
			{
				Debug::Log("Regular findpath failure: (%d,%d) to (%d, %d)\n",
					subStart.X, subStart.Y, subDest.X, subDest.Y);
			}
		}

		++retryCount;
		this->Fill_DVector(techno);
		this->Init();

		useHierarchical = (this->IsSearching != 0);

		if (retryCount >= maxRetries)
			break;

		if (useHierarchical
		 && !this->__Find_Path_Hierarchical(&subStart, &subDest,
			 static_cast<MovementZone>(mzoneOverride), (FootClass*)techno))
			break;
	}

	return result;
}

// -----------------------------------------------------------------------
// Calc_Moves — unchanged structurally.
// -----------------------------------------------------------------------
void PhobosAStarPathFinderClass::Calc_Moves(PathType* path, FootClass* techno)
{
	auto moves = path->Command;
	int* overlap = reinterpret_cast<int*>(path->Overlap);
	CellStruct curCell = path->Start;

	const int totalSteps = path->Length - 1;

	int stepIdx = 0;
	int validCount = 0;

	short accDeltaX = 0;
	short accDeltaY = 0;
	short prevDeltaX = 0;
	short prevDeltaY = 0;

	int maxAbsX = 0;
	int maxAbsY = 0;
	int maxChebyshev = 0;

	CellStruct pivotCell = { 0, 0 };
	int        pivotStep = 0;
	int        adjStep = 0;

	int* stepPtr = moves;

	if (totalSteps > 0)
	{
		while (stepIdx < totalSteps)
		{
			if (stepIdx >= 20)
				break;

			const int dir = *stepPtr;

			if (dir == 8)
			{
				curCell += CellSpread::AdjacentCell[0];

				++stepIdx;
				++stepPtr;

				accDeltaX = 0;
				accDeltaY = 0;
				prevDeltaX = 0;
				prevDeltaY = 0;
				maxAbsX = 0;
				maxAbsY = 0;
				maxChebyshev = 0;
				pivotCell = { 0, 0 };
				adjStep = stepIdx;
				pivotStep = stepIdx;
				validCount = 0;
				continue;
			}

			if (dir == -2)
			{
				++stepIdx;
				++stepPtr;
				validCount = 0;
				continue;
			}

			const int   dirIdx = (int)dir & 7;
			const short adjX = CellSpread::AdjacentCell[dirIdx].X;
			const short adjY = CellSpread::AdjacentCell[dirIdx].Y;

			const short newAccX = static_cast<short>(accDeltaX + adjX);
			const short newAccY = static_cast<short>(accDeltaY + adjY);
			const short newPrevX = static_cast<short>(prevDeltaX + adjX);
			const short newPrevY = static_cast<short>(prevDeltaY + adjY);

			const int absNewPrevX = Math::abs(static_cast<int>(newPrevX));
			const int absNewPrevY = Math::abs(static_cast<int>(newPrevY));

			if (absNewPrevX < maxAbsX || absNewPrevY < maxAbsY)
			{
				if (pivotCell.X || pivotCell.Y)
				{
					adjStep = pivotStep;
					accDeltaX = static_cast<short>(pivotCell.X - curCell.X);
					accDeltaY = static_cast<short>(pivotCell.Y - curCell.Y);
					maxAbsX = 0;
					maxAbsY = 0;
					prevDeltaX = 0;
					prevDeltaY = 0;

					const int absDX = Math::abs(static_cast<int>(accDeltaX));
					const int absDY = Math::abs(static_cast<int>(accDeltaY));
					maxChebyshev = std::max(absDX, absDY);
					pivotCell = curCell;
					pivotStep = stepIdx;
				}
				else
				{
					maxAbsX = 0;
					maxAbsY = 0;
					prevDeltaX = 0;
					prevDeltaY = 0;
					pivotCell = curCell;
					pivotStep = stepIdx;
				}

				validCount = 0;
				continue;
			}

			maxAbsY = absNewPrevY;
			prevDeltaX = newPrevX;
			prevDeltaY = newPrevY;
			maxAbsX = absNewPrevX;

			const int absAccX = Math::abs(static_cast<int>(newAccX));
			const int absAccY = Math::abs(static_cast<int>(newAccY));
			const int newCheb = std::max(absAccX, absAccY);

			const CellStruct stepCell = curCell + CellSpread::AdjacentCell[dirIdx];
			curCell = stepCell;

			if (maxChebyshev >= newCheb)
			{
				CellStruct adjOutCell = stepCell;
				int        adjOutStep = 0;

				this->Adj_Cell((int*)moves, stepIdx, adjStep, &adjOutStep, &adjOutCell);

				const CellStruct delta = stepCell - adjOutCell;

				this->Generate_Moves(
					((int*)&moves[adjOutStep]),
					stepIdx - adjOutStep + 1,
					&adjOutCell,
					const_cast<CellStruct*>(&delta),
					techno,
					&overlap[adjOutStep],
					false);
			}
			else
			{
				maxChebyshev = newCheb;
			}

			++stepIdx;
			accDeltaX = newAccX;
			accDeltaY = newAccY;
			++stepPtr;
			validCount = 0;
		}

		if (pivotCell.X || pivotCell.Y)
		{
			const int absDX = Math::abs(static_cast<int>(curCell.X - pivotCell.X));
			const int absDY = Math::abs(static_cast<int>(curCell.Y - pivotCell.Y));
			const int pivotCheb = std::max(absDX, absDY);

			if ((stepIdx - 1 - pivotStep) > pivotCheb)
			{
				CellStruct adjOutCell = curCell;
				int        adjOutStep = 0;

				this->Adj_Cell((int*)moves, stepIdx - 1, pivotStep, &adjOutStep, &adjOutCell);

				const CellStruct delta = curCell - adjOutCell;;

				this->Generate_Moves(
					((int*)&moves[adjOutStep]),
					stepIdx - adjOutStep,
					&adjOutCell,
					const_cast<CellStruct*>(&delta),
					techno,
					&overlap[adjOutStep],
					true);
			}
		}
	}

	// Compact out the -2 holes.
	{
		int* readPtr = moves;
		int* writePtr = moves;
		int readStep = 0;
		validCount = 0;

		int cur = (int)*readPtr;

		if (cur != -1)
		{
			while (readStep < totalSteps && cur != -1)
			{
				if (cur != -2)
				{
					*writePtr++ = *readPtr;
					++validCount;
				}

				cur = (int)readPtr[1];
				++readPtr;
				++readStep;
			}
		}
	}

	{
		const int padUntil = path->Length + 1;

		for (int i = validCount; i < padUntil; ++i)
			moves[i] = -1;
	}

	path->Length = validCount + 1;
}

// -----------------------------------------------------------------------
// Find_Path_Regular
// DIFF: VisitStamper borrows raw pointers via .get().
// DIFF: QueueAdvance replaces the 3 pop/peek/push-back patterns and no longer
//       touches a public Count field — MinHeap::Top() returns null when empty.
// BUGFIX: the facing==8 / tubeIdx==-1 branch used to write through an
//       uninitialised neighborCellPtr. It now just skips the tube.
// -----------------------------------------------------------------------
PathType* PhobosAStarPathFinderClass::Find_Path_Regular(
	CellStruct* start,
	CellStruct* dest,
	FootClass* techno,
	int* moves,
	int         maxCount,
	bool        useHierarchical)
{
	CellClass** destCellPtr = CellArrayPtr(dest->X, dest->Y);
	CellClass** startCellPtr = CellArrayPtr(start->X, start->Y);

	CellClass* destCell = *destCellPtr;
	CellClass* startCell = *startCellPtr;

	if (!destCell || !startCell)
		return nullptr;

	const bool technoIsAircraft = (techno->WhatAmI() == AbstractType::Aircraft);

	int endLevel = static_cast<int>(destCell->Level);

	if (!technoIsAircraft && destCell->ContainsBridge())
		endLevel += 4;

	this->EndLevel = endLevel;

	int startLevel = static_cast<int>(startCell->Level);

	if (!technoIsAircraft && techno->OnBridge)
		startLevel += 4;

	this->StartLevel = startLevel;

	if (techno->GetTechnoType()->IsTrain && startCell->ContainsBridge())
	{
		const int coordLevel = techno->GetCoords().Z / Unsorted::LevelHeight;

		if (Math::abs(coordLevel - this->StartLevel) > 2)
			this->StartLevel += 4;
	}

	this->FinderSpeedType = static_cast<SpeedType>(techno->GetTechnoType()->SpeedType);
	this->PathLength = 0;
	this->CellStructBuffer = *start;

	int* zoneArray = this->LevelVisitedMarkers[0].get();

	AStarWorkPathStruct* workPath = this->Calc_sqrt(nullptr, startCellPtr, dest, 0.0f);

	if (start->X == dest->X && start->Y == dest->Y && this->StartLevel == this->EndLevel)
		return nullptr;

	if (this->FindMode)
		this->Process_Paths(reinterpret_cast<TechnoClass*>(techno));

	// Borrowed view over the Reset()-owned buffers. Must not outlive this call.
	VisitStamper stamper {
		this->VisitCounts.get(),
		this->AltVisitCounts.get(),
		this->Distances.get(),
		this->AltDistances.get(),
		this->SearchID
	};

	const int startFlatIdx =
		static_cast<int>(startCell->MapCoords.Y) * RegionSize()
		+ static_cast<int>(startCell->MapCoords.X);

	stamper.Stamp(startFlatIdx, 0.0f, this->StartLevel <= static_cast<int>(startCell->Level));

	const bool isTrain = techno->GetTechnoType()->IsTrain;

	if (isTrain)
	{
		DirStruct facingDir = techno->PrimaryFacing.Current();
		const unsigned int technoFacing =
			(((static_cast<unsigned int>(facingDir.Raw) >> 12) + 1) >> 1) & 7u;

		for (int facingIdx = 0; facingIdx < static_cast<int>(dword_7E3774.size()); ++facingIdx)
		{
			const int diff = Math::abs(static_cast<int>(technoFacing) - facingIdx);

			if (diff <= 2 || diff >= 6)
				continue;

			CellClass* neighbor = startCellPtr[dword_7E3774[facingIdx]];

			if (!neighbor)
				continue;

			const int flatIdx =
				static_cast<int>(neighbor->MapCoords.Y) * RegionSize()
				+ static_cast<int>(neighbor->MapCoords.X);
			const int neighborLevel = static_cast<int>(neighbor->Level) + 1;

			stamper.Stamp(flatIdx, 0.0f, this->StartLevel <= neighborLevel);
		}
	}

	bool isPassive = false;

	if (auto pUnit = cast_to<UnitClass*, false>(techno)) {
		if (pUnit->Type->Passive || TechnoExtContainer::Instance.Find(techno)->Is_DriverKilled)
			isPassive = true;
	}

	if (maxCount < 0)
		maxCount = 0xFFF7;

	if (!workPath)
	{
		if (this->FindMode)
			this->Process_Paths(techno);

		return nullptr;
	}

	// Selects the lower-cost head between bestNode and the queue top, pushing
	// the other one back.
	const auto QueueAdvance = [&](AStarWorkPathStruct*& work,
								  AStarWorkPathStruct* best) -> bool
		{
			if (!best)
			{
				work = this->PathQueue->Pop();
				return work != nullptr;
			}

			AStarWorkPathStruct* const top = this->PathQueue->Top();

			if (!top || top->PathCost > best->PathCost)
			{
				work = best;
				return true;
			}

			this->PathQueue->Pop();
			this->PathQueue->Push(best);
			work = top;
			return true;
		};

	int  iterCount = 0;
	bool destinationReached = false;

	while (workPath)
	{
		if (iterCount >= maxCount)
			break;

		CellClass** curCells = workPath->Data->Cells;
		CellClass* curFirstCell = *curCells;

		if (curCells == destCellPtr && workPath->Data->CellLevel == this->EndLevel)
		{
			destinationReached = true;
			break;
		}

		const int curFlatIdx =
			static_cast<int>(curFirstCell->MapCoords.Y) * RegionSize()
			+ static_cast<int>(curFirstCell->MapCoords.X);

		AStarWorkPathStruct* bestNode = nullptr;

		for (int facing = 0; facing <= 8 && !destinationReached; ++facing)
		{
			CellClass** neighborCellPtr = nullptr;

			if (facing == 8)
			{
				const int tubeIdx = static_cast<int>(curFirstCell->TubeIndex);

				// BUGFIX: was `*neighborCellPtr = nullptr;` through an
				//         uninitialised pointer.
				if (tubeIdx == -1)
					continue;

				const CellStruct endCell = TubeClass::Array->Items[tubeIdx]->ExitCell;
				neighborCellPtr = CellArrayPtr(endCell.X, endCell.Y);
			}
			else
			{
				neighborCellPtr = &curCells[dword_7E3774[facing]];
			}

			CellClass* neighborCell = *neighborCellPtr;

			if (!neighborCell)
				continue;

			int neighborFlatIdx;

			if (facing == 8)
			{
				neighborFlatIdx =
					static_cast<int>(neighborCell->MapCoords.Y) * RegionSize()
					+ static_cast<int>(neighborCell->MapCoords.X);
			}
			else
			{
				neighborFlatIdx = curFlatIdx + adjust_89A304[facing];
			}

			bool useAlt = true;

			if (neighborCell->ContainsBridge())
			{
				const int levelDiff = Math::abs(this->StartLevel - static_cast<int>(neighborCell->Level));
				useAlt = (levelDiff <= 1);
			}

			const int zone_ = MapClass::Instance->MapClass_zone_56D3F0(&neighborCell->MapCoords);
			const int zoneVal = static_cast<int>(
				MapClass::Instance->LevelAndPassabilityStruct2pointer_70[zone_].data[0]);

			if (zoneArray[zoneVal] != this->SearchID)
			{
				if (useAlt && !neighborCell->BlockedNeighbours && useHierarchical)
					continue;
			}

			const float prevCost = workPath->MovementCost;

			if (stamper.IsVisited(neighborFlatIdx, useAlt)
			 && stamper.GetDist(neighborFlatIdx, useAlt) < static_cast<double>(prevCost) + dbl_7E37C0)
				continue;

			Move canEnter = techno->IsCellOccupied(
				neighborCell,
				facing,
				this->StartLevel,
				*curCells,
				this->IsAlt);

			if (isTrain && canEnter < Move::No)
				canEnter = Move::OK;

			float stepCost;

			if (facing == 8)
			{
				const int dx = Math::abs(static_cast<int>(curFirstCell->MapCoords.X)
									  - static_cast<int>(neighborCell->MapCoords.X));
				const int dy = Math::abs(static_cast<int>(curFirstCell->MapCoords.Y)
									  - static_cast<int>(neighborCell->MapCoords.Y));
				stepCost = static_cast<float>(std::max(dx, dy));
			}
			else
			{
				const float calcFloat = static_cast<float>(this->Calc_Float(
					curCells, neighborCellPtr,
					useAlt ? 0 : 1,
					canEnter,
					techno));
				stepCost = calcFloat * this->PathCostFactor + adjust_81872C[facing];
			}

			if (canEnter >= Move::No)
			{
				if (neighborCellPtr == destCellPtr && !isPassive)
				{
					const int levelDiff = Math::abs(this->StartLevel - this->EndLevel);

					if (levelDiff <= 1)
					{
						destinationReached = true;
						break;
					}
				}

				continue;
			}

			if (stamper.IsVisited(neighborFlatIdx, useAlt))
				continue;

			AStarWorkPathStruct* newNode = this->Calc_sqrt(
				workPath, neighborCellPtr, dest, stepCost);

			if (!newNode)
				break;

			if (!bestNode || newNode->PathCost < bestNode->PathCost)
			{
				if (bestNode)
					this->PathQueue->Push(bestNode);

				bestNode = newNode;
			}
			else
			{
				this->PathQueue->Push(newNode);
			}

			stamper.Stamp(neighborFlatIdx, newNode->MovementCost, useAlt);

			if (zoneVal == static_cast<int>(
				this->PassabilityData[0].Indices[this->PathLength + 1]))
			{
				++this->PathLength;
				this->CellStructBuffer = neighborCell->MapCoords;
			}
		}

		if (destinationReached)
			break;

		QueueAdvance(workPath, bestNode);

		if (workPath)
			this->StartLevel = workPath->Data->CellLevel;

		++iterCount;
	}

	if (iterCount == 10000
	 || !workPath
	 || iterCount == maxCount
	 || workPath->PathLength < 2)
	{
		if (this->FindMode)
			this->Process_Paths(techno);

		return nullptr;
	}

	PathType* result = this->Get_Path(workPath, moves);
	this->Process_Moves(result, techno);
	this->Calc_Moves(result, techno);

	if (this->FindMode)
		this->Process_Paths(techno);

	return result;
}

// ===========================================================================
// Hook table
//
// Every method the game calls must land here, or vanilla code will walk the
// new PathQueue / ZoneIndices as if they were the old types.
//
// VERIFY: the ctor and dtor addresses are not in the reversed listing you gave
//         me — fill them in and hook them, otherwise the game constructs the
//         vanilla layout at 0x87E8B8 at startup and every unique_ptr member
//         starts life as garbage.

PhobosAStarPathFinderClass PhobosAStarPathFinderClass::Instance;

double FakeAStarPathFinderClass::Calc_Float(CellClass** arg0, CellClass** a3, int a4, Move a5, FootClass* a6) const
{
	return PhobosAStarPathFinderClass::Instance.Calc_Float(arg0, a3, a4, a5, a6);
}

PathType* FakeAStarPathFinderClass::Find_Path_Regular(CellStruct* start, CellStruct* dest, FootClass* techno,
	int* moves, int maxCount, bool useHierarchical)
{
	return PhobosAStarPathFinderClass::Instance.Find_Path_Regular(start, dest, techno, moves, maxCount, useHierarchical);
}

void FakeAStarPathFinderClass::Reset(RectangleStruct* rect)
{
	PhobosAStarPathFinderClass::Instance.Reset(rect);
}

void FakeAStarPathFinderClass::AllocZoneArrays()
{
	PhobosAStarPathFinderClass::Instance.AllocZoneArrays(this);
}

PathType* FakeAStarPathFinderClass::Find_Path(CellStruct* start, CellStruct* dest, FootClass* techno, int* moves,
	int maxCount, MovementZone mzoneOverride, AStarPostProcessType findModeOverride)
{
	return PhobosAStarPathFinderClass::Instance.Find_Path(start, dest , techno , moves , maxCount , mzoneOverride , findModeOverride);
}

unsigned int FakeAStarPathFinderClass::Attempt(CellStruct* startPos, CellStruct* destPos, FootClass* foot,
	bool bridge1, bool bridge2, MovementZone mzone)
{
	return PhobosAStarPathFinderClass::Instance.Attempt(startPos, destPos, foot, bridge1, bridge2, mzone);
}

//ASMJIT_PATCH(0x4B4023, DriveLoco_Track, 0x7)
//{
//	GET(int, something, EAX);
//	GET(int, path1, ESI);
//	GET(int, path0, EBX);
//
//	Debug::LogInfo("Path0 {} , path1 {} ,array {}", path0, path1, something);
//	return 0x0;
//}


// ===========================================================================
//DEFINE_FUNCTION_JUMP(LJMP, 0x429830, FakeAStarPathFinderClass::Calc_Float)
//DEFINE_FUNCTION_JUMP(LJMP, 0x429A90, FakeAStarPathFinderClass::Find_Path_Regular)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42AC00, FakeAStarPathFinderClass::Reset)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42C1C0, FakeAStarPathFinderClass::AllocZoneArrays)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42C900, FakeAStarPathFinderClass::Find_Path)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42D170, FakeAStarPathFinderClass::Attempt)
 
//DEFINE_FUNCTION_JUMP(LJMP, 0x42A460, FakeAStarPathFinderClass::Calc_sqrt)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42A5B0, FakeAStarPathFinderClass::Init)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42A690, FakeAStarPathFinderClass::IsVisited)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42AA40, FakeAStarPathFinderClass::CellStruct_helper_distance)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42AA90, FakeAStarPathFinderClass::Get_Path)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42ACF0, FakeAStarPathFinderClass::Process_Paths)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42B080, FakeAStarPathFinderClass::Get_Occupier)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42B210, FakeAStarPathFinderClass::Process_Moves)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42B420, FakeAStarPathFinderClass::Tube_Crap)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42B7F0, FakeAStarPathFinderClass::Calc_Moves)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42BCA0, FakeAStarPathFinderClass::Adj_Cell)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42BE20, FakeAStarPathFinderClass::Generate_Moves)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42C290, FakeAStarPathFinderClass::__Find_Path_Hierarchical)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42CCD0, FakeAStarPathFinderClass::Fill_DVector)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42CEB0, FakeAStarPathFinderClass::Is_Cell_In_Vector)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42CF10, FakeAStarPathFinderClass::Add_Cell_To_Vector)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42CF80, FakeAStarPathFinderClass::UpdateZoneVector)
//DEFINE_FUNCTION_JUMP(LJMP, 0x42D490, FakeAStarPathFinderClass::tube_42D490)

#endif

std::vector<CellStruct> AStarClass::LineCells;
std::vector<unsigned short> AStarClass::StraightSubzones[3];
std::vector<int> AStarClass::IsStraightFlag[3];

static bool InitStaticAStarContainers()
{
	if (AStarClass::EnableRectilinear)
	{
		AStarClass::LineCells.reserve(400);
		AStarClass::LineCells.clear();

		for (int i = 0; i < 3; ++i)
		{
			AStarClass::StraightSubzones[i].reserve(100);
			AStarClass::StraightSubzones[i].clear();
			AStarClass::IsStraightFlag[i].clear();
			AStarClass::IsStraightFlag[i].resize((3 - i) * (3 - i) * 100, 0);
		}
	}

	return true;
}
bool AStarClass::ContainersInit = InitStaticAStarContainers();

static inline int GetMovementPassability(MovementZone movementZone, PassabilityType passType)
{
	return MapClass::MovementAdjustArray[static_cast<int>(movementZone)][static_cast<int>(passType)];
}

static inline GlobalPassabilityData& GetPassabilityStruct(const CellStruct* pCell)
{
	return MapClass::Instance->LevelAndPassabilityStruct2pointer_70[MapClass::Instance->GetCellPathIndex(*pCell)];
}

static inline unsigned short GetCellPassabilityIndex(const CellStruct* pCell, int level)
{
	return static_cast<unsigned short>(GetPassabilityStruct(pCell).data[level]);
}

#pragma region NextPathCell

CellStruct AStarClass::NextPathCell(
	const CellStruct cell,
	const int dir
)
{
	if (dir != 8)
		return cell + CellSpread::AdjacentCell[dir];

	const int tubeIndex = MapClass::Instance->GetCellAt(cell)->TubeIndex;
	return (tubeIndex == -1) ? CellStruct::Empty : TubeClass::Array->Items[tubeIndex]->ExitCell;
}

#pragma endregion

#pragma region FindRegularPath

AStarClass::PathFinderData* AStarClass::FindRegularPath(
	const CellStruct* const pStart,
	const CellStruct* const pEnd,
	const FootClass* const pFoot,
	int* const pDirs,
	int maxSteps,
	const bool useHierarchical
)
{
	int step = 0;

	const auto pCellsFromEnd = &MapClass::Instance->Cells.Items[MapClass::GetCellIndex(*pEnd)];
	const auto pEndCell = *pCellsFromEnd;
	const auto pCellsFromStart = &MapClass::Instance->Cells.Items[MapClass::GetCellIndex(*pStart)];
	const auto pStartCell = *pCellsFromStart;

	if (!pStartCell || !pEndCell)
		return nullptr;

	const auto absType = pFoot->WhatAmI();
	this->EndLevel = (absType == AbstractType::Aircraft || !pEndCell->ContainsBridge()) ? pEndCell->Level : pEndCell->Level + 4;
	this->StartLevel = (absType == AbstractType::Aircraft || !pFoot->OnBridge) ? pStartCell->Level : pStartCell->Level + 4;
	const auto pType = pFoot->GetTechnoType();
	this->FinderSpeedType = pType->SpeedType;
	const bool isTrain = pType->IsTrain;

	if (isTrain && pStartCell->ContainsBridge() && Math::abs(pFoot->Location.Z / Unsorted::LevelHeight - this->StartLevel) > 2)
		this->StartLevel += 4;

	const auto pCount = this->LevelVisitedMarkers[0];
	this->PathLength = 0;
	this->CellStructBuffer = *pStart;

	auto pPathNode = this->CreatePathNode(0, pCellsFromStart, pEnd, 0.0f);

	do
	{
		if (pStart->X == pEnd->X && pStart->Y == pEnd->Y && this->StartLevel == this->EndLevel)
			return nullptr;

		if (this->FindMode)
			this->AStarClass::PostProcessCells(pFoot);

		const int astarStartIndex = pStartCell->MapCoords.X + AStarClass::MapSides() * pStartCell->MapCoords.Y;

		if (this->StartLevel <= pStartCell->Level)
		{
			this->VisitCounts[astarStartIndex] = this->SearchID;
			this->Distances[astarStartIndex] = 0.0f;
		}
		else
		{
			this->AltVisitCounts[astarStartIndex] = this->SearchID;
			this->AltDistances[astarStartIndex] = 0.0f;
		}

		if (isTrain)
		{
			int dirOffset = 0;

			for (const auto dirCellOffset : AStarClass::DirCellOffsets)
			{
				const int absFace = Math::abs(static_cast<int>(pFoot->PrimaryFacing.Current().GetFacing<8>()) - dirOffset);

				if (absFace > 2 && absFace < 6)
				{
					const auto pAdjCell = pCellsFromStart[dirCellOffset];
					const int astarAdjIndex = pAdjCell->MapCoords.X + AStarClass::MapSides() * pAdjCell->MapCoords.Y;

					if (this->StartLevel <= pAdjCell->Level + 1)
					{
						this->VisitCounts[astarAdjIndex] = this->SearchID;
						this->Distances[astarAdjIndex] = 0.0f;
					}
					else
					{
						this->AltVisitCounts[astarAdjIndex] = this->SearchID;
						this->AltDistances[astarAdjIndex] = 0.0f;
					}
				}

				++dirOffset;
			}
		}

		const bool isPassiveUnit = absType == AbstractType::Unit && static_cast<const UnitClass*>(pFoot)->Type->Passive;

		if (maxSteps < 0)
			maxSteps = 65527;

		if (pPathNode)
		{
			do
			{
				if (step >= maxSteps)
					break;

				const auto pCellsInChecking = pPathNode->NodeData->CellItems;

				if (pCellsInChecking == pCellsFromEnd && pPathNode->NodeData->Level == this->EndLevel)
					break;

				AStarClass::PathQueueNode* pBestCandidateNode = nullptr;
				int astarCheckCellIndex = (*pCellsInChecking)->MapCoords.X + AStarClass::MapSides() * (*pCellsInChecking)->MapCoords.Y;

				for (int dir = 0; dir <= 8; ++dir)
				{
					const CellClass* const* pCellsPtr = nullptr;

					if (dir == 8)
					{
						const int tubeIndex = (*pCellsInChecking)->TubeIndex;

						if (tubeIndex == -1)
							pCellsPtr = &AStarClass::InvalidCell();
						else
							pCellsPtr = &MapClass::Instance->Cells.Items[MapClass::GetCellIndex(TubeClass::Array->Items[tubeIndex]->ExitCell)];
					}
					else
					{
						pCellsPtr = &pCellsInChecking[AStarClass::DirCellOffsets[dir]];
					}

					const auto pCell = *pCellsPtr;

					if (!pCell)
						continue;

					const auto& checkCell = pCell->MapCoords;
					const int astarCheckNextIndex = (dir == 8) ? (checkCell.X + AStarClass::MapSides() * checkCell.Y) : (astarCheckCellIndex + AStarClass::DirSides[dir]);
					const bool notAlternate = !pCell->ContainsBridge() || Math::abs(this->StartLevel - pCell->Level) <= 1;
					const unsigned short cellPassabilityIndex = GetCellPassabilityIndex(&checkCell, 0);

					if (notAlternate
						? (pCount[cellPassabilityIndex] != this->SearchID && !pCell->BlockedNeighbours && useHierarchical
							|| this->VisitCounts[astarCheckNextIndex] == this->SearchID && this->Distances[astarCheckNextIndex] < (pPathNode->PathCost + 1.009))
						: (this->AltVisitCounts[astarCheckNextIndex] == this->SearchID && this->AltDistances[astarCheckNextIndex] < (pPathNode->PathCost + 1.009)))
					{
						continue;
					}

					auto moveType = pFoot->IsCellOccupied(const_cast<CellClass*>(pCell), dir, this->StartLevel, const_cast<CellClass*>(*pCellsInChecking), this->IsAlt);

					if (isTrain && moveType < Move::No)
						moveType = Move::OK;

					const float cost = static_cast<float>((dir == 8)
						? MaxImpl(Math::abs((*pCellsInChecking)->MapCoords.X - checkCell.X), Math::abs((*pCellsInChecking)->MapCoords.Y - checkCell.Y))
						: this->CalculateMoveCost(pCellsInChecking, pCellsPtr, !notAlternate, moveType, pFoot) * this->PathCostFactor + AStarClass::DirPathCosts[dir]);

					if (moveType >= Move::No)
					{
						if (pCellsPtr == pCellsFromEnd && !isPassiveUnit && Math::abs(this->StartLevel - this->EndLevel) <= 1)
							goto BREAK_STEP_LOOP;

						continue;
					}

					if (notAlternate ? (this->VisitCounts[astarCheckNextIndex] == this->SearchID) : (this->AltVisitCounts[astarCheckNextIndex] == this->SearchID))
						continue;

					auto pNewPathNode = this->CreatePathNode(pPathNode, pCellsPtr, pEnd, cost);

					do
					{
						if (pBestCandidateNode)
						{
							const auto pPathQueue = this->PathQueue;
							const int count = pPathQueue->Count;
							int newCount = count + 1;

							if (newCount < pPathQueue->Capacity)
							{
								if (count == -1)
								{
									const auto pQueueNodes = pPathQueue->Nodes;
									int harfNewCount = newCount >> 1;
									const float newCost = pNewPathNode->TotalCost;

									for (; newCount > 1; harfNewCount >>= 1)
									{
										const auto pParentNode = pQueueNodes[harfNewCount];

										if (pParentNode->TotalCost <= newCost)
											break;

										pQueueNodes[newCount] = pParentNode;
										newCount = harfNewCount;
									}

									pQueueNodes[newCount] = pNewPathNode;
									++pPathQueue->Count;

									if ((uintptr_t)pNewPathNode > pPathQueue->LMost)
										pPathQueue->LMost = (uintptr_t)pNewPathNode;

									if ((uintptr_t)pNewPathNode < pPathQueue->RMost)
										pPathQueue->RMost = (uintptr_t)pNewPathNode;

									break;
								}
								else
								{
									const auto pQueueNodes = pPathQueue->Nodes;
									int harfNewCount = newCount >> 1;
									const float newCost = pBestCandidateNode->TotalCost;

									for (; newCount > 1; harfNewCount >>= 1)
									{
										const auto pParentNode = pQueueNodes[harfNewCount];

										if (pParentNode->TotalCost <= newCost)
											break;

										pQueueNodes[newCount] = pParentNode;
										newCount = harfNewCount;
									}

									pQueueNodes[newCount] = pBestCandidateNode;
									++pPathQueue->Count;

									if ((uintptr_t)pBestCandidateNode > pPathQueue->LMost)
										pPathQueue->LMost = (uintptr_t)pBestCandidateNode;

									if ((uintptr_t)pBestCandidateNode < pPathQueue->RMost)
										pPathQueue->RMost = (uintptr_t)pBestCandidateNode;
								}
							}
						}

						pBestCandidateNode = pNewPathNode;
					}
					while (false);

					if (notAlternate)
					{
						this->VisitCounts[astarCheckNextIndex] = this->SearchID;
						this->Distances[astarCheckNextIndex] = pNewPathNode->PathCost;
					}
					else
					{
						this->AltVisitCounts[astarCheckNextIndex] = this->SearchID;
						this->AltDistances[astarCheckNextIndex] = pNewPathNode->PathCost;
					}

					const int newPathLength = this->PathLength + 1;

					if (cellPassabilityIndex == static_cast<unsigned short>(this->PassabilityData[0].Indices[newPathLength]))
					{
						this->PathLength = newPathLength;
						this->CellStructBuffer = checkCell;
					}
				}

				const auto pPathQueue = this->PathQueue;

				if (!pBestCandidateNode)
				{
					if (const int count = pPathQueue->Count)
					{
						const auto pQueueNodes = pPathQueue->Nodes;
						const auto pExtractedNode = pQueueNodes[1];
						pQueueNodes[1] = pQueueNodes[count];
						pQueueNodes[count] = nullptr;
						const int newCount = count - 1;
						pPathQueue->Count = newCount;

						int heapIndex = 1;
						int childIndex = (newCount < 2 || pQueueNodes[1]->TotalCost <= pQueueNodes[2]->TotalCost) ? 1 : 2;

						do
						{
							if (newCount < 3 || pQueueNodes[childIndex]->TotalCost <= pQueueNodes[3]->TotalCost)
							{
								if (childIndex == 1)
									break;
							}
							else
							{
								childIndex = 3;
							}

							do
							{
								std::swap(pQueueNodes[heapIndex], pQueueNodes[childIndex]);
								heapIndex = childIndex;
								const int leftChildIndex = 2 * childIndex;
								const int rightChildIndex = leftChildIndex + 1;

								if (leftChildIndex <= newCount && pQueueNodes[childIndex]->TotalCost > pQueueNodes[leftChildIndex]->TotalCost)
									childIndex = leftChildIndex;

								if (rightChildIndex <= newCount && pQueueNodes[childIndex]->TotalCost > pQueueNodes[rightChildIndex]->TotalCost)
									childIndex = rightChildIndex;
							}
							while (childIndex != heapIndex);
						}
						while (false);

						pPathNode = pExtractedNode;
					}
					else
					{
						pPathNode = nullptr;
						++step;
						break;
					}
				}
				else
				{
					do
					{
						if (const int count = pPathQueue->Count)
						{
							const auto pQueueNodes = pPathQueue->Nodes;
							const auto pExtractedNode = pQueueNodes[1];

							if (pExtractedNode->TotalCost <= pBestCandidateNode->TotalCost)
							{
								pQueueNodes[1] = pBestCandidateNode;

								int heapIndex = 1;
								int childIndex = (count < 2 || pQueueNodes[1]->TotalCost <= pQueueNodes[2]->TotalCost) ? 1 : 2;

								do
								{
									if (count < 3 || pQueueNodes[childIndex]->TotalCost <= pQueueNodes[3]->TotalCost)
									{
										if (childIndex == 1)
											break;
									}
									else
									{
										childIndex = 3;
									}

									do
									{
										std::swap(pQueueNodes[heapIndex], pQueueNodes[childIndex]);
										heapIndex = childIndex;
										const int leftChildIndex = 2 * childIndex;
										const int rightChildIndex = leftChildIndex + 1;

										if (leftChildIndex <= count && pQueueNodes[childIndex]->TotalCost > pQueueNodes[leftChildIndex]->TotalCost)
											childIndex = leftChildIndex;

										if (rightChildIndex <= count && pQueueNodes[childIndex]->TotalCost > pQueueNodes[rightChildIndex]->TotalCost)
											childIndex = rightChildIndex;
									}
									while (childIndex != heapIndex);
								}
								while (false);

								pPathNode = pExtractedNode;
								break;
							}
						}

						pPathNode = pBestCandidateNode;
					}
					while (false);
				}

				if (pPathNode)
					this->StartLevel = pPathNode->NodeData->Level;

				++step;
			}
			while (pPathNode);

		BREAK_STEP_LOOP:
			if (step != 10000 && pPathNode && step != maxSteps && pPathNode->NodeCount >= 2)
				break;
		}

		if (this->FindMode)
			this->PostProcessCells(pFoot);

		return nullptr;
	}
	while (false);

	const auto pData = this->BuildFinalPath(pPathNode, pDirs);

	this->ProcessFinalPath(pData, pFoot);
	this->OptimizeFinalPath(pData, pFoot);

	if (this->FindMode)
		this->PostProcessCells(pFoot);

	return pData;
}

#pragma endregion

#pragma region FindHierarchicalPath

bool AStarClass::FindHierarchicalPath(
	const CellStruct* const pStart,
	const CellStruct* const pEnd,
	const MovementZone movementZone,
	const FootClass* const pFoot
)
{
	constexpr float RECTILINEAR_DISCOUNT = 0.1f;
	if (AStarClass::EnableRectilinear)
	{
		AStarClass::LineCells.clear();
		const short x0 = pStart->X;
		const short y0 = pStart->Y;
		const short x1 = pEnd->X;
		const short y1 = pEnd->Y;
		const short dx = static_cast<short>(Math::abs(x1 - x0));
		const short sx = x0 < x1 ? 1 : -1;
		const short dy = static_cast<short>(Math::abs(y1 - y0));
		const short sy = y0 < y1 ? 1 : -1;
		short err = dx - dy;
		short x = x0;
		short y = y0;

		while (true)
		{
			AStarClass::LineCells.emplace_back(x, y);

			if (x == x1 && y == y1)
				break;

			const int e2 = 2 * err;

			if (e2 > -dy)
			{
				err -= dy;
				x += sx;
			}

			if (e2 < dx)
			{
				err += dx;
				y += sy;
			}
		}
	}

	auto FindRectilinearPath = [this, pStart, pEnd, movementZone](int level) -> bool
		{
			auto& straightSubzones = AStarClass::StraightSubzones[level];
			auto& straightFlags = AStarClass::IsStraightFlag[level];
			straightSubzones.clear();

			{
				int lastIdx = -1;
				for (auto& _cell : AStarClass::LineCells)
				{
					const int idx = GetCellPassabilityIndex(&_cell, level);
					if (idx != lastIdx)
					{
						straightSubzones.emplace_back(static_cast<unsigned short>(idx));
						lastIdx = idx;
					}
				}
			}

			if (straightSubzones.empty())
				return false;

			auto AreConnected = [level, movementZone](unsigned int fromIdx, unsigned int toIdx) -> bool
				{
					if (fromIdx == toIdx)
						return true;

					const auto pSubzoneTracking = MapClass::Instance->SubzoneTrackings[level].Items;
					const auto pFinderSubzoneConnections = &pSubzoneTracking[fromIdx].SubzoneConnections;
					for (int i = 0; i < pFinderSubzoneConnections->Count; ++i)
					{
						if (pFinderSubzoneConnections->Items[i].ConnectionPenaltyFlag == toIdx)
							return GetMovementPassability(movementZone, static_cast<PassabilityType>(pSubzoneTracking[toIdx].MovementCostType)) == 1;
					}

					return false;
				};

			const size_t straightSubzoneCount = straightSubzones.size();
			for (size_t i = 0; i + 1 < straightSubzoneCount; ++i)
			{
				if (!AreConnected(straightSubzones[i], straightSubzones[i + 1]))
				{
					const size_t subzoneCount = static_cast<size_t>(MapClass::Instance->SubzoneTrackings[level].Count);

					if (straightFlags.size() < subzoneCount)
						straightFlags.resize(subzoneCount, 0);

					for (int idx : straightSubzones)
						straightFlags[idx] = this->SearchID;

					return false;
				}
			}

			this->PassabilityCounts[level] = static_cast<int>(straightSubzoneCount);

			auto& data = this->PassabilityData[level];
			for (size_t i = 0; i < straightSubzoneCount; ++i)
				data.Indices[i] = straightSubzones[i];

			auto pLevelVisitedMarkers = this->LevelVisitedMarkers[level];
			for (int idx : straightSubzones)
				pLevelVisitedMarkers[idx] = this->SearchID;

			return true;
		};

	double threatAvoidanceCoefficient = 0.0;
	HouseClass* pOwner = nullptr;
	bool calculateThreat = false;

	if (pFoot)
	{
		threatAvoidanceCoefficient = pFoot->GetThreatAvoidanceCoefficient();
		pOwner = pFoot->Owner;
		calculateThreat = threatAvoidanceCoefficient > 0.00001;
	}

	const auto& sourceStruct = GetPassabilityStruct(pStart);
	const auto& targetStruct = GetPassabilityStruct(pEnd);

	int level = 2;

	while (true)
	{
		do
		{
			{
				const auto pHierarchyQueue = this->HierarchyQueue;

				for (int i = 0; i <= pHierarchyQueue->Count; pHierarchyQueue->Nodes[i - 1] = nullptr)
					++i;

				pHierarchyQueue->Count = 0;
			}

			const int sourceSubzoneIndex = static_cast<unsigned short>(sourceStruct.data[level]);
			const int targetSubzoneIndex = static_cast<unsigned short>(targetStruct.data[level]);

			if (AStarClass::EnableRectilinear && FindRectilinearPath(level))
				break;

			const bool isMaxLevel = level == 2;
			const auto pSuperiorLevelVisitedMarkers = isMaxLevel ? nullptr : this->LevelVisitedMarkers[level + 1];

			const auto pLevelVisitedMarkers = this->LevelVisitedMarkers[level];
			const auto pOpenSetMarkers = this->OpenSetMarkers[level];
			const auto pGCostArray = this->GCostArray[level];

			pLevelVisitedMarkers[sourceSubzoneIndex] = this->SearchID;
			pLevelVisitedMarkers[targetSubzoneIndex] = this->SearchID;

			if (sourceSubzoneIndex == targetSubzoneIndex)
			{
				if (!level)
				{
					const auto pBufferNodes = this->HierarchyBuffer->Nodes;
					pBufferNodes->Count = 0;
					pBufferNodes->SubzoneIndex = sourceSubzoneIndex;
				}

				this->PassabilityData[level].Indices[0] = static_cast<unsigned short>(sourceSubzoneIndex);
				this->PassabilityCounts[level] = 1;
			}
			else
			{
				const auto pFirstNode = &this->HierarchyBuffer->Nodes[0];
				pFirstNode->PreviousNodeIndex = -1;
				pFirstNode->SubzoneIndex = sourceSubzoneIndex;
				pFirstNode->Cost = 0.0f;
				pFirstNode->Count = 0;

				pOpenSetMarkers[sourceSubzoneIndex] = this->SearchID;
				pGCostArray[sourceSubzoneIndex] = 0.0f;
				auto pFinderNode = pFirstNode;

				int bufferIndex = 1;

				const auto pZoneIndices = &this->ZoneIndices[level];
				const bool noZoneIndices = pZoneIndices->Count <= 0;

				while (true)
				{
					const int finderSubzoneIndex = pFinderNode->SubzoneIndex;

					if (finderSubzoneIndex == targetSubzoneIndex)
						break;

					const auto pSubzoneTracking = MapClass::Instance->SubzoneTrackings[level].Items;
					const auto pFinderSubzoneConnections = &pSubzoneTracking[finderSubzoneIndex].SubzoneConnections;
					int subzoneTrackingConnectionsCount = pFinderSubzoneConnections->Count;

					if (subzoneTrackingConnectionsCount > 0)
					{
						auto pSubzoneTrackingConnectionsItem = pFinderSubzoneConnections->Items;

						do
						{
							const int checkSubzoneIndex = static_cast<int>(pSubzoneTrackingConnectionsItem->NeighborSubzoneIndex);
							const bool isDiagonalConnection = pSubzoneTrackingConnectionsItem->ConnectionPenaltyFlag;
							const auto pCheckSubzoneTracking = &pSubzoneTracking[checkSubzoneIndex];
							const int checkSubzoneSuperiorIndex = pCheckSubzoneTracking->ParentZoneIndex;
							const auto checkSubzonePassability = static_cast<PassabilityType>(pCheckSubzoneTracking->MovementCostType);

							float cost = static_cast<float>(AStarClass::PassabilityCoefficients.operator[](static_cast<int>(checkSubzonePassability))
								+ pFinderNode->Cost
								+ (!calculateThreat ? 0 : static_cast<int>(MapClass::Instance->GetThreatPosedEstimates(pOwner, level, finderSubzoneIndex, checkSubzoneIndex) * threatAvoidanceCoefficient))
								+ (isDiagonalConnection ? 0.001f : 0.0f));

							if (AStarClass::EnableRectilinear)
							{
								const auto& straightFlags = AStarClass::IsStraightFlag[level];
								if (checkSubzoneIndex < static_cast<int>(straightFlags.size()) && straightFlags[checkSubzoneIndex] == this->SearchID)
									cost *= RECTILINEAR_DISCOUNT;
							}

							const int searchID = this->SearchID;

							if ((pOpenSetMarkers[checkSubzoneIndex] != searchID
								|| pGCostArray[checkSubzoneIndex] > cost)
								&& (isMaxLevel
									|| pSuperiorLevelVisitedMarkers[checkSubzoneSuperiorIndex] == searchID
									|| checkSubzonePassability == PassabilityType::Crushable)
								&& GetMovementPassability(movementZone, checkSubzonePassability) == 1)
							{
								do
								{
									if (!noZoneIndices)
									{
										const unsigned int mixIndex = static_cast<unsigned short>(checkSubzoneIndex) < static_cast<unsigned short>(finderSubzoneIndex)
											? static_cast<unsigned short>(finderSubzoneIndex) | (static_cast<unsigned short>(checkSubzoneIndex) << 16)
											: static_cast<unsigned short>(checkSubzoneIndex) | (static_cast<unsigned short>(finderSubzoneIndex) << 16);

										auto ShouldProcessNode = [pZoneIndices, mixIndex]() -> bool
											{
												int zoneIndicesNewCount = pZoneIndices->Count - 1;
												auto pZoneIndex = &pZoneIndices->Items[zoneIndicesNewCount];
												while (*pZoneIndex != mixIndex)
												{
													--zoneIndicesNewCount;
													--pZoneIndex;

													if (zoneIndicesNewCount < 0)
														return true;
												}

												return false;
											};

										if (!ShouldProcessNode())
											break;
									}

									if (bufferIndex >= 10000)
										return false;

									const auto pHierarchyBuffer = this->HierarchyBuffer;
									const auto pHierarchicalNode = &pHierarchyBuffer->Nodes[bufferIndex++];
									pHierarchicalNode->PreviousNodeIndex = pFinderNode - &pHierarchyBuffer->Nodes[0];
									pHierarchicalNode->SubzoneIndex = checkSubzoneIndex;
									pHierarchicalNode->Cost = cost;
									pHierarchicalNode->Count = pFinderNode->Count + 1;

									const auto pHierarchyQueue = this->HierarchyQueue;
									const auto pQueueNodes = pHierarchyQueue->Nodes;
									int newCount = pHierarchyQueue->Count + 1;
									int harfNewCount = newCount >> 1;

									if (newCount < pHierarchyQueue->Capacity)
									{
										for (; newCount > 1; harfNewCount >>= 1)
										{
											const auto pParentNode = pQueueNodes[harfNewCount];

											if (pParentNode->Cost <= cost)
												break;

											pQueueNodes[newCount] = pParentNode;
											newCount = harfNewCount;
										}

										pQueueNodes[newCount] = pHierarchicalNode;
										++pHierarchyQueue->Count;

										if ((uintptr_t)pHierarchicalNode > pHierarchyQueue->LMost)
											pHierarchyQueue->LMost = (uintptr_t)pHierarchicalNode;

										if ((uintptr_t)pHierarchicalNode < pHierarchyQueue->RMost)
											pHierarchyQueue->RMost = (uintptr_t)pHierarchicalNode;
									}

									pOpenSetMarkers[checkSubzoneIndex] = this->SearchID;
									pGCostArray[checkSubzoneIndex] = cost;
								}
								while (false);
							}

							++pSubzoneTrackingConnectionsItem;
							--subzoneTrackingConnectionsCount;
						}
						while (subzoneTrackingConnectionsCount);
					}

					const auto pHierarchyQueue = this->HierarchyQueue;
					const int count = pHierarchyQueue->Count;

					if (!count)
						return false;

					const auto pQueueNodes = pHierarchyQueue->Nodes;
					const auto pExtractedNode = pQueueNodes[1];
					pQueueNodes[1] = pQueueNodes[count];
					pQueueNodes[count] = nullptr;
					const int newCount = count - 1;
					pHierarchyQueue->Count = newCount;

					int heapIndex = 1;
					int childIndex = (newCount < 2 || pQueueNodes[1]->Cost <= pQueueNodes[2]->Cost) ? 1 : 2;

					do
					{
						if (newCount < 3 || pQueueNodes[childIndex]->Cost <= pQueueNodes[3]->Cost)
						{
							if (childIndex == 1)
								break;
						}
						else
						{
							childIndex = 3;
						}

						do
						{
							std::swap(pQueueNodes[heapIndex], pQueueNodes[childIndex]);
							heapIndex = childIndex;
							const int leftChildIndex = 2 * childIndex;
							const int rightChildIndex = leftChildIndex + 1;

							if (leftChildIndex <= newCount && pQueueNodes[childIndex]->Cost > pQueueNodes[leftChildIndex]->Cost)
								childIndex = leftChildIndex;

							if (rightChildIndex <= newCount && pQueueNodes[childIndex]->Cost > pQueueNodes[rightChildIndex]->Cost)
								childIndex = rightChildIndex;
						}
						while (childIndex != heapIndex);
					}
					while (false);

					pFinderNode = pExtractedNode;

					if (!pFinderNode)
						return false;
				}

				this->PassabilityCounts[level] = pFinderNode->Count + 1;

				if (pFinderNode->PreviousNodeIndex != -1)
				{
					auto pDataIndex = &this->PassabilityData[level].Indices[pFinderNode->Count];

					do
					{
						pLevelVisitedMarkers[pFinderNode->SubzoneIndex] = this->SearchID;
						*pDataIndex-- = static_cast<unsigned short>(pFinderNode->SubzoneIndex);
						pFinderNode = &this->HierarchyBuffer->Nodes[pFinderNode->PreviousNodeIndex];
					}
					while (pFinderNode->PreviousNodeIndex != -1);
				}

				this->PassabilityData[level].Indices[0] = static_cast<unsigned short>(pFinderNode->SubzoneIndex);
			}
		}
		while (false);

		if (--level >= 0)
			continue;

		return true;
	}
}

#pragma endregion

#pragma region CalculateMoveCost

double AStarClass::CalculateMoveCost(
	const CellClass* const* const pFromCellPtr,
	const CellClass* const* const pToCellPtr,
	const bool isAlternate,
	const Move moveType,
	const FootClass* const pFoot
) const
{
	const auto pToCell = *pToCellPtr;
	const auto pFromCell = *pFromCellPtr;
	float moveCost = AStarClass::MoveCosts[static_cast<int>(moveType)];

	if (moveType == Move::MovingBlock)
	{
		if (const int mode = this->FindMode)
		{
			moveCost = (mode == 2) ? 1000.0f : 4.0f;
		}
		else
		{
			auto pCellObj = isAlternate ? pToCell->AltObject : pToCell->FirstObject;

			for (int step = 0; pCellObj; )
			{
				if (const auto pCellFoot = flag_cast_to<FootClass*, true>(pCellObj))
				{
					int dir = 0;

					if (pCellFoot->SpeedPercentage == 0.0)
					{
						dir = pCellFoot->PathDirections[0];

						if (dir == -1)
							break;
					}
					else
					{
						dir = pCellFoot->PrimaryFacing.Current().GetFacing<8>();
					}

					const auto pAdjCell = MapClass::Instance->GetCellAt(CellSpread::AdjacentCell[dir & 7] + pCellObj->GetMapCoords());
					pCellObj = (pAdjCell->ContainsBridge() && (pCellObj->OnBridge || (pCellObj->GetCell()->Level - pAdjCell->Level) > 2))
						? pAdjCell->AltObject
						: pAdjCell->FirstObject;

					if (++step < 10)
						continue;
				}

				moveCost = 4.0f;
				break;
			}
		}
	}

	const auto flags = pToCell->Flags;

	if (flags & CellFlags::Tube)
		moveCost *= 4.0f;

	if (!isAlternate || !this->FindBridgeDir)
		return moveCost;

	const auto deltaCell = pToCell->MapCoords - pFromCell->MapCoords;
	const int dirOffsetIndex = AStarClass::BridgeDirOffsets[4 + (3 * deltaCell.Y) + deltaCell.X];
	const auto& [pBridgeCheckCell, pBridgeReverseCheckCell] = (flags & CellFlags::BridgeDir)
		? std::make_pair(pToCellPtr[AStarClass::BridgeDir1OffsetIndexes[dirOffsetIndex]], pToCellPtr[AStarClass::BridgeDir1OffsetIndexes[(dirOffsetIndex - 4) & 7]])
		: std::make_pair(pToCellPtr[AStarClass::BridgeDir0OffsetIndexes[dirOffsetIndex]], pToCellPtr[AStarClass::BridgeDir0OffsetIndexes[(dirOffsetIndex - 4) & 7]]);

	if (!pBridgeCheckCell->ContainsBridge())
		return 10.0f * moveCost;

	return pBridgeReverseCheckCell->ContainsBridge() ? 2.0f * moveCost : moveCost;
}

#pragma region ProcessFinalPath

#pragma endregion

void AStarClass::ProcessFinalPath(
	PathFinderData* const pPath,
	const FootClass* const pFoot
) const
{
	const int pathLength = pPath->PathLength - 1;

	if (pathLength <= 0)
		return;

	const int* const pLevels = pPath->Levels;
	int* const pDirs = pPath->Directions;

	int lastDir = -1;
	int diagonalDir = -1;
	int step = 0;
	int segmentLength = 0;
	bool adjustSegment = false;
	int adjustIndex = 0;
	int adjustOffset = 0;

	auto currentCell = pPath->StartCell;
	auto nextCell = currentCell;

	do
	{
		if (adjustSegment)
		{
			if (pDirs[adjustOffset + adjustIndex] != lastDir)
			{
				step += AStarClass::AdjustFinalPath(pFoot, &pDirs[step], &pLevels[step], segmentLength, adjustOffset, &currentCell);

				segmentLength = 1;
				adjustSegment = false;

				nextCell = CellSpread::AdjacentCell[pDirs[step] & 7] + currentCell;
				diagonalDir = pDirs[step];
			}
			else
			{
				++adjustOffset;
			}
		}
		else
		{
			const int currentDir = pDirs[step + segmentLength];
			const int diffDir = (currentDir - diagonalDir) & 7;

			if (currentDir == diagonalDir)
			{

				++segmentLength;
			}
			else if ((diffDir != 2 && diffDir != 6) 
				|| diagonalDir == -1
				|| diagonalDir == 8
				|| currentDir == 8)
			{
				step += segmentLength;
				segmentLength = 1;

				diagonalDir = (currentDir & 1) ? currentDir : -1;
				currentCell = nextCell;
			}
			else
			{
				adjustSegment = true;
				lastDir = pDirs[step + segmentLength];
				adjustOffset = 1;
				adjustIndex = step + segmentLength;
			}

			nextCell = AStarClass::NextPathCell(nextCell, currentDir);
		}
	}
	while ((step + segmentLength) < pathLength && (adjustOffset + adjustIndex) < pathLength);

	if (adjustSegment)
		AStarClass::AdjustFinalPath(pFoot, &pDirs[step], &pLevels[step], segmentLength, adjustOffset, &currentCell);
}

#pragma endregion

#pragma region AdjustFinalPath

int AStarClass::AdjustFinalPath(
	const FootClass* const pFoot,
	int* const pDirs,
	const int* const pLevels,
	const int steps,
	int offset,
	CellStruct* const pCurrent
) const
{
	const int firstDir = pDirs[0];
	const int lastDir = pDirs[steps];

	int avgDir = (firstDir + lastDir) >> 1;
	{
		const int checkDir = avgDir + 1;

		if (checkDir != lastDir && checkDir != firstDir)
			avgDir = 0;
	}

	if (firstDir == 8 || lastDir == 8)
	{
		const int count = steps + offset;

		if (count > 0)
		{
			int i = 0;
			auto cell = *pCurrent;

			do
			{
				cell = AStarClass::NextPathCell(cell, pDirs[i]);
			}
			while (++i < count);

			*pCurrent = cell;
		}

		return count;
	}

	auto midCell = *pCurrent;

	if (steps < offset)
	{
		offset = steps;
	}
	else if (offset < steps)
	{
		const int count = steps - offset;

		if (count > 0)
		{
			int i = 0;
			auto cell = midCell;

			do
			{
				cell = AStarClass::NextPathCell(cell, pDirs[i]);
			}
			while (++i < count);
			midCell = cell;
		}
	}

	const double threat = pFoot->GetThreatAvoidanceCoefficient();
	const auto pOwner = pFoot->Owner;

	if (offset > 0)
	{
		const auto& dirOffset = CellSpread::AdjacentCell[avgDir & 7];

		int remainingOffset = 2 * offset;

		do
		{
			int currentLevel = pLevels[steps + offset - remainingOffset];
			auto nextCell = midCell + dirOffset;
			auto pCell = MapClass::Instance->GetCellAt(nextCell);

			int i = remainingOffset;
			bool blocked = false;

			do
			{
				if (i <= 0)
				{
					const int size = 2 * offset;
					const int count = steps - offset;

					if (size > 0)
						std::fill_n(pDirs + count, size, avgDir);

					if (count > 0)
					{
						int j = 0;
						auto cell = *pCurrent;

						do
						{
							cell = AStarClass::NextPathCell(cell, pDirs[j]);
						}
						while (++j < count);

						*pCurrent = cell;
					}

					return count;
				}

				blocked = (pFoot->IsCellOccupied(pCell, avgDir, currentLevel, nullptr, true) != Move::OK)
					|| (pCell->Flags & CellFlags::Tube)
					|| (MapClass::Instance->GetThreatPosed(midCell, pOwner) * threat > 1.0);

				--i;

				nextCell += dirOffset;
				pCell = MapClass::Instance->GetCellAt(nextCell);

				const int level = pCell->Level;
				const int upLevel = level + 4;
				currentLevel = (currentLevel == upLevel && pCell->ContainsBridge()) ? upLevel : level;
			}
			while (!blocked);

			midCell += CellSpread::AdjacentCell[firstDir & 7];
			remainingOffset -= 2;
		}
		while (--offset > 0);
	}

	if (steps > 0)
	{
		int i = 0;
		auto cell = *pCurrent;

		do
		{
			cell = AStarClass::NextPathCell(cell, pDirs[i]);
		}
		while (++i < steps);

		*pCurrent = cell;
	}

	return steps;
}

#pragma endregion

#pragma region OptimizeFinalPath

void AStarClass::OptimizeFinalPath(
	AStarClass::PathFinderData* const pPath,
	const FootClass* const pFoot
) const
{
	int* const pDirs = pPath->Directions;
	const int pathLength = pPath->PathLength - 1;

	if (pathLength > 0)
	{
		auto MaxCellAxisDeviation = [](const CellStruct cell) -> int
			{
				return MaxImpl(Math::abs(cell.X), Math::abs(cell.Y));
			};
		const int* const pLevels = pPath->Levels;
		const int* pCurDir = pDirs;

		auto currentPosition = pPath->StartCell;
		auto lastValidPosition = CellStruct::Empty;
		auto totalMovementOffset = CellStruct::Empty;
		auto segmentMovementOffset = CellStruct::Empty;
		auto maxMovementDeviation = Point2D::Empty;
		int maxSegmentDiagonal = 0;
		int processedStepCount = 0;
		int lastOptimizedIndex = 0;
		int lastValidOptimizedIndex = 0;

		do
		{
			if (processedStepCount >= 20)
				break;

			const int currentDirection = *pCurDir;

			if (currentDirection == 8)
			{
				currentPosition += CellSpread::AdjacentCell[0];
				++processedStepCount;
				++pCurDir;

				lastOptimizedIndex = processedStepCount;
				lastValidOptimizedIndex = processedStepCount;
				maxMovementDeviation = Point2D::Empty;
				maxSegmentDiagonal = 0;
				segmentMovementOffset = CellStruct::Empty;
				totalMovementOffset = CellStruct::Empty;
				lastValidPosition = CellStruct::Empty;
				continue;
			}

			if (currentDirection == -2)
			{
				++processedStepCount;
				++pCurDir;
				continue;
			}

			const auto& dirOffset = CellSpread::AdjacentCell[currentDirection & 7];
			const auto newSegmentMovementOffset = segmentMovementOffset + dirOffset;
			const auto newTotalMovementOffset = totalMovementOffset + dirOffset;
			const auto newMovementDeviation = Point2D { Math::abs(newTotalMovementOffset.X), Math::abs(newTotalMovementOffset.Y) };

			if (newMovementDeviation.X < maxMovementDeviation.X || newMovementDeviation.Y < maxMovementDeviation.Y)
			{
				if (lastValidPosition != CellStruct::Empty)
				{
					lastOptimizedIndex = lastValidOptimizedIndex;
					segmentMovementOffset = lastValidPosition - currentPosition;
					maxSegmentDiagonal = MaxCellAxisDeviation(segmentMovementOffset);
					maxMovementDeviation = Point2D::Empty;
					totalMovementOffset = CellStruct::Empty;
					lastValidPosition = currentPosition;
					lastValidOptimizedIndex = processedStepCount;
				}
				else
				{
					maxMovementDeviation = Point2D::Empty;
					totalMovementOffset = CellStruct::Empty;
					lastValidPosition = currentPosition;
					lastValidOptimizedIndex = processedStepCount;
				}
			}
			else
			{
				maxMovementDeviation = newMovementDeviation;
				totalMovementOffset = newTotalMovementOffset;

				const int newDiagonalOffset = MaxCellAxisDeviation(newSegmentMovementOffset);
				currentPosition += CellSpread::AdjacentCell[currentDirection & 7];

				if (maxSegmentDiagonal >= newDiagonalOffset)
				{
					int step = 0;
					auto cell = currentPosition;
					AStarClass::Instance->GetFinalStepCell(pDirs, processedStepCount, lastValidOptimizedIndex, &step, &cell);

					const auto vecCell = currentPosition - cell;
					const int plotLength = processedStepCount - step + 1;
					AStarClass::PlotStraightPath(&pDirs[step], plotLength, &cell, &vecCell, pFoot, pLevels[step], false);
				}
				else
				{
					maxSegmentDiagonal = newDiagonalOffset;
				}

				++processedStepCount;
				++pCurDir;
				segmentMovementOffset = newSegmentMovementOffset;
			}
		}
		while (processedStepCount < pathLength);

		if (lastValidPosition != CellStruct::Empty)
		{
			const int finalDiagonalOffset = MaxCellAxisDeviation(currentPosition - lastValidPosition);
			const int endStep = processedStepCount - 1;

			if (endStep - lastValidOptimizedIndex > finalDiagonalOffset)
			{
				int step = 0;
				auto cell = currentPosition;
				AStarClass::Instance->GetFinalStepCell(pDirs, endStep, lastValidOptimizedIndex, &step, &cell);

				const auto vecCell = currentPosition - cell;
				const int plotLength = endStep - step + 1;
				AStarClass::PlotStraightPath(&pDirs[step], plotLength, &cell, &vecCell, pFoot, pLevels[step], true);
			}
		}
	}

	int dir = *pDirs;
	int validStepCount = 0;

	if (dir != -1)
	{
		const int* pSourceDir = pDirs;
		int* pDestDirs = pDirs;
		int steps = 0;

		do
		{
			if (steps >= pathLength)
				break;

			if (dir != -2)
			{
				++validStepCount;
				*pDestDirs++ = *pSourceDir;
			}

			dir = *(++pSourceDir);
			++steps;
		}
		while (dir != -1);
	}

	int steps = validStepCount;
	const int totalLength = pPath->PathLength + 1;

	if (validStepCount < totalLength)
	{
		int* pDestDirs = &pDirs[validStepCount];

		do
		{
			*pDestDirs++ = -1;
			++steps;
		}
		while (steps < totalLength);
	}

	pPath->PathLength = validStepCount + 1;
}

#pragma endregion

#pragma region GetFinalStepCell

void AStarClass::GetFinalStepCell(
	const int* const pDirs,
	const int segmentEndIdx,
	const int segmentStartIdx,
	int* const pOutIdx,
	CellStruct* const pAdjacent
) const
{
	auto finalCell = *pAdjacent;
	int currentIndex = segmentStartIdx;

	if (segmentEndIdx >= segmentStartIdx)
	{
		auto accumulated = CellStruct::Empty;
		bool foundBetterPath = false;
		int maxDeviation = 0;

		for (int searchIndex = segmentEndIdx; searchIndex >= segmentStartIdx; --searchIndex)
		{
			const int curDir = pDirs[searchIndex];

			if (curDir != -2)
			{
				const int oppDir = (curDir - 4) & 7;
				const auto& dirOffset = CellSpread::AdjacentCell[oppDir];

				const auto newAccumulated = accumulated + dirOffset;
				const auto newCell = finalCell + dirOffset;
				const int currentDeviation = MaxImpl(Math::abs(newAccumulated.X), Math::abs(newAccumulated.Y));

				if (currentDeviation <= maxDeviation)
				{
					foundBetterPath = true;
				}
				else if (!foundBetterPath)
				{
					maxDeviation = currentDeviation;
				}
				else
				{
					*pOutIdx = searchIndex + 1;
					*pAdjacent = newCell + CellSpread::AdjacentCell[(oppDir - 4) & 7];
					return;
				}

				accumulated = newAccumulated;
				currentIndex = segmentStartIdx;
				finalCell = newCell;
			}
		}
	}

	*pOutIdx = currentIndex;
	*pAdjacent = finalCell;
}

#pragma endregion

#pragma region PlotStraightPath

bool AStarClass::PlotStraightPath(
	int* const pDirs,
	const int maxLength,
	const CellStruct* const pCurrent,
	const CellStruct* const pVector,
	const FootClass* const pFoot,
	const int curLevel,
	const bool allowThreats
) const
{
	const int vecX = pVector->X;
	const int vecY = pVector->Y;
	const int sumXY = vecY + vecX;
	int primaryDir = (vecX >= 0) ? (vecY >= 0 ? 3 : 1) : (vecY >= 0 ? 5 : 7);
	int secondaryDir = (vecX - vecY <= 0) ? (sumXY <= 0 ? 6 : 4) : (sumXY <= 0 ? 0 : 2);

	const int absX = Math::abs(vecX);
	const int absY = Math::abs(vecY);
	int minSteps = MinImpl(absX, absY);
	int diagSteps = MaxImpl(absX, absY) - minSteps;

	const double threat = pFoot->GetThreatAvoidanceCoefficient();
	const auto pOwner = pFoot->Owner;

	int phase = 0;
	bool blocked = false;

	while (true)
	{
		int firstSteps = minSteps;
		int secondSteps = diagSteps;
		int threatCount = 0;
		auto currentPosition = *pCurrent;

		if (phase > 0)
		{
			std::swap(primaryDir, secondaryDir);
			std::swap(minSteps, diagSteps);
		}

		if (minSteps)
		{
			int currentLevel = curLevel;

			if (minSteps > 0)
			{
				do
				{
					currentPosition += CellSpread::AdjacentCell[primaryDir & 7];
					const auto pCell = MapClass::Instance->GetCellAt(currentPosition);

					if (threat > 0.00001 && MapClass::Instance->GetThreatPosed(currentPosition, pOwner) * threat >= 0.01)
						++threatCount;

					blocked = (pFoot->IsCellOccupied(pCell, primaryDir, currentLevel, nullptr, true) != Move::OK)
						|| (pCell->Flags & CellFlags::Tube)
						|| (threatCount > 3)
						|| (!allowThreats && threatCount > 0);

					--firstSteps;

					const int level = pCell->Level;
					const int upLevel = level + 4;
					currentLevel = (currentLevel == upLevel && pCell->ContainsBridge()) ? upLevel : level;
				}
				while (firstSteps > 0 && !blocked);
			}

			if (diagSteps > 0 && !blocked)
			{
				do
				{
					currentPosition += CellSpread::AdjacentCell[secondaryDir & 7];
					const auto pCell = MapClass::Instance->GetCellAt(currentPosition);

					if (threat > 0.00001 && MapClass::Instance->GetThreatPosed(currentPosition, pOwner) * threat >= 0.01)
						++threatCount;

					blocked = (pFoot->IsCellOccupied(pCell, secondaryDir, currentLevel, nullptr, true) != Move::OK)
						|| (pCell->Flags & CellFlags::Tube)
						|| (threatCount > 3)
						|| (!allowThreats && threatCount > 0);

					--secondSteps;

					const int level = pCell->Level;
					const int upLevel = level + 4;
					currentLevel = (currentLevel == upLevel && pCell->ContainsBridge()) ? upLevel : level;
				}
				while (secondSteps > 0 && !blocked);
			}

			if (!blocked)
				break;
		}

		if (++phase >= 2)
			return false;
	}

	if (minSteps > 0)
		std::fill_n(pDirs, minSteps, primaryDir);

	if (diagSteps > 0)
		std::fill_n(pDirs + minSteps, diagSteps, secondaryDir);

	const int totalSteps = minSteps + diagSteps;
	const int remainingSteps = maxLength - totalSteps;

	if (remainingSteps > 0)
		std::fill_n(pDirs + totalSteps, remainingSteps, -2);

	return true;
}

DEFINE_FUNCTION_JUMP(CALL, 0x42CC02, AStarClass::FindRegularPath);
DEFINE_FUNCTION_JUMP(CALL, 0x42CB58, AStarClass::FindHierarchicalPath);
DEFINE_FUNCTION_JUMP(CALL, 0x42CCB3, AStarClass::FindHierarchicalPath);
DEFINE_FUNCTION_JUMP(CALL, 0x42D222, AStarClass::FindHierarchicalPath);
DEFINE_FUNCTION_JUMP(CALL, 0x429F8A, AStarClass::CalculateMoveCost);
DEFINE_FUNCTION_JUMP(CALL, 0x42A415, AStarClass::ProcessFinalPath);
DEFINE_FUNCTION_JUMP(CALL, 0x42A41E, AStarClass::OptimizeFinalPath);
DEFINE_HOOK(0x42A608, AStarClass_CleanUp_ResetSearchID, 0x5)
{
	for (int i = 0; i < 3; ++i)
	{
		auto& vec = AStarClass::IsStraightFlag[i];
		std::fill(vec.begin(), vec.end(), 0);
	}

	return 0;
}
