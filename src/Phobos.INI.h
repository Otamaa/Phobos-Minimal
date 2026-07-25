#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <memory>
#include <optional>

#include <CRC.h>
#include <Utilities/Debug.h>

struct PhobosPercent { double Value; };
struct PhobosRate { int Frames; };

/**
 *  Convenient generator-like class for splitting and trimming strings.
 */

enum class StringSplitOptions : unsigned {
	None = 0,
	RemoveEmpty = 1 << 0,
	Trim = 1 << 1,
};

inline StringSplitOptions operator|(StringSplitOptions a, StringSplitOptions b) {
	return static_cast<StringSplitOptions>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

inline bool HasFlag(StringSplitOptions v, StringSplitOptions f) {
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

// ---------------------------------------------------------------------------
// PhobosParser<T, N>
//
// Count-aware typed parser. Replaces Parser<T,N> from Ares/Phobos.
// - N == 1  : single value, TryParse writes directly to *out.
// - N  > 1  : splits on ',', parses up to N tokens into out[N].
// - Parse()    : stops on first failure, returns count parsed.
// - TryParse() : all-or-nothing; out[] unchanged on partial failure.
//
// Specialize PhobosParser<T,1>::From() to add new types.
// All multi-value variants (N>1) are driven automatically from that.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Single-value specialization base (N==1)
// ---------------------------------------------------------------------------
template<typename T, size_t N = 1>
struct PhobosParser
{
	// Parse up to N comma-separated tokens from `value` into out[0..N-1].
	// Returns number of elements successfully parsed.
	// Stops at first failure — does not skip.
	static size_t Parse(std::string_view value, T* out)
	{
		size_t count = 0;

		for (auto token : SplitView(value, ',', StringSplitOptions::Trim))
		{
			if (count >= N)
				break;

			auto parsed = PhobosParser<T, 1>::From(token);
			if (!parsed.has_value())
				break;

			out[count++] = std::move(*parsed);
		}

		return count;
	}

	// All-or-nothing: writes to out[] only if all N tokens parse successfully.
	// Returns true on full success, false on any failure (out[] unchanged).
	static bool TryParse(std::string_view value, T* out)
	{
		T temp[N] = {};
		if (Parse(value, temp) != N)
			return false;

		for (size_t i = 0; i < N; ++i)
			out[i] = std::move(temp[i]);

		return true;
	}
};

// ---------------------------------------------------------------------------
// N==1 specialization: adds From() which all N>1 variants call.
// ---------------------------------------------------------------------------
template<typename T>
struct PhobosParser<T, 1>
{
	// Implement From() in explicit specializations below.
	// Returns nullopt on parse failure.
	static std::optional<T> From(std::string_view token);

	static size_t Parse(std::string_view value, T* out)
	{
		auto parsed = From(value);
		if (!parsed.has_value())
			return 0;

		if (out)
			*out = std::move(*parsed);

		return 1;
	}

	static bool TryParse(std::string_view value, T* out)
	{
		return Parse(value, out) == 1;
	}
};

// ---------------------------------------------------------------------------
// From() specializations
// ---------------------------------------------------------------------------

template<>
inline std::optional<std::string> PhobosParser<std::string, 1>::From(std::string_view token)
{
	return std::string(token);
}

template<> inline std::optional<bool> PhobosParser<bool, 1>::From(std::string_view token)
{
	if (token == "true" || token == "yes" || token == "1") return true;
	if (token == "false" || token == "no" || token == "0") return false;
	return std::nullopt;
}

template<> inline std::optional<int> PhobosParser<int, 1>::From(std::string_view token)
{
	int value = 0;
	const auto [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), value);
	if (ec != std::errc {} || ptr != token.data() + token.size())
		return std::nullopt;
	return value;
}

template<> inline std::optional<unsigned int> PhobosParser<unsigned int, 1>::From(std::string_view token)
{
	unsigned int value = 0;
	const auto [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), value);
	if (ec != std::errc {} || ptr != token.data() + token.size())
		return std::nullopt;
	return value;
}

