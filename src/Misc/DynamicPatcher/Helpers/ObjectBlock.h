#pragma once
#include <vector>

namespace DynamicPatcher_Utilities
{
	class ObjectBlockContainer;
	class ObjectBlock
	{
	public:

		ObjectBlockContainer* Container;
		ObjectBlockID ID;
		CellStruct Center;
		std::list<ObjectClass*> Objects;

		ObjectBlock(ObjectBlockContainer* container, ObjectBlockID id) :
			Container { container }
			, Center { id.X * container->BlockLength + container->BlockRange, id.Y * container->BlockLength + container->BlockRange }
			, ID { id }
			, Objects { }
		{ }

		ObjectBlock() :
			Container { }
			, Center { }
			, ID { }
			, Objects { }
		{ }


		bool IsObjectInBlock(ObjectClass* pObject);

		void AddObject(ObjectClass* pObject)
		{
			Objects.push_back(pObject);
		}

		void Clear()
		{
			Objects.clear();
		}

		std::list<ObjectBlock> GetNearBlocks();
	};

}
