#pragma once

#include <StringTable.h>

#include <unordered_map>
#include <string>
#include <string_view>
#include <cctype>
#include <cstring>

// =============================================================================
// Small filename helpers shared by the CSF and LLF loaders.
// =============================================================================
namespace PhobosCSFDetail
{
	// Case-insensitive extension test. `ext` must include the dot, e.g. ".llf".
	inline bool HasExtension(std::string_view fileName, std::string_view ext)
	{
		if (fileName.size() < ext.size())
			return false;

		const size_t off = fileName.size() - ext.size();

		for (size_t i = 0; i < ext.size(); ++i)
		{
			const int a = std::tolower(static_cast<unsigned char>(fileName[off + i]));
			const int b = std::tolower(static_cast<unsigned char>(ext[i]));

			if (a != b)
				return false;
		}

		return true;
	}

	// Strips the last extension (if any) and appends `newExt` (which includes the dot).
	inline std::string ReplaceExtension(std::string_view fileName, std::string_view newExt)
	{
		std::string result(fileName);

		const size_t lastDot = result.find_last_of('.');
		const size_t lastSep = result.find_last_of("\\/");

		if (lastDot != std::string::npos && (lastSep == std::string::npos || lastDot > lastSep))
			result.erase(lastDot);

		result.append(newExt);
		return result;
	}
}

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

	// -------------------------------------------------------------------------
	// LLF (Localization List Format) — plain UTF-8 text sibling of the CSF
	// container. See CSF.LLF.cpp for the full grammar description.
	//
	// Resolution goes through CCFileClass, so LLF files inside MIX archives are
	// picked up exactly like loose files.
	// -------------------------------------------------------------------------
	static bool           ParseLLFFile(std::string_view pFileName);

	// Scans stringtable00..stringtable99. For every index the .csf is parsed
	// first and the .llf second, so LLF always wins on a duplicate label.
	static void           LoadAdditionalStringTables(bool ignoreLanguage = false);

	// UTF-8 -> UTF-16 (wchar_t is 16 bit on Win32; astral planes become surrogate
	// pairs). Invalid / overlong / truncated sequences decode to U+FFFD.
	static std::wstring   Utf8ToWide(std::string_view text);

	// -------------------------------------------------------------------------
	// LLF parser tunables — runtime so they can be driven from INI later.
	// -------------------------------------------------------------------------

	// false (default): continuation lines of a plain `Label: value` entry are
	//                  joined with L'\n', matching the block (`>-`) behaviour.
	// true           : they are folded with a single space, YAML `>` style.
	static bool LLFFoldPlainContinuations;

	// true (default): the entire leading whitespace run of a continuation line is
	//                 removed, so generator indent width does not leak into the
	//                 string. false: only the mandatory two spaces are removed.
	static bool LLFTrimContinuationIndent;

	// EXTENSION (true by default): `\#` emits a literal '#' instead of starting a
	// comment. Not part of the base LLF spec — set to false for strict parsing.
	static bool LLFAllowHashEscape;
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