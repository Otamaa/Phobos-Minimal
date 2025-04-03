#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/SavegameDef.h>

class BarTypeClass final : public Enumerable<BarTypeClass>
{
public:

	BarTypeClass(const char* pTitle = GameStrings::NoneStr()) : Enumerable<BarTypeClass>(pTitle)
	{ }

	void LoadFromINI(CCINIClass * pINI);
	void LoadFromStream(PhobosStreamReader & Stm) {
		this->Serialize(Stm);
	}

	void SaveToStream(PhobosStreamWriter & Stm) {
		this->Serialize(Stm);
	}

private:
	template <typename T>
	void Serialize(T & Stm)
	{
	}

};
