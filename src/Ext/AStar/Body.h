#pragma once

#include <AStarClass.h>
#include <GeneralDefinitions.h>

class TechnoClass;
class NOVTABLE FakeAStarPathFinderClass : public AStarPathFinderClass
{
public:

	bool __Find_Path_Hierarchical(CellStruct* from, CellStruct* to, MovementZone mzone, FootClass* foot);
};