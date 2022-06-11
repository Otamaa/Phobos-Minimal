#pragma once
#include <Base/Always.h>

class MapViewOfFileClass
{
public:
	explicit MapViewOfFileClass(const wchar_t* fileName);
	~MapViewOfFileClass();

	LPVOID GetMapViewOfFile() const { return FileBase; }
	PIMAGE_DOS_HEADER GetDosHeader() const { return DosHeader; }
	PIMAGE_NT_HEADERS GetNtHeader() const { return NTHeader; }
	PIMAGE_OPTIONAL_HEADER GetOptionalHeader() const { return OptionalHeader; }
	PIMAGE_SECTION_HEADER GetSectionHeaders() const { return SectionHeaders; }
	WORD GetSectionHeaderCount() const { return NTHeader ? NTHeader->FileHeader.NumberOfSections : 0; }

private:
	HANDLE File;
	HANDLE FileMapping;
	LPVOID FileBase;
	PIMAGE_DOS_HEADER DosHeader;
	PIMAGE_NT_HEADERS NTHeader;
	PIMAGE_OPTIONAL_HEADER OptionalHeader;
	PIMAGE_SECTION_HEADER SectionHeaders;
};

struct ImageSectionInfo
{
	LPVOID BaseOfCode;
	LPVOID BaseOfData;
	SIZE_T SizeOfCode;
	SIZE_T SizeOfData;
};

bool GetModuleSectionInfo(ImageSectionInfo& info, HMODULE nMod = NULL);