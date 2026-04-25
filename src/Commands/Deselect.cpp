#include "Deselect.h"

#include <Utilities/GeneralUtils.h>

const char* DeselectObjectCommandClass::GetName() const
{
	return "DeselectOne";
}

const wchar_t* DeselectObjectCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DESELECT", L"Deselect 1 Objects");
}

const wchar_t* DeselectObjectCommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* DeselectObjectCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DESELECT_DESC", L"Deselect 1 object from current selection.");
}

void DeselectObjectCommandClass::Execute(WWKey eInput) const
{
	int nCount = ObjectClass::CurrentObjects->Count;

	if (nCount > 0)
	{
		ObjectClass::CurrentObjects->Items[0]->Deselect();
	}
}

const char* DeselectObject5CommandClass::GetName() const
{
	return "DeselectFive";
}

const wchar_t* DeselectObject5CommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DESELECT5", L"Deselect 5 Objects");
}

const wchar_t* DeselectObject5CommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* DeselectObject5CommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DESELECT5_DESC", L"Deselect 5 object from current selection.");
}

void DeselectObject5CommandClass::Execute(WWKey eInput) const
{
	int nCount = ObjectClass::CurrentObjects->Count;

	if (nCount > 0)
	{
		int max = nCount >= 5 ? 5 : nCount;
		for (int i = max - 1; i >= 0; i--)
		{
			ObjectClass::CurrentObjects->Items[i]->Deselect();
		}
	}
}