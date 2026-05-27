#include "Body.h"

#include <Phobos.h>
#include <Phobos.CRT.h>

#include <Base/Always.h>
#include <Straws.h>
/**
 *  Convenient generator-like class for splitting and trimming strings.
 */
enum class StringSplitOptions : unsigned
{
	None = 0,
	RemoveEmpty = 1 << 0,
	Trim = 1 << 1,
};

inline StringSplitOptions operator|(StringSplitOptions a, StringSplitOptions b)
{
	return static_cast<StringSplitOptions>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

inline bool HasFlag(StringSplitOptions v, StringSplitOptions f)
{
	return (static_cast<unsigned>(v) & static_cast<unsigned>(f)) != 0;
}

class SplitView
{
	std::string_view s_;
	char delim_;
	StringSplitOptions opts_;

public:
	SplitView(std::string_view s, char delim, StringSplitOptions opts = StringSplitOptions::None) : s_(s), delim_(delim), opts_(opts) {}

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = std::string_view;
		using difference_type = std::ptrdiff_t;
		using pointer = const std::string_view*;
		using reference = const std::string_view&;

		iterator() : done_(true) {} // End iterator constructor

		iterator(std::string_view s, char delim, StringSplitOptions opts, bool done) : s_(s), delim_(delim), opts_(opts), pos_(0), done_(done)
		{
			if (!done_) advance();
		}

		reference operator*() const { return current_; }
		pointer operator->() const { return &current_; }

		iterator& operator++()
		{
			advance();
			return *this;
		}

		iterator operator++(int)
		{
			iterator tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const iterator& other) const
		{
			if (done_ && other.done_) return true;
			return done_ == other.done_ && pos_ == other.pos_;
		}
		bool operator!=(const iterator& other) const { return !(*this == other); }

	private:
		void advance()
		{
			while (pos_ <= s_.size())
			{
				size_t next_delim = s_.find(delim_, pos_);
				size_t end = (next_delim == std::string_view::npos) ? s_.size() : next_delim;

				std::string_view token = s_.substr(pos_, end - pos_);

				pos_ = (next_delim == std::string_view::npos) ? s_.size() + 1 : next_delim + 1;

				if (HasFlag(opts_, StringSplitOptions::Trim))
				{
					auto b = token.find_first_not_of(" \t");
					if (b == std::string_view::npos)
						token = {};
					else
					{
						auto e = token.find_last_not_of(" \t");
						token = token.substr(b, e - b + 1);
					}
				}

				if (token.empty() && HasFlag(opts_, StringSplitOptions::RemoveEmpty))
				{
					continue;
				}

				current_ = token;
				return;
			}
			done_ = true;
		}

		std::string_view s_;
		char delim_ = 0;
		StringSplitOptions opts_ = StringSplitOptions::None;
		size_t pos_ = 0;
		bool done_ = false;
		std::string_view current_;
	};

	iterator begin() const { return iterator { s_, delim_, opts_, false }; }
	iterator end() const { return iterator {}; }
};

/**
 *  Reads a line from the INI file.
 *
 *  @author: ZivDero
 */
bool Read_Line(FileStraw* file, std::string& line)
{
	line.clear();

	while (true)
	{
		char c;
		if (file->Get(&c, sizeof(c)) != sizeof(c))
		{ // EOF
			return !line.empty();
		}

		if (c == '\n')
		{
			return true;
		}

		if (c != '\r')
		{
			line.push_back(c);
		}
	}
}

std::string INIClassExt::Get_String(char const* section, char const* entry, std::string const& defvalue)
{
	char buffer[Phobos::readLength];
	if (ReadString(section, entry, defvalue.c_str(), buffer, Phobos::readLength) > 0)
	{
		return std::string(buffer);
	}
	return defvalue;
}


std::string INIClassExt::Get_TextBlock(char const* section)
{
	std::string buffer;

	int count = GetKeyCount(section);
	if (count > 0)
	{
		buffer.resize(Phobos::readLength * count + 1);

		int len = GetTextBlock(section, buffer.data(), buffer.size());
		if (len > 0)
		{
			buffer.resize(std::strlen(buffer.data()));
		}
		else
		{
			buffer.clear();
		}
	}

	return buffer;
}

