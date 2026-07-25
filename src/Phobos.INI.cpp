#include "Phobos.INI.h"

#include <Straws.h>
#include <Pipes.h>

//TODO : test writing back ini 
//   - handle double to percent when writing back
#pragma region INISection

int PhobosINISection::ComputeCRC(std::string_view s)
{
	return SafeChecksummer()(s.data(), s.size());
}

PhobosINIEntry* PhobosINISection::FindEntry(std::string_view key)
{
	const auto it = this->EntryIndex.find(ComputeCRC(key));
	if (it == this->EntryIndex.end())
		return nullptr;

	return &this->Entries[it->second];
}

bool PhobosINISection::SetEntry(std::string_view key, std::string_view value)
{
	const int crc = ComputeCRC(key);
	const auto it = this->EntryIndex.find(crc);

	if (it != this->EntryIndex.end()) {
		this->Entries[it->second].Value = value;
		return false;
	}

	const size_t idx = this->Entries.size();
	this->Entries.emplace_back(key, value);
	this->EntryIndex.emplace(crc, idx);
	return true;
}

bool PhobosINISection::RemoveEntry(std::string_view key)
{
	const int crc = ComputeCRC(key);
	const auto it = this->EntryIndex.find(crc);

	if (it == this->EntryIndex.end())
		return false;

	const size_t idx = it->second;
	const size_t last = this->Entries.size() - 1;

	if (idx != last) {
		const int movedCrc = ComputeCRC(this->Entries[last].Key);
		this->EntryIndex[movedCrc] = idx;
		this->Entries[idx] = std::move(this->Entries[last]);
	}

	this->Entries.pop_back();
	this->EntryIndex.erase(it);
	return true;
}

const char* PhobosINISection::GetKeyName(int index)
{
	const std::string str = this->GetKeyNameA(index);
	return str.empty() ? nullptr : str.c_str();
}

std::string PhobosINISection::GetKeyNameA(int index)
{
	if (index < 0 || static_cast<size_t>(index) >= this->Entries.size())
		return {};

	return this->Entries[static_cast<size_t>(index)].Key.c_str();
}

#pragma endregion

#pragma region INIClass

int PhobosINIClass::IteratorValue;

void PhobosINIClass::ResetStatics()
{
	PhobosINIClass::IteratorValue = 0;
}

size_t PhobosINIClass::ReadLine(Straw* straw, char* buf, int cap, bool* eof)
{
	if (!cap || !buf)
		return 0;

	int  count = 0;
	char c;

	if (straw->Get(&c, 1) != 1) {
		*eof = true;
		buf[0] = '\0';
		return 0;
	}

	while (c != '\n') {
		if (c != '\r' && count < cap - 1)
			buf[count++] = c;

		if (straw->Get(&c, 1) != 1) {
			*eof = true;
			buf[count] = '\0';
			return static_cast<size_t>(count);
		}
	}

	buf[count] = '\0';
	return static_cast<size_t>(count);
}

bool PhobosINIClass::IsSectionHeader(std::string_view line)
{
	const auto b = line.find_first_not_of(" \t\r\n");
	if (b == std::string_view::npos)
		return false;

	line = line.substr(b);
	return line[0] == '[' && line.find(']') != std::string_view::npos;
}

std::string_view  PhobosINIClass::StripComment(std::string_view line) {
	for (auto tok : SplitView(line, ';')) {
		const auto e = tok.find_last_not_of(" \t\r\n");
		return (e == std::string_view::npos) ? std::string_view {} : tok.substr(0, e + 1);
	}

	return {};
}

std::string_view PhobosINIClass::TrimSW(std::string_view s) {
	const auto b = s.find_first_not_of(" \t");
	if (b == std::string_view::npos)
		return {};

	return s.substr(b, s.find_last_not_of(" \t") - b + 1);
}

std::pair<std::string_view, std::string_view> PhobosINIClass::ParseKeyValue(std::string_view line)
{
	const auto eq = line.find('=');
	if (eq == std::string_view::npos)
		return {};

	return { 
		PhobosINIClass::TrimSW(line.substr(0, eq)), 
		PhobosINIClass::TrimSW(line.substr(eq + 1))
	};
}

