#pragma once

#include <AStarClass.h>
#include <GeneralDefinitions.h>

#pragma once

#include <GeneralStructures.h>
#include <CellClass.h>

#include <array>
#include <bit>
#include <cstddef>
#include <memory>
#include <queue>
#include <vector>

// ===========================================================================
// LAYOUT CONTRACT — read before touching anything below.
// ---------------------------------------------------------------------------
// The single instance lives at 0x87E8B8. Vanilla code outside this class pokes
// the scalar fields (CanFindPath, PathCostFactor, FindBridgeDir, IsAlt, ...)
// at hardcoded offsets, so every offset must stay put. That is possible
// because:
//   * unique_ptr<T> / unique_ptr<T[]>       == 4 bytes (EBO on default_delete)
//   * array<unique_ptr<T[]>, 3>             == 12 bytes (same as T*[3])
//   * std::vector is 12 or 16 bytes depending on _ITERATOR_DEBUG_LEVEL, so
//     ZoneIndexList pads itself to the old DynamicVectorClass<CellStruct>
//     footprint (0x18) and everything after it keeps its offset.
// The static_asserts at the bottom of this file enforce all of it.
//
// HARD PREREQUISITE: every AStarClass method the game calls (the 0x42xxxx list
// on each declaration) must be hooked, *including the ctor and dtor*. Any
// un-hooked vanilla method that walks PathQueue / ZoneIndices internals will
// read the new types as the old ones.
// ===========================================================================

// -----------------------------------------------------------------------
// MinHeapCompare — inverts operator< so std::priority_queue behaves as a
// MIN-heap.
//
// GOTCHA: std::priority_queue is a MAX-heap under std::less. The old
// SafePriorityQueueClass<T, std::less<T>> bubbled up on Comp(child, parent),
// i.e. it was a MIN-heap. A* needs the lowest PathCost / Score on top.
// Forgetting to invert here silently turns the search into a
// worst-first flood fill.
// -----------------------------------------------------------------------
template<typename T>
struct MinHeapCompare
{
	bool operator()(const T* pLeft, const T* pRight) const
	{
		if (!pLeft || !pRight)
			return pRight < pLeft; // preserve the old null ordering

		return *pRight < *pLeft;   // inverted -> min-heap
	}
};

// -----------------------------------------------------------------------
// MinHeap<T> — std::priority_queue adapter keeping the vanilla call shape.
//
// Private inheritance is deliberate: it gives access to the protected
// container `c` for reserve() / clear() / size(), which the adaptor does not
// expose.
//
// GOTCHA: Push() keeps a capacity check even though std::priority_queue is
// unbounded. This is NOT about the queue. __Find_Path_Hierarchical
// bump-allocates nodes out of the fixed 10000-entry HierarchyBuffer arena and
// relies on Push() returning false to stop the bump. Remove the check and the
// arena walks off its end.
// -----------------------------------------------------------------------
template<typename T>
class MinHeap : private std::priority_queue<T*, std::vector<T*>, MinHeapCompare<T>>
{
	using Base = std::priority_queue<T*, std::vector<T*>, MinHeapCompare<T>>;

public:
	explicit MinHeap(std::size_t capacity)
		: Base {}
		, Capacity { capacity }
	{
		this->c.reserve(capacity + 1);
	}

	MinHeap(const MinHeap&) = delete;
	MinHeap& operator=(const MinHeap&) = delete;

	bool Push(T* pValue)
	{
		if (!pValue)
			return false;

		if (this->c.size() >= this->Capacity)
			return false;

		this->push(pValue);
		return true;
	}

	T* Pop()
	{
		if (this->c.empty())
			return nullptr;

		T* const pResult = this->top();
		this->pop();
		return pResult;
	}

	T* Top() const
	{
		return this->c.empty() ? nullptr : this->top();
	}

	// DIFF: keeps the reserved capacity, unlike the old null-writing loop.
	void Clear()
	{
		this->c.clear();
	}

	bool IsEmpty() const { return this->c.empty(); }
	std::size_t Size() const { return this->c.size(); }
	std::size_t GetCapacity() const { return this->Capacity; }

private:
	std::size_t Capacity;
};

// -----------------------------------------------------------------------
// ZoneIndexList — std::vector<CellStruct> padded to the vanilla
// DynamicVectorClass<CellStruct> footprint (0x18).
//
// The pad is what keeps PassabilityData / PassabilityCounts on their
// hardcoded offsets. Do not remove it, and do not let it go negative.
// -----------------------------------------------------------------------
class ZoneIndexList
{
public:
	void clear() { this->Items.clear(); }
	void push_back(const CellStruct& value) { this->Items.push_back(value); }

