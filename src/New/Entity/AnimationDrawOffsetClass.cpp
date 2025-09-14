#include "AnimationDrawOffsetClass.h"

#include <New/PhobosAttachedAffect/PhobosAttachEffectTypeClass.h>

bool AnimationDrawOffsetClass::LoadFromINI(CCINIClass* pINI, const char* pSection, int index)
{
	INI_EX exINI(pINI);

	const std::string offset = std::string("Animation.DrawOffset") + std::to_string(index);

	this->Offset.Read(exINI, pSection, offset.c_str());

	if (this->Offset->IsEmpty())
		return false;

	this->RequiredTypes.Read(exINI, pSection, (offset + ".RequiredTypes").c_str());

	return true;
}

#pragma region(save/load)

template <class T>
bool AnimationDrawOffsetClass::Serialize(T& stm)
{
	return stm
		.Process(this->Offset)
		.Process(this->RequiredTypes)
		.Success();
}

bool AnimationDrawOffsetClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool AnimationDrawOffsetClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<AnimationDrawOffsetClass*>(this)->Serialize(stm);
}

#pragma endregion(save/load)