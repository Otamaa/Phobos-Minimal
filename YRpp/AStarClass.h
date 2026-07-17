#pragma once

#include <Base/Always.h>
#include <CellClass.h>
#include <PriorityQueueClass.h>
#include <algorithm>
#include <functional>
#include <array>

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
		if (Count > Capacity)
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
	void HeapifyDown(int index) {
		while (true) {
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

	bool Comp(T* p1, T* p2) const {
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

enum AStarPostProcessType : int {
	ASTAR_PASS_0 = 0x0,
	ASTAR_PASS_1 = 0x1,
	ASTAR_PASS_2 = 0x2,
};

struct AStarWorkPathStructNode {
	CellClass** Cells;
	int CellLevel;
	AStarWorkPathStructNode* Prev;
};

struct __declspec(align(8)) AStarWorkPathStruct {
	AStarWorkPathStructNode* Data;
	float MovementCost;
	float PathCost;
	int PathLength;

	bool operator<(const AStarWorkPathStruct& other) const
	{
		return PathCost < other.PathCost;
	}
};

#pragma pack(push, 4)

struct AStarWorkPathStructHeap {
	std::array<AStarWorkPathStruct, 65536> Nodes;
	DWORD ActiveCount;
};

struct AStarWorkPathStructDataHeap {
	std::array<AStarWorkPathStructNode, 131072> Nodes;
	DWORD ActiveCount;
};

struct AStarQueueNodeHierarchical {
	int BufferDelta;
	DWORD Index;
	float Score;
	int Number;

	bool operator<(const AStarQueueNodeHierarchical& other) const {
		return Score < other.Score;
	}
};
static_assert(sizeof(AStarQueueNodeHierarchical) == 16, "Invalid Size !");

struct PathType {
	CellStruct Start;       // Starting cell number.
	int Cost;               // Accumulated terrain cost.
	int Length;             // Command string length.
	int* Command;    // Pointer to command string.
	int field_10;           // unused?
	CellStruct* Overlap;    // Pointer to overlap list
	CellStruct LastOverlap; // stores position of last overlap
	CellStruct LastFixup;   // stores position of last overlap
};

#pragma pack(pop)

struct AStarClass_PassabilityData
{
	std::array<unsigned short, 500> Indices;
};
static_assert(sizeof(AStarClass_PassabilityData) == 0x3E8);

class AStarPathFinderClass
{
public:
	static COMPILETIMEEVAL reference<AStarPathFinderClass, 0x87E8B8> const Instance {};

#ifdef NUKED
	AStarPathFinderClass();
	~AStarPathFinderClass();

	static CellStruct* __fastcall Find_Some_Cell(CellStruct* retstr, CellStruct* cell, int count, int path) JMP_FAST(0x429780);
	
	//AStarClass__Get_Movement_Cost        00429830
	double Calc_Float(
		CellClass** arg0,
		CellClass** a3,
		int         a4,
		int         a5,
		FootClass* a6) const;
	
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
	PathType* Get_Path(AStarWorkPathStruct* work_path, int* moves);

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
	bool __Find_Path_Hierarchical(CellStruct* from, CellStruct* to, MovementZone mzone, FootClass* foot);

	//	AStarClass__Fixup_Final_Path_Regular        0042B420
	int Tube_Crap(
		FootClass* techno,
		int* dirArray,
		int* levelArray,
		int         pathLen,
		int         lookAhead,
		CellStruct* posPtr);

	//	AStarClass__Process_Final_Path_Regular        0042B210
	void Process_Moves(PathType* path, FootClass* techno);

	//AStarClass__Adjacent_Cell_Regular        0042BCA0
	void Adj_Cell(
		int* dirArray,
		int         startIdx,
		int         minIdx,
		int* outIdx,
		CellStruct * posPtr);

	// AStarClass__Is_Cell_Index_Set_Registered        0042CEB0
	bool Is_Cell_In_Vector(unsigned int a, unsigned int b, int vectorNum) const;

	//		AStarClass__Register_Cell_Index_Sets        0042CF80
	void UpdateZoneVector(unsigned int zoneValue, int vectorIdx);

	//		AStarClass__Register_Cell_Index_Set        0042CF10
	void Add_Cell_To_Vector(unsigned int a, unsigned int b, int vectorNum);

	//	AStarClass__Plot_Straight_Line_Regular        0042BE20
	bool Generate_Moves(
		int*		moves,
		int         capacity,
		CellStruct* startCell,
		CellStruct* delta,
		FootClass* techno,
		int* levelPtr,
		bool        tolerateThreats);

	//void AStarClass__Clear_Pointers()     JMP_THIS(0x42C1C0);
	void AllocZoneArrays();

	//AStarClass__AStar_Find_Path_Regular        00429A90
	PathType* Find_Path_Regular(
		CellStruct* start,
		CellStruct* dest,
		FootClass* techno,
		int* moves,
		int          maxCount,
		bool         useHierarchical);

	//AStarClass__Optimize_Final_Path        0042B7F0
	void Calc_Moves(PathType* path, FootClass* techno);

	//PathType* AStarClass__Find_Path(CellStruct* a2,
	//	CellStruct* dest,
	//	TechnoClass* a4,
	//	int* path,
	//	int max_count,
	//	MovementZone a7,
	//	ZoneType cellPath)       JMP_THIS(0x42C900);
	PathType* Find_Path(
		CellStruct* start,
		CellStruct* dest,
		FootClass* techno,
		int* moves,
		int          maxCount,
		MovementZone          mzoneOverride,
		AStarPostProcessType          findModeOverride);

	//AStarClass__Init_Cell_Index_Sets        0042CCD0
	void Fill_DVector(FootClass* techno);

	//int AttemptPath(
	//	CellStruct* pFromMapCrd,
	//	CellStruct* pToMapCrd,
	//	TechnoClass* pTechno,
	//	bool bFromAlt,
	//	bool bToAlt,
	//	MovementZone nMovementZone = MovementZone::None)
	//{
	//	JMP_THIS(0x42D170);
	//}
	unsigned int Attempt(
	CellStruct* startPos,
	CellStruct* destPos,
	FootClass* foot,
	bool        bridge1,
	bool        bridge2,
	MovementZone         mzone = MovementZone::None);
#endif

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
	SafePriorityQueueClass<AStarQueueNodeHierarchical>* HierarchyQueue;
	int PathLength;
	CellStruct CellStructBuffer;
	DynamicVectorClass<CellStruct> ZoneIndices[3];
	AStarClass_PassabilityData PassabilityData[3];
	int PassabilityCounts[3];
};
static_assert(sizeof(AStarPathFinderClass) == 0xC80);