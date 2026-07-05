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
#include <bcrypt.h>

// Password/encryption support layer for ZipFileSystem.
//
// Architecture: decryption is a pre-processing step that produces a plain
// memory buffer, which is then handed to mz_zip_reader_init_mem(). This
// means the zip reader layer never needs to know about encryption at all.
//
// Two tiers supported:
//
//   Tier 1 — Hardcoded compile-time key (XOR obfuscation)
//     For "keep modders out of internal assets" scenarios.
//     NOT cryptographically secure — decompilable. Use only for obfuscation.
//     Enable with: ZIPFS_OBFUSCATION_KEY defined at compile time.
//
//   Tier 2 — AES-256-CBC with a runtime password
//     Requires Windows CNG (BCrypt.h, Bcrypt.lib) — already available in
//     the YR target environment (Win32, no extra deps).
//     Suitable for actual content protection.
//     Enable with: ZIPFS_AES_PASSWORD support.
//
// The ZipArchiveDecryptor interface is the extension point — if you later
// want minizip-ng or libsodium, implement a new subclass.

// ---------------------------------------------------------------------------
// ZipArchiveDecryptor — abstract decrypt interface
// ---------------------------------------------------------------------------
class ZipArchiveDecryptor
{
public:
	virtual ~ZipArchiveDecryptor() = default;

	// Decrypt `inSize` bytes from `pIn` into `outBuffer`.
	// Returns true on success. outBuffer is resized to the plaintext size.
	virtual bool Decrypt(const uint8_t* pIn, size_t inSize,
						 std::vector<uint8_t>& outBuffer) const = 0;

	// Human-readable name for logging.
	virtual const char* Name() const = 0;
};

// ---------------------------------------------------------------------------
// Tier 1 — XOR obfuscation (compile-time key)
// Not secure. Fast. Good enough to prevent casual hex editing.
//
// Usage:
//   auto dec = std::make_unique<XorObfuscator>(0xA5B3C1D7u);
//   ZipFileSystem::Instance().SetDecryptor(std::move(dec));
// ---------------------------------------------------------------------------
class XorObfuscator : public ZipArchiveDecryptor
{
public:
	explicit XorObfuscator(uint32_t key) : m_Key(key) {}

	virtual bool Decrypt(const uint8_t* pIn, size_t inSize,
						 std::vector<uint8_t>& outBuffer) const override
	{
		outBuffer.resize(inSize);

		// XOR each byte with rotating key bytes.
		const uint8_t* k = reinterpret_cast<const uint8_t*>(&m_Key);
		for (size_t i = 0; i < inSize; ++i)
			outBuffer[i] = pIn[i] ^ k[i & 3];

		return true;
	}

	virtual const char* Name() const override { return "XOR-Obfuscation"; }

private:
	uint32_t m_Key;
};

// -------------------------------------------------------------------------- -
// Tier 2 — AES-256-CBC via Windows CNG (BCrypt)
// Requires: #pragma comment(lib, "Bcrypt.lib") in your precompiled header
//           or project settings.
//
// File format expected:
//   [16 bytes IV][N bytes AES-256-CBC ciphertext]
//
// Usage:
//   auto dec = std::make_unique<AES256Decryptor>("MyModPassword123");
//   ZipFileSystem::Instance().SetDecryptor(std::move(dec));
// ---------------------------------------------------------------------------
class AES256Decryptor : public ZipArchiveDecryptor
{
public:
	// password: UTF-8 string, stretched to 32-byte key via SHA-256.
	explicit AES256Decryptor(const char* password);
	virtual ~AES256Decryptor() override;

	virtual bool Decrypt(const uint8_t* pIn, size_t inSize,
						 std::vector<uint8_t>& outBuffer) const override;

	virtual const char* Name() const override { return "AES-256-CBC/CNG"; }

private:
	// Derive a 32-byte key from the password using SHA-256 (CNG).
	bool DeriveKey(const char* password);

	uint8_t       m_Key[32] {};
	bool          m_KeyReady = false;

	// CNG algorithm handles — opened once, reused per decrypt call.
	BCRYPT_ALG_HANDLE  m_AesAlg = nullptr;
	BCRYPT_ALG_HANDLE  m_HashAlg = nullptr;
};


