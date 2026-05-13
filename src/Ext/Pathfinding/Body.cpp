#include "Body.h"

#include <MapClass.h>
#include <HouseClass.h>
#include <Misc/PhobosGlobal.h>

PathType* FakeAStarPathFinderClass::__AStarClass__Find_Path(CellStruct* a2,
	CellStruct* dest,
	TechnoClass* a4,
	int* path,
	int max_count,
	MovementZone a7,
	ZoneType cellPath)
{
	PhobosGlobal::Instance()->PathfindTechno = { a4  ,*a2 , *dest };

	return this->AStarClass__Find_Path(a2, dest, a4, path, max_count, a7, cellPath);
}

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

DEFINE_FUNCTION_JUMP(CALL, 0x4CBC31, FakeAStarPathFinderClass::__AStarClass__Find_Path)
DEFINE_FUNCTION_JUMP(LJMP, 0x42C290, FakeAStarPathFinderClass::__Find_Path_Hierarchical)