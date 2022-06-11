#pragma once
#include <functional>
#include <assert.h>
#include <ArrayClasses.h>
#include <CellClass.h>
#include <ObjectClass.h>
#include <vector>
#include "ObjectBlock.h"

namespace DynamicPatcher_Utilities
{
	struct ObjectBlockID
	{
		ObjectBlockID(int x, int y) :
			X { x },
			Y { y }
		{ }

		ObjectBlockID() :
			X { },
			Y { }
		{ }

		bool operator ==(ObjectBlockID id2) { return X == id2.X && Y == id2.Y; }
		bool operator !=(ObjectBlockID id2) { return !((*this) == id2); }

		int GetHashCode()
		{
			std::hash<int> nHasher;
			return nHasher(X) + nHasher(Y);
		}

		int X;
		int Y;
	};

//	template <uintptr_t arr, int blockLength, int mapWidth, int mapHeight>
	struct ObjectBlockContainer
	{
		ObjectBlock Blocks[mapWidth * 2 / blockLength][mapHeight * 2 / blockLength];
		ObjectBlock OuterBlock;
		int BlockLength;
		int BlockRange;
		int CurrentFrame;

		ObjectBlockContainer()
			, Blocks { }
			, OuterBlock { }
			, BlockLength { blockLength }
			, BlockRange { (blockLength - 1) / 2 }
		{
			assert(blockLength % 2 == 0);
			AllocateBlocks();
		}

		ObjectBlockID GetIDBy(CoordStruct coord)
		{
			CellStruct location = CellClass::Coord2Cell(coord);
			return GetIDBy(location);
		}

		ObjectBlockID GetIDBy(CellStruct location)
		{
			return ObjectBlockID(location.X / BlockLength, location.Y / BlockLength);
		}

		ObjectBlock GetBlock(CoordStruct coord)
		{
			CellStruct location = CellClass::Coord2Cell(coord);
			return GetBlock(location);
		}

		ObjectBlock GetBlock(CellStruct location)
		{
			ObjectBlockID id = GetIDBy(location);
			return GetBlock(id);
		}

		ObjectBlock GetBlock(ObjectBlockID id)
		{
			if (HasChange())
				RefreshBlocks();

			return GetBlockForce(id);
		}

		ObjectBlock GetBlockForce(ObjectBlockID id)
		{
			int xLength = Blocks[0].Data.size();
			int yLength = Blocks[1].Data.size();

			// notice that we may have negative id
			int x = id.X + xLength / 2;
			int y = id.Y + yLength / 2;

			if (x < xLength && y < yLength)
				return Blocks[x].Data[y];

			return OuterBlock;
		}

		void ClearObjects()
		{
			for (auto& block : Blocks)
			{
				block.Data.clear();
			}
		}


		/// <summary>
		/// clear all blocks and insert objects to blocks.
		/// </summary>
		void RefreshBlocks();

		bool HasChange()
		{
			// we assume that nothing move in one frame
			// TOCHECK: whether one frame don't change the object array
			if (CurrentFrame == Unsorted::CurrentFrame)
			{
				return false;
			}

			return true;
		}

		std::list<ObjectBlock> GetCoveredBlocks(CoordStruct coord, int range)
		{
			CellStruct location = CellClass::Coord2Cell(coord);
			return GetCoveredBlocks(location, range);
		}

		std::list<ObjectBlock> GetCoveredBlocks(CellStruct location, int range)
		{
			ObjectBlock centerBlock = GetBlock(location);
			ObjectBlockID centerId = GetIDBy(location);
			std::list<ObjectBlock> list { centerBlock };

			int rangeInCells = range / 256;
			int rangeInBlocks = rangeInCells / BlockLength;
			int tryRangeInBlocks = rangeInBlocks + 1;

			for (int x = -tryRangeInBlocks; x <= tryRangeInBlocks; x++)
			{
				for (int y = -tryRangeInBlocks; y <= tryRangeInBlocks; y++)
				{
					if (x == 0 && y == 0)
					{
						continue;
					}

					// check try block distance
					if (abs(x) > rangeInBlocks || abs(y) > rangeInBlocks)
					{
						ObjectBlockID id = GetIDBy(location + CellStruct(x * rangeInCells, y * rangeInCells));
						if (abs(id.X - centerId.X) > rangeInBlocks || abs(id.Y - centerId.Y) > rangeInBlocks)
						{
							ObjectBlock block = GetBlock(id);
							list.push_back(block);
						}
					}
					else
					{
						ObjectBlock block = GetBlock(ObjectBlockID(centerId.X + y, centerId.Y + y));
						list.push_back(block);
					}
				}
			}


			return list;
		}

	};
}