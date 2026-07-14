#pragma once

#include <StringTable.h>

#include <unordered_map>
#include <string>

// =============================================================================
// CSFLoader — full takeover of TextManager::Init + TextManager::ParseCSF.
// No engine arrays (Labels/Values/ExtraValues) are used.
// All label storage lives in LabelMap; lookup is O(1) via unordered_map.
// No label name length limit (char[0x20] was a StringLookUp layout artifact).
// No entry count ceiling (was MaxEntries = 320000).
// =============================================================================
struct CaseInsensitiveCompare {
	using is_transparent = void;

	bool operator()(const std::string& lhs, const std::string& rhs) const
	{
		if (lhs.size() != rhs.size()) return false;
		return compare_bytes(lhs.data(), rhs.data(), lhs.size());
	}

	bool operator()(const std::string& lhs, const char* rhs) const
	{
		size_t r_len = std::strlen(rhs);
		if (lhs.size() != r_len) return false;
		return compare_bytes(lhs.data(), rhs, lhs.size());
	}

	bool operator()(const char* lhs, const std::string& rhs) const
	{
		size_t l_len = std::strlen(lhs);
		if (l_len != rhs.size()) return false;
		return compare_bytes(lhs, rhs.data(), rhs.size());
	}

private:
	bool compare_bytes(const char* s1, const char* s2, size_t len) const {
		for (size_t i = 0; i < len; ++i) {
			if (std::tolower(static_cast<unsigned char>(s1[i])) != std::tolower(static_cast<unsigned char>(s2[i]))) {
				return false;
			}
		}
		return true;
	}
};

struct CaseInsensitiveHash
{
	using is_transparent = void; // Enables const char* lookups without allocations

	size_t operator()(const std::string& str) const
	{
		return hash_bytes(str.data(), str.size());
	}

	size_t operator()(const char* str) const
	{
		return hash_bytes(str, std::strlen(str));
	}

private:
	// Simple FNV-1a case-insensitive hash
	size_t hash_bytes(const char* data, size_t len) const
	{
		size_t hash = 2166136261U;
		for (size_t i = 0; i < len; ++i)
		{
			hash ^= static_cast<size_t>(std::tolower(static_cast<unsigned char>(data[i])));
			hash *= 16777619U;
		}
		return hash;
	}
};

class CSFLoader
{
public:
	// -------------------------------------------------------------------------
	// Primary label storage.
	// Key   : label name — unlimited length, case-preserved.
	// Value : XOR-decoded + whitespace-stripped wstring + optional speech string.
	// Duplicate key on insert_or_assign = last-loaded wins (override semantics).
	// -------------------------------------------------------------------------
	struct CSFEntry
	{
		std::wstring Value;
		std::string  ExtraValue; // WRTS speech filename; empty if absent
	};

	struct RecordedCSFEntry
	{
		CSFEntry Entry;
		std::string Source;
	};
	static std::unordered_map<std::string,
		RecordedCSFEntry, 
		CaseInsensitiveHash,
		CaseInsensitiveCompare
	> LabelMap;

	// -------------------------------------------------------------------------
	// Missing / NOSTR: string cache — separate namespace from real labels.
	// Write-once: pointer returned by c_str() is stable for lifetime of entry.
	// -------------------------------------------------------------------------
	struct CSFStringStorage
	{
		std::wstring Text;
		bool         TextLoaded;
		bool         IsMissingValue;

		CSFStringStorage() : Text {}, TextLoaded {}, IsMissingValue { true } {}
		~CSFStringStorage() = default;
	};

	static std::unordered_map<std::string,
		CSFStringStorage, 
		CaseInsensitiveHash,
		CaseInsensitiveCompare
	> DynamicStrings;

	// -------------------------------------------------------------------------
	// Additional-file load counter.
	// 0 = base CSF pass; >0 = additional CSF override pass.
	// -------------------------------------------------------------------------
	static int CSFCount;

	// -------------------------------------------------------------------------
	// Public API
	// -------------------------------------------------------------------------
	static bool           PhobosInit(const char* pFileName);
	static bool           ParseCSFFile(std::string_view pFileName, bool ignoreLanguage = false);
	static void           ReleaseStorage();
	static void           LoadAdditionalCSF(std::string_view fileName, bool ignoreLanguage = false);
	static const wchar_t* GetDynamicString(const char* name, const char* def, bool isNostr);
	static const wchar_t* __fastcall FetchStringManager(const char* label, char* speech,
														 const char* file, int line);

	static FORCEDINLINE CSFStringStorage* FindOrAllocateDynamicStrings(const char* val)
	{
		return &DynamicStrings[val];
	}

	static FORCEDINLINE bool IsStringPatternFound(const char* val)
	{
		return DynamicStrings.contains(val);
	}
};