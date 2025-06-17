#include "Phobos.Lua.h"

#include <map>
#include <string>
#include "Phobos.h"

#include <Ext/Script/Lua/Wrapper.h>

#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <Misc/Ares/Hooks/Header.h>

#include <MessageBoxLogging.h>

#include <Utilities/Macro.h>

#include <MixFileClass.h>

// TODO : encryption support
// Otamaa : change this variable if you want to load desired name lua file
std::string filename = "\\renameinternal.lua";
std::string LuaData::LuaDir;
std::string CoreHandles;
HelperedVector<std::pair<uintptr_t, std::string>> map_replaceAddrTo;
std::string LuaData::MainWindowStr;
std::map<std::string, bool> SafeFiles {};
bool IsActive;

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
	if (CoreHandles.empty()) return;

	size_t key_len = CoreHandles.length();
	for (size_t i = 0; i < size; ++i)
	{
		pBuffer[i] = content[i] ^ CoreHandles[i % key_len];
	}
}

void ApplyCore(char* content, size_t size)
{
	if (CoreHandles.empty()) return;

	size_t key_len = CoreHandles.length();
	for (size_t i = 0; i < size; ++i)
	{
		content[i] ^= CoreHandles[i % key_len];
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

void* __fastcall FakeFileLoader::_Retrieve(const char* pFilename, bool bLoadAsSHP)
{
	//void* pData = FakeFileLoader::Retrieve(pFilename, bLoadAsSHP);

	//if (pData && IsActive)
	//{
	//	if (pFilename)
	//	{
	//		auto it = SafeFiles.find(pFilename);

	//		if (it != SafeFiles.end())
	//		{
	//			long fileSize = 0;
	//			if (MixFileClass::Offset(pFilename, nullptr, nullptr, nullptr, &fileSize)) {
	//				if (fileSize > 0) {
	//					ApplyCore(static_cast<char*>(pData), static_cast<size_t>(fileSize));
	//				}
	//			}
	//		}
	//	}
	//}

	return FakeFileLoader::Retrieve(pFilename, bLoadAsSHP);
}

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

	auto it = SafeFiles.find(lpFileName);
	if(it != SafeFiles.end()) {
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

DEFINE_FUNCTION_JUMP(CALL, 0x41CAF7, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x41CB08, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4279DA, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x427A04, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x427B15, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x427BE9, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x427C07, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x42891E, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4309FD, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x430A61, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45E904, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45E988, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45E999, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45EA16, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45EA79, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F28D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F2A5, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F525, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F543, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F615, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F6E9, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F7C1, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F82D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F84B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F91D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45FA0B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45FA39, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45FA6B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x47EFFD, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x47F00E, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x47F26A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4A38DE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4A3985, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4A8862, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4A8873, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6D07, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6D1A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6D2D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6E52, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6F41, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B7349, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B73A8, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x51916D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5194FF, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BBE3, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BC55, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BCFD, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BE6D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BF26, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BFDA, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52C08E, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52C142, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x531381, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x534C04, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x534CC3, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x546725, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5468DB, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5468F8, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5469BF, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x560D7B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x561093, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5D2EBA, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F76EE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F773C, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F778A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F77D8, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9249, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9267, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9281, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9685, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9931, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE68C, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE6AE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE6F6, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE714, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE928, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FEBEC, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x62769F, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x66C5F4, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x66C606, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x677FAC, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x677FBE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x690660, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6906BD, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x69071A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x690A4D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x690B1D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6A5012, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6A8167, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6ABD57, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6ABD68, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6ABD79, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6B1B94, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6B1BCE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6B57C2, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6CE89B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6CE8B1, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6CEE20, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6CEE38, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6DAE07, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x715820, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x715A38, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x715A54, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x715B05, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x716C77, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x716D04, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x716D1A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x716D6F, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x71DFBB, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x73CEE0, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x73D3EF, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x747490, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x7474A1, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x747BB4, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x748093, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x74D47D, FakeFileLoader::_Retrieve);
#pragma endregion

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
		if (lua_isstring(L, -1))
		{
			std::string val = lua_tostring(L, -1);
			PhobosCRT::uppercase(val);
			SafeFiles[val] = false;
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

	bool getGlobalString(const char* name, std::string& result)
	{
		auto L = Internal.get();
		lua_getglobal(L, name);
		if (lua_isstring(L, -1))
		{
			result = lua_tostring(L, -1);
		}
		lua_pop(L, 1);

		return !result.empty();
	}

	bool getGlobalString(const char* name, std::wstring& result)
	{
		auto L = Internal.get();
		lua_getglobal(L, name);
		if (lua_isstring(L, -1))
		{
			result = PhobosCRT::StringToWideString(lua_tostring(L, -1));
		}

		lua_pop(L, 1);

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
		if (Lua.getGlobalString("AdminMode", adminName))
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
					//Phobos::Otamaa::ReplaceGameMemoryAllocator = true;
					Phobos::Otamaa::IsAdmin = true;
				}
			}
		}
	}

	const auto _renamer = LuaData::LuaDir + filename;

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
						if (Phobos::Otamaa::IsAdmin)
						{
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

		if (Lua.getGlobalString("MainWindowString", LuaData::MainWindowStr))
		{
			Patch::Apply_OFFSET(0x777CC6, (uintptr_t)LuaData::MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777CCB, (uintptr_t)LuaData::MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777D6D, (uintptr_t)LuaData::MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777D72, (uintptr_t)LuaData::MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777CA1, (uintptr_t)LuaData::MainWindowStr.c_str());
		}

		//core part to activate , disable it for now
		//lua_get_string_array_of_SafeFiles(Lua.get(), "FetchHandles");
		//Lua.getGlobalString("CoreHandles", CoreHandles);

		//IsActive = !SafeFiles.empty() && !CoreHandles.empty();

		Lua.getGlobalString("MovieMDINI", StaticVars::MovieMDINI);
		Lua.getGlobalString("DebugLogName", Debug::LogFileMainName);
		Lua.getGlobalString("CrashDumpFileName", Debug::CrashDumpFileName);
		Lua.getGlobalString("DesyncLogName", Debug::SyncFileFormat);
		Lua.getGlobalString("DesyncLogName2", Debug::SyncFileFormat2);
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


