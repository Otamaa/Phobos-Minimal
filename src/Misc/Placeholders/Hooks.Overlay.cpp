#include <Helpers/Macro.h>
#include <CCINIClass.h>
#include <ScenarioClass.h>
#include <OverlayClass.h>
#include <OverlayTypeClass.h>
#include <Straws.h>
#include <Pipes.h>
#include <IsometricTileTypeClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#ifndef ENABLE_TOMSOnOVERLAYWRAPPER
static int Isotile_LoadFile_Wrapper(IsometricTileTypeClass* pTile)
{
	bool available = false;
	int file_size = 0;

	{
		CCFileClass file(pTile->FileName);
		available = file.Exists();
		file_size = file.GetFileSize();
	}

	if (!available)
	{
		Debug::Log("ISOTILEDEBUG - Isometric Tile %s is missing!\n", pTile->FileName);
		return 0;
	}

	if (file_size == 0)
	{
		Debug::Log("ISOTILEDEBUG - Isometric Tile %s is a empty file!\n", pTile->FileName);
		return 0;
	}

	int read_size = pTile->LoadTile();

	if (pTile->Image == nullptr)
	{
		Debug::Log("ISOTILEDEBUG - Failed to load image for Isometric Tile %s!\n", pTile->FileName);
		return 0;
	}

	if (read_size != file_size)
	{
		Debug::Log("ISOTILEDEBUG - Isometric Tile %s file size %d doesn't match read size!\n", file_size, read_size, pTile->FileName);
	}

	return read_size;
}

DEFINE_HOOK_AGAIN(0x549E67, IsotileTypeClass_CheckTile_Replace, 0x5)
DEFINE_HOOK_AGAIN(0x549AF7, IsotileTypeClass_CheckTile_Replace, 0x5)
DEFINE_HOOK_AGAIN(0x546FCC, IsotileTypeClass_CheckTile_Replace, 0x5)
DEFINE_HOOK_AGAIN(0x544CC9, IsotileTypeClass_CheckTile_Replace, 0x5)
DEFINE_HOOK_AGAIN(0x544C97, IsotileTypeClass_CheckTile_Replace, 0x5)
DEFINE_HOOK(0x544C3F, IsotileTypeClass_CheckTile_Replace, 0x5)
{
	GET(IsometricTileTypeClass*, pThis, ESI);
	R->EAX(Isotile_LoadFile_Wrapper(pThis));
	return R->Origin() + 0x5;
}
#endif

struct OverlayByteReader
{
	OverlayByteReader(CCINIClass* pINI, const char* pSection)
		: ls { TRUE, 0x2000 }, bs { nullptr, 0 }
	{
		pBuffer = YRMemory::Allocate(512000);
		uuLength = pINI->ReadUUBlock(pSection, pBuffer, 512000);
		if (this->IsAvailable())
		{
			bs.Buffer.Buffer = pBuffer;
			bs.Buffer.Size = uuLength;
			bs.Buffer.Allocated = false;
			ls.Get_From(bs);
		}
	}

	~OverlayByteReader()
	{
		YRMemory::Deallocate(pBuffer);
	}

	bool IsAvailable() const { return uuLength > 0; }

	unsigned char Get()
	{
		if (IsAvailable())
		{
			unsigned char ret;
			ls.Get(&ret, sizeof(ret));
			return ret;
		}
		return 0;
	}

	size_t uuLength;
	void* pBuffer;
	LCWStraw ls;
	BufferStraw bs;
};

struct OverlayReader
{
	size_t Get() {
		unsigned char ret[4];

		ret[0] = ByteReaders[0].Get();
		ret[1] = ByteReaders[1].Get();
		ret[2] = ByteReaders[2].Get();
		ret[3] = ByteReaders[3].Get();

		return ret[0] == 0xFF ? 0xFFFFFFFF : (ret[0] | (ret[1] << 8) | (ret[2] << 16) | (ret[3] << 24));
	}

	OverlayReader(CCINIClass* pINI)
		:ByteReaders { {pINI, GameStrings::OverlayPack() }, { pINI,"OverlayPack2" }, { pINI,"OverlayPack3" }, { pINI,"OverlayPack4" }, }
	{ }

	~OverlayReader() = default;

private:
	OverlayByteReader ByteReaders[4];
};

struct OverlayByteWriter
{
	OverlayByteWriter(const char* pSection, size_t nBufferLength)
		: lpSectionName { pSection }, uuLength { 0 }, Buffer { nullptr }, bp { nullptr, 0 }, lp { FALSE,0x2000 } {
		this->Buffer = YRMemory::Allocate(nBufferLength);
		bp.Buffer.Buffer = this->Buffer;
		bp.Buffer.Size = nBufferLength;
		bp.Buffer.Allocated = false;
		lp.Put_To(bp);
	}

	~OverlayByteWriter() {
		YRMemory::Deallocate(this->Buffer);
	}

