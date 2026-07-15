#pragma once

#include <Base/Always.h>
#include <CellClass.h>
#include <PriorityQueueClass.h>

template<typename T, typename Pr = std::less<T>>
class SafePriorityQueueClass
{
public:
	SafePriorityQueueClass(int capacity) :
		Count(0),
		Capacity(capacity),
		MaxPointer(nullptr),
		MinPointer(reinterpret_cast<T*>(~0ULL))
	{
		if (capacity <= 0)
		{
			//Debug::FatalError("PriorityQueue created with invalid capacity: %d\n", capacity);
			capacity = 1;
		}

		Nodes = static_cast<T**>(YRMemory::AllocateChecked(sizeof(T*) * (capacity + 1)));
		std::memset(Nodes, 0, sizeof(T*) * (capacity + 1));
	}

	~SafePriorityQueueClass()
	{
		Clear();
		YRMemory::Deallocate(Nodes);
		Nodes = nullptr;
	}

	bool Push(T* pValue)
	{
		// Null check
		if (!pValue)
		{
			//Debug::Log("Attempt to push null into PriorityQueue\n");
			return false;
		}

		// Capacity check
		if (Count >= Capacity)
		{
			//Debug::Log("PriorityQueue overflow: Count=%d, Capacity=%d\n", Count, Capacity);
			return false;
		}

		// Insert at end
		Nodes[++Count] = pValue;

		// Bubble up
		int current = Count;
		while (current > 1)
		{
			int parent = current / 2;

			// Null safety
			if (!Nodes[current] || !Nodes[parent])
			{
				//Debug::Log("Null node during bubble-up at index %d\n", current);
				break;
			}

			if (!Comp(Nodes[current], Nodes[parent]))
				break;

			std::swap(Nodes[current], Nodes[parent]);
			current = parent;
		}

		// Update bounds
		UpdatePointerBounds(pValue);

		return true;
	}

	T* Pop()
	{
		if (Count <= 0)
			return nullptr;

		T* result = Nodes[1];

		// Move last to root
		Nodes[1] = Nodes[Count];
		Nodes[Count] = nullptr;
		--Count;

		// Heapify down
		if (Count > 0)
			HeapifyDown(1);

		return result;
	}

	T* Top() const
	{
		return (Count > 0) ? Nodes[1] : nullptr;
	}

	void Clear()
	{
		int clearCount = std::min(Count + 1, Capacity + 1);
		for (int i = 0; i < clearCount; ++i)
		{
			Nodes[i] = nullptr;
		}
		Count = 0;
		MaxPointer = nullptr;
		MinPointer = reinterpret_cast<T*>(~0ULL);
	}

	bool IsEmpty() const { return Count <= 0; }
	int GetCount() const { return Count; }
	int GetCapacity() const { return Capacity; }

	// Debug validation
	bool ValidateHeap() const
	{
		for (int i = 1; i <= Count; ++i)
		{
			if (!Nodes[i])
			{
				//Debug::Log("Null node at index %d (Count=%d)\n", i, Count);
				return false;
			}

			int left = i * 2;
			int right = left + 1;

			if (left <= Count && Nodes[left] && Comp(Nodes[left], Nodes[i]))
			{
				//Debug::Log("Heap property violated: left child %d < parent %d\n", left, i);
				return false;
			}

			if (right <= Count && Nodes[right] && Comp(Nodes[right], Nodes[i]))
			{
				//Debug::Log("Heap property violated: right child %d < parent %d\n", right, i);
				return false;
			}
		}
		return true;
	}

private:
	void HeapifyDown(int index)
	{
		while (true)
		{
			int smallest = index;
			int left = index * 2;
			int right = left + 1;

			if (left <= Count && left <= Capacity)
			{
				if (Nodes[left] && Nodes[smallest] && Comp(Nodes[left], Nodes[smallest]))
					smallest = left;
			}

			if (right <= Count && right <= Capacity)
			{
				if (Nodes[right] && Nodes[smallest] && Comp(Nodes[right], Nodes[smallest]))
					smallest = right;
			}

			if (smallest == index)
				break;

			std::swap(Nodes[index], Nodes[smallest]);
			index = smallest;
		}
	}

	bool Comp(T* p1, T* p2) const
	{
		if (!p1 || !p2)
			return p1 < p2;
		return Pr()(*p1, *p2);
	}

