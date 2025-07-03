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

	static CellStruct* __fastcall Find_Some_Cell(CellStruct* retstr, CellStruct* cell, int count, int path) JMP_STD(0x429780);
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

	// Reverse-engineered hierarchical pathfinding function
	// Based on assembly at 0x42C290
	uint Find_Path_Hierarchical(CellStruct* from, CellStruct* to, int movementZone, FootClass* pFoot);

	// Member variables inferred from assembly
	int initedcount;                                    // +0x28, initialization counter
	int* ints_40_costs[3];                             // +0x40, cost arrays for 3 levels
	int* ints_4C_costs[3];                             // +0x4C, additional cost arrays  
	float* HierarchicalCosts[3];                       // +0x58, hierarchical cost arrays
	AStarQueueNodeHierarchical* BufferForHierarchicalQueue; // +0x64, buffer for queue nodes
	PriorityQueueClass_AStarHierarchical* HierarchicalQueue; // +0x68, priority queue
	DynamicVectorClass<int> CellIndexesVector[3];      // +0x74, cell index vectors
	WORD somearray_BC[1500];                           // +0xBC, path storage (500 * 3 levels)
	int maxvalues_field_C74[3];                        // +0xC74, max values for each level

public:
	bool bool_0;                                            // always false
	bool bool_1;                                            // skips 004299D2 if false
	bool bool_2;                                            // always false
	bool boolchar_3;                                        // blocks Process_Paths if false
	float float_4;                                          // used only by "Regular"
	char bool_8;                                            // used as last arg in Can_Enter_Cell call
	AStarWorkPathStructDataHeap* WorkPathDataHeap;                                 // used only by "Regular"
	AStarWorkPathStructHeap* WorkPathHeap;                                     // used only by "Regular"
	TPriorityQueueClass<AStarWorkPathStruct>* RegularQueue;    // confirmish
	int* celllevel_costarray2_alt;                                // used only by "Regular"
	int* celllevel_costarray1;                                // used only by "Regular"
	float* MovementCosts;                             // used only by "Regular"
	float* MovementCostsAlt;                             // used only by "Regular"
	int ObjectSpeed;                                              // confirmish, used only by "Regular"
	int StartCellLevel;                                     // confirmish, used only by "Regular"
	int EndCellLevel;                                       // confirmish, used only by "Regular"
	bool boolpathfind_38;                                   // 0042D170 was called
	AStarPostProcessType __PathsNeedProcessing;                         // used only by "Regular"
	int dword_6C;                                           // some index, used only by "Regular"
	CellStruct __OriginCell;                                      // some index, used only by "Regular"
};
static_assert(sizeof(AStarPathFinderClass) == 0xC80);