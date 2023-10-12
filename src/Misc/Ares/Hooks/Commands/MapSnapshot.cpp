#include "MapSnapshot.h"

#include <MessageListClass.h>
#include <Utilities/GeneralUtils.h>

const char* MapSnapshotCommandClass::GetName() const
{
	return "MapSnapshot";
}

const wchar_t* MapSnapshotCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_MAP_SNAPSHOT", L"Map Snapshot");
}

const wchar_t* MapSnapshotCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* MapSnapshotCommandClass::GetUIDescription() const
{
	return  GeneralUtils::LoadStringUnlessMissing("TXT_MAP_SNAPSHOT_DESC", L"Saves the currently played map.");
}

void MapSnapshotCommandClass::Execute(WWKey dwUnk) const
{
	if (this->CheckDebugDeactivated()) {
		return;
	}

	char fName[0x80];

	SYSTEMTIME time;
	GetLocalTime(&time);

	IMPL_SNPRNINTF(fName, 0x7F, "Map.%04u%02u%02u-%02u%02u%02u-%05u.yrm",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

	Game::WriteMapFiles(fName);
	wchar_t msg[0xA0] = L"\0";
	wsprintfW(msg, L"Map Snapshot saved as '%hs'.", fName);

	MessageListClass::Instance->PrintMessage(msg);
}