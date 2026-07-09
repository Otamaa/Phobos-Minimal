#include "Phobos.MIXFile.h"

#include <algorithm>
#include <ranges>

#include <CRC.h>
#include <CCFileClass.h>

#include <Misc/BlowfishEngine.h>

std::vector<PhobosMixFileClass*> PhobosMixFileClass::Array;

PhobosMixHeaderData* PhobosMixFileClass::BinarySearchHeaders(
	std::vector<PhobosMixHeaderData>* headers,
	int                  crc)
{
	if (headers->empty())
		return nullptr;

	PhobosMixHeaderData* base = headers->data();
	int remaining = int(headers->size());

	while (remaining > 0) {
		PhobosMixHeaderData* mid = base + (remaining / 2);

		if ((int)mid->ID == crc)
			return mid;

		if ((int)mid->ID < crc) {
			// Narrow to upper half.
			base = mid + 1;
			remaining = remaining - 1 - (remaining / 2);
		} else {
			// Narrow to lower half.
			remaining /= 2;
		}
	}

	return nullptr;
}

bool PhobosMixFileClass::Offset(const char* filename, void** realptr, PhobosMixFileClass** mixfile, int* offset, int* size)
{
	if (!filename)
		return false;

	std::string filenamecpy = filename;
	std::transform(filenamecpy.begin(), filenamecpy.end(), filenamecpy.begin(), ::toupper);
	int hash = SafeChecksummer()(filenamecpy.c_str(), filenamecpy.size());

	PhobosMixHeaderData* foundBlock = nullptr;

	for (auto& mix : PhobosMixFileClass::Array) {
		PhobosMixHeaderData* block = BinarySearchHeaders(&mix->Headers,hash);
		if (!block) {
			continue;
		}

		if (mixfile)
			*mixfile = mix;

		if (size)
			*size = block->Size;

		if (realptr)
			*realptr = nullptr;

		if (offset)
			*offset = foundBlock->Offset;

		if (realptr && mix->Data)
			*realptr = (char*)mix->Data + foundBlock->Offset;

		if (!mix->Data && offset)
			*offset += mix->DataStart;

		return true;
	}

	return false;
}

PhobosMixFileClass::PhobosMixFileClass(RawFileClass* pFile, PKey* pKey) :
	Filename(),
	IsDigest(false),
	IsEncrypted(false),
	IsAllocated(false),
	DataSize(0),
	DataStart(0),
	Headers(),
	Data(nullptr)
{
	this->Filename = _strdup(pFile->Filename);

	// Exact mirror of FileStraw::Get — same open/available checks
	auto StrawGet = [&](void* buffer, int length) -> int {
		if (!pFile || !buffer || length <= 0)
			return 0;

		if (!pFile->IsOpen()) {
				if (!pFile->IsAvaible(false) || !pFile->Open1(FileAccessMode::Read))
					return 0;
		}

			return pFile->Read(buffer, length);
		};

	PhobosMixPeekHeader peekHeader {};
	StrawGet(&peekHeader, sizeof(PhobosMixPeekHeader));
	PhobosMixFileHeader fileHeader {};

	if (!peekHeader.FileCount) {
		this->IsDigest = (peekHeader.FormatFlags & 1) != 0;
		this->IsEncrypted = (peekHeader.FormatFlags & 2) != 0;

		if (this->IsEncrypted) {
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
			std::memcpy(&fileHeader, plainFirst, sizeof(PhobosMixFileHeader));

			this->Headers.resize(fileHeader.Count);
			this->DataSize = fileHeader.DataSize;

			if(!this->Headers.empty()){
				// Mirror of pStrawUsed->Get(this->Headers, sizeof(MixHeaderData) * this->Count)
				// Carry 2 bytes from first decrypted block
				constexpr int CARRY = BF_BLOCK - static_cast<int>(sizeof(PhobosMixFileHeader));
				const int indexSize = static_cast<int>(sizeof(PhobosMixHeaderData) * fileHeader.Count);
				const int remaining = indexSize - CARRY;

				uint8_t* dst = reinterpret_cast<uint8_t*>(this->Headers.data());
				std::memcpy(dst, plainFirst + sizeof(PhobosMixFileHeader), CARRY);
				dst += CARRY;

				if (remaining > 0) {
					const int remBlocks = (remaining + BF_BLOCK - 1) / BF_BLOCK;
					const int remReadSize = remBlocks * BF_BLOCK;
					std::vector<uint8_t> encRem(remReadSize, 0);
					std::vector<uint8_t> plainRem(remReadSize, 0);
					StrawGet(encRem.data(), remReadSize);
					bf.Decrypt(encRem.data(), remReadSize, plainRem.data());
					std::memcpy(dst, plainRem.data(), remaining);
				}
			}

		} else {
			StrawGet(&fileHeader, sizeof(PhobosMixFileHeader));

			this->Headers.resize(fileHeader.Count);
			this->DataSize = fileHeader.DataSize;

			if (!this->Headers.empty()) {
				StrawGet(this->Headers.data(), sizeof(PhobosMixHeaderData) * fileHeader.Count);
			}
		}
	} else {
		fileHeader = peekHeader;
		StrawGet(&fileHeader.DataSizeHi, sizeof(short));

		this->Headers.resize(fileHeader.Count);
		this->DataSize = fileHeader.DataSize;

		if (!this->Headers.empty()) {
			StrawGet(this->Headers.data(), sizeof(PhobosMixHeaderData)* fileHeader.Count);
		}
	}

	const int seekres = pFile->Seek(0, FileSeekMode::Current);
	this->DataStart = pFile->BiasStart + seekres;
	Array.emplace_back(this);
}

PhobosMixFileClass::~PhobosMixFileClass() {
	auto iter = std::ranges::find(Array, this);

	if (iter != Array.end())
		Array.erase(iter);
};