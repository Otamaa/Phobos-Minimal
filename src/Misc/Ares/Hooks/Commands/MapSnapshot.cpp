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

	SYSTEMTIME time;
	GetLocalTime(&time);

	std::string fName = std::format("Map.{:04}{:02}{:02}-{:02}{:02}{:02}-{:05}.yrm",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

	Game::WriteMapFiles(fName.c_str());
	wchar_t msg[0xA0] = L"\0";
	wsprintfW(msg, L"Map Snapshot saved as '%hs'.", fName.c_str());

	MessageListClass::Instance->PrintMessage(msg);
}