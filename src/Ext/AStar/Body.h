#pragma once
#include <Base/Always.h>

#include <PriorityQueueClass.h>

#include <CellClass.h>
#include <CellStruct.h>
#include <CoordStruct.h>
#include <DynamicVectorClass.h>

#include <vector>

class AStarClass
{
public:

	struct PathFinderData
	{
		CellStruct StartCell;
		int TotalDistance;
		int PathLength;
		int* Directions;
		int unknown_int_10;
		int* Levels;
		CellStruct unknown_cellstruct_18;
		int unknown_int_1C;
	};

	struct PathNode
	{
		const CellClass* const* CellItems;
		int Level;
		PathNode* PreviousNode;
	};

	struct PathNodeBuffer
	{
		PathNode Nodes[131072];
		int Count;
	};

	struct PassabilityData
	{
		unsigned short Indices[500];
	};

	struct PathQueueNode
	{
		PathNode* NodeData;
		float PathCost;
		float TotalCost;
		int NodeCount;
	};

	struct PathQueueNodeBuffer
	{
		PathQueueNode Nodes[131072]; // [65536] => [131072]
		int Count;
	};

	struct HierarchicalNode
	{
		int PreviousNodeIndex;
		int SubzoneIndex;
		float Cost;
		int Count;
	};

	struct HierarchicalNodeBuffer
	{
		HierarchicalNode Nodes[10000];
	};

	DEFINE_REFERENCE(AStarClass, Instance, 0x87E8B8u)
	DEFINE_REFERENCE(AStarClass::PathFinderData, PathData, 0x89A2D8u)
	DEFINE_REFERENCE(const int, MapSides, 0x89C2DCu)
	DEFINE_REFERENCE(const CellClass*, InvalidCell, 0x89C2E0u)

	DEFINE_ARRAY_REFERENCE(const int, 8, BridgeDir0OffsetIndexes, 0x7E3710)
	DEFINE_ARRAY_REFERENCE(const int, 8, BridgeDir1OffsetIndexes, 0x7E3730)
	DEFINE_ARRAY_REFERENCE(const int, 9, BridgeDirOffsets, 0x7E3750)
	DEFINE_ARRAY_REFERENCE(const int, 8, DirCellOffsets, 0x7E3774)
	DEFINE_ARRAY_REFERENCE(const float, 8, PassabilityCoefficients, 0x7E3794)
	DEFINE_ARRAY_REFERENCE(const float, 8, MoveCosts, 0x81870C)
	DEFINE_ARRAY_REFERENCE(const float, 8, DirPathCosts, 0x81872C)
	DEFINE_ARRAY_REFERENCE(const int, 8, DirSides, 0x89A304)

	static constexpr bool EnableRectilinear = true;
	static std::vector<CellStruct> LineCells;
	static std::vector<unsigned short> StraightSubzones[3];
	static std::vector<int> IsStraightFlag[3];
	static bool ContainersInit;

	AStarClass()
		JMP_THIS(0x42A6D0);

	~AStarClass()
		JMP_THIS(0x42A900);

	void CleanUp()
		JMP_THIS(0x42A5B0);

	void ClearPassability()
		JMP_THIS(0x42C1C0);

	void ReinitCostArrays(
		const RectangleStruct* const pMapRect
	) JMP_THIS(0x42AC00);

	void RecordCellIndex(
		const FootClass* const pFoot
	) JMP_THIS(0x42CCD0);

	void RegisterCellIndex(
		const int index,
		const int dataIndex
	) JMP_THIS(0x42CF80);

	// static CellStruct* __fastcall NextPathCell(CellStruct* pBuffer, CellStruct* pCurrent, int dir) JMP_STD(0x42D490);
	static CellStruct NextPathCell(
		const CellStruct cell,
		const int dir
	);

	PathFinderData* FindPath(
		const CellStruct* const pStart,
		const CellStruct* const pEnd,
		const FootClass* const pFoot,
		int* const pDirs,
		int maxSteps,
		MovementZone movementZone,
		const int mode
	) JMP_THIS(0x42C900);

