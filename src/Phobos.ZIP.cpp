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

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

namespace fs = std::filesystem;

std::string ZipFileSystem::zipfileExt { ".yrarch" };
std::string ZipFileSystem::zipfileBaseDir { "." };

// ---------------------------------------------------------------------------
// AES256Decryptor — constructor / destructor
// ---------------------------------------------------------------------------
AES256Decryptor::AES256Decryptor(const char* password)
{
	// Open AES algorithm provider.
	if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(
		&m_AesAlg, BCRYPT_AES_ALGORITHM, nullptr, 0)))
	{
		Debug::Log("[ZipFS/AES] Failed to open AES provider\n");
		return;
	}

	// Set CBC chaining mode.
	if (!BCRYPT_SUCCESS(BCryptSetProperty(
		m_AesAlg,
		BCRYPT_CHAINING_MODE,
		(PUCHAR)BCRYPT_CHAIN_MODE_CBC,
		sizeof(BCRYPT_CHAIN_MODE_CBC), 0)))
	{
		Debug::Log("[ZipFS/AES] Failed to set CBC mode\n");
		return;
	}

	// Open SHA-256 provider for key derivation.
	if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(
		&m_HashAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0)))
	{
		Debug::Log("[ZipFS/AES] Failed to open SHA-256 provider\n");
		return;
	}

	m_KeyReady = DeriveKey(password);

	if (!m_KeyReady)
		Debug::Log("[ZipFS/AES] Key derivation failed\n");
}

AES256Decryptor::~AES256Decryptor()
{
	// Zero the key material before releasing.
	SecureZeroMemory(m_Key, sizeof(m_Key));

	if (m_AesAlg)  BCryptCloseAlgorithmProvider(m_AesAlg, 0);
	if (m_HashAlg) BCryptCloseAlgorithmProvider(m_HashAlg, 0);
}

// ---------------------------------------------------------------------------
// DeriveKey — SHA-256 hash of password -> 32-byte AES key
// ---------------------------------------------------------------------------
bool AES256Decryptor::DeriveKey(const char* password)
{
	if (!password || !m_HashAlg)
		return false;

	BCRYPT_HASH_HANDLE hHash = nullptr;
	DWORD  hashObjSize = 0;
	DWORD  bytesReturned = 0;

	// Query hash object size.
	if (!BCRYPT_SUCCESS(BCryptGetProperty(
		m_HashAlg, BCRYPT_OBJECT_LENGTH,
		(PUCHAR)&hashObjSize, sizeof(DWORD), &bytesReturned, 0)))
		return false;

	std::vector<uint8_t> hashObj(hashObjSize);

	if (!BCRYPT_SUCCESS(BCryptCreateHash(
		m_HashAlg, &hHash,
		hashObj.data(), hashObjSize,
		nullptr, 0, 0)))
		return false;

	const DWORD pwLen = static_cast<DWORD>(std::char_traits<char>::length(password));
	bool ok = BCRYPT_SUCCESS(BCryptHashData(hHash, (PUCHAR)password, pwLen, 0))
		&& BCRYPT_SUCCESS(BCryptFinishHash(hHash, m_Key, 32, 0));

	BCryptDestroyHash(hHash);
	return ok;
}

// ---------------------------------------------------------------------------
// Decrypt
// File format: [16 bytes IV][ciphertext...]
// ---------------------------------------------------------------------------
bool AES256Decryptor::Decrypt(const uint8_t* pIn, size_t inSize,
							   std::vector<uint8_t>& outBuffer) const
{
	if (!m_KeyReady || !m_AesAlg)
	{
		Debug::Log("[ZipFS/AES] Decrypt called but key not ready\n");
		return false;
	}

	// Minimum: 16 bytes IV + at least one AES block (16 bytes).
	if (inSize < 32)
	{
		Debug::Log("[ZipFS/AES] Ciphertext too short (%zu bytes)\n", inSize);
		return false;
	}

	const uint8_t* iv = pIn;
	const uint8_t* ciphertext = pIn + 16;
	const size_t   ctSize = inSize - 16;

	// Import the raw key.
	BCRYPT_KEY_HANDLE hKey = nullptr;

	// Key blob: BCRYPT_KEY_DATA_BLOB_HEADER + raw key bytes.
	struct KeyBlob
	{
		BCRYPT_KEY_DATA_BLOB_HEADER hdr;
		uint8_t                     key[32];
	} blob;

	blob.hdr.dwMagic = BCRYPT_KEY_DATA_BLOB_MAGIC;
	blob.hdr.dwVersion = BCRYPT_KEY_DATA_BLOB_VERSION1;
	blob.hdr.cbKeyData = 32;
	memcpy(blob.key, m_Key, 32);

	if (!BCRYPT_SUCCESS(BCryptImportKey(
		m_AesAlg, nullptr, BCRYPT_KEY_DATA_BLOB,
		&hKey, nullptr, 0,
		(PUCHAR)&blob, sizeof(blob), 0)))
	{
		SecureZeroMemory(&blob, sizeof(blob));
		Debug::Log("[ZipFS/AES] BCryptImportKey failed\n");
		return false;
	}

	SecureZeroMemory(&blob, sizeof(blob));

	// Copy IV (BCrypt modifies the IV buffer in-place during CBC).
	uint8_t ivCopy[16];
	memcpy(ivCopy, iv, 16);

	// Decrypt in-place into outBuffer.
	outBuffer.resize(ctSize);
	memcpy(outBuffer.data(), ciphertext, ctSize);

	DWORD plainSize = 0;
	const bool ok = BCRYPT_SUCCESS(BCryptDecrypt(
		hKey,
		outBuffer.data(), static_cast<ULONG>(ctSize),
		nullptr,
		ivCopy, 16,
		outBuffer.data(), static_cast<ULONG>(ctSize),
		&plainSize,
		BCRYPT_BLOCK_PADDING));

	BCryptDestroyKey(hKey);

	if (!ok)
	{
		Debug::Log("[ZipFS/AES] BCryptDecrypt failed — wrong password?\n");
		outBuffer.clear();
		return false;
	}

	// BCryptDecrypt with PKCS7 padding may shrink the output.
	outBuffer.resize(plainSize);
	return true;
}

