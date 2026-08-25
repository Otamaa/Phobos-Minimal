#include "Phobos.Lua.h"

#include <TechnoClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>

#include <MapClass.h>
#include <HouseClass.h>

#include <Utilities/Debug.h>

#pragma region Techno

constexpr const char* kMetaName = "LuaAPI.Techno";

// Defined below; pushes a "LuaAPI.Techno" userdata wrapping pTechno.
void LuaAPI::PushTechno(lua_State* L, void* pTechno) {
	auto* ud = static_cast<void**>(lua_newuserdatauv(L, sizeof(void*), 0));
	*ud = pTechno;
	luaL_getmetatable(L, kMetaName);
	lua_setmetatable(L, -2);
}

// Timed-disable registry, processed every frame from OnGameFrame.
struct DisableEntry
{
	TechnoClass* ptr;
	bool isBuilding;
	bool hadPower;        // BuildingClass::HasPower prior to the blackout
	unsigned int expiryFrame;
};
std::vector<DisableEntry> g_disabledEntries;

bool IsValid(TechnoClass* pTechno)
{
	return pTechno != nullptr && pTechno->IsAlive && pTechno->Health > 0;
}

TechnoClass* CheckTechno(lua_State* L, int idx)
{
	void* ud = luaL_checkudata(L, idx, kMetaName);
	auto* pTechno = *static_cast<TechnoClass**>(ud);
	if (!pTechno)
	{
		luaL_error(L, "techno object is no longer valid");
		return nullptr;
	}
	return pTechno;
}

// --- instance methods ------------------------------------------------------

// obj:GetTypeName() -> string
int Techno_GetTypeName(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
		return 0;
	lua_pushstring(L, pTechno->GetType()->get_ID());
	return 1;
}

// obj:GetHealth() -> int
int Techno_GetHealth(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	lua_pushinteger(L, pTechno->Health);
	return 1;
}

// obj:GetMaxHealth() -> int
int Techno_GetMaxHealth(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
		return 0;
	lua_pushinteger(L, pTechno->GetType()->Strength);
	return 1;
}

// obj:GetOwner() -> house | nil
int Techno_GetOwner(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
		return 0;
	return LuaAPI::PushHouse(L, pTechno->Owner);
}

// obj:GetPosition() -> table {x, y, z} in map cells
int Techno_GetPosition(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
		return 0;

	CoordStruct coords = pTechno->GetCoords();

	lua_createtable(L, 0, 3);
	lua_pushinteger(L, coords.X / 256);
	lua_setfield(L, -2, "x");
	lua_pushinteger(L, coords.Y / 256);
	lua_setfield(L, -2, "y");
	lua_pushinteger(L, coords.Z / 256);
	lua_setfield(L, -2, "z");
	return 1;
}

// obj:IsAlive() -> bool
int Techno_IsAlive(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	lua_pushboolean(L, IsValid(pTechno) ? 1 : 0);
	return 1;
}

// obj:GetId() -> unsigned int (engine-wide unique object ID)
int Techno_GetId(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	lua_pushinteger(L, static_cast<lua_Integer>(pTechno->UniqueID));
	return 1;
}


// obj:GetKind() -> string ("building" | "unit" | "infantry" | "aircraft" | "other")
int Techno_GetKind(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
		return 0;

	switch (pTechno->WhatAmI())
	{
	case AbstractType::Building:  lua_pushliteral(L, "building");  break;
	case AbstractType::Unit:      lua_pushliteral(L, "unit");      break;
	case AbstractType::Infantry:  lua_pushliteral(L, "infantry");  break;
	case AbstractType::Aircraft:  lua_pushliteral(L, "aircraft");  break;
	default:                      lua_pushliteral(L, "other");     break;
	}
	return 1;
}

