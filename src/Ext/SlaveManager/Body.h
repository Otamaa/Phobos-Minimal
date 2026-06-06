#pragma once
#include <SlaveManagerClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class SlaveManagerExtData
{
public:
	static COMPILETIMEEVAL DWORD Canary = 0xD0FCE096;
};

class FakeSlaveManagerClass : public SlaveManagerClass
{
public:

};