#include "Phobos.Lua.h"

#include "Phobos.h"

#include <Ext/Script/Lua/Wrapper.h>

#include <Ext/Team/Body.h>

#include <TechnoClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>
#include <FootClass.h>
#include <WarheadTypeClass.h>
#include <CellClass.h>

#include <MapClass.h>
#include <HouseClass.h>

#include <Utilities/Debug.h>

#include <exception>
#include <vector>
#include <string>
#include <cstdint>
#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include <MessageBoxLogging.h>
#include <MixFileClass.h>

#include <Misc/PhobosGlobal.h>

// TODO : encryption support
//		: some custom file name are disabled after new exception
// Otamaa : change this variable if you want to load desired name lua file
std::string LuaData::filename = "\\renameinternal.lua";
std::string LuaData::LuaDir;
std::string LuaData::CoreHandles;
std::vector<std::pair<uintptr_t, std::string>> LuaData::map_replaceAddrTo;
std::string LuaData::AdditionalStringTableFmt { "stringtable{:02}.csf" };
std::string LuaData::MainWindowStr;
std::string LuaData::FontName = GameStrings::GAME_FNT();
std::string LuaData::StatisticPacketName = "stats.dmp";

std::map<std::string, bool> LuaData::SafeFiles;
bool LuaData::IsActive;

auto MessageLog = [](const std::string& first, const std::string& second)
	{
		std::string fmt__ = fmt::format("fail to load {} && cause {}", first, second);
		MessageBoxA(0, fmt__.c_str(), "Debug", MB_OK);
	};

// RC4 stream cipher encryption/decryption
void rc4_crypt(std::vector<char>& data, const std::string& key)
{
	uint8_t S[256];
	for (int i = 0; i < 256; ++i) S[i] = i;

	uint8_t j = 0;
	for (int i = 0; i < 256; ++i)
	{
		j += S[i] + static_cast<uint8_t>(key[i % key.size()]);
		std::swap(S[i], S[j]);
	}

	uint8_t i = 0;
	j = 0;
	for (size_t k = 0; k < data.size(); ++k)
	{
		i += 1;
		j += S[i];
		std::swap(S[i], S[j]);
		uint8_t rnd = S[(S[i] + S[j]) & 0xFF];
		data[k] ^= rnd;
	}
}

#pragma region TEA
// TEA core encrypts 64-bit block (8 bytes)
void tea_encrypt_block(uint32_t* v, const uint32_t* k)
{
	uint32_t v0 = v[0], v1 = v[1];
	uint32_t sum = 0;
	const uint32_t delta = 0x9E3779B9;
	for (int i = 0; i < 32; ++i)
	{
		sum += delta;
		v0 += ((v1 << 4) + k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + k[1]);
		v1 += ((v0 << 4) + k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + k[3]);
	}
	v[0] = v0;
	v[1] = v1;
}

void tea_decrypt_block(uint32_t* v, const uint32_t* k)
{
	uint32_t v0 = v[0], v1 = v[1];
	uint32_t sum = 0x9E3779B9 * 32;
	const uint32_t delta = 0x9E3779B9;
	for (int i = 0; i < 32; ++i)
	{
		v1 -= ((v0 << 4) + k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + k[3]);
		v0 -= ((v1 << 4) + k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + k[1]);
		sum -= delta;
	}
	v[0] = v0;
	v[1] = v1;
}

void tea_encrypt_buffer(std::vector<char>& buffer, const uint32_t k[4])
{
	for (size_t i = 0; i < buffer.size(); i += 8)
	{
		uint32_t v[2];
		std::memcpy(v, buffer.data() + i, 8);
		tea_encrypt_block(v, k);
		std::memcpy(buffer.data() + i, v, 8);
	}
}

void tea_decrypt_buffer(std::vector<char>& buffer, const uint32_t k[4])
{
	for (size_t i = 0; i < buffer.size(); i += 8)
	{
		uint32_t v[2];
		std::memcpy(v, buffer.data() + i, 8);
		tea_decrypt_block(v, k);
		std::memcpy(buffer.data() + i, v, 8);
	}
}

// Convert std::string to TEA 128-bit key (4 * 32-bit)
void key_from_string(const std::string& key, uint32_t k[4])
{
	std::memset(k, 0, 4 * sizeof(uint32_t));
	for (size_t i = 0; i < 16 && i < key.size(); ++i)
	{
		reinterpret_cast<uint8_t*>(&k[i / 4])[i % 4] = static_cast<uint8_t>(key[i]);
	}
}

#pragma endregion

std::string crc_to_mask(uint32_t crc) {
	return fmt::format("{:08x}", crc);
}

std::string get_dll_name()
{
	char path[MAX_PATH] = {};
	HMODULE hModule = nullptr;

	if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		reinterpret_cast<LPCSTR>(&get_dll_name),
		&hModule))
	{
		GetModuleFileNameA(hModule, path, MAX_PATH);
		std::string full_path = std::string(path);
		size_t last_slash = full_path.find_last_of("/\\");
		if (last_slash != std::string::npos)
		{
			return full_path.substr(last_slash + 1); // filename only
		}
		return full_path;
	}
	return {};
}

std::string derive_key(const std::string& first, uint32_t crc, const std::string& salt = "")
{
	std::string mask = crc_to_mask(crc);
	std::string thesalt = salt.empty() ? get_dll_name() : salt;
	std::string combined_key = first + thesalt;
	for (size_t i = 0; i < combined_key.size(); ++i)
	{
		combined_key[i] ^= mask[i % mask.size()];
	}
	return combined_key;
}

void ApplyCore(char* pBuffer, char* content, size_t size)
{
	if (LuaData::CoreHandles.empty()) return;

	size_t key_len = LuaData::CoreHandles.length();
	for (size_t i = 0; i < size; ++i)
	{
		pBuffer[i] = content[i] ^ LuaData::CoreHandles[i % key_len];
	}
}

void ApplyCore(char* content, size_t size)
{
	if (LuaData::CoreHandles.empty()) return;

	size_t key_len = LuaData::CoreHandles.length();
	for (size_t i = 0; i < size; ++i)
	{
		content[i] ^= LuaData::CoreHandles[i % key_len];
	}
}

void ApplyCore(std::vector<char> content, std::string key)
{
	if (key.empty()) return;

	size_t key_len = key.length();
	for (size_t i = 0; i < content.size(); ++i)
	{
		content[i] ^= key[i % key_len];
	}
}

void Transform_buffer(std::vector<char>& buffer, std::string key, uint32_t crc = 0)
{
	if (buffer.size() % 8 != 0)
	{
		//throw std::invalid_argument("Buffer size must be multiple of 8 bytes.");
		return;
	}

	key = derive_key(key, crc);

	ApplyCore(buffer, key);
	rc4_crypt(buffer, key);

	uint32_t k[4];
	key_from_string(key, k);
	tea_encrypt_buffer(buffer, k);
}

void UnTranform_buffer(std::vector<char>& buffer, std::string key, uint32_t crc = 0)
{
	if (buffer.size() % 8 != 0)
	{
		//throw std::invalid_argument("Buffer size must be multiple of 8 bytes.");
		return;
	}

	key = derive_key(key, crc);

	uint32_t k[4];
	key_from_string(key, k);
	tea_decrypt_buffer(buffer, k);

	rc4_crypt(buffer, key);
	ApplyCore(buffer, key);
}

// void* __fastcall FakeFileLoader::_Retrieve(const char* pFilename, bool bLoadAsSHP)
// {
// 	//void* pData = FakeFileLoader::Retrieve(pFilename, bLoadAsSHP);

// 	//if (pData && IsActive)
// 	//{
// 	//	if (pFilename)
// 	//	{
// 	//		auto it = SafeFiles.find(pFilename);

// 	//		if (it != SafeFiles.end())
// 	//		{
// 	//			long fileSize = 0;
// 	//			if (MixFileClass::Offset(pFilename, nullptr, nullptr, nullptr, &fileSize)) {
// 	//				if (fileSize > 0) {
// 	//					ApplyCore(static_cast<char*>(pData), static_cast<size_t>(fileSize));
// 	//				}
// 	//			}
// 	//		}
// 	//	}
// 	//}

