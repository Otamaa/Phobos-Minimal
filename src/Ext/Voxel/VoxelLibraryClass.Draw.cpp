// ===========================================================================
// VoxelLibraryClass::Draw  -  vanilla 0x756590 .. 0x756854
//   __thiscall(VoxelDrawStruct* draw_data, Vector3* pos), retn 8
//
// ###########################################################################
// # THIS IS THE 255 WALL                                                    #
// ###########################################################################
//
// Everything else scaled with BufferSize. This did not, and could not, because
// the field it writes into is only 16 bits wide:
//
//   .text:007566A7  66 89 44 24 38   mov [esp+..+Start.X_I], ax
//   .text:007566CB  66 89 44 24 3A   mov [esp+..+Start.Y_J], ax
//   .text:007566EB  66 89 44 24 3C   mov [esp+..+Start.Z_K], ax
//
// `Vector3i16 __Start_v0_p_128_m_v3` is three int16. The value stored is
//
//     (world + centre - camera) * 256.0
//
// i.e. 8.8 fixed point, so the INTEGER part gets 8 bits and the reachable
// coordinate range is 0..255 regardless of how large the buffer is.
//
//   centre 128 -> a centred voxel is 128 * 256 == 0x8000       fits
//   centre 256 -> a centred voxel is 256 * 256 == 0x10000      truncates to 0
//
// At 512 with centre 256 every voxel snapped to the corner and everything near
// it wrapped - the "glitches all over the place". Raising the buffer, the clip
// rect and the surfaces was all necessary and none of it was sufficient, because
// the coordinate could never leave 0..255 in the first place.
//
// FIX: DrawStruct carries int32 StartX / StartY / StartZ appended past the
// vanilla layout, and this port fills them. The narrow Start fields are still
// written so the struct stays byte-compatible, but nothing reads them - all 21
// rasterizers now take the wide ones.
//
// The dispatch table can still be called directly: all 32 slots point at
// functions whose entry points carry our LJMPs, so the table reaches the ports.
//
// ###########################################################################
//
// LAYOUTS, all derived from the addressing in this function.
//
//   VoxelDrawStruct    +0x04 HeaderIndex  +0x08 InfoIndex  +0x0C entry
//                      +0x28 Vector3 vectors[]   (12 bytes each)
//   InternalLayerHeader  12 bytes, Index at +0
//   VoxelInternalLayerInfo  164 bytes
//                      +0x00 DataStartPtr  +0x04 DataEndPtr  +0x08 CurrentDataPtr
//                      +0xA0 SizeX  +0xA1 SizeY  +0xA2 SizeZ  +0xA3 NormalType
//   VoxelVals @ 0x8468C0, stride 36  (lea esi,[esi+esi*8] ; shl esi,2)
//                      +0x00 bitvalue     +0x04 vectorentry0  +0x08 vectorentry1
//                      +0x0C vectorentry2 +0x10 vectorentry3  +0x14 scaleval1
//                      +0x18 scaleval2    +0x1C XSteps        +0x20 YSteps
//
// DISPATCH INDEX
//   bit 1 (2)  VoxelUseBuffer_NighthawkNeedsThis   -> spec/depth buffer
//   bit 2 (4)  Voxel_NormalRenderingEnabled        -> lit
//   bit 3 (8)  !info->NormalType                   -> one-byte span stream
// on top of VoxelVals[entry].bitvalue.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

#include <cstdint>

namespace
{
	// VERIFY: 0x8468C0, stride 36.
	struct VoxelValsEntry
	{
		int BitValue;      // +0x00
		int VectorEntry0;  // +0x04
		int VectorEntry1;  // +0x08
		int VectorEntry2;  // +0x0C
		int VectorEntry3;  // +0x10
		int ScaleVal1;     // +0x14
		int ScaleVal2;     // +0x18
		int XSteps;        // +0x1C
		int YSteps;        // +0x20
	};
	static_assert(sizeof(VoxelValsEntry) == 36, "VoxelVals stride is 36 bytes.");

	inline VoxelValsEntry* const VoxelVals =
		reinterpret_cast<VoxelValsEntry*>(0x8468C0);

