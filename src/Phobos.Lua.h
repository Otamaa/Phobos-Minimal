#pragma once

#include <Lib/Lua/lua.hpp>

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <mutex>
		
class HouseClass;

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
	static std::string AdditionalStringTableFmt;
	static std::string FontName;
	static std::string StatisticPacketName;
	static std::string filename;
	static std::string CoreHandles;
	static std::vector<std::pair<uintptr_t, std::string>> map_replaceAddrTo;
	static std::map<std::string, bool> SafeFiles;
	static bool IsActive;

	static void ApplyCoreHooks();

};

struct LuaAPI {
	static bool g_scriptReady;
	static unique_luastate g_L;
	static std::once_flag g_engineOnce;

	static void RunInitScript(lua_State* L);
	static void OnGameFrame();
	static lua_State* CreateEngine();

public:
	// Registers the global "World" namespace and the "LuaAPI.Techno" userdata
	// metatable on the given lua_State.
	static void RegisterTechnoBindings(lua_State* L);

	// Pushes a userdata wrapping a TechnoClass-derived pointer (BuildingClass,
	// UnitClass, ...) onto the stack. Always pushes one value.
	static void PushTechno(lua_State* L, void* pTechno);

	// Expires timed disables; call once per game frame from the main thread.
	static void ProcessObjects(unsigned int currentFrame);

	// Registers the global "House" namespace and the "LuaAPI.House" userdata
	// metatable on the given lua_State.
	static void RegisterHouseBindings(lua_State* L);

	// Pushes a userdata wrapping pHouse onto the stack (or nothing if null).
	// Returns the number of values pushed (1 or 0).
	static int PushHouse(lua_State* L, HouseClass* pHouse);
};

#define make_unique_luastate(to) unique_luastate to {}; to.reset(luaL_newstate())
#define close_unique_luastate(to) to.reset(nullptr)