#include "Patch.h"

#include "Macro.h"
#include "Debug.h"
#include <Phobos.h>

#include <tlhelp32.h>
#include <Psapi.h>

int Patch::GetSection(HANDLE hInstance, const char* sectionName, void** pVirtualAddress)
{
	char buf[MAX_PATH + 1] = { 0 };
	GetModuleFileNameA(NULL, buf, sizeof(buf));

	const auto pHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(((PIMAGE_DOS_HEADER)hInstance)->e_lfanew + (long)hInstance);

	for (int i = 0; i < pHeader->FileHeader.NumberOfSections; i++)
	{
		const auto sct_hdr = IMAGE_FIRST_SECTION(pHeader) + i;

		if (strncmp(sectionName, (char*)sct_hdr->Name, 8) == 0)
		{
			*pVirtualAddress = (void*)((DWORD)hInstance + sct_hdr->VirtualAddress);
			return sct_hdr->Misc.VirtualSize;
		}
	}
	return 0;
}

void Patch::ApplyStatic()
{
	void* buffer;
	const int len = GetSection(Phobos::hInstance , PATCH_SECTION_NAME, &buffer);

	for (int offset = 0; offset < len; offset += sizeof(Patch))
	{
		const auto pPatch = (Patch*)((DWORD)buffer + offset);
		if (pPatch->offset == 0)
			return;

		pPatch->Apply();
	}
}

void Patch::Apply()
{
	void* pAddress = (void*)this->offset;

	DWORD protect_flag {};
	DWORD protect_flagb {};
	VirtualProtect(pAddress, this->size, PAGE_EXECUTE_READWRITE, &protect_flag);
	std::memcpy(pAddress, this->pData, this->size);
	VirtualProtect(pAddress, this->size, protect_flag, &protect_flagb);
	FlushInstructionCache(Game::hInstance, (LPVOID)pAddress, size);
}

void Patch::Apply_RAW(uintptr_t offset, std::initializer_list<BYTE> data)
{
	Patch::Apply_RAW(offset, data.size(), const_cast<byte*>(data.begin()));
}

void Patch::Apply_RAW(uintptr_t offset, size_t sz , BYTE* data)
{
	PatchWrapper dummy { offset, sz, data };
}

void Patch::Apply_LJMP(uintptr_t offset, uintptr_t pointer)
{
	const _LJMP data(offset, pointer);
	Patch::Apply_RAW(offset, data.size(), (BYTE*)&data);
}

void Patch::Apply_CALL(uintptr_t offset, uintptr_t pointer)
{
	const _CALL data(offset, pointer);
	Patch::Apply_RAW(offset, data.size(), (BYTE*)&data);
}

void Patch::Apply_CALL6(uintptr_t offset, uintptr_t pointer)
{
	const _CALL6 data(offset, pointer);
	Patch::Apply_RAW(offset, data.size(), (BYTE*)&data);
}

void Patch::Apply_VTABLE(uintptr_t offset, uintptr_t pointer)
{
	const _VTABLE data(offset, pointer);
	Patch::Apply_RAW(offset, data.size(), (BYTE*)&data);
}

