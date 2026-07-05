#include "MixFileClass.h"
#include <CRC.h>
#include <CRT.h>
#include <CD.h>
#include <Exit.h>
#include <Utilities/Debug.h>
#include <Straws.h>
#include <Random3Class.h>
#include <CCFileClass.h>
#include <Misc/BlowfishEngine.h>

#pragma region MixCache

uint32_t __fastcall LoadedFileCache::CacheFile(char* filename)
{
	char a2[260];
	CRT::strcpy(a2, filename);
	CRT::strupr(a2);

	int result = SafeChecksummer()(a2, strlen(a2));
	auto pList = LoadedFileCache::Head();

	while (pList)
	{
		int v3 = pList->CRC;
		bool v4 = v3 < result;

		if (v3 == result) {
			if (pList->FilePtr) {
				pList->FilePtr = 0;
				return result;
			}
			v4 = v3 < result;
		}

		if (v4) {
			pList = pList->Next;
		} else {
			pList = pList->Prev;
		}
	}

	return result;
}

void __fastcall LoadedFileCache::Destroy()
{
	auto pList = LoadedFileCache::Head();
	if (LoadedFileCache::Head()) {
		if (LoadedFileCache::Head->Prev) {
			Delete(LoadedFileCache::Head->Prev);
		}

		if (auto v1 = pList->Next) {
			Delete(v1);
		}

		YRMemory::free(pList);
	}

	LoadedFileCache::Head = nullptr;
}

#pragma endregion

#pragma region MixFileClass

bool MixFileClass::Contains(const char* pName)
{
	char filenameUpper[260];
	CRT::strcpy(filenameUpper, pName);
	CRT::strupr(filenameUpper);
	auto crcName = SafeChecksummer()(filenameUpper, strlen(filenameUpper));

	for (int i = 0; i < this->Count; ++i) {
		if (this->Headers[i].ID == crcName) {
			return true;
		}
	}

	return false;
}

void MixFileClass::DumpAllEntries() {
	for (auto mix = MixFileClass::MIXes->First(); mix; mix = mix->Next()) {
		if(mix->IsValid()){
			Debug::LogInfo("=== MIX: {} | Count={} | DataStart={:#x} | DataSize={:#x} ===",
				mix->Filename, mix->Count, mix->DataStart, mix->DataSize);

			for (int i = 0; i < mix->Count; ++i) {
				const auto& h = mix->Headers[i];
				const bool outOfBounds = (h.Offset + h.Size) > static_cast<DWORD>(mix->DataSize);
				Debug::LogInfo("  [{:04d}] CRC={:#010x} Offset={:#010x} Size={:#010x}{}",
					i, h.ID, h.Offset, h.Size,
					outOfBounds ? " *** OUT OF BOUNDS ***" : "");
			}
		}
	}
}

bool __fastcall MixFileClass::Free(const char* pFilename)
{
	char fname[256];
	char ext[256];
	char rebuilt[260];

	MixFileClass* mix = MixFileClass::MIXes->First();

	// Walk the list looking for a matching filename (compared by name+ext only,
	// ignoring drive/directory components).
	for (;; mix = mix->Next())
	{
		if (!mix || !mix->NextNode || !mix->PrevNode)
			return 0; // not found / reached sentinel node

		_splitpath(mix->Filename, nullptr, nullptr, fname, ext);
		_makepath(rebuilt, nullptr, nullptr, fname, ext);

		if (!_strcmpi(rebuilt, pFilename))
			break; // found match
	}

	mix->Free();
	return 1;
}

// WW-safe Straw scope guard — resets vtable and calls manual teardown
// in the correct sequence regardless of how we exit (return, exception, etc.)
struct PKStrawGuard
{
	PKStraw* pPK;
	FileStraw* pFStraw;

	PKStrawGuard(CCFileClass* pFile , PKStraw::CodeControl decode , Random3Straw* pRand)
		: pPK(GameCreate<PKStraw>(decode, pRand)) , pFStraw(GameCreate<FileStraw>(pFile))
	{ }
	
	~PKStrawGuard()
	{
		// Explicitly unlink chain before any destruction
		 // This prevents Straw::~Straw from walking into freed/invalid memory
		pPK->Get_From(nullptr);      // unlink pk → fstraw
		pFStraw->Get_From(nullptr);  // unlink fstraw → whatever

		pPK->PKStraw::~PKStraw();
		YRMemory::free(pPK);

		pFStraw->FileStraw::~FileStraw();
		YRMemory::free(pFStraw);
	}
};