	void Put(unsigned char data) {
		uuLength += lp.Put(&data, 1);
	}

	void PutBlock(CCINIClass* pINI) {
		pINI->Clear(this->lpSectionName, nullptr);
		pINI->WriteUUBlock(this->lpSectionName, this->Buffer, uuLength);
	}

	const char* lpSectionName;
	size_t uuLength;
	void* Buffer;
	BufferPipe bp;
	LCWPipe lp;
};

struct OverlayWriter
{
	OverlayWriter(size_t nLen)
		: ByteWriters { { GameStrings::OverlayPack(), nLen}, { "OverlayPack2", nLen }, { "OverlayPack3", nLen }, { "OverlayPack4", nLen } }
	{ }

	~OverlayWriter() = default;

	void Put(int nOverlay)
	{
		unsigned char bytes[4];
		bytes[0] = (nOverlay & 0xFF);
		bytes[1] = ((nOverlay >> 8) & 0xFF);
		bytes[2] = ((nOverlay >> 16) & 0xFF);
		bytes[3] = ((nOverlay >> 24) & 0xFF);
		ByteWriters[0].Put(bytes[0]);
		ByteWriters[1].Put(bytes[1]);
		ByteWriters[2].Put(bytes[2]);
		ByteWriters[3].Put(bytes[3]);
	}

	void PutBlock(CCINIClass* pINI)
	{
		ByteWriters[0].PutBlock(pINI);
		ByteWriters[1].PutBlock(pINI);
		ByteWriters[2].PutBlock(pINI);
		ByteWriters[3].PutBlock(pINI);
	}

private:
	OverlayByteWriter ByteWriters[4];
};

DEFINE_HOOK(0x5FD2E0, OverlayClass_ReadINI, 0x7)
{
	GET(CCINIClass*, pINI, ECX);

	pINI->CurrentSectionName = nullptr;
	pINI->CurrentSection = nullptr;

	if (ScenarioClass::NewINIFormat > 1)
	{
		OverlayReader reader(pINI);

		for (short i = 0; i < 0x200; ++i)
		{
			for (short j = 0; j < 0x200; ++j)
			{
				CellStruct mapCoord { j,i };
				size_t nOvl = reader.Get();
				if (nOvl != 0xFFFFFFFF)
				{
					auto const pType = OverlayTypeClass::Array->GetItem(nOvl);
					if (pType->GetImage() || pType->CellAnim)
					{
						if (SessionClass::Instance->GameMode != GameMode::Campaign && pType->Crate)
							continue;
						if (!MapClass::Instance->CoordinatesLegal(mapCoord))
							continue;

						auto pCell = MapClass::Instance->GetCellAt(mapCoord);
						auto const nOriginOvlData = pCell->OverlayData;
						GameCreate<OverlayClass>(pType, mapCoord, -1);
						if (nOvl == 24 || nOvl == 25 || nOvl == 237 || nOvl == 238) // bridges
							pCell->OverlayData = nOriginOvlData;
					}
				}
			}
		}

		auto pBuffer = YRMemory::Allocate(256000);
		size_t uuLength = pINI->ReadUUBlock(GameStrings::OverlayDataPack(), pBuffer, 256000);
		if (uuLength > 0)
		{
			BufferStraw bs(pBuffer, uuLength);
			LCWStraw ls(TRUE, 0x2000);
			ls.Get_From(bs);

			for (short i = 0; i < 0x200; ++i)
			{
				for (short j = 0; j < 0x200; ++j)
				{
					CellStruct mapCoord { j,i };
					unsigned char buffer;
					ls.Get(&buffer, sizeof(buffer));
					if (MapClass::Instance->CoordinatesLegal(mapCoord))
					{
						auto pCell = MapClass::Instance->GetCellAt(mapCoord);
						pCell->OverlayData = buffer;
					}
				}
			}
		}
		YRMemory::Deallocate(pBuffer);
	}

	AbstractClass::RemoveAllInactive();

	return 0x5FD69A;
}

DEFINE_HOOK(0x5FD6A0, OverlayClass_WriteINI, 0x6)
{
	GET(CCINIClass*, pINI, ECX);

	pINI->Clear(GameStrings::OVERLAY(), nullptr);

	size_t len = DSurface::Alternate->Width * DSurface::Alternate->Height;
	OverlayWriter writer(len);
	OverlayByteWriter datawriter(GameStrings::OverlayDataPack(), len);

	for (short i = 0; i < 0x200; ++i)
	{
		for (short j = 0; j < 0x200; ++j)
		{
			CellStruct mapCoord { j,i };
			auto const pCell = MapClass::Instance->GetCellAt(mapCoord);
			writer.Put(pCell->OverlayTypeIndex);
			datawriter.Put(pCell->OverlayData);
		}
	}

	writer.PutBlock(pINI);
	datawriter.PutBlock(pINI);

	return 0x5FD8EB;
}