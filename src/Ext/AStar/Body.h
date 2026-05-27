#pragma once

#include <AStarClass.h>
#include <GeneralDefinitions.h>

class TechnoClass;
class NOVTABLE FakeAStarPathFinderClass : public AStarPathFinderClass
{
public:
	PathType* __AStarClass__Find_Path(CellStruct* a2,
		CellStruct* dest,
		TechnoClass* a4,
		int* path,
		int max_count,
		MovementZone a7,
		ZoneType cellPath);

	bool __Find_Path_Hierarchical(CellStruct* from, CellStruct* to, MovementZone mzone, FootClass* foot);
};