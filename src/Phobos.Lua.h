#pragma once

#include <Lib/Lua/lua.hpp>

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <utility>

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
	static std::string MainWindowStr;
	static std::string filename;
	static std::string CoreHandles;
	static std::vector<std::pair<uintptr_t, std::string>> map_replaceAddrTo;
	static std::map<std::string, bool> SafeFiles;
	static bool IsActive;

	static void ApplyCoreHooks();

};

#define make_unique_luastate(to) unique_luastate to {}; to.reset(luaL_newstate())
#define close_unique_luastate(to) to.reset(nullptr)