// 	return FakeFileLoader::Retrieve(pFilename, bLoadAsSHP);
// }

struct FakeFileData
{
	LPVOID memory;
	HANDLE mapping;
	size_t size;
};

std::unordered_map<std::string, FakeFileData> keeper;
struct HandleData
{
	DWORD fileActualSize;
	LPVOID  basePointer;
	DWORD currentoffset;
};

std::unordered_map<HANDLE, HandleData> HandleDataKeeper;

HANDLE __stdcall _CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{

	auto it = LuaData::SafeFiles.find(lpFileName);
	if(it != LuaData::SafeFiles.end()) {
		auto it_cache = &keeper[lpFileName];

		if (!it_cache->memory)
		{
			auto fileHandle = CreateFileA(lpFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			LARGE_INTEGER fileSize;
			if (!GetFileSizeEx(fileHandle, &fileSize) || fileSize.QuadPart == 0) {
				return fileHandle;
			}

			HANDLE hMap = CreateFileMapping(fileHandle, nullptr, PAGE_READWRITE, 0, 0, nullptr);
			if (!hMap) {
				return fileHandle;
			}

			LPVOID fileData = MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

			if (!fileData) {
				CloseHandle(hMap);
				return fileHandle;
			}

			it_cache->size = fileSize.QuadPart;
			it_cache->mapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, it_cache->size, NULL);
			it_cache->memory = MapViewOfFile(it_cache->mapping, FILE_MAP_ALL_ACCESS, 0, 0, it_cache->size);
			ApplyCore((char*)it_cache->memory, (char*)fileData, it_cache->size);
			UnmapViewOfFile(fileData);
			CloseHandle(hMap);
		}

		HANDLE duplicatedHandle;
		if (!DuplicateHandle(
			Patch::CurrentProcess,												// source process
			it_cache->mapping,									// source handle
			Patch::CurrentProcess,												// target process
			&duplicatedHandle,                                   // out duplicated handle
			0,                                                   // desired access (same)
			FALSE,                                               // inherit handle
			DUPLICATE_SAME_ACCESS))								// options
		{
			return INVALID_HANDLE_VALUE;
		}

		// Store size info
		auto mapped = &HandleDataKeeper[duplicatedHandle];
		mapped->fileActualSize  = it_cache->size;
		mapped->basePointer = it_cache->memory;
		mapped->currentoffset = 0;
		return duplicatedHandle;
	}

	return CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

BOOL __stdcall _CloseHandle(HANDLE hObject)
{
	HandleDataKeeper.erase(hObject);
	return CloseHandle(hObject);
}

DWORD __stdcall _GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh)
{
	const auto it = HandleDataKeeper.find(hFile);

	if (it != HandleDataKeeper.end()) {
		if (lpFileSizeHigh) *lpFileSizeHigh = 0;
		return it->second.fileActualSize;
	}

	return GetFileSize(hFile, lpFileSizeHigh);
}

BOOL WINAPI _ReadFile(
	HANDLE hFile,
	LPVOID lpBuffer,
	DWORD nNumberOfBytesToRead,
	LPDWORD lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
)
{
	auto it = HandleDataKeeper.find(hFile);

	if (it != HandleDataKeeper.end())
	{
		if (it->second.currentoffset >= it->second.fileActualSize) {
			if (lpNumberOfBytesRead) *lpNumberOfBytesRead = 0;
			return TRUE;
		}

		DWORD toRead = MinImpl(nNumberOfBytesToRead, it->second.fileActualSize - it->second.currentoffset);
		std::memcpy(lpBuffer, reinterpret_cast<uint8_t*>(it->second.basePointer) + it->second.currentoffset, toRead);
		it->second.currentoffset += toRead;
		if (lpNumberOfBytesRead) *lpNumberOfBytesRead = toRead;
		return TRUE;
	}
	return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

DWORD WINAPI _SetFilePointer(
	HANDLE hFile,
	LONG lDistanceToMove,
	PLONG lpDistanceToMoveHigh,
	DWORD dwMoveMethod
)
{
	auto it = HandleDataKeeper.find(hFile);

	if (it != HandleDataKeeper.end())
	{
		DWORD size = it->second.fileActualSize;
		LONG high = lpDistanceToMoveHigh ? *lpDistanceToMoveHigh : 0;
		LONGLONG newOffset = it->second.currentoffset;
		LONGLONG move = ((LONGLONG)high << 32) | (DWORD)lDistanceToMove;

		switch (dwMoveMethod)
		{
		case FILE_BEGIN:  newOffset = move; break;
		case FILE_CURRENT: newOffset += move; break;
		case FILE_END:    newOffset = size + move; break;
		default: return INVALID_SET_FILE_POINTER;
		}

		if (newOffset < 0 || newOffset > size)
			return INVALID_SET_FILE_POINTER;

		it->second.currentoffset = static_cast<DWORD>(newOffset);
		if (lpDistanceToMoveHigh) *lpDistanceToMoveHigh = 0;
		return it->second.currentoffset;
	}

	return SetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
}

BOOL WINAPI _SetFileTime(
	HANDLE hFile,
	const FILETIME* lpCreationTime,
	const FILETIME* lpLastAccessTime,
	const FILETIME* lpLastWriteTime
)
{
	if (HandleDataKeeper.find(hFile) != HandleDataKeeper.end()) {
		return TRUE;
	}

	return SetFileTime(hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime);
}


//ASMJIT_PATCH(0x473B36, CCFIleClass_ReadBuffer, 0x6)
//{
//	GET(CCFileClass*, pFile, ESI);
//
//	auto it = keeper.find(pFile->FileName);
//	if (it != keeper.end()) {
//		std::memcpy
//		ApplyCore((char*)pFile->Buffer.Buffer, pFile->Buffer.Size);
//	}
//
//	return 0x0;
//}

#pragma region _Retrieve


#pragma endregion

using uintptr_string_pair = std::pair<uintptr_t, std::string>;
#include <BitFont.h>

class NOVTABLE _BitFontWrapper
{
public :

	static BitFont* __fastcall _CreateMe(BitFont* ptr ,discard_t , const char* discarded) {
		ptr->BitFont::BitFont(LuaData::FontName.c_str());
		return ptr;
	}
};
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
		if (lua_isstring(L, -1))
		{
			std::string val = lua_tostring(L, -1);
			PhobosCRT::uppercase(val);
			LuaData::SafeFiles[val] = false;
		}

		lua_pop(L, 1); // pop value
	}

	lua_pop(L, 1); // pop the table
}

struct LuaWrapper
{
	LuaWrapper() : Internal {}
	{
		this->Internal.reset(luaL_newstate());
		luaL_openlibs(this->Internal.get());
	}

	~LuaWrapper() = default;

	bool loadfile(const std::string& path, std::function<void(const std::string&, const std::string&)> f)
	{
		auto L = Internal.get();

		if (luaL_dofile(L, path.c_str()) != LUA_OK)
		{
			if (f)
			{
				f(path, lua_tostring(L, -1));
				lua_pop(L, 1);
			}
			return false;
		}
		return true;
	}

	bool getGlobalString(const char* name, std::string& result, bool validate)
	{
		auto L = Internal.get();
		lua_getglobal(L, name);
		if (lua_isstring(L, -1))
		{
			result = lua_tostring(L, -1);
		}
		lua_pop(L, 1);

		if (validate && result.empty()) {
			Debug::FatalError("%s is Invalid !", name);
		}

		return !result.empty();
	}

	bool getGlobalString(const char* name, std::wstring& result, bool validate)
	{
		auto L = Internal.get();
		lua_getglobal(L, name);
		if (lua_isstring(L, -1))
		{
			result = PhobosCRT::StringToWideString(lua_tostring(L, -1));
		}

		lua_pop(L, 1);

		if (validate && result.empty()) {
			Debug::FatalError("%s is Invalid !", name);
		}

		return !result.empty();
	}