template<> inline std::optional<short> PhobosParser<short, 1>::From(std::string_view token)
{
	const auto v = PhobosParser<int, 1>::From(token);
	if (!v || *v < std::numeric_limits<short>::min() || *v > std::numeric_limits<short>::max())
		return std::nullopt;
	return static_cast<short>(*v);
}

template<> inline std::optional<unsigned char> PhobosParser<unsigned char, 1>::From(std::string_view token)
{
	const auto v = PhobosParser<unsigned int, 1>::From(token);
	if (!v || *v > std::numeric_limits<unsigned char>::max())
		return std::nullopt;
	return static_cast<unsigned char>(*v);
}

template<> inline std::optional<float> PhobosParser<float, 1>::From(std::string_view token)
{
	// VERIFY: from_chars<float> requires MSVC 19.14+ / GCC 11+.
	const char* first = token.data();
	const char* last  = token.data() + token.size();
 
	// from_chars does NOT skip leading whitespace and does NOT accept a leading '+',
	// unlike vanilla sscanf("%f"). Normalise both so behaviour matches.
	while (first != last && (*first == ' ' || *first == '\t'))
		++first;
	if (first != last && *first == '+')
		++first;
 
	float value = 0.f;
	const auto [ptr, ec] = std::from_chars(first, last, value);
	if (ec != std::errc {})
		return std::nullopt;
 
	// Skip whitespace between the number and any trailing token.
	const char* rest = ptr;
	while (rest != last && (*rest == ' ' || *rest == '\t'))
		++rest;
 
	// EXTENSION: a trailing '%' means "percent" -> scale by 1/100.
	// VERIFY: Ares' Parser<float> only divides when '%' *immediately* follows the
	// number; vanilla INIClass divides if '%' appears *anywhere* via strchr. This
	// accepts a single '%' either immediately after or whitespace-separated.
	// Tighten to `*ptr == '%'` if strict Ares parity is required.
	if (rest != last && *rest == '%')
	{
		value *= 0.01f;
		++rest;
		while (rest != last && (*rest == ' ' || *rest == '\t'))
			++rest;
	}
 
	// Reject leftover garbage (e.g. "50abc", "5%%").
	if (rest != last)
		return std::nullopt;
 
	return value;
}

template<> inline std::optional<double> PhobosParser<double, 1>::From(std::string_view token)
{
	// VERIFY: from_chars<double> requires MSVC 19.14+ / GCC 11+.
	const char* first = token.data();
	const char* last  = token.data() + token.size();
 
	// from_chars does NOT skip leading whitespace and does NOT accept a leading '+',
	// unlike vanilla sscanf("%lf"). Normalise both so behaviour matches.
	while (first != last && (*first == ' ' || *first == '\t'))
		++first;
	if (first != last && *first == '+')
		++first;
 
	double value = 0.0;
	const auto [ptr, ec] = std::from_chars(first, last, value);
	if (ec != std::errc {})
		return std::nullopt;
 
	// Skip whitespace between the number and any trailing token.
	const char* rest = ptr;
	while (rest != last && (*rest == ' ' || *rest == '\t'))
		++rest;
 
	// EXTENSION: a trailing '%' means "percent" -> scale by 1/100.
	// VERIFY: Ares' Parser<double> only divides when '%' *immediately* follows the
	// number; vanilla INIClass divides if '%' appears *anywhere* via strchr. This
	// accepts a single '%' either immediately after or whitespace-separated.
	// Tighten to `*ptr == '%'` if strict Ares parity is required.
	if (rest != last && *rest == '%')
	{
		value *= 0.01;
		++rest;
		while (rest != last && (*rest == ' ' || *rest == '\t'))
			++rest;
	}
 
	// Reject leftover garbage (e.g. "50abc", "5%%").
	if (rest != last)
		return std::nullopt;
 
	return value;
}

template<>
struct PhobosParser<PhobosPercent, 1>
{
	static std::optional<PhobosPercent> From(std::string_view token)
	{
		bool isPercent = false;
		if (!token.empty() && token.back() == '%')
		{
			token = token.substr(0, token.size() - 1);
			isPercent = true;
		}

		const auto v = PhobosParser<double, 1>::From(token);
		if (!v.has_value())
			return std::nullopt;

		return PhobosPercent { isPercent ? (*v / 100.0) : *v };
	}