// obj:GetDistanceTo(other_obj) -> number (in map cells)
int Techno_GetDistanceTo(lua_State* L)
{
	auto* pSelf = CheckTechno(L, 1);

	void* ud = luaL_testudata(L, 2, kMetaName);
	if (!ud)
		return luaL_argerror(L, 2, "expected a techno object");

	auto* pOther = *static_cast<TechnoClass**>(ud);
	if (!IsValid(pSelf) || !IsValid(pOther))
	{
		lua_pushnil(L);
		return 1;
	}

	CellStruct a = pSelf->GetMapCoords();
	CellStruct b = pOther->GetMapCoords();

	lua_pushnumber(L, a.DistanceFrom(b));
	return 1;
}

// obj:TakeDamage(damage_amount, [warheadName]) -> int remaining health
//
// With a warhead name (default "Fire", fallback Rules->C4Warhead) the damage
// goes through the NATIVE TechnoClass::ReceiveDamage pipeline - triggering
// proper fire/splash anims, InfDeath animations, screams, sounds and kill
// credit instead of a raw Health write.
int Techno_TakeDamage(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
	{
		lua_pushinteger(L, 0);
		return 1;
	}

	lua_Integer damage = luaL_checkinteger(L, 2);
	const char* warheadName = luaL_optstring(L, 3, nullptr);

	WarheadTypeClass* pWH = nullptr;
	if (warheadName && *warheadName)
		pWH = WarheadTypeClass::Find(warheadName);

	int dmg = static_cast<int>(damage);
	pTechno->ReceiveDamage(&dmg, 0, pWH, nullptr, true, true, nullptr);
	lua_pushinteger(L, pTechno->Health);
	return 1;
}

// obj:Disable(duration_frames)
//
// Real EMP-style lock:
// - buildings: cut HasPower (drives IsPowerOnline(), so weapons stop firing)
//   AND call DisableStuff() (switched-off state);
// - feet: start the game's own ParalysisTimer (giant-squid mechanism)
//   AND set Deactivated.
// All state is restored automatically when the timer expires.
int Techno_Disable(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
		return 0;

	lua_Integer frames = luaL_checkinteger(L, 2);
	if (frames <= 0)
		return 0;

	DisableEntry entry {};
	entry.ptr = pTechno;
	entry.expiryFrame = Unsorted::CurrentFrame() + static_cast<unsigned int>(frames);

	if (pTechno->WhatAmI() == AbstractType::Building)
	{
		auto* pBuilding = static_cast<BuildingClass*>(pTechno);
		entry.isBuilding = true;
		entry.hadPower = pBuilding->HasPower;
		pBuilding->HasPower = false;      // IsPowerOnline() -> false: no firing
		pBuilding->DisableStuff();        // official switched-off state
		pTechno->Deactivated = true;
	}
	else
	{
		entry.isBuilding = false;
		entry.hadPower = true;
		// Units/infantry are always FootClass-derived.
		static_cast<FootClass*>(pTechno)->ParalysisTimer.Start(static_cast<int>(frames)); // native paralysis
		pTechno->Deactivated = true;
	}

	g_disabledEntries.push_back(entry);
	return 0;
}

// --- navigation (FootClass only: units / infantry / aircraft) ---------------

// Returns FootClass* if the techno is a mobile unit, else nullptr.
FootClass* AsFoot(TechnoClass* pTechno)
{
	switch (pTechno->WhatAmI())
	{
	case AbstractType::Unit:
	case AbstractType::Infantry:
	case AbstractType::Aircraft:
		return static_cast<FootClass*>(pTechno);
	default:
		return nullptr;
	}
}

// obj:Scatter([opt_x, opt_y]) - flee from current position (or towards a cell).
int Techno_Scatter(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
		return 0;

	FootClass* pFoot = AsFoot(pTechno);
	if (!pFoot)
		return 0;

	CoordStruct crd = pTechno->GetCoords();
	if (lua_gettop(L) >= 3 && lua_isnumber(L, 2) && lua_isnumber(L, 3))
	{
		int cx = static_cast<int>(lua_tointeger(L, 2));
		int cy = static_cast<int>(lua_tointeger(L, 3));
		crd.X = cx * 256 + 128;
		crd.Y = cy * 256 + 128;
	}

	pFoot->Scatter(crd, true, false);
	return 0;
}

