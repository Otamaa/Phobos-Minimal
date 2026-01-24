#pragma once

#include <Base/Always.h>
#include <CellClass.h>
#include <PriorityQueueClass.h>

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

class AStarPathFinderClass
{
public:
	static COMPILETIMEEVAL reference<AStarPathFinderClass, 0x87E8B8> const Instance {};

	AStarPathFinderClass() JMP_THIS(0x42A6D0);
	~AStarPathFinderClass() JMP_THIS(0x42A900);

	static CellStruct* __fastcall Find_Some_Cell(CellStruct* retstr, CellStruct* cell, int count, int path) JMP_FAST(0x429780);
	/*
	AStarClass__Get_Movement_Cost        00429830
	AStarClass__AStar_Find_Path_Regular        00429A90
	AStarClass__Create_Node        0042A460
	AStarClass__Cleanup        0042A5B0
	AStarClass_is_same_cost_Common        0042A690
	AStarClass__AStarClass        0042A6D0
	AStarClass__DTOR        0042A900
	AStar_helper_facing        0042AA40
	AStarClass__Build_Final_Path_Regular        0042AA90
	static_deinit_42ABF0        0042ABF0
	AStarClass__Reinit_Cost_Arrays        0042AC00
	AStarClass__Post_Process_Cells        0042ACF0
	AStarClass__Get_Occupier_Regular        0042B080
	CellStruct_totibarray_42B1C0        0042B1C0
	two_times_width_times_height_plus_four        0042B1F0
	AStarClass__Process_Final_Path_Regular        0042B210
	AStarClass__Fixup_Final_Path_Regular        0042B420
	AStarClass__Optimize_Final_Path        0042B7F0
	AStarClass__Adjacent_Cell_Regular        0042BCA0
	AStarClass__Plot_Straight_Line_Regular        0042BE20
	*/
	void AStarClass__Clear_Pointers()     JMP_THIS(0x42C1C0);
	//AStarClass__AStar_Find_Path_Hierarchical        0042C290
	PathType* AStarClass__Find_Path(CellStruct* a2,
		CellStruct* dest,
		TechnoClass* a4,
		int* path,
		int max_count,
		MovementZone a7,
		ZoneType cellPath)       JMP_THIS(0x42C900);
	/*AStarClass__Init_Cell_Index_Sets        0042CCD0
	AStarClass__Is_Cell_Index_Set_Registered        0042CEB0
	AStarClass__Register_Cell_Index_Set        0042CF10
	AStarClass__Register_Cell_Index_Sets        0042CF80
	AStarClass__Test_Cell_Walk        0042D170
	__thiscall Cell::Cell(short,short)        0042D470
	Find_Adjacent_Cell_0        0042D490
	Cell const __thiscall Cell::operator+(Cell const &)        0042D510
	*/
public:
	bool bool_0;                                            // char_0: always false
	bool bool_1;                                            // char_1: skips 004299D2 if false, bridge-related cost adjustment
	bool bool_2;                                            // char_2: always false  
	bool boolchar_3;                                        // char_3: blocks Process_Paths if false
	float float_4;                                          // float_4: used only by "Regular", multiplier for Calc_Float
	char bool_8;                                            // char_8: used as last arg in Can_Enter_Cell call
	AStarWorkPathStructDataHeap* WorkPathDataHeap;          // AStarStruct3_C: node data heap for regular pathfinding
	AStarWorkPathStructHeap* WorkPathHeap;                  // AStarStruct4_10: path struct heap for regular pathfinding
	TPriorityQueueClass<AStarWorkPathStruct>* RegularQueue; // AStarStruct1_14: priority queue for regular A*
	int* VisitedCellsAlt;                                   // pvoid18: visited tracking array (alternative bridge level)
	int* VisitedCells;                                      // pvoid1C: visited tracking array (ground level)
	float* MovementCosts;                                   // costarray2: movement costs for regular pathfinding
	float* MovementCostsAlt;                                // costarray1: alternative movement costs (bridge level)
	int initedcount;                                        // initedcount: epoch counter for visited array invalidation
	int ObjectSpeed;                                        // ObjectSpeed: unit speed, used in regular pathfinding
	int StartCellLevel;                                     // cellptr_30: start cell ground level (0=ground, 4=bridge)
	int EndCellLevel;                                       // cellptr_34: end cell ground level
	bool boolpathfind_38;                                   // pathfind_38: set by Test_Cell_Walk (0x42D170)
	AStarPostProcessType __PathsNeedProcessing;             // call_Process_Paths: post-processing mode for regular A*
	int* HierarchicalVisited[3];                            // field_40/ints_40_costs: visited arrays per hierarchy level
	int* HierarchicalOpenSet[3];                            // ints_4C_costs: open set tracking per hierarchy level
	float* HierarchicalCosts[3];                            // field_58: movement costs per hierarchy level
	AStarQueueNodeHierarchical* HierarchicalNodeBuffer;     // buffer4: 160KB buffer for hierarchical A* nodes (10000 * 16 bytes)
	PriorityQueueClass_AStarHierarchical* HierarchicalQueue;// AStarclass1_68: priority queue for hierarchical A*
	int RegularPathProgress;                                // dword_6C: current progress index in zone path
	CellStruct RegularOriginCell;                           // cell_70: origin cell for regular pathfinding
	DynamicVectorClass<CellStruct> BlockedCellPairs[3];     // Vectors: blocked cell pairs per hierarchy level
	short HierarchicalPath[1500];                           // somearray_BC: zone path storage (3 levels * 500 entries)
	int HierarchicalPathLength[3];                          // maxvalues_field_C74: path length per hierarchy level
};
static_assert(sizeof(AStarPathFinderClass) == 0xC80);

/*
bool Find_Path_Hierarchical(AStarPathFinderClass* pThis, CellStruct* from, CellStruct* to, MovementZone move , FootClass* pWho)
{
	double threat = 0.0;
	HouseClass* Owner = nullptr;
	bool Avaible = false;

	if (pWho) {
		threat = pWho->GetThreatAvoidance();
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