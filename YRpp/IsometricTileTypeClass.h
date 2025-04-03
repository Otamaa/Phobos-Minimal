#pragma once

#include <ObjectTypeClass.h>
#include <Drawing.h>
#include <FileFormats/IsoTileImageStruct.h>
#include <CCINIClass.h>

struct TileInsert
{
	int Next;
	int Previous;
};

//static_assert(sizeof(TileInsert) == 0x8);

class DECLSPEC_UUID("5AF2CE7A-0634-11D2-ACA4-006008055BB5")
	NOVTABLE IsometricTileTypeClass : public ObjectTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::IsotileType;

	//Array
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<IsometricTileTypeClass*>, 0xA8ED28u> const Array {};

	IMPL_Find(IsometricTileTypeClass)

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x544CE0);
	}

	//Array
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<LightConvertClass*>, 0x87F698u> const TileDrawers{};
	static COMPILETIMEEVAL reference<SHPStruct*, 0xAA1060u, 0x4u> const SlopeZshape {};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override R0;

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) override RX;
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;
	virtual void ComputeCRC(CRCEngine& checksum) const override RX;

	virtual int GetArrayIndex() const R0;

	//ObjectTypeClass
	virtual CoordStruct* vt_entry_6C(CoordStruct* pDest, CoordStruct* pSrc) const override  R0;
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) override  R0;
	virtual ObjectClass* CreateObject(HouseClass* pOwner) override  R0;
	virtual CellStruct* GetFoundationData(bool IncludeBib) const override  R0;

	virtual SHPStruct* GetImage() const override  R0;

	//Destructor
	virtual ~IsometricTileTypeClass() override JMP_THIS(0x54A170);

	int LoadTile() const
	{ JMP_THIS(0x547020); }

	//Constructor
	IsometricTileTypeClass(int ArrayIndex, int Minus65, int Zero1,
		const char* pName, int Zero2) noexcept
		: IsometricTileTypeClass(noinit_t())
	{ JMP_THIS(0x5447C0); }

	static LightConvertClass* __fastcall SetupLightConvert(int r, int g, int b)
		{ JMP_STD(0x544E70); }

protected:
	explicit __forceinline IsometricTileTypeClass(noinit_t) noexcept
		: ObjectTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	int ArrayTypeIndex;
	int MarbleMadnessTile;
	int NonMarbleMadnessTile;
	DWORD ArrayIndex; //2A0
	DECLARE_PROPERTY(DynamicVectorClass<Color16Struct*>, RadarImage); //2A4
	IsometricTileTypeClass* NextTileTypeInSet; //2BC
	int ToSnowTheater;
	int ToTemperateTheater;
	int TileAnimIndex; //Tile%02dAnim, actually an AnimTypeClass array index...
	int TileXOffset; //Tile%02dXOffset
	int TileYOffset; //Tile%02dYOffset
	int TileAttachesTo; //Tile%02dAttachesTo, iso tile index?
	int TileZAdjust; //Tile%02dZAdjust
	DWORD maskint2DC; //0xBF
	bool Morphable;
	bool ShadowCaster;
	bool AllowToPlace; //default true
	bool RequiredByRMG;
	DWORD TileWidth; //2E4
	DWORD TileHeight; //2E8
	DWORD unk_2EC;
	int TilesInSequence; //default 1, no idea 2F0
	bool IsFileLoaded; //like always true 2F4
	char FileName[0xE]; // WARNING! Westwood strncpy's 0xE bytes into this buffer without NULL terminating it.
	bool AllowBurrowing;
	bool AllowTiberium;
	DWORD TilesLoadedMaybe_; //308
};

static_assert(sizeof(IsometricTileTypeClass) == 0x30C);