// -------------------------------------------------------------------------- -
// ZipEntry — one file inside a registered zip
// ---------------------------------------------------------------------------
struct ZipEntry
{
	std::string ArchivePath;  // path to the .zip on disk
	std::string EntryName;    // normalised: basename, uppercase
	mz_uint     EntryIndex;   // miniz index within the archive
};

// ---------------------------------------------------------------------------
// ZipFileSystem — singleton registry
// ---------------------------------------------------------------------------
class ZipFileSystem
{
public:
	static std::string zipfileExt;
	static std::string zipfileBaseDir;

public:
	static ZipFileSystem& Instance();

	// Scan `directory` for *.zip files. Call at game startup.
	// Later zips override earlier ones for duplicate entry names.
	void ScanDirectory();

	// Register a single zip explicitly (useful for testing).
	bool RegisterZip(const char* zipPath , const ZipArchiveDecryptor* decryptor = nullptr);

	// Release all open archive handles and clear the index.
	void Clear();

	// Returns true if `filename` (case-insensitive, basename only) is in any zip.
	bool Contains(const char* filename) const;

	// Extract `filename` into a newly allocated buffer.
	// Caller owns the returned buffer. outSize is set to byte count.
	// Returns nullptr on failure.
	std::unique_ptr<uint8_t[]> Extract(const char* filename, size_t& outSize) const;

	// Enumerate all registered entries (debugging / tooling).
	void ForEach(std::function<void(const ZipEntry&)> callback) const;

	// Set a global default decryptor used when RegisterZip is called
	// without an explicit one. Useful for "all zips use the same key".
	void SetDefaultDecryptor(std::unique_ptr<ZipArchiveDecryptor> dec)
	{
		m_DefaultDecryptor = std::move(dec);
	}
private:
	ZipFileSystem() = default;
	~ZipFileSystem() = default;
	ZipFileSystem(const ZipFileSystem&) = delete;
	ZipFileSystem& operator=(const ZipFileSystem&) = delete;

	// Normalise a filename: strip directory prefix, uppercase.
	static std::string Normalise(const char* name);

	// An open miniz archive kept alive for the session.
	struct OpenArchive
	{
		std::string    Path;
		mz_zip_archive Handle {};   // fully defined via miniz.h — no cast needed
		bool           Valid = false;
		bool                    IsInMemory = false;
		std::vector<uint8_t>    DecryptedBuf {};
	};

	// Returns (or opens and caches) an archive by path.
	OpenArchive* GetOrOpen(const char* zipPath,
						   const ZipArchiveDecryptor* decryptor = nullptr);

	// normalised name -> entry (last-registered zip wins)
	std::unordered_map<std::string, ZipEntry>     EntryMap;

	// open archive handles, keyed by zip path
	std::vector<std::unique_ptr<OpenArchive>>     Archives;

	std::unique_ptr<ZipArchiveDecryptor>          m_DefaultDecryptor;
};

// Phobos extension of RAMFileClass that adds an optional debug name.
//
// WHY a subclass and not patching RAMFileClass directly:
//   - RAMFileClass has a static_assert(sizeof == 0x20) that must not change.
//   - The binary vtable at 0x7F0874 expects exactly 0x20 bytes.
//   - Adding a field to RAMFileClass would silently corrupt every engine
//     site that stack-allocates it (e.g. the PublicKey INI usage in MixFile).
//
// This subclass adds zero cost to vanilla usage — only Phobos code
// that explicitly uses NamedRAMFileClass gets the name field.
//
// Usage:
//   // Wrap a const string literal (no copy, zero alloc):
//   NamedRAMFileClass f("[PublicKey] ini text...", len, "PublicKey-INI");
//
//   // Wrap an existing heap buffer (no copy):
//   NamedRAMFileClass f(myBuf, mySize, "SHP:conyard.shp");
//
//   // Allocate a fresh buffer (RAMFileClass allocates internally):
//   NamedRAMFileClass f(nullptr, size, "WriteBuffer:temp");
//
//   f.FileName();   // returns "SHP:conyard.shp" instead of "UNKNOWN"

class NamedRAMFileClass : public RAMFileClass
{
public:
	// ------------------------------------------------------------------
	// Constructors
	// ------------------------------------------------------------------

	// Wrap an existing buffer with a debug name.
	// If pData == nullptr and nSize > 0, RAMFileClass allocates internally.
	// DebugName is a caller-owned string literal or long-lived pointer —
	// this class does NOT copy or free it.
	NamedRAMFileClass(void* pData, size_t nSize, const char* debugName)
		: RAMFileClass(pData, nSize)
		, m_DebugName(debugName)
	{}