std::vector<module_export> Patch::enumerate_module_exports(HMODULE handle)
{
	const auto image_base = reinterpret_cast<const BYTE*>(handle);
	const auto image_header = reinterpret_cast<const IMAGE_NT_HEADERS*>(image_base +
		reinterpret_cast<const IMAGE_DOS_HEADER*>(image_base)->e_lfanew);

	if (image_header->Signature != IMAGE_NT_SIGNATURE ||
		image_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size == 0)
		return {}; // The handle does not point to a valid module

	const auto export_dir = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(image_base +
		image_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	const auto export_base = static_cast<WORD>(export_dir->Base);

	if (export_dir->NumberOfFunctions == 0)
		return {}; // This module does not contain any exported functions

	std::vector<module_export> exports;
	exports.reserve(export_dir->NumberOfNames);

	for (size_t i = 0; i < exports.capacity(); i++)
	{
		module_export& symbol = exports.emplace_back();
		symbol.ordinal = export_base +
			reinterpret_cast<const  WORD*>(image_base + export_dir->AddressOfNameOrdinals)[i];
		symbol.name = reinterpret_cast<const char*>(image_base +
			reinterpret_cast<const DWORD*>(image_base + export_dir->AddressOfNames)[i]);
		symbol.address = const_cast<void*>(reinterpret_cast<const void*>(image_base +
			reinterpret_cast<const DWORD*>(image_base + export_dir->AddressOfFunctions)[symbol.ordinal - export_base]));
	}

	return exports;
}

uintptr_t Patch::GetModuleBaseAddress(const char* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetProcessId(Patch::CurrentProcess));
	if (hSnap != NULL && hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry { };
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_strcmpi(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			}
			while (Module32Next(hSnap, &modEntry));
		}

		CloseHandle(hSnap);
	}

	return modBaseAddr;
}

#include <DebugLog.h>

DWORD Patch::GetDebuggerProcessId(DWORD dwSelfProcessId)
{
	DWORD dwParentProcessId = NULL;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(2, 0);

	if (hSnapshot != NULL && hSnapshot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe32 { };
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &pe32))
		{
			do
			{
				if (pe32.th32ProcessID == dwSelfProcessId)
				{
					dwParentProcessId = pe32.th32ParentProcessID;
					break;
				}
			}
			while (Process32Next(hSnapshot, &pe32));
		}

		CloseHandle(hSnapshot);
	}

	return dwParentProcessId;
}