	void UpdatePointerBounds(T* pValue)
	{
		if (pValue > MaxPointer)
			MaxPointer = pValue;
		if (pValue < MinPointer)
			MinPointer = pValue;
	}

public:
	int Count;
	int Capacity;
	T** Nodes;
	T* MaxPointer;
	T* MinPointer;
};

template<typename T>
static SafePriorityQueueClass<T>* AllocPriorityQueue(int capacity, std::size_t nodeBytes)
{
	auto* q = static_cast<SafePriorityQueueClass<T>*>(operator new(sizeof(SafePriorityQueueClass<T>)));
	if (!q)
		return nullptr;

	q->Count = 0;
	q->Capacity = capacity;
	q->MaxPointer = (T*)0u;
	q->MinPointer = (T*)0xFFFFFFFFu;
	q->Nodes = static_cast<T**>(operator new(nodeBytes));

	// Vanilla: memset(Nodes, 0, sizeof(T*)*(Count+1)) — Count==0 at this point → 1 slot.
	// BUT the element-zero loop runs 0..Capacity inclusive (asm at 0x42A77B / 0x42A7CD).
	// SUSPECT: vanilla writes all (Capacity+1) slots here, not just (Count+1).
	//          The nodeBytes alloc is sized for (Capacity+1) pointers, so this is safe.
	std::memset(q->Nodes, 0, nodeBytes);

	return q;
}

enum AStarPostProcessType : int
{
	ASTAR_PASS_0 = 0x0,
	ASTAR_PASS_1 = 0x1,
	ASTAR_PASS_2 = 0x2,
};

struct AStarWorkPathStructNode
{
	CellClass** Cells;
	int CellLevel;
	AStarWorkPathStructNode* Prev;
};

struct __declspec(align(8)) AStarWorkPathStruct
{
	AStarWorkPathStructNode* Data;
	float MovementCost;
	float PathCost;
	int PathLength;
};

#pragma pack(push, 4)
struct AStarWorkPathStructHeap
{
	AStarWorkPathStruct Nodes[65536];
	DWORD ActiveCount;
};

struct AStarWorkPathStructDataHeap
{
	AStarWorkPathStructNode Nodes[131072];
	DWORD ActiveCount;
};

struct AStarQueueNodeHierarchical
{
	int BufferDelta;
	DWORD Index;
	float Score;
	int Number;
};
static_assert(sizeof(AStarQueueNodeHierarchical) == 16, "Invalid Size !");

struct PathType {
	CellStruct Start;                // Starting cell number.
	int Cost;                    // Accumulated terrain cost.
	int Length;                // Command string length.
	FacingType* Command;            // Pointer to command string.
	int field_10; //unused?
	CellStruct* Overlap;            // Pointer to overlap list
	CellStruct LastOverlap;        // stores position of last overlap
	CellStruct LastFixup;            // stores position of last overlap
};

struct PriorityQueueClass_AStarHierarchical
{
	int Count;
	int Capacity;
	AStarQueueNodeHierarchical** Heap;
	void* MaxNodePointer;
	void* MinNodePointer;

	void Heapify(bool shortitems = true) {
		JMP_THIS(0x42DCA0);
	}
};
#pragma pack(pop)

struct AStarClass_PassabilityData
{
	unsigned short Indices[500];
};

static_assert(sizeof(AStarClass_PassabilityData) == 0x3E8);
class AStarPathFinderClass
{
public:
	static COMPILETIMEEVAL reference<AStarPathFinderClass, 0x87E8B8> const Instance {};

	AStarPathFinderClass();
	~AStarPathFinderClass();

	static CellStruct* __fastcall Find_Some_Cell(CellStruct* retstr, CellStruct* cell, int count, int path) JMP_FAST(0x429780);
	
	//AStarClass__Get_Movement_Cost        00429830
	double Calc_Float(
		CellClass** arg0,
		CellClass** a3,
		int         a4,
		int         a5,
		int         a6) const;
	
	//	AStarClass__Create_Node        0042A460
	AStarWorkPathStruct* Calc_sqrt(
	AStarWorkPathStruct* parentNode,
	CellClass** a3,
	CellStruct* goalCell,
	float                a5);

	//	AStarClass__Cleanup        0042A5B0
	void Init();

	//	AStarClass_is_same_cost_Common        0042A690
	bool IsVisited(int index, bool useAlt) const;

	//	AStar_helper_facing        0042AA40
	int __fastcall CellStruct_helper_distance(CellStruct* a1, CellStruct* a2);

