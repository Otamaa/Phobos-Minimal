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

#define CATEGORY_TEAM StringTable::LoadString(GameStrings::TXT_TEAM())
#define CATEGORY_INTERFACE StringTable::LoadString(GameStrings::TXT_INTERFACE())
#define CATEGORY_TAUNT StringTable::LoadString(GameStrings::TXT_TAUNT())
#define CATEGORY_SELECTION StringTable::LoadString(GameStrings::TXT_SELECTION())
#define CATEGORY_CONTROL StringTable::LoadString(GameStrings::TXT_CONTROL())
#define CATEGORY_DEBUG GeneralUtils::LoadStringUnlessMissing("TXT_DEBUG", L"Debug")
#define CATEGORY_GUIDEBUG StringTable::LoadString(GameStrings::GUI_Debug)
#define CATEGORY_DEVELOPMENT GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development")