void Patch::PrintAllModuleAndBaseAddr()
{	// Get a handle to the current process
	// Enumerate the loaded modules in the process
	HMODULE hModules[1024];
	DWORD cbNeeded;
	if (EnumProcessModules(Patch::CurrentProcess, hModules, sizeof(hModules), &cbNeeded))
	{
		const int moduleCount = static_cast<int>(cbNeeded / sizeof(HMODULE));
		for (int i = 0; i < moduleCount; ++i)
		{
			// Get the base name of the module
			CHAR moduleName[MAX_PATH] = { 0 };
			if (GetModuleBaseNameA(Patch::CurrentProcess, hModules[i], moduleName, sizeof(moduleName)))
			{
				// Get information about the module
				MODULEINFO info = { 0 };
				if (GetModuleInformation(Patch::CurrentProcess, hModules[i], &info, sizeof(info)))
				{
					DWORD_PTR image_base = (DWORD_PTR)hModules[i];
					PIMAGE_DOS_HEADER dosHeaders = (PIMAGE_DOS_HEADER)image_base;
					PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(image_base + dosHeaders->e_lfanew);
					//void* image_base_void = (void*)image_base;

					if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
						continue; // The handle does not point to a valid module

					_strlwr_s(moduleName);
					moduleName[0] &= ~0x20; // LOL HACK to uppercase a letter

					dllData& data = Patch::ModuleDatas.emplace_back(moduleName, hModules[i], (uintptr_t)info.lpBaseOfDll, (size_t)info.SizeOfImage);

					if (ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size != 0)
					{
						const IMAGE_EXPORT_DIRECTORY* export_dir = (const IMAGE_EXPORT_DIRECTORY*)(image_base +
							ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
						const WORD export_base = (WORD)export_dir->Base;

						if (export_dir->NumberOfFunctions != 0)
						{
							data.Exports.reserve(export_dir->NumberOfNames);

							for (size_t a = 0; a < (size_t)export_dir->NumberOfNames; a++)
							{
								module_export& symbol = data.Exports.emplace_back();
								symbol.ordinal = export_base +
									reinterpret_cast<const  WORD*>(image_base + export_dir->AddressOfNameOrdinals)[a];
								symbol.name = reinterpret_cast<const char*>(image_base +
								reinterpret_cast<const DWORD*>(image_base + export_dir->AddressOfNames)[a]);
								symbol.address = const_cast<void*>(
									reinterpret_cast<const void*>(image_base +
										reinterpret_cast<const DWORD*>(image_base + export_dir->AddressOfFunctions)[symbol.ordinal - export_base]));
							}
						}
					}

					IMAGE_DATA_DIRECTORY importsDirectory = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

					if (importsDirectory.Size != 0)
					{
						PIMAGE_IMPORT_DESCRIPTOR importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(importsDirectory.VirtualAddress + image_base);

						PIMAGE_THUNK_DATA originalFirstThunk = (PIMAGE_THUNK_DATA)(image_base + importDescriptor->OriginalFirstThunk);
						PIMAGE_THUNK_DATA firstThunk = (PIMAGE_THUNK_DATA)(image_base + importDescriptor->FirstThunk);

						while (originalFirstThunk->u1.AddressOfData != NULL)
						{
							module_Import& symbol = data.Impors.emplace_back();
							symbol.name = ((PIMAGE_IMPORT_BY_NAME)(image_base + originalFirstThunk->u1.AddressOfData))->Name;
							symbol.address = (void*)firstThunk->u1.Function;
							++originalFirstThunk;
							++firstThunk;
						}
					}

					/*
					char patchbuffer[MAX_PATH] = { 0 };
					void* buffer = nullptr;
					int len = 0;

					for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++)
					{
						const auto sct_hdr = IMAGE_FIRST_SECTION(ntHeaders) + i;

						if (strncmp(PATCH_SECTION_NAME, (char*)sct_hdr->Name, 8) == 0)
						{
							buffer = (void*)((DWORD)data.Handle + sct_hdr->VirtualAddress);
							len = sct_hdr->Misc.VirtualSize;
							break;
						}
					}

					if (len > 0 && buffer)
					{
						for (int offset = 0; offset < len;)
						{
							const auto pPatch = (Patch*)((DWORD)buffer + offset);

							if (!pPatch)
								break;

							if (pPatch->offset == 0)
								return;

							const char* DataType = "unknown";
#pragma pack(push, 1)
#pragma warning(push)
#pragma warning( disable : 4324)
							struct DataTypeStruct
							{
								BYTE command;
								DWORD pointer;
							};
#pragma warning(pop)
#pragma pack(pop)
							const auto dataptr = (DataTypeStruct*)((DWORD)pPatch + pPatch->size + 0x3);

							switch (pPatch->size)
							{
							case 5:
							{
								switch (dataptr->command)
								{
								case CALL_LETTER:
									DataType = "CALLJMP";
									break;
								case LJMP_LETTER:
									DataType = "LJMP";
									break;
								default:
									break;
								}

								break;
							}
							case 4:
								DataType = "VTABLEJMP";
								break;
							case 6:
								DataType = "CALL6";
								break;
							default:
								break;
							}

							_snprintf(patchbuffer, sizeof(patchbuffer), "Patch[%s] Offs [%d - %x(%d)]", DataType, offset, pPatch->offset, pPatch->size);
							data.Patches.emplace_back(patchbuffer);
							offset += 0x5 + 0x3 + pPatch->size;
						}
					}
					*/
				}
			}
		}
	}
}

uintptr_t Patch::GetEATAddress(const char* moduleName, const char* funcName)
{
	for (auto const& modules : ModuleDatas) {
		if (strcmp(moduleName, modules.ModuleName.c_str()) == 0 && !modules.Exports.empty()) {
			for (auto const& exportData : modules.Exports) {
				if (strcmp(exportData.name, funcName) == 0) {
					return (uintptr_t)exportData.address;
				}
			}
		}
	}

	return 0xffffffff;
}

uintptr_t Patch::GetIATAddress(const char* moduleName, const char* funcName)
{
	for (auto const& modules : ModuleDatas) {
		if (strcmp(moduleName, modules.ModuleName.c_str()) == 0 && !modules.Impors.empty()) {
			for (auto const& importData : modules.Impors) {
				if (strcmp(importData.name, funcName) == 0) {
					return (uintptr_t)importData.address;
				}
			}
		}
	}

	return 0xffffffff;
}
