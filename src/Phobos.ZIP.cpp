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


//completely replacing the old way loaded file are linked within
//the linked list with map for better maintaiability and readbility
//the map has no ownership of the resource i suppoe 
//it only map it to the appropriate place 
//not sure how the game know that the file linked here is already invalid 
//safety issues
std::unordered_map<std::string, void*> g_GlobalFileLinks {};

void* __fastcall FakeFileLoader::_Retrieve(const char* name, bool forceShapeCache)
{
#ifdef ReplaceImpl

	if (!name)
		return nullptr;

	// Uppercase copy — MIX lookups and zip keys are always uppercase
	char upperName[260];
	strcpy(upperName, name);
	_strupr(upperName);

	auto find = g_GlobalFileLinks.find(upperName);

	// Load from any FileClass — raw alloc or ShapeCache depending on type
	auto LoadFrom = [&](FileClass& file) -> void* {
		if (forceShapeCache || strstr(upperName, ".SHP")) {
			return (void*)GameCreate<SHPReference>(name) ;
		}

		return CCFileClass::Load_Alloc_Data(file);
	};


	// ------------------------------------------------------------------
	// BST cache — already-loaded data (MIX or zip from prior call)
	//     Zero allocation, zero I/O on hit.
	// ------------------------------------------------------------------
	bool alreadyExist = false;
	if (find != g_GlobalFileLinks.end()) {
		if(find->second)
			return find->second;

		alreadyExist = true;
	}


	// ------------------------------------------------------------------
	// ZipFileSystem — highest priority
	//     Checked before BST so art/ini overrides beat already-cached vanilla.
	//     Non-.SHP files (INI, PAL, CSF, audio, map files) work fully.
	//     .SHP files fall through to vanilla path — see note in LoadFrom.
	// ------------------------------------------------------------------
	//if (ZipFileSystem::Instance().Contains(upperName)) {
	//	// .SHP needs ShapeCache ctor which re-opens by name — skip zip for now
	//	const bool isSHP = forceShapeCache || strstr(upperName, ".SHP") != nullptr;
	//	if (!isSHP) {
	//		void* fileData = nullptr;
	//
	//		//find first zip file that have this file and extract it
	//		ZipFileSystem::Instance().ForEach([crc, &fileData](const ZipEntry& entry) {
	//		 if (SafeChecksummer()(entry.EntryName.c_str(), entry.EntryName.size()) == crc) {
	//				 ZipBackedFileClass zipFile(entry.EntryName.c_str());
	//				 if (zipFile.Valid()) {
	//					 Debug::LogInfo("[ZipFS] Retrieving file {}' from archive'{}'", entry.EntryName.c_str(), entry.EntryName.size());
	//					 fileData = CCFileClass::Load_Alloc_Data(zipFile);
	//					 return; //only if valid
	//				 }
	//			}
	//		});
	//
	//		if (fileData)
	//			return CacheAndReturn(fileData);
	//
	//		// Extraction failed despite Contains() — warn, fall through to [2]/[3]
	//		Debug::LogInfo("[ZipFS] Extract failed for '{}', falling back", upperName);
	//	}
	//	// .SHP: intentional fall-through to [2]/[3] — vanilla ShapeCache handles it
	//}

	// ------------------------------------------------------------------
	// Disk / vanilla MIX fallback — lowest priority
	//     CCFileClass transparently resolves through MixFileClass::MIXes.
	//     Result cached into BST so next call hits [2].
	// ------------------------------------------------------------------
	CCFileClass file(name);
	if (!file.IsAvaible(false))
		return nullptr;

	auto pPtr = LoadFrom(file);

	if (alreadyExist)
		find->second = pPtr;
	else
		g_GlobalFileLinks.emplace(upperName, pPtr);

	return pPtr;
#endif

	return Retrieve(name, forceShapeCache);
}

//DEFINE_FUNCTION_JUMP(LJMP, 0x5B40B0, FakeFileLoader::_Retrieve)
DEFINE_FUNCTION_JUMP(CALL, 0x41CAF7, FakeFileLoader::_Retrieve)
DEFINE_FUNCTION_JUMP(CALL, 0x41CB08, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4279DA, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x427A04, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x427B15, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x427BE9, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x427C07, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x42891E, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4309FD, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x430A61, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45E904, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45E988, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45E999, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45EA16, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45EA79, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F28D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F2A5, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F525, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F543, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F615, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F6E9, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F7C1, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F82D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F84B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F91D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45FA0B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45FA39, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45FA6B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x47EFFD, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x47F00E, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x47F26A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4A38DE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4A3985, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4A8862, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4A8873, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6D07, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6D1A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6D2D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6E52, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6F41, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B7349, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B73A8, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x51916D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5194FF, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BBE3, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BC55, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BCFD, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BE6D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BF26, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BFDA, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52C08E, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52C142, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x531381, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x534C04, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x534CC3, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x546725, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5468DB, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5468F8, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5469BF, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x560D7B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x561093, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5D2EBA, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F76EE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F773C, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F778A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F77D8, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9249, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9267, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9281, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9685, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9931, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE68C, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE6AE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE6F6, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE714, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE928, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FEBEC, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x62769F, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x66C5F4, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x66C606, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x677FAC, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x677FBE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x690660, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6906BD, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x69071A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x690A4D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x690B1D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6A5012, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6A8167, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6ABD57, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6ABD68, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6ABD79, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6B1B94, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6B1BCE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6B57C2, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6CE89B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6CE8B1, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6CEE20, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6CEE38, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6DAE07, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x715820, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x715A38, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x715A54, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x715B05, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x716C77, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x716D04, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x716D1A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x716D6F, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x71DFBB, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x73CEE0, FakeFileLoader::_Retrieve); //OREGATH.SHP
DEFINE_FUNCTION_JUMP(CALL, 0x73D3EF, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x747490, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x7474A1, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x747BB4, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x748093, FakeFileLoader::_Retrieve);

void __fastcall _Cache_File(char* filename)
{
	// ????
	if(!filename)
		return;

	char upperName[260];
	strcpy(upperName, filename);
	_strupr(upperName);
	g_GlobalFileLinks[upperName] = nullptr;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x5B4270, _Cache_File)


void __fastcall _Destroy_Cache_()
{
	g_GlobalFileLinks.clear();
}

DEFINE_FUNCTION_JUMP(LJMP, 0x5B4310, _Cache_File)