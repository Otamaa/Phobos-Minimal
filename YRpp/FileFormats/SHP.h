#pragma once

#include <BasicStructures.h>
#include <RectangleStruct.h>
/*
*	SHP structs come in different forms: the plain file data, and a kind of
*	reference used for caching. Usually, it is not needed to know what type a
*	SHPCaches is of, because the the member functions work with both.
*/

enum SHPFlags : __int32
{
	ShapeFlags_1 = 0x1,
	ShapeFlags_2 = 0x2,
	ShapeFlags_Type_4 = 0x4,
	ShapeFlags_Type_8 = 0x8,
	ShapeFlags_Type_10 = 0x10,
	ShapeFlags_Type_20 = 0x20,
	ShapeFlags_Type_40 = 0x40,
	ShapeFlags_Type_80 = 0x80,
	ShapeFlags_Type_100 = 0x100,
	SHAPE_CENTER = 0x200,
	ShapeFlags_Type_400 = 0x400,
	ShapeFlags_Type_800 = 0x800,
	ShapeFlags_Type_1000 = 0x1000,
	ShapeFlags_Type_2000 = 0x2000,
	SHAPE_USE_ZBUFFER = 0x4000,
	ShapeFlags_Type_8000 = 0x8000,
	ShapeFlags_Type_10000 = 0x10000,
	ShapeFlags_Type_20000 = 0x20000,
	ShapeFlags_Type_40000 = 0x40000,
};
MAKE_ENUM_FLAGS(SHPFlags)

struct SHPCaches;
struct SHPFile;

// Containing Header of Shape file
struct SHPHeader 
{
	short	Type { -1 };
	short	Width {};
	short	Height {};
	short	Frames {};
};
static_assert(sizeof(SHPHeader) == 0x8);

struct SHPCaches
{
protected:
	SHPCaches() = delete;
	SHPCaches(SHPCaches const& rvalue) = delete;
	SHPCaches& operator=(SHPCaches const& rvalue) = delete;
public:

	static COMPILETIMEEVAL reference<SHPCaches*, 0xB077B0u> List {};

	SHPHeader GetHeader() {
		return this->CurrentHeader;
	}

	SHPCaches(const char* filename)
	{ JMP_THIS(0x69E430); }

	~SHPCaches()
	{ JMP_THIS(0x69E500); }

	SHPFile* GetData()
	{ JMP_THIS(0x69E580); }

	// loads the file, if this is a referece
	void Load()
	{ JMP_THIS(0x69E090); }

	// unloads the data, if this is a reference
	void Unload()
	{ JMP_THIS(0x69E100); }

	RectangleStruct* GetFrameBounds(RectangleStruct& buffer, int idxFrame) const
	{ JMP_THIS(0x69E7E0); }

	RectangleStruct* GetFrameBounds_ptr(RectangleStruct* pbuffer, int idxFrame) const
	{ JMP_THIS(0x69E7E0); }

	RectangleStruct GetFrameBounds(int idxFrame) const
	{
		RectangleStruct buffer;
		GetFrameBounds(buffer, idxFrame);
		return buffer;
	}

	ColorStruct* GetColor(ColorStruct& buffer, int idxFrame) const
	{ JMP_THIS(0x69E860); }

	ColorStruct GetColor(int idxFrame) const
	{
		ColorStruct buffer;
		return *GetColor(buffer, idxFrame);
	}

	uint8_t* GetPixels(int idxFrame) const
	{ JMP_THIS(0x69E740); }

	// Flags & 2
	bool HasCompression(int idxFrame) const
	{ JMP_THIS(0x69E900); }

	bool IsReference() const
	{
		return CurrentHeader.Type == -1;
	}

	COMPILETIMEEVAL OPTIONALINLINE int GetWidth() const { return CurrentHeader.Width; }
	COMPILETIMEEVAL OPTIONALINLINE int GetHeight() const { return CurrentHeader.Height; }
	COMPILETIMEEVAL OPTIONALINLINE int GetFrameCount() const { return CurrentHeader.Frames; }

public:

	SHPHeader CurrentHeader;
	const char*		Filename; //strdup
	SHPFile*		Data;
	bool			Loaded;
	int				Index;
	//linked list of all SHPReferences
	SHPCaches*	Next;
	SHPCaches*	Prev;
	DWORD			unknown_20;
};
static_assert(sizeof(SHPCaches) == 0x24);

struct SHPFrame
{
	RectangleStruct GetFrameDimensions() const {
		return { Left, Top, Width, Height };
	}

public:

	short		Left; //X
	short		Top; //Y
	short		Width;
	short		Height;
	SHPFlags	Flags;
	ColorStruct	Color;
	char		arrayof5[5];
	int			Offset;
};
static_assert(sizeof(SHPFrame) == 0x18);

struct SHPFile 
{
	operator void* () const { return (*this); } // This allows the struct to be passed implicitly as a raw pointer.

	COMPILETIMEEVAL OPTIONALINLINE int GetWidth() const { return Header.Width; }
	COMPILETIMEEVAL OPTIONALINLINE int GetHeight() const { return Header.Height; }
	COMPILETIMEEVAL OPTIONALINLINE int GetFrameCount() const { return Header.Frames; }
public:

	SHPHeader   Header;
	SHPFrame	FirstFrame;
};
static_assert(sizeof(SHPFile) == 0x20);