// ---------------------------------------------------------------------------
// Normalise: strip directory prefix inside the zip, uppercase.
// "assets/ui/mypip.shp" -> "MYPIP.SHP"
// ---------------------------------------------------------------------------
std::string ZipFileSystem::Normalise(const char* name)
{
	std::string s(name ? name : "");
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
void ZipFileSystem::ScanDirectory()
{
	std::error_code ec;

	for (auto& de : fs::directory_iterator(ZipFileSystem::zipfileBaseDir, ec)) {
		if (ec) break;
		if (!de.is_regular_file()) continue;

		auto ext = de.path().extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		if (ext == ZipFileSystem::zipfileExt)
			RegisterZip(de.path().string().c_str());
	}
}

// ---------------------------------------------------------------------------
// RegisterZip
// ---------------------------------------------------------------------------
bool ZipFileSystem::RegisterZip(const char* zipPath, const ZipArchiveDecryptor* decryptor)
{
	auto* arc = GetOrOpen(zipPath, decryptor);
	if (!arc->Valid)
		return false;

	mz_uint count = mz_zip_reader_get_num_files(&arc->Handle);
	int      registered = 0;
	int      skipped = 0;

	for (mz_uint i = 0; i < count; ++i)
	{
		if (mz_zip_reader_is_file_a_directory(&arc->Handle, i)) continue;

		// After file-level decryption the zip itself is plain — individual
		// entries should never show the encrypted bit. Warn if they do
		// (indicates double-encryption or a malformed archive).
		if (mz_zip_reader_is_file_encrypted(&arc->Handle, i))
		{
			char n[512] = {};
			mz_zip_reader_get_filename(&arc->Handle, i, n, sizeof(n));
			Debug::Log("[ZipFS] WARNING: entry '%s' in '%s' is individually "
					   "encrypted — cannot extract (miniz limitation)\n", n, zipPath);
			++skipped;
			continue;
		}

		if (!mz_zip_reader_is_file_supported(&arc->Handle, i))
		{
			char n[512] = {};
			mz_zip_reader_get_filename(&arc->Handle, i, n, sizeof(n));
			Debug::Log("[ZipFS] WARNING: unsupported compression for '%s' in '%s'\n",
					   n, zipPath);
			++skipped;
			continue;
		}

		char raw[512] = {};
		mz_zip_reader_get_filename(&arc->Handle, i, raw, sizeof(raw));
		std::string key = Normalise(raw);
		if (key.empty()) continue;

		bool isOverride = (EntryMap.count(key) > 0);
		EntryMap[key] = ZipEntry { zipPath, key, i };
		if (isOverride)
			Debug::Log("[ZipFS] '%s' overridden by '%s'\n", key.c_str(), zipPath);

		++registered;
	}

	Debug::Log("[ZipFS] Registered '%s': %d files, %d skipped\n",
			   zipPath, registered, skipped);
	return registered > 0;
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------
void ZipFileSystem::Clear()
{
	EntryMap.clear();
	for (auto& arc : Archives)
	{
		if (arc->Valid)
			mz_zip_reader_end(&arc->Handle);
		// DecryptedBuf is a vector — freed automatically by unique_ptr.
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
std::unique_ptr<uint8_t[]> ZipFileSystem::Extract(const char* filename, size_t& outSize) const
{
	outSize = 0;

	auto it = EntryMap.find(Normalise(filename));
	if (it == EntryMap.end())
		return nullptr;

	const ZipEntry& entry = it->second;
	auto* arc = const_cast<ZipFileSystem*>(this)->GetOrOpen(entry.ArchivePath.c_str());

	if (!arc->Valid) {
		Debug::Log("[ZipFS] ERROR: archive '%s' is no longer valid during Extract\n",
				   entry.ArchivePath.c_str());
		return nullptr;
	}

	mz_zip_archive_file_stat stat {};
	if (!mz_zip_reader_file_stat(&arc->Handle, entry.EntryIndex, &stat)) {
		Debug::Log("[ZipFS] ERROR: mz_zip_reader_file_stat failed for '%s' in '%s': %s\n",
				   entry.EntryName.c_str(), entry.ArchivePath.c_str(),
				   mz_zip_get_error_string(mz_zip_get_last_error(&arc->Handle)));
		return nullptr;
	}

	outSize = static_cast<size_t>(stat.m_uncomp_size);
	auto buf = std::make_unique<uint8_t[]>(outSize);

	if (!mz_zip_reader_extract_to_mem(&arc->Handle, entry.EntryIndex,
		buf.get(), outSize, 0)) {
		// This path is now only reachable if the archive is corrupt on disk,
		// since encrypted/unsupported entries are already filtered in RegisterZip.
		Debug::Log("[ZipFS] ERROR: extraction failed for '%s' in '%s': %s\n",
				   entry.EntryName.c_str(), entry.ArchivePath.c_str(),
				   mz_zip_get_error_string(mz_zip_get_last_error(&arc->Handle)));
		outSize = 0;
		return nullptr;
	}

	return buf;
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
// GetOrOpen
// ---------------------------------------------------------------------------
ZipFileSystem::OpenArchive* ZipFileSystem::GetOrOpen(const char* zipPath,
	const ZipArchiveDecryptor* decryptor)
{
	// Cache lookup.
	for (auto& arc : Archives) {
		if (arc->Path == zipPath)
			return arc.get();
	}

	auto arc = std::make_unique<OpenArchive>();
	arc->Path = zipPath;

	// Resolve decryptor: explicit > default > none.
	const ZipArchiveDecryptor* dec = decryptor
		? decryptor
		: m_DefaultDecryptor.get();

	if (dec) {
		// --- Encrypted path: read whole file, decrypt into memory, init from mem ---
		Debug::Log("[ZipFS] Decrypting '%s' with %s\n", zipPath, dec->Name());

		// Read raw file.
		std::ifstream f(zipPath, std::ios::binary | std::ios::ate);

		if (!f.is_open()) {
			Debug::Log("[ZipFS] ERROR: cannot open '%s' for decryption\n", zipPath);
			arc->Valid = false;
			Archives.push_back(std::move(arc));
			return Archives.back().get();
		}

		const size_t fileSize = static_cast<size_t>(f.tellg());
		f.seekg(0);
		std::vector<uint8_t> raw(fileSize);
		f.read(reinterpret_cast<char*>(raw.data()), fileSize);
		f.close();

		// Decrypt into arc->DecryptedBuf.
		if (!dec->Decrypt(raw.data(), raw.size(), arc->DecryptedBuf)) {
			Debug::Log("[ZipFS] ERROR: decryption failed for '%s'\n", zipPath);
			arc->Valid = false;
			Archives.push_back(std::move(arc));
			return Archives.back().get();
		}

		// Init miniz from the decrypted memory buffer.
		arc->IsInMemory = true;
		arc->Valid = (mz_zip_reader_init_mem(
			&arc->Handle,
			arc->DecryptedBuf.data(),
			arc->DecryptedBuf.size(), 0) != MZ_FALSE);

		if (!arc->Valid)
			Debug::Log("[ZipFS] ERROR: decrypted '%s' is not a valid zip: %s\n",
					   zipPath,
					   mz_zip_get_error_string(mz_zip_get_last_error(&arc->Handle)));
	} else {
		// --- Plain path: open file directly ---
		arc->IsInMemory = false;
		arc->Valid = (mz_zip_reader_init_file(&arc->Handle, zipPath, 0) != MZ_FALSE);

		if (!arc->Valid)
			Debug::Log("[ZipFS] ERROR: failed to open '%s': %s\n",
					   zipPath,
					   mz_zip_get_error_string(mz_zip_get_last_error(&arc->Handle)));
	}

	Archives.push_back(std::move(arc));
	return Archives.back().get();
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
ZipBackedFileClass::ZipBackedFileClass(const char* filename)
	: PhobosRAMFileClass(filename,(void*)nullptr, 0 )   // name set, no buffer yet
{
	// Extract from zip into our owned buffer.
	m_Buffer = ZipFileSystem::Instance().Extract(filename, m_Size);
	this->SetManualBuffer(reinterpret_cast<char*>(m_Buffer.get()), static_cast<int>(m_Size));
}

// ---------------------------------------------------------------------------
// Open1 — read-only guard
// ---------------------------------------------------------------------------
bool ZipBackedFileClass::Open(PhobosFileAccessMode access)
{
	if (!Valid())
		return false;

	if (access == PhobosFileAccessMode::Write || access == PhobosFileAccessMode::ReadWrite)
		return false;   // zip buffers are immutable

	return PhobosRAMFileClass::Open(access);
}

#include <MixFileClass.h>
#include <map>