	void getGlobalBool(const char* name, bool& fallback)
	{
		auto L = Internal.get();
		lua_getglobal(L, name);

		if (lua_isboolean(L, -1))
		{
			fallback = lua_toboolean(L, -1);
		}
		lua_pop(L, 1);

	}

	auto get()
	{
		return this->Internal.get();
	}

	unique_luastate Internal;
};

void Phobos::ExecuteLua()
{
	LuaWrapper Lua {};

	if (Lua.loadfile(LuaData::LuaDir + "\\AdminMode.lua", nullptr))
	{
		std::string adminName {};
		if (Lua.getGlobalString("AdminMode", adminName , false))
		{
			if (adminName.size() <= MAX_COMPUTERNAME_LENGTH + 1)
			{
				DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
				TCHAR PCName[MAX_COMPUTERNAME_LENGTH + 1];
				GetComputerName(PCName, &dwSize);

				if (IS_SAME_STR_(PCName, adminName.c_str()))
				{
					Phobos::Config::MultiThreadSinglePlayer = false;
					Phobos::Config::DebugFatalerrorGenerateDump = true;
					Phobos::Otamaa::OutputAudioLogs = true;
					//Phobos::Otamaa::ReplaceGameMemoryAllocator = true;
					Phobos::Otamaa::IsAdmin = true;
				}
			}
		}
	}

	const auto _renamer = LuaData::LuaDir + LuaData::filename;

	if (Lua.loadfile(_renamer, nullptr))
	{
		auto L = Lua.get();
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
							for (auto begin = LuaData::map_replaceAddrTo.begin(); begin != LuaData::map_replaceAddrTo.end(); ++begin)
							{
								if (begin->first == addr)
								{
									result = begin.operator->();
									found = true;
									break;
								}
							}

							if (!found)
								result = &LuaData::map_replaceAddrTo.emplace_back(addr, "");
						}

						const auto maxlen = std::char_traits<char>::length((const char*)result->first);
						lua_pushstring(L, "To");
						lua_gettable(L, -2);
						result->second = lua_tostring(L, -1);
						lua_pop(L, 1);

						if (Phobos::Otamaa::IsAdmin)
						{
							std::string copy = PhobosCRT::trim(result->second.c_str());
							Debug::Log("Patching string [%d] [0x%x - %s (%d) - max %d]\n", i, addr, copy.c_str(), result->second.size(), maxlen);
						}

						// do not exceed maximum length of the string , otherwise it will broke the .exe file
						Patch::WriteToProcessMemory(addr, result->second.c_str(), (size_t)maxlen);
					}
				}
				lua_pop(L, 1);
			}
		}

		lua_pop(L, 1);

		if (Lua.getGlobalString("MainWindowString", LuaData::MainWindowStr, true))
		{
			Patch::Apply_OFFSET(0x777CC5 + 1, (uintptr_t)LuaData::MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777CCA + 1, (uintptr_t)LuaData::MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777D6C + 1, (uintptr_t)LuaData::MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777D71 + 1, (uintptr_t)LuaData::MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777CA0 + 1, (uintptr_t)LuaData::MainWindowStr.c_str());
		}

		if (Lua.getGlobalString("FontName", LuaData::FontName, true))
		{
			if (LuaData::FontName.find(".fnt") == std::string::npos && LuaData::FontName.find(".FNT") == std::string::npos) {
				LuaData::FontName += ".fnt";
			}

			//Patch::Apply_CALL(0x434AEE, _BitFontWrapper::_CreateMe);
			//Patch::Apply_CALL(0x4354E5, _BitFontWrapper::_CreateMe);';
			Patch::Apply_OFFSET(0x434AE7 + 1, (uintptr_t)LuaData::FontName.c_str());
			Patch::Apply_OFFSET(0x4354DE + 1, (uintptr_t)LuaData::FontName.c_str());
		}

		//core part to activate , disable it for now
		//lua_get_string_array_of_SafeFiles(Lua.get(), "FetchHandles");
		//Lua.getGlobalString("CoreHandles", CoreHandles);

		//IsActive = !SafeFiles.empty() && !CoreHandles.empty();

		Lua.getGlobalString("MovieMDINI", PhobosGlobal::Instance()->MovieMDINI, true);
		//Lua.getGlobalString("DebugLogName", Debug::LogFileMainName, true);

		//if(Lua.getGlobalString("CrashDumpFileName", Debug::CrashDumpFileName, true)) {
		//	if (Debug::CrashDumpFileName.find(L".dmp") == std::string::npos && Debug::CrashDumpFileName.find(L".DMP") == std::string::npos) {
		//		Debug::CrashDumpFileName += L".dmp";
		//	}
		//}

		if (Lua.getGlobalString("StatisticPacketName", LuaData::StatisticPacketName, true)) {
			if (LuaData::StatisticPacketName.find(".dmp") == std::string::npos && LuaData::StatisticPacketName.find(".DMP") == std::string::npos) {
				LuaData::StatisticPacketName += ".dmp";
			}
		}

		//Lua.getGlobalString("DesyncLogName", Debug::SyncFileFormat, true);
		//Lua.getGlobalString("DesyncLogName2", Debug::SyncFileFormat2, true);
		Lua.getGlobalString("AdditionalStringtableFormat", LuaData::AdditionalStringTableFmt, true);
		Lua.getGlobalBool("CompatibilityMode", Phobos::Otamaa::CompatibilityMode);
		Lua.getGlobalBool("ReplaceGameMemoryAllocator", Phobos::Otamaa::ReplaceGameMemoryAllocator);
		Lua.getGlobalBool("AllowMultipleInstances", Phobos::Otamaa::AllowMultipleInstance);
	}
}

#include <filesystem>

DWORD WINAPI _GetFileType(HANDLE hFile)
{
	if (HandleDataKeeper.contains(hFile))
	{
		return FILE_TYPE_DISK;
	}
	return GetFileType(hFile);
}