std::string_view PhobosINIClass::ParseSectionHeader(std::string_view line, std::string_view* outParent)
{
	const auto open = line.find('[');
	const auto close = line.find(']', open != std::string_view::npos ? open : 0);

	if (open == std::string_view::npos || close == std::string_view::npos)
		return {};

	const std::string_view name = 
		PhobosINIClass::TrimSW(line.substr(open + 1, close - open - 1));

	if (outParent) {
		*outParent = {};
		const auto tail = line.substr(close + 1);
		const auto colon = tail.find(':');

		if (colon != std::string_view::npos) {
			const auto pOpen = tail.find('[', colon);
			const auto pClose = (pOpen != std::string_view::npos)
				? tail.find(']', pOpen) : std::string_view::npos;

			if (pOpen != std::string_view::npos && pClose != std::string_view::npos)
				*outParent = PhobosINIClass::TrimSW(tail.substr(pOpen + 1, pClose - pOpen - 1));
		}
	}

	return name;
}

std::pair<std::string, std::string> PhobosINIClass::RewriteIteratorKey(std::string_view key, std::string_view val)
{
	if (key != "+")
		return {
		std::string(key),
		std::string(val)
	};

	return { 
		"var_" + std::to_string(IteratorValue++), 
		std::string(val) 
	};
}

// ---------------------------------------------------------------------------
// PhobosINIClass — private
// ---------------------------------------------------------------------------

int PhobosINIClass::CRC(std::string_view s)
{
	return SafeChecksummer()(s.data(), s.size());
}

// ---------------------------------------------------------------------------
// Section API
// ---------------------------------------------------------------------------

PhobosINISection* PhobosINIClass::GetSection(std::string_view name)
{
	if (!this->LastSectionName.empty() && this->LastSectionName == name)
		return this->LastSection;

	const auto it = this->SectionIndex.find(CRC(name));
	if (it == this->SectionIndex.end())
		return nullptr;

	PhobosINISection* sec = &this->SectionList[it->second];
	this->LastSectionName = name;
	this->LastSection = sec;
	return sec;
}

PhobosINISection& PhobosINIClass::GetOrCreateSection(std::string_view name)
{
	const int crc = CRC(name);
	const auto it = this->SectionIndex.find(crc);

	if (it != this->SectionIndex.end())
		return this->SectionList[it->second];

	const size_t idx = SectionList.size();
	this->SectionList.emplace_back(name);
	this->SectionIndex.emplace(crc, idx);

	// Invalidate cache: emplace_back may have reallocated SectionList,
	// making the old LastSection pointer dangle.
	this->InvalidateCache();

	return this->SectionList[idx];
}

bool PhobosINIClass::RemoveSection(std::string_view name)
{
	const int crc = CRC(name);
	const auto it = this->SectionIndex.find(crc);

	if (it == this->SectionIndex.end())
		return false;

	const size_t idx = it->second;
	const size_t last = this->SectionList.size() - 1;

	if (idx != last) {
		const int movedCrc = CRC(this->SectionList[last].Name);
		this->SectionIndex[movedCrc] = idx;
		this->SectionList[idx] = std::move(this->SectionList[last]);
	}

	this->SectionList.pop_back();
	this->SectionIndex.erase(it);
	this->InvalidateCache();
	return true;
}

void PhobosINIClass::Clear()
{
	this->SectionList.clear();
	this->SectionIndex.clear();
	this->LoadedIncludes.clear();
	this->InvalidateCache();
}

// ---------------------------------------------------------------------------
// Key/value API
// ---------------------------------------------------------------------------

std::string PhobosINIClass::ReadString(std::string_view section, std::string_view key, const char* pDefault)
{
	if (PhobosINISection* sec = this->GetSection(section)) {
		if (const PhobosINIEntry* entry = sec->FindEntry(key)) {
			return entry->Value;
		}
	}
	
	if (pDefault)
		return pDefault;


	return {};
}

bool PhobosINIClass::WriteString(std::string_view section, std::string_view key, std::string_view value)
{
	this->GetOrCreateSection(section).SetEntry(key, value);
	return true;
}

int PhobosINIClass::GetKeyCount(std::string_view section)
{
	if (PhobosINISection* sec = this->GetSection(section)) {
		return sec->GetKeyCount();
	}

	return 0;
}

const char* PhobosINIClass::GetKeyName(std::string_view section, int index)
{
	const std::string ret = this->GetKeyNameA(section, index);

	return ret.empty() ? nullptr : ret.c_str();
}

std::string PhobosINIClass::GetKeyNameA(std::string_view section, int index)
{
	if (PhobosINISection* sec = this->GetSection(section))
		return sec->GetKeyNameA(index);

	return {};
}

bool PhobosINIClass::KeyPresent(std::string_view section, std::string_view key)
{
	if (PhobosINISection* sec = GetSection(section)) {
		if (sec->FindEntry(key)) {
			return true;
		}
	}

	return false;
}

