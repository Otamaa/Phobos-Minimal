#pragma once

// ===========================================================================
// PhobosObjectShader - per-AnimType / per-WeaponType custom HLSL
//
// Distinct from the global .fx path (PhobosFXBuffer). That one lets an effect
// post-process the whole frame. This one lets a single AnimType or laser carry
// its OWN vertex and pixel shader, compiled at load time, bound while that one
// object draws.
//
// The object is drawn as a quad, four vertices, ids 0..3:
//
//     0---1     0,1 carry TopZ
//     |   |     2,3 carry BottomZ
//     2---3
//
// The vertex shader receives the id as `int vertexid : TEXCOORD0` and builds
// clip-space position plus UV itself. Interpolating TopZ/BottomZ down the quad
// is what gives per-pixel depth: the pixel shader compares the interpolated
// value against the engine ZBuffer and discards when occluded. Laying the quad
// flat is a matter of setting TopZ == BottomZ, which is all FXFlatDrawing is.
//
// ---------------------------------------------------------------------------
// REGISTER CONTRACT
// ---------------------------------------------------------------------------
//
// Sampler registers are fixed, and shader authors rely on them by number. They
// are part of the public API - changing one breaks every published shader.
//
//   s10  backbuffer      window size
//   s11  top mask        window size, non-zero where UI/mouse drew
//   s12  alpha buffer    tactical size, brightness multiplier, 127 = unity
//   s13  object texture  the anim's or laser's own art
//   s14  z buffer        tactical size
//   s15  RESERVED        bound but must not be sampled
//
// s15 is bound by the reference implementation with the comment
// "设置了但是不许使用" - set, but you may not use it. Reserved here for the
// same reason: sampling the distortion target while it is also the render
// target is undefined.
//
// Constant registers are typed as structs below rather than written as loose
// float4s, so a layout mistake is a compile error rather than a shader reading
// garbage from c2.
// ===========================================================================

#include <d3d9.h>

// ID3DBlob lives here. Compile() takes one by reference in the private section
// below, so the declaration is needed in the header - <d3dcompiler.h> in the
// .cpp alone is too late.
#include <d3dcommon.h>

#include "Runtime/reshade_api.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <YRPP.h>

namespace reshade
{
	namespace api
	{
		struct device;
	}
}

// ---------------------------------------------------------------------------
// Fixed sampler slots. Public API - do not renumber.
// ---------------------------------------------------------------------------
namespace PhobosShaderSlot
{
	constexpr UINT BackBuffer = 10;
	constexpr UINT TopMask = 11;
	constexpr UINT AlphaBuffer = 12;
	constexpr UINT ObjectTexture = 13;
	constexpr UINT ZBuffer = 14;
	constexpr UINT Reserved = 15;   // bound, never sampled
}

// ---------------------------------------------------------------------------
// Constant blocks.
//
// Each maps to consecutive float4 registers starting at c0. The static_asserts
// are the point: they make a mismatch between this layout and the documented
// register contract fail the build.
// ---------------------------------------------------------------------------

// Vertex shader, animation. c0..c3.
struct AnimVertexConstants
{
	// c0: quad rect in pixels, origin top-left of the canvas.
	float Left, Top, Width, Height;

	// c1: canvas size in pixels, then the Z values interpolated down the quad.
	// Setting TopZ == BottomZ is FXFlatDrawing.
	float CanvasWidth, CanvasHeight, TopZ, BottomZ;

	// c2: x current anim frame, y total frames, z game frames this anim frame
	// lasts, w game frames elapsed within it.
	//
	// At Rate=15, z = 900/15 = 60: x advances once per 60 game frames while w
	// cycles 0..59. Authors use w for smooth sub-frame motion.
	float Frame, FrameCount, FrameDuration, FrameElapsed;