HANDLE WINAPI _CreateFileMappingA(
	HANDLE hFile,
	LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
	DWORD flProtect,
	DWORD dwMaximumSizeHigh,
	DWORD dwMaximumSizeLow,
	LPCSTR lpName
)
{
	if (HandleDataKeeper.contains(hFile)) {
		for (auto& [_, mapped] : keeper) {
			if (mapped.mapping == hFile) {
				return mapped.mapping;
			}
		}
	}

	return CreateFileMappingA(hFile, lpFileMappingAttributes, flProtect,
							  dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
}

LPVOID WINAPI _MapViewOfFile(
	HANDLE hFileMappingObject,
	DWORD dwDesiredAccess,
	DWORD dwFileOffsetHigh,
	DWORD dwFileOffsetLow,
	SIZE_T dwNumberOfBytesToMap
)
{
	const auto it = HandleDataKeeper.find(hFileMappingObject);

	if (it != HandleDataKeeper.end()) {
		return it->second.basePointer;
	}

	return MapViewOfFile(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);
}

LPVOID WINAPI _MapViewOfFileEx(
	HANDLE hFileMappingObject,
	DWORD dwDesiredAccess,
	DWORD dwFileOffsetHigh,
	DWORD dwFileOffsetLow,
	SIZE_T dwNumberOfBytesToMap,
	LPVOID lpBaseAddress
)
{
	const auto it = HandleDataKeeper.find(hFileMappingObject);

	if (it != HandleDataKeeper.end()) {
		return it->second.basePointer;
	}

	return MapViewOfFileEx(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap, lpBaseAddress);
}

BOOL WINAPI _UnmapViewOfFile(LPCVOID lpBaseAddress)
{
	for (auto& [path, mapped] : keeper) {
		if (mapped.mapping == lpBaseAddress) {
			return TRUE;
		}
	}

	return UnmapViewOfFile(lpBaseAddress);
}

BOOL WINAPI _FlushFileBuffers(HANDLE hFile)
{
	if (HandleDataKeeper.count(hFile))
	{
		return TRUE; // fake success
	}
	return FlushFileBuffers(hFile);
}

BOOL WINAPI _GetFileInformationByHandle(HANDLE hFile, BY_HANDLE_FILE_INFORMATION* lpFileInformation)
{
	auto it = HandleDataKeeper.find(hFile);

	if (it != HandleDataKeeper.end())
	{
		ZeroMemory(lpFileInformation, sizeof(*lpFileInformation));
		lpFileInformation->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
		lpFileInformation->nFileSizeLow = it->second.fileActualSize;
		lpFileInformation->nFileSizeHigh = 0;
		return TRUE;
	}

	return GetFileInformationByHandle(hFile, lpFileInformation);
}

BOOL WINAPI _WriteFile(
	HANDLE hFile,
	LPCVOID lpBuffer,
	DWORD nNumberOfBytesToWrite,
	LPDWORD lpNumberOfBytesWritten,
	LPOVERLAPPED lpOverlapped
)
{
	auto it = HandleDataKeeper.find(hFile);

	if (it != HandleDataKeeper.end())
	{
		auto& data = it->second;
		DWORD size = data.fileActualSize;

		if (data.currentoffset >= size)
		{
			if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = 0;
			return TRUE;
		}

		DWORD toWrite = std::min(nNumberOfBytesToWrite, size - data.currentoffset);
		std::memcpy(reinterpret_cast<uint8_t*>(data.basePointer) + data.currentoffset, lpBuffer, toWrite);
		data.currentoffset += toWrite;
		if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = toWrite;
		return TRUE;
	}

	return WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

DWORD WINAPI _GetFinalPathNameByHandleA(
	HANDLE hFile,
	LPSTR lpszFilePath,
	DWORD cchFilePath,
	DWORD dwFlags
)
{
	for (const auto& [name, mapped] : keeper) {
		if (mapped.mapping == hFile) {
			const auto path = PhobosCRT::WideStringToString(Debug::ApplicationFilePath + L"\\" + PhobosCRT::StringToWideString(name));
			if (cchFilePath < path.size() + 1) return path.size() + 1;
			std::strncpy(lpszFilePath, path.c_str(), cchFilePath);
			return path.size();
		}
	}

	return GetFinalPathNameByHandleA(hFile, lpszFilePath, cchFilePath, dwFlags);
}


void LuaData::ApplyCoreHooks()
{
	if (!IsActive)
		return;

	Imports::CreateFileA = _CreateFileA;
	Imports::CloseHandle = _CloseHandle;
	Imports::GetFileSize = _GetFileSize;
	Imports::ReadFile = _ReadFile;
	Imports::SetFilePointer = _SetFilePointer;
	Imports::SetFileTime = _SetFileTime;
	//7C8482 , writefile
	//7C851E , GetFileInformationByHandle
	//7DCEEC , FlushFileBuffers
	//7C8422 , UnmapViewOfFile
	//7C8410 , MapViewOfFileEx
	//7DCEFE , GetFileType
}

#include <algorithm>

bool LuaAPI::g_scriptReady {};
unique_luastate LuaAPI::g_L {};
std::unique_ptr<std::once_flag> LuaAPI::g_engineOnce = std::make_unique<std::once_flag>();
uint32_t LuaAPI::g_scriptFingerprint {};

uint32_t LuaAPI::ComputeScriptFingerprint()
{
	std::wstring scriptsDir = Debug::ApplicationFilePath + L"\\Luascripts\\";

	std::vector<std::wstring> luaFiles;

	WIN32_FIND_DATAW findData {};
	HANDLE hFind = FindFirstFileW((scriptsDir + L"*.lua").c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				luaFiles.push_back(scriptsDir + findData.cFileName);
		}
		while (FindNextFileW(hFind, &findData));
		FindClose(hFind);
	}

	// NOTE: top-level *.lua only, not recursive into subfolders. If scripts
	// are organized into subdirectories, extend this with a recursive walk -
	// flagging as a known limitation rather than silently under-covering it.

	std::sort(luaFiles.begin(), luaFiles.end()); // deterministic order regardless of filesystem enumeration order

	SafeChecksummer crc {};
	std::vector<char> buffer;

	for (const auto& path : luaFiles)
	{
		HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
			continue; // BUGFIX-style: unreadable file just doesn't contribute, doesn't abort fingerprinting

		LARGE_INTEGER size {};
		if (GetFileSizeEx(hFile, &size) && size.QuadPart > 0)
		{
			buffer.resize(static_cast<size_t>(size.QuadPart));
			DWORD readBytes = 0;
			if (ReadFile(hFile, buffer.data(), static_cast<DWORD>(buffer.size()), &readBytes, nullptr) && readBytes == buffer.size())
				crc.operator()(buffer.data(), buffer.size());
		}
		CloseHandle(hFile);
	}

	return crc;
}

// =============================================================================
// Domino-failure protection helpers
// =============================================================================
namespace
{
	// max Lua VM instructions a single callback invocation may run
	// before being force-aborted. Prevents `while true do end`-style scripts
	// from freezing the main game thread forever.
	constexpr int kInstructionBudget = 100'000'000;

	// after this many consecutive failures, a given callback is
	// skipped entirely (until a successful call resets the counter to 0).
	// Prevents endless per-frame log spam / wasted work from a script that is
	// permanently broken, without touching the other callbacks.
	constexpr int kMaxConsecutiveFailures = 30;

	// g_L is a single lua_State shared across OnGameFrame / OnRender /
	// OnInvalidatePointer. Any call that pushes N values but tells lua_pcall a
	// different N (see original OnRender/OnInvalidatePointer) leaves the stack
	// unbalanced; the leftover/missing values then corrupt the *next* frame's
	// lua_getglobal/lua_pcall. This guard makes stack restoration unconditional,
	// including on early-return and on Lua errors.
	struct StackGuard
	{
		lua_State* L;
		int top;
		explicit StackGuard(lua_State* Lstate) : L(Lstate), top(lua_gettop(Lstate)) {}
		~StackGuard() { lua_settop(L, top); }
	};

	// lua_sethook callback that aborts the running script once the
	// instruction budget is exceeded. luaL_error() longjmps back into the
	// enclosing lua_pcall, so this is caught exactly like any other Lua error.
	void InstructionHook(lua_State* L, lua_Debug*)
	{
		luaL_error(L, "script exceeded instruction budget (possible infinite loop)");
	}

	// Calls the global Lua function `funcName` with `nargs` values already
	// pushed by `pushArgs`, under a stack guard + instruction budget +
	// per-callback circuit breaker. A failure in this callback never prevents
	// other callbacks (or later frames' calls to this same callback, until the
	// breaker trips) from running.
	template <typename PushArgsFn>
	void DispatchCallback(lua_State* L, const char* funcName, int nargs, int& failCounter, PushArgsFn&& pushArgs)
	{
		if (failCounter >= kMaxConsecutiveFailures)
			return; // circuit breaker tripped - callback disabled until it succeeds again externally

		StackGuard guard(L);

		lua_getglobal(L, funcName);
		if (!lua_isfunction(L, -1))
			return; // not defined by the script this session; guard pops the nil for us

		pushArgs(L);

		lua_sethook(L, InstructionHook, LUA_MASKCOUNT, kInstructionBudget);
		int result = lua_pcall(L, nargs, 0, 0); // nargs now always matches what pushArgs actually pushed
		lua_sethook(L, nullptr, 0, 0);

		if (result != LUA_OK)
		{
			const char* err = lua_tostring(L, -1);
			Debug::LogInfo("[Lua] {} error: {}", funcName, err ? err : "unknown error");

			++failCounter;
			if (failCounter == kMaxConsecutiveFailures)
				Debug::LogInfo("[Lua] {} disabled after {} consecutive failures", funcName, kMaxConsecutiveFailures);
		}
		else
		{
			failCounter = 0; // reset the streak on any success
		}
	}
}

void LuaAPI::EnsureEngine()
{
	// CreateEngine()/RunInitScript() can throw a genuine C++ exception
	// (e.g. from string conversion or file I/O), which would otherwise escape
	// std::call_once uncaught -> call_once treats the call as not having
	// completed and will retry on the *next* invocation, throwing again every
	// single frame forever. Catching here means: on failure, scripting is
	// cleanly disabled (g_L stays null) instead of repeatedly throwing.
	std::call_once(*g_engineOnce, []()
 {
	 try
	 {
		 if (!Debug::ApplicationFilePath.empty())
		 {
			 g_L.reset(CreateEngine());
			 if (g_L)
				 RunInitScript(g_L.get());
		 }
	 }
	 catch (const std::exception& ex)
	 {
		 Debug::LogInfo("[Lua] engine init threw: {}", ex.what());
		 g_L.reset();
	 }
	 catch (...)
	 {
		 Debug::LogInfo("[Lua] engine init threw an unknown exception");
		 g_L.reset();
	 }
	});
}

void LuaAPI::OnGameFrame()
{
	LuaAPI::ProcessObjects(Unsorted::CurrentFrame());

	EnsureEngine();

	if (!g_L || !g_scriptReady)
		return;

	static int failCount = 0;
	unsigned int frame = Unsorted::CurrentFrame();

	DispatchCallback(g_L.get(), "OnUpdate", 1, failCount, [frame](lua_State* L)
 {
	 lua_pushinteger(L, frame);
	});
}

void LuaAPI::OnRender()
{
	EnsureEngine();

	if (!g_L || !g_scriptReady)
		return;

	static int failCount = 0;

	// original pushed zero args but called lua_pcall(L, 1, 0, 0),
	// which told Lua to consume one argument that was never pushed - this
	// silently ate the function value itself off the stack instead of
	// calling it, and desynced the stack for every subsequent frame.
	DispatchCallback(g_L.get(), "OnRender", 0, failCount, [](lua_State*)
 {
	 // no arguments
	});
}

void LuaAPI::InvalidateTechnoUserdata(lua_State* L, void* ptr)
{
	if (!ptr)
		return;

	constexpr const char* kTechnoRegistryKey = "LuaAPI.TechnoRegistry";

	lua_getfield(L, LUA_REGISTRYINDEX, kTechnoRegistryKey);
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return; // registry not set up yet (engine not fully initialized)
	}

	lua_pushlightuserdata(L, ptr);
	lua_gettable(L, -2);

	if (lua_isuserdata(L, -1))
	{
		// BUGFIX/zero the wrapped pointer BEFORE the engine frees
		// the object. Any script still holding this userdata will now fail
		// CheckTechno's null check with a clean Lua error instead of
		// dereferencing freed memory on its next call.
		auto* ud = static_cast<void**>(lua_touserdata(L, -1));
		*ud = nullptr;
	}
	lua_pop(L, 1); // userdata or nil

	// Drop the (now-stale) registry entry itself.
	lua_pushlightuserdata(L, ptr);
	lua_pushnil(L);
	lua_settable(L, -3);

	lua_pop(L, 1); // registry table
}