	int Size() const { return static_cast<int>(this->Items.size()); }
	bool IsEmpty() const { return this->Items.empty(); }

	CellStruct& operator[](int index) { return this->Items[index]; }
	const CellStruct& operator[](int index) const { return this->Items[index]; }

	auto begin() const { return this->Items.begin(); }
	auto end() const { return this->Items.end(); }

	// Vanilla packs (hi << 16) | lo into the CellStruct and compares the whole
	// thing as one int. Reproduced verbatim, just without the reinterpret_cast.
	bool Contains(int packedKey) const
	{
		for (const CellStruct& cell : this->Items)
		{
			if (std::bit_cast<int>(cell) == packedKey)
				return true;
		}

		return false;
	}

private:

	std::vector<CellStruct> Items {};
};

// -----------------------------------------------------------------------
// PhobosAStarPathFinderClass
// -----------------------------------------------------------------------
class PhobosAStarPathFinderClass
{
public:
	using PathQueueType = MinHeap<AStarWorkPathStruct>;
	using HierarchyQueueType = MinHeap<AStarQueueNodeHierarchical>;

	static COMPILETIMEEVAL std::size_t PathQueueCapacity = 0x10000;
	static COMPILETIMEEVAL std::size_t HierarchyCapacity = 0x2710;

	PhobosAStarPathFinderClass();
	~PhobosAStarPathFinderClass();

	PhobosAStarPathFinderClass(const PhobosAStarPathFinderClass&) = delete;
	PhobosAStarPathFinderClass& operator=(const PhobosAStarPathFinderClass&) = delete;

	//static CellStruct* __fastcall Find_Some_Cell(CellStruct* retstr, CellStruct* cell, int count, int path) JMP_FAST(0x429780);

	// AStarClass__Get_Movement_Cost        00429830
	double Calc_Float(CellClass** arg0, CellClass** a3, int a4, Move a5, FootClass* a6) const;

	// AStarClass__Create_Node        0042A460
	AStarWorkPathStruct* Calc_sqrt(AStarWorkPathStruct* parentNode, CellClass** a3, CellStruct* goalCell, float a5);

	// AStarClass__Cleanup        0042A5B0
	void Init();

	// AStarClass_is_same_cost_Common        0042A690
	bool IsVisited(int index, bool useAlt) const;

	// AStar_helper_facing        0042AA40
	int __fastcall CellStruct_helper_distance(CellStruct* a1, CellStruct* a2);

	// AStarClass__Build_Final_Path_Regular        0042AA90
	PathType* Get_Path(AStarWorkPathStruct* work_path, int* moves);

	// AStarClass__Reinit_Cost_Arrays        0042AC00
	void Reset(RectangleStruct* rect);

	// AStarClass__Get_Occupier_Regular        0042B080
	FootClass* Get_Occupier(CellStruct* pos, int level) const;

	// AStarClass__Post_Process_Cells        0042ACF0
	bool Process_Paths(TechnoClass* techno);

	// Find_Adjacent_Cell_0        0042D490
	CellStruct* __fastcall tube_42D490(CellStruct* a1, CellStruct* a2, int Facing);

	// AStarClass__AStar_Find_Path_Hierarchical        0042C290
	bool __Find_Path_Hierarchical(CellStruct* from, CellStruct* to, MovementZone mzone, FootClass* foot);

	// AStarClass__Fixup_Final_Path_Regular        0042B420
	int Tube_Crap(FootClass* techno, int* dirArray, int* levelArray, int pathLen, int lookAhead, CellStruct* posPtr);

	// AStarClass__Process_Final_Path_Regular        0042B210
	void Process_Moves(PathType* path, FootClass* techno);

	// AStarClass__Adjacent_Cell_Regular        0042BCA0
	void Adj_Cell(int* dirArray, int startIdx, int minIdx, int* outIdx, CellStruct* posPtr);

	// AStarClass__Is_Cell_Index_Set_Registered        0042CEB0
	bool Is_Cell_In_Vector(unsigned int a, unsigned int b, int vectorNum) const;

	// AStarClass__Register_Cell_Index_Sets        0042CF80
	void UpdateZoneVector(unsigned int zoneValue, int vectorIdx);

	// AStarClass__Register_Cell_Index_Set        0042CF10
	void Add_Cell_To_Vector(unsigned int a, unsigned int b, int vectorNum);