///**
// *  Extracts a section name from a line.
// *
// *  @author: ZivDero
// */
//std::string Extract_Section_Name(std::string_view line)
//{
//	auto l = line.find('[');
//	if (l == std::string_view::npos) return {};
//
//	auto r = line.find(']', l + 1);
//	if (r == std::string_view::npos || r <= l + 1) return {};
//
//	std::string name(line.substr(l + 1, r - l - 1));
//	CRT::strtrim(name.data());
//
//	return name;
//}
//
//
///**
// *  Inherits the INI data from another INI file.
// *
// *  @author: ZivDero
// */
//void INIClassExt::Inherit_File(INIClass const& ini)
//{
//	for (const INISection* section = ini.Sections.First(); section; section = section->IsValid() ? section->Next() : nullptr)
//	{
//		if (strcmp(section->Name, "$Inherit") == 0 || strcmp(section->Name, "$Include") == 0)
//		{
//			continue;
//		}
//		for (const INIEntry* entry = section->Entries.First(); entry; entry = entry->IsValid() ? entry->Next() : nullptr)
//		{
//			if (Is_Present(section->Name, entry->Key))
//			{
//				continue;
//			}
//			this->WriteString(section->Name, entry->Key, entry->Value);
//		}
//	}
//}
//
//
///**
// *  Includes the INI data from another INI file.
// *
// *  @author: ZivDero
// */
//void INIClassExt::Include_File(INIClass const& ini)
//{
//	for (const INISection* section = ini.Sections.First(); section; section = section->Is_Valid() ? section->Next() : nullptr)
//	{
//		if (strcmp(section->Name, "$Inherit") == 0 || strcmp(section->Name, "$Include") == 0)
//		{
//			continue;
//		}
//		for (const INIEntry* entry = section->Entries.First(); entry; entry = entry->Is_Valid() ? entry->Next() : nullptr)
//		{
//			this->WriteString(section->Name, entry->Key, entry->Value);
//		}
//	}
//}
//
//
///**
// *  Loads the INI data from the data stream (straw).
// *
// *  @author: ZivDero, tomsons26
// */
//int INIClassExt::_Load(FileStraw* ffile, bool loadcomments)
//{
//	std::string line;
//	line.reserve(1024);
//
//	CacheStraw file;
//	file.Get_From(ffile);
//
//	std::string section;
//	bool isEof;
//
//	while (Straw::Read_Line(&file,line.data(), 1024,&isEof))
//	{
//
//		/**
//		 *  Determine if this line is a comment or blank line. Throw it out if it is.
//		 */
//		Strip_Comments(line.data());
//		if (line.empty() || line[0] == ';' || line[0] == '=')
//		{
//			continue;
//		}
//
//		/**
//		 *  Process a section.
//		 */
//		if (Line_Contains_Section(line.data()))
//		{
//			section = Extract_Section_Name(line);
//			CRT::strtrim(section.data());
//			continue;
//		}
//
//		/**
//		 *  We haven't found the first section yet, discard the line.
//		 */
//		if (section.empty())
//		{
//			continue;
//		}
//
//		/**
//		 *  The line isn't an obvious comment. Make sure that there is the "=" character
//		 *  at an appropriate spot.
//		 */
//		char* buffer = line.data();
//		char* divider = strchr(buffer, '=');
//		if (!divider) continue;
//
//		/**
//		 *  Split the line into entry and value sections. Be sure to catch the
//		 *  "=foobar" and "foobar=" cases. These lines are ignored.
//		 */
//		*divider++ = '\0';
//		CRT::strtrim(buffer);
//		if (!strlen(buffer)) continue;
//
//		CRT::strtrim(divider);
//		if (!strlen(divider)) continue;
//
//		if (this->WriteString(section.c_str(), buffer, divider) == false)
//		{
//			return false;
//		}
//	}
//
//	constexpr const char* inherit_section = "$Inherit";
//	constexpr const char* include_section = "$Include";
//
//	std::vector<std::string> inherits;
//	if (Section_Present(inherit_section))
//	{
//		int count = GetKeyCount(inherit_section);
//		for (int i = 0; i < count; i++)
//		{
//			std::string entry = this->GetString(inherit_section, GetKeyName(inherit_section, i), {});
//			if (std::ranges::find(inherits, entry) == inherits.end())
//			{
//				inherits.emplace_back(entry);
//			}
//		}
//	}
//
//	std::vector<std::string> includes;
//	if (Section_Present(include_section))
//	{
//		int count = GetKeyCount(include_section);
//		for (int i = 0; i < count; i++)
//		{
//			std::string entry = GetString(include_section, GetKeyName(include_section, i), {});
//			if (std::ranges::find(includes, entry) == includes.end())
//			{
//				includes.emplace_back(entry);
//			}
//		}
//	}
//
//	for (auto& filename : inherits)
//	{
//		CCFileClass ifile(filename.c_str());
//		if (ifile.Is_Available())
//		{
//			INIClass iini;
//			iini.Load(ifile);
//			Inherit_File(iini);
//		}
//		else
//		{
//			DEBUG_FATAL("INIClassExt::_Load - Inherit file not found: %s\n", filename.c_str());
//			char error[512];
//			std::snprintf(error, sizeof(error), "INIClassExt::_Load - Inherit file not found: %s\nThe game will now exit.", filename.c_str());
//			MessageBox(MainWindow, error, "Vinifera", MB_OK | MB_ICONERROR);
//			Emergency_Exit(EXIT_FAILURE);
//		}
//	}
//
//	for (auto& filename : includes)
//	{
//		CCFileClass ifile(filename.c_str());
//		if (ifile.Is_Available())
//		{
//			INIClass iini;
//			iini.Load(ifile);
//			Include_File(iini);
//		}
//		else
//		{
//			DEBUG_FATAL("INIClassExt::_Load - Include file not found: %s\n", filename.c_str());
//			char error[512];
//			std::snprintf(error, sizeof(error), "INIClassExt::_Load - Include file not found: %s\nThe game will now exit.", filename.c_str());
//			MessageBox(MainWindow, error, "Vinifera", MB_OK | MB_ICONERROR);
//			Emergency_Exit(EXIT_FAILURE);
//		}
//	}
//
//	return true;
//}
//
//
///**
// *  Cached inherited sections.
// */
//static std::unordered_map<void*, std::vector<std::string>> InheritedSections;
//
//
///**
// *  Get_String replacement that checks inherited sections if the entry is not found in the main section.
// *
// *  @author: ZivDero, tomsons26
// */
//int INIClassExt::_Get_String(char const* section, char const* entry, char const* defvalue, char* buffer, int size) const
//{
//	/**
//	 *  Verify that the parameters are nominally legal.
//	 */
//	if (buffer == nullptr || size < 2 || section == nullptr || entry == nullptr) return 0;
//
//	/**
//	 *  Fetch the entry string if it is present.
//	 */
//	bool has_value = false;
//	INIEntry* entryptr = Find_Entry(section, entry);
//	if (entryptr != nullptr && entryptr->Value != nullptr)
//	{
//		defvalue = entryptr->Value;
//		has_value = true;
//	}
//
//	/**
//	 *  Attempt to find the entry string among inherited sections. If not,
//	 *  then the normal default value will be used as the entry value.
//	 */
//	if (!has_value)
//	{
//		INISection* sectionptr = Find_Section(section);
//		if (InheritedSections.contains(sectionptr))
//		{
//			for (const std::string& inherited_section : InheritedSections[sectionptr])
//			{
//				int count = Get_String(inherited_section.c_str(), entry, "", buffer, size);
//				if (count > 0)
//				{
//					//DEBUG_INFO("Fetched [%s]->%s from %s\n", section, entry, inherited_section.c_str());
//					return count;
//				}
//			}
//		}
//	}
//
//	/**
//	 *  Fill in the buffer with the entry value and return with the length of the string.
//	 */
//	if (defvalue == nullptr)
//	{
//		buffer[0] = '\0';
//		return 0;
//	}
//	else
//	{
//		strncpy(buffer, defvalue, size);
//		buffer[size - 1] = '\0';
//		strtrim(buffer);
//		return strlen(buffer);
//	}
//}
//
//
///**
// *  Caches the inherited sections upon putting the $Inherits entry.
// *
// *  @author: ZivDero
// */
//DEFINE_HOOK(0x004DDD3A, _INIClass_this->WriteString_Cache_Inherits, 5)
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
//DEFINE_HOOK(0x004DED22, _INIClass_INISection_DTOR_Clear_Inherits, 5)
//{
//	GET(INIClass::INISection*, secptr, ECX);
//
//	InheritedSections.erase(secptr);
//
//	return 0;
//}
//
//
///**
// *  Various INI getter replacements.
// *
// *  @author: tomsons26, ZivDero
// */
//int INIClassExt::_Get_Int(char const* section, char const* entry, int defvalue) const
//{
//	if (section == nullptr || entry == nullptr) return defvalue;
//
//	std::string value = Get_String(section, entry, {});
//	if (!value.empty())
//	{
//		if (value[0] == '$')
//		{
//			sscanf(value.c_str(), "$%x", &defvalue);
//		}
//		else
//		{
//			if (tolower(value.back()) == 'h')
//			{
//				sscanf(value.c_str(), "%xh", &defvalue);
//			}
//			else
//			{
//				defvalue = atoi(value.c_str());
//			}
//		}
//	}
//	return defvalue;
//}
//
//bool INIClassExt::_Get_Bool(char const* section, char const* entry, bool defvalue) const
//{
//	if (section == nullptr || entry == nullptr) return defvalue;
//
//	std::string value = Get_String(section, entry, {});
//	if (!value.empty())
//	{
//		switch (toupper(value[0]))
//		{
//		case 'Y':
//		case 'T':
//		case '1':
//			return true;
//
//		case 'N':
//		case 'F':
//		case '0':
//			return false;
//		}
//	}
//	return defvalue;
//}
//
//double INIClassExt::_Get_Float(char const* section, char const* entry, double defvalue) const
//{
//	if (section == nullptr || entry == nullptr) return defvalue;
//
//	std::string value = Get_String(section, entry, {});
//	if (!value.empty())
//	{
//		float val;
//		sscanf(value.c_str(), "%f", &val);
//		defvalue = val;
//		if (value.find('%') != std::string::npos)
//		{
//			defvalue /= 100.0;
//		}
//	}
//	return defvalue;
//}
//
//TPoint2D<int> INIClassExt::_Get_Point(char const* section, char const* entry, TPoint2D<int> const& defvalue) const
//{
//	char buffer[64];
//	if (Get_String(section, entry, "", buffer, sizeof(buffer)))
//	{
//		int x, y;
//		std::sscanf(buffer, "%d,%d", &x, &y);
//		return { x, y };
//	}
//	return defvalue;
//}
//
//TPoint3D<int> INIClassExt::_Get_Point(char const* section, char const* entry, TPoint3D<int> const& defvalue) const
//{
//	char buffer[64];
//	if (Get_String(section, entry, "", buffer, sizeof(buffer)))
//	{
//		int x, y, z;
//		std::sscanf(buffer, "%d,%d,%d", &x, &y, &z);
//		return { x, y, z };
//	}
//	return defvalue;
//}
//
//TPoint3D<float> INIClassExt::_Get_Point(char const* section, char const* entry, TPoint3D<float> const& defvalue) const
//{
//	char buffer[64];
//	if (Get_String(section, entry, "", buffer, sizeof(buffer)))
//	{
//		float x, y, z;
//		std::sscanf(buffer, "%f,%f,%f", &x, &y, &z);
//		return { x, y, z };
//	}
//	return defvalue;
//}
//
//int INIClassExt::_Get_UUBlock(char const* section, void* block, int len) const
//{
//	if (section == nullptr) return 0;
//
//	Base64Pipe b64pipe(Base64Pipe::DECODE);
//	BufferPipe bpipe(block, len);
//
//	b64pipe.Put_To(&bpipe);
//
//	int total = 0;
//	int counter = Entry_Count(section);
//	for (int index = 0; index < counter; index++)
//	{
//		char buffer[128];
//
//		int length = Get_String(section, Get_Entry(section, index), "=", buffer, sizeof(buffer));
//		int outcount = b64pipe.Put(buffer, length);
//		total += outcount;
//	}
//	total += b64pipe.End();
//	return total;
//}
//
//int INIClassExt::_Get_TextBlock(char const* section, char* buffer, int len) const
//{
//	if (len <= 0) return 0;
//
//	buffer[0] = '\0';
//	if (len <= 1) return 0;
//
//	int elen = Entry_Count(section);
//	int total = 0;
//	for (int index = 0; index < elen; index++)
//	{
//		if (index > 0)
//		{
//			*buffer++ = ' ';
//			len--;
//			total++;
//		}
//
//		Get_String(section, Get_Entry(section, index), "", buffer, len);
//
//		int partial = std::strlen(buffer);
//		total += partial;
//		buffer += partial;
//		len -= partial;
//		if (len <= 1) break;
//	}
//	return total;
//}
//
//CLSID INIClassExt::_Get_UUID(char const* section, char const* entry, CLSID defvalue) const
//{
//	char buffer[128];
//
//	if (Get_String(section, entry, "", buffer, sizeof(buffer)))
//	{
//		wchar_t wBuffer[128];
//		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buffer, -1, wBuffer, std::size(wBuffer));
//		CLSID uuid;
//		if (SUCCEEDED(CLSIDFromString(wBuffer, &uuid)))
//		{
//			return uuid;
//		}
//	}
//	return defvalue;
//}
//
//Rect INIClassExt::_Get_Rect(char const* section, char const* entry, Rect const& defvalue) const
//{
//	char buffer[64];
//
//	if (Get_String(section, entry, "", buffer, sizeof(buffer)))
//	{
//		Rect retval = defvalue;
//		sscanf(buffer, "%d,%d,%d,%d", &retval.X, &retval.Y, &retval.Width, &retval.Height);
//		return retval;
//	}
//	return defvalue;
//}