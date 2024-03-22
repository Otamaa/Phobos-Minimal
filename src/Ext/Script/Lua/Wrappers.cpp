
#include "Lib/Lua542/lua.hpp"
#include <TeamClass.h>

class TeamClassWrapper {

	static int SetStepCompleted(lua_State* L) {
		return 0;
	}
private:
	TeamClass* Team;
};

struct LuaScript{

int Number;
std::string Name;
std::string Lua_Name;
std::unique_ptr<lua_state> State;

  LuaScript::LuaScript(int number , const char* name)
  Number(number), Name(name) ,Lua_Name(name)
{  
	Lua_Name += ".lua";

	State.reset(lua_newstate());

	if(lua_dofile(Statem.get()) != LUA_OK)
	   State.reset(nullptr);

   if(State) {
	  if(lua_getfunction(State.get()) != LUA_OK)
	    State.reset(nullptr);
   }
}

 ~LuaScript::LuaScript = default;
};

static std::vector<LuaScript> LuaScripts {};

struct LuaBridge
{
	static void OnCalled(TeamClass*) {
         for(auto& luascript: LuaScripts){
			if(luascript.State){
                 
			}
		 }
	}

	static void ReadList() {
       static constexpr const char* MainList = "ListHolder.lua";



	}
};