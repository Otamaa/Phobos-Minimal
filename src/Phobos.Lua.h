#pragma once

#include <Lua542/lua.hpp>
#include <memory>

struct luastatedeleter
{
	void operator ()(lua_State* l) noexcept {
		if (l) {
			lua_close(l);
		}
	}
};
using unique_luastate = std::unique_ptr<lua_State, luastatedeleter>;
#define make_unique_luastate(to) unique_luastate to {}; to.reset(luaL_newstate())
#define close_unique_luastate(to) to.reset(nullptr)