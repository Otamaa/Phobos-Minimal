#pragma once

#include <Lib/Lua/lua.hpp>

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <mutex>
#include <objidl.h>

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


// -----------------------------------------------------------------------------
// unique_luastate is assumed defined elsewhere (deleter calling lua_close).
// Kept as forward declaration reference only; not redefined here.
// -----------------------------------------------------------------------------

class HouseClass;
class TeamClass;

struct LuaAPI
{
	static bool g_scriptReady;
	static unique_luastate g_L;

	// unique_ptr, not a plain std::once_flag - once_flag is neither
	// copyable nor movable, so it can't be reset in place. OnScenarioClear
	// re-arms engine bring-up by swapping in a brand new once_flag.
	static std::unique_ptr<std::once_flag> g_engineOnce;

	// CRC32 over the concatenated bytes of every .lua file under
	// Luascripts (deterministic filename order), computed once the currently
	// loaded script set finishes initializing. Written into every save's Lua
	// blob so a later load can tell whether the script set has changed since
	// that save was made - see ComputeScriptFingerprint / OnGlobalGameSave /
	// OnGlobalGameLoad in the .cpp for the full contract.
	static uint32_t g_scriptFingerprint;

	static void OnGameRunning();
	static void RunInitScript(lua_State* L);
	static void OnGameFrame();
	static void OnRender();
	static void OnInvalidatePointer(void* ptr, bool removed);

	// fired when a scenario is torn down (returning to menu, next
	// mission/skirmish, etc). Gives the current script a last chance to run
	// its own OnScenarioClear() cleanup hook, then fully closes and re-arms
	// the Lua engine so the next scenario starts from a fresh VM + init.lua run.
	static void OnScenarioClear();

	// COM Structured Storage save/load entry points. Data crosses
	// the boundary as [fingerprint: uint32][payload length: uint32][payload
	// bytes]. The fingerprint is g_scriptFingerprint at save time; on load it
	// is compared against the CURRENT script set's fingerprint and the result
	// is handed to the script as the second argument of its own
	// OnGlobalGameLoad(payload, scriptsChanged) - the script decides what to
	// do with mismatched data, C++ never refuses to load over it. A
	// failing/erroring script never fails the HRESULT - see .cpp for why
	// (out-of-order stream access by other subsystems).
	static HRESULT OnGlobalGameSave(IStream* pStm);
	static HRESULT OnGlobalGameLoad(IStream* pStm);

	static lua_State* CreateEngine();

	// brings the engine up exactly once, on the calling (main game) thread,
	// swallowing any C++ exception thrown during init so call_once never re-throws
	// on a later frame (it would otherwise retry forever without a functioning engine).
	static void EnsureEngine();

	// CRC32 over every .lua file found (recursively) under
	// Debug::ApplicationFilePath + "\\Luascripts", processed in sorted
	// filename order for determinism. Called once from RunInitScript after a
	// successful script load; result is stored in g_scriptFingerprint.
	// VERIFY: if the engine already exposes a CRC32 utility elsewhere (e.g.
	// used for sync-check purposes), prefer that over this self-contained
	// implementation to avoid two divergent CRC32 tables in the binary.
	static uint32_t ComputeScriptFingerprint();

public:
	// Registers the global "World" namespace and the "LuaAPI.Techno" userdata
	// metatable on the given lua_State.
	static void RegisterTechnoBindings(lua_State* L);

	// Pushes a userdata wrapping a TechnoClass-derived pointer (BuildingClass,
	// UnitClass, ...) onto the stack. Always pushes one value.
	// also registers the new userdata (weak value) in the
	// invalidation registry so it can be zeroed out by InvalidateTechnoUserdata.
	static void PushTechno(lua_State* L, void* pTechno);

	// Expires timed disables; call once per game frame from the main thread.
	static void ProcessObjects(unsigned int currentFrame);

	// zeroes out the internal pointer slot of any live Techno
	// userdata wrapping `ptr`, so scripts holding a stale reference get a
	// clean "techno object is no longer valid" Lua error instead of touching
	// freed engine memory. Must be called BEFORE the object is actually freed
	// (i.e. from the engine's own invalidate-pointer hook), and before the
	// Lua-side OnInvalidatePointer callback runs.
	static void InvalidateTechnoUserdata(lua_State* L, void* ptr);

	// Registers the global "House" namespace and the "LuaAPI.House" userdata
	// metatable on the given lua_State.
	static void RegisterHouseBindings(lua_State* L);

	// Pushes a userdata wrapping pHouse onto the stack (or nothing if null).
	// Returns the number of values pushed (1 or 0).
	static int PushHouse(lua_State* L, HouseClass* pHouse);

#pragma region Team
	// Registers the "LuaAPI.Team" userdata metatable and its invalidation
	// registry. There's no global "Team" namespace to register (unlike
	// World/House) - teams are only ever handed to scripts as arguments to
	// OnTeamMission, never queried in bulk.
	static void RegisterTeamBindings(lua_State* L);

	// Pushes a userdata wrapping a TeamClass* onto the stack. Same
	// weak-registry pattern as PushTechno - see InvalidateTeamUserdata.
	static void PushTeam(lua_State* L, void* pTeam);

	// Zeroes out the internal pointer slot of any live Team userdata
	// wrapping `ptr`, called from OnInvalidatePointer before the underlying
	// TeamClass instance is freed. Same contract as InvalidateTechnoUserdata.
	static void InvalidateTeamUserdata(lua_State* L, void* ptr);

	// fallback dispatch for TeamMissionType action IDs that
	// neither the native switch nor Ares/Phobos's own script-action
	// extension points recognize. Call this from ExecuteTMissions'
	// (and equivalent patches') default: case, AFTER those existing
	// checks, and only fall through to native unknown-action handling
	// (dissolve/log) if this returns false.
	//
	// Returns true if a Lua OnTeamMission(team, action, argument) handler
	// exists AND chose to handle this action - regardless of whether it
	// completed the mission this tick (multi-tick missions poll every
	// tick until the script calls team:Complete(), same as native
	// Wait_till_fully_loaded-style missions). Returns false if no handler
	// is defined at all, or the handler explicitly returned false for
	// this action id.
	//
	// A Lua error inside the handler is logged and treated as "handled,
	// not complete" rather than propagating a fall-through to native
	// unknown-action handling - a script bug shouldn't cause a team to
	// suddenly dissolve/warn as if the action were never recognized.
	static bool OnTeamMission(TeamClass* pTeam, int action, int argument);
#pragma endregion

	// global "Scenario" namespace - global/local variable
	// control matching vanilla TeamMissionType::Set_global/Clear_global/
	// Set_local/Clear_local. No dedicated userdata type needed - there's
	// only ever one active ScenarioClass::Instance.
	static void RegisterScenarioBindings(lua_State* L);
};

#define make_unique_luastate(to) unique_luastate to {}; to.reset(luaL_newstate())
#define close_unique_luastate(to) to.reset(nullptr)