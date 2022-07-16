#include "Patch.h"

#include "Macro.h"
#include "Debug.h"
#include <Phobos.h>

#include <tlhelp32.h>

int GetSection(const char* sectionName, void** pVirtualAddress)
{
	char buf[MAX_PATH + 1] = { 0 };
	GetModuleFileName(NULL, buf, sizeof(buf));

	auto hInstance = Phobos::hInstance;

	auto pHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(((PIMAGE_DOS_HEADER)hInstance)->e_lfanew + (long)hInstance);

	for (int i = 0; i < pHeader->FileHeader.NumberOfSections; i++)
	{
		auto sct_hdr = IMAGE_FIRST_SECTION(pHeader) + i;

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
	const int len = GetSection(PATCH_SECTION_NAME, &buffer);

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

	DWORD protect_flag;
	VirtualProtect(pAddress, this->size, PAGE_EXECUTE_READWRITE, &protect_flag);
	memcpy(pAddress, this->pData, this->size);
	VirtualProtect(pAddress, this->size, protect_flag, nullptr);
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
	HANDLE hCurrentProcess = GetCurrentProcess();
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetProcessId(hCurrentProcess));
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
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
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

DWORD Patch::GetDebuggerProcessId(DWORD dwSelfProcessId)
{
	DWORD dwParentProcessId = NULL;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(2, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe32;
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
	}
	CloseHandle(hSnapshot);
	return dwParentProcessId;
};