#pragma once

// ---------------------------------------------------------------------------
// PhobosAlphaMask - deferred (record-and-replay) variant
//
// DIFF vs the immediate-blit version this replaces:
//
//   1. TWO BYTES PER PIXEL, not one. The reference fork clears its buffer with
//      `memset(buffer, 0, 2 * width * height)` and advances rows by `2 * width`.
//      Every queued record carries a 16-bit parameter, so the mask is an
//      intensity/tint value, not a boolean coverage flag. D3DFMT_L8 -> L16.
//
//   2. Draw-pass hooks RECORD instead of blitting. The pass appends 40-byte
//      records to a queue; the whole queue is replayed once at EndPass. The
//      game thread is single-threaded through the tactical draw, so recording
//      needs no lock at all - the mutex is now taken once per frame, around
//      the replay, instead of being held across the entire draw pass.
//
//      This is what removes the BeginPass/EndPass pairing hazard outright:
//      there is no lock to leak if EndPass is missed, and `_active` is gone.
//
//   3. The queue is drained UNCONDITIONALLY on every exit path. The reference
//      fork put its drain inside `if (EnhancedLight)` while its producers were
//      gated on a per-type flag instead, so the queue grew without bound
//      whenever the feature was switched off. See EndPass.
//
// Unchanged from the previous version: texture-semantic binding (no runtime
// struct offsets), DYNAMIC+DEFAULT pool, double buffering, bounds-clamped
// blitters.
//
// The semantic an .fx must declare:
//
//     texture PhobosAlphaMaskTex : PHOBOS_ALPHAMASK;
//     sampler sPhobosAlphaMask { Texture = PhobosAlphaMaskTex; };
//
// Sampling changes with the format: r16_unorm reads in .r as a normalised
// 0..1 float. To recover the two packed bytes:
//
//     float  raw  = tex2D(sPhobosAlphaMask, uv).r * 65535.0;
//     float  low  = fmod(raw, 256.0);      // AnimTypeExt +60
//     float  high = floor(raw / 256.0);    // AnimTypeExt +64
// ---------------------------------------------------------------------------

#include <d3d9.h>

#include <cstdint>
#include <mutex>
#include <vector>

#include <RectangleStruct.h>
#include <Point2D.h>
#include <FileFormats/SHP.h>

#include <Helpers/ComPtr.h>

#include "Runtime/reshade_api.hpp"

namespace reshade
{
	class runtime;

	namespace api
	{
		struct device;
	}
}

// ---------------------------------------------------------------------------
// One deferred draw record.
//
// Layout mirrors the reference fork's queue element exactly. Confirmed against
// both call sites:
//
//   BulletClass_DrawSHP_Final:
//     queue_FX(shape, frame, *(_OWORD*)rect, xy[0], xy[1], 0x2E00, 127)
//
//   sub_10017ED0 (anim update):
//     queue_FX(pType->Image, frame, rect, x, y, flags, low + (high << 8))
//
// 4 + 4 + 16 + 4 + 4 + 4 + 4 = 40, matching the /40 stride in the fork's
// vector destructor. No padding, no slack.
// ---------------------------------------------------------------------------
struct FXEntry
{
	SHPCaches* Shape;    // +0x00
	int             Frame;    // +0x04
	RectangleStruct Bounds;   // +0x08  passed by value (the __int128)
	int             X;        // +0x18
	int             Y;        // +0x1C
	DWORD           Flags;    // +0x20
	uint16_t        Param;    // +0x24  low byte | high byte << 8
};

static_assert(sizeof(RectangleStruct) == 16, "FXEntry layout assumes a 16-byte RectangleStruct");

class PhobosAlphaMask
{
public:
	static constexpr const char* Semantic = "PHOBOS_ALPHAMASK";

	// Records reserved up front so a steady-state frame never allocates. The
	// reference fork's vector reached the >= 0x1000-byte branch in its
	// destructor (102 records), so this is comfortably above observed peak.
	static constexpr size_t QueueReserve = 512;

	static PhobosAlphaMask& Instance();

	bool OnInit(IDirect3DDevice9* pDevice, reshade::api::device* pReShadeDevice, int width, int height);
	void OnReset();

	// Called from the ReShade round-trip, after EndPass and before effects run.
	void UploadAndBind(reshade::runtime* pRuntime);

	// --- Draw-pass interface, called from the ASMJIT hooks -----------------

	// TacticalClass draw pass entry (0x6D8F0F). Clears the queue. No lock.
	void BeginPass();

	// TacticalClass draw pass exit (0x6D97BF). Replays the queue into the mask
	// under the lock, publishes it, and drains. Safe to call unpaired.
	void EndPass();

	// Append one record. Cheap, allocation-free in steady state, no locking.
	// Returns false when no pass is open.
	bool Queue(SHPCaches* pShape, int frame, DWORD flags,
		const RectangleStruct& bounds, int x, int y, uint16_t param);

	bool IsPassActive() const { return _pass_active; }

private:
	PhobosAlphaMask() = default;

	// Replay of a single record: clip, then dispatch to the right blitter.
	void BlitEntry(const FXEntry& entry);

	// VERIFY: overlap resolution. `max` is order-independent, which matters now
	// that records replay in queue order rather than draw order - two lights on
	// one pixel must not flicker as the draw order shuffles between frames.
	// If FXBlit turns out to overwrite instead, this becomes `return value;`.
	static uint16_t Combine(uint16_t existing, uint16_t value)
	{
		return existing > value ? existing : value;
	}

	// VERIFY: the SHP index is treated as coverage only - non-zero means "this
	// pixel is covered", and `param` is what lands in the buffer. This follows
	// from the parameter already occupying both bytes (low | high << 8), which
	// leaves nowhere to pack the palette index. If FXBlit actually modulates by
	// the index, both blitters change here and only here.
	static uint16_t Sample(uint8_t /*code*/, uint16_t param) { return param; }

	static void BlitRaw(uint16_t* pDest, int destStride, const uint8_t* pFrame,
		int srcStride, int srcSkipCols, int srcSkipRows, int width, int rows,
		uint16_t param);

	static void BlitRLE(uint16_t* pDest, int destStride, const uint8_t* pRow,
		int srcSkipCols, int srcSkipRows, int width, int rows,
		uint16_t param, const uint16_t* pBufferEnd);

	// GPU side.
	com_ptr<IDirect3DTexture9> _texture {};
	reshade::api::resource_view _srv {};
	reshade::api::device* _reshade_device { nullptr };
	IDirect3DDevice9* _device { nullptr };

	// CPU side. 16 bits per pixel.
	std::vector<uint16_t> _draw {};
	std::vector<uint16_t> _upload {};
	std::vector<FXEntry>  _queue {};

	std::mutex _mutex {};

	int  _width { 0 };
	int  _height { 0 };
	bool _pass_active { false };
	bool _upload_pending { false };
};