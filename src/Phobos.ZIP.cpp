#include "Phobos.ZIP.h"
// ZipFileSystem.cpp
// See ZipFileSystem.h for architecture notes.
//
// Requires: miniz.h (https://github.com/richgel999/miniz, single-header)
// Place miniz.h in the same directory or on your include path.
// Define MINIZ_IMPLEMENTATION in exactly one .cpp — this one.

#include <Phobos.h>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

std::string ZipFileSystem::Normalise(const char* name)
{
	std::string s(name ? name : "");

	// Strip directory prefix inside the zip ("assets/foo.shp" -> "FOO.SHP")
	auto slash = s.find_last_of("/\\");
	if (slash != std::string::npos)
		s = s.substr(slash + 1);

	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return static_cast<char>(std::toupper(c)); });
	return s;
}

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------

ZipFileSystem& ZipFileSystem::Instance()
{
	static ZipFileSystem inst;
	return inst;
}

// ---------------------------------------------------------------------------
// ScanDirectory
// ---------------------------------------------------------------------------

void ZipFileSystem::ScanDirectory(const char* directory)
{
	std::error_code ec;
	for (auto& entry : fs::directory_iterator(directory, ec))
	{
		if (ec)   break;
		if (!entry.is_regular_file()) continue;

		auto ext = entry.path().extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		if (ext == ".yrarc")
			RegisterZip(entry.path().string().c_str());
	}
}

// ---------------------------------------------------------------------------
// RegisterZip
// ---------------------------------------------------------------------------

bool ZipFileSystem::RegisterZip(const char* zipPath)
{
	auto* arc = GetOrOpenArchive(zipPath);
	if (!arc->Valid)
		return false;

	mz_uint numFiles = mz_zip_reader_get_num_files(&arc->Handle);
	for (mz_uint i = 0; i < numFiles; ++i)
	{
		if (mz_zip_reader_is_file_a_directory(&arc->Handle, i))
			continue;

		char entryName[512] = {};
		mz_zip_reader_get_filename(&arc->Handle, i, entryName, sizeof(entryName));

		std::string key = Normalise(entryName);
		if (key.empty()) continue;

		// Last registered zip wins.
		EntryMap[key] = ZipEntry { zipPath, key, i };
	}
	return true;
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------

void ZipFileSystem::Clear()
{
	EntryMap.clear();
	// Close all open archives properly.
	for (auto& arc : Archives)
	{
		if (arc->Valid)
			mz_zip_reader_end(&arc->Handle);
	}
	Archives.clear();
}

// ---------------------------------------------------------------------------
// Contains
// ---------------------------------------------------------------------------

bool ZipFileSystem::Contains(const char* filename) const
{
	return EntryMap.count(Normalise(filename)) > 0;
}

// ---------------------------------------------------------------------------
// Extract
// ---------------------------------------------------------------------------

std::unique_ptr<uint8_t[]> ZipFileSystem::Extract(const char* filename, size_t& outSize)
{
	outSize = 0;

	std::string key = Normalise(filename);
	auto it = EntryMap.find(key);
	if (it == EntryMap.end())
		return nullptr;

	const ZipEntry& entry = it->second;
	auto* arc = GetOrOpenArchive(entry.ArchivePath.c_str());
	if (!arc->Valid)
		return nullptr;

	mz_zip_archive_file_stat stat {};
	if (!mz_zip_reader_file_stat(&arc->Handle, entry.EntryIndex, &stat))
		return nullptr;

	outSize = static_cast<size_t>(stat.m_uncomp_size);
	auto buffer = std::make_unique<uint8_t[]>(outSize);

	if (!mz_zip_reader_extract_to_mem(&arc->Handle, entry.EntryIndex,
		buffer.get(), outSize, 0))
	{
		outSize = 0;
		return nullptr;
	}

	return buffer;
}

// ---------------------------------------------------------------------------
// ForEach
// ---------------------------------------------------------------------------

void ZipFileSystem::ForEach(std::function<void(const ZipEntry&)> callback) const
{
	for (auto& [key, entry] : EntryMap)
		callback(entry);
}

// ---------------------------------------------------------------------------
// GetOrOpenArchive  —  returns cached or newly opened archive
// ---------------------------------------------------------------------------

ZipFileSystem::OpenArchive* ZipFileSystem::GetOrOpenArchive(const char* zipPath)
{
	// Check cache.
	for (auto& arc : Archives)
	{
		if (arc->Path == zipPath)
			return arc.get();
	}

	// Open new.
	auto arc = std::make_unique<OpenArchive>();
	arc->Path = zipPath;
	// mz_zip_archive is a plain struct — zero-init via {} in the header is fine.
	arc->Valid = mz_zip_reader_init_file(&arc->Handle, zipPath, 0) != MZ_FALSE;

	Archives.push_back(std::move(arc));
	return Archives.back().get();
}