
#include "Wrapper.h"

#include <Phobos.Lua.h>

#include <vector>
#include <string>

#include <TeamClass.h>

#include <Ext/Techno/Body.h>

#include <FootClass.h>
#include <UnitClass.h>

/*
* // the stack are push back
		- 3 is the first argument
		- 2 is the first argument
		- 1 is the third
*/

struct TeamClassWrapper
{
	static int SetStepCompleted(lua_State* L)
	{
		TeamClass* pTeam = (TeamClass*)lua_tointeger(L, -1);
		pTeam->StepCompleted = true;
		return 0;
	}

	static int GetCurrentScriptArg(lua_State* L)
	{
		TeamClass* pTeam = (TeamClass*)lua_tointeger(L, -1);
		lua_pushnumber(L, pTeam->CurrentScript->GetCurrentAction().Argument);
		return 1;
	}

	static int IsGuardAreaTimerTicking(lua_State* L)
	{
		TeamClass* pTeam = (TeamClass*)lua_tointeger(L, -1);
		const auto result = pTeam->GuardAreaTimer.IsTicking();
		lua_pushboolean(L, result);
		return 1;
	}

	static int GetAreaGuardTimerTimeLeft(lua_State* L)
	{

		TeamClass* pTeam = (TeamClass*)lua_tointeger(L, -1);
		const int result = pTeam->GuardAreaTimer.GetTimeLeft();
		lua_pushnumber(L, result);
		return 1;
	}

	static int StopGuardAreaTimer(lua_State* L)
	{
		TeamClass* pTeam = (TeamClass*)lua_tointeger(L, -1);
		pTeam->GuardAreaTimer.Stop();
		return 0;
	}

	static int StartGuardAreaTimer(lua_State* L)
	{
		TeamClass* pTeam = (TeamClass*)lua_tointeger(L, -2);
		pTeam->GuardAreaTimer.Start((int)lua_tointeger(L, -1));
		return 0;
	}

	static int GetFirstUnit(lua_State* L)
	{
		TeamClass* pTeam = (TeamClass*)lua_tointeger(L, -1);
		lua_pushnumber(L, (uintptr_t)pTeam->FirstUnit);
		return 1;
	}
};

struct _Unit
{
	static int GetNextTeamMember(lua_State* L)
	{
		FootClass* pFoot = (FootClass*)lua_tointeger(L, -1);
		lua_pushnumber(L, (uintptr_t)pFoot->NextTeamMember);
		return 1;
	}

};

struct _MissionClass
{
	static int QueueMission(lua_State* L)
	{
		MissionClass* pRadio = (MissionClass*)lua_tointeger(L, -3);
		pRadio->QueueMission((Mission)lua_tointeger(L, -2), lua_toboolean(L, -1));
		return 0;
	}
};

struct _TechnoExtData
{
	static int IsInWarfactory(lua_State* L)
	{
		TechnoClass* pRadio = (TechnoClass*)lua_tointeger(L, -1);
		lua_pushboolean(L, TechnoExtData::IsInWarfactory(pRadio));
		return 1;
	}
};

struct LuaScript
{
	int Number {};
	std::string Name {};
	std::string Lua_Name {};
	unique_luastate State {};

	void Initialize(int number, const char* name)
	{
		Number = number;
		Name = name;
		Lua_Name = Name + ".lua";

		InitState();
	}

	void InitState()
	{
		State.reset(luaL_newstate());

		if (luaL_dofile(State.get(), Lua_Name.c_str()) == LUA_OK)
		{
			luaL_openlibs(State.get());

			lua_register(State.get(), "_TeamClass_GetCurrentScriptArg", TeamClassWrapper::GetCurrentScriptArg);
			lua_register(State.get(), "_TeamClass_SetStepCompleted", TeamClassWrapper::SetStepCompleted);
			lua_register(State.get(), "_TeamClass_IsGuardAreaTimerTicking", TeamClassWrapper::IsGuardAreaTimerTicking);
			lua_register(State.get(), "_TeamClass_GetAreaGuardTimerTimeLeft", TeamClassWrapper::GetAreaGuardTimerTimeLeft);
			lua_register(State.get(), "_TeamClass_GetFirstUnit", TeamClassWrapper::GetFirstUnit);
			lua_register(State.get(), "_TeamClass_StartGuardAreaTimer", TeamClassWrapper::StartGuardAreaTimer);
			lua_register(State.get(), "_TeamClass_StopGuardAreaTimer", TeamClassWrapper::StopGuardAreaTimer);
			lua_register(State.get(), "_TechnoExtData_IsInWarfactory", _TechnoExtData::IsInWarfactory);
			lua_register(State.get(), "_MissionClass_QueueMission", _MissionClass::QueueMission);
			lua_register(State.get(), "_Unit_GetNextTeamMember", _Unit::GetNextTeamMember);

		}else{

			Debug::Log("Cannot Find [%s] File ! Reason (%s)\n", Lua_Name.c_str(), lua_tostring(State.get(), -1));
			State.reset(nullptr);
		}
	}
};

static std::vector<LuaScript> LuaScripts {};

bool LuaBridge::OnCalled(TeamClass* pTeam)
{
	auto const& [action, argument] = pTeam->CurrentScript->GetCurrentAction();

	for (auto& luascript : LuaScripts)
	{
		if (luascript.State && (TeamMissionType)luascript.Number == action)
		{
			lua_getglobal(luascript.State.get(), "OnExecute");

			if (lua_isfunction(luascript.State.get(), -1) != 1)
			{
				Debug::Log("Lua [%s] Failed to call `OnExecute` Function ! Reason (function not found)\n", luascript.Lua_Name.c_str());
				luascript.State.reset(nullptr);
				return false;
			}
			lua_pushnumber(luascript.State.get(), (lua_Number)((uintptr_t)pTeam));

			if (lua_pcall(luascript.State.get(), 1, 0, 0) != LUA_OK)
			{
				luascript.State.reset(nullptr);
				return false;
			}

			return true;
		}
	}

	return false;
}

void LuaBridge::InitScriptLuaList()
{
	static constexpr const char* filename = "ListHolder.lua";
	make_unique_luastate(unique_lua);
	auto L = unique_lua.get();

	if (luaL_dofile(L, filename) == LUA_OK)
	{
		lua_getglobal(L, "Scripts"); // T ==> -1

		// get the first table
		if (lua_istable(L, -1))
		{ // is T table ?
			const size_t scriptSize = (size_t)lua_rawlen(L, -1);
			LuaScripts.resize(scriptSize);

			for (size_t i = 0; i < scriptSize; i++)
			{
				lua_pushinteger(L, i + 1);
				lua_gettable(L, -2);

				if (lua_istable(L, -2))
				{
					lua_pushstring(L, "Number");
					lua_gettable(L, -2);
					const auto number = (int)lua_tointeger(L, -1);
					lua_pop(L, 1);

					lua_pushstring(L, "Name");
					lua_gettable(L, -2);
					auto name = lua_tostring(L, -1);
					lua_pop(L, 1);

					LuaScripts[i].Initialize(number, name);
				}

				lua_pop(L, 1);
			}
		}
	}
}