	static size_t Parse(std::string_view value, PhobosPercent* out)
	{
		auto parsed = From(value);
		if (!parsed.has_value())
			return 0;
		if (out)
			*out = *parsed;
		return 1;
	}

	static bool TryParse(std::string_view value, PhobosPercent* out)
	{
		return Parse(value, out) == 1;
	}
};

template<>
struct PhobosParser<PhobosRate, 1>
{
	static std::optional<PhobosRate> From(std::string_view token)
	{
		const auto v = PhobosParser<double, 1>::From(token);
		if (!v.has_value())
			return std::nullopt;
		return PhobosRate { static_cast<int>(*v * 900.0) };
	}

	static size_t Parse(std::string_view value, PhobosRate* out)
	{
		auto parsed = From(value);
		if (!parsed.has_value())
			return 0;
		if (out)
			*out = *parsed;
		return 1;
	}

	static bool TryParse(std::string_view value, PhobosRate* out)
	{
		return Parse(value, out) == 1;
	}
};

template<typename T>
struct PhobosFormatter; // Intentionally undefined — linker error on unsupported T.

template<>
struct PhobosFormatter<std::string>
{
	static std::string ToString(const std::string& v) { return v; }
	static std::string ToString(std::string_view v) { return std::string(v); }
};

template<>
struct PhobosFormatter<const char*>
{
	static std::string ToString(const char* v) { return v ? v : ""; }
};

template<>
struct PhobosFormatter<bool>
{
	static std::string ToString(bool v) { return v ? "yes" : "no"; }
};

template<>
struct PhobosFormatter<int>
{
	static std::string ToString(int v, bool hex = false)
	{
		char buf[32];
		if (hex)
			sprintf_s(buf, "0x%X", v);
		else
			sprintf_s(buf, "%d", v);
		return buf;
	}
	// Default non-hex overload so it fits the uniform ToString(v) contract.
	static std::string ToString(int v) { return ToString(v, false); }
};

template<>
struct PhobosFormatter<unsigned int>
{
	static std::string ToString(unsigned int v)
	{
		char buf[32];
		sprintf_s(buf, "%u", v);
		return buf;
	}
};

template<>
struct PhobosFormatter<short>
{
	static std::string ToString(short v)
	{
		char buf[16];
		sprintf_s(buf, "%d", static_cast<int>(v));
		return buf;
	}
};

template<>
struct PhobosFormatter<unsigned char> // BYTE
{
	static std::string ToString(unsigned char v)
	{
		char buf[8];
		sprintf_s(buf, "%u", static_cast<unsigned int>(v));
		return buf;
	}
};

template<>
struct PhobosFormatter<float>
{
	static std::string ToString(float v)
	{
		char buf[64];
		sprintf_s(buf, "%g", static_cast<double>(v));
		return buf;
	}
};

template<>
struct PhobosFormatter<double>
{
	static std::string ToString(double v)
	{
		char buf[64];
		sprintf_s(buf, "%g", v);
		return buf;
	}
};

// Rate: int frames -> decimal fraction of 900 (e.g. 450 -> "0.5")
template<>
struct PhobosFormatter<PhobosRate>
{
	static std::string ToString(const PhobosRate& v)
	{
		char buf[64];
		sprintf_s(buf, "%g", v.Frames / 900.0);
		return buf;
	}
};

// Percent: double 0..1 -> "50%" style string (e.g. 0.5 -> "50%")
template<>
struct PhobosFormatter<PhobosPercent>
{
	static std::string ToString(const PhobosPercent& v)
	{
		char buf[64];
		sprintf_s(buf, "%g%%", v.Value * 100.0);
		return buf;
	}

	static std::string ToStringRaw(const PhobosPercent& v)
	{
		char buf[64];
		sprintf_s(buf, "%g", v.Value);
		return buf;
	}
};