void LuaAPI::OnInvalidatePointer(void* ptr, bool removed)
{
	EnsureEngine();

	if (!g_L || !g_scriptReady)
		return;

	// internal userdata invalidation runs unconditionally and
	// BEFORE the script-facing callback, regardless of whether the script
	// defines OnInvalidatePointer at all. Checked against both registries -
	// each is a no-op table lookup if `ptr` isn't a Techno/Team at all.
	InvalidateTechnoUserdata(g_L.get(), ptr);
	InvalidateTeamUserdata(g_L.get(), ptr);

	static int failCount = 0;

	DispatchCallback(g_L.get(), "OnInvalidatePointer", 2, failCount, [ptr, removed](lua_State* L)
 {
	 // lua_pushinteger(L, ptr) does not compile / is not valid for
	 // a void* - pointers must go through lightuserdata, not an integer push.
	 lua_pushlightuserdata(L, ptr);
	 lua_pushboolean(L, removed ? 1 : 0);
	});
}

HRESULT LuaAPI::OnGlobalGameSave(IStream* pStm)
{
	if (!pStm)
		return E_POINTER;

	// No engine, script not ready, or script has no save hook: write a
	// zero-length blob so OnGlobalGameLoad can tell "nothing was saved" apart
	// from a corrupt/truncated stream.
	std::string payload;

	if (g_L && g_scriptReady)
	{
		lua_State* L = g_L.get();
		StackGuard guard(L);

		lua_getglobal(L, "OnGlobalGameSave");
		if (lua_isfunction(L, -1))
		{
			lua_sethook(L, InstructionHook, LUA_MASKCOUNT, kInstructionBudget);
			int result = lua_pcall(L, 0, 1, 0); // script returns one string to persist
			lua_sethook(L, nullptr, 0, 0);

			if (result == LUA_OK)
			{
				if (lua_isstring(L, -1))
				{
					size_t len = 0;
					const char* s = lua_tolstring(L, -1, &len);
					payload.assign(s, len);
				}
				// else: script returned something other than a string -> treat as "nothing to save"
			}
			else
			{
				const char* err = lua_tostring(L, -1);
				Debug::LogInfo("[Lua] OnGlobalGameSave error: {}", err ? err : "unknown error");
				// BUGFIX-style choice: fall through with an empty payload instead
				// of failing the whole engine save over one broken script.
			}
		}
	}

	// fingerprint written FIRST and unconditionally (even when
	// payload is empty / no script) so OnGlobalGameLoad always has a
	// consistent header shape to read, regardless of what was saved.
	uint32_t fingerprint = g_scriptFingerprint;
	uint32_t size = static_cast<uint32_t>(payload.size());
	ULONG written = 0;

	HRESULT hr = pStm->Write(&fingerprint, sizeof(fingerprint), &written);
	if (FAILED(hr) || written != sizeof(fingerprint))
		return FAILED(hr) ? hr : E_FAIL;

	hr = pStm->Write(&size, sizeof(size), &written);
	if (FAILED(hr) || written != sizeof(size))
		return FAILED(hr) ? hr : E_FAIL;

	if (size > 0)
	{
		hr = pStm->Write(payload.data(), size, &written);
		if (FAILED(hr) || written != size)
			return FAILED(hr) ? hr : E_FAIL;
	}

	return S_OK;
}

HRESULT LuaAPI::OnGlobalGameLoad(IStream* pStm)
{
	if (!pStm)
		return E_POINTER;

	uint32_t savedFingerprint = 0;
	uint32_t size = 0;
	ULONG read = 0;

	HRESULT hr = pStm->Read(&savedFingerprint, sizeof(savedFingerprint), &read);
	if (FAILED(hr))
		return hr;
	if (read != sizeof(savedFingerprint))
		return E_FAIL; // stream truncated / corrupt

	hr = pStm->Read(&size, sizeof(size), &read);
	if (FAILED(hr))
		return hr;
	if (read != sizeof(size))
		return E_FAIL;

	std::string payload;
	if (size > 0)
	{
		// VERIFY: sanity cap against a corrupt/hostile stream requesting a huge
		// allocation. Raise this if legitimate script save payloads exceed it.
		constexpr uint32_t kMaxPayload = 64u * 1024u * 1024u; // 64 MB
		if (size > kMaxPayload)
			return E_FAIL;

		payload.resize(size);
		hr = pStm->Read(payload.data(), size, &read);
		if (FAILED(hr) || read != size)
			return FAILED(hr) ? hr : E_FAIL;
	}

	EnsureEngine();

	if (!g_L || !g_scriptReady)
		return S_OK; // nothing to hand the payload to; not a load failure

	// compare against the CURRENTLY loaded script set's fingerprint,
	// not the one that was active at save time. A mismatch means the .lua
	// files changed since this save was made - the script decides what that
	// means (ignore, migrate, warn), C++ does not refuse to load over it.
	bool scriptsChanged = (savedFingerprint != g_scriptFingerprint);
	if (scriptsChanged)
	{
		Debug::LogInfo("[Lua] script fingerprint mismatch on load (saved={:#010x}, current={:#010x})",
			savedFingerprint, g_scriptFingerprint);
	}

	lua_State* L = g_L.get();
	StackGuard guard(L);

	lua_getglobal(L, "OnGlobalGameLoad");
	if (!lua_isfunction(L, -1))
		return S_OK; // script doesn't define a loader; not an error

	lua_pushlstring(L, payload.data(), payload.size());
	lua_pushboolean(L, scriptsChanged ? 1 : 0);

	lua_sethook(L, InstructionHook, LUA_MASKCOUNT, kInstructionBudget);
	int result = lua_pcall(L, 2, 0, 0); // nargs matches the two values now pushed
	lua_sethook(L, nullptr, 0, 0);

	if (result != LUA_OK)
	{
		const char* err = lua_tostring(L, -1);
		Debug::LogInfo("[Lua] OnGlobalGameLoad error: {}", err ? err : "unknown error");
		// BUGFIX-style choice: a broken load script must not fail the whole
		// engine load - other subsystems' streams may still need reading from
		// this same IStream storage after this call returns.
	}

	return S_OK;
}

