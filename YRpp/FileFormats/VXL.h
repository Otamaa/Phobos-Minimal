#pragma once

class CCFileClass;
#include <GeneralStructures.h>
#include <RectangleStruct.h>
#include <ColorStruct.h>
#include <Memory.h>
#include <Matrix3D.h>

struct VoxelDrawStruct;
struct VoxelShadowDrawStruct;
struct VoxelSectionHeader;
struct VoxelSectionTailer;
class Surface;
class VoxLib {
public:
	bool LoadFailed;
	DWORD CountHeaders;
	DWORD CountTailers;
	DWORD TotalSize;
	VoxelSectionHeader* HeaderData;
	VoxelSectionTailer* TailerData;
	byte * BodyData;

	VoxLib() noexcept
		: LoadFailed { false }
	    , CountHeaders { 0 }
	    , CountTailers { 0 }
		, TotalSize { 0 }
		, HeaderData { nullptr }
		, TailerData { nullptr }
		, BodyData{ nullptr }
		{  }

	VoxLib(CCFileClass *Source, bool UseContainedPalette = 0) noexcept
		: LoadFailed{ false }
		, CountHeaders{ 0 }
		, CountTailers{ 0 }
		, TotalSize{ 0 }
		, HeaderData{ nullptr }
		, TailerData{ nullptr }
		, BodyData{ nullptr }
	{
		if (!this->ReadFile(Source , UseContainedPalette))
			LoadFailed = true;
	}

	~VoxLib() noexcept {

		if (HeaderData) {
			YRMemory::Deallocate(HeaderData);
			HeaderData = nullptr;
		}


		if(TailerData) {
			YRMemory::Deallocate(TailerData);
			TailerData = nullptr;
		}

		if(BodyData) {
			YRMemory::Deallocate(BodyData);
			BodyData = nullptr;
		}
	}

	static COMPILETIMEEVAL reference<Point3D , 0xB2D5E0u> const ClippingMax {};
	static COMPILETIMEEVAL reference2D<char , 0xB2FF78u , 256 , 256> const PixelBuffers {};
	static COMPILETIMEEVAL reference<char, 0xB45990u, 0x256> const NormalToLutP {};
	static COMPILETIMEEVAL reference<WORD, 0xB45590u, 256> const DistanceLut {};
	static COMPILETIMEEVAL reference<Matrix3D, 0xB45188u, 21> const RampMatrixes {};
	static COMPILETIMEEVAL reference<Matrix3D, 0xB450B8u, 4> const _Matrixes {};
	static COMPILETIMEEVAL reference<Vector3D<double>, 0xB444A0u, 256> const Unused_Vectors {};
	static COMPILETIMEEVAL reference<Vector3D<double>, 0xB432D8u, 256> const Unused_Vectors2 {};
	static COMPILETIMEEVAL reference<Quaternion, 0xB44348u, 21> const Unused_Quaterions {};
	static COMPILETIMEEVAL reference<Quaternion, 0xB43188u, 21> const Unused_Quaterions2 {};
	static COMPILETIMEEVAL reference<Matrix3D, 0xB44318u> const DefaultMatrix {};
	static COMPILETIMEEVAL reference<Matrix3D, 0xB43F10u, 21 > const _Matrixes1 {};
	static COMPILETIMEEVAL reference2D<short, 0xB41178u, 256, 32> const VPLLookups {};


	// returns 0 on success, non zero on failure
	bool ReadFile(CCFileClass *ccFile, bool UseContainedPalette)
		{ JMP_THIS(0x755DB0); }

	// return &this->HeaderData[headerIndex];
	VoxelSectionHeader* leaSectionHeader(int headerIndex)
		{ JMP_THIS(0x7564A0); }

	// return &this->TailerData[a3 + this->HeaderData[headerIndex].limb_number];
	VoxelSectionTailer* leaSectionTailer(int headerIndex, int a3)
		{ JMP_THIS(0x7564B0); }

	bool IsNotInitiated() const
		{ JMP_THIS(0x717AE0); }

	void Clear() const
		{ JMP_THIS(0x755D60); }