	// AStarClass__Plot_Straight_Line_Regular        0042BE20
	bool Generate_Moves(int* moves, int capacity, CellStruct* startCell, CellStruct* delta,
		FootClass* techno, int* levelPtr, bool tolerateThreats);

	// AStarClass__Clear_Pointers        0042C1C0
	void AllocZoneArrays(AStarPathFinderClass* pOriginal);

	// AStarClass__AStar_Find_Path_Regular        00429A90
	PathType* Find_Path_Regular(CellStruct* start, CellStruct* dest, FootClass* techno,
		int* moves, int maxCount, bool useHierarchical);

	// AStarClass__Optimize_Final_Path        0042B7F0
	void Calc_Moves(PathType* path, FootClass* techno);

	// AStarClass__Find_Path        0042C900
	PathType* Find_Path(CellStruct* start, CellStruct* dest, FootClass* techno, int* moves,
		int maxCount, MovementZone mzoneOverride, AStarPostProcessType findModeOverride);

	// AStarClass__Init_Cell_Index_Sets        0042CCD0
	void Fill_DVector(FootClass* techno);

	// AStarClass__AttemptPath        0042D170
	unsigned int Attempt(CellStruct* startPos, CellStruct* destPos, FootClass* foot,
		bool bridge1, bool bridge2, MovementZone mzone = MovementZone::None);

	//signed int __fastcall astarmap_5889F0_getmovezone(int val1, int val2)
public:
	// --- 0x00 : scalars poked by vanilla code outside this class -----------
	char unknown_byte_0;
	bool FindBridgeDir;
	char unknown_byte_2;
	bool CanFindPath;
	float PathCostFactor;
	bool IsAlt;

	// --- 0x0C : owned buffers ---------------------------------------------
	std::unique_ptr<AStarWorkPathStructDataHeap> PathNodeBuffer;
	std::unique_ptr<AStarWorkPathStructHeap> PathQueueBuffer;
	std::unique_ptr<PathQueueType> PathQueue;

	// --- 0x18 : per-cell scratch, sized by Reset() -------------------------
	std::unique_ptr<int[]> VisitCounts;
	std::unique_ptr<int[]> AltVisitCounts;
	std::unique_ptr<float[]> AltDistances;
	std::unique_ptr<float[]> Distances;

	// --- 0x28 -------------------------------------------------------------
	int SearchID;
	SpeedType FinderSpeedType;
	int StartLevel;
	int EndLevel;
	bool IsSearching;

	AStarPostProcessType FindMode;

	// --- 0x40 : per-level scratch, sized by AllocZoneArrays() --------------
	std::array<std::unique_ptr<int[]>, 3> LevelVisitedMarkers;
	std::array<std::unique_ptr<int[]>, 3> OpenSetMarkers;
	std::array<std::unique_ptr<float[]>, 3> GCostArray;

	// --- 0x64 -------------------------------------------------------------
	std::unique_ptr<AStarQueueNodeHierarchical[]> HierarchyBuffer;
	std::unique_ptr<HierarchyQueueType> HierarchyQueue;
	int PathLength;
	CellStruct CellStructBuffer;

	// --- 0x74 -------------------------------------------------------------
	std::array<ZoneIndexList, 3> ZoneIndices;

	// --- 0xBC -------------------------------------------------------------
	std::array<AStarClass_PassabilityData, 3> PassabilityData;

	// --- 0xC74 ------------------------------------------------------------
	std::array<int, 3> PassabilityCounts;

public:
	static PhobosAStarPathFinderClass Instance;
};

class FakeAStarPathFinderClass : public AStarPathFinderClass
{
public:
	static COMPILETIMEEVAL reference<FakeAStarPathFinderClass, 0x87E8B8> const Instance {};

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

	double Calc_Float(CellClass** arg0, CellClass** a3, int a4, Move a5, FootClass* a6) const;
	PathType* Find_Path_Regular(CellStruct* start, CellStruct* dest, FootClass* techno,
		int* moves, int maxCount, bool useHierarchical);
	void Reset(RectangleStruct* rect);
	void AllocZoneArrays();
	PathType* Find_Path(CellStruct* start, CellStruct* dest, FootClass* techno, int* moves,
		int maxCount, MovementZone mzoneOverride, AStarPostProcessType findModeOverride);
	unsigned int Attempt(CellStruct* startPos, CellStruct* destPos, FootClass* foot,
		bool bridge1, bool bridge2, MovementZone mzone = MovementZone::None);
};