	//	AStarClass__Build_Final_Path_Regular        0042AA90
	PathType* Get_Path(AStarWorkPathStruct* work_path, FacingType* moves);

	// AStarClass__Reinit_Cost_Arrays        0042AC00
	void Reset(RectangleStruct* rect);

	//	AStarClass__Get_Occupier_Regular        0042B080
	FootClass* Get_Occupier(CellStruct* pos, int level) const;

	//	AStarClass__Post_Process_Cells        0042ACF
	bool Process_Paths(TechnoClass* techno);

	// Find_Adjacent_Cell_0        0042D490
	CellStruct* __fastcall tube_42D490(CellStruct* a1, CellStruct* a2, int Facing);

	// backported on Fake class
	// AStarClass__AStar_Find_Path_Hierarchical        0042C290
	
	//	AStarClass__Fixup_Final_Path_Regular        0042B420

	/*	
	AStarClass__AStar_Find_Path_Regular        00429A90

	AStarClass__Process_Final_Path_Regular        0042B210

	AStarClass__Optimize_Final_Path        0042B7F0
	AStarClass__Adjacent_Cell_Regular        0042BCA0
	AStarClass__Plot_Straight_Line_Regular        0042BE20
	void AStarClass__Clear_Pointers()     JMP_THIS(0x42C1C0);



	PathType* AStarClass__Find_Path(CellStruct* a2,
		CellStruct* dest,
		TechnoClass* a4,
		int* path,
		int max_count,
		MovementZone a7,
		ZoneType cellPath)       JMP_THIS(0x42C900);


		AStarClass__Init_Cell_Index_Sets        0042CCD0
		AStarClass__Is_Cell_Index_Set_Registered        0042CEB0
		AStarClass__Register_Cell_Index_Set        0042CF10
		AStarClass__Register_Cell_Index_Sets        0042CF80
	*/

	int AttemptPath(
		CellStruct* pFromMapCrd,
		CellStruct* pToMapCrd,
		TechnoClass* pTechno,
		bool bFromAlt,
		bool bToAlt,
		MovementZone nMovementZone = MovementZone::None)
	{
		JMP_THIS(0x42D170);
	}
	
public:
	char unknown_byte_0;
	bool FindBridgeDir;
	char unknown_byte_2;
	bool CanFindPath;
	float PathCostFactor;
	bool IsAlt;
	PROTECTED_PROPERTY(BYTE, padding_9_B[3]);
	AStarWorkPathStructDataHeap* PathNodeBuffer;
	AStarWorkPathStructHeap* PathQueueBuffer;
	SafePriorityQueueClass<AStarWorkPathStruct>* PathQueue;
	int* VisitCounts;
	int* AltVisitCounts;
	float* AltDistances;
	float* Distances;
	int SearchID;
	SpeedType FinderSpeedType;
	int StartLevel;
	int EndLevel;
	bool IsSearching;
	PROTECTED_PROPERTY(BYTE, padding_39_3B[3]);
	AStarPostProcessType FindMode;
	int* LevelVisitedMarkers[3];
	int* OpenSetMarkers[3];
	float* GCostArray[3];
	AStarQueueNodeHierarchical* HierarchyBuffer;
	PriorityQueueClass_AStarHierarchical* HierarchyQueue;
	int PathLength;
	CellStruct CellStructBuffer;
	DynamicVectorClass<CellStruct> ZoneIndices[3];
	AStarClass_PassabilityData PassabilityData[3];
	int PassabilityCounts[3];
};
static_assert(sizeof(AStarPathFinderClass) == 0xC80);

