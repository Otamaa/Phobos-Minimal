#include "DumpMemory.h"

#include <MessageListClass.h>

#include <string>
#include <Misc/Ares/Hooks/Classes/Dialogs.h>

#include <Utilities/Debug.h>
#include <Utilities/GeneralUtils.h>

const char* MemoryDumperCommandClass::GetName() const
{
	return "Dump Process Memory";
}

const wchar_t* MemoryDumperCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_MEMORY", L"Dump Memory");
}

const wchar_t* MemoryDumperCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* MemoryDumperCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_MEMORY_DESC", L"Dumps the current process's memory");
}

void MemoryDumperCommandClass::Execute(WWKey dwUnk) const
{
	if (this->CheckDebugDeactivated()) {
		return;
	}

	Dialogs::TakeMouse();

	HCURSOR loadCursor = LoadCursor(nullptr, IDC_WAIT);
	SetClassLong(Game::hWnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
	SetCursor(loadCursor);

	MessageListClass::Instance->PrintMessage(L"Dumping process memory...");

	std::wstring filename = Dialogs::FullDump();

	Debug::Log("Process memory dumped to %ls\n", filename.c_str());

	filename = L"Process memory dumped to " + filename;

	MessageListClass::Instance->PrintMessage(filename.c_str());

	loadCursor = LoadCursor(nullptr, IDC_ARROW);
	SetClassLong(Game::hWnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
	SetCursor(loadCursor);

	Dialogs::ReturnMouse();
}