// ---------------------------------------------------------------------------
// PhobosFormatArray<T, N>
// Serializes T[N] -> "v0,v1,...,vN-1" (comma-joined).
// Mirrors PhobosParser<T,N>::TryParse in the write direction.
// ---------------------------------------------------------------------------
template<typename T, size_t N>
struct PhobosFormatArray
{
	static std::string ToString(const T* values, char delim = ',')
	{
		std::string result;
		for (size_t i = 0; i < N; ++i)
		{
			if (i > 0)
				result += delim;
			result += PhobosFormatter<T>::ToString(values[i]);
		}
		return result;
	}
};

struct PhobosINIEntry
{
	std::string Key;
	std::string Value;

	PhobosINIEntry() = default;
	~PhobosINIEntry() = default;

	PhobosINIEntry(std::string_view key, std::string_view value)
		: Key(key), Value(value)
	{}

	PhobosINIEntry(const PhobosINIEntry&) = delete;
	PhobosINIEntry& operator=(const PhobosINIEntry&) = delete;

	PhobosINIEntry(PhobosINIEntry&&) noexcept = default;
	PhobosINIEntry& operator=(PhobosINIEntry&&) noexcept = default;
};

class Pipe;
class Straw;
class RawFileClass;
class PhobosINISection
{
public:
	std::string Name;

	// Insertion-ordered entries — mirrors vanilla Entries linked list.
	// Do NOT sort: GetKeyName(index) depends on insertion order.
	std::vector<PhobosINIEntry> Entries;

	// CRC(key) -> index into Entries.
	std::unordered_map<int, size_t> EntryIndex;

	PhobosINISection() = default;
	explicit PhobosINISection(std::string_view name) : Name(name) {}

	PhobosINISection(const PhobosINISection&) = delete;
	PhobosINISection& operator=(const PhobosINISection&) = delete;

	PhobosINISection(PhobosINISection&&) noexcept = default;
	PhobosINISection& operator=(PhobosINISection&&) noexcept = default;

	// Returns nullptr if not found.
	PhobosINIEntry* FindEntry(std::string_view key);

	// Adds or overwrites. Returns true if key was new.
	bool SetEntry(std::string_view key, std::string_view value);

	// Removes key. Returns true if it existed.
	// NOTE: uses swap-erase — changes insertion order of last element.
	//       Acceptable for INI use where removal is rare.
	bool RemoveEntry(std::string_view key);

	// Returns key name at insertion-order index, nullptr if out of range.
	const char* GetKeyName(int index);
	std::string GetKeyNameA(int index);

	int GetKeyCount() const { return static_cast<int>(Entries.size()); }

private:
	static int ComputeCRC(std::string_view s);
};

class PhobosINIClass
{
public:
	static constexpr size_t BufferSize = 512;

	static int IteratorValue;

	static void ResetStatics();
	static size_t ReadLine(Straw* straw, char* buf, int cap, bool* eof);
	static bool IsSectionHeader(std::string_view line);
	static std::string_view StripComment(std::string_view line);
	static std::pair<std::string_view, std::string_view> ParseKeyValue(std::string_view line);
	static std::string_view ParseSectionHeader(std::string_view line, std::string_view* outParent = nullptr);
	static std::string_view TrimSW(std::string_view s);
	static std::pair<std::string, std::string> RewriteIteratorKey(std::string_view key, std::string_view val);

public:

	// Insertion-ordered sections — mirrors vanilla Sections linked list.
	std::vector<PhobosINISection>       SectionList;

	// CRC(sectionName) -> index into SectionList.
	std::unordered_map<int, size_t>     SectionIndex;

	// One-entry section cache — mirrors CurrentSectionName / CurrentSection.
	std::string                         LastSectionName;
	PhobosINISection*                   LastSection;

	// Tracks filenames already loaded via $include / #include in this instance.
	// Replaces INIInheritance::SavedIncludes static + LastINIFile guard.
	std::set<std::string>               LoadedIncludes;

	PhobosINIClass() = default;

	PhobosINIClass(const PhobosINIClass&) = delete;
	PhobosINIClass& operator=(const PhobosINIClass&) = delete;

	PhobosINIClass(PhobosINIClass&&) noexcept = default;
	PhobosINIClass& operator=(PhobosINIClass&&) noexcept = default;

