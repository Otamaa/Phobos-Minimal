#pragma once

class TeamClass;
struct LuaBridge
{
	static bool OnCalled(TeamClass* pTeam);
	static void InitScriptLuaList();
};