	inline int& VoxelUseBuffer = *reinterpret_cast<int*>(0xB43180);
	inline int& VoxelNormalRendering = *reinterpret_cast<int*>(0xB43184);

	// VERIFY: 0x846840, 32 slots. Every slot's target carries one of our LJMPs.
	using VoxelDrawFn = void(__cdecl*)(VoxelRaster::DrawStruct*);
	inline VoxelDrawFn* const VoxelDrawFunctionTbl =
		reinterpret_cast<VoxelDrawFn*>(0x846840);

	// Westwood's __ftol. SUSPECT: vanilla evaluates these on the x87 stack at
	// 80-bit precision; MSVC will use SSE at 64-bit. A 1-ULP difference can shift
	// a truncated 8.8 coordinate by 1/256 px. Accepted, same as in Draw_Shadow.
	inline int F2I(double value) noexcept { return static_cast<int>(value); }

	struct LayerHeader { int Index; int Unknown04; int Unknown08; };
	static_assert(sizeof(LayerHeader) == 12, "LayerHeader stride is 12 bytes.");

	struct LayerInfo
	{
		const int*    DataStartPtr;    // +0x00
		const int*    DataEndPtr;      // +0x04
		std::uint8_t* CurrentDataPtr;  // +0x08
		std::uint8_t  Padding0C[0x94]; // +0x0C .. +0x9F   VERIFY
		std::uint8_t  SizeX;           // +0xA0
		std::uint8_t  SizeY;           // +0xA1
		std::uint8_t  SizeZ;           // +0xA2
		std::uint8_t  NormalType;      // +0xA3
	};
	static_assert(offsetof(LayerInfo, SizeX) == 0xA0, "SizeX @ +0xA0");
	static_assert(sizeof(LayerInfo) == 164, "LayerInfo stride is 164 bytes.");

	struct Library
	{
		std::uint8_t Padding00[0x10];
		LayerHeader* LayerHeaders;     // +0x10
		LayerInfo*   LayerInfos;       // +0x14
	};

	struct VoxelDrawData
	{
		int          Unknown00;        // +0x00
		int          HeaderIndex;      // +0x04
		int          InfoIndex;        // +0x08
		int          Entry;            // +0x0C
		std::uint8_t Padding10[0x18];  // +0x10 .. +0x27   VERIFY
		Vector3      Vectors[1];       // +0x28, 12 bytes each
	};
	static_assert(offsetof(VoxelDrawData, Entry) == 0x0C, "Entry @ +0x0C");
	static_assert(offsetof(VoxelDrawData, Vectors) == 0x28, "Vectors @ +0x28");
}