// obj:MoveTo(cellX, cellY) -> bool success
int Techno_MoveTo(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	FootClass* pFoot = AsFoot(pTechno);
	if (!pFoot)
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	int cellX = static_cast<int>(luaL_checkinteger(L, 2));
	int cellY = static_cast<int>(luaL_checkinteger(L, 3));
	CellStruct cell { static_cast<short>(cellX), static_cast<short>(cellY) };

	CellClass* pCell = MapClass::Instance->TryGetCellAt(cell);
	if (!pCell)
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	// Engine-team pattern: point the nav destination at the cell, then queue Move.
	pFoot->Destination = pCell;
	pFoot->QueueMission(Mission::Move, true);

	Debug::LogInfo("[Nav] {} moving to ({},{})", pTechno->GetType()->get_ID(), cellX, cellY);
	lua_pushboolean(L, 1);
	return 1;
}

// obj:Hunt() - enter aggressive auto-target mode.
int Techno_Hunt(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
		return 0;

	FootClass* pFoot = AsFoot(pTechno);
	if (!pFoot)
		return 0;

	pFoot->QueueMission(Mission::Hunt, true);
	return 0;
}

// obj:IsIdle() -> bool (Guard / Stop / Sleep missions)
int Techno_IsIdle(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	FootClass* pFoot = AsFoot(pTechno);
	if (!pFoot)
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	Mission m = pFoot->CurrentMission;
	lua_pushboolean(L, (m == Mission::Guard || m == Mission::Stop || m == Mission::Sleep) ? 1 : 0);
	return 1;
}

const luaL_Reg kTechnoMethods[] = {
	{ "GetTypeName",   Techno_GetTypeName   },
	{ "GetHealth",     Techno_GetHealth     },
	{ "GetMaxHealth",  Techno_GetMaxHealth  },
	{ "GetOwner",      Techno_GetOwner      },
	{ "GetPosition",   Techno_GetPosition   },
	{ "IsAlive",       Techno_IsAlive       },
	{ "GetDistanceTo", Techno_GetDistanceTo },
	{ "GetId",         Techno_GetId         },
	{ "GetKind",       Techno_GetKind       },
	{ "Scatter",       Techno_Scatter       },
	{ "MoveTo",        Techno_MoveTo        },
	{ "Hunt",          Techno_Hunt          },
	{ "IsIdle",        Techno_IsIdle        },
	{ "TakeDamage",    Techno_TakeDamage    },
	{ "Disable",       Techno_Disable       },
	{ nullptr, nullptr }
};

// --- World namespace -------------------------------------------------------

template <typename T>
int CollectArray(lua_State* L, DynamicVectorClass<T*>& array)
{
	lua_createtable(L, static_cast<int>(array.Count), 0);
	int n = 0;
	for (int i = 0; i < array.Count; ++i)
	{
		T* pItem = array.Items[i];
		if (!pItem)
			continue;
		LuaAPI::PushTechno(L, pItem);
		lua_seti(L, -2, ++n);
	}
	return 1;
}

// World.GetBuildings() -> table of techno objects
int World_GetBuildings(lua_State* L)
{
	return CollectArray(L, *BuildingClass::Array);
}

// World.GetUnits() -> table of all mobile technos (vehicles + infantry +
// aircraft), i.e. every entry of TechnoClass::Array that is not a building.
int World_GetUnits(lua_State* L)
{
	lua_createtable(L, static_cast<int>(TechnoClass::Array->Count), 0);
	int n = 0;
	for (int i = 0; i < TechnoClass::Array->Count; ++i)
	{
		auto* pItem = TechnoClass::Array->Items[i];

		if (!pItem || pItem->WhatAmI() == AbstractType::Building)
			continue;

		LuaAPI::PushTechno(L, pItem);
		lua_seti(L, -2, ++n);
	}
	return 1;
}

