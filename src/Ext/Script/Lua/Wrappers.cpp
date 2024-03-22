
#include "Lib/Lua542/lua.hpp"
#include <TeamClass.h>

class TeamClassWrapper {

	static int SetStepCompleted(lua_State* L) {
		return 0;
	}
private:
	TeamClass* Team;
};


// Load the `ListHolder.Lua` first to obtail all the avaible scripts
// generate the `LuaName` for them
// put then onto the map/vector
// loop `OnExecute`
struct LuaBridge
{
	static void OnCalled(TeamClass*) {

	}
};