void LuaAPI::RunInitScript(lua_State* L)
{
	// previously checked for the existence of ".../scripts/init.lua"
	// but then called luaL_dofile() on _narrowDir - the Luascripts DIRECTORY
	// path itself, not a file. dofile-ing a directory is never valid Lua and
	// would always fail (or silently no-op depending on platform). This also
	// unifies on a single folder name ("Luascripts") - the two prior paths
	// ("scripts" for the check, "Luascripts" for everything else, including
	// package.path and ComputeScriptFingerprint's scan dir) never agreed.
	std::wstring scriptsDir = Debug::ApplicationFilePath + L"\\Luascripts";
	std::wstring initScriptPath = scriptsDir + L"\\init.lua";

	std::string _narrowDir = PhobosCRT::WideStringToString(scriptsDir);
	std::string _narrowInitPath = PhobosCRT::WideStringToString(initScriptPath);

	// Make require() find modules next to init.lua (forward slashes for Lua).
	std::string pkgExpr = "package.path = '" + _narrowDir + "/?.lua;' .. package.path";

	for (auto& c : pkgExpr)
	{
		if (c == '\\')
			c = '/';
	}

	if (luaL_dostring(L, pkgExpr.c_str()) != LUA_OK)
	{
		Debug::LogInfo("Failed to extend package.path");
		lua_pop(L, 1);
	}

	DWORD attrs = GetFileAttributesW(initScriptPath.c_str());
	if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY))
	{
		Debug::LogInfo("Script not found: {}", _narrowInitPath);
		return;
	}

	if (luaL_dofile(L, _narrowInitPath.c_str()) != LUA_OK)
	{
		const char* err = lua_tostring(L, -1);
		Debug::LogInfo("Script error: {}", err ? err : "unknown error");
		lua_pop(L, 1);
		return;
	}

	g_scriptReady = true;
	g_scriptFingerprint = ComputeScriptFingerprint(); // snapshot script set identity for save/load consistency checks
	Debug::LogInfo("Lua engine initialized on game thread, script executed (fingerprint={:#010x})", g_scriptFingerprint);
}

int LuaPrint(lua_State* L)
{
	int n = lua_gettop(L);
	std::string out;
	for (int i = 1; i <= n; ++i)
	{
		if (i > 1)
			out += '\t';
		size_t len = 0;
		const char* s = luaL_tolstring(L, i, &len);
		out.append(s, len);
		lua_pop(L, 1);
	}
	Debug::LogInfo("[LUA script] {}", out);
	return 0;
}

lua_State* LuaAPI::CreateEngine()
{
	lua_State* L = luaL_newstate();
	if (!L)
	{
		Debug::LogInfo("luaL_newstate failed");
		return nullptr;
	}

	luaL_openlibs(L);
	lua_register(L, "print", LuaPrint);

	lua_newtable(L);
	lua_pushliteral(L, "0.2.0");
	lua_setfield(L, -2, "version");
	lua_setglobal(L, "Engine");

	RegisterHouseBindings(L);
	RegisterTechnoBindings(L);
	RegisterTeamBindings(L);

	return L;
}

#pragma region Techno

// split from the House metatable name - both used to be registered
// under the same "LuaAPI.Techno" string, so a House userdata would validate
// against (and expose) Techno's method table and vice versa via
// luaL_checkudata/luaL_getmetatable.
constexpr const char* kTechnoMetaName = "LuaAPI.Techno";
constexpr const char* kTechnoRegistryKey = "LuaAPI.TechnoRegistry";

void LuaAPI::PushTechno(lua_State* L, void* pTechno)
{
	auto* ud = static_cast<void**>(lua_newuserdatauv(L, sizeof(void*), 0));
	*ud = pTechno;
	luaL_getmetatable(L, kTechnoMetaName);
	lua_setmetatable(L, -2);

	// register this userdata (as a WEAK value) in the invalidation
	// registry, keyed by the raw pointer it wraps. If two userdatas ever wrap
	// the same pointer, only the most recently pushed one is tracked here -
	// acceptable since InvalidateTechnoUserdata's job is just to make sure no
	// *live* userdata is left pointing at freed memory, not to track every copy.
	lua_getfield(L, LUA_REGISTRYINDEX, kTechnoRegistryKey);
	if (lua_istable(L, -1))
	{
		lua_pushlightuserdata(L, pTechno);
		lua_pushvalue(L, -3); // the userdata we just created (now 3rd from top)
		lua_settable(L, -3);
	}
	lua_pop(L, 1); // registry table
}

struct RecorderEntries
{
	TechnoClass* ptr;
	bool isBuilding;
	bool hadPower;        // BuildingClass::HasPower prior to the blackout
	unsigned int expiryFrame;
};
std::vector<RecorderEntries> g_recorded_entries;


void LuaAPI::OnScenarioClear()
{
	if (g_L && g_scriptReady)
	{
		static int failCount = 0;
		DispatchCallback(g_L.get(), "OnScenarioClear", 0, failCount, [](lua_State*)
 {
	 // no arguments
		});
	}

	// full reset so the next scenario starts from a brand new Lua
	// VM + init.lua run rather than carrying over any state from this one.
	g_recorded_entries.clear();
	g_scriptReady = false;
	g_L.reset();                                     // closes lua_State via unique_luastate deleter
	g_engineOnce = std::make_unique<std::once_flag>(); // re-arm EnsureEngine for next scenario
}

bool IsValid(TechnoClass* pTechno)
{
	return pTechno != nullptr && pTechno->IsAlive && pTechno->Health > 0;
}

TechnoClass* CheckTechno(lua_State* L, int idx)
{
	void* ud = luaL_checkudata(L, idx, kTechnoMetaName);
	auto* pTechno = *static_cast<TechnoClass**>(ud);
	if (!pTechno)
	{
		luaL_error(L, "techno object is no longer valid");
		return nullptr;
	}
	return pTechno;
}

// --- instance methods ------------------------------------------------------

int Techno_GetTypeName(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
		return 0;
	lua_pushstring(L, pTechno->GetType()->get_ID());
	return 1;
}

int Techno_GetHealth(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	// previously dereferenced pTechno->Health with no IsValid() check.
	if (!IsValid(pTechno))
		return 0;
	lua_pushinteger(L, pTechno->Health);
	return 1;
}

int Techno_GetMaxHealth(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
		return 0;
	lua_pushinteger(L, pTechno->GetType()->Strength);
	return 1;
}

int Techno_GetOwner(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
		return 0;
	return LuaAPI::PushHouse(L, pTechno->Owner);
}

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

int Techno_IsAlive(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	lua_pushboolean(L, IsValid(pTechno) ? 1 : 0);
	return 1;
}

int Techno_GetId(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	// previously dereferenced pTechno->UniqueID with no IsValid() check.
	if (!IsValid(pTechno))
		return 0;
	lua_pushinteger(L, static_cast<lua_Integer>(pTechno->UniqueID));
	return 1;
}