// MixFileClass::ReadFromCCFIleWithoutStraws
// Reads and parses a MIX file header from an already-opened CCFileClass.
//
// Decrypt pipeline (encrypted MIX only):
//   [File] -> [RSA decrypt session key] -> [BlowfishEngine] -> [plaintext header]
//
// Uses PhobosBlowStraw + BlowfishEngine directly — avoids the WW PKStraw/BlowStraw
// chain entirely, which had fragile manual vtable/lifetime requirements.
void MixFileClass::ReadFromCCFIleWithoutStraws(CCFileClass* pFile, PKey* pKey)
{
	this->Filename = _strdup(pFile->Filename);

	// Exact mirror of FileStraw::Get — same open/available checks
	auto StrawGet = [&](void* buffer, int length) -> int
		{
			if (!pFile || !buffer || length <= 0)
				return 0;

			if (!pFile->IsOpen())
			{
				if (!pFile->IsAvaible(false) || !pFile->Open1(FileAccessMode::Read))
					return 0;
			}

			return pFile->Read(buffer, length);
		};

	MixPeekHeader peekHeader {};
	StrawGet(&peekHeader, sizeof(MixPeekHeader));

	MixFileHeader fileHeader {};

	if (!peekHeader.FileCount)
	{
		this->IsDigest = (peekHeader.FormatFlags & 1) != 0;
		this->IsEncrypted = (peekHeader.FormatFlags & 2) != 0;

		if (this->IsEncrypted)
		{
			// Mirror of PKStraw::Get DECODE path:
			// 1. read encrypted key block
			// 2. PKey::Decrypt -> session key
			// 3. BlowStraw::Key(session key, 56)
			// 4. all further reads go through BF decrypt
			const int bytesPerBlock = (pKey->GetBitPrecision() - 1) / 8;
			const int blocksNeeded = 55 / bytesPerBlock + 1;
			const int encKeySize = (bytesPerBlock + 1) * blocksNeeded;
			const int plainKeySize = bytesPerBlock * blocksNeeded;

			std::vector<uint8_t> encKey(encKeySize, 0);
			StrawGet(encKey.data(), encKeySize);

			std::vector<uint8_t> plainKey(plainKeySize, 0);
			pKey->Decrypt(encKey.data(), encKeySize, plainKey.data());

			BlowfishEngine bf;
			bf.Submit_Key(plainKey.data(), 56);

			// Mirror of pStrawUsed->Get(&fileHeader, sizeof(MixFileHeader))
			// BF stream read — 8 byte blocks, MixFileHeader = 6 bytes
			constexpr int BF_BLOCK = 8;

			uint8_t encFirst[BF_BLOCK] {};
			uint8_t plainFirst[BF_BLOCK] {};
			StrawGet(encFirst, BF_BLOCK);
			bf.Decrypt(encFirst, BF_BLOCK, plainFirst);
			std::memcpy(&fileHeader, plainFirst, sizeof(MixFileHeader));

			this->Count = fileHeader.Count;
			this->DataSize = fileHeader.DataSize;

			if (this->Count <= 0)
				return;

			this->Headers = static_cast<MixHeaderData*>(
				YRMemory::Allocate(sizeof(MixHeaderData) * this->Count));
			if (!this->Headers)
				return;

			// Mirror of pStrawUsed->Get(this->Headers, sizeof(MixHeaderData) * this->Count)
			// Carry 2 bytes from first decrypted block
			constexpr int CARRY = BF_BLOCK - static_cast<int>(sizeof(MixFileHeader));
			const int indexSize = static_cast<int>(sizeof(MixHeaderData) * this->Count);
			const int remaining = indexSize - CARRY;

			uint8_t* dst = reinterpret_cast<uint8_t*>(this->Headers);
			std::memcpy(dst, plainFirst + sizeof(MixFileHeader), CARRY);
			dst += CARRY;

			if (remaining > 0)
			{
				const int remBlocks = (remaining + BF_BLOCK - 1) / BF_BLOCK;
				const int remReadSize = remBlocks * BF_BLOCK;
				std::vector<uint8_t> encRem(remReadSize, 0);
				std::vector<uint8_t> plainRem(remReadSize, 0);
				StrawGet(encRem.data(), remReadSize);
				bf.Decrypt(encRem.data(), remReadSize, plainRem.data());
				std::memcpy(dst, plainRem.data(), remaining);
			}
		}
		else
		{
			// Mirror of pStrawUsed->Get(&fileHeader, sizeof(MixFileHeader))
			StrawGet(&fileHeader, sizeof(MixFileHeader));

			this->Count = fileHeader.Count;
			this->DataSize = fileHeader.DataSize;

			if (this->Count <= 0)
				return;

			this->Headers = static_cast<MixHeaderData*>(
				YRMemory::Allocate(sizeof(MixHeaderData) * this->Count));
			if (!this->Headers)
				return;

			// Mirror of pStrawUsed->Get(this->Headers, sizeof(MixHeaderData) * this->Count)
			StrawGet(this->Headers, sizeof(MixHeaderData) * this->Count);
		}
	}
	else
	{
		// Mirror of: FileHeader = peekHeader
		fileHeader = peekHeader;

		// Mirror of: pStrawUsed->Get(&FileHeader.DataSizeHi, sizeof(short))
		StrawGet(&fileHeader.DataSizeHi, sizeof(short));

		this->Count = fileHeader.Count;
		this->DataSize = fileHeader.DataSize;

		if (this->Count <= 0)
			return;

		this->Headers = static_cast<MixHeaderData*>(
			YRMemory::Allocate(sizeof(MixHeaderData) * this->Count));
		if (!this->Headers)
			return;

		// Mirror of: pStrawUsed->Get(this->Headers, sizeof(MixHeaderData) * this->Count)
		StrawGet(this->Headers, sizeof(MixHeaderData) * this->Count);
	}

	const int seekres = pFile->Seek(0, FileSeekMode::Current);
	Debug::LogInfo("[MixFile] {} BiasStart={:#x} Seek={:#x} DataStart={:#x}",
		this->Filename,
		pFile->BiasStart,
		seekres,
		pFile->BiasStart + seekres);

	this->DataStart = pFile->BiasStart + seekres;
	MixFileClass::MIXes->AddTail(this);
}