/*
bool Find_Path_Hierarchical(AStarPathFinderClass* pThis, CellStruct* from, CellStruct* to, MovementZone move , FootClass* pWho)
{
	double threat = 0.0;
	HouseClass* Owner = nullptr;
	bool Avaible = false;

	if (pWho) {
		threat = pWho->GetThreatAvoidanceCoefficient();
		Owner = pWho->Owner;
		Avaible = true;

		if (threat <= 0.00001)
		{
			Avaible = false;

		}

		int some_startIndex = 2;
		int some_startIndex2 = 2;
		while (2)
		{
			/// Clear the hierarchialqueue
			for (int i = 0; i < pThis->HierarchicalQueue->Count; ++i) {
				pThis->HierarchicalQueue->Heap[i - 1] = 0;
			}

			pThis->HierarchicalQueue->Count = 0;
			///

			const auto CellsArray_From = GlobalPassabilityDatas[MapClass::Instance->MapClass_zone_56D3F0(from)].data[some_startIndex];
			const auto CellsArray_To = GlobalPassabilityDatas[MapClass::Instance->MapClass_zone_56D3F0(to)].data[some_startIndex];

			auto some_startIndex3 = some_startIndex == 2 ? 0 : pThis->ints_40_costs[some_startIndex + 1];

			int* _ints_40_costs = pThis->ints_40_costs[some_startIndex];                                    // used by both
			int* _ints_4C_costs = pThis->ints_4C_costs[some_startIndex];                                    // used only by "Hierarchical"
			float* _HierarchicalCosts= pThis->HierarchicalCosts[some_startIndex];

			_ints_40_costs[CellsArray_From] = pThis->initedcount;
			_ints_40_costs[CellsArray_To] = pThis->initedcount;
			if (CellsArray_From == CellsArray_To) {
				if (!some_startIndex) {
					auto something = pThis->BufferForHierarchicalQueue;
					something->Index = CellsArray_From;
					something->Score = 0.0f;
				}

				pThis->somearray_BC[500 * some_startIndex] = CellsArray_From;
				pThis->maxvalues_field_C74[some_startIndex] = 0;

			}

			pThis->BufferForHierarchicalQueue->BufferDelta = -1;
			pThis->BufferForHierarchicalQueue->Index = CellsArray_From;
			pThis->BufferForHierarchicalQueue->Score = 0.0f;
			pThis->BufferForHierarchicalQueue->Number = 0;

			int HierarchicalQueue_count1 = pThis->HierarchicalQueue->Count + 1;
			int HierarchicalQueue_count2 = HierarchicalQueue_count1 >> 1;

			if (HierarchicalQueue_count1 < pThis->HierarchicalQueue->Capacity)
			{
				for (; HierarchicalQueue_count1 > 1; HierarchicalQueue_count2 >>= 1)
				{
					auto Elements = pThis->HierarchicalQueue->Heap;
					if (Elements[HierarchicalQueue_count2]->Score <= 0.0)
					{
						break;
					}

					Elements[HierarchicalQueue_count1] = Elements[HierarchicalQueue_count2];
				}

				pThis->HierarchicalQueue->Heap[HierarchicalQueue_count1] = pThis->BufferForHierarchicalQueue;
				++pThis->HierarchicalQueue->Count;

				if ((uintptr_t)pThis->BufferForHierarchicalQueue > (uintptr_t)pThis->HierarchicalQueue->MaxNodePointer)
				{
					pThis->HierarchicalQueue->MaxNodePointer = pThis->BufferForHierarchicalQueue;
				}

				if ((uintptr_t)pThis->BufferForHierarchicalQueue < (uintptr_t)pThis->HierarchicalQueue->MinNodePointer)
				{
					pThis->HierarchicalQueue->MinNodePointer = pThis->BufferForHierarchicalQueue;
				}

				bool initial__ = true;
				_ints_4C_costs[CellsArray_From] = pThis->initedcount;
				_HierarchicalCosts[CellsArray_From] = 0.0f;
				AStarQueueNodeHierarchical* someIdx_here = nullptr;

				// pop front ???
				if (pThis->HierarchicalQueue->Count)
				{
					someIdx_here = pThis->HierarchicalQueue->Heap[1];
					pThis->HierarchicalQueue->Heap[1] = pThis->HierarchicalQueue->Heap[pThis->HierarchicalQueue->Count];
					pThis->HierarchicalQueue->Heap[pThis->HierarchicalQueue->Count--] = 0;
					pThis->HierarchicalQueue->Heapify();
				}


				if (!someIdx_here)
				{
					return false;
				}

				const bool CellIndexesIsInvalid = pThis->CellIndexesVector[some_startIndex].Count == 0;

				int subzoneVectorIdx = some_startIndex >> 3;
				while (true)
				{
					if (someIdx_here->Index == CellsArray_To)
						break;

					const auto data = SubzoneTrackingStruct::Array[0].Items + subzoneVectorIdx;
					const auto data_Item = data->SubzoneConnections.Items + someIdx_here->Index;

					for (int i = data->SubzoneConnections.Count; i > 0; --i) {

					}
				}
			}
		}
	}
}
*/