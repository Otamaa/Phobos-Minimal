#include "Phobos.Lua.h"

#include <map>
#include <string>
#include "Phobos.h"

#include <Ext/Script/Lua/Wrapper.h>

#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <Misc/Ares/Hooks/Header.h>

// TODO : encryption support
// Otamaa : change this variable if you want to load desired name lua file
std::string filename = "\\renameinternal.lua";
std::string LuaData::LuaDir;
HelperedVector<std::pair<uintptr_t, std::string>> map_replaceAddrTo;
std::string MainWindowStr;

void Phobos::ExecuteLua()
{
	make_unique_luastate(unique_lua);
	auto L = unique_lua.get();
	luaL_openlibs(L);

	if (luaL_dofile(L, (LuaData::LuaDir + "\\AdminMode.lua").c_str()) == LUA_OK)
	{
		lua_getglobal(L, "AdminMode");
		if (lua_isstring(L, -1) == 1)
		{
			std::string adminName = lua_tostring(L, -1);

			if (adminName.size() <= MAX_COMPUTERNAME_LENGTH + 1)
			{
				DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
				TCHAR PCName[MAX_COMPUTERNAME_LENGTH + 1];
				GetComputerName(PCName, &dwSize);

				if (IS_SAME_STR_(PCName, adminName.c_str()))
				{
					Phobos::EnableConsole = true;
					//Phobos::Config::MultiThreadSinglePlayer = true;
					Phobos::Config::DebugFatalerrorGenerateDump = true;
					Phobos::Otamaa::IsAdmin = true;
				}
			}
		}
	}

	std::string replaces_filename = LuaData::LuaDir + filename;
	if (luaL_dofile(L, replaces_filename.c_str()) == LUA_OK)
	{
		lua_getglobal(L, "Replaces");

		if (lua_istable(L, -1))
		{
			const size_t replace_size = (size_t)lua_rawlen(L, -1);
			for (size_t i = 0; i < replace_size; i++)
			{
				lua_pushinteger(L, lua_Integer(i + 1));
				lua_gettable(L, -2);
				if (lua_istable(L, -2))
				{
					lua_pushstring(L, "Addr");
					lua_gettable(L, -2);
					const auto addr = (uintptr_t)lua_tointeger(L, -1);
					lua_pop(L, 1);

					// i dont know if the address is correct 
					// this is assuming that player not using some kind of weird modded gamemd.exe
					if (addr > 0 && addr >= 0x401000 && addr <= 0xB79BE4)
					{
						std::pair<uintptr_t, std::string>* result = nullptr;

						{
							bool found = false;
							for (auto begin = map_replaceAddrTo.begin(); begin != map_replaceAddrTo.end(); ++begin)
							{
								if (begin->first == addr)
								{
									result = begin.operator->();
									found = true;
									break;
								}
							}

							if (!found)
								result = &map_replaceAddrTo.emplace_back(addr, "");
						}

						const auto maxlen = strlen((const char*)result->first);
						lua_pushstring(L, "To");
						lua_gettable(L, -2);
						result->second = lua_tostring(L, -1);
						lua_pop(L, 1);

						DWORD protectFlag;
						if (Phobos::Otamaa::IsAdmin) {
							std::string copy = PhobosCRT::trim(result->second.c_str());
							Debug::LogDeferred("Patching string [%d] [0x%x - %s (%d) - max %d]\n", i, addr, copy.c_str(), result->second.size(), maxlen);
						}

						// do not exceed maximum length of the string , otherwise it will broke the .exe file
						Patch::Apply_withmemcpy(addr, result->second.c_str(), protectFlag, PAGE_READWRITE, (size_t)maxlen);
					}

				}
				lua_pop(L, 1);
			}
		}

		lua_getglobal(L, "MainWindowString");

		if (lua_isstring(L, -1) == 1)
		{
			MainWindowStr = lua_tostring(L, -1);
			Patch::Apply_OFFSET(0x777CC6, (uintptr_t)MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777CCB, (uintptr_t)MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777D6D, (uintptr_t)MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777D72, (uintptr_t)MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777CA1, (uintptr_t)MainWindowStr.c_str());
		}

		lua_getglobal(L, "MovieMDINI");

		if (lua_isstring(L, -1) == 1)
		{
			StaticVars::MovieMDINI = lua_tostring(L, -1);
		}

		lua_getglobal(L, "CompatibilityMode");

		if (lua_isboolean(L, -1) == 1)
		{
			Phobos::Otamaa::CompatibilityMode = lua_toboolean(L, -1);
		}

		lua_getglobal(L, "ReplaceGameMemoryAllocator");

		if (lua_isboolean(L, -1) == 1)
		{
			Phobos::Otamaa::ReplaceGameMemoryAllocator = lua_toboolean(L, -1);
		}

		lua_getglobal(L, "AllowMultipleInstances");

		if (lua_isboolean(L, -1) == 1) {
			Phobos::Otamaa::AllowMultipleInstance = lua_toboolean(L, -1);
		}
	}

	LuaBridge::InitScriptLuaList(unique_lua);
}
