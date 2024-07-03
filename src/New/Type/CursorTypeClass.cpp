#include "CursorTypeClass.h"

Enumerable<CursorTypeClass>::container_t Enumerable<CursorTypeClass>::Array;

const char* Enumerable<CursorTypeClass>::GetMainSection()
{
	return "MouseCursors";
}

void CursorTypeClass::AddDefaults()
{
	if (!Array.empty())
		return;

	for (size_t i = 0; i < MouseCursorTypeToStrings.size(); ++i) {
		AllocateWithDefault(MouseCursorTypeToStrings[i], MouseCursor::DefaultCursors[i]);

		if(i < NewMouseCursorTypeToStrings.size())
			AllocateWithDefault(NewMouseCursorTypeToStrings[i], CursorTypeClass::NewMouseCursorTypeData[i]);
	}
}

void CursorTypeClass::LoadFromINI(CCINIClass* pINI)
{
	auto const pKey = this->Name.data();

	if (IS_SAME_STR_(pKey, MouseCursorTypeToStrings[0]))
		return;

	INI_EX exINI { pINI };
	auto const pSection = this->GetMainSection();

	this->CursorData.Read(exINI, pSection, pKey);
}

void CursorTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void CursorTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

void CursorTypeClass::LoadFromINIList_New(CCINIClass* pINI, bool bDebug)
{
	if (!pINI)
		return;

	const char* section = GetMainSection();

	if (!pINI->GetSection(section))
		return;

	auto const pKeyCount = pINI->GetKeyCount(section);
	if (!pKeyCount)
		return;

	Array.reserve(pKeyCount);

	for (int i = 0; i < pKeyCount; ++i) {
		if (auto const pItem = FindOrAllocate(pINI->GetKeyName(section, i))) {
			pItem->LoadFromINI(pINI);
		}
	}
}

template<typename T>
void CursorTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->CursorData)
		;
}