	// ------------------------------------------------------------------------
	template<typename Func, typename FuncPre>
	void ParseSection(Func&& action , FuncPre&& preaction, std::string_view section) {
		if (auto pSection = this->GetSection(section)) {
			const int keyCount = pSection->GetKeyCount();

			preaction(this , section , keyCount);

			for (int i = 0; i < keyCount; ++i) {
				action(this, section, pSection->GetKeyName(i));
			}
		}
	}
	// ---- Section API -------------------------------------------------------

	PhobosINISection* GetSection(std::string_view name);

	// Returns existing section or creates a new one.
	PhobosINISection& GetOrCreateSection(std::string_view name);

	// Removes a section entirely. Returns true if it existed.
	bool RemoveSection(std::string_view name);

	void Clear();

	// ---- Key/value API -----------------------------------------------------

	// Returns stored value or pDefault if section/key absent.
	std::string ReadString(std::string_view section, std::string_view key, const char* pDefault);

	// Creates section if needed. Always returns true (throws on OOM).
	bool WriteString(std::string_view section, std::string_view key, std::string_view value);

	int         GetKeyCount(std::string_view section);
	const char* GetKeyName(std::string_view section, int index);
	std::string GetKeyNameA(std::string_view section, int index);

	bool SectionPresent(std::string_view section) {
		return this->GetSection(section) != nullptr; 
	}

	bool KeyPresent(std::string_view section, std::string_view key);

	bool Load(Straw* straw, bool loadcomments = false);
	bool LoadFile(RawFileClass* file);

	int  SaveToPipe(Pipe* pipe);
	bool SaveToFile(FileClass* file);

	std::string GetTextBlock(std::string_view section);
	int         GetTextBlock(std::string_view section, char* buffer, int len);
	bool        PutTextBlock(std::string_view section, std::string_view text);

	// -----------------------------------------------------------------------
   // Single-value typed read
   // -----------------------------------------------------------------------

   // Returns parsed value or nullopt. Logs on parse failure.
	template<typename T>
	std::optional<T> Read(std::string_view section, std::string_view key)
	{
		if (PhobosINISection* sec = this->GetSection(section)) {
			if (PhobosINIEntry* entry = sec->FindEntry(key)) {
				auto result = PhobosParser<T, 1>::From(entry->Value);

				if (!result) {

					Debug::Log(
						"PhobosINI: [%.*s] %.*s — cannot parse '%s'.\n",
						static_cast<int>(section.size()), section.data(),
						static_cast<int>(key.size()), key.data(),
						entry->Value.c_str());
				}

				return result;
			}
		}

		return std::nullopt;
	}

	// Returns parsed value or defaultValue on absence / parse failure.
	template<typename T>
	T ReadOrDefault(std::string_view section, std::string_view key, T defaultValue) {
		return Read<T>(section, key).value_or(std::move(defaultValue));
	}

	// Writes parsed result into *out. Returns true on success.
	template<typename T>
	bool ReadInto(std::string_view section, std::string_view key, T* out)
	{
		auto result = Read<T>(section, key);

		if (result.has_value() && out) {
			*out = std::move(*result);
			return true;
		}

		return false;
	}

	// -----------------------------------------------------------------------
	// Fixed-count array read
	// -----------------------------------------------------------------------

	// Parses up to N comma-separated tokens into out[0..N-1].
	// Stops at first failure. Returns count actually parsed.
	// Use when partial results are acceptable.
	template<typename T, size_t N>
	size_t ReadPartial(std::string_view section, std::string_view key, T* out)
	{
		if (const PhobosINISection* sec = GetSection(section)) {
			if (const PhobosINIEntry* entry = sec->FindEntry(key)) {
				return PhobosParser<T, N>::Parse(entry->Value, out);
			}
		}

		return 0;
	}

	// All-or-nothing: writes to out[N] only if all N tokens parse successfully.
	// out[] is completely unchanged on partial failure.
	// Use when you need exactly N values or nothing.
	template<typename T, size_t N>
	bool TryRead(std::string_view section, std::string_view key, T* out)
	{
		if (PhobosINISection* sec = GetSection(section)) {
			if (PhobosINIEntry* entry = sec->FindEntry(key)) {
				return PhobosParser<T, N>::TryParse(entry->Value, out);
			}
		}

		return false;
	}

