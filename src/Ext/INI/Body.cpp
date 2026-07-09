#include "Body.h"

#include <Phobos.h>
#include <Phobos.CRT.h>

#include <Base/Always.h>
#include <Straws.h>

#include <Utilities/Debug.h>

#include <Pipes.h>

#include <Phobos.INI.h>

/**
 *  Reads a line from the INI file.
 *
 *  @author: ZivDero
 */
bool Read_Line(FileStraw* file, std::string& line)
{
	line.clear();

	while (true) {
		char c;
		// EOF
		if (file->Get(&c, sizeof(c)) != sizeof(c)) { 
			return !line.empty();
		}

		if (c == '\n') {
			return true;
		}

		if (c != '\r') {
			line.push_back(c);
		}
	}
}

std::string FakeCCINIClass::Get_String(char const* section, char const* entry, std::string const& defvalue)
{
	char buffer[Phobos::readLength];
	if (ReadString(section, entry, defvalue.c_str(), buffer, Phobos::readLength) > 0) {
		return std::string(buffer);
	}

	return defvalue;
}

std::string FakeCCINIClass::Get_TextBlock(char const* section)
{
	std::string buffer;
	int count = GetKeyCount(section);

	if (count > 0) {
		buffer.resize(Phobos::readLength * count + 1);

		const int len = GetTextBlock(section, buffer.data(), buffer.size());

		if (len > 0) {
			buffer.resize(std::strlen(buffer.data()));
		} else {
			buffer.clear();
		}
	}

	return buffer;
}

/**
 *  Extracts a section name from a line.
 *
 *  @author: ZivDero
 */
std::string Extract_Section_Name(std::string_view line)
{
	auto l = line.find('[');
	if (l == std::string_view::npos) return {};

	auto r = line.find(']', l + 1);
	if (r == std::string_view::npos || r <= l + 1) return {};

	std::string name(line.substr(l + 1, r - l - 1));
	CRT::strtrim(name.data());

	return name;
}

constexpr const char* inherit_section = "$Inherit";
constexpr const char* include_section = "$Include";

/**
 *  Inherits the INI data from another INI file.
 *
 *  @author: ZivDero
 */
void FakeCCINIClass::Inherit_File(INIClass const& ini)
{
	for (const INISection* section = ini.Sections.First(); section; section = section->NextValid()) {
		if (CRT::strcmp(section->Name, inherit_section) == 0 || CRT::strcmp(section->Name, include_section) == 0) {
			continue;
		}

		for (const INIEntry* entry = section->Entries.First(); entry; entry = entry->NextValid()) {
			if (Is_Present(section->Name, entry->Key)) {
				continue;
			}
			this->WriteString(section->Name, entry->Key, entry->Value);
		}
	}
}

/**
 *  Includes the INI data from another INI file.
 *
 *  @author: ZivDero
 */
void FakeCCINIClass::Include_File(INIClass const& ini)
{
	for (const INISection* section = ini.Sections.First(); section; section = section->NextValid()) {
		if (CRT::strcmp(section->Name, inherit_section) == 0 || CRT::strcmp(section->Name, include_section) == 0) {
			continue;
		}

		for (const INIEntry* entry = section->Entries.First(); entry; entry = entry->NextValid()) {
			this->WriteString(section->Name, entry->Key, entry->Value);
		}
	}
}

/**
 *  Loads the INI data from the data stream (straw).
 *
 *  @author: ZivDero, tomsons26
 */
