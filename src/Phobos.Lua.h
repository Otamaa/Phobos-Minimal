#pragma once

#include <string>

struct lua_State;
class PhobosLua
{
public:
	static lua_State* p_LuaState;

	static void Construct();
	static lua_State* CreateThread(const std::string& file, int& refkey);
	static void Destroy();
	static void Test();
	static void RegisterFunctions();
};