void PhobosINIClass::CopyInheritedSection(
	const PhobosINISection& parent, std::string_view destName)
{
	for (const auto& entry : parent.Entries) {
		// SetEntry returns false if key already exists — child wins, no overwrite.
		PhobosINISection& dest = GetOrCreateSection(destName);
		if (!dest.FindEntry(entry.Key))
			dest.SetEntry(entry.Key, entry.Value);
	}
}

void PhobosINIClass::ProcessIncludes(bool useNewIncludes)
{
	// Section name priority mirrors original hook:
	//   useNewIncludes=true  → "$include" / "$Include"
	//   useNewIncludes=false → "#include" / "#Include"  (Ares compat)
	static constexpr const char* NewPrimary = "$include";
	static constexpr const char* NewFallback = "$Include";
	static constexpr const char* AresPrimary = "#include";
	static constexpr const char* AresFallback = "#Include";

	const char* primary = useNewIncludes ? NewPrimary : AresPrimary;
	const char* fallback = useNewIncludes ? NewFallback : AresFallback;

	PhobosINISection* sec = this->GetSection(primary);
	if (!sec)
		sec = this->GetSection(fallback);

	if (!sec)
		return;

	for (const auto& entry : sec->Entries)
	{
		if (entry.Value.empty())
			continue;

		const std::string& filename = entry.Value;

		if (this->LoadedIncludes.contains(filename))
			continue;

		this->LoadedIncludes.insert(filename);

		CCFileClass nFile { filename.c_str() };

		if (!nFile.IsAvaible())
		{
			Debug::Log("PhobosINI: included file '%s' does not exist\n", filename.c_str());
			continue;
		}

		if (Phobos::Otamaa::IsAdmin)
			Debug::Log("PhobosINI: reading included file '%s'\n", filename.c_str());

		// Recurse: LoadFile merges the included INI into this instance.
		// This will itself trigger ProcessIncludes if the sub-file has $include.
		this->LoadFile(&nFile);
	}
}

bool PhobosINIClass::LoadFile(RawFileClass* file)
{
	if (!file || !file->IsAvaible())
		return false;

	file->Open1(FileAccessMode::Read);
	FileStraw fs (file);

	const bool result = Load(&fs, false);

	file->Close();
	return result;
}

