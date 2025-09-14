#pragma once
#include <Utilities/TemplateDef.h>

class CCINIClass;
class PhobosAttachEffectTypeClass;
class AnimationDrawOffsetClass
{
public:
	Valueable<Point2D> Offset;
	ValueableVector<PhobosAttachEffectTypeClass*> RequiredTypes;

public:

	bool LoadFromINI(CCINIClass* pINI, const char* pSection, int index);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};