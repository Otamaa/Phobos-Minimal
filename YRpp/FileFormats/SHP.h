#pragma once

#include <BasicStructures.h>
#include <RectangleStruct.h>
/*
*	SHP structs come in different forms: the plain file data, and a kind of
*	reference used for caching. Usually, it is not needed to know what type a
*	SHPStruct is of, because the the member functions work with both.
*/


struct SHPReference;
struct SHPFile;

//SHP file stuff
//ShapeCache
struct SHPStruct //header
{
	SHPStruct() : Type(0), Width(0), Height(0), Frames(0)
		{}

	~SHPStruct()
		{ JMP_THIS(0x69E500); }

	// loads the file, if this is a referece
	void Load()
		{ JMP_THIS(0x69E090); }

	// unloads the data, if this is a reference
	void Unload()
		{ JMP_THIS(0x69E100); }

	// resolves to the actual file data, and loads it if necessary
	SHPFile* GetData()
		{ JMP_THIS(0x69E580); }

	RectangleStruct* GetFrameBounds(RectangleStruct &buffer, int idxFrame) const
		{ JMP_THIS(0x69E7E0); }

	RectangleStruct* GetFrameBounds_ptr(RectangleStruct* pbuffer, int idxFrame) const
	{ JMP_THIS(0x69E7E0); }

	RectangleStruct GetFrameBounds(int idxFrame) const {
		RectangleStruct buffer;
		GetFrameBounds(buffer, idxFrame);
		return buffer;
	}

	ColorStruct* GetColor(ColorStruct &buffer, int idxFrame) const
		{ JMP_THIS(0x69E860); }

	ColorStruct GetColor(int idxFrame) const {
		ColorStruct buffer;
		return *GetColor(buffer, idxFrame);
	}

	byte* GetPixels(int idxFrame)
		{ JMP_THIS(0x69E740); }

	// Flags & 2
	bool HasCompression(int idxFrame) const
		{ JMP_THIS(0x69E900); }

	bool IsReference() const {
		return Type == 0xFFFF;
	}

	SHPReference* AsReference();

	const SHPReference* AsReference() const;

	SHPFile* AsFile();

	const SHPFile* AsFile() const;

	WORD	Type;
	short	Width;
	short	Height;
	short	Frames;
};

struct Theater_SHPStruct : public SHPStruct { };

struct SHPReference : public SHPStruct
{
	//=== GLOBAL LINKED LIST OF ALL LOADED SHP FILES
	// defined but not used
	static constexpr reference<SHPReference*, 0xB077B0u> List{};

	SHPReference(const char* filename)
		{ JMP_THIS(0x69E430); }

	//SHPFile* GetData()
	//	{ JMP_THIS(0x69E580); }

	char*			Filename;
	SHPStruct*		Data;
	bool			Loaded;
	int				Index;
	//linked list of all SHPReferences
	SHPReference*	Next;
	SHPReference*	Prev;
	DWORD			unknown_20;
};

struct SHPFrame
{
	RectangleStruct GetFrameDimensions() const { return { Left, Top, Width, Height }; }

	short		Left; //X
	short		Top; //Y
	short		Width; 
	short		Height;
	DWORD		Flags;
	ColorStruct	Color;
	DWORD		unknown_10;
	int			Offset;
};

struct SHPFile : public SHPStruct
{
	const SHPFrame& GetFrameHeader(int idxFrame) const {
		return (&FirstFrame)[idxFrame];
	}

	SHPFrame	FirstFrame;
};

inline SHPReference* SHPStruct::AsReference() {
	return IsReference() ? static_cast<SHPReference*>(this) : nullptr;
}

inline const SHPReference* SHPStruct::AsReference() const {
	return IsReference() ? static_cast<const SHPReference*>(this) : nullptr;
}

inline SHPFile* SHPStruct::AsFile() {
	return !IsReference() ? static_cast<SHPFile*>(this) : nullptr;
}

inline const SHPFile* SHPStruct::AsFile() const {
	return !IsReference() ? static_cast<const SHPFile*>(this) : nullptr;
}

#pragma pack(4)
struct ShapeFileStruct
{
public:
	operator void* () const { return (*this); } // This allows the struct to be passed implicitly as a raw pointer.

	SHPFrame* GetFrameData(int index)
	{
		return &(&FrameData)[index * sizeof(SHPFrame)];
	}

	int GetWidth() const { return Header.Width; }
	int GetHeight() const { return Header.Height; }
	int GetFrameCount() const { return Header.Frames; }

private:
	SHPStruct Header;

	/**
	 *  This is an instance of the first frame in the shape file, use Get_Frame_Data
	 *  to get the frame information, do not access this directly!
	 */
	SHPFrame FrameData;
};
#pragma pack()

//=== GLOBAL LINKED LIST OF ALL LOADED SHP FILES
// defined but not used
// static SHPStruct* SHPStruct_first=(SHPStruct*)0xB077B0;
//==============================================
