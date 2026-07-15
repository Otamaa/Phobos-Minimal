#include "Body.h"

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <FootClass.h>
#include <TechnoClass.h>
#include <Memory.h>
#include <MapClass.h>
#include <HouseClass.h>
#include <TubeClass.h>

#include <Misc/PhobosGlobal.h>

#include <CellSpread.h>

static COMPILETIMEEVAL constant_ptr<float, 0x7E3794> _pathfind_adjusment {};

static bool PriorityQueue_Insert_Safe(
	PriorityQueueClass_AStarHierarchical* queue,
	AStarQueueNodeHierarchical* element);

static AStarQueueNodeHierarchical* PriorityQueue_Pop_Safe(
	PriorityQueueClass_AStarHierarchical* queue);

static void PriorityQueue_HeapifyDown_Safe(
	PriorityQueueClass_AStarHierarchical* queue,
	int index);

// Safe insert into priority queue
static bool PriorityQueue_Insert_Safe(
	PriorityQueueClass_AStarHierarchical* queue,
	AStarQueueNodeHierarchical* element)
{
	// Null checks
	if (!queue || !element)
	{
		Debug::Log("[A* Queue] Insert failed: null queue or element\n");
		return false;
	}

	if (!queue->Heap)
	{
		Debug::Log("[A* Queue] Insert failed: null heap array\n");
		return false;
	}

	// Capacity check
	int newPos = queue->Count + 1;
	if (newPos >= queue->Capacity)
	{
		Debug::Log("[A* Queue] Insert failed: overflow (Count=%d, Capacity=%d)\n",
			queue->Count, queue->Capacity);
		return false;
	}

	// Bubble up
	unsigned int insertPos = static_cast<unsigned int>(newPos);
	unsigned int parentPos = insertPos >> 1;

	while (insertPos > 1)
	{
		// Bounds check for parent
		if (parentPos < 1 || parentPos > static_cast<unsigned int>(queue->Count))
			break;

		auto* parent = queue->Heap[parentPos];

		// Null check for parent
		if (!parent)
		{
			Debug::Log("[A* Queue] Insert: null parent at index %u\n", parentPos);
			break;
		}

		// Compare scores (min-heap: parent should be <= child)
		if (parent->Score <= element->Score)
			break;

		// Move parent down
		queue->Heap[insertPos] = parent;
		insertPos = parentPos;
		parentPos >>= 1;
	}

	// Place element
	queue->Heap[insertPos] = element;
	++queue->Count;

	// Update bounds pointers
	if (element > queue->MaxNodePointer)
		queue->MaxNodePointer = element;
	if (element < queue->MinNodePointer)
		queue->MinNodePointer = element;

	return true;
}

// Safe pop from priority queue
static AStarQueueNodeHierarchical* PriorityQueue_Pop_Safe(
	PriorityQueueClass_AStarHierarchical* queue)
{
	// Null checks
	if (!queue || !queue->Heap)
	{
		Debug::Log("[A* Queue] Pop failed: null queue or heap\n");
		return nullptr;
	}

	if (queue->Count <= 0)
		return nullptr;

	// Bounds validation
	if (queue->Count > queue->Capacity)
	{
		Debug::Log("[A* Queue] Pop: Count(%d) > Capacity(%d), resetting\n",
			queue->Count, queue->Capacity);
		queue->Count = 0;
		return nullptr;
	}

	// Get root element
	auto* result = queue->Heap[1];
	if (!result)
	{
		Debug::Log("[A* Queue] Pop: root is null but Count=%d\n", queue->Count);
		queue->Count = 0;
		return nullptr;
	}

	// Move last element to root
	queue->Heap[1] = queue->Heap[queue->Count];
	queue->Heap[queue->Count] = nullptr;
	--queue->Count;

	// Heapify down if elements remain
	if (queue->Count > 0)
	{
		PriorityQueue_HeapifyDown_Safe(queue, 1);
	}

	return result;
}

// Safe heapify down
static void PriorityQueue_HeapifyDown_Safe(
	PriorityQueueClass_AStarHierarchical* queue,
	int index)
{
	if (!queue || !queue->Heap || queue->Count <= 0)
		return;

	const int count = queue->Count;
	auto** heap = queue->Heap;

	while (true)
	{
		int smallest = index;
		int leftChild = index * 2;
		int rightChild = leftChild + 1;

		// Check left child
		if (leftChild <= count && leftChild <= queue->Capacity)
		{
			auto* leftNode = heap[leftChild];
			auto* currentNode = heap[smallest];

			if (leftNode && currentNode && leftNode->Score < currentNode->Score)
			{
				smallest = leftChild;
			}
		}

		// Check right child
		if (rightChild <= count && rightChild <= queue->Capacity)
		{
			auto* rightNode = heap[rightChild];
			auto* currentNode = heap[smallest];

			if (rightNode && currentNode && rightNode->Score < currentNode->Score)
			{
				smallest = rightChild;
			}
		}

		// If no swap needed, we're done
		if (smallest == index)
			break;

		// Swap
		auto* temp = heap[index];
		heap[index] = heap[smallest];
		heap[smallest] = temp;

		index = smallest;
	}
}

// Safe priority queue clear
static void PriorityQueue_Clear_Safe(PriorityQueueClass_AStarHierarchical* queue)
{
	if (!queue || !queue->Heap)
		return;

	int clearCount = std::min(queue->Count + 1, queue->Capacity);
	for (int i = 0; i <= clearCount; ++i)
	{
		queue->Heap[i] = nullptr;
	}
	queue->Count = 0;
}