	// Convenience wrappers for common fixed-count cases.
	template<typename T> bool Read2(std::string_view s, std::string_view k, T* out) { return TryRead<T, 2>(s, k, out); }
	template<typename T> bool Read3(std::string_view s, std::string_view k, T* out) { return TryRead<T, 3>(s, k, out); }
	template<typename T> bool Read4(std::string_view s, std::string_view k, T* out) { return TryRead<T, 4>(s, k, out); }

	template<typename T> size_t Read2Partial(std::string_view s, std::string_view k, T* out) { return ReadPartial<T, 2>(s, k, out); }
	template<typename T> size_t Read3Partial(std::string_view s, std::string_view k, T* out) { return ReadPartial<T, 3>(s, k, out); }

	// -----------------------------------------------------------------------
	// Unbounded list read
	// -----------------------------------------------------------------------

	// Splits value on delim, parses each token into T.
	// Invalid tokens skipped + logged. Missing key returns empty vector.
	template<typename T>
	std::vector<T> ReadList(std::string_view section, std::string_view key,
							char delim = ',')
	{
		PhobosINISection* sec = GetSection(section);
		if (!sec)
			return {};

		PhobosINIEntry* entry = sec->FindEntry(key);
		if (!entry)
			return {};

		std::vector<T> result;

		for (auto token : SplitView(entry->Value, delim,
			StringSplitOptions::Trim |
			StringSplitOptions::RemoveEmpty))
		{
			auto parsed = PhobosParser<T, 1>::From(token);

			if (parsed.has_value())
			{
				result.push_back(std::move(*parsed));
			}
			else
			{
				Debug::Log(
					"PhobosINI: [%.*s] %.*s — invalid token '%.*s', skipped.\n",
					static_cast<int>(section.size()), section.data(),
					static_cast<int>(key.size()), key.data(),
					static_cast<int>(token.size()), token.data());
			}
		}

		return result;
	}

	// Raw string list — no strtok, owned strings.
	std::vector<std::string> ParseList(std::string_view section,
									   std::string_view key,
									   char delim = ',')
	{
		return ReadList<std::string>(section, key, delim);
	}

	// -----------------------------------------------------------------------
	// Speed helper (mirrors INI_EX::ReadSpeed)
	// -----------------------------------------------------------------------
	bool ReadSpeed(std::string_view section, std::string_view key, int* out)
	{
		const auto v = Read<double>(section, key);
		if (!v.has_value() || !out)
			return false;

		const int speed = static_cast<int>((std::min(*v, 100.0) * 256.0) / 100.0);
		*out = std::min(speed, 255);
		return true;
	}

	// -----------------------------------------------------------------------
	// Typed write — mirrors Read/TryRead/WriteList symmetrically.
	// All routes through WriteString — no separate buffers.
	// -----------------------------------------------------------------------

	// Single value: Write<T>(section, key, value)
	template<typename T>
	bool Write(std::string_view section, std::string_view key, const T& value)
	{
		return WriteString(section, key, PhobosFormatter<T>::ToString(value));
	}

	// int overload to support hex flag: Write<int>(s, k, v, true)
	bool WriteInt(std::string_view section, std::string_view key,
				  int value, bool hex = false)
	{
		return WriteString(section, key, PhobosFormatter<int>::ToString(value, hex));
	}

	// Fixed-count array: WriteArray<T,N>(section, key, values[N])
	// Serializes as "v0,v1,...,vN-1" — mirror of TryRead<T,N>.
	template<typename T, size_t N>
	bool WriteArray(std::string_view section, std::string_view key,
					const T* values, char delim = ',')
	{
		return WriteString(section, key, PhobosFormatArray<T, N>::ToString(values, delim));
	}

	// Convenience wrappers matching Read2/Read3/Read4 names.
	template<typename T> bool Write2(std::string_view s, std::string_view k, const T* v) { return WriteArray<T, 2>(s, k, v); }
	template<typename T> bool Write3(std::string_view s, std::string_view k, const T* v) { return WriteArray<T, 3>(s, k, v); }
	template<typename T> bool Write4(std::string_view s, std::string_view k, const T* v) { return WriteArray<T, 4>(s, k, v); }

