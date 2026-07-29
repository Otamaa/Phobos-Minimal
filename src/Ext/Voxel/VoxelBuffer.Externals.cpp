// ===========================================================================
// Voxel_get_VoxelPixelBuffer  -  vanilla 0x753C60 .. 0x753C65
//
//   .text:00753C60  B8 78 FF B2 00   mov eax, offset VoxelPixelBuffer
//   .text:00753C65  C3               retn
//
// Six bytes total. A 5-byte LJMP overwrites the `mov` and leaves the `C3`
// orphaned but unreachable, which is fine.
//
// WHY A JUMP RATHER THAN THE PATCH AT 0x753C61
// --------------------------------------------
// I originally said to keep this as a DEFINE_PATCH_TYPED, on the grounds that a
// two-instruction function with one constant is exactly what patching is for.
// That misses a practical problem:
//
//     DEFINE_PATCH_TYPED(DWORD, 0x753C61, DWORD(&Replacer::VoxelPixelBuffer))
//
// pins the buffer address at compile time, which is fine, but it is one more
// place that has to agree with everything else. The jump resolves it at run time
// like every other port, so there is a single source of truth.
//
// With this ported, no DEFINE_PATCH_TYPED anywhere references either buffer's
// address - the only two patches left in the whole set are the `fadd` operands
// that recentre the projection.
//
// CALLERS ARE STILL UNKNOWN
// -------------------------
// VERIFY: nothing in what has been analysed so far calls this. It appears in the
// .data xref list for VoxelPixelBuffer, so something does. Worth finding them
// before raising BufferSize - a caller that does `ptr + y * 256 + x` is still
// wrong at 512 no matter how correct the pointer is. This is the one place the
// hardcoded 256 stride can escape the voxel code entirely.
// ===========================================================================

#include "VoxelRaster.h"

#include <Utilities/Macro.h>

// ---------------------------------------------------------------------------
// Zero-argument function returning in EAX. __fastcall, __stdcall and __cdecl are
// all identical for this shape - no arguments to pass, nothing to clean, plain
// `ret` - so the declared convention only has to match for readability. IDA types
// it __fastcall, so that is what is used here.
//
// DIFF: returns uint8_t* where vanilla's IDA type says char*. Pointer signedness
// has no ABI effect.
// ---------------------------------------------------------------------------
static std::uint8_t* __fastcall VoxelGet_PixelBuffer() noexcept
{
	return &Replacer::VoxelPixelBuffer[0][0];
}

// ---------------------------------------------------------------------------
// Once this is active, DELETE the patch at 0x753C61.
//
// VERIFY: Phobos spells this DEFINE_FUNCTION_JUMP. If your tree uses
// DEFINE_FUNCTION_PATCH, rename accordingly - the arguments are identical.
// ---------------------------------------------------------------------------
DEFINE_FUNCTION_JUMP(LJMP, 0x753C60, VoxelGet_PixelBuffer)

// ===========================================================================
// sub_754910  -  vanilla 0x754910 .. 0x754937
//   Static initialiser, registered at .data:00815694.
//
//   fld  num_256           ; 256.0, a DOUBLE CONSTANT at 0x7E1710
//   fld  num_2             ; 2.0,   a double constant at 0x7E1708
//   call __CIpow           ; pow(256.0, 2.0) == 65536
//   fadd st, st            ; + itself == 131072
//   call FastMath::Sqrt    ; sqrt(131072) == 362.038671...
//   fstp dbl_B43EF8
//
//   dbl_B43EF8 = sqrt(2 * 256^2) = 256 * sqrt(2)
//
// i.e. the DIAGONAL LENGTH OF THE VOXEL BUFFER.
//
// ###########################################################################
// # A 256 DEPENDENCY THAT NO XREF SCAN WOULD HAVE FOUND                     #
// ###########################################################################
//
// The 256 here is not an instruction immediate - it is a double constant in
// .rdata at 0x7E1710, loaded by `fld`. It does not appear in any xref of
// VoxelPixelBuffer, and DEFINE_PATCH_TYPED cannot safely reach it: 256.0 is a
// generic constant that other unrelated code almost certainly shares. Patching
// 0x7E1710 would change every one of those too.
//
// So this one HAS to be ported rather than patched. Recomputing it from
// Replacer::BufferSize keeps it correct at any size.
//
// VERIFY - THIS IS THE OPEN QUESTION
// ----------------------------------
// Nothing analysed so far reads dbl_B43EF8, so what it gates is unknown. Given
// it is a buffer diagonal, the likely use is a bounding-radius or
// maximum-distance test deciding whether a voxel fits, or how far to search.
// If so it scales with BufferSize and this port is correct. If it turns out to
// mean something else, this needs revisiting.
//
// **Please send the xrefs for dbl_B43EF8 (0xB43EF8).** It is the only remaining
// hidden 256 found so far, and the fact that it exists at all suggests looking
// for others of the same shape - float/double constants rather than immediates.
// ===========================================================================

// VERIFY: 0x4CAC40, from the rel32 at 0x754929 (E8 12 63 D7 FF, next instruction
// 0x75492E). __cdecl, takes a double on the stack, returns in st(0).
// Used rather than std::sqrt so the stored value stays bit-identical to vanilla -
// FastMath::Sqrt is likely an approximation and std::sqrt is correctly rounded.
static const auto FastMath_Sqrt =
	reinterpret_cast<double(__cdecl*)(double)>(0x4CAC40);

// VERIFY: 0xB43EF8.
static double& VoxelBufferDiagonal = *reinterpret_cast<double*>(0xB43EF8);

// !! THE 256 HERE IS DELIBERATELY NOT BufferSize. !!
//
// I originally scaled this with BufferSize on the assumption that it is the
// render buffer's diagonal. That assumption is UNVERIFIED and is the most likely
// cause of the "glitches everywhere" at 512.
//
// The three consumers are all `fmul` / `fdiv` by this value:
//
//   .text:00754940  fld  dbl_B43EF8      unknown_libname_643
//   .text:007549F7  fmul dbl_B43EF8      calc_flt_7549E0+17
//   .text:00754A64  fdiv dbl_B43EF8      sub_754A50+14
//
// Multiplying and dividing by it is normalisation. If it normalises against the
// VOXEL COORDINATE SPACE - and .vxl models are 0..255 in their own units, which
// has nothing to do with the render buffer - then doubling it halves every
// resulting scale factor, and the geometry comes out wrong everywhere. That
// matches the symptom exactly.
//
// Set VANILLA. Change to Replacer::BufferSize only once those three functions
// have been read and it is actually established to be a buffer dimension.
static constexpr double DiagonalEdge = 256.0;

static void __cdecl VoxelInit_BufferDiagonal() noexcept
{
	// Vanilla: pow(edge, 2.0), then `fadd st, st` to double it, then sqrt.
	const double squared = DiagonalEdge * DiagonalEdge;
	VoxelBufferDiagonal = FastMath_Sqrt(squared + squared);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x754910, VoxelInit_BufferDiagonal)
