#include "Phobos.Lua.h"

#include <map>
#include <string>
#include "Phobos.h"

#include <Ext/Script/Lua/Wrapper.h>

#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <Misc/Ares/Hooks/Header.h>

#include <MessageBoxLogging.h>

// TODO : encryption support
// Otamaa : change this variable if you want to load desired name lua file
std::string filename = "\\renameinternal.lua";
std::string LuaData::LuaDir;
std::string CoreHandles;
HelperedVector<std::pair<uintptr_t, std::string>> map_replaceAddrTo;
std::string MainWindowStr;
std::map<std::string, HANDLE> SafeFiles {};
bool IsActive;

auto MessageLog = [](const std::string& first , const std::string& second) {
	std::string fmt__ = fmt::format("fail to load {} && cause {}", first, second);
	MessageBoxA(0, fmt__.c_str(), "Debug", MB_OK);
};


bool load_lua_file(lua_State* L, const std::string& path, std::function<void(const std::string&, const std::string&)> f)
{
	if (luaL_dofile(L, path.c_str()) != LUA_OK)
	{
		if (f){
			f(path, lua_tostring(L, -1));
			lua_pop(L, 1);
		}
		return false;
	}
	return true;
}

inline bool lua_get_global_string(lua_State* L, const char* name, std::string& result)
{
	lua_getglobal(L, name);
	if (lua_isstring(L, -1)) {
		result = lua_tostring(L, -1);
	}
	lua_pop(L, 1);

	return !result.empty();
}

inline bool lua_get_global_string(lua_State* L, const char* name, std::wstring& result)
{
	lua_getglobal(L, name);
	if (lua_isstring(L, -1)) {
		result = PhobosCRT::StringToWideString(lua_tostring(L, -1));
	}

	lua_pop(L, 1);

	return !result.empty();
}

inline void lua_get_global_bool(lua_State* L, const char* name, bool& fallback)
{
	lua_getglobal(L, name);

	if (lua_isboolean(L, -1)) {
		fallback = lua_toboolean(L, -1);
	}
	lua_pop(L, 1);

}

using uintptr_string_pair = std::pair<uintptr_t, std::string>;

inline std::vector<uintptr_string_pair> lua_read_ptr_string_array(lua_State* L, const char* global_table_name)
{
	std::vector<uintptr_string_pair> results;

	lua_getglobal(L, global_table_name);
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return results;
	}

	lua_pushnil(L); // first key
	while (lua_next(L, -2) != 0)
	{
		// now at: -1 = value (subtable), -2 = key (index)
		if (lua_istable(L, -1))
		{
			lua_rawgeti(L, -1, 1); // get [1]
			lua_rawgeti(L, -2, 2); // get [2]

			if (lua_isnumber(L, -2) && lua_isstring(L, -1))
			{
				uintptr_t ptr = static_cast<uintptr_t>(lua_tointeger(L, -2));
				std::string name = lua_tostring(L, -1);
				results.emplace_back(ptr, name);
			}

			lua_pop(L, 2); // pop [2], [1]
		}

		lua_pop(L, 1); // pop value
	}

	lua_pop(L, 1); // pop MyArray table
	return results;
}

inline void lua_get_string_array_of_SafeFiles(lua_State* L, const char* global_table_name)
{
	lua_getglobal(L, global_table_name);
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return;
	}

	lua_pushnil(L); // first key
	while (lua_next(L, -2) != 0)
	{
		// stack: -1 => value, -2 => key
		if (lua_isstring(L, -1)) {
			std::string val = lua_tostring(L, -1);
			PhobosCRT::uppercase(val);
			SafeFiles[val] = NULL;
		}

		lua_pop(L, 1); // pop value
	}

	lua_pop(L, 1); // pop the table
}