	// Convenience: wrap a const char* string directly (e.g. inline INI text).
	// The string pointer must outlive this object — no copy is made.
	NamedRAMFileClass(const char* stringData, size_t nSize, const char* debugName)
		: RAMFileClass(const_cast<void*>(static_cast<const void*>(stringData)), nSize)
		, m_DebugName(debugName)
	{}

	// Default: unnamed (falls back to "UNKNOWN" from base).
	explicit NamedRAMFileClass(void* pData, size_t nSize)
		: RAMFileClass(pData, nSize)
		, m_DebugName(nullptr)
	{}

	virtual ~NamedRAMFileClass() override = default;

	// ------------------------------------------------------------------
	// FileClass overrides
	// ------------------------------------------------------------------

	// Returns the debug name if set, otherwise vanilla "UNKNOWN".
	virtual const char* FileName() const override
	{
		return m_DebugName ? m_DebugName : RAMFileClass::FileName();
	}

	// SetFileName: store the new name pointer (no alloc — caller owns it).
	virtual const char* SetFileName(const char* name) override
	{
		m_DebugName = name;
		return m_DebugName;
	}

	// ------------------------------------------------------------------
	// Debug helper
	// ------------------------------------------------------------------

	// Useful for logging: "RAMFile[SHP:conyard.shp](size=12345)"
	// Returns a stack-allocated string — do not store the pointer.
	// Only available in debug builds to avoid overhead in release.
//#ifndef NDEBUG
//	const char* DebugDescription(char* buf, size_t bufSize) const
//	{
//		_snprintf_s(buf, bufSize, _TRUNCATE,
//			"RAMFile[%s](size=%d)",
//			m_DebugName ? m_DebugName : "UNKNOWN",
//			static_cast<int>(this->Size()));
//		return buf;
//	}
//#endif

private:
	// Pointer to a caller-owned name string. Not freed on destruction.
	// Typical usage: string literal ("SHP:conyard.shp") or a persistent
	// std::string::c_str() from a longer-lived owner.
	const char* m_DebugName;
};

// No sizeof assert here — size depends on compiler padding of m_DebugName,
// and this class is never used in binary-layout-sensitive positions.


// Now inherits NamedRAMFileClass instead of raw FileClass.
// This means:
//   - FileName() returns the actual zip entry name automatically.
//   - No need to override FileName() / SetFileName() ourselves.
//   - Read/Seek/Size/Open/Close all come from RAMFileClass — no manual impl.
//   - We only override what RAMFileClass can't do: Open1 (read-only guard),
//     IsAvaible (check if extraction succeeded), and the constructor.
//
// NOTE: RAMFileClass::Read/Seek/Write/Close work correctly on the internal
// buffer once we call Open1(). We no longer need our own Read/Seek/Close.
class ZipBackedFileClass : public NamedRAMFileClass
{
public:
	// Construct from a zip entry name. Check Valid() before use.
	explicit ZipBackedFileClass(const char* filename);
	virtual ~ZipBackedFileClass() override = default;

	// Returns false if the file was not found in any registered zip.
	bool Valid() const { return m_Buffer != nullptr; }

	// --- Minimal overrides ---

	// Guard: zip files are read-only.
	virtual bool Open1(FileAccessMode access) override;

	// Use our extraction result, not the base buffer check.
	virtual bool IsAvaible(bool writeShared = false) override { return Valid(); }

	// Write is meaningless on a zip-backed buffer.
	virtual int Write(void* /*buf*/, int /*len*/) override { return 0; }

	// GetDataTime / SetDateTime: no meaningful timestamp from zip.
	virtual LONG GetDataTime()          override { return 0; }
	virtual bool SetDateTime(LONG)      override { return false; }

	// Silence error output — missing zip files are handled by caller.
	virtual void Error(FileErrorType, bool, const char*) override {}

private:
	// Heap buffer extracted from the zip. Kept alive for the object's lifetime.
	// RAMFileClass stores a raw char* internally; we own the allocation here
	// and pass the raw pointer to RAMFileClass via the base constructor.
	std::unique_ptr<uint8_t[]> m_Buffer;
	size_t                     m_Size = 0;
};


void* __fastcall _RetrieveFile(char* name, char force_shape_cache);