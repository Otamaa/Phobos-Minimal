//This can be used to load PCX files into BSurfaces!

#pragma once

#include <GeneralDefinitions.h>
#include <Helpers/CompileTime.h>
#include <RectangleStruct.h>
#include <Wstring.h>

struct BytePalette;
class FileClass;
class Surface;
class BSurface;
class DSurface;
class NOVTABLE PCXEntry {
  Wstring FilenameTemp;
  char Filename[772];
  PCXEntry *Next;
};

static_assert(sizeof(PCXEntry) == 0x30C, "Invalid Size !");
#pragma pack(push, 4)
class NOVTABLE PCXImages
{
public:

  PCXEntry **entries;
  DWORD Count;
  int Bitmask1;
  DWORD Bitmask2;
  int dword_10;
  char char_14;
  void* func_18; //callback ?
  int field_1C;
  double qword_20;
  double qword_28;
  int dword_30;

public:
	static COMPILETIMEEVAL reference<PCXImages, 0xAC4848u> const Instance{};
	static OPTIONALINLINE COMPILETIMEEVAL WORD const DefaultTransparentColor = COLOR_PURPLE;

public:

	static Surface* __fastcall Read_PCX_File(FileClass* file, unsigned char* palette, void* buffer, int buf_size)
	{ JMP_FAST(0x630310); }

	// Load a PCX file
	bool ForceLoadFile(const char* pFileName, int flag1, int flag2)
		{ JMP_THIS(0x6B9D00); }

	//Load a PCX file
	bool LoadFile(const char *pFileName, int flag1 = 2, int flag2 = 0);

	// Get a BSurface for a PCX file. File needs to be loaded some time first!
	BSurface* GetSurface(const char* pFileName, BytePalette * pPalette = nullptr)
		{ JMP_THIS(0x6BA140); }

	bool BlitToSurface(RectangleStruct *BoundingRect,
		DSurface* TargetSurface,
		BSurface* PCXSurface,
		WORD TransparentColor = DefaultTransparentColor
	)
		{ JMP_THIS(0x6BA580); }
public:

  	PCXImages(){
		JMP_THIS(0x6B9450);
	}

	~PCXImages(){
		JMP_THIS(0x6B9530);
	}

};
#pragma pack(pop)

static_assert(sizeof(PCXImages) == 0x34, "Invalid Size !");