void Phobos::ExecuteLua()
{
	make_unique_luastate(unique_lua);
	auto L = unique_lua.get();
	luaL_openlibs(L);

	if (load_lua_file(L, LuaData::LuaDir + "\\AdminMode.lua" , nullptr))
	{
		std::string adminName {};
		if (lua_get_global_string(L, "AdminMode", adminName))
		{
			if (adminName.size() <= MAX_COMPUTERNAME_LENGTH + 1)
			{
				DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
				TCHAR PCName[MAX_COMPUTERNAME_LENGTH + 1];
				GetComputerName(PCName, &dwSize);

				if (IS_SAME_STR_(PCName, adminName.c_str())) {
					Phobos::Config::MultiThreadSinglePlayer = false;
					Phobos::Config::DebugFatalerrorGenerateDump = true;
					//Phobos::Otamaa::ReplaceGameMemoryAllocator = true;
					Phobos::Otamaa::IsAdmin = true;
				}
			}
		}
	}

	auto _renamer = LuaData::LuaDir + filename;

	if (load_lua_file(L, _renamer, nullptr))
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

		lua_pop(L, 1);

		if (lua_get_global_string(L,"MainWindowString", MainWindowStr)) {
			Patch::Apply_OFFSET(0x777CC6, (uintptr_t)MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777CCB, (uintptr_t)MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777D6D, (uintptr_t)MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777D72, (uintptr_t)MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777CA1, (uintptr_t)MainWindowStr.c_str());
		}

		//lua_get_string_array_of_SafeFiles(L, "FetchHandles");
		//lua_get_global_string(L, "CoreHandles", CoreHandles);

		//IsActive = !SafeFiles.empty() && !CoreHandles.empty();

		lua_get_global_string(L, "MovieMDINI", StaticVars::MovieMDINI);
		lua_get_global_string(L, "DebugLogName", Debug::LogFileMainName);
		lua_get_global_string(L, "CrashDumpFileName", Debug::CrashDumpFileName);
		lua_get_global_string(L, "DesyncLogName", Debug::SyncFileFormat);
		lua_get_global_string(L, "DesyncLogName2", Debug::SyncFileFormat2);
		lua_get_global_bool(L, "CompatibilityMode", Phobos::Otamaa::CompatibilityMode);
		lua_get_global_bool(L, "ReplaceGameMemoryAllocator", Phobos::Otamaa::ReplaceGameMemoryAllocator);
		lua_get_global_bool(L, "AllowMultipleInstances", Phobos::Otamaa::AllowMultipleInstance);

	}
}

void LuaData::ApplyCore(char* pBuffer, size_t buffersize) {
	if (CoreHandles.empty()) return;

	size_t key_len = CoreHandles.length();
	for (size_t i = 0; i < buffersize; ++i) {
		pBuffer[i] ^= CoreHandles[i % key_len];
	}
}

#include <filesystem>

//#pragma optimize("", off )
//HANDLE __stdcall _CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
//{
//	auto fileHandle = CreateFileA(lpFileName, GENERIC_READ | GENERIC_WRITE, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
//
//	auto it = SafeFiles.find(lpFileName);
//	if(it != SafeFiles.end()) {
//		LARGE_INTEGER fileSize;
//		if (!GetFileSizeEx(fileHandle, &fileSize) || fileSize.QuadPart == 0) {
//			auto fmt = std::format("CreateFileMapping failed: {}", lpFileName);
//			MessageBoxA(0, fmt.c_str(), "Debug", MB_OK);
//			return fileHandle;
//		}
//
//		HANDLE hMap = CreateFileMapping(fileHandle, nullptr, PAGE_READWRITE, 0, 0, nullptr);
//		if (!hMap) {
//			DWORD err = GetLastError();
//			auto fmt = std::format("CreateFileMapping failed: {}" , err);
//			MessageBoxA(0, fmt.c_str(), "Debug", MB_OK);
//		}
//
//		LPVOID fileData = MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
//		LuaData::ApplyCore((char*)fileData, fileSize.QuadPart);
//		UnmapViewOfFile(fileData);
//		CloseHandle(hMap);
//	}
//
//	return fileHandle;
//}
//#pragma optimize("", on )


void LuaData::FetchHandlesAndApply()
{
	if (!IsActive)
		return;

	//Imports::CreateFileA = _CreateFileA;
}


