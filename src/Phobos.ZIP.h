#pragma once
// ZipFileSystem.h
// Provides transparent zip-backed asset loading for YR/Phobos.
// Modders drop .zip files next to the game exe; files inside are found
// before Mix/disk. Uses miniz (miniz.h, single-header, no extra deps).
//
// Load order (highest priority first):
//   1. Zip files (last registered wins, like Mix priority)
//   2. Mix files (vanilla)
//   3. Loose disk files
//
// Integration: call ZipFileSystem::ScanDirectory() early in startup
// (e.g. in a DEFINE_HOOK on the game init path).
#define MINIZ_IMPLEMENTATION
#include  <Lib/miniz/miniz.h>

#include <CCFileClass.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

// ---------------------------------------------------------------------------
// ZipEntry  —  one file inside a registered zip
// ---------------------------------------------------------------------------
struct ZipEntry
{
	std::string  ArchivePath;   // path to the .zip on disk
	std::string  EntryName;     // normalised (basename, uppercase)
	mz_uint      EntryIndex;    // miniz index within the archive
};

// ---------------------------------------------------------------------------
// ZipFileSystem  —  singleton registry
// ---------------------------------------------------------------------------
class ZipFileSystem
{
public:
	static ZipFileSystem& Instance();

	// Scan a directory for *.zip files. Call at startup.
	// Later zips override earlier ones for the same entry name.
	void ScanDirectory(const char* directory = ".");

	// Register a single zip explicitly.
	bool RegisterZip(const char* zipPath);

	// Unregister everything (hot-reload / teardown).
	void Clear();

	// Returns true if filename (case-insensitive, basename only) is in any zip.
	bool Contains(const char* filename) const;

	// Extract filename into a heap buffer. Caller owns the buffer.
	// outSize is set to byte count. Returns nullptr on failure.
	std::unique_ptr<uint8_t[]> Extract(const char* filename, size_t& outSize);

	// Enumerate all entries (debugging / tooling).
	void ForEach(std::function<void(const ZipEntry&)> callback) const;

private:
	ZipFileSystem() = default;
	~ZipFileSystem() = default;
	ZipFileSystem(const ZipFileSystem&) = delete;
	ZipFileSystem& operator=(const ZipFileSystem&) = delete;

	// normalised-name -> entry (last-registered wins)
	std::unordered_map<std::string, ZipEntry> EntryMap;

	// Cached open archives keyed by path.
	// Kept open for the session to avoid repeated fopen overhead.
	struct OpenArchive
	{
		std::string     Path;
		mz_zip_archive  Handle {};   // miniz type — fully defined via miniz.h
		bool            Valid = false;
	};
	std::vector<std::unique_ptr<OpenArchive>> Archives;

	static std::string Normalise(const char* name);

	// Returns (or opens) a cached archive. Never returns nullptr.
	OpenArchive* GetOrOpenArchive(const char* zipPath);
};

// ---------------------------------------------------------------------------
// ZipBackedFileClass — drop-in FileClass backed by a zip entry.
// Inherits RAMFileClass so the engine reads it like any other file.
// ---------------------------------------------------------------------------
class ZipBackedFileClass : public RAMFileClass
{
public:
	// Constructs from a filename. Returns an invalid (IsAvailable==false)
	// object if the file is not found in any registered zip.
	explicit ZipBackedFileClass(const char* filename) : RAMFileClass(nullptr, 0)
		, m_FileName(filename ? filename : "")
	{
		m_Buffer = ZipFileSystem::Instance().Extract(filename, m_Size);
		if (m_Buffer)
		{
			m_Valid = true;
			// Hand the buffer to RAMFileClass so Read/Seek/etc. work automatically.
			// RAMFileClass expects a pointer + size; we pass the raw buffer.
			// NOTE: RAMFileClass does NOT take ownership — we keep m_Buffer alive.
			RAMFileClass::ReadBytes(m_Buffer.get(), static_cast<long>(m_Size));
		}
	}

	virtual ~ZipBackedFileClass() override
	{
		// m_Buffer owns the memory; RAMFileClass::Set_Data just stores a pointer.
		// Nothing extra needed — unique_ptr cleans up.
	}

	// FileClass interface
	virtual bool        Exists(bool writeShared = false) override
	{
		return m_Valid;
	}

	virtual const char* GetFileName() const              override
	{
		return m_FileName.c_str();
	}

	bool Valid() const { return m_Valid; }

private:
	std::string                  m_FileName;
	std::unique_ptr<uint8_t[]>   m_Buffer;
	size_t                       m_Size = 0;
	bool                         m_Valid = false;
};