// Checks whether the pointer is still present in the engine's active object
// arrays. Only compares addresses - never dereferences ptr.
bool StillExists(TechnoClass* ptr)
{
	if (!ptr)
		return false;

	for (int i = 0; i < BuildingClass::Array->Count; ++i)
		if (BuildingClass::Array->Items[i] == ptr) return true;
	for (int i = 0; i < UnitClass::Array->Count; ++i)
		if (UnitClass::Array->Items[i] == ptr) return true;
	for (int i = 0; i < InfantryClass::Array->Count; ++i)
		if (InfantryClass::Array->Items[i] == ptr) return true;
	for (int i = 0; i < AircraftClass::Array->Count; ++i)
		if (AircraftClass::Array->Items[i] == ptr) return true;

	return false;
}

void LuaAPI::ProcessObjects(unsigned int currentFrame)
{
	for (auto it = g_disabledEntries.begin(); it != g_disabledEntries.end();)
	{
		// Validate BEFORE any dereference: objects destroyed by damage/victory
		// are freed by the engine and must never be touched again.
		bool alive = StillExists(it->ptr) && it->ptr->Health > 0;
		if (!alive)
		{
			it = g_disabledEntries.erase(it); // dangling or dead: drop silently
			continue;
		}

		if (currentFrame >= it->expiryFrame)
		{
			if (it->isBuilding)
			{
				auto* pBuilding = static_cast<BuildingClass*>(it->ptr);
				pBuilding->EnableStuff();
				pBuilding->HasPower = it->hadPower; // restore pre-blackout state
				if (pBuilding->Deactivated)
					pBuilding->Deactivated = false;
			}
			else if (it->ptr->Deactivated)
			{
				it->ptr->Deactivated = false; // ParalysisTimer expires on its own
			}
			Debug::LogInfo("[Combat] EMP Lock removed from {}", it->ptr->GetType()->get_ID());
			it = g_disabledEntries.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void LuaAPI::RegisterTechnoBindings(lua_State* L)
{
	// Userdata metatable
	luaL_newmetatable(L, kMetaName);

	lua_newtable(L);
	luaL_setfuncs(L, kTechnoMethods, 0);
	lua_setfield(L, -2, "__index");

	lua_pop(L, 1); // pop metatable

	// Global "World" namespace
	lua_newtable(L);
	lua_pushcfunction(L, World_GetBuildings);
	lua_setfield(L, -2, "GetBuildings");
	lua_pushcfunction(L, World_GetUnits);
	lua_setfield(L, -2, "GetUnits");
	lua_setglobal(L, "World");
}
#pragma endregion

#pragma region House
HouseClass* CheckHouse(lua_State* L, int idx)
{
	void* ud = luaL_checkudata(L, idx, kMetaName);
	auto* pHouse = *static_cast<HouseClass**>(ud);
	if (!pHouse)
	{
		luaL_error(L, "house object is no longer valid");
		return nullptr;
	}
	return pHouse;
}

HouseClass** NewHouse(lua_State* L, HouseClass* pHouse)
{
	auto* ud = static_cast<HouseClass**>(lua_newuserdatauv(L, sizeof(HouseClass*), 0));
	*ud = pHouse;
	luaL_getmetatable(L, kMetaName);
	lua_setmetatable(L, -2);
	return ud;
}

int LuaAPI::PushHouse(lua_State* L, HouseClass* pHouse)
{
	if (!pHouse)
		return 0;
	NewHouse(L, pHouse);
	return 1;
}

// House.GetPlayer() -> house | nil
int House_GetPlayer(lua_State* L)
{
	HouseClass* pHouse = HouseClass::CurrentPlayer;
	if (!pHouse)
		return 0; // nil

	NewHouse(L, pHouse);
	return 1;
}

// House.GetCount() -> int
int House_GetCount(lua_State* L)
{
	lua_pushinteger(L, HouseClass::Array->Count);
	return 1;
}

// House.GetByIndex(idx) -> house | nil
int House_GetByIndex(lua_State* L)
{
	lua_Integer idx = luaL_checkinteger(L, 1);
	if (idx < 0 || idx >= HouseClass::Array->Count)
	{
		Debug::LogInfo("House.GetByIndex({}) out of range (count={})", idx, HouseClass::Array->Count);
		return 0; // nil
	}

	HouseClass* pHouse = HouseClass::Array->Items[static_cast<int>(idx)];
	if (!pHouse)
		return 0;

	NewHouse(L, pHouse);
	return 1;
}

// --- instance methods ------------------------------------------------------

// house:GetCredits() -> int
int House_GetCredits(lua_State* L)
{
	HouseClass* pHouse = CheckHouse(L, 1);
	lua_pushinteger(L, static_cast<lua_Integer>(pHouse->Available_Money()));
	return 1;
}

// house:SetCredits(amount)
int House_SetCredits(lua_State* L)
{
	HouseClass* pHouse = CheckHouse(L, 1);
	lua_Integer target = luaL_checkinteger(L, 2);

	long current = pHouse->Available_Money();
	long delta = static_cast<long>(target) - current;
	if (delta != 0)
		pHouse->TransactMoney(delta);

	Debug::LogInfo("[House] {} credits set to {} (delta {:+})", pHouse->get_ID(), target, delta);
	return 0;
}

// house:AddCredits(delta)
int House_AddCredits(lua_State* L)
{
	HouseClass* pHouse = CheckHouse(L, 1);
	lua_Integer delta = luaL_checkinteger(L, 2);

	if (delta != 0)
		pHouse->TransactMoney(static_cast<long>(delta));

	Debug::LogInfo("[House] {} credits adjusted ({:+})", pHouse->get_ID(), delta);
	return 0;
}

// house:GetPowerOutput() -> int
int House_GetPowerOutput(lua_State* L)
{
	HouseClass* pHouse = CheckHouse(L, 1);
	lua_pushinteger(L, pHouse->PowerOutput);
	return 1;
}

// house:GetPowerDrain() -> int
int House_GetPowerDrain(lua_State* L)
{
	HouseClass* pHouse = CheckHouse(L, 1);
	lua_pushinteger(L, pHouse->PowerDrain);
	return 1;
}

// house:GetName() -> string
int House_GetName(lua_State* L)
{
	HouseClass* pHouse = CheckHouse(L, 1);
	lua_pushstring(L, pHouse->get_ID());
	return 1;
}

// house:IsHuman() -> bool
int House_IsHuman(lua_State* L)
{
	HouseClass* pHouse = CheckHouse(L, 1);
	lua_pushboolean(L, pHouse->IsControlledByHuman() ? 1 : 0);
	return 1;
}

// house:IsAlliedWith(other_house) -> bool
int House_IsAlliedWith(lua_State* L)
{
	HouseClass* pSelf = CheckHouse(L, 1);

	void* ud = luaL_testudata(L, 2, kMetaName);
	if (!ud)
		return luaL_argerror(L, 2, "expected a house object");

	auto* pOther = *static_cast<HouseClass**>(ud);
	if (!pSelf || !pOther)
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	lua_pushboolean(L, pSelf->IsAlliedWith(pOther) ? 1 : 0);
	return 1;
}

const luaL_Reg kHouseMethods[] = {
	{ "GetCredits",     House_GetCredits     },
	{ "SetCredits",     House_SetCredits     },
	{ "AddCredits",     House_AddCredits     },
	{ "GetPowerOutput", House_GetPowerOutput },
	{ "GetPowerDrain",  House_GetPowerDrain  },
	{ "GetName",        House_GetName        },
	{ "IsHuman",        House_IsHuman        },
	{ "IsAlliedWith",   House_IsAlliedWith   },
	{ nullptr, nullptr }
};

void LuaAPI::RegisterHouseBindings(lua_State* L)
{
	// Userdata metatable
	luaL_newmetatable(L, kMetaName);

	// metatable.__index points to the methods table
	lua_newtable(L);
	luaL_setfuncs(L, kHouseMethods, 0);
	lua_setfield(L, -2, "__index");

	lua_pop(L, 1); // pop metatable

	// Global "House" namespace
	lua_newtable(L);
	lua_pushcfunction(L, House_GetPlayer);
	lua_setfield(L, -2, "GetPlayer");
	lua_pushcfunction(L, House_GetCount);
	lua_setfield(L, -2, "GetCount");
	lua_pushcfunction(L, House_GetByIndex);
	lua_setfield(L, -2, "GetByIndex");
	lua_setglobal(L, "House");
}

#pragma endregion