bool PhobosINIClass::Load(Straw* straw, bool loadcomments)
{
	InvalidateCache();

	// Asm @ 0x525A74: merge = (SectionIndex.IndexCount > 0)
	const bool merge = !SectionList.empty();

	CacheStraw file;
	file.Get_From(straw);

	bool eof = false;
	char buffer[512];
	buffer[0] = '\0';

	std::string currentSection; // merge path only

	// -----------------------------------------------------------------------
	// Pass 1: consume pre-section lines until first [header] or EOF.
	// Asm @ 0x525AE8 loop.
	// -----------------------------------------------------------------------
	for (;;) {
		ReadLine(&file, buffer, sizeof(buffer), &eof);

		if (eof)
		{
			// Fresh load, nothing found — clear and return false.
			// Merge load, EOF before first header — data already present, true.
			if (!merge)
				Clear();

			return merge;
		}

		if (IsSectionHeader(buffer))
			break;
	}

	// -----------------------------------------------------------------------
	// Pass 2: merge path.
	// Asm @ 0x525BE2, inner loop @ 0x525C5C.
	// `buffer` holds the first [section] header on entry.
	// -----------------------------------------------------------------------
	if (merge) {
		while (!eof) {
			currentSection.clear();

			if (IsSectionHeader(buffer)) {
				std::string_view parentName;
				const auto name = ParseSectionHeader(buffer, &parentName);
				currentSection = name;

				// Phobos: [CHILD]:[PARENT] inheritance — copy parent before parsing entries.
				if (!parentName.empty())
				{
					const PhobosINISection* parent = GetSection(parentName);
					if (parent)
						CopyInheritedSection(*parent, currentSection);
					else
						Debug::LogError("PhobosINI: [{}] inherits from '{}' which doesn't exist yet.", currentSection, parentName);
				}
			}

			bool nextHeader = false;
			while (!nextHeader && !eof) {
				const size_t readLen =
					ReadLine(&file, buffer, sizeof(buffer), &eof);

				if (IsSectionHeader(buffer)) {
					nextHeader = true;
					break;
				}

				const std::string_view line = StripComment(buffer);

				if (!readLen || line.empty() || line[0] == ';' || line[0] == '=')
					continue;

				auto [key, val] = ParseKeyValue(line);

				if (key.empty() || val.empty())
					continue;

				auto [finalKey, finalVal] = RewriteIteratorKey(key, val);

				// Asm @ 0x525D3A: Put_String failure → Clear + return false.
				if (!WriteString(currentSection, finalKey, finalVal)) {
					Clear();
					return false;
				}
			}
		}

		// Process $include after merge pass completes.
		ProcessIncludes(Phobos::Config::UseNewIncludes);
		return true;
	}

	// -----------------------------------------------------------------------
	// Pass 3: fresh path.
	// Asm @ 0x525D88 (free LineComments, no-op here), then @ 0x525DB8 main loop.
	// -----------------------------------------------------------------------
	if (eof)
		return true;

	for (;;) {
		std::string_view parentName;
		std::string_view sectionName;

		if (IsSectionHeader(buffer)) {
			sectionName = ParseSectionHeader(buffer, &parentName);
		} else {
			// Asm @ 0x525DE0: buffer[0]=0; buffer[1]=0 when no '[' found.
			// SUSPECT: vanilla creates unnamed section here; preserved verbatim.
			buffer[0] = '\0';
			buffer[1] = '\0';
			sectionName = {};
		}

		// Ares: remove stale duplicate before inserting.
		// Mirrors Remove_Duplicate_Section_If_Present.
		RemoveSection(sectionName);
		PhobosINISection& sec = GetOrCreateSection(sectionName);

		// Phobos: [CHILD]:[PARENT] — copy parent entries before parsing own entries.
		// Child keys parsed below will NOT be overwritten by the copy (child wins).
		if (!parentName.empty()) {
			const PhobosINISection* parent = GetSection(parentName);
			if (parent)
				CopyInheritedSection(*parent, sectionName);
			else
				Debug::LogError("PhobosINI: [{}] inherits from '{}' which doesn't exist yet.", sectionName, parentName);
		}

		// Entry loop. Asm @ 0x525E5B.
		while (!eof) {
			const size_t readLen =
				ReadLine(&file, buffer, sizeof(buffer), &eof);

			if (eof)
				break;

			if (IsSectionHeader(buffer))
				break;

			// Asm @ 0x525F69: inlined Strip_Comments.
			const std::string_view line = StripComment(buffer);

			// Asm @ 0x525F90: skip empty / ';' / '=' leading lines.
			if (!readLen || line.empty() || line[0] == ';' || line[0] == '=')
				continue;

			auto [key, val] = ParseKeyValue(line);

			if (key.empty() || val.empty())
				continue;

			auto [finalKey, finalVal] = RewriteIteratorKey(key, val);

			// SetEntry: child entries go in after inherited ones — no overwrite needed
			// because CopyInheritedSection only writes keys absent from dest.
			sec.SetEntry(finalKey, finalVal);
		}

		// Asm @ 0x5261F7: discard empty sections.
		if (sec.Entries.empty())
			RemoveSection(sectionName);

		if (eof) {
			// Process $include after fresh pass completes.
			ProcessIncludes(Phobos::Config::UseNewIncludes);
			return true;
		}

		// buffer already holds next [section] header — outer loop continues.
	}
}

int PhobosINIClass::SaveToPipe(Pipe* pipe)
{
	if (!pipe)
		return 0;

	int total = 0;

	for (size_t si = 0; si < SectionList.size(); ++si)
	{
		PhobosINISection& sec = SectionList[si];

		// Blank line between sections (vanilla writes one when no leading comment).
		if (si > 0)
			total += pipe->Put("\r\n", 2);

		// "[SectionName]\r\n"
		total += pipe->Put("[", 1);
		total += pipe->Put(sec.Name.c_str(), static_cast<int>(sec.Name.size()));
		total += pipe->Put("]\r\n", 3);

		for (const auto& entry : sec.Entries)
		{
			// "Key=Value\r\n"
			total += pipe->Put(entry.Key.c_str(), static_cast<int>(entry.Key.size()));
			total += pipe->Put("=", 1);
			total += pipe->Put(entry.Value.c_str(), static_cast<int>(entry.Value.size()));
			total += pipe->Put("\r\n", 2);
		}
	}

	return total + pipe->End();
}

bool PhobosINIClass::SaveToFile(FileClass* file)
{
	if (!file)
		return false;

	file->Open1(FileAccessMode::Write);
	FilePipe fp;
	const int written = SaveToPipe(&fp);
	return written > 0;
}

