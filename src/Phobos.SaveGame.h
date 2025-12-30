#pragma once

#include "Phobos.h"
#include <Utilities/Debug.h>

struct SaveHeader
{
	DWORD       Magic;
	int         Version;
	int			VersionIdentifier;
	std::time_t Timestamp;
	std::string ModName;
	std::string ModVersion;

public:
	static bool WriteSaveHeader(json& root, const SaveHeader& header)
	{
		json h;

		// DWORD → force 32-bit unsigned
		h["magic"] = static_cast<uint32_t>(header.Magic);

		// version → int (JSON number)
		h["version"] = header.Version;

		h["version_identifier"] = header.VersionIdentifier;

		// timestamp → force 64-bit integer for safety
		h["timestamp"] = static_cast<int64_t>(header.Timestamp);

		// strings
		h["mod_name"] = header.ModName;
		h["mod_version"] = header.ModVersion;

		root["header"] = std::move(h);
		return true;
	}

	static bool ReadSaveHeader(const json& root, SaveHeader& out)
	{
		auto itHeader = root.find("header");
		if (itHeader == root.end() || !itHeader->is_object())
			return false;

		const json& h = *itHeader;

		// magic (DWORD)
		{
			auto it = h.find("magic");
			if (it == h.end() || !it->is_number_unsigned())
				return false;

			uint64_t v = it->get<uint64_t>();
			if (v > UINT32_MAX)
				return false;

			out.Magic = static_cast<DWORD>(v);
		}

		// version (int)
		{
			auto it = h.find("version");
			if (it == h.end() || !it->is_number_integer())
				return false;

			out.Version = it->get<int>();
		}

		// VersionIdentifier (int)
		{
			auto it = h.find("version_identifier");
			if (it == h.end() || !it->is_number_integer())
				return false;

			out.VersionIdentifier = it->get<int>();
		}

		// timestamp (time_t stored as integer)
		{
			auto it = h.find("timestamp");
			if (it == h.end() || !it->is_number_integer())
				return false;

			out.Timestamp = static_cast<std::time_t>(it->get<int64_t>());
		}

		// mod_name
		{
			auto it = h.find("mod_name");
			if (it == h.end() || !it->is_string())
				return false;

			out.ModName = it->get<std::string>();
		}

		// mod_version
		{
			auto it = h.find("mod_version");
			if (it == h.end() || !it->is_string())
				return false;

			out.ModVersion = it->get<std::string>();
		}

		return true;
	}
};

class ExtensionSaveJson
{
public:

	static constexpr const wchar_t* FileExtension = L"_ext.json";

	static bool Save(const wchar_t* baseSave);
	static bool Load(const wchar_t* baseSave);

public:

	static inline void WriteHex(json& j, const char* key, uint32_t value)
	{
		char buf[11];
		std::snprintf(buf, sizeof(buf), "0x%08X", value);
		j[key] = buf;
	}

	static inline bool ReadHex(const json& j, const char* key, uint32_t& out)
	{
		auto it = j.find(key);
		if (it == j.end() || !it->is_string())
			return false;

		out = static_cast<uint32_t>(
			std::stoul(it->get<std::string>(), nullptr, 0)
		);
		return true;
	}


	static inline std::filesystem::path GetExtPath(const wchar_t* baseSave)
	{
		std::filesystem::path p(baseSave);

		// Examples:
		// L"SAVE1.SAV" -> L"SAVE1_ext.json"
		// L"C:\Games\RA2\Saved Games\SAVE1.SAV" -> L"C:\Games\RA2\Saved Games\SAVE1_ext.json"

		std::filesystem::path dir = p.parent_path();
		std::wstring stem = p.stem().wstring();

		std::filesystem::path extFile = stem + ExtensionSaveJson::FileExtension;

		if (dir.empty())
			return extFile;

		return dir / extFile;
	}

	static inline std::filesystem::path GetExtPath(const char* baseSave)
	{
		// Convert to wide string
		int len = MultiByteToWideChar(CP_ACP, 0, baseSave, -1, nullptr, 0);
		std::wstring wide(len - 1, L'\0');
		MultiByteToWideChar(CP_ACP, 0, baseSave, -1, wide.data(), len);

		return GetExtPath(wide.c_str());
	}

};

struct Base64Handler
{
	static int __fastcall Base64_Encode(const void* source, int slen, void* dest, int dlen)
	{ JMP_STD(0x42FD30); }

	static int __fastcall Base64_Decode(const void* source, int slen, void* dest, int dlen)
	{ JMP_STD(0x42FE50); }

	static std::string encodeBase64(std::vector<uint8_t>& data)
	{
		size_t rawSize = data.size();
		size_t outSize = ((rawSize + 2) / 3) * 4 + 1;

		std::string encoded;
		encoded.resize(outSize);

		int written = Base64_Encode(
			data.data(),
			(int)rawSize,
			encoded.data(),
			(int)outSize
		);

		encoded.resize(written); // trim
		return encoded;
	}

	static std::vector<uint8_t> decodeBase64(const std::string& encoded, size_t expectedRawSize)
	{
		std::vector<uint8_t> decoded;
		decoded.resize(expectedRawSize);

		int written = Base64_Decode(
			encoded.data(),
			static_cast<int>(encoded.size()),
			decoded.data(),
			static_cast<int>(decoded.size())
		);

		// Safety checks
		if (written <= 0)
		{
			Debug::FatalErrorAndExit("Base64 decode failed");
		}

		if (static_cast<size_t>(written) != expectedRawSize)
		{
			Debug::FatalErrorAndExit(
				"Base64 decode size mismatch (expected %zu, got %d)",
				expectedRawSize,
				written
			);
		}

		return decoded;
	}

};