bool __thiscall FakeAStarPathFinderClass::__Find_Path_Hierarchical(
	CellStruct* startCell,
	CellStruct* destCell,
	MovementZone mzone,
	FootClass* foot)
{
	// NOTE: initedcount is NOT incremented here - it is managed by AStarClass::Init()
	// which is called by Find_Path() before this function. The original pseudocode
	// does not increment it in Find_Path_Hierarchical.

	// Get global passability data pointer (original: MapClass___LevelAndPassabilityStruct2pointer_70)
	GlobalPassabilityData* globalPassabilityArray = MapClass::Instance->LevelAndPassabilityStruct2pointer_70;

	// Movement zone array for passability checks (original: MoveZones_AStarMoveAdjustArray[mzone])
	const int mzoneIndex = static_cast<int>(mzone);
	auto& moveZoneArray = MapClass::MovementAdjustArray[mzoneIndex];

	// Game's actual adjustments data at 0x7E3794 (connection cost weights per MovementCostType)
	const float* gameAdjustments = _pathfind_adjusment;

	// Initialize threat avoidance (matches pseudocode flow exactly)
	double threatAvoidance = 0.0;
	HouseClass* ownerHouse = nullptr;
	bool useThreatAvoidance = false;

	if (foot)
	{
		threatAvoidance = foot->GetThreatAvoidanceCoefficient();
		ownerHouse = foot->Owner;

		if (threatAvoidance > 0.00001)
			useThreatAvoidance = true;
	}

	// Get zone raw indices from cells (original: MapClass_zone_56D3F0)
	const int startZoneRaw = MapClass::Instance->MapClass_zone_56D3F0(startCell);
	const int destZoneRaw = MapClass::Instance->MapClass_zone_56D3F0(destCell);

	// Process hierarchy levels 2, 1, 0 (coarse to fine)
	for (int level = 2; level >= 0; --level)
	{
		// Clear hierarchical queue (original clears Elements[] and resets Count)
		PriorityQueue_Clear_Safe(this->HierarchyQueue);

		// Get subzone indices from GlobalPassabilityData::data[level]
		// These are unsigned WORDs (zone indices), zero-extended to int
		const unsigned short startZone = globalPassabilityArray[startZoneRaw].data[level];
		const unsigned short destZone = globalPassabilityArray[destZoneRaw].data[level];

		const bool isTopLevel = (level == 2);

		// Parent level visited array (null for top level)
		int* parentLevelVisited = isTopLevel ? nullptr : this->LevelVisitedMarkers[level + 1];

		// Current level arrays
		int* visitedArray = this->LevelVisitedMarkers[level];
		int* openSetArray = this->OpenSetMarkers[level];
		float* costArray = this->GCostArray[level];

		// Mark start and dest as visited
		visitedArray[startZone] = this->SearchID;
		visitedArray[destZone] = this->SearchID;

		// Early exit: start equals destination (original: if v10 == v75)
		if (startZone == destZone)
		{
			if (level == 0)
			{
				this->HierarchyBuffer->Number = 0;
				this->HierarchyBuffer->Index = static_cast<DWORD>(startZone);
			}

			this->PassabilityData[level].Indices[0] = static_cast<unsigned short>(startZone);
			this->PassabilityCounts[level] = 1;
			continue; // goto LABEL_85 equivalent
		}

		// Initialize first element in node buffer
		AStarQueueNodeHierarchical* firstElement = this->HierarchyBuffer;
		firstElement->BufferDelta = -1;  // No parent (sentinel)
		firstElement->Index = static_cast<DWORD>(startZone);
		firstElement->Score = 0.0f;
		firstElement->Number = 0;

		if (!PriorityQueue_Insert_Safe(this->HierarchyQueue, firstElement))
			return false;

		int elementCount = 1;                 // v69: tracks buffer allocation index
		int bufferOffset = 1 * 16;            // v70: byte offset into buffer for next node

		openSetArray[startZone] = this->SearchID;
		costArray[startZone] = 0.0f;

		// Pop initial element from queue
		AStarQueueNodeHierarchical* currentElement = PriorityQueue_Pop_Safe(this->HierarchyQueue);
		if (!currentElement)
			return false;

		const bool noBlockedPairs = (this->ZoneIndices[level].Count == 0);

		// SubzoneTracking data for this level
		auto& subzoneTrackingArray = SubzoneTrackingStruct::Array[level];

		// Main A* search loop
		while (true)
		{
			const int currentNode = static_cast<int>(currentElement->Index);

			// Reached destination?
			if (currentNode == static_cast<int>(destZone))
				break;

			// Get neighbor connections for current node
			SubzoneTrackingStruct& currentSubzone = subzoneTrackingArray.Items[currentNode];
			auto& connections = currentSubzone.SubzoneConnections;
			const int neighborCount = connections.Count;

			if (neighborCount > 0)
			{
				for (int n = 0; n < neighborCount; ++n)
				{
					SubzoneConnectionStruct& connection = connections.Items[n];

					const int neighborNode = static_cast<int>(connection.NeighborSubzoneIndex);
					const char connectionFlag = static_cast<char>(connection.ConnectionPenaltyFlag);

					// Get neighbor's subzone data
					SubzoneTrackingStruct& neighborSubzone = subzoneTrackingArray.Items[neighborNode];
					const unsigned short parentZone = neighborSubzone.ParentZoneIndex;
					const int movementType = static_cast<int>(neighborSubzone.MovementCostType);

					// Calculate threat cost if applicable
					int threatCost = 0;
					if (useThreatAvoidance)
					{
						const int rawThreat = MapClass::Instance->subZone_585F40(
							ownerHouse, level, currentNode, neighborNode);
						threatCost = static_cast<int>(static_cast<double>(rawThreat) * threatAvoidance);
					}

					// Connection penalty (original: ptrfield_4 ? 0.001 : 0.0)
					const float connectionPenalty = connectionFlag ? 0.001f : 0.0f;

					// Calculate new cost using game's adjustment data at 0x7E3794
					const float newCost = gameAdjustments[movementType]
						+ currentElement->Score
						+ static_cast<float>(threatCost)
						+ connectionPenalty;

					// Condition 1: Not in open set, or found a better path
					if (openSetArray[neighborNode] == this->SearchID
						&& !(costArray[neighborNode] > static_cast<double>(newCost)))
					{
						continue;
					}

					// Condition 2: Parent level connectivity check
					// (isTopLevel || parentVisited[parentZone] == SearchID || movementType == 1)
					if (!isTopLevel
						&& parentLevelVisited[parentZone] != this->SearchID
						&& movementType != 1)
					{
						continue;
					}

					// Condition 3: Movement zone passability check
					if (moveZoneArray[movementType] != 1)
						continue;

					// Condition 4: Blocked cell pairs check
					if (!noBlockedPairs)
					{
						unsigned short lo = static_cast<unsigned short>(neighborNode);
						unsigned short hi = static_cast<unsigned short>(currentNode);

						if (lo < hi)
							std::swap(lo, hi);

						const int pairKey = static_cast<int>(lo) | (static_cast<int>(hi) << 16);

						auto& blockedVector = this->ZoneIndices[level];
						bool isBlocked = false;

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

					// Create new node in buffer and push to queue
					char* buffer = reinterpret_cast<char*>(this->HierarchyBuffer);
					AStarQueueNodeHierarchical* newElement = reinterpret_cast<AStarQueueNodeHierarchical*>(buffer + bufferOffset);

					// BufferDelta = byte offset of parent in buffer, divided by 16
					newElement->BufferDelta = static_cast<int>(
						(reinterpret_cast<char*>(currentElement) - buffer) >> 4);
					newElement->Index = static_cast<DWORD>(neighborNode);
					newElement->Score = newCost;
					newElement->Number = currentElement->Number + 1;

					if (!PriorityQueue_Insert_Safe(this->HierarchyQueue, newElement))
						continue;

					// Update open set and cost
					openSetArray[neighborNode] = this->SearchID;
					costArray[neighborNode] = newCost;

					++elementCount;
					bufferOffset += 16;
				}
			}

			// Pop next best element from queue
			currentElement = PriorityQueue_Pop_Safe(this->HierarchyQueue);
			if (!currentElement)
			{
				Debug::Log("[A* Hierarchical] No path at level %d: zones %u->%u\n",
					level, startZone, destZone);
				return false;
			}
		}

		// currentElement now points to the destination node
		if (!currentElement)
			return false;

		// Phase 1: Walk back through path, marking nodes as visited in visitedArray
		// This is used by lower hierarchy levels to validate parent connectivity
		auto* walkNode = currentElement;
		if (walkNode->BufferDelta != -1)
		{
			do
			{
				visitedArray[walkNode->Index] = this->SearchID;

				walkNode = reinterpret_cast<AStarQueueNodeHierarchical*>(
					reinterpret_cast<char*>(this->HierarchyBuffer)
					+ walkNode->BufferDelta * 16);
			}
			while (walkNode->BufferDelta != -1);
		}

		// Phase 2: Store path in PassabilityData array (reversed)
		const int pathLength = currentElement->Number + 1;
		this->PassabilityCounts[level] = pathLength;

		int pathIdx = pathLength - 1;
		auto* storeNode = currentElement;

		if (pathIdx > 0)
		{
			short* resultPtr = reinterpret_cast<short*>(&this->PassabilityData[level].Indices[pathIdx]);

			do
			{
				*resultPtr = static_cast<short>(storeNode->Index);
				--resultPtr;

				storeNode = reinterpret_cast<AStarQueueNodeHierarchical*>(
					reinterpret_cast<char*>(this->HierarchyBuffer)
					+ storeNode->BufferDelta * 16);
				--pathIdx;
			}
			while (pathIdx > 0);
		}

		// Store first node (the start zone)
		this->PassabilityData[level].Indices[0] = static_cast<unsigned short>(storeNode->Index);
	}

	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x42C290, FakeAStarPathFinderClass::__Find_Path_Hierarchical)


ObjectClass* GetCellObj(CellClass* pCell,  bool alt)
{
	return alt ? pCell->AltObject : pCell->FirstObject;
}

AStarPathFinderClass::AStarPathFinderClass()
	: unknown_byte_0 { 0 }
	, FindBridgeDir { false }
	, unknown_byte_2 { 0 }
	, CanFindPath { true }                         // [esi+3] = 1
	, PathCostFactor { 1.0f }                         // 3F800000h
	, IsAlt { true }                         // [esi+8] = 1
	, PathNodeBuffer { nullptr }                      // [esi+0Ch]
	, PathQueueBuffer { nullptr }                      // [esi+10h]
	, PathQueue { nullptr }                      // [esi+14h]
	, VisitCounts { nullptr }                      // [esi+18h]
	, AltVisitCounts { nullptr }                      // [esi+1Ch]
	, AltDistances { nullptr }                      // [esi+20h]
	, Distances { nullptr }                      // [esi+24h]
	, SearchID { -1 }                           // [esi+28h]
	, FinderSpeedType { static_cast<SpeedType>(-1) }   // [esi+2Ch]
	, IsSearching { true }                         // [esi+38h]
	, FindMode { ASTAR_PASS_0 }                 // [esi+3Ch]
	, HierarchyBuffer { nullptr }                      // [esi+64h]
	, HierarchyQueue { nullptr }                      // [esi+68h]
	, PathLength { -1 }                           // [esi+6Ch]
	, CellStructBuffer { 0, 0 }                         // [esi+70h/72h]
{

	// --- ZoneIndices[0..2] [0x42A718–0x42A73D] ---
	// Asm: 3× DynamicVectorClass ctor + vftable fix-up + GrowthStep=10 + ActiveCount=0
	for (int i = 0; i < 3; ++i)
	{
		ZoneIndices[i] = DynamicVectorClass<CellStruct>();
	}

	// --- PathQueue: TPriorityQueueClass<AStarWorkPathStruct> [0x42A73F–0x42A78D] ---
	// 0x14-byte header + 0x40004 Nodes array (65537 × 4 bytes)
	// IDA: AStarStruct1_14 → [esi+14h]
	// VERIFY: confirm YRpp field name for [esi+14h]
	PathQueue = AllocPriorityQueue<AStarWorkPathStruct>(0x10000, 0x40004u);

	// --- HierarchyQueue: TPriorityQueueClass<AStarWorkPathStructNode> [0x42A78E–0x42A7DF] ---
	// 0x14-byte header + 0x9C44 Nodes array
	// IDA: AStarclass1_68 → [esi+68h]
	// VERIFY: confirm YRpp field name for [esi+68h]; sizeof gap — see header
	HierarchyQueue = reinterpret_cast<PriorityQueueClass_AStarHierarchical*>(
		AllocPriorityQueue<AStarWorkPathStructNode>(0x2710, 0x9C44u));

	// --- PathQueueBuffer: AStarWorkPathStructHeap [0x42A7E0–0x42A813] ---
	// IDA: AStarStruct4_10 → [esi+10h]
	// sizeof(AStarWorkPathStructHeap) = 16 × 65536 + 4 = 0x100004 ✓
	// Asm zeroes @4,@8,@C of each AStarWorkPathStruct entry — skips @0 (Data*).
	// SUSPECT: Data* left uninitialised by vanilla — intentional, set before use.
	{
		auto* buf = static_cast<AStarWorkPathStructHeap*>(operator new(0x100004u));
		if (buf)
		{
			auto* cursor = reinterpret_cast<std::uint32_t*>(
				reinterpret_cast<std::uint8_t*>(buf) + 8);

			for (int n = 0x10000; n > 0; --n)
			{
				cursor[-1] = 0u;    // MovementCost
				cursor[0] = 0u;    // PathCost
				cursor[1] = 0u;    // PathLength
				cursor += 4;
			}

			buf->ActiveCount = 0;
			PathQueueBuffer = buf;
		}
	}

	// --- PathNodeBuffer: AStarWorkPathStructDataHeap [0x42A814–0x42A834] ---
	// IDA: AStarStruct3_C → [esi+0Ch]
	// VERIFY: sizeof(AStarWorkPathStructNode) == 12 → 12 × 131072 = 0x180000 + 4 = 0x180004 ✓
	{
		auto* buf = static_cast<AStarWorkPathStructDataHeap*>(operator new(0x180004u));
		if (buf)
		{
			buf->ActiveCount = 0;   // [eax+180000h] at 0x42A828
			PathNodeBuffer = buf;
		}

		// BUGFIX[0x42A835]: vanilla writes sentinel again unconditionally — null-deref on failure.
		if (PathNodeBuffer)
			PathNodeBuffer->ActiveCount = 0;
	}

	// BUGFIX[0x42A840]: same pattern for PathQueueBuffer.
	if (PathQueueBuffer)
		PathQueueBuffer->ActiveCount = 0;

	// --- PathQueue::Clear() inlined [0x42A846–0x42A85A] ---
	// Redundant (Count already 0) but vanilla does it — reproduced for behavioral parity.
	if (PathQueue)
		PathQueue->Clear();

	// --- HierarchyQueue::Clear() inlined [0x42A85D–0x42A873] ---
	if (HierarchyQueue)
		reinterpret_cast<TPriorityQueueClass<AStarWorkPathStructNode>*>(HierarchyQueue)->Clear();

	// --- Triple init loop [0x42A876–0x42A8DE] ---
	// asm: ebp=pThis+0x4C, writes [ebp-0xC]/[ebp]/[ebp+0xC] per iteration (ebp+=4)
	// YRpp: LevelVisitedMarkers[3]@0x40, OpenSetMarkers[3]@0x4C, GCostArray[3]@0x58
	// PassabilityCounts offset: 0x4C + 0xC28 = 0xC74 = PassabilityCounts[0] ✓
	for (int i = 0; i < 3; ++i)
	{
		LevelVisitedMarkers[i] = nullptr;
		OpenSetMarkers[i] = nullptr;
		GCostArray[i] = nullptr;

		ZoneIndices[i].clear();     // vtable [edx+0Ch] at 0x42A8A3

		std::memset(&PassabilityData[i], 0, sizeof(AStarClass_PassabilityData));

		PassabilityCounts[i] = 0;
	}

	// --- HierarchyBuffer: raw node pool [0x42A8E0–0x42A8ED] ---
	// IDA: buffer4 → [esi+64h]
	// VERIFY: sizeof(AStarQueueNodeHierarchical) == 16 → 0x27100 / 16 == 10000 nodes
	HierarchyBuffer = static_cast<AStarQueueNodeHierarchical*>(operator new(0x27100u));
}

AStarPathFinderClass::~AStarPathFinderClass()
{
	// --- PathQueue: delete Nodes array then header [esi+14h] ---
	// IDA: v2 = AStarStruct1_14; delete v2->Elements; delete v2;
	if (PathQueue)
	{
		operator delete(PathQueue->Nodes);
		operator delete(PathQueue);
		PathQueue = nullptr;
	}

	// --- HierarchyQueue: same pattern [esi+68h] ---
	// IDA: v3 = AStarclass1_68; delete v3->Elements; delete v3;
	// Null written BEFORE v3 block in vanilla (this->AStarStruct1_14 = 0 between v2/v3).
	// DIFF: we null after delete — functionally identical, cleaner.
	if (HierarchyQueue)
	{
		auto* q = reinterpret_cast<TPriorityQueueClass<AStarWorkPathStructNode>*>(HierarchyQueue);
		operator delete(q->Nodes);
		operator delete(q);
		HierarchyQueue = nullptr;
	}

	// --- PathQueueBuffer [esi+10h] ---
	// IDA: v4 = AStarStruct4_10; delete v4;
	if (PathQueueBuffer)
	{
		operator delete(PathQueueBuffer);
		PathQueueBuffer = nullptr;
	}

	// --- PathNodeBuffer [esi+0Ch] ---
	// IDA: v5 = AStarStruct3_C; delete v5;
	if (PathNodeBuffer)
	{
		operator delete(PathNodeBuffer);
		PathNodeBuffer = nullptr;
	}

	// --- VisitCounts [esi+18h] ---
	// IDA: v6 = pvoid18; delete v6; pvoid18 = 0;
	if (VisitCounts)
	{
		operator delete(VisitCounts);
		VisitCounts = nullptr;
	}

	// --- AltVisitCounts [esi+1Ch] ---
	if (AltVisitCounts)
	{
		operator delete(AltVisitCounts);
		AltVisitCounts = nullptr;
	}

	// --- Distances [esi+24h] = costarray2 — deleted BEFORE AltDistances ---
	// VERIFY: IDA deletes costarray2 before costarray1; confirm field order matters.
	if (Distances)
	{
		operator delete(Distances);
		Distances = nullptr;
	}

	// --- AltDistances [esi+20h] = costarray1 ---
	if (AltDistances)
	{
		operator delete(AltDistances);
		AltDistances = nullptr;
	}

	// --- LevelVisitedMarkers / OpenSetMarkers / GCostArray [0x42A876 pattern] ---
	// IDA: v7 = pointers_4C; loop 3×: delete *(v7-3), delete *v7, delete v7[3]; ++v7
	// v7 starts at OpenSetMarkers[0] (offset 0x4C).
	// *(v7-3) = LevelVisitedMarkers[i] (offset 0x40 base)
	// *v7     = OpenSetMarkers[i]      (offset 0x4C base)
	// v7[3]   = GCostArray[i]          (offset 0x58 base)
	for (int i = 0; i < 3; ++i)
	{
		if (LevelVisitedMarkers[i])
		{
			operator delete(LevelVisitedMarkers[i]);
			LevelVisitedMarkers[i] = nullptr;
		}

		if (OpenSetMarkers[i])
		{
			operator delete(OpenSetMarkers[i]);
			OpenSetMarkers[i] = nullptr;
		}

		if (GCostArray[i])
		{
			operator delete(GCostArray[i]);
			GCostArray[i] = nullptr;
		}
	}

	// --- HierarchyBuffer [esi+64h] ---
	// IDA: delete buffer4 — no null written after, vanilla pattern reproduced.
	operator delete(HierarchyBuffer);
	HierarchyBuffer = nullptr;

	// --- ZoneIndices[0..2]: DynamicVectorClass dtor ---
	// IDA v9 loop is garbled — real intent confirmed by pattern:
	//   checks internal buffer pointer, if IsAllocated frees it, zeros pointer + ActiveCount.
	// Calling explicit dtor handles IsAllocated check internally.
	// DIFF: vanilla iterates in descending order (v10 counts down from 3).
	//   DynamicVectorClass dtor is order-independent here (no cross-references between slots).
	for (int i = 2; i >= 0; --i)
		ZoneIndices[i].~DynamicVectorClass<CellStruct>();
}

// Cost multiplier per pass/layer index (a5).
// flt_81870C[0..6] = {1.0, 1000.0, 1.0, 1.0, 60.0, 20.0, 8.0}
// VERIFY: indices 3..6 — only 0/1/2 used by AStarPostProcessType; rest may be
//         used by other callers. Array size confirmed as 7 entries from segment dump.
static constexpr float PassCostMultiplier[7] =
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
static constexpr int NeighborIndexTable_Flat[8] =
{
	-2, -2, 0, 1, 1, 1, 0, -2
};

// Direction-to-neighbor-index LUT for bridge cells.
// dword_7E3730[0..7] = {0,-1024,-1024,-1024,0,512,512,512}
// Used when CellClass::Bitfield2 bit 0x800 is set.
// VERIFY: naming — "bridge neighbor offset table"; large values suggest zone/buffer indexing
static constexpr int NeighborIndexTable_Bridge[8] =
{
	0, -1024, -1024, -1024, 0, 512, 512, 512
};

// Cell delta to direction index LUT.
// dword_7E3760[0..4] = {-1, 2, 5, 4, 3}
// Indexed by: 3 * (target.Y - source.Y) + (target.X - source.X)
// Maps a {dx,dy} delta in [-1,1] to a facing/direction index.
// VERIFY: -1 entry at index 0 — may indicate invalid/diagonal-only direction.
static constexpr int DeltaToDirectionIndex[5] =
{
	-1, 2, 5, 4, 3
};

// Scalar float constants confirmed from asm bit patterns:
// flt_7E37B4 = 2.0f  (both-sides-bridge multiplier)
// flt_7E37B8 = 10.0f (no-bridge penalty)
// flt_7E37BC = 4.0f  (tunnel/wall cost multiplier, bit 0x40000)
static constexpr float BridgeBothSidesMultiplier = 2.0f;
static constexpr float NoBridgeMultiplier = 10.0f;
static constexpr float TunnelCostMultiplier = 4.0f;

// -----------------------------------------------------------------------
// AStarPathFinderClass::Calc_Float()  [0x429830]
//
// Calculates the movement cost multiplier for entering a target cell.
//
// arg0     : pointer to array of CellClass* — source cell neighborhood buffer
// a3       : pointer to array of CellClass* — target cell neighborhood buffer
// a4       : bool — use alt occupier (bridge layer)
// a5       : pass/layer index → indexes PassCostMultiplier
// a6       : unused in this function body (confirmed by asm — no reference after entry)
//
// VERIFY: a3 is CellClass** (neighborhood buffer pointer), not a single CellClass*.
//         Pseudocode uses a3 as both array pointer and single-cell pointer — asm confirms
//         [esi] dereference at entry, so *a3 is the target cell. Naming kept as-is.
// VERIFY: return type — IDA says double but asm uses x87 float stack with single-precision
//         constants (fld dword). SUSPECT: actually returns float promoted to double by ABI.
// -----------------------------------------------------------------------
double AStarPathFinderClass::Calc_Float(
	CellClass** arg0,
	CellClass** a3,
	int         a4,
	int         a5,
	int         a6) const
{
	CellClass* targetCell = *a3;
	CellClass* sourceCell = *arg0;

	// Load base cost for this pass layer
	float cost = PassCostMultiplier[a5];

	// --- Occupier chain walk (only for pass layer 2) [0x42985C–0x4299A9] ---
	// Walks the occupier linked list of the target cell to detect blocking units.
	// Adjusts cost based on FindMode.
	if (a5 == 2)
	{
		ObjectClass* occupier = GetCellObj(targetCell, a4);

		int chainDepth = 0;
		bool chainBroken = false;

		if (!FindMode)
		{
			while (occupier)
			{
				// Check IsDisabled bit (TargetBitfield[0] & 4) — skip disabled units
				if ((occupier->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
				{
					chainBroken = true;
					break;
				}

				unsigned int facingIndex;
				FootClass* pFootOccupy = (FootClass*)occupier;

				if (pFootOccupy->SpeedPercentage == 0.0)
				{
					// Unit is stopped — use its queued path direction
					facingIndex = pFootOccupy->PathDirections[0];
					if (facingIndex == static_cast<unsigned int>(-1))
						break; // no path queued — chain ends cleanly
				}
				else
				{
					// Unit is moving — derive facing from PrimaryFacing
					// Asm: (FacingClass::Current >> 12 + 1) >> 1  & 7
					DirStruct dir = pFootOccupy->PrimaryFacing.Current();
					facingIndex = (((static_cast<unsigned int>(dir.Raw) >> 12) + 1) >> 1) & 7u;
				}

				// Get cell the occupier is heading toward
				CellStruct occupierCell = occupier->GetMapCoords();
				CellStruct nextCell =
				{
					static_cast<short>(CellSpread::AdjacentCell[facingIndex & 7u].X + occupierCell.X),
					static_cast<short>(occupierCell.Y + CellSpread::AdjacentCell[facingIndex & 7u].Y)
				};

				CellClass* nextCellPtr = MapClass::Instance->GetCellAt(nextCell);

				// Determine which occupier layer to follow into next cell
				// Asm: checks Bitfield2 bit 0x100 (IsBridge?) and IsOnBridge / level delta
				bool usesAlt = nextCellPtr->ContainsBridge()
					&& (pFootOccupy->OnBridge
						|| (pFootOccupy->GetCell()->Level - nextCellPtr->Level > 2));

				occupier = GetCellObj(nextCellPtr, usesAlt);
				++chainDepth;

				if (chainDepth >= 10)
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

		// Apply cost penalty based on FindMode and chain result
		// Asm at 0x42998D: FindMode != 0 → cost = 4.0
		// Asm at 0x429995: FindMode == 2 → cost = 1000.0
		if (chainBroken || FindMode)
			cost = 4.0f;

		if (FindMode == 2)
			cost = 1000.0f;
	}

	// --- Tunnel/wall penalty [0x4299AA–0x4299C5] ---
	// CellClass::Bitfield2 bit 0x40000 — tunnel or similar obstruction
	int bitfield2 = targetCell->UINTFlags;
	if (bitfield2 & 0x40000)
		cost *= TunnelCostMultiplier;

	// --- Early out: no alt layer or FindBridgeDir not set [0x4299C6–0x429A82] ---
	if (!a4 || !FindBridgeDir)
		return cost;

	// --- Bridge direction cost adjustment [0x4299D7–0x429A7F] ---
	// Computes which direction we're moving (dx,dy) then looks up neighbor cells
	// to determine if we're crossing a bridge cleanly or at an angle.
	// VERIFY: a3 here is the neighborhood buffer — [esi+ecx*4] indexes into it.

	// Delta from source to target position
	int dx = targetCell->MapCoords.X - sourceCell->MapCoords.X;
	int dy = targetCell->MapCoords.Y - sourceCell->MapCoords.Y;

	// Map (dx,dy) in {-1,0,1} → flat index [0..4] via: 3*dy + dx, offset by +4
	// Asm: lea ecx,[ecx+ecx*2]; add ecx,eax → 3*dy+dx
	int dirIndex = DeltaToDirectionIndex[3 * dy + dx + 4]; // VERIFY: +4 offset — confirm bounds

	CellClass* neighborA;
	CellClass* neighborB;
	int        neighborBIdx;

	if (bitfield2 & 0x800) // bridge cell flag
	{
		int idxA = NeighborIndexTable_Bridge[dirIndex];
		neighborBIdx = NeighborIndexTable_Bridge[(dirIndex - 4) & 7];
		neighborA = a3[idxA];
		neighborB = a3[neighborBIdx];
	} else {
		int idxA = NeighborIndexTable_Flat[dirIndex];
		neighborBIdx = NeighborIndexTable_Flat[(dirIndex - 4) & 7];
		neighborA = a3[idxA];
		neighborB = a3[neighborBIdx];
	}

	// neighborA not a bridge cell → heavy penalty
	if (neighborA->ContainsBridge())
		return NoBridgeMultiplier * cost;

	// neighborB also a bridge cell → both-sides bridge, halved penalty
	const float multiplier = neighborB->ContainsBridge() != 0 ?
		BridgeBothSidesMultiplier : 1.0f;

	return multiplier * cost;
}

AStarWorkPathStruct* AStarPathFinderClass::Calc_sqrt(
	AStarWorkPathStruct* parentNode,
	CellClass** a3,
	CellStruct* goalCell,
	float                a5)
{
	// --- Allocate next AStarWorkPathStruct from PathQueueBuffer [0x42A460–0x42A47E] ---
	// asm: edx = [eax+100000h] (ActiveCount), esi = edx*16 + eax, then increment
	AStarWorkPathStruct* newQueueNode =
		&PathQueueBuffer->Nodes[PathQueueBuffer->ActiveCount++];

	// --- Allocate next AStarWorkPathStructNode from PathNodeBuffer [0x42A47F–0x42A494] ---
	// asm: edx = [eax+180000h] (ActiveCount), edi = edx*12 + eax, then increment
	AStarWorkPathStructNode* newPathNode =
		&PathNodeBuffer->Nodes[PathNodeBuffer->ActiveCount++];

	// --- Init newPathNode->Cells [0x42A495–0x42A49C] ---
	newPathNode->Cells = a3;

	if (parentNode)
	{
		// --- Copy parent's Cells pointer into Prev [0x42A49F–0x42A4A4] ---
		// asm: [edi+8] = [ebx]  where ebx=parentNode → *parentNode = parentNode->Data
		newPathNode->Prev = parentNode->Data;

		// --- CellLevel from target cell [0x42A4A4–0x42A4B5] ---
		// asm: movsx edx,[eax+11Bh] where eax=*a3 → (*a3)->Level
		CellClass* targetCell = *a3;
		CellClass* parentFirst = *parentNode->Data->Cells; // ***a2 in pseudocode
		int        targetLevel = targetCell->Level;        // byte at +0x11B, sign-extended

		newPathNode->CellLevel = targetLevel;

		// --- Bridge crossing level adjustment [0x42A4B6–0x42A522] ---
		// targetCell is a bridge cell (Bitfield2 & 0x100)
		if (targetCell->ContainsBridge())
		{
			bool parentIsBridge = parentFirst->ContainsBridge();

			if (parentIsBridge)
			{
				// Both cells are bridge — check if parent level + 4 == parentFirst level
				// asm: [ebp+4] == parentFirst->Level + 4
				// VERIFY: [ebp+4] here is parentNode->Data->CellLevel (int at +4 in node)
				if (parentNode->Data->CellLevel == parentFirst->Level + 4)
				{
					newPathNode->CellLevel = targetLevel + 1;
					// falls through to LABEL_12
				}
				// else: parentIsBridge still set, falls to test below
			}

			if (!parentIsBridge)
			{
				// Parent is not a bridge — check level delta
				// asm: abs(targetLevel - [edx+4] + 3) <= 1
				// [edx+4] = parentNode->Data->CellLevel
				// DIFF: IDA __int64 abs pattern = standard abs() on int
				int delta = targetLevel - parentNode->Data->CellLevel + 3;
				if (std::abs(delta) <= 1)
				{
					// Bump level by 4 (bridge entry)
					newPathNode->CellLevel = newPathNode->CellLevel + 4;
				}
				// else: leave CellLevel as targetLevel (no adjustment needed)
			}
		}
		// non-bridge target: CellLevel stays as targetLevel — no adjustment
	}
	else
	{
		// --- Root node (no parent) [0x42A516–0x42A521] ---
		newPathNode->Prev = nullptr;
		newPathNode->CellLevel = StartLevel;   // [ecx+30h] — VERIFY YRpp field name
	}

	// --- Link newQueueNode -> newPathNode [0x42A523–0x42A525] ---
	newQueueNode->Data = newPathNode;

	// --- Cost fields [0x42A526–0x42A549] ---
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

	// --- Heuristic: Euclidean distance to goal [0x42A54A–0x42A59E] ---
	// asm: abs((*a3)->Position.X - goalCell->X) and abs((*a3)->Position.Y - goalCell->Y)
	// then: Sqrt(dx*dx + dy*dy) + MovementCost → PathCost
	// DIFF: IDA uses unsigned abs via cdq/xor/sub — std::abs on int is equivalent.
	CellClass* targetCell = *a3;
	int dx = std::abs(static_cast<int>(targetCell->MapCoords.X) - static_cast<int>(goalCell->X));
	int dy = std::abs(static_cast<int>(targetCell->MapCoords.Y) - static_cast<int>(goalCell->Y));

	newQueueNode->PathCost = static_cast<float>(Math::sqrt(
		static_cast<double>(dx * dx + dy * dy))) + newQueueNode->MovementCost;

	return newQueueNode;
}

COMPILETIMEEVAL reference<int, 0x89C2DC> const RegionSize {};
COMPILETIMEEVAL constant_ptr<int*, 0x89C2DC> const MapClass___movezones {};

void AStarPathFinderClass::Init()
{
	// --- Reset PathNodeBuffer and PathQueueBuffer write heads [0x42A5B3–0x42A5C8] ---
	PathNodeBuffer->ActiveCount = 0;
	PathQueueBuffer->ActiveCount = 0;

	// --- PathQueue::Clear() [0x42A5C9–0x42A5E1] ---
	if (PathQueue)
		PathQueue->Clear();

	// --- HierarchyQueue::Clear() [0x42A5E2–0x42A5FA] ---
	if (HierarchyQueue)
		reinterpret_cast<TPriorityQueueClass<AStarWorkPathStructNode>*>(
			HierarchyQueue)->Clear();

	// --- SearchID generation counter [0x42A5FB–0x42A602] ---
	// Full zone clear only fires when SearchID wraps to 0 (every 2^32 calls).
	++SearchID;
	if (SearchID != 0)
		return;

	// === Full zone array reinit (SearchID overflow path only) ===

	// --- VisitCounts / AltVisitCounts clear [0x42A608–0x42A623] ---
	// Asm: counts down from (RegionSize² - 1) to 0, writes [edx+eax*4+4].
	// VERIFY: +4 offset = 1-based slot indexing, slot 0 never written — intentional.
	{
		const int regionArea = RegionSize() * RegionSize();
		for (int k = regionArea - 1; k >= 0; --k)
		{
			VisitCounts[k + 1] = 0;
			AltVisitCounts[k + 1] = 0;
		}
	}

	// --- Triple pass: zero LevelVisitedMarkers/OpenSetMarkers/GCostArray [0x42A624–0x42A67E] ---
	// Asm: eax = (byte*)MapClass___movezones[4] - (byte*)this (fixed delta)
	//      [eax + OpenSetMarkers[i]] reads the per-pass entry count from movezones.
	// Confirmed double-index from sub_594B50: MapClass___movezones[MapType][PassType].
	// Here index [4+i] selects the zone pass; inner index implicit (likely 0).
	// SUSPECT: exact inner index — VERIFY which passability slot Init() uses.
	// Inner loop zeroes entries [count-1..0] of all three parallel arrays.
	for (int i = 0; i < 3; ++i)
	{
		// VERIFY: MapClass___movezones[4+i][0] as the count — confirm inner index.
		int count = MapClass___movezones[4 + i]
			? static_cast<int>(MapClass___movezones[4 + i][0])
			: 0;

		for (int k = count - 1; k >= 0; --k)
		{
			LevelVisitedMarkers[i][k] = 0;
			OpenSetMarkers[i][k] = 0;
			reinterpret_cast<int*>(GCostArray[i])[k] = 0;  // 0 bits == 0.0f, IEEE754 safe
		}
	}

	// --- SearchID bumped again after full clear [0x42A683] ---
	// Net: 0 → 1, so normal path skips the next (2^32 - 1) calls.
	++SearchID;
}

bool AStarPathFinderClass::IsVisited(int index, bool useAlt) const
{
	if (useAlt)
		return VisitCounts[index] == SearchID;

	return AltVisitCounts[index] == SearchID;
}

static constexpr int CellAdjacencyDirectionLUT[9] =
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

int __fastcall AStarPathFinderClass::CellStruct_helper_distance(CellStruct* a1, CellStruct* a2)
{
	static constexpr int InvalidAdjacency = 8;

	const int dy = a2->Y - a1->Y;

	if (std::abs(dy) > 1)
		return InvalidAdjacency;

	const int dx = a2->X - a1->X;

	if (std::abs(dx) > 1)
		return InvalidAdjacency;

	return CellAdjacencyDirectionLUT[3 * dy + dx + 4];
}

COMPILETIMEEVAL reference<PathType, 0x89A2D8> const path {};
COMPILETIMEEVAL reference<CellStruct, 0x89A324, 0x7D0> const MainOverlap {};

PathType* AStarPathFinderClass::Get_Path(AStarWorkPathStruct* work_path, FacingType* moves)
{
	// --- One-time atexit registration [0x42AA90–0x42AAB3] ---
	// --- Init path fields [0x42AABC–0x42AAFB] ---
	// path.Cost: ftol(PathCost) — truncate float to int
	path->Cost = static_cast<int>(work_path->PathCost);
	path->Length = work_path->PathLength;
	path->field_10 = 0;
	path->LastOverlap = { 0, 0 };       // asm: two word zeroes at path+0x18/0x1A
	path->Command = moves;
	path->Overlap = reinterpret_cast<CellStruct*>(MainOverlap());

	// --- Walk node chain [0x42AAFC–0x42ABA6] ---
	// v5 = current node (start of chain = deepest/goal node)
	// v6 = v5->Prev (parent — one step back toward start)
	// Loop runs PathLength-1 steps, filling moves[] in reverse.
	AStarWorkPathStructNode* curNode = work_path->Data;
	AStarWorkPathStructNode* parentNode = work_path->Data->Prev;

	// v15 starts at moves[PathLength-2], decrements each iteration
	// asm: lea edi,[edx+eax*4] where eax=PathLength-2
	const int loopCount = work_path->PathLength - 2;

	if (loopCount >= 0)
	{
		// Byte delta: MainOverlap - moves (used to write overlap at same index as moves)
		// asm: ecx = MainOverlap; sub ecx,edx → ecx = MainOverlap - moves (byte delta)
		const std::ptrdiff_t overlapDelta =
			reinterpret_cast<std::uint8_t*>(MainOverlap()) -
			reinterpret_cast<std::uint8_t*>(moves);

		FacingType* writePtr = moves + loopCount;   // starts at moves[PathLength-2]
		int         remaining = loopCount + 1;

		do
		{
			if (parentNode)
			{
				// Write CellLevel into MainOverlap at same index as writePtr
				// asm: [ecx+eax] = [ebx+4] where ecx=overlapDelta applied to writePtr
				*reinterpret_cast<int*>(
					reinterpret_cast<std::uint8_t*>(writePtr) + overlapDelta)
					= parentNode->CellLevel;

				// Direction from curNode's cell to parentNode's cell
				CellStruct fromCell = (*curNode->Cells)->MapCoords;
				CellStruct toCell = (*parentNode->Cells)->MapCoords;

				// VERIFY: GetPosition() returns CellStruct* at +0x24 — asm: add ecx/esi,24h
				*writePtr = static_cast<FacingType>(
					CellStruct_helper_distance(&fromCell, &toCell));
			}

			// Advance both nodes toward start of chain
			curNode = curNode->Prev;
			parentNode = parentNode ? parentNode->Prev : nullptr;

			--writePtr;
			--remaining;
		}
		while (remaining);
	}

	// --- Terminate moves[] and write path.Start [0x42ABAF–0x42ABD3] ---
	// moves[PathLength-1] = -1 (sentinel)
	// asm: [edx + eax*4 - 4] = -1 where eax=PathLength → moves[PathLength-1]
	moves[work_path->PathLength - 1] = static_cast<FacingType>(-1);

	// path.Start = position of the start cell (curNode after loop = root node)
	// asm: mov ecx,[ebp]; mov edx,[ecx]; mov eax,[edx+24h] → (*curNode->Cells)->Position
	path->Start = (*curNode->Cells)->MapCoords;    // VERIFY: Position at +0x24 in CellClass

	// Clamp path.Cost to minimum 1
	if (!path->Cost)
		path->Cost = 1;

	return path.operator->();
}

COMPILETIMEEVAL reference<int, 0x89A304, 8> const adjust_89A304 {};   // 

void AStarPathFinderClass::Reset(RectangleStruct* rect)
{
	// --- Free existing arrays [0x42AC06–0x42AC51] ---
	// Delete order (from asm): VisitCounts, AltVisitCounts, Distances, AltDistances
	if (VisitCounts) {
		operator delete(VisitCounts);
		VisitCounts = nullptr;
	}

	if (AltVisitCounts) {
		operator delete(AltVisitCounts);
		AltVisitCounts = nullptr;
	}

	if (Distances) {
		operator delete(Distances);
		Distances = nullptr;
	}

	if (AltDistances) {
		operator delete(AltDistances);
		AltDistances = nullptr;
	}

	// --- Compute RegionSize and allocation size [0x42AC52–0x42AC6F] ---
	// asm: lea eax,[ecx+edx+1] where ecx=[eax+0Ch]=Height, edx=[eax+8]=Width
	RegionSize = rect->Width + rect->Height + 1;
	const int allocBytes = 4 * RegionSize() * RegionSize();   // sizeof(int) × RegionSize²

	// --- Allocate arrays [0x42AC70–0x42AC91] ---
	// Alloc order from asm: AltVisitCounts, VisitCounts, Distances, AltDistances
	// DIFF: alloc order reversed vs delete order — reproduced faithfully.
	AltVisitCounts = static_cast<int*>(operator new(static_cast<unsigned>(allocBytes)));
	VisitCounts = static_cast<int*>(operator new(static_cast<unsigned>(allocBytes)));
	Distances = static_cast<float*>(operator new(static_cast<unsigned>(allocBytes)));
	AltDistances = static_cast<float*>(operator new(static_cast<unsigned>(allocBytes)));

	// --- Rebuild adjust_89A304[8] [0x42AC93–0x42ACE3] ---
	// Cell index stride for each of 8 directions at current RegionSize.
	// Entry[i] = flat-index delta to move one cell in direction i.
	// asm assignment order confirmed from offsets +0,+4,+8,+C,+10,+14,+18,+1C:
	adjust_89A304[0] = -RegionSize();          // N   (row above)
	adjust_89A304[1] = 1 - RegionSize();       // NE
	adjust_89A304[2] = 1;                    // E
	adjust_89A304[3] = RegionSize() + 1;       // SE
	adjust_89A304[4] = RegionSize();           // S
	adjust_89A304[5] = RegionSize() - 1;       // SW
	adjust_89A304[6] = -1;                   // W
	adjust_89A304[7] = -1 - RegionSize();      // NW
}

static inline void ToggleBit40000(unsigned int* bitfield)
{
	*bitfield ^= (*bitfield ^ ~*bitfield) & 0x40000;
}

FootClass* AStarPathFinderClass::Get_Occupier(CellStruct* pos, int level) const
{
	// World-space center of cell at given level
	// asm: X<<8+128, Y<<8+128, level*dword_89C2D8
	const int worldX = (static_cast<int>(pos->X) << 8) + 128;
	const int worldY = (static_cast<int>(pos->Y) << 8) + 128;
	const int worldZ = level * Unsorted::LevelHeight;

	for (int dy = -2; dy < 3; ++dy)
	{
		for (int dx = -2; dx < 3; ++dx)
		{
			CellStruct searchCell =
			{
				static_cast<short>(pos->X + dx),
				static_cast<short>(pos->Y + dy)
			};

			CellClass* cell = MapClass::Instance->GetCellAt(searchCell);

			// Bridge layer selection: abs(cell->Level - level) > 2
			const int levelDelta = std::abs(static_cast<int>(cell->Level) - level);
			ObjectClass* occupier = GetCellObj(cell, (cell->ContainsBridge() && levelDelta > 2));

			while (occupier)
			{
				if (auto pFoot = flag_cast_to<FootClass*>(occupier))
				{
					// Build CoordStruct and call Is_Moving_Here
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

bool AStarPathFinderClass::Process_Paths(TechnoClass* techno)
{
	if (!CanFindPath)
		return false;

	// --- Current cell and facing [0x42AD06–0x42AD88] ---
	CellStruct curPos = techno->GetMapCoords();
	CellClass* curCell = MapClass::Instance->GetCellAt(curPos);

	// Derive facing index from PrimaryFacing — same pattern as Calc_Float
	DirStruct facingDir = techno->PrimaryFacing.Current();
	const unsigned int facingIdx =
		(((static_cast<unsigned int>(facingDir.Raw) >> 12) + 1) >> 1) & 7u;

	CellStruct frontPos =
	{
		static_cast<short>(curPos.X + CellSpread::AdjacentCell[facingIdx].X),
		static_cast<short>(curPos.Y + CellSpread::AdjacentCell[facingIdx].Y)
	};
	CellClass* frontCell = MapClass::Instance->GetCellAt(frontPos);

	// --- Bridge layer selection [0x42AD93–0x42ADD3] ---
	// abs(curCell->Level - frontCell->Level) > 3 || techno->IsOnBridge → alt layer
	const int levelDelta = std::abs(
		static_cast<int>(curCell->Level) - static_cast<int>(frontCell->Level));
	const bool useAlt = frontCell->ContainsBridge()
		&& (levelDelta > 3 || techno->OnBridge);

	ObjectClass* occupier = GetCellObj(frontCell, useAlt);

	// --- Get_Occupier fallback [0x42ADD8–0x42ADF6] ---
	if (!occupier)
	{
		// level = frontCell->Level + (useAlt ? 4 : 0)
		// asm: neg al; sbb eax,eax; and eax,4 → useAlt ? 4 : 0
		occupier = Get_Occupier(
			&frontCell->MapCoords,
			static_cast<int>(frontCell->Level) + (useAlt ? 4 : 0));
	}

	TechnoTypeClass* myType = techno->GetTechnoType();
	bool             foundBlocker = false;

	// --- Occupier chain walk [0x42AE09–0x42AFBE] ---
	while (occupier)
	{
		const AbstractType kind = occupier->WhatAmI();

		// Only Units (1) and Infantry (0xF)
		if (kind != AbstractType::Unit && kind != AbstractType::Infantry)
		{
			occupier = occupier->NextObject;
			continue;
		}

		CellStruct          occupierPos = ((FootClass*)occupier)->CurrentMapCoords;   // VERIFY: offset 0x558
		TechnoTypeClass* occupierType = occupier->GetTechnoType();

		// --- Yield condition [0x42AE4E–0x42AEF0] ---
		// FindMode==2, OR (different type AND my MaxSpeed > their MaxSpeed AND in radar)
		const bool shouldYield =
			(FindMode == 2)
			|| (myType != occupierType
				&& myType->Speed > occupierType->Speed    // VERIFY: offset 0x678
				&& MapClass::Instance->IsWithinUsableArea(occupierPos, true));

		if (!shouldYield)
		{
			occupier = occupier->NextObject;
			continue;
		}

		// --- Path validity check [0x42AE7D–0x42AEF0] ---
		// Unit needs Path[0] and Path[1] valid; Infantry also needs Path[2].
		// VERIFY: Path at [ebp+5E0h], Path[1] at [ebp+5E4h], Path[2] at [ebp+5E8h]
		bool pathValid = false;

		if (kind == AbstractType::Unit)
		{
			pathValid = (((FootClass*)occupier)->PathDirections[0] != -1 && ((FootClass*)occupier)->PathDirections[1] != -1);
		}
		else // Infantry
		{
			pathValid = (
						 ((FootClass*)occupier)->PathDirections[0] != -1
					  && ((FootClass*)occupier)->PathDirections[1] != -1
					  && ((FootClass*)occupier)->PathDirections[2] != -1);
		}

		if (!pathValid)
		{
			occupier = occupier->NextObject;
			continue;
		}

		// --- Toggle path cells [0x42AEF1–0x42AFBA] ---
		foundBlocker = true;

		int* pathStep = ((FootClass*)occupier)->PathDirections;
		int  stepCount = 0;

		while (stepCount < 24 && *pathStep != -1)
		{
			if (*pathStep == 8)
			{
				// Tunnel: look up end cell from Tubes
				// VERIFY: CellClass::TubeIndex at +0x116, TubeClass::EndCell at +0x28
				const int tubeIdx = static_cast<int>(MapClass::Instance->GetCellAt(occupierPos)->TubeIndex);    // VERIFY: TubeIndex field name

				if (tubeIdx == -1) {
					occupierPos = { 0, 0 };
				} else {
					// asm: Tubes.Vector_Item[tubeIdx]->EndCell at [edx+28h]
					occupierPos = TubeClass::Array->Items[tubeIdx]->ExitCell;
				}
			}
			else
			{
				occupierPos =
				{
					static_cast<short>(occupierPos.X + CellSpread::AdjacentCell[*pathStep].X),
					static_cast<short>(occupierPos.Y + CellSpread::AdjacentCell[*pathStep].Y)
				};
			}

			CellClass* stepCell = MapClass::Instance->GetCellAt(occupierPos);

			// Toggle bit 0x40000 — asm reads Bitfield2 twice (same cell), XORs result
			// SUSPECT: two Map[] calls to same cell in vanilla — likely redundant, one is enough
			ToggleBit40000(&stepCell->UINTFlags);

			++pathStep;
			++stepCount;
		}

		occupier = occupier->NextObject;
	}

	// --- Post-loop: FindMode==1 early return [0x42AEB9–0x42AED5] ---
	// Skip neighbor toggle only when no blocker was found AND FindMode==1.
	// All other paths (foundBlocker, or FindMode!=1) fall through to the toggle.
	if (!foundBlocker && FindMode == 1)
	{
		FindMode = ASTAR_PASS_0;
		return true;    // asm: returns `this` cast to char — non-zero = true
	}

	// --- 5×5 neighbor toggle [0x42AFCB–0x42B068] ---
	// Toggles bit 0x40000 on cells within ±2 of frontCell that have Flag set,
	// excluding the origin cell (curPos). Then toggles frontCell itself.
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

			// VERIFY: CellClass::Flag byte at offset 0x124
			if (!neighbor->OccupationFlags)
				continue;

			// Skip origin cell
			if (neighbor->MapCoords == curPos)
				continue;

			ToggleBit40000(&neighbor->UINTFlags);
		}
	}

	// Toggle frontCell itself
	ToggleBit40000(&frontCell->UINTFlags);

	return false;   // asm: returns *Bitfield2 (the toggled value) — treated as bool at call sites
}

CellStruct* __fastcall AStarPathFinderClass::tube_42D490(CellStruct* a1, CellStruct* a2, int facing)
{
	if (facing == 8) {
		int tubeIdx = MapClass::Instance->GetCellAt(a2)->TubeIndex;
		if (tubeIdx == -1) {
			*a1 = CellStruct::Empty;
		} else {
			*a1 = TubeClass::Array->Items[facing]->ExitCell;
		}
	} else {
		*a1 = CellSpread::AdjacentCell[facing & 7].operator+(*a2);
	}

	return a1;
}

CellStruct ResolveStep(int dir, CellStruct currentPos)
{
	if (dir == 8)
	{
		// Tunnel: look up EndCell from Tubes
		const int tubeIdx = static_cast<int>(MapClass::Instance->GetCellAt(currentPos)->TubeIndex);  // VERIFY: field name
		if (tubeIdx == -1)
			return { 0, 0 };

		// asm: Tubes.Vector_Item[tubeIdx] -> [edx+28h] = EndCell
		return TubeClass::Array->Items[dir]->ExitCell;
	}

	return currentPos + CellSpread::AdjacentCell[dir & 7];
}

