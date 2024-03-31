
#include "Wrapper.h"

#include <Phobos.Lua.h>

#include <vector>
#include <string>

#include <TeamClass.h>

#include <Ext/Techno/Body.h>

#include <FootClass.h>
#include <UnitClass.h>

#include <sol/sol.hpp>
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
	sol::state State {};

	void Initialize(int number, const char* name)
	{
		Number = number;
		Name = name;
		Lua_Name = Name + ".lua";

		InitState();
	}

	void InitState()
	{
		State.open_libraries(sol::lib::base, sol::lib::io);

		try {
			if (State.safe_script_file(Lua_Name).status() == sol::call_status::ok)
			{
				State["_TeamClass_GetCurrentScriptArg"] = TeamClassWrapper::GetCurrentScriptArg;
				State["_TeamClass_SetStepCompleted"] = TeamClassWrapper::SetStepCompleted;
				State["_TeamClass_IsGuardAreaTimerTicking"] = TeamClassWrapper::IsGuardAreaTimerTicking;
				State["_TeamClass_GetAreaGuardTimerTimeLeft"] = TeamClassWrapper::GetAreaGuardTimerTimeLeft;
				State["_TeamClass_GetFirstUnit"] = TeamClassWrapper::GetFirstUnit;
				State["_TeamClass_StartGuardAreaTimer"] = TeamClassWrapper::StartGuardAreaTimer;
				State["_TeamClass_StopGuardAreaTimer"] = TeamClassWrapper::StopGuardAreaTimer;
				State["_TechnoExtData_IsInWarfactory"] = _TechnoExtData::IsInWarfactory;
				State["_MissionClass_QueueMission"] = _MissionClass::QueueMission;
				State["_Unit_GetNextTeamMember"] = _Unit::GetNextTeamMember;
			}

		}catch(const sol::error& what) {

			Debug::Log("Cannot Find [%s] File ! Reason (%s)\n", Lua_Name.c_str(), what.what());
			State = nullptr;
		}
	}
};

//static std::vector<LuaScript> LuaScripts {};
struct PairOfNumbers {
	int Original;
	int Alternate;
};
static std::vector<PairOfNumbers> SriptNumbers {};

bool LuaBridge::OnCalled(TeamClass* pTeam)
{
#ifdef EXPERIMENTAL
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
#endif
	return false;
}

void LuaBridge::InitScriptLuaList(sol::state& sol_state)
{
	const std::string filename = LuaData::LuaDir + "\\ScriptAlternativeNumbering.lua";

	try
	{
		if (sol_state.safe_script_file(filename).status() == sol::call_status::ok)
		{

			if (const std::optional<sol::table> replaces = sol_state["Scripts"])
			{
				SriptNumbers.reserve(replaces->size());

				for (const auto& entry : replaces.value()) {
					sol::object key = entry.first;
					sol::object value = entry.second;

					const sol::optional sKey = key.as<std::string>();

					if (!sKey.has_value())
						continue;

					auto state = SriptNumbers.emplace_back();

					if (sKey == "Original") { state.Original = value.as<int>(); }
					if (sKey == "Alternative") { state.Alternate = value.as<int>(); }
				}
			}
		}
	}
	catch (const sol::error& what)
	{
		Debug::Log("%s\n", what.what());
	}
}

int LuaBridge::GetAppropriateAction(int from)
{
	for (auto& cur : SriptNumbers) {
		if (cur.Alternate == from){
			return cur.Original;
		}
	}

	return from;
}
