#pragma once

#include <ObjectClass.h>
#include <Utilities/Container.h>

#include <AlphaShapeClass.h>

class ObjectExt
{
public:

	class ExtData
	{
	public:
		AlphaShapeClass* AttachedAlpha { nullptr };

		ExtData() = default;
		virtual ~ExtData() = default; //{ GameDelete<true, true>(AttachedAlpha); }

	public:
		template <typename T>
		void inline Serialize(T& Stm)
		{
			Stm
				.Process(this->AttachedAlpha);
		}
	};
};