// this one using original game patten but suffer from bug where it will crash when 
// the straws dtors it weird interaction between them that causing me written the 
// alternate version that more easier to digest and mantain than this original code
// but put it here for educational purposes , use it at your own risk
void MixFileClass::ReadFromCCFile(CCFileClass * pFile, PKey* pKey)
{
	this->Filename = _strdup(pFile->Filename);

	PKStrawGuard _Guard(pFile, PKStraw::CodeControl::DECODE, Random3Straw::Instance.operator->());
	Debug::LogInfo("pFStraw ChainTo={:p} ChainFrom={:p}",
	(void*)_Guard.pFStraw->ChainTo,
	(void*)_Guard.pFStraw->ChainFrom);
	Debug::LogInfo("pPK ChainTo={:p} ChainFrom={:p} BF.ChainTo={:p}",
		(void*)_Guard.pPK->ChainTo,
		(void*)_Guard.pPK->ChainFrom,
		(void*)_Guard.pPK->GetBF()->ChainTo);

	MixPeekHeader  peekHeader {};
	_Guard.pFStraw->Get(&peekHeader, sizeof(MixPeekHeader));
	MixFileHeader  FileHeader {};

	Straw* pStrawUsed = _Guard.pFStraw;

	if (!peekHeader.FileCount) {
		// Flagged format
		this->IsDigest = (peekHeader.FormatFlags & 1) != 0;
		this->IsEncrypted = (peekHeader.FormatFlags & 2) != 0;

		//decrypt the header if necessary
		if (this->IsEncrypted) {
			_Guard.pPK->Key((PKey*)pKey);
			_Guard.pPK->Get_From(_Guard.pFStraw);
			pStrawUsed = _Guard.pPK;
		}

		pStrawUsed->Get(&FileHeader, sizeof(MixFileHeader));

	} else {
		FileHeader = peekHeader;
		pStrawUsed->Get(&FileHeader.DataSizeHi, sizeof(short));
	}

	this->Count = FileHeader.Count;
	this->DataSize = FileHeader.DataSize;

	this->Headers = (MixHeaderData*)YRMemory::Allocate(sizeof(MixHeaderData) * this->Count);

	if (this->Headers) {
		pStrawUsed->Get(this->Headers, sizeof(MixHeaderData) * this->Count);

		// DataStart = bias origin + current seek position (end of header)
		const int seekres = pFile->Seek(0, FileSeekMode::Current);
		this->DataStart = pFile->BiasStart + seekres;

		// Add_Tail — link this node at the tail of MixFileClass::List
		MixFileClass::MIXes->AddTail(this);
	}
}

