#pragma once

#include <Base/Always.h>
#include <CellClass.h>
#include "PriorityQueueClass.h"

//ToDo : YR Changes
typedef struct WorkingPathNodeData
{
	CellClass** Cells;
	int CellLevel;
	WorkingPathNodeData* Prev;
} WorkingPathNodeData;

typedef struct WorkingPathNode
{
	WorkingPathNodeData* Data;
	float MovementCost;
	float PathCost;
	int PathLength;
} WorkingPathNode;

typedef struct QueueNodeHierarchical
{
	int BufferDelta;
	int Index;
	float Score;
	int Number;
} QueueNodeRegular;

typedef struct PathType
{
	CellStruct Start;                 // Starting cell number.
	int Cost;                   // Accumulated terrain cost.
	int Length;                 // Command string length.
	FacingType* Command;        // Pointer to command string.
	int field_10;
	unsigned long* Overlap;     // Pointer to overlap list
	CellStruct LastOverlap;           // stores position of last overlap
	CellStruct LastFixup;
} PathType;

class AStarPathFinderClass
{
public:
	AStarPathFinderClass() JMP_THIS(0x42A6D0);
	~AStarPathFinderClass() JMP_THIS(0x42A900);

	// AStarClass_Get_Adj_Cell_41A570 // 0041A570
	float Get_Movement_Cost(CellClass** from, CellClass** to, bool use_alt, Move move, FootClass* object) JMP_THIS(0x429830);
	PathType* Find_Path_Regular(CellStruct* from, CellStruct* to, FootClass* object, FacingType* final_moves, int max_loops, bool a6 = false) JMP_THIS(0x429A90);
	// AStarClass_Process_Working_Path_41B250 // 0041B250
	// AStarClass_clear?_41B3A0 // 0041B3A0
	// AStarClass_operator_eq_?_41B480 // 0041B480
	// AStarClass_41B830 // 0041B830
	PathType* Build_Final_Path(WorkingPathNode* nodes, FacingType* moves) JMP_THIS(0x42AA90);
	// AStarClass_Reset?_41B9F0 // 0041B9F0
	// AStarClass_Process_Paths_41BAE0 // 0041BAE0
	// AStarClass_Get_Occupier_41BE60 // 0041BE60
	// AStarClass_41BFA0 // 0041BFA0
	// AStarClass_41BFD0 // 0041BFD0
	// AStarClass_Process_Moves_41BFF0 // 0041BFF0
	int Fixup_Path(FootClass* object, FacingType* final_moves, int* overlap, int number1, int number2, CellStruct* a7) JMP_THIS(0x42B420);
	void Optimize_Final_Path(PathType* path, FootClass* object) JMP_THIS(0x42B7F0);
	void Adjacent_Cell(FacingType* moves, int a2, int a3, int* a4, CellStruct* cell_out) JMP_THIS(0x42BCA0);
	// AStarClass_Generate_Moves_41CC00 // 0041CC00
	// AStarClass_Reinit_Buffers_41CFA0 // 0041CFA0
	bool Find_Path_Hierarchical(CellStruct* from, CellStruct* to, MovementZone mzone, FootClass* object) JMP_THIS(0x42C290);
	PathType* Find_Path(CellStruct* from, CellStruct* to, FootClass* object, FacingType* final_moves, int max_loops, MovementZone mzone, bool a7 = false) JMP_THIS(0x42C900);
	// AStarClass_Fill_DVector_41DAA0 // 0041DAA0
	// AStarClass_Is_Cell_In_Vector_41DC80 // 0041DC80
	// AStarClass_Add_Cell_To_Vector_41DCE0 // 0041DCE0
	// AStarClass_41DD50 // 0041DD50
	// AStarClass_Attempt_41DF40 // 0041DF40
	// AStarClass_Get_Tunnel_Coords?_41E260 // 0041E260

public:
	enum PathEnum
	{
		PATH_0,
		PATH_1,
		PATH_2,
	};

	bool bool_0;                                            // always false
	bool bool_1;                                            // skips 004299D2 if false
	bool bool_2;                                            // always false
	bool boolchar_3;                                        // blocks Process_Paths if false
	float float_4;                                          // used only by "Regular"
	char bool_8;                                            // used as last arg in Can_Enter_Cell call
	void* WorkPathDataHeap;                                 // used only by "Regular"
	void* WorkPathHeap;                                     // used only by "Regular"
	TPriorityQueueClass<WorkingPathNode>* RegularQueue;    // confirmish
	void* celllevel_pvoid18;                                // used only by "Regular"
	void* celllevel_pvoid1C;                                // used only by "Regular"
	void* celllevel_costarray1;                             // used only by "Regular"
	void* celllevel_costarray2;                             // used only by "Regular"
	int initedcount;                                        // used by both
	int Speed;                                              // confirmish, used only by "Regular"
	int StartCellLevel;                                     // confirmish, used only by "Regular"
	int EndCellLevel;                                       // confirmish, used only by "Regular"
	bool boolpathfind_38;                                   // 0042D170 was called
	PathEnum __PathsNeedProcessing;                         // used only by "Regular"
	int* pointers_40[3];                                    // used by both
	int* pointers_4C[3];                                    // used only by "Hierarchical"
	float* pointers_58[3];                                  // used only by "Hierarchical"
	void* BufferForHierarchicalQueue;                       // some index, used only by "Hierarchical"
	TPriorityQueueClass<QueueNodeHierarchical>* HierarchicalQueue; // confirmish
	int dword_6C;                                           // some index, used only by "Regular"
	CellStruct __OriginCell;                                      // some index, used only by "Regular"
	DynamicVectorClass<CellStruct> CellIndexesVector[3];          // some index, used only by "Hierarchical"
	short somearray_BC[1500];                               // some indexes, used by both
	int maxvalues_field_C74[3];                             // some index, used only by "Hierarchical"
};