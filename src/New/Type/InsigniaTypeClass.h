#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>

class InsigniaTypeClass final : public Enumerable<InsigniaTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "InsigniaTypes";
	static COMPILETIMEEVAL const char* ClassName = "InsigniaTypeClass";

public:
	Promotable<SHPStruct*> Insignia { };
	Promotable<int> InsigniaFrame { -1 };

	InsigniaTypeClass(const char* const pTitle) : Enumerable<InsigniaTypeClass>(pTitle)	{ }
	virtual ~InsigniaTypeClass() = default;

	void LoadFromINI(CCINIClass* pINI);

	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};