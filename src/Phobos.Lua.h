#pragma once

#include <Lib/Lua/lua.hpp>

#include <string>
#include <memory>

struct luastatedeleter
{
	void operator ()(lua_State* l) noexcept
	{
		if (l)
		{
			lua_close(l);
		}
	}
};

using unique_luastate = std::unique_ptr<lua_State, luastatedeleter>;

struct LuaData {
	static std::string LuaDir;
	static void ApplyCoreHooks();
};

#define make_unique_luastate(to) unique_luastate to {}; to.reset(luaL_newstate())
#define close_unique_luastate(to) to.reset(nullptr)