// obj:GetPtr() -> lightuserdata | nil
//
// exposes the raw pointer identity this userdata wraps, as a
// lightuserdata value. Lightuserdata compares by pointer VALUE in Lua
// (unlike full userdata, which compares by object identity) - so this is
// the SAME comparable value that OnInvalidatePointer(ptr, removed) receives
// for this object, letting scripts build their own pointer-keyed
// bookkeeping tables (buffs, AI state, caches, ...) and reliably clean them
// up when the underlying object is destroyed. See
// advanced_object_registry_example.lua for the full pattern.
//
// This value has NO methods and is NOT a Techno userdata - it exists only
// to be used as an opaque, comparable table key. Never index/call it.
int Techno_GetPtr(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
	{
		lua_pushnil(L);
		return 1;
	}
	lua_pushlightuserdata(L, pTechno);
	return 1;
}

int Techno_GetKind(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
		return 0;

	switch (pTechno->WhatAmI())
	{
	case AbstractType::Building: lua_pushliteral(L, "building"); break;
	case AbstractType::Unit:     lua_pushliteral(L, "unit");     break;
	case AbstractType::Infantry: lua_pushliteral(L, "infantry"); break;
	case AbstractType::Aircraft: lua_pushliteral(L, "aircraft"); break;
	default:                     lua_pushliteral(L, "other");    break;
	}
	return 1;
}

int Techno_GetDistanceTo(lua_State* L)
{
	auto* pSelf = CheckTechno(L, 1);

	void* ud = luaL_testudata(L, 2, kTechnoMetaName);
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

int Techno_Disable(lua_State* L)
{
	auto* pTechno = CheckTechno(L, 1);
	if (!IsValid(pTechno))
		return 0;

	lua_Integer frames = luaL_checkinteger(L, 2);
	if (frames <= 0)
		return 0;

	RecorderEntries entry {};
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

	g_recorded_entries.push_back(entry);
	return 0;
}

// --- navigation (FootClass only: units / infantry / aircraft) ---------------

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

	pFoot->Destination = pCell;
	pFoot->QueueMission(Mission::Move, true);

	Debug::LogInfo("[Nav] {} moving to ({},{})", pTechno->GetType()->get_ID(), cellX, cellY);
	lua_pushboolean(L, 1);
	return 1;
}

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
	{ "GetPtr",        Techno_GetPtr        },
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

int World_GetBuildings(lua_State* L)
{
	return CollectArray(L, *BuildingClass::Array);
}

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

