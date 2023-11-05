#pragma once

#include <Utilities/TemplateDefB.h>

struct InsigniaData
{
	Promotable<SHPStruct*> Shapes { nullptr };
	Promotable<int> Frame { -1 };
	Valueable<Point3D> Frames { { -1, -1, -1 } };

	inline bool Load(PhobosStreamReader& stm, bool registerForChange)
	{
		return this->Serialize(stm);
	}

	inline bool Save(PhobosStreamWriter& stm) const
	{
		return const_cast<InsigniaData*>(this)->Serialize(stm);
	}

private:
	template <typename T>
	inline bool Serialize(T& stm)
	{
		return stm
			.Process(this->Shapes)
			.Process(this->Frame)
			.Process(this->Frames)
			.Success();
	}
};