int FakeCCINIClass::_Load(FileStraw* ffile, bool loadcomments)
{
	std::string line;
	line.reserve(1024);

	CacheStraw file;
	file.Get_From(ffile);

	std::string section;
	bool isEof;

	while (Straw::Read_Line(&file,line.data(), 1024,&isEof))
	{
		/**
		 *  Determine if this line is a comment or blank line. Throw it out if it is.
		 */
		Strip_Comments(line.data());
		if (line.empty() || line[0] == ';' || line[0] == '=')
		{
			continue;
		}

		/**
		 *  Process a section.
		 */
		if (Line_Contains_Section(line.data()))
		{
			section = Extract_Section_Name(line);
			CRT::strtrim(section.data());
			continue;
		}

		/**
		 *  We haven't found the first section yet, discard the line.
		 */
		if (section.empty())
		{
			continue;
		}

		/**
		 *  The line isn't an obvious comment. Make sure that there is the "=" character
		 *  at an appropriate spot.
		 */
		char* buffer = line.data();
		char* divider = strchr(buffer, '=');
		if (!divider) continue;

		/**
		 *  Split the line into entry and value sections. Be sure to catch the
		 *  "=foobar" and "foobar=" cases. These lines are ignored.
		 */
		*divider++ = '\0';
		CRT::strtrim(buffer);
		if (!strlen(buffer)) continue;

		CRT::strtrim(divider);
		if (!strlen(divider)) continue;

		if (this->WriteString(section.c_str(), buffer, divider) == false)
		{
			return false;
		}
	}

	std::vector<std::string> inherits;
	if (Section_Present(inherit_section)) {
		int count = GetKeyCount(inherit_section);
		for (int i = 0; i < count; i++) {
			std::string entry = this->Get_String(inherit_section, GetKeyName(inherit_section, i), {});
			if (std::ranges::find(inherits, entry) == inherits.end()) {
				inherits.emplace_back(entry);
			}
		}
	}

	std::vector<std::string> includes;
	if (Section_Present(include_section)) {
		int count = GetKeyCount(include_section);
		for (int i = 0; i < count; i++) {
			std::string entry = this->Get_String(include_section, GetKeyName(include_section, i), {});
			if (std::ranges::find(includes, entry) == includes.end()) {
				includes.emplace_back(entry);
			}
		}
	}

	for (auto& filename : inherits) {
		CCFileClass ifile(filename.c_str());
		if (ifile.IsAvaible()) {
			INIClass iini;
			iini.Load(&ifile, false);
			Inherit_File(iini);
		} else {
			Debug::Log("FakeCCINIClass::_Load - Inherit file not found: %s\n", filename.c_str());
			char error[512];
			std::snprintf(error, sizeof(error), "FakeCCINIClass::_Load - Inherit file not found: %s\nThe game will now exit.", filename.c_str());
			MessageBox(Game::hWnd(), error, "Phobos", MB_OK | MB_ICONERROR);
			Debug::ExitGame(EXIT_FAILURE);
		}
	}

	for (auto& filename : includes){
		CCFileClass ifile(filename.c_str());
		if (ifile.IsAvaible()) {
			INIClass iini;
			iini.Load(&ifile, false);
			Include_File(iini);
		} else {
			Debug::Log("FakeCCINIClass::_Load - Include file not found: %s\n", filename.c_str());
			char error[512];
			std::snprintf(error, sizeof(error), "FakeCCINIClass::_Load - Include file not found: %s\nThe game will now exit.", filename.c_str());
			MessageBox(Game::hWnd(), error, "Phobos", MB_OK | MB_ICONERROR);
			Debug::ExitGame(EXIT_FAILURE);
		}
	}

	return true;
}

/**
 *  Cached inherited sections.
 */
static std::unordered_map<void*, std::vector<std::string>> InheritedSections;

/**
 *  Get_String replacement that checks inherited sections if the entry is not found in the main section.
 *
 *  @author: ZivDero, tomsons26
 */
int FakeCCINIClass::_Get_String(char const* section, char const* entry, char const* defvalue, char* buffer, int size)
{
	/**
	 *  Verify that the parameters are nominally legal.
	 */
	if (buffer == nullptr || size < 2 || section == nullptr || entry == nullptr) return 0;

	/**
	 *  Fetch the entry string if it is present.
	 */
	bool has_value = false;
	INIEntry* entryptr = this->GetKey(section, entry);
	if (entryptr != nullptr && entryptr->Value != nullptr)
	{
		defvalue = entryptr->Value;
		has_value = true;
	}

	/**
	 *  Attempt to find the entry string among inherited sections. If not,
	 *  then the normal default value will be used as the entry value.
	 */
	if (!has_value)
	{
		INISection* sectionptr = this->GetSection(section);
		if (InheritedSections.contains(sectionptr))
		{
			for (const std::string& inherited_section : InheritedSections[sectionptr])
			{
				int count = this->_ReadString(inherited_section.c_str(), entry, "", buffer, size);
				if (count > 0)
				{
					//DEBUG_INFO("Fetched [%s]->%s from %s\n", section, entry, inherited_section.c_str());
					return count;
				}
			}
		}
	}

	/**
	 *  Fill in the buffer with the entry value and return with the length of the string.
	 */
	if (defvalue == nullptr)
	{
		buffer[0] = '\0';
		return 0;
	}
	else
	{
		CRT::strncpy(buffer, defvalue, size);
		buffer[size - 1] = '\0';
		CRT::strtrim(buffer);
		return strlen(buffer);
	}
}

///**
// *  Caches the inherited sections upon putting the $Inherits entry.
// *
// *  @author: ZivDero
// */
//ASMJIT_PATCH(0x004DDD3A, _INIClass_this->WriteString_Cache_Inherits, 5)
//{
//	GET_STACK(INIClass::INISection*, secptr, 0x14);
//	GET_STACK(char const*, entry, 0x50);
//	GET_STACK(char const*, string, 0x54);
//
//	if (strcmp(entry, "$Inherits") == 0)
//	{
//		InheritedSections[secptr].clear();
//		for (std::string_view part : SplitView(string, ',', StringSplitOptions::RemoveEmpty | StringSplitOptions::Trim))
//		{
//			InheritedSections[secptr].emplace_back(part);
//		}
//	}
//
//	return 0;
//}
//
//ASMJIT_PATCH(0x004DED22, _INIClass_INISection_DTOR_Clear_Inherits, 5)
//{
//	GET(INIClass::INISection*, secptr, ECX);
//
//	InheritedSections.erase(secptr);
//
//	return 0;
//}


