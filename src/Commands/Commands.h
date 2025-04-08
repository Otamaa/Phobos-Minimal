#pragma once

#include <CommandClass.h>
#include <StringTable.h>
#include <MessageListClass.h>
#include <Phobos.h>

class PhobosCommandClass : public CommandClass
{
protected:
	bool CheckDebugDeactivated() const;

};

#define CATEGORY_TEAM StringTable::FetchString(GameStrings::TXT_TEAM())
#define CATEGORY_INTERFACE StringTable::FetchString(GameStrings::TXT_INTERFACE())
#define CATEGORY_TAUNT StringTable::FetchString(GameStrings::TXT_TAUNT())
#define CATEGORY_SELECTION StringTable::FetchString(GameStrings::TXT_SELECTION())
#define CATEGORY_CONTROL StringTable::FetchString(GameStrings::TXT_CONTROL())
#define CATEGORY_DEBUG GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_DEBUG", L"Debug")
#define CATEGORY_GUIDEBUG StringTable::FetchString(GameStrings::GUI_Debug)
#define CATEGORY_DEVELOPMENT GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_DEVELOPMENT", L"Development")