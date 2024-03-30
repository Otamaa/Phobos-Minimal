#pragma once

#include <Phobos.Lua.h>

class TeamClass;
struct LuaBridge
{
	static bool OnCalled(TeamClass* pTeam);
	static void InitScriptLuaList();

	static int GetAppropriateAction(int from);
};