/**
 *  Various INI getter replacements.
 *
 *  @author: tomsons26, ZivDero
 */
int FakeCCINIClass::_Get_Int(char const* section, char const* entry, int defvalue)
{
	if (section == nullptr || entry == nullptr) return defvalue;

	std::string value = this->Get_String(section, entry, {});
	if (!value.empty())
	{
		if (value[0] == '$')
		{
			sscanf(value.c_str(), "$%x", &defvalue);
		}
		else
		{
			if (tolower(value.back()) == 'h')
			{
				sscanf(value.c_str(), "%xh", &defvalue);
			}
			else
			{
				defvalue = atoi(value.c_str());
			}
		}
	}
	return defvalue;
}

bool FakeCCINIClass::_Get_Bool(char const* section, char const* entry, bool defvalue)
{
	if (section == nullptr || entry == nullptr) return defvalue;

	std::string value = this->Get_String(section, entry, {});
	if (!value.empty())
	{
		switch (toupper(value[0]))
		{
		case 'Y':
		case 'T':
		case '1':
			return true;

		case 'N':
		case 'F':
		case '0':
			return false;
		}
	}
	return defvalue;
}

double FakeCCINIClass::_Get_Float(char const* section, char const* entry, double defvalue)
{
	if (section == nullptr || entry == nullptr) return defvalue;

	std::string value = this->Get_String(section, entry, {});
	if (!value.empty())
	{
		float val;
		sscanf(value.c_str(), "%f", &val);
		defvalue = val;
		if (value.find('%') != std::string::npos)
		{
			defvalue /= 100.0;
		}
	}
	return defvalue;
}

Point2D FakeCCINIClass::_Get_Point(char const* section, char const* entry, Point2D const& defvalue)
{
	char buffer[64];
	if (this->_ReadString(section, entry, "", buffer, sizeof(buffer))) {
		int x, y;
		std::sscanf(buffer, "%d,%d", &x, &y);
		return { x, y };
	}

	return defvalue;
}

Point3D FakeCCINIClass::_Get_Point(char const* section, char const* entry, Point3D const& defvalue)
{
	char buffer[64];
	if (this->_ReadString(section, entry, "", buffer, sizeof(buffer)))
	{
		int x, y, z;
		std::sscanf(buffer, "%d,%d,%d", &x, &y, &z);
		return { x, y, z };
	}
	return defvalue;
}

Vector3D<float> FakeCCINIClass::_Get_Point(char const* section, char const* entry, Vector3D<float> const& defvalue)
{
	char buffer[64];
	if (this->_ReadString(section, entry, "", buffer, sizeof(buffer)))
	{
		float x, y, z;
		std::sscanf(buffer, "%f,%f,%f", &x, &y, &z);
		return { x, y, z };
	}
	return defvalue;
}

int FakeCCINIClass::_Get_UUBlock(char const* section, void* block, int len)
{
	if (section == nullptr) return 0;

	Base64Pipe b64pipe(Base64Pipe::CodeControl::DECODE);
	BufferPipe bpipe(block, len);

	b64pipe.Put_To(&bpipe);

	int total = 0;
	int counter = this->GetKeyCount(section);
	for (int index = 0; index < counter; index++)
	{
		char buffer[128];

		int length = this->_ReadString(section, this->GetKeyName(section, index), "=", buffer, sizeof(buffer));
		int outcount = b64pipe.Put(buffer, length);
		total += outcount;
	}
	total += b64pipe.End();
	return total;
}

int FakeCCINIClass::_Get_TextBlock(char const* section, char* buffer, int len)
{
	if (len <= 0) return 0;

	buffer[0] = '\0';
	if (len <= 1) return 0;

	int elen = this->GetKeyCount(section);
	int total = 0;
	for (int index = 0; index < elen; index++)
	{
		if (index > 0)
		{
			*buffer++ = ' ';
			len--;
			total++;
		}

		this->_ReadString(section, this->GetKeyName(section, index), "", buffer, len);

		int partial = std::strlen(buffer);
		total += partial;
		buffer += partial;
		len -= partial;
		if (len <= 1) break;
	}
	return total;
}

CLSID FakeCCINIClass::_Get_UUID(char const* section, char const* entry, CLSID defvalue)
{
	char buffer[128];

	if (this->_ReadString(section, entry, "", buffer, sizeof(buffer)))
	{
		wchar_t wBuffer[128];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buffer, -1, wBuffer, std::size(wBuffer));
		CLSID uuid;
		if (SUCCEEDED(CLSIDFromString(wBuffer, &uuid)))
		{
			return uuid;
		}
	}
	return defvalue;
}

