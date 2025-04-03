#pragma once

#include <ArrayClasses.h>
#include <ObjectClass.h>
#include <CoordStruct.h>
#include <list>
#include "ObjectBlockContainer.h"

namespace DynamicPatcher_Utilities
{
	class ObjectFinder
	{

	public:

		enum FindMethod
		{
			BruteForce,
			ObjectBlock
		}

		static ObjectBlockContainer<0x8A0360, 10 + 1, 500, 500> container = ObjectBlockContainer<0x8A0360, 10 + 1, 500, 500>();

		static std::list<ObjectClass*> BruteFindObjectsNear(CoordStruct location, int range)
		{
			auto objects = *reinterpret_cast<DynamicVectorClass<ObjectClass*>*>(0x8A0360);
			std::list<ObjectClass*> list { };

			for (auto const& pObject : objects)
			{
				if (pObject->GetCoords().DistanceFrom(location) <= range)
				{
					list.push_back(pObject);
				}
			}

			return list;
		}

		static std::list<ObjectClass*> BlockFindObjectsNear(CoordStruct location, int range)
		{
			var blocks = container.GetCoveredBlocks(location, range);
			std::list<ObjectClass*> list { };

			foreach(var block in blocks)
			{
				foreach(var pObject in block.Objects)
				{
					if (pObject.Ref.Base.GetCoords().DistanceFrom(location) <= range)
					{
						list.Add(pObject);
					}
				}
			}

			return list;
		}


		/// <summary>
		/// find object by specified method.
		/// </summary>
		/// <param name="location">the center to find object.</param>
		/// <param name="range">distance from location.</param>
		/// <returns>the list of objects in range.</returns>
		static std::list<ObjectClass*> FindObjectsNear(CoordStruct location, int range, FindMethod method = FindMethod.ObjectBlock)
		{
			return method == FindMethod::BruteForce ?
				BruteFindObjectsNear(location, range) : BlockFindObjectsNear(location, range);
		}
	};
}