// World.GetById(id) -> techno | nil
//
// resolves a UniqueID (as returned by obj:GetId()) back to a
// live Techno object. Primarily meant for reconnecting persisted data
// (which must be keyed by GetId(), not GetPtr() - see GetPtr's docs) to a
// real object after a save/load. Linear scan across the object arrays -
// fine for occasional use (e.g. once per restored save entry), not meant
// to be called every frame per object.
int World_GetById(lua_State* L)
{
	lua_Integer id = luaL_checkinteger(L, 1);

	auto findIn = [id](auto* pArray) -> TechnoClass*
		{
			for (int i = 0; i < pArray->Count; ++i)
			{
				auto* pItem = pArray->Items[i];
				if (pItem && static_cast<lua_Integer>(pItem->UniqueID) == id)
					return pItem;
			}
			return nullptr;
		};

	TechnoClass* pFound = findIn(BuildingClass::Array.operator->());
	if (!pFound) pFound = findIn(UnitClass::Array.operator->());
	if (!pFound) pFound = findIn(InfantryClass::Array.operator->());
	if (!pFound) pFound = findIn(AircraftClass::Array.operator->());

	if (!pFound)
		return 0; // nil: no longer exists (destroyed since it was saved)

	LuaAPI::PushTechno(L, pFound);
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
	for (auto it = g_recorded_entries.begin(); it != g_recorded_entries.end();)
	{
		// original was
		//   bool alive = StillExists(it->ptr) || !it->ptr->IsAlive;
		// `||` short-circuits, so when StillExists() is false (pointer may be
		// dangling/freed) the right-hand side `it->ptr->IsAlive` still runs -
		// dereferencing a pointer that was just proven to possibly be freed.
		// StillExists() is the ONLY safe liveness proof we have here; only
		// dereference `it->ptr` once membership in a live array is confirmed.
		if (!StillExists(it->ptr))
		{
			it = g_recorded_entries.erase(it); // dangling: never touch the memory
			continue;
		}

		if (!it->ptr->IsAlive) // safe now: pointer confirmed to be in a live engine array
		{
			it = g_recorded_entries.erase(it);
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
			it = g_recorded_entries.erase(it);
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
	luaL_newmetatable(L, kTechnoMetaName);

	lua_newtable(L);
	luaL_setfuncs(L, kTechnoMethods, 0);
	lua_setfield(L, -2, "__index");

	lua_pop(L, 1); // pop metatable

	// weak-value invalidation registry, keyed by raw pointer
	// (lightuserdata) -> the most recently created userdata wrapping it.
	// __mode = "v" lets garbage-collected userdatas fall out on their own so
	// this table never grows unbounded and never keeps dead userdata alive.
	lua_newtable(L);
	lua_newtable(L); // metatable for the registry
	lua_pushliteral(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
	lua_setfield(L, LUA_REGISTRYINDEX, kTechnoRegistryKey);

	// Global "World" namespace
	lua_newtable(L);
	lua_pushcfunction(L, World_GetBuildings);
	lua_setfield(L, -2, "GetBuildings");
	lua_pushcfunction(L, World_GetUnits);
	lua_setfield(L, -2, "GetUnits");
	lua_pushcfunction(L, World_GetById);
	lua_setfield(L, -2, "GetById");
	lua_setglobal(L, "World");
}
#pragma endregion

#pragma region House

// previously registered/checked against kMetaName == "LuaAPI.Techno",
// meaning House userdata shared (and could be confused with) the Techno
// metatable/method table. House now gets its own metatable name.
constexpr const char* kHouseMetaName = "LuaAPI.House";

HouseClass* CheckHouse(lua_State* L, int idx)
{
	void* ud = luaL_checkudata(L, idx, kHouseMetaName);
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
	luaL_getmetatable(L, kHouseMetaName);
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

int House_GetPlayer(lua_State* L)
{
	HouseClass* pHouse = HouseClass::CurrentPlayer;
	if (!pHouse)
		return 0; // nil

	NewHouse(L, pHouse);
	return 1;
}

int House_GetCount(lua_State* L)
{
	lua_pushinteger(L, HouseClass::Array->Count);
	return 1;
}

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

int House_GetCredits(lua_State* L)
{
	HouseClass* pHouse = CheckHouse(L, 1);
	lua_pushinteger(L, static_cast<lua_Integer>(pHouse->Available_Money()));
	return 1;
}

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

int House_AddCredits(lua_State* L)
{
	HouseClass* pHouse = CheckHouse(L, 1);
	lua_Integer delta = luaL_checkinteger(L, 2);

	if (delta != 0)
		pHouse->TransactMoney(static_cast<long>(delta));

	Debug::LogInfo("[House] {} credits adjusted ({:+})", pHouse->get_ID(), delta);
	return 0;
}

int House_GetPowerOutput(lua_State* L)
{
	HouseClass* pHouse = CheckHouse(L, 1);
	lua_pushinteger(L, pHouse->PowerOutput);
	return 1;
}

int House_GetPowerDrain(lua_State* L)
{
	HouseClass* pHouse = CheckHouse(L, 1);
	lua_pushinteger(L, pHouse->PowerDrain);
	return 1;
}

int House_GetName(lua_State* L)
{
	HouseClass* pHouse = CheckHouse(L, 1);
	lua_pushstring(L, pHouse->get_ID());
	return 1;
}

// house:GetPtr() -> lightuserdata
//
// same purpose as Techno's GetPtr - a comparable identity key
// matching what OnInvalidatePointer(ptr, removed) receives, for scripts
// that keep their own House-keyed bookkeeping. NOTE: unlike Techno, House
// userdata is NOT auto-invalidated by the C++ side (houses persist for the
// whole match, so this is considered lower risk) - if you rely on this key,
// still listen for OnInvalidatePointer yourself rather than assuming a
// House pointer is stable for the entire session.
int House_GetPtr(lua_State* L)
{
	HouseClass* pHouse = CheckHouse(L, 1);
	lua_pushlightuserdata(L, pHouse);
	return 1;
}

int House_IsHuman(lua_State* L)
{
	HouseClass* pHouse = CheckHouse(L, 1);
	lua_pushboolean(L, pHouse->IsControlledByHuman() ? 1 : 0);
	return 1;
}

int House_IsAlliedWith(lua_State* L)
{
	HouseClass* pSelf = CheckHouse(L, 1);

	void* ud = luaL_testudata(L, 2, kHouseMetaName);
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
	{ "GetPtr",         House_GetPtr         },
	{ "IsHuman",        House_IsHuman        },
	{ "IsAlliedWith",   House_IsAlliedWith   },
	{ nullptr, nullptr }
};

void LuaAPI::RegisterHouseBindings(lua_State* L)
{
	// Userdata metatable
	luaL_newmetatable(L, kHouseMetaName);

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

#pragma region Team

// BUGFIX-style separation: own metatable name, following the same pattern
// as splitting House from Techno - never share a metatable across types.
constexpr const char* kTeamMetaName = "LuaAPI.Team";
constexpr const char* kTeamRegistryKey = "LuaAPI.TeamRegistry";

void LuaAPI::PushTeam(lua_State* L, void* pTeam)
{
	auto* ud = static_cast<void**>(lua_newuserdatauv(L, sizeof(void*), 0));
	*ud = pTeam;
	luaL_getmetatable(L, kTeamMetaName);
	lua_setmetatable(L, -2);

	// Same weak-value invalidation registry pattern as PushTechno - see
	// InvalidateTeamUserdata for the cleanup half of this contract.
	lua_getfield(L, LUA_REGISTRYINDEX, kTeamRegistryKey);
	if (lua_istable(L, -1))
	{
		lua_pushlightuserdata(L, pTeam);
		lua_pushvalue(L, -3); // the userdata we just created
		lua_settable(L, -3);
	}
	lua_pop(L, 1); // registry table
}

void LuaAPI::InvalidateTeamUserdata(lua_State* L, void* ptr)
{
	if (!ptr)
		return;

	lua_getfield(L, LUA_REGISTRYINDEX, kTeamRegistryKey);
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return; // registry not set up yet
	}

	lua_pushlightuserdata(L, ptr);
	lua_gettable(L, -2);
	if (lua_isuserdata(L, -1))
	{
		auto* ud = static_cast<void**>(lua_touserdata(L, -1));
		*ud = nullptr; // any script still holding this team now fails cleanly instead of touching freed memory
	}
	lua_pop(L, 1); // userdata or nil

	lua_pushlightuserdata(L, ptr);
	lua_pushnil(L);
	lua_settable(L, -3); // drop the stale registry entry

	lua_pop(L, 1); // registry table
}

TeamClass* CheckTeam(lua_State* L, int idx)
{
	void* ud = luaL_checkudata(L, idx, kTeamMetaName);
	auto* pTeam = *static_cast<TeamClass**>(ud);
	if (!pTeam)
	{
		luaL_error(L, "team object is no longer valid");
		return nullptr;
	}
	return pTeam;
}

// team:GetPtr() -> lightuserdata
//
// Same purpose as Techno's GetPtr() - a comparable identity key matching
// what OnInvalidatePointer(ptr, removed) receives, for scripts keeping
// their own team-keyed bookkeeping (e.g. a multi-tick mission's progress
// state - see team_missions_example.lua).
int Team_GetPtr(lua_State* L)
{
	auto* pTeam = CheckTeam(L, 1);
	lua_pushlightuserdata(L, pTeam);
	return 1;
}

// team:Complete()
//
// Signals that the team's CURRENT script line is finished, exactly as
// every native _TMission_* handler signals completion by setting
// StepCompleted. The team's script advances to its next line on the
// following tick. A mission that needs multiple ticks simply does not
// call this yet on ticks where it's still waiting - OnTeamMission will be
// called again next tick with the same action/argument.
int Team_Complete(lua_State* L)
{
	auto* pTeam = CheckTeam(L, 1);
	pTeam->StepCompleted = true;
	return 0;
}

// team:GetTypeName() -> string (the TeamType ID from the .ini, e.g. "TeamA")
int Team_GetTypeName(lua_State* L)
{
	auto* pTeam = CheckTeam(L, 1);
	lua_pushstring(L, pTeam->Type->ID);
	return 1;
}

// team:GetOwner() -> house | nil
int Team_GetOwner(lua_State* L)
{
	auto* pTeam = CheckTeam(L, 1);
	return LuaAPI::PushHouse(L, pTeam->OwnerHouse);
}

// team:GetMemberCount() -> int
int Team_GetMemberCount(lua_State* L)
{
	auto* pTeam = CheckTeam(L, 1);
	lua_pushinteger(L, pTeam->TotalObjects);
	return 1;
}

// team:GetMembers() -> table of Techno objects
//
// Walks the FirstUnit/NextTeamMember linked list (teams are not a
// DynamicVectorClass like BuildingClass::Array, so this can't reuse
// CollectArray). O(member count), not O(TechnoClass::Array->Count).
int Team_GetMembers(lua_State* L)
{
	auto* pTeam = CheckTeam(L, 1);

	lua_newtable(L);
	int n = 0;
	for (FootClass* pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
	{
		LuaAPI::PushTechno(L, pMember);
		lua_seti(L, -2, ++n);
	}
	return 1;
}

// team:IsMoving() -> bool
int Team_IsMoving(lua_State* L)
{
	auto* pTeam = CheckTeam(L, 1);
	lua_pushboolean(L, pTeam->IsMoving ? 1 : 0);
	return 1;
}

const luaL_Reg kTeamMethods[] = {
	{ "Complete",       Team_Complete       },
	{ "GetPtr",         Team_GetPtr         },
	{ "GetTypeName",    Team_GetTypeName    },
	{ "GetOwner",       Team_GetOwner       },
	{ "GetMemberCount", Team_GetMemberCount },
	{ "GetMembers",     Team_GetMembers     },
	{ "IsMoving",       Team_IsMoving       },
	{ nullptr, nullptr }
};

void LuaAPI::RegisterTeamBindings(lua_State* L)
{
	luaL_newmetatable(L, kTeamMetaName);

	lua_newtable(L);
	luaL_setfuncs(L, kTeamMethods, 0);
	lua_setfield(L, -2, "__index");

	lua_pop(L, 1); // pop metatable

	// Weak-value invalidation registry, same pattern as Techno's.
	lua_newtable(L);
	lua_newtable(L); // metatable for the registry
	lua_pushliteral(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
	lua_setfield(L, LUA_REGISTRYINDEX, kTeamRegistryKey);

	// No global "Team" namespace - unlike World/House, teams are never
	// queried in bulk from Lua; they only ever arrive as an argument to
	// OnTeamMission.
}

bool LuaAPI::OnTeamMission(TeamClass* pTeam, int action, int argument)
{
	if (!g_L || !g_scriptReady)
		return false;

	lua_State* L = g_L.get();
	StackGuard guard(L);

	lua_getglobal(L, "OnTeamMission");
	if (!lua_isfunction(L, -1))
		return false; // script defines no custom-mission dispatch at all

	PushTeam(L, pTeam);
	lua_pushinteger(L, action);
	lua_pushinteger(L, argument);

	lua_sethook(L, InstructionHook, LUA_MASKCOUNT, kInstructionBudget);
	int result = lua_pcall(L, 3, 1, 0); // team, action, argument -> handled:boolean
	lua_sethook(L, nullptr, 0, 0);

	if (result != LUA_OK)
	{
		const char* err = lua_tostring(L, -1);
		Debug::LogInfo("[Lua] OnTeamMission error (action={}): {}", action, err ? err : "unknown error");
		// BUGFIX-style choice: a broken handler still counts as "handled" so
		// the team retries the same script line next tick instead of the
		// caller falling through to native unknown-action handling
		// (dissolve/log) purely because of a script bug.
		return true;
	}

	bool handled = lua_toboolean(L, -1) != 0;
	return handled;
}

#pragma endregion