RectangleStruct FakeCCINIClass::_Get_RectangleStruct(char const* section, char const* entry, RectangleStruct const& defvalue)
{
	char buffer[64];

	if (this->_ReadString(section, entry, "", buffer, sizeof(buffer)))
	{
		RectangleStruct retval = defvalue;
		sscanf(buffer, "%d,%d,%d,%d", &retval.X, &retval.Y, &retval.Width, &retval.Height);
		return retval;
	}
	return defvalue;
}

int FakeCCINIClass::_SaveToPipe(Pipe* pipe)
{
	this->CurrentSectionName = nullptr;
	this->CurrentSection = nullptr;

	int total = 0;

	// Space padding buffer — filled with spaces, null-terminated at end.
	char spacebuff[512];
	std::memset(spacebuff, ' ', sizeof(spacebuff));
	spacebuff[511] = '\0';

	// -----------------------------------------------------------------------
	// Walk sections (insertion order via SectionList)
	// -----------------------------------------------------------------------
	for (auto* sectionptr = this->Sections.First();
		 sectionptr && sectionptr->Next() && sectionptr->Prev();
		 sectionptr = static_cast<INISection*>(sectionptr->Next())) {
		// Blank line between sections when the incoming section has no
		// leading comment block of its own.
		if (total > 0 && !sectionptr->Comments)
			total += pipe->Put("\r\n", 2);

		// Section leading comments
		for (auto* c = sectionptr->Comments; c; c = static_cast<INIComment*>(c->Next)) {
			if (c->Value) {
				total += pipe->Put(c->Value, std::strlen(c->Value));
				total += pipe->Put("\r\n", 2);
			}
		}

		// "[SectionName]\r\n"
		total += pipe->Put("[", 1);
		total += pipe->Put(sectionptr->Name, std::strlen(sectionptr->Name));
		total += pipe->Put("]", 1);
		total += pipe->Put("\r\n", 2);

		// -------------------------------------------------------------------
		// Walk entries (insertion order via EntryList)
		// -------------------------------------------------------------------
		for (auto* entryptr = sectionptr->Entries.First();
			 entryptr && entryptr->Next() && entryptr->Prev();
			 entryptr = static_cast<INIEntry*>(entryptr->Next())) {
			// Entry leading comments
			for (auto* c = entryptr->Comments; c; c = static_cast<INIComment*>(c->Next)) {
				if (c->Value) {
					total += pipe->Put(c->Value, std::strlen(c->Value));
					total += pipe->Put("\r\n", 2);
				}
			}

			const int keyLen = static_cast<int>(std::strlen(entryptr->Key));
			const int valueLen = static_cast<int>(std::strlen(entryptr->Value));

			// Running column tracker (bytes written so far on this line).
			int column = 0;

			// Write key
			total += pipe->Put(entryptr->Key, keyLen);
			column += keyLen;

			// Spaces between key and '=' (SUSPECT: can be negative -> no write)
			int preindent = entryptr->PreIndentCursor - keyLen;
			if (preindent > 256) preindent = 256;
			if (preindent > 0) {
				total += pipe->Put(spacebuff, preindent);
				column += preindent;
			}

			// Write '='
			total += pipe->Put("=", 1);
			column += 1;

			// Spaces between '=' and value
			// SUSPECT: postindent subtracts `column` which already includes
			// preindent and '=', matching vanilla's running-pos calculation.
			int postindent = entryptr->PostIndentCursor - column - keyLen;
			if (postindent > 256) postindent = 256;
			if (postindent > 0) {
				total += pipe->Put(spacebuff, postindent);
				column += postindent;
			}

			// Write value
			total += pipe->Put(entryptr->Value, valueLen);
			column += valueLen;

			// Optional inline comment
			if (entryptr->CommentString)
			{
				int commentindent = entryptr->CommentCursor - column - keyLen;
				if (commentindent > 256) commentindent = 256;
				if (commentindent > 0)
					total += pipe->Put(spacebuff, commentindent);

				total += pipe->Put(";", 1);
				total += pipe->Put(entryptr->CommentString,
								   std::strlen(entryptr->CommentString));
			}

			total += pipe->Put("\r\n", 2);
		}
	}

	// -----------------------------------------------------------------------
	// Trailing loose line comments (after last section)
	// -----------------------------------------------------------------------
	for (auto* c = this->LineComments; c; c = static_cast<INIComment*>(c->Next)) {
		if (c->Value) {
			total += pipe->Put(c->Value, std::strlen(c->Value));
			total += pipe->Put("\r\n", 2);
		}
	}

	return total + pipe->End();
}