//0x5B4430
bool __fastcall MixFileClass::Offset(const char* filename, void** realptr, MixFileClass** mixfile, int* offset, int* size)
{
	if (!filename)
		return false;

	char filenameUpper[260];
	CRT::strcpy(filenameUpper, filename);
	CRT::strupr(filenameUpper);

	const int hash = SafeChecksummer()(filenameUpper, strlen(filenameUpper));

	MixFileClass* ptr = MixFileClass::MIXes->First();
	MixHeaderData* foundBlock = nullptr;

	// Outer loop: scan each mixfile in the list.
	while (true)
	{
		if (!ptr || !ptr->NextNode || !ptr->PrevNode)
			return false; // reached list sentinel / not found anywhere

		// --- Inner "binary search" over ptr->HeaderBuffer[0..Count) ---
		// VERIFY: v7 here is a shrinking remaining-count, not a hi/lo bound pair.
		int remaining = ptr->Count;
		MixHeaderData* base = ptr->Headers;
		MixHeaderData* block = nullptr;
		bool advanceToNextMixfile = false;

		if (remaining <= 0)
		{
			advanceToNextMixfile = true;
		}
		else
		{
			while (true)
			{
				block = &base[remaining / 2];
				const int midCRC = block->ID;

				if (midCRC <= hash)
				{
					// Found a candidate <= hash — exit inner search loop to check equality.
					if (midCRC != hash)
					{
						// Not an exact match — narrow search to the upper half.
						base = block + 1;
						remaining += -1 - remaining / 2;

						if (remaining <= 0)
						{
							advanceToNextMixfile = true;
						}
						else
						{
							continue; // keep searching within this mixfile
						}
					}
					// else: exact match found, midCRC == hash — fall through with `block` set.
					break;
				}

				// midCRC > hash — narrow search to the lower half.
				remaining /= 2;
				if (remaining <= 0)
				{
					advanceToNextMixfile = true;
					break;
				}
				// else continue loop with smaller `remaining`, same `base`.
			}
		}

		if (advanceToNextMixfile)
		{
			ptr = ptr->Next();
			continue;
		}

		// block->CRC == hash here — found this filename's entry in this mixfile.
		foundBlock = block;
		break;
	}

	// ------------------------------------------------------------------
	// Populate output parameters
	// ------------------------------------------------------------------
	if (mixfile)
		*mixfile = ptr;

	if (size)
		*size = foundBlock->Size;

	if (realptr)
		*realptr = nullptr;

	if (offset)
		*offset = foundBlock->Offset;

	if (realptr && ptr->Data)
		*realptr = (char*)ptr->Data + foundBlock->Offset;

	// If the mixfile's data isn't loaded in RAM, offset is relative to
	// DataStart within the underlying file instead of an in-memory pointer.
	if (!ptr->Data && offset)
		*offset += ptr->DataStart;

	return true;
}

MixFileClass::MixFileClass(const char* filename, PKey* pKey) :
	Node<MixFileClass>(),
	Filename(nullptr),
	IsDigest(false),
	IsEncrypted(false),
	IsAllocated(false),
	Count(0),
	DataSize(0),
	DataStart(0),
	Headers(nullptr),
	Data(nullptr)
{
	CD cdd;
	if (!cdd.ForceAvailable(CD::Disk())) {
		Debug::FreeMouse();
		Debug::ExitGame(1);
	}

	CCFileClass file(filename);

	if (file.IsAvaible(false)) {
		this->ReadFromCCFIleWithoutStraws(&file, pKey);
	}
}

MixFileClass::MixFileClass(CCFileClass* pFile, PKey* pKey) :
	Node<MixFileClass>(),
	Filename(nullptr),
	IsDigest(false),
	IsEncrypted(false),
	IsAllocated(false),
	Count(0),
	DataSize(0),
	DataStart(0),
	Headers(nullptr),
	Data(nullptr) {
		CD cdd;
		if (!cdd.ForceAvailable(CD::Disk())) {
			Debug::FreeMouse();
			Debug::ExitGame(1);
		}

		this->ReadFromCCFIleWithoutStraws(pFile, pKey);
	}
#pragma endregion