	Vector3D<float>* GetVector3(Vector3D<float>* a2, int header, int layer) const
		{ JMP_THIS(0x7564E0); }

	size_t MemoryUsed() const
		{ JMP_THIS(0x756570); }

	void Draw(VoxelDrawStruct* draw_data, Vector3D<float>* pos) const
		{ JMP_THIS(0x756590); }

	void ShadowAlreadyDrawn(VoxelShadowDrawStruct* shadow_draw_data, Vector3D<float>* pos) const
		{ JMP_THIS(0x756860); }

	void Adjust() const
		{ JMP_THIS(0x756BB0); }

	static bool IsInvalid(const VoxLib* pThis)
		{ return !pThis || pThis->LoadFailed; }
};

struct TransformVector {
	Vector3D<float> XYZ;
	float Unknown;
};

struct VoxelDrawStruct {
	VoxLib* lib;
	int HeaderIndex;
	int InfoIndex;
	int entry;
	Vector3D<float> vector3_10;
	Vector3D<float> vector3_1C;
	Vector3D<float> vector3array_28[8];
};

struct VoxelShadowDrawStruct {

	static COMPILETIMEEVAL constant_ptr<VoxelShadowDrawStruct* , 0xB3FF78u> const DrawQueue {};
	static COMPILETIMEEVAL reference<size_t , 0xB2FB70u> const DrawQueue_count {};

	VoxLib* lib;
	int HeaderIndex;
	int InfoIndex;
	Vector3D<float> v0;
	Vector3D<float> v1;
	Vector3D<float> v2;
	Vector3D<float> v3;
	Surface* SurfacePtr;
	int ShadowPointX;
	int ShadowPointY;
};

// file header
struct VoxFileHeader {
	char filename[16];
	int PaletteCount;
	int countHeaders_OrSections1;
	int countTailers_OrSections2;
	int totalSize;
};

// internal representation of the next struct
struct VoxelSectionHeader {
	int limb_number;
	int unk1;
	char unk2;
};

struct VoxelCalcStruct {
  RectangleStruct rect;
  int datalength;
  int spanmaybe;
};

// in file
struct VoxelSectionFileHeader {
	char Name[16];
	VoxelSectionHeader headerData;
};

// in file
struct VoxelSectionFileTailer {
	int span_start_off;
	int span_end_off;
	int span_data_off;
	float DetFloat;
	Matrix3D TransformationMatrix;
	Vector3D<float> MinBounds;
	Vector3D<float> MaxBounds;
	char size_X;
	char size_Y;
	char size_Z;
	char NormalsMode;
};

// internal representation
struct VoxelSectionTailer {
	int span_start_off;
	int span_end_off;
	int span_data_off;
	float HVAMultiplier;
	Matrix3D TransformationMatrix;
	Vector3D<float> Bounds[8];
	char size_X;
	char size_Y;
	char size_Z;
	char NormalsMode;
};

class VoxelPaletteClass
{
public:
	static COMPILETIMEEVAL reference<int, 0xB45A90u, 16> const Scales1 {};

	VoxelPaletteClass(char* palette, char* lut) { JMP_THIS(0x758950); }
	~VoxelPaletteClass() { JMP_THIS(0x7589C0); }

	bool ReadPalette(CCFileClass* file) const { JMP_THIS(0x7589F0); }
	bool Read(CCFileClass* file) const { JMP_THIS(0x758A30); }
	bool Write(CCFileClass* file) const { JMP_THIS(0x758AD0); }
	void Calculate_Lookup_Table(float* scale, int lut_count) const { JMP_THIS(0x758B70); }
	unsigned char Closest_Color(float red, float green, float blue) const { JMP_THIS(0x758E10); }
	unsigned char Closest_Remap_Color(float red, float green, float blue, bool check_remap) const { JMP_THIS(0x758EA0); }

public:
	int RemapStart;
	int RemapEnd;
	int LUTCount;
	int Unused;
	RGBClass* Palette;
	char(*LookupTable)[1];
	DWORD PaletteAllocated;
	DWORD LUTAllocated;
};