	int AttemptPath(
		const CellStruct* const pStart,
		const CellStruct* const pEnd,
		const FootClass* const pFoot,
		const bool checkStartBridge,
		const bool checkEndBridge,
		MovementZone movementZone
	) JMP_THIS(0x42D170);

	PathFinderData* FindRegularPath(
		const CellStruct* const pStart,
		const CellStruct* const pEnd,
		const FootClass* const pFoot,
		int* const pDirs,
		int maxSteps,
		const bool useHierarchical
	); // JMP_THIS(0x429A90)

	bool FindHierarchicalPath(
		const CellStruct* const pStart,
		const CellStruct* const pEnd,
		const MovementZone movementZone,
		const FootClass* const pFoot
	); // JMP_THIS(0x42C290)

	PathQueueNode* CreatePathNode(
		const PathQueueNode* const pPrevNode,
		const CellClass* const* const pCellPtr,
		const CellStruct* const pCoords,
		const float cost
	) JMP_THIS(0x42A460);

	void PostProcessCells(
		const FootClass* const pFoot
	) JMP_THIS(0x42ACF0);

	FootClass* GetOccupier(
		const CellStruct* const pCell,
		const int level
	) const JMP_THIS(0x42B080);

	double CalculateMoveCost(
		const CellClass* const* const pFromCellPtr,
		const CellClass* const* const pToCellPtr,
		const bool isAlternate,
		const Move moveType,
		const FootClass* const pFoot
	) const; // JMP_THIS(0x429830)

	PathFinderData* BuildFinalPath(
		PathQueueNode* const pEndNode,
		int* const pDirs
	) const JMP_THIS(0x42AA90);

	void ProcessFinalPath(
		PathFinderData* const pPath,
		const FootClass* const pFoot
	) const; // JMP_THIS(0x42B210)

	int AdjustFinalPath(
		const FootClass* const pFoot,
		int* const pDirs,
		const int* const pLevels,
		const int steps,
		int offset,
		CellStruct* const pCurrent
	) const; // JMP_THIS(0x42B420)

	void OptimizeFinalPath(
		PathFinderData* const pPath,
		const FootClass* const pFoot
	) const; // JMP_THIS(0x42B7F0)

	void GetFinalStepCell(
		const int* const pDirs,
		const int segmentEndIdx,
		const int segmentStartIdx,
		int* const pOutIdx,
		CellStruct* const pAdjacent
	) const; // JMP_THIS(0x42BCA0)

	bool PlotStraightPath(
		int* const pDirs,
		const int maxLength,
		const CellStruct* const pCurrent,
		const CellStruct* const pVector,
		const FootClass* const pFoot,
		const int curLevel,
		const bool allowThreats
	) const; // JMP_THIS(0x42BE20)


	char unknown_byte_0; // false
	bool FindBridgeDir; // false
	char unknown_byte_2; // false
	bool CanFindPath; // true
	float PathCostFactor; // 1.0
	bool IsAlt; // true
	PathNodeBuffer* PathNodeBuffer;
	PathQueueNodeBuffer* PathQueueBuffer;

	TPriorityQueueClass<PathQueueNode>* PathQueue; // Count = 65537 => 131073

	int* VisitCounts;
	int* AltVisitCounts;
	float* AltDistances;
	float* Distances;
	int SearchID;
	SpeedType FinderSpeedType;
	int StartLevel;
	int EndLevel;
	bool IsSearching; // true
	int FindMode; // 0

	int* LevelVisitedMarkers[3];
	int* OpenSetMarkers[3];
	float* GCostArray[3];

	HierarchicalNodeBuffer* HierarchyBuffer;

	TPriorityQueueClass<HierarchicalNode>* HierarchyQueue; // Count = 10001

	int PathLength;
	CellStruct CellStructBuffer;


	DynamicVectorClass<unsigned int> ZoneIndices[3];
	PassabilityData PassabilityData[3];
	int PassabilityCounts[3];
};
