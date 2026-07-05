#pragma once

#include <CCINIClass.h>
#include <Point2D.h>
#include <Point3D.h>
#include <RectangleStruct.h>
#include <string>
#include <Straws.h>
#include <CRC.h>

class Pipe;
class FileStraw;
/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class NOVTABLE FakeCCINIClass : public CCINIClass
{
public:
	static int IteratorValue;

public:
	int _Load(FileStraw* ffile, bool loadcomments);
	int _Get_String(char const* section, char const* entry, char const* defvalue, char* buffer, int size);

	int _Get_Int(char const* section, char const* entry, int defvalue);
	bool _Get_Bool(char const* section, char const* entry, bool defvalue);
	double _Get_Float(char const* section, char const* entry, double defvalue);
	Point2D _Get_Point(char const* section, char const* entry, Point2D const& defvalue);
	Point3D _Get_Point(char const* section, char const* entry, Point3D const& defvalue);
	Vector3D<float> _Get_Point(char const* section, char const* entry, Vector3D<float> const& defvalue);
	int _Get_UUBlock(char const* section, void* block, int len);
	int _Get_TextBlock(char const* section, char* buffer, int len);
	CLSID _Get_UUID(char const* section, char const* entry, CLSID defvalue);
	RectangleStruct _Get_RectangleStruct(char const* section, char const* entry, RectangleStruct const& defvalue);

	int __GetPipType(const char* pSection, const char* pKey, int fallback);
	int GetStringOld(const char* section, const char* entry, const char* defvalue, char* buffer, size_t length);
	bool WriteStringOld(const char* section, const char* entry, const char* string);

	void Inherit_File(INIClass const& ini);
	void Include_File(INIClass const& ini);

	int _ReadString(const char* pSection, const char* pKey, const char* pDefault, char* pBuffer, size_t szBufferSize)
	{
		JMP_THIS(0x528A10);
	}

	std::string Get_String(char const* section, char const* entry, std::string const& defvalue) ;
	std::string Get_TextBlock(char const* section);

	bool __Load(Straw* straw, bool loadcomments);

	static size_t __fastcall Read_Line(Straw* file, char* buffer, int len, bool* eof)
	{
		if (!len || !buffer)
			return 0;

		int count = 0;
		char c;

		if (file->Get(&c, 1) != 1)
		{
			*eof = true;
			buffer[0] = '\0';
			return 0;
		}

		while (c != '\n')
		{
			// (source - buffer) + 1 < len
			// equivalent to: count + 1 < len
			// equivalent to: count < len - 1
			if (c != '\r' && count < len - 1)
			{
				buffer[count] = c;
				++count;
			}

			if (file->Get(&c, 1) != 1)
			{
				*eof = true;
				buffer[count] = '\0';
				return strlen(buffer);
			}
		}

		buffer[count] = '\0';
		return strlen(buffer);
	}

	static char* __fastcall Extract_Line_Comment(char* buffer, int* pre_indent_cursor, int* entry_indent, int* comment_cursor)
	{
		*pre_indent_cursor = -1;
		*entry_indent = -1;
		*comment_cursor = -1;

		char* comment = nullptr;
		int   column = 0;

		 if (*buffer) {
			unsigned char chr = static_cast<unsigned char>(*buffer);

			do
			{
				// Track start-of-value indent: first non-space char after '='
				if (*pre_indent_cursor >= 0 && *entry_indent < 0 && chr > ' ')
				{
					*entry_indent = column;
				}

				if (chr == ';')
				{
					*comment_cursor = column;
					comment = buffer + 1;
					break; // comment found - stop scanning
				}

				if (chr == '\t')
				{
					// Expand to next 8-column tab stop
					column = (column & ~7) + 8;
				}
				else if (chr == '=')
				{
					if (*pre_indent_cursor < 0)
					{
						*pre_indent_cursor = column;
					}
					++column;
				}
				else
				{
					++column;
				}

				chr = static_cast<unsigned char>(*++buffer);

			}
			while (chr);
		}

		// Clamp -1 sentinels to 0 (nothing found = treat as column 0)
		if (*pre_indent_cursor < 0) *pre_indent_cursor = 0;
		if (*entry_indent < 0) *entry_indent = 0;
		if (*comment_cursor < 0) *comment_cursor = 0;

		return comment;
	}

	INISection* _GetSection(const char* pSection) const
	{
		if (!pSection) {
			return nullptr;
		}

		const int crc = SafeChecksummer()(pSection, std::strlen(pSection));

		if (!this->SectionIndex.IsPresent(crc)) {
			return nullptr;
		}

		return this->SectionIndex.Archive->Data;
	}

	INIEntry* _GetKey(const char* section, const char* entry)
	{
		INISection* pSection = nullptr;

		if (section == this->CurrentSectionName) {
			// Fast path: same pointer as last lookup, reuse cached section.
			pSection = this->CurrentSection;
		} else {
			if (!section) {
				this->CurrentSectionName = nullptr;
				this->CurrentSection = nullptr;
				return nullptr;
			}

			int sectionCrc = SafeChecksummer()(section, std::strlen(section));

			if (!this->SectionIndex.IsPresent(sectionCrc)) {
				this->CurrentSectionName = nullptr;
				this->CurrentSection = nullptr;
				return nullptr;
			}

			pSection = this->SectionIndex.Archive->Data;

			if (!pSection) {
				this->CurrentSectionName = nullptr;
				this->CurrentSection = nullptr;
				return nullptr;
			}

			// Update section pointer cache.
			this->CurrentSectionName = section;
			this->CurrentSection = pSection;
		}

		if (!pSection || !entry) {
			return nullptr;
		}

		const int entryCrc = SafeChecksummer()(entry, std::strlen(entry));
		auto& entryIndex = pSection->EntryIndex;

		if (!entryIndex.IsPresent(entryCrc)) {
			return nullptr;
		}

		return entryIndex.Archive->Data;
	}

	int _GetKeyCount(const char* section) const
	{
		if (!section) {
			return 0;
		}

		const int crc = SafeChecksummer()(section, std::strlen(section));

		if (!this->SectionIndex.IsPresent(crc)) {
			return 0;
		}

		const auto* pSection = this->SectionIndex.Archive->Data;
		return pSection ? pSection->EntryIndex.IndexCount : 0;
	}

	const char* _GetKeyName(const char* section, int index) const {
		if (!section) {
			return nullptr;
		}

		const int crc = SafeChecksummer()(section, std::strlen(section));

		if (!this->SectionIndex.IsPresent(crc)) {
			return nullptr;
		}

		const auto* pSection = this->SectionIndex.Archive->Data;

		if (!pSection || index >= pSection->EntryIndex.IndexCount) {
			return nullptr;
		}

		// Walk EntryList by insertion order (NOT EntryIndex which is CRC-sorted).
		// Sentinel nodes have NextNode == null or PrevNode == null — skip them.
		auto node = pSection->Entries.First();

		while (node && node->NextValid() && node->PrevValid())
		{
			if (index == 0) {
				return node->Key;
			}

			node = node->Next();
			--index;
		}

		return nullptr;
	}

	void Free_Comment_List(INIComment* head);

	int _SaveToPipe(Pipe* pipe);

	static void Reset() {
		IteratorValue = 0;
	}
};