std::string PhobosINIClass::GetTextBlock(std::string_view section)
{
	PhobosINISection* sec = this->GetSection(section);
	if (!sec || sec->Entries.empty())
		return {};

	std::string result;
	result.reserve(sec->Entries.size() * 16); // reasonable initial capacity

	for (const auto& entry : sec->Entries) {

		// Vanilla: looks up entry key name by index, then looks up that
		// key's value in the same section. In the sidecar both are already
		// stored together — entry.Key is the key name, entry.Value is the value.
		if (entry.Value.empty())
			continue;

		if (!result.empty())
			result += ' '; // vanilla: inserts a space before each subsequent value

		result += entry.Value;
	}

	return result;
}

int PhobosINIClass::GetTextBlock(std::string_view section, char* buffer, int len)
{
	if (!buffer || len <= 0)
		return 0;

	*buffer = '\0';

	if (len <= 1)
		return 0;

	const std::string text = GetTextBlock(section);

	if (text.empty())
		return 0;

	CRT::strncpy(buffer, text.c_str(), static_cast<size_t>(len));
	buffer[len - 1] = '\0';
	CRT::strtrim(buffer);

	return static_cast<int>(std::strlen(buffer));
}

bool PhobosINIClass::PutTextBlock(std::string_view section, std::string_view text)
{
	if (section.empty())
		return false;

	// Vanilla: Clear(section, 0) — wipe section before writing numbered keys.
	RemoveSection(section);

	static constexpr size_t ChunkMax = 75;

	int         line = 1;
	size_t      pos = 0;

	while (pos < text.size())
	{
		// Skip leading whitespace between chunks.
		while (pos < text.size() && static_cast<unsigned char>(text[pos]) <= ' ')
			++pos;

		if (pos >= text.size())
			break;

		std::string_view remaining = text.substr(pos);

		// Take up to ChunkMax characters.
		std::string_view chunk = remaining.substr(0, std::min(remaining.size(), ChunkMax));

		if (chunk.size() >= ChunkMax && remaining.size() > ChunkMax)
		{
			// Word-wrap: walk back from ChunkMax to find last whitespace.
			// Vanilla: if count reaches 0 with no whitespace found, return 1.
			size_t boundary = chunk.size();
			while (boundary > 0)
			{
				--boundary;
				const unsigned char c = static_cast<unsigned char>(chunk[boundary]);
				if (!c || c <= ' ')
					break;
			}

			// SUSPECT: vanilla returns 1 (success) mid-loop if no word boundary
			// found — matches original early-exit on unbreakable 75-char run.
			if (boundary == 0)
				return true;

			chunk = chunk.substr(0, boundary);
		}

		// Trim trailing whitespace from chunk.
		const auto trimEnd = chunk.find_last_not_of(" \t\r\n");
		if (trimEnd != std::string_view::npos)
			chunk = chunk.substr(0, trimEnd + 1);

		if (!chunk.empty())
		{
			WriteString(section, std::to_string(line), chunk);
			++line;
		}

		pos += chunk.size();
	}

	return true;
}

// ---------------------------------------------------------------------------
// PhobosINIContainer
// ---------------------------------------------------------------------------
std::unique_ptr<PhobosINIClass> PhobosINIContainer::Rules_INI;
std::unique_ptr<PhobosINIClass> PhobosINIContainer::Art_INI;
std::unique_ptr<PhobosINIClass> PhobosINIContainer::Ai_INI;
std::unique_ptr<PhobosINIClass> PhobosINIContainer::Ui_INI;
std::unique_ptr<PhobosINIClass> PhobosINIContainer::Ra2_INI;
std::unique_ptr<PhobosINIClass> PhobosINIContainer::Mission_INI;
std::unique_ptr<PhobosINIClass> PhobosINIContainer::Movie_INI;

PhobosINIContainer& PhobosINIContainer::Instance()
{
	static PhobosINIContainer s_instance;
	return s_instance;
}

PhobosINIClass& PhobosINIContainer::GetOrCreate(const void* pINI)
{
	return this->Map[pINI];
}

PhobosINIClass* PhobosINIContainer::Find(const void* pINI)
{
	const auto it = this->Map.find(pINI);
	if (it == this->Map.end())
		return nullptr;

	return &it->second;
}

void PhobosINIContainer::Remove(const void* pINI)
{
	this->Map.erase(pINI);
}

void PhobosINIContainer::Clear()
{
	this->Map.clear();
}

#pragma endregion