	// c3: remaining loop iterations, LoopCount-derived. Padded to a full
	// register - a float1 still consumes c3 entirely.
	//
	// NOTE: the reference implementation reports 0 rather than 1 for a
	// non-looping anim reached via Next unless LoopCount=1 is written
	// explicitly. Phobos should normalise that to 1; a shader dividing by it
	// should not have to special-case the engine's bookkeeping.
	float RemainingIterations, _pad0, _pad1, _pad2;
};

static_assert(sizeof(AnimVertexConstants) == 4 * 4 * sizeof(float),
	"AnimVertexConstants must occupy exactly c0..c3");

// Pixel shader, animation. c0..c2.
struct AnimPixelConstants
{
	// c0: x overall brightness 0..2000 (0..2x, so 1000 is unity), y opacity
	// 0..100 percent, zw window client size in pixels.
	float AlphaLevel, Opacity, WindowWidth, WindowHeight;

	// c1: same frame progress the vertex shader receives.
	float Frame, FrameCount, FrameDuration, FrameElapsed;

	// c2: x distortion displacement, y remaining iterations, zw sheet grid.
	float DistortionDisplacement, RemainingIterations, SheetWidth, SheetHeight;
};

static_assert(sizeof(AnimPixelConstants) == 3 * 4 * sizeof(float),
	"AnimPixelConstants must occupy exactly c0..c2");

// Pixel shader, laser. c0..c4.
struct LaserPixelConstants
{
	// c0: x intensity 0..1, y distortion displacement, z distortion width,
	// w frames rendered so far, 0..Duration-1.
	float Intensity, DistortionDisplacement, DistortionWidth, FrameIndex;

	// c1: window size, laser duration, per-laser random seed.
	float WindowWidth, WindowHeight, Duration, Seed;

	// c2: inner colour 0..255, w non-zero when it is the house colour.
	float InnerR, InnerG, InnerB, IsHouseColor;

	// c3 / c4: outer colour and spread, 0..255. Each occupies a whole register
	// despite being float3.
	float OuterR, OuterG, OuterB, _pad0;
	float SpreadR, SpreadG, SpreadB, _pad1;
};

static_assert(sizeof(LaserPixelConstants) == 5 * 4 * sizeof(float),
	"LaserPixelConstants must occupy exactly c0..c4");

// ---------------------------------------------------------------------------
// A compiled shader pair, shared between every type that names the same
// file/entry/target triple.
// ---------------------------------------------------------------------------
struct PhobosShaderKey
{
	std::string File;
	std::string Entry;
	std::string Target;

	bool operator==(const PhobosShaderKey& other) const
	{
		return File == other.File && Entry == other.Entry && Target == other.Target;
	}
};

struct PhobosShaderKeyHash
{
	size_t operator()(const PhobosShaderKey& k) const noexcept;
};

class PhobosShaderCache
{
public:
	static PhobosShaderCache& Instance();

	// Compiles or returns a cached shader. Both out-params stay null on
	// failure and the caller falls back to the default shader.
	bool GetPixelShader(const char* file, const char* entry, IDirect3DPixelShader9** ppOut);
	bool GetVertexShader(const char* file, const char* entry, IDirect3DVertexShader9** ppOut);

	void OnDeviceInit(IDirect3DDevice9* pDevice);
	void OnDeviceReset();

	// Compile diagnostics for the in-game overlay. The reference build writes
	// these to a side log file, which nobody reads until after they have
	// already filed the bug.
	const std::vector<std::string>& Diagnostics() const { return _diagnostics; }
	void ClearDiagnostics() { _diagnostics.clear(); }

private:
	PhobosShaderCache() = default;

	bool Compile(const PhobosShaderKey& key, com_ptr<ID3DBlob>& outCode);

	IDirect3DDevice9* _device { nullptr };

	std::unordered_map<PhobosShaderKey, com_ptr<IDirect3DPixelShader9>, PhobosShaderKeyHash> _pixel;
	std::unordered_map<PhobosShaderKey, com_ptr<IDirect3DVertexShader9>, PhobosShaderKeyHash> _vertex;

	std::vector<std::string> _diagnostics;
};