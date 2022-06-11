#pragma once
#include <vector>

class AttachEffectType;
struct AttachEffectData
{
	std::vector<AttachEffectType*> AttachEffectTypes;
	int CabinLength;

	AttachEffectData() :
		AttachEffectTypes { }
		, CabinLength { 0 }
	{ }

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(AttachEffectTypes)
			.Process(CabinLength)
			;
	}

};