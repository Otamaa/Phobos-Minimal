
#include <Phobos.Lua.h>

#include <vector>
#include <string>

#include <TeamClass.h>

class TeamClassWrapper
{

	static int SetStepCompleted(lua_State* L)
	{
		return 0;
	}
private:
	TeamClass* Team;
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

		State.reset(luaL_newstate());

		if (luaL_dofile(State.get() , Lua_Name.c_str()) != LUA_OK)
			State.reset(nullptr);
        // todo : move to the loop
		// take Team as argument
		// maybe return bool value to say it handled
		if (State && (lua_getglobal(State.get(), "OnExecute") != LUA_OK || lua_isfunction(State.get(), -1) != 1) {
			State.reset(nullptr);
		}
	}

};

static std::vector<LuaScript> LuaScripts {};

struct LuaBridge
{
	static void OnCalled(TeamClass* pTeam)
	{
		auto const& [action, argument] = pTeam->CurrentScript->GetCurrentAction();

		for (auto& luascript : LuaScripts) {
			if (luascript.State && (TeamMissionType)luascript.Number == action) {

			}
		}
	}

	static void InitScriptLuaList()
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
};