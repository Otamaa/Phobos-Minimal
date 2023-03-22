#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/Enum.h>

#include <WWMouseClass.h>
#include <GeneralDefinitions.h>

class CursorTypeClass final : public Enumerable<CursorTypeClass>
{
public:

	Valueable<MouseCursor> CursorData;
	CursorTypeClass(const char* const pTitle) : Enumerable<CursorTypeClass>(pTitle)
		, CursorData { }
	{ }

	virtual ~CursorTypeClass() override = default;
	static void AddDefaults();

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	static void LoadFromINIList_New(CCINIClass* pINI, bool bDebug = false);

	static std::array<const char* const, (size_t)MouseCursorType::count> MouseCursorTypeToStrings;
	static std::array<const char*, (size_t)NewMouseCursorType::count> NewMouseCursorTypeToStrings;
	static std::array<MouseCursor, (size_t)NewMouseCursorType::count> NewMouseCursorTypeData;

private:
	template <typename T>
	void Serialize(T& Stm);

};
