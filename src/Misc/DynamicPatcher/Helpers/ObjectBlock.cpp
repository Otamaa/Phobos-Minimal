#include "ObjectBlock.h"
#include "ObjectBlockContainer.h"

bool DynamicPatcher_Utilities::ObjectBlock::IsObjectInBlock(ObjectClass* pObject)
{
	CellStruct objectLoc = CellClass::Coord2Cell(pObject->GetCoords());
	int blockRange = Container->BlockRange;

	return abs(objectLoc.X - Center.X) <= blockRange
		&& abs(objectLoc.Y - Center.Y) <= blockRange;
}

std::list<ObjectBlock> DynamicPatcher_Utilities::ObjectBlock::GetNearBlocks()
{
	std::list<ObjectBlock> list {};

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			if (x == 0 && y == 0)
			{
				continue;
			}

			auto block = Container->GetBlock(ObjectBlockID(ID.X + x, ID.Y + y));
			list.push_back(block);
		}
	}

	return list;
}

void DynamicPatcher_Utilities::ObjectBlockContainer::AllocateBlocks()
{
	int xLength = Blocks[0].size();
	int yLength = Blocks[1].size();

	for (int x = 0; x < xLength; x++)
	{
		for (int y = 0; y < yLength; y++)
		{
			auto block = DynamicPatcher_Utilities::ObjectBlock((*this), ObjectBlockID(x - xLength / 2, y - yLength / 2));
			Blocks[x].Data[y] = block;
		}
	}
}

void DynamicPatcher_Utilities::ObjectBlockContainer::RefreshBlocks()
{
	ClearObjects();
	auto objects = *reinterpret_cast<DynamicVectorClass<ObjectClass*>*>(ObjectArrayPointer);

	for (auto const& pObject : objects)
	{
		CellStruct objectLoc = CellClass::Coord2Cell(pObject->GetCoords());
		ObjectBlockID id = GetIDBy(objectLoc);
		ObjectBlock block = GetBlockForce(id);
		block.AddObject(pObject);
	}

	CurrentFrame = Unsorted::CurrentFrame;
}