// ---------------------------------------------------------------------------
// __fastcall with an unused second parameter expresses __thiscall: `this` in
// ECX, the two arguments on the stack, callee cleans - i.e. `retn 8`.
// ---------------------------------------------------------------------------
static void __fastcall VoxelLibrary_Draw(
	Library* pThis,
	void* /* unused, occupies the EDX slot */,
	VoxelDrawData* pDrawData,
	Vector3* pPos) noexcept
{
	const int infoIndex =
		pDrawData->InfoIndex + pThis->LayerHeaders[pDrawData->HeaderIndex].Index;
	const LayerInfo& info = pThis->LayerInfos[infoIndex];

	const VoxelValsEntry& vals = VoxelVals[pDrawData->Entry];

	VoxelRaster::DrawStruct data {};

	const int sizeX = info.SizeX;
	const int sizeY = info.SizeY;
	const int sizeZ = info.SizeZ;

	data.SizeX = info.SizeX;
	data.SizeY = info.SizeY;
	data.SizeZ = info.SizeZ;
	data.ColumnOffsetsStart = info.DataStartPtr;
	data.ColumnOffsetsEnd = info.DataEndPtr;
	data.SpanData = info.CurrentDataPtr;

	const Vector3& v0 = pDrawData->Vectors[vals.VectorEntry0];
	const Vector3& v1 = pDrawData->Vectors[vals.VectorEntry1];
	const Vector3& v2 = pDrawData->Vectors[vals.VectorEntry2];
	const Vector3& v3 = pDrawData->Vectors[vals.VectorEntry3];

	data.XSteps = vals.XSteps;
	data.YSteps = sizeX * vals.YSteps;
	data.DataPos = (sizeX - 1) * vals.ScaleVal1
		+ sizeX * (sizeY - 1) * vals.ScaleVal2;

	// -----------------------------------------------------------------------
	// THE WIDENED PART.
	//
	// centre is BufferSize / 2, not a hardcoded 128, and the results go into
	// int32 fields instead of int16. The * 256.0 stays - it is the 8.8 fixed
	// point scale, not a dimension.
	//
	// Z does NOT use the screen centre - it drives the depth accumulator, which
	// is a separate axis with its own range. The depth buffer is now 16-bit, so
	// depth is centred at 32768 and a model has +/- 32768 units of depth headroom
	// instead of the +/- 128 that made big models lose whole segments.
	// -----------------------------------------------------------------------
	constexpr double centre = Replacer::BufferCenter;
	constexpr double depthCentre = Replacer::DepthCenter;

	data.StartX = F2I((static_cast<double>(v0.X) + centre - pPos->X) * 256.0);
	data.StartY = F2I((static_cast<double>(v0.Y) + centre - pPos->Y) * 256.0);
	data.StartZ = F2I((static_cast<double>(v0.Z) + depthCentre - pPos->Z) * 256.0);

	// Narrow fields kept in sync so the struct stays byte-compatible with
	// anything that still expects the vanilla layout. Nothing reads them.
	data.Start.X = static_cast<std::int16_t>(data.StartX);
	data.Start.Y = static_cast<std::int16_t>(data.StartY);
	data.Start.Z = static_cast<std::int16_t>(data.StartZ);

	// Axis steps are deltas, not positions - they stay int16 and stay signed.
	const double fSizeX = static_cast<double>(sizeX);
	const double fSizeY = static_cast<double>(sizeY);
	const double fSizeZ = static_cast<double>(sizeZ);

	data.AxisX.X = static_cast<std::int16_t>(F2I((v1.X - v0.X) / fSizeX * 256.0));
	data.AxisY.X = static_cast<std::int16_t>(F2I((v2.X - v0.X) / fSizeY * 256.0));
	data.AxisZ.X = static_cast<std::int16_t>(F2I((v3.X - v0.X) / fSizeZ * 256.0));

	data.AxisX.Y = static_cast<std::int16_t>(F2I((v1.Y - v0.Y) / fSizeX * 256.0));
	data.AxisY.Y = static_cast<std::int16_t>(F2I((v2.Y - v0.Y) / fSizeY * 256.0));
	data.AxisZ.Y = static_cast<std::int16_t>(F2I((v3.Y - v0.Y) / fSizeZ * 256.0));

	// Vanilla only computes the Z components when the depth buffer is in use.
	if (VoxelUseBuffer != 0)
	{
		data.AxisX.Z = static_cast<std::int16_t>(F2I((v1.Z - v0.Z) / fSizeX * 256.0));
		data.AxisY.Z = static_cast<std::int16_t>(F2I((v2.Z - v0.Z) / fSizeY * 256.0));
		data.AxisZ.Z = static_cast<std::int16_t>(F2I((v3.Z - v0.Z) / fSizeZ * 256.0));
	}

	int slot = vals.BitValue;

	if (VoxelUseBuffer != 0)
	{
		slot |= 2;
	}

	if (VoxelNormalRendering != 0)
	{
		slot |= 4;
	}

	if (info.NormalType == 0)
	{
		slot |= 8;
	}

	VoxelDrawFunctionTbl[slot](&data);
}

// ---------------------------------------------------------------------------
// Hook: whole-function replacement via a 5-byte LJMP at the entry point.
// Vanilla is __thiscall with two stack arguments and `retn 8`; the __fastcall
// declaration above produces a byte-compatible frame and cleanup.
//
// Once this is active, DELETE the two projection-centre patches at 0x756696 and
// 0x7566B5 - the centre is applied here now, and those would be patching an
// instruction that no longer runs.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x756590, VoxelLibrary_Draw)