	// Unbounded list: WriteList<T>(section, key, begin, end)
	// Accepts any iterator range — vector, array, initializer_list etc.
	template<typename It>
	bool WriteList(std::string_view section, std::string_view key,
				   It begin, It end, char delim = ',')
	{
		using T = std::remove_const_t<std::remove_reference_t<decltype(*begin)>>;
		std::string result;
		for (auto it = begin; it != end; ++it)
		{
			if (!result.empty())
				result += delim;
			result += PhobosFormatter<T>::ToString(*it);
		}
		return WriteString(section, key, result);
	}

	// -----------------------------------------------------------------------
   // Rate helpers — value stored as decimal fraction of 900 frames.
   // e.g. "0.5" <-> 450 frames. Mirrors INIClass::ReadRate/WriteRate.
   // -----------------------------------------------------------------------
	bool ReadRate(std::string_view section, std::string_view key, int* out)
	{
		const auto v = Read<PhobosRate>(section, key);
		if (!v.has_value() || !out)
			return false;
		*out = v->Frames;
		return true;
	}

	bool WriteRate(std::string_view section, std::string_view key, int frames)
	{
		return Write<PhobosRate>(section, key, PhobosRate { frames });
	}

	// -----------------------------------------------------------------------
	// Percent helpers — "50%" or "0.5" both read as 0.5 (double 0..1).
	// Write always emits "50%" style.
	// -----------------------------------------------------------------------
	bool ReadPercent(std::string_view section, std::string_view key, double* out)
	{
		const auto v = Read<PhobosPercent>(section, key);
		if (!v.has_value() || !out)
			return false;
		*out = v->Value;
		return true;
	}

	bool WritePercent(std::string_view section, std::string_view key, double value)
	{
		return Write<PhobosPercent>(section, key, PhobosPercent { value });
	}

	// Reads "1%,1%,0%,..." -> std::vector<double> (0..1 range).
	// Accepts mixed "50%" and "0.5" tokens in the same list.
	std::vector<double> ReadPercentList(std::string_view section, std::string_view key)
	{
		const auto raw = ReadList<PhobosPercent>(section, key);
		std::vector<double> result;
		result.reserve(raw.size());
		for (const auto& p : raw)
			result.push_back(p.Value);
		return result;
	}

	// Writes std::vector<double> (0..1 range) -> "50%,50%,0%" style.
	bool WritePercentList(std::string_view section, std::string_view key,
						  const std::vector<double>& values)
	{
		std::string result;
		result.reserve(values.size() * 5);
		for (size_t i = 0; i < values.size(); ++i)
		{
			if (i > 0)
				result += ',';
			result += PhobosFormatter<PhobosPercent>::ToString(PhobosPercent { values[i] });
		}
		return WriteString(section, key, result);
	}

private:
	static int  CRC(std::string_view s);

	void        InvalidateCache() { LastSectionName.clear(); LastSection = nullptr; }

	// Copies all entries from parentSection into destSectionName.
	// Existing keys in dest are NOT overwritten (child wins).
	void CopyInheritedSection(const PhobosINISection& parent, std::string_view destName);

	// Processes $include / #include section after a Load pass completes.
	// Mirrors CCINIClass_Parse hook logic but uses per-instance LoadedIncludes.
	void ProcessIncludes(bool useNewIncludes);
};

class PhobosINIContainer
{
public:
	static std::unique_ptr<PhobosINIClass> Rules_INI;
	static std::unique_ptr<PhobosINIClass> Art_INI;
	static std::unique_ptr<PhobosINIClass> Ai_INI;
	static std::unique_ptr<PhobosINIClass> Ui_INI;
	static std::unique_ptr<PhobosINIClass> Ra2_INI;
	static std::unique_ptr<PhobosINIClass> Mission_INI;
	static std::unique_ptr<PhobosINIClass> Movie_INI;

public:
	static PhobosINIContainer& Instance();

	PhobosINIClass& GetOrCreate(const void* pINI);
	PhobosINIClass* Find(const void* pINI);
	void Remove(const void* pINI);
	void Clear();

private:
	std::unordered_map<const void*, PhobosINIClass> Map;
};
