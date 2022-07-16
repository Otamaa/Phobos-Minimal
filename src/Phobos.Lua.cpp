#include "Phobos.Lua.h"
#include "Phobos.h"

#include <Utilities/Debug.h>
#include <Utilities/GeneralUtils.h>

extern "C" {
	#include <Lib/Lua/lua.h>
	#include <Lib/Lua/lualib.h>
	#include <Lib/Lua/lauxlib.h>
}

lua_State* PhobosLua::p_LuaState = nullptr;

void PhobosLua::Construct()
{
	PhobosLua::p_LuaState = luaL_newstate();

	// open any library we may use
	luaL_openlibs(PhobosLua::p_LuaState);

	// register all function needed from c++ to Lua
	RegisterFunctions();
}

void PhobosLua::Destroy()
{
	lua_close(PhobosLua::p_LuaState);
}

void PhobosLua::Test()
{
	if (PhobosLua::p_LuaState)
		return;

	auto& L = PhobosLua::p_LuaState;

	// read the Lua script off disk and execute it
	if ((luaL_dofile(L, "test.lua")) != 0)
	{
		// handle any errors
		Debug::Log("Uneable To Find test.lua ! \n");
		return ;
	}

	lua_getglobal(L, "x");

	// interpret the value at the top of the stack
	double val = (double)lua_tonumber(L, -1);

	// pop the value of X off the stack
	lua_pop(L, 1);
	Debug::Log("Value of X:%fl \n" , val);
}

int L_GenUtils_FastPow_Wrap(lua_State* L)
{
	const int nArg = lua_gettop(L);
	double a = (double)lua_tonumber(L, nArg - 1);
	double b = (double)lua_tonumber(L, nArg);
	auto nRes = GeneralUtils::FastPow(a, b);
	lua_pushnumber(L, nRes);
	return 1;
}

void PhobosLua::RegisterFunctions()
{
	if (!PhobosLua::p_LuaState)
		return;

	auto& L = PhobosLua::p_LuaState;

	lua_register(L, "GenUtils_FastPow", L_GenUtils_FastPow_Wrap);
}

//not sure ,..
lua_State* PhobosLua::CreateThread(const std::string& file, int& refKey)
{
	lua_State* newState = lua_newthread(PhobosLua::p_LuaState);
	refKey = luaL_ref(PhobosLua::p_LuaState, LUA_REGISTRYINDEX);

	lua_newtable(newState); //new globals table
	lua_newtable(newState); //metatable
	lua_pushliteral(newState, "__index");
	lua_pushglobaltable(newState); //original globals
	lua_settable(newState, -3);
	lua_setmetatable(newState, -2);

	// read the Lua script off disk and execute it
	if ((luaL_dofile(newState, file.c_str())) != 0)
	{
		// handle any errors
		Debug::Log("Uneable To Find %s ! \n", file.c_str());
		lua_close(newState);
		return nullptr;
	}

	return newState;
}