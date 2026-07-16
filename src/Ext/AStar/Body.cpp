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

#include <CellSpread.h>

// -----------------------------------------------------------------------
// Stamp a flat cell index as visited and record its movement cost.
// useAlt selects the bridge (alt) layer vs ground layer arrays.
// Consolidates the 6+ VisitCounts/AltVisitCounts stamp pairs in
// Find_Path_Regular and Init.
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
			VisitCounts[flatIdx] = SearchID;
			Distances[flatIdx] = cost;
		}
		else
		{
			AltVisitCounts[flatIdx] = SearchID;
			AltDistances[flatIdx] = cost;
		}
	}

	bool IsVisited(int flatIdx, bool useAlt) const
	{
		return useAlt
			? (VisitCounts[flatIdx] == SearchID)
			: (AltVisitCounts[flatIdx] == SearchID);
	}

	float GetDist(int flatIdx, bool useAlt) const
	{
		return useAlt ? Distances[flatIdx] : AltDistances[flatIdx];
	}
};

static COMPILETIMEEVAL constant_ptr<float, 0x7E3794> _pathfind_adjusment {};

// Cost multiplier per pass/layer index (a5).
// flt_81870C[0..6] = {1.0, 1000.0, 1.0, 1.0, 60.0, 20.0, 8.0}
// VERIFY: indices 3..6 — only 0/1/2 used by AStarPostProcessType; rest may be
//         used by other callers. Array size confirmed as 7 entries from segment dump.
static COMPILETIMEEVAL float PassCostMultiplier[7] =
{
	1.0f,     // ASTAR_PASS_0
	1000.0f,  // ASTAR_PASS_1
	1.0f,     // ASTAR_PASS_2
	1.0f,     // VERIFY: usage unknown
	60.0f,    // VERIFY: usage unknown
	20.0f,    // VERIFY: usage unknown
	8.0f,     // VERIFY: usage unknown
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

static COMPILETIMEEVAL int CellAdjacencyDirectionLUT[9] =
{
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

COMPILETIMEEVAL double dbl_7E37C0 = std::bit_cast<double>(0x3FF024DD2F1A9FBEULL);
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
	if (dir == 8)
	{
		const int tubeIdx = static_cast<int>(
			MapClass::Instance->GetCellAt(currentPos)->TubeIndex);
		if (tubeIdx == -1)
			return { 0, 0 };
		// BUGFIX: vanilla tube_42D490 incorrectly used Items[facing] here.
		return TubeClass::Array->Items[tubeIdx]->ExitCell;
	}
	return currentPos + CellSpread::AdjacentCell[dir & 7];
}

// -----------------------------------------------------------------------
// Walk the HierarchyBuffer back-pointer chain.
// BufferDelta == -1 is the root sentinel.
// bufferBase must be reinterpret_cast<char*>(HierarchyBuffer).
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

// -----------------------------------------------------------------------
// AStarPathFinderClass ctor
// -----------------------------------------------------------------------
AStarPathFinderClass::AStarPathFinderClass()
	: unknown_byte_0 { 0 }
	, FindBridgeDir { false }
	, unknown_byte_2 { 0 }
	, CanFindPath { true }
	, PathCostFactor { 1.0f }
	, IsAlt { true }
	, PathNodeBuffer { nullptr }
	, PathQueueBuffer { nullptr }
	, PathQueue { nullptr }
	, VisitCounts { nullptr }
	, AltVisitCounts { nullptr }
	, AltDistances { nullptr }
	, Distances { nullptr }
	, SearchID { -1 }
	, FinderSpeedType { static_cast<SpeedType>(-1) }
	, IsSearching { true }
	, FindMode { ASTAR_PASS_0 }
	, HierarchyBuffer { }
	, HierarchyQueue { nullptr }
	, PathLength { -1 }
	, CellStructBuffer { 0, 0 }
{
	for (int i = 0; i < 3; ++i)
		ZoneIndices[i] = DynamicVectorClass<CellStruct>();

	PathQueue = new SafePriorityQueueClass<AStarWorkPathStruct>(0x10000);
	HierarchyQueue = new SafePriorityQueueClass<AStarQueueNodeHierarchical>(0x2710);

	PathQueueBuffer = new AStarWorkPathStructHeap();
	PathNodeBuffer = new AStarWorkPathStructDataHeap();

	for (int i = 0; i < 3; ++i) {
		LevelVisitedMarkers[i] = nullptr;
		OpenSetMarkers[i] = nullptr;
		GCostArray[i] = nullptr;
		ZoneIndices[i].clear();
		std::memset(&PassabilityData[i], 0, sizeof(AStarClass_PassabilityData));
		PassabilityCounts[i] = 0;
	}

	HierarchyBuffer = static_cast<AStarQueueNodeHierarchical*>(operator new(0x2710 * sizeof(AStarQueueNodeHierarchical)));
}

// -----------------------------------------------------------------------
// AStarPathFinderClass dtor
// DIFF: delete () replaces 14 identical delete+nullptr pairs.
// -----------------------------------------------------------------------
AStarPathFinderClass::~AStarPathFinderClass()
{
	// PathQueue: delete internal Nodes array then the object itself
	if (PathQueue) {
		delete PathQueue;
	}

	if (HierarchyQueue) {
		delete HierarchyQueue;
	}

	delete (PathQueueBuffer);
	delete (PathNodeBuffer);
	delete (VisitCounts);
	delete (AltVisitCounts);

	// VERIFY: IDA deletes Distances before AltDistances — order reproduced.
	delete (Distances);
	delete (AltDistances);

	for (int i = 0; i < 3; ++i)
	{
		delete (LevelVisitedMarkers[i]);
		delete (OpenSetMarkers[i]);
		delete (GCostArray[i]);
	}

	// No null written after in vanilla — reproduced.
	operator delete(HierarchyBuffer);
	HierarchyBuffer = nullptr;

	// DIFF: vanilla descends; order is independent here.
	for (int i = 2; i >= 0; --i)
		ZoneIndices[i].~DynamicVectorClass<CellStruct>();
}

// -----------------------------------------------------------------------
// __Find_Path_Hierarchical
// DIFF: HierarchyParent() / IsHierarchyRoot() replace the two inline
//       back-pointer chain walks (phase-1 mark + phase-2 store).
// -----------------------------------------------------------------------
bool AStarPathFinderClass::__Find_Path_Hierarchical(
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

	double     threatAvoidance = 0.0;
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

	char* const hierarchyBase = reinterpret_cast<char*>(this->HierarchyBuffer);

	for (int level = 2; level >= 0; --level)
	{
		this->HierarchyQueue->Clear();

		const unsigned short startZone =
			globalPassabilityArray[startZoneRaw].data[level];
		const unsigned short destZone =
			globalPassabilityArray[destZoneRaw].data[level];

		const bool isTopLevel = (level == 2);

		int* parentLevelVisited = isTopLevel ? nullptr : this->LevelVisitedMarkers[level + 1];
		int* visitedArray = this->LevelVisitedMarkers[level];
		int* openSetArray = this->OpenSetMarkers[level];
		float* costArray = this->GCostArray[level];

		visitedArray[startZone] = this->SearchID;
		visitedArray[destZone] = this->SearchID;

		if (startZone == destZone)
		{
			if (level == 0)
			{
				this->HierarchyBuffer->Number = 0;
				this->HierarchyBuffer->Index = static_cast<DWORD>(startZone);
			}
			this->PassabilityData[level].Indices[0] = static_cast<unsigned short>(startZone);
			this->PassabilityCounts[level] = 1;
			continue;
		}

		AStarQueueNodeHierarchical* firstElement = this->HierarchyBuffer;
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

		const bool noBlockedPairs = (this->ZoneIndices[level].Count == 0);
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
				const int   neighborNode = static_cast<int>(connection.NeighborSubzoneIndex);
				const char  connectionFlag = static_cast<char>(connection.ConnectionPenaltyFlag);

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

				if (!noBlockedPairs)
				{
					unsigned short lo = static_cast<unsigned short>(neighborNode);
					unsigned short hi = static_cast<unsigned short>(currentNode);
					if (lo < hi)
						std::swap(lo, hi);

					const int pairKey = static_cast<int>(lo) | (static_cast<int>(hi) << 16);

					auto& blockedVector = this->ZoneIndices[level];
					bool  isBlocked = false;
					for (int searchIdx = blockedVector.Count - 1; searchIdx >= 0; --searchIdx)
					{
						if (*reinterpret_cast<int*>(&blockedVector.Items[searchIdx]) == pairKey)
						{
							isBlocked = true;
							break;
						}
					}
					if (isBlocked)
						continue;
				}

				// Allocate new node from buffer
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
			if (!currentElement) {
				Debug::Log("[A* Hierarchical] No path at level %d: zones %u->%u\n",
					level, startZone, destZone);
				return false;
			}
		}

		if (!currentElement)
			return false;

		// Phase 1: walk back and mark visited.
		// DIFF: HierarchyParent() / IsHierarchyRoot() replace manual ptr arithmetic.
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

		short* resultPtr = reinterpret_cast<short*>(
			&this->PassabilityData[level].Indices[pathIdx]);

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

DEFINE_FUNCTION_JUMP(LJMP, 0x42C290, FakeAStarPathFinderClass::__Find_Path_Hierarchical)

// -----------------------------------------------------------------------
// Calc_Float
// No structural changes — helpers already used via AStarHelpers.h.
// -----------------------------------------------------------------------
double AStarPathFinderClass::Calc_Float(
	CellClass** arg0,
	CellClass** a3,
	int         a4,
	int         a5,
	FootClass* a6) const
{
	CellClass* targetCell = *a3;
	CellClass* sourceCell = *arg0;

	float cost = PassCostMultiplier[a5];

	if (a5 == 2)
	{
		ObjectClass* occupier = GetCellObj(targetCell, a4);
		int          chainDepth = 0;
		bool         chainBroken = false;

		if (!FindMode)
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
				bool usesAlt = nextCellPtr->ContainsBridge()
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

		if (chainBroken || FindMode)
			cost = 4.0f;
		if (FindMode == 2)
			cost = 1000.0f;
	}

	if (targetCell->UINTFlags & 0x40000)
		cost *= TunnelCostMultiplier;

	if (!a4 || !FindBridgeDir)
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
// Calc_sqrt — unchanged structurally, no repetition to collapse here.
// -----------------------------------------------------------------------
AStarWorkPathStruct* AStarPathFinderClass::Calc_sqrt(
	AStarWorkPathStruct* parentNode,
	CellClass** a3,
	CellStruct* goalCell,
	float                a5)
{
	AStarWorkPathStruct* newQueueNode = &PathQueueBuffer->Nodes[PathQueueBuffer->ActiveCount++];
	AStarWorkPathStructNode* newPathNode = &PathNodeBuffer->Nodes[PathNodeBuffer->ActiveCount++];

	newPathNode->Cells = a3;

	if (parentNode)
	{
		newPathNode->Prev = parentNode->Data;

		CellClass* targetCell = *a3;
		CellClass* parentFirst = *parentNode->Data->Cells;
		int        targetLevel = targetCell->Level;

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
		newPathNode->CellLevel = StartLevel;
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
// DIFF: VisitStamper not used here (bulk-zero, not single-stamp).
//       Loop structure unchanged.
// -----------------------------------------------------------------------
void AStarPathFinderClass::Init()
{
	PathNodeBuffer->ActiveCount = 0;
	PathQueueBuffer->ActiveCount = 0;

	if (PathQueue)
		PathQueue->Clear();
	if (HierarchyQueue)
		HierarchyQueue->Clear();

	++SearchID;
	if (SearchID != 0)
		return;

	{
		const int regionArea = RegionSize() * RegionSize();
		for (int k = regionArea - 1; k >= 0; --k)
		{
			VisitCounts[k + 1] = 0;
			AltVisitCounts[k + 1] = 0;
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		//std::array<int*, 13> MovementZones;
				// CONFIRMED[0x42C1C0]: MovementZones[4+i] stores the subzone count as a raw
		// integer in the pointer slot — not a real pointer, never dereferenced.
		// Slots [0..3] are real uint16_t* zone-ID arrays; [4..6] are int-as-pointer counts.
		const int count = static_cast<int>(reinterpret_cast<std::uintptr_t>(MapClass::Instance->MovementZones[4 + i]));
		
		for (int k = count - 1; k >= 0; --k) {
			LevelVisitedMarkers[i][k] = 0;
			OpenSetMarkers[i][k] = 0;
			reinterpret_cast<int*>(GCostArray[i])[k] = 0;
		}
	}

	++SearchID;
}

// -----------------------------------------------------------------------
// IsVisited — trivial, unchanged.
// -----------------------------------------------------------------------
bool AStarPathFinderClass::IsVisited(int index, bool useAlt) const
{
	return useAlt
		? (VisitCounts[index] == SearchID)
		: (AltVisitCounts[index] == SearchID);
}

// -----------------------------------------------------------------------
// CellStruct_helper_distance — unchanged.
// -----------------------------------------------------------------------
int __fastcall AStarPathFinderClass::CellStruct_helper_distance(CellStruct* a1, CellStruct* a2)
{
	static constexpr int InvalidAdjacency = 8;
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
PathType* AStarPathFinderClass::Get_Path(AStarWorkPathStruct* work_path, FacingType* moves)
{
	outpath->Cost = static_cast<int>(work_path->PathCost);
	outpath->Length = work_path->PathLength;
	outpath->field_10 = 0;
	outpath->LastOverlap = { 0, 0 };
	outpath->Command = moves;
	outpath->Overlap = reinterpret_cast<CellStruct*>(MainOverlap());

	AStarWorkPathStructNode* curNode = work_path->Data;
	AStarWorkPathStructNode* parentNode = work_path->Data->Prev;

	const int loopCount = work_path->PathLength - 2;

	if (loopCount >= 0)
	{
		const std::ptrdiff_t overlapDelta =
			reinterpret_cast<std::uint8_t*>(MainOverlap()) -
			reinterpret_cast<std::uint8_t*>(moves);

		FacingType* writePtr = moves + loopCount;
		int         remaining = loopCount + 1;

		do
		{
			if (parentNode)
			{
				*reinterpret_cast<int*>(
					reinterpret_cast<std::uint8_t*>(writePtr) + overlapDelta)
					= parentNode->CellLevel;

				CellStruct fromCell = (*curNode->Cells)->MapCoords;
				CellStruct toCell = (*parentNode->Cells)->MapCoords;
				*writePtr = static_cast<FacingType>(
					CellStruct_helper_distance(&fromCell, &toCell));
			}

			curNode = curNode->Prev;
			parentNode = parentNode ? parentNode->Prev : nullptr;
			--writePtr;
			--remaining;
		}
		while (remaining);
	}

	moves[work_path->PathLength - 1] = static_cast<FacingType>(-1);
	outpath->Start = (*curNode->Cells)->MapCoords;

	if (!outpath->Cost)
		outpath->Cost = 1;

	return outpath.operator->();
}

// -----------------------------------------------------------------------
// Reset — unchanged structurally.
// -----------------------------------------------------------------------
void AStarPathFinderClass::Reset(RectangleStruct* rect)
{
	delete (VisitCounts);
	delete (AltVisitCounts);
	delete (Distances);
	delete (AltDistances);

	RegionSize = rect->Width + rect->Height + 1;
	const int allocBytes = 4 * RegionSize() * RegionSize();

	AltVisitCounts = static_cast<int*>(operator new(static_cast<unsigned>(allocBytes)));
	VisitCounts = static_cast<int*>(operator new(static_cast<unsigned>(allocBytes)));
	Distances = static_cast<float*>(operator new(static_cast<unsigned>(allocBytes)));
	AltDistances = static_cast<float*>(operator new(static_cast<unsigned>(allocBytes)));

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
FootClass* AStarPathFinderClass::Get_Occupier(CellStruct* pos, int level) const
{
	const int worldX = (static_cast<int>(pos->X) << 8) + 128;
	const int worldY = (static_cast<int>(pos->Y) << 8) + 128;
	const int worldZ = level * Unsorted::LevelHeight;

	for (int dy = -2; dy < 3; ++dy)
	{
		for (int dx = -2; dx < 3; ++dx)
		{
			CellStruct  searchCell = { static_cast<short>(pos->X + dx), static_cast<short>(pos->Y + dy) };
			CellClass* cell = MapClass::Instance->GetCellAt(searchCell);
			const int   levelDelta = Math::abs(static_cast<int>(cell->Level) - level);
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
bool AStarPathFinderClass::Process_Paths(TechnoClass* techno)
{
	if (!CanFindPath)
		return false;

	CellStruct curPos = techno->GetMapCoords();
	CellClass* curCell = MapClass::Instance->GetCellAt(curPos);

	DirStruct        facingDir = techno->PrimaryFacing.Current();
	const unsigned int facingIdx =
		(((static_cast<unsigned int>(facingDir.Raw) >> 12) + 1) >> 1) & 7u;

	CellStruct frontPos = curPos + CellSpread::AdjacentCell[facingIdx];
	CellClass* frontCell = MapClass::Instance->GetCellAt(frontPos);

	const int  levelDelta = Math::abs(
		static_cast<int>(curCell->Level) - static_cast<int>(frontCell->Level));
	const bool useAlt = frontCell->ContainsBridge()
		&& (levelDelta > 3 || techno->OnBridge);

	ObjectClass* occupier = GetCellObj(frontCell, useAlt);

	if (!occupier)
	{
		occupier = Get_Occupier(
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
			(FindMode == 2)
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
			if (*pathStep == 8)
			{
				const int tubeIdx = static_cast<int>(
					MapClass::Instance->GetCellAt(occupierPos)->TubeIndex);
				occupierPos = (tubeIdx == -1)
					? CellStruct { 0, 0 }
				: TubeClass::Array->Items[tubeIdx]->ExitCell;
			}
			else
			{
				occupierPos += CellSpread::AdjacentCell[*pathStep];
			}

			CellClass* stepCell = MapClass::Instance->GetCellAt(occupierPos);
			ToggleBit40000(&stepCell->UINTFlags);
			++pathStep;
			++stepCount;
		}

		occupier = occupier->NextObject;
	}

	if (!foundBlocker && FindMode == 1)
	{
		FindMode = ASTAR_PASS_0;
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
// DIFF: now delegates to ResolveStep() from AStarHelpers.h, eliminating
//       the duplicate implementation.
// -----------------------------------------------------------------------
CellStruct* __fastcall AStarPathFinderClass::tube_42D490(CellStruct* a1, CellStruct* a2, int facing)
{
	*a1 = ResolveStep(*a2, facing);
	return a1;
}

// -----------------------------------------------------------------------
// Tube_Crap — ResolveStep() already replaces both local copies.
// -----------------------------------------------------------------------
int AStarPathFinderClass::Tube_Crap(
	FootClass* techno,
	int* dirArray,
	int* levelArray,
	int         pathLen,
	int         lookAhead,
	CellStruct* posPtr)
{
	const int firstDir = dirArray[0];
	const int lastDir = dirArray[pathLen];
	const int midDir = (firstDir + lastDir) >> 1;
	const int midDirValue = ((midDir + 1 == lastDir) || (midDir + 1 == firstDir))
		? midDir : 0;

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
		const int  scanCount = 2 * lookAhead;
		int        scanLevel = levelArray[pathLen + lookAhead - scanCount];
		bool       blocked = false;

		CellStruct scanPos =
		{
			static_cast<short>(a1a.X + CellSpread::AdjacentCell[midDir & 7].X),
			static_cast<short>(a1a.Y + CellSpread::AdjacentCell[midDir & 7].Y)
		};
		CellClass* scanCell = MapClass::Instance->GetCellAt(scanPos);

		for (int s = scanCount; s > 0; --s)
		{
			if (techno->IsCellOccupied(scanCell, (FacingType)midDirValue, scanLevel, nullptr, true) == Move::OK
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
			if (scanLevel - newLevel == 4)
			{
				scanLevel = newLevel + 4;
				if (scanCell->ContainsBridge())
					continue;
			}
			scanLevel = newLevel;

			if (blocked)
				break;
		}

		if (!blocked)
		{
			const int fillStart = pathLen - lookAhead;
			const int fillCount = 2 * lookAhead;
			if (fillCount > 0)
				std::fill_n(dirArray + fillStart, fillCount, midDirValue);

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
// Process_Moves — unchanged structurally (ResolveStep already used).
// -----------------------------------------------------------------------
void AStarPathFinderClass::Process_Moves(PathType* path, FootClass* techno)
{
	FacingType* moves = path->Command;
	int* overlap = reinterpret_cast<int*>(path->Overlap);
	CellStruct  start = path->Start;

	const int totalLen = path->Length - 1;
	if (totalLen <= 0)
		return;

	FacingType prevDir = FacingType::None;
	int        runLen = 0;
	int        baseIdx = 0;
	int        diagRunLen = 0;
	int        scanOffset = 0;
	bool       inDiagRun = false;
	FacingType expectedDir = FacingType::None;

	CellStruct curPos = start;
	CellStruct endCell = start;

	const auto UpdateEndCell = [&](FacingType dir)
		{
			if (dir == FacingType::Count)
			{
				const int tubeIdx = static_cast<int>(
					MapClass::Instance->GetCellAt(endCell)->TubeIndex);
				endCell = (tubeIdx == -1)
					? CellStruct { 0, 0 }
				: TubeClass::Array->Items[tubeIdx]->ExitCell;
			}
			else
			{
				endCell += CellSpread::AdjacentCell[(int)dir & 7];
			}
		};

	while (true)
	{
		const bool pastEnd = (diagRunLen + scanOffset >= totalLen)
			|| (baseIdx + runLen >= totalLen);

		if (pastEnd)
		{
			if (inDiagRun)
			{
				Tube_Crap(techno,
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
				UpdateEndCell(expectedDir);
				continue;
			}

			baseIdx += Tube_Crap(techno,
				(int*)&moves[baseIdx],
				&overlap[baseIdx],
				runLen, diagRunLen, &curPos);
			runLen = 1;
			inDiagRun = false;
			diagRunLen = 0;
			scanOffset = 0;

			FacingType nextDir = FacingType((int)moves[baseIdx] & (int)FacingType::NorthWest);
			endCell =
			{
				static_cast<short>(curPos.X + CellSpread::AdjacentCell[(int)nextDir].X),
				static_cast<short>(curPos.Y + CellSpread::AdjacentCell[(int)nextDir].Y)
			};
			prevDir = moves[baseIdx];
			UpdateEndCell(moves[baseIdx]);
			continue;
		}

		const FacingType curDir = moves[baseIdx + runLen];
		const int        delta = ((int)curDir - (int)prevDir) & 7;

		if (curDir == prevDir)
		{
			++runLen;
		}
		else if ((delta == 2 || delta == 6)
			  && prevDir != FacingType::None
			  && prevDir != FacingType::Max
			  && curDir != FacingType::Max)
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
			prevDir = ((int)curDir & 1) ? curDir : FacingType::None;
			curPos = endCell;
		}

		UpdateEndCell(curDir);
	}
}

// -----------------------------------------------------------------------
// Adj_Cell / Is_Cell_In_Vector / UpdateZoneVector / Add_Cell_To_Vector —
// unchanged structurally; no repetition to collapse.
// -----------------------------------------------------------------------
void AStarPathFinderClass::Adj_Cell(
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

bool AStarPathFinderClass::Is_Cell_In_Vector(unsigned int a, unsigned int b, int vectorNum) const
{
	if (b < a)
		std::swap(a, b);

	const CellStruct key = CellStruct::UnPack(((b & 0xFFFFu) << 16) | (a & 0xFFFFu));

	auto& vec = this->ZoneIndices[vectorNum];
	int   count = vec.Count - 1;

	if (count < 0)
		return false;

	for (int i = count; i >= 0; --i)
	{
		if (vec[i] == key)
			return true;
	}
	return false;
}

void AStarPathFinderClass::UpdateZoneVector(unsigned int zoneValue, int vectorIdx)
{
	const int count = PassabilityCounts[vectorIdx];
	if (count <= 1)
	{
		IsSearching = false;
		return;
	}

	const uint16_t* data = PassabilityData[vectorIdx].Indices;
	int             foundIdx = -1;

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
		IsSearching = false;
		return;
	}

	unsigned int zoneA, zoneB;
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

	Add_Cell_To_Vector(zoneA, zoneB, vectorIdx);

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
				Add_Cell_To_Vector(entryZone, zoneA, vectorIdx);
			}
		}
	}
}

void AStarPathFinderClass::Add_Cell_To_Vector(unsigned int a, unsigned int b, int vectorNum)
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
bool AStarPathFinderClass::Generate_Moves(
	int* moves,
	int         capacity,
	CellStruct* startCell,
	CellStruct* delta,
	FootClass* techno,
	int* levelPtr,
	bool        tolerateThreats)
{
	static constexpr double kThreatAvoidanceThreshold = 0.00001;
	static constexpr double kCellThreatThreshold = 0.01;

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
	const bool   checkThreats = (threatFactor > kThreatAvoidanceThreshold);

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
				if (techno->IsCellOccupied(cell, (FacingType)dir, curLevel, nullptr, false) != Move::OK)
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
unsigned int AStarPathFinderClass::Attempt(
	CellStruct* startPos,
	CellStruct* destPos,
	FootClass* foot,
	bool         bridge1,
	bool         bridge2,
	MovementZone mzone)
{
	IsSearching = true;
	Init();

	for (int i = 0; i < 3; ++i)
		ZoneIndices[i].clear();

	CellClass* startCell = MapClass::Instance->GetCellAt(startPos);
	CellClass* destCell = MapClass::Instance->GetCellAt(destPos);

	CellStruct subStart, subDest;
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
	const int passCount0 = PassabilityCounts[0];
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
			ZoneType zoneA = MapClass::Instance->GetMovementZoneType(destPos, MovementZone::AmphibiousDestroyer, 0);
			ZoneType zoneB = MapClass::Instance->GetMovementZoneType(startPos, MovementZone::AmphibiousDestroyer, 0);
			if (zoneA == zoneB && zoneA != ZoneType::None)
				return static_cast<unsigned int>(baseDist);
		}

		if (passCount0 >= 4)
		{
			const uint16_t entry = PassabilityData[0].Indices[passCount0 - 2];
			if (TrySubzone(destCell, destPos, entry, nullptr))
				bridge2Done = true;
		}

		if (!bridge2Done)
		{
			const uint16_t entry = PassabilityData[0].Indices[passCount0 - 1];
			TrySubzone(destCell, destPos, entry, &subDest);
		}
	}

	if (bridge1)
	{
		bool bridge1Done = false;

		if (passCount0 >= 4)
		{
			const uint16_t entry = PassabilityData[0].Indices[1];
			if (TrySubzone(startCell, startPos, entry, nullptr))
				bridge1Done = true;
		}

		if (!bridge1Done)
		{
			const uint16_t entry = PassabilityData[0].Indices[0];
			TrySubzone(startCell, startPos, entry, &subStart);
		}
	}

	return static_cast<unsigned int>(std::max(penalty, baseDist));
}

// -----------------------------------------------------------------------
// Fill_DVector — unchanged structurally.
// -----------------------------------------------------------------------
void AStarPathFinderClass::Fill_DVector(FootClass* techno)
{
	const int startZoneIdx = MapClass::Instance->MapClass_zone_56D3F0(&CellStructBuffer);
	GlobalPassabilityData& startZoneStruct =
		MapClass::Instance->LevelAndPassabilityStruct2pointer_70[startZoneIdx];

	for (int passIdx = 0; passIdx < 3; ++passIdx)
	{
		const uint16_t startZoneValue = startZoneStruct.data[passIdx];

		DynamicVectorClass<uint16_t> tempVec {};
		CellClass* cell = MapClass::Instance->GetCellAt(CellStructBuffer);

		if (MapClass::Instance->Subzone_5840C0(cell, passIdx, &tempVec, techno))
		{
			const int      cellZoneIdx = MapClass::Instance->MapClass_zone_56D3F0(&CellStructBuffer);
			const uint16_t cellZoneValue =
				MapClass::Instance->LevelAndPassabilityStruct2pointer_70[cellZoneIdx].data[passIdx];
			UpdateZoneVector(static_cast<unsigned int>(cellZoneValue), passIdx);
		}
		else
		{
			for (int i = tempVec.Count - 1; i >= 0; --i)
			{
				Add_Cell_To_Vector(
					static_cast<unsigned int>(tempVec[i]),
					static_cast<unsigned int>(startZoneValue),
					passIdx);
			}
		}
	}
}

// -----------------------------------------------------------------------
// AllocZoneArrays
// DIFF: delete () replaces the 6 manual delete+null pairs.
// -----------------------------------------------------------------------
void AStarPathFinderClass::AllocZoneArrays()
{
	for (int i = 0; i < 3; ++i)
	{
		// CONFIRMED[0x42C1C0]: pointer slot stores the count as a raw integer, not a pointer.
		const int count = static_cast<int>(
			reinterpret_cast<std::uintptr_t>(MapClass::Instance->MovementZones[4 + i]));

		delete (LevelVisitedMarkers[i]);
		LevelVisitedMarkers[i] = static_cast<int*>(operator new(static_cast<unsigned>(count * 4)));
		std::fill_n(LevelVisitedMarkers[i], count, 0);

		delete (OpenSetMarkers[i]);
		OpenSetMarkers[i] = static_cast<int*>(operator new(static_cast<unsigned>(count * 4)));
		std::fill_n(OpenSetMarkers[i], count, 0);

		delete (GCostArray[i]);
		GCostArray[i] = reinterpret_cast<float*>(operator new(static_cast<unsigned>(count * 4)));
		std::fill_n(reinterpret_cast<int*>(GCostArray[i]), count, 0);
	}
}

// -----------------------------------------------------------------------
// Find_Path
// DIFF: VisitStamper used for the StartLevel/EndLevel stamp pairs.
// -----------------------------------------------------------------------
PathType* AStarPathFinderClass::Find_Path(
	CellStruct* start,
	CellStruct* dest,
	FootClass* techno,
	int* moves,
	int                  maxCount,
	MovementZone         mzoneOverride,
	AStarPostProcessType findModeOverride)
{
	IsSearching = true;
	Init();

	for (int i = 0; i < 3; ++i)
		ZoneIndices[i].clear();

	FindMode = findModeOverride;

	CellClass* startCell = MapClass::Instance->GetCellAt(start);
	CellClass* destCell = MapClass::Instance->GetCellAt(dest);

	MovementZone resolvedMzone = (mzoneOverride == MovementZone::None)
		? techno->GetTechnoType()->MovementZone
		: mzoneOverride;

	const int startZone = MapClass::Instance->GetMapZone(start, resolvedMzone, techno->OnBridge);

	MovementZone destMzone = (mzoneOverride == MovementZone::None)
		? techno->GetTechnoType()->MovementZone
		: mzoneOverride;

	const bool destIsBridge = (destCell->UINTFlags >> 8) & 1;
	const int  destZone = MapClass::Instance->GetMapZone(dest, destMzone, destIsBridge);

	CellStruct subStart, subDest;
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
			auto& loco = techno->Locomotor;
			static constexpr GUID CLSID_IPersist = {
				0x0000010C, 0x0000, 0x0000,
				{ 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
			};
			IPersistStream* pStm;
			loco.QueryInterface(CLSID_IPersist, (void**)&pStm);
			ULARGE_INTEGER _size;
			if (pStm->GetSizeMax(&_size))
				pStm->Release();
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

	const int maxRetries = (maxCount == -1) ? 5 : 4;
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
		Fill_DVector(techno);
		Init();

		useHierarchical = (IsSearching != 0);

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
void AStarPathFinderClass::Calc_Moves(PathType* path, FootClass* techno)
{
	auto  moves = path->Command;
	int* overlap = reinterpret_cast<int*>(path->Overlap);
	CellStruct curCell = path->Start;

	const int totalSteps = path->Length - 1;

	int  stepIdx = 0;
	int  validCount = 0;

	short accDeltaX = 0;
	short accDeltaY = 0;
	short prevDeltaX = 0;
	short prevDeltaY = 0;

	int maxAbsX = 0;
	int maxAbsY = 0;
	int maxChebyshev = 0;

	CellStruct  pivotCell = { 0, 0 };
	int         pivotStep = 0;
	int         adjStep = 0;

	FacingType* stepPtr = moves;

	if (totalSteps > 0)
	{
		while (stepIdx < totalSteps)
		{
			if (stepIdx >= 20)
				break;

			const FacingType dir = *stepPtr;

			if (dir == FacingType::Count)
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
			}
			else if (dir == (FacingType)-2)
			{
				++stepIdx;
				++stepPtr;
			}
			else
			{
				const int   dirIdx = (int)dir & 7;
				const short adjX = CellSpread::AdjacentCell[dirIdx].X;
				const short adjY = CellSpread::AdjacentCell[dirIdx].Y;

				const short newAccX = static_cast<short>(accDeltaX + adjX);
				const short newAccY = static_cast<short>(accDeltaY + adjY);
				const short newPrevX = static_cast<short>(prevDeltaX + adjX);
				const short newPrevY = static_cast<short>(prevDeltaY + adjY);

				const CellStruct newCell =
				{
					static_cast<short>(curCell.X + adjX),
					static_cast<short>(curCell.Y + adjY)
				};

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
				}
				else
				{
					maxAbsY = absNewPrevY;
					prevDeltaX = newPrevX;
					prevDeltaY = newPrevY;
					maxAbsX = absNewPrevX;

					const int absAccX = Math::abs(static_cast<int>(newAccX));
					const int absAccY = Math::abs(static_cast<int>(newAccY));
					const int newCheb = std::max(absAccX, absAccY);

					const int subDirIdx = static_cast<unsigned int>(dir) & 7u;
					const CellStruct stepCell = curCell + CellSpread::AdjacentCell[subDirIdx];
					curCell = stepCell;

					if (maxChebyshev >= newCheb)
					{
						CellStruct adjOutCell = stepCell;
						int        adjOutStep = 0;

						Adj_Cell((int*)moves, stepIdx, adjStep, &adjOutStep, &adjOutCell);

						const CellStruct delta = stepCell - adjOutCell;

						Generate_Moves(
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
				}

				validCount = 0;
			}
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

				Adj_Cell((int*)moves, stepIdx - 1, pivotStep, &adjOutStep, &adjOutCell);

				const CellStruct delta =
				{
					static_cast<short>(curCell.X - adjOutCell.X),
					static_cast<short>(curCell.Y - adjOutCell.Y)
				};

				Generate_Moves(
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

	{
		FacingType* readPtr = moves;
		FacingType* writePtr = moves;
		int  readStep = 0;
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
			moves[i] = FacingType::None;
	}

	path->Length = validCount + 1;
}

// -----------------------------------------------------------------------
// Find_Path_Regular
// DIFF: VisitStamper consolidates the 6 VisitCounts/AltVisitCounts pairs.
//       WorkPath pop/push-swap idiom extracted to lambda QueueSwap.
// -----------------------------------------------------------------------
PathType* AStarPathFinderClass::Find_Path_Regular(
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
	EndLevel = endLevel;

	int startLevel = static_cast<int>(startCell->Level);
	if (!technoIsAircraft && techno->OnBridge)
		startLevel += 4;
	StartLevel = startLevel;

	if (techno->GetTechnoType()->IsTrain && startCell->ContainsBridge())
	{
		const int coordLevel = techno->GetCoords().Z / Unsorted::LevelHeight;
		if (Math::abs(coordLevel - StartLevel) > 2)
			StartLevel += 4;
	}

	FinderSpeedType = static_cast<SpeedType>(techno->GetTechnoType()->Speed);
	PathLength = 0;
	CellStructBuffer = *start;

	int* zoneArray = LevelVisitedMarkers[0];

	AStarWorkPathStruct* workPath = Calc_sqrt(nullptr, destCellPtr, dest, 0.0f);

	if (start->X == dest->X && start->Y == dest->Y && StartLevel == EndLevel)
		return nullptr;

	if (FindMode)
		Process_Paths(reinterpret_cast<TechnoClass*>(techno));

	// VisitStamper: consolidates all VisitCounts/AltVisitCounts stamp pairs.
	VisitStamper stamper { VisitCounts, AltVisitCounts, Distances, AltDistances, SearchID };

	const int startFlatIdx =
		static_cast<int>(startCell->MapCoords.Y) * RegionSize()
		+ static_cast<int>(startCell->MapCoords.X);

	stamper.Stamp(startFlatIdx, 0.0f, StartLevel <= static_cast<int>(startCell->Level));

	bool isTrain = techno->GetTechnoType()->IsTrain;

	if (isTrain)
	{
		int facingIdx = 0;
		for (auto neighborPtr = std::begin(dword_7E3774);
			 neighborPtr <= std::end(dword_7E3774);
			 ++neighborPtr, ++facingIdx)
		{
			DirStruct        facingDir = techno->PrimaryFacing.Current();
			const unsigned int technoFacing =
				(((static_cast<unsigned int>(facingDir.Raw) >> 12) + 1) >> 1) & 7u;

			const int diff = Math::abs(static_cast<int>(technoFacing) - facingIdx);
			if (diff > 2 && diff < 6 && neighborPtr != std::end(dword_7E3774))
			{
				CellClass* neighbor = startCellPtr[*neighborPtr];
				if (neighbor)
				{
					const int flatIdx =
						static_cast<int>(neighbor->MapCoords.Y) * RegionSize()
						+ static_cast<int>(neighbor->MapCoords.X);
					const int neighborLevel = static_cast<int>(neighbor->Level) + 1;
					stamper.Stamp(flatIdx, 0.0f, StartLevel <= neighborLevel);
				}
			}
		}
	}

	bool isPassive = false;
	if (auto pUnit = cast_to<UnitClass* , false>(techno)) {
		//TODO : killdriver
		if (pUnit->Type->Passive)
			isPassive = true;
	}

	if (maxCount < 0)
		maxCount = 0xFFF7;

	if (!workPath)
	{
		if (FindMode)
			Process_Paths(techno);
		return nullptr;
	}

	// QueueSwap: replaces the 3 identical pop/peek/push-back patterns.
	// Selects the lower-cost head between bestNode and the queue top,
	// pushing the other one back.
	const auto QueueAdvance = [&](AStarWorkPathStruct*& work,
								  AStarWorkPathStruct* best) -> bool
		{
			if (!best)
			{
				AStarWorkPathStruct* top = PathQueue->Top();
				if (!top)
				{
					work = nullptr;
					return false;
				}
				PathQueue->Pop();
				work = top;
				return true;
			}

			if (!PathQueue->Count || PathQueue->Top()->PathCost >= best->PathCost)
			{
				work = best;
				return true;
			}

			AStarWorkPathStruct* top = PathQueue->Top();
			PathQueue->Pop();
			PathQueue->Push(best);
			work = top;
			return true;
		};

	int  iterCount = 0;
	bool destinationReached = false;

	while (workPath && !destinationReached)
	{
		if (iterCount >= maxCount)
			break;

		CellClass** curCells = workPath->Data->Cells;
		CellClass* curFirstCell = *curCells;

		if (curCells == destCellPtr && workPath->Data->CellLevel == EndLevel)
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
			CellClass** neighborCellPtr;

			if (facing == 8)
			{
				const int tubeIdx = static_cast<int>(curFirstCell->TubeIndex);
				if (tubeIdx == -1)
				{
					*neighborCellPtr = nullptr;
				}
				else
				{
					CellStruct endCell = TubeClass::Array->Items[tubeIdx]->ExitCell;
					neighborCellPtr = CellArrayPtr(endCell.X, endCell.Y);
				}
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

			const bool neighborIsBridge = neighborCell->ContainsBridge();
			bool useAlt;
			if (neighborIsBridge)
			{
				const int levelDiff = Math::abs(StartLevel - static_cast<int>(neighborCell->Level));
				useAlt = (levelDiff <= 1);
			}
			else
			{
				useAlt = true;
			}

			const int zone_ = MapClass::Instance->MapClass_zone_56D3F0(&neighborCell->MapCoords);
			const int zoneVal = static_cast<int>(
				MapClass::Instance->LevelAndPassabilityStruct2pointer_70[zone_].data[0]);

			if (zoneArray[zoneVal] != SearchID)
			{
				if (useAlt && !neighborCell->BlockedNeighbours && useHierarchical)
					continue;
			}

			const float prevCost = workPath->MovementCost;

			// DIFF: stamper.IsVisited() / stamper.GetDist() replace the 2 symmetric if-blocks.
			if (stamper.IsVisited(neighborFlatIdx, useAlt)
			 && stamper.GetDist(neighborFlatIdx, useAlt) < static_cast<double>(prevCost) + dbl_7E37C0)
				continue;

			int canEnter = (int)techno->IsCellOccupied(
				neighborCell,
				static_cast<FacingType>(facing),
				StartLevel,
				*curCells,
				IsAlt);

			if (isTrain && canEnter < 7)
				canEnter = 0;

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
				const float calcFloat = Calc_Float(
					curCells, neighborCellPtr,
					useAlt ? 0 : 1,
					canEnter,
					techno);
				stepCost = calcFloat * PathCostFactor + PassCostMultiplier[facing];
			}

			if (canEnter >= 7)
			{
				if (neighborCellPtr == destCellPtr && !isPassive)
				{
					const int levelDiff = Math::abs(StartLevel - EndLevel);
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

			{
				AStarWorkPathStruct* newNode = Calc_sqrt(
					workPath, neighborCellPtr, dest, stepCost);

				if (!bestNode || newNode->PathCost < bestNode->PathCost)
				{
					if (bestNode)
						PathQueue->Push(bestNode);
					bestNode = newNode;
				}
				else
				{
					PathQueue->Push(newNode);
				}

				// DIFF: stamper.Stamp() replaces the 2 symmetric if/else stamp blocks.
				stamper.Stamp(neighborFlatIdx, newNode->MovementCost, useAlt);

				if (zoneVal == static_cast<int>(
					PassabilityData[0].Indices[PathLength + 1]))
				{
					++PathLength;
					CellStructBuffer = neighborCell->MapCoords;
				}
			}
		}

		if (!destinationReached)
			QueueAdvance(workPath, bestNode);

		if (workPath)
			StartLevel = workPath->Data->CellLevel;

		++iterCount;
	}

	if (iterCount == 10000
	 || !workPath
	 || iterCount == maxCount
	 || workPath->PathLength < 2)
	{
		if (FindMode)
			Process_Paths(techno);
		return nullptr;
	}

	PathType* result = Get_Path(workPath, reinterpret_cast<FacingType*>(moves));
	Process_Moves(result, techno);
	Calc_Moves(result, techno);

	if (FindMode)
		Process_Paths(techno);

	return result;
}