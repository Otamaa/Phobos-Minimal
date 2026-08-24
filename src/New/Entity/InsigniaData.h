#pragma once

#include <Utilities/TemplateDefB.h>

#include <YRMathVector.h>

class PhobosStreamReader;
class PhobosStreamWriter;
struct SHPCaches;
struct InsigniaData
{
	Promotable<SHPCaches*> Shapes { nullptr };
	Promotable<int> Frame { -1 };
	Valueable<Vector3D<int>> Frames { { -1, -1, -1 } };

public:

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};