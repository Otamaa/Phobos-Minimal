#pragma once

// ---------------------------------------------------------------------------
// PhobosAlphaMask
//
// A per-frame, one-byte-per-pixel mask that the GAME writes into, and that
// ReShade effects sample. Anims whose type has FXLightEnable blit their SHP
// into this buffer during the tactical draw pass; the finished buffer is
// uploaded to a D3DFMT_L8 texture and bound to the effect runtime under a
// texture semantic, so any .fx can read "where did the game draw light-emitting
// art this frame".
//
// ---------------------------------------------------------------------------
// DIFF vs the reverse-engineered original
// ---------------------------------------------------------------------------
// The original DLL hacked two pointers into reshade::runtime itself, at fixed
// offsets +0x750 (texture) and +0x754 (surface), past the end of the upstream
// object. That works exactly once, against exactly one build: any ReShade
// revision that adds or reorders a member silently relocates those slots, and
// nothing warns you. It is also why that DLL had to hard-pin a 0x760-byte
// runtime.
//
// Newer ReShade has a first-class mechanism for precisely this - the same one
// the depth-buffer add-on uses:
//
//     runtime::update_texture_bindings(semantic, srv)
//
// An effect declares `texture Mask : PHOBOS_ALPHAMASK;` and the runtime wires
// our resource view into every effect that asks for it, across reloads. No
// offsets, no struct surgery, survives rebases.
//
// Other deliberate changes from the original, each marked at its site:
//   - D3DPOOL_MANAGED -> D3DPOOL_DEFAULT + D3DUSAGE_DYNAMIC. MANAGED is
//     REJECTED outright by D3D9Ex, and this is a per-frame-updated texture,
//     which is the exact case DYNAMIC exists for. This also removes the need
//     for the separate `_mask_surface` staging member entirely.
//   - Double-buffered so the upload never reads the buffer the game is
//     writing, and the mutex is never held across a D3D call.
//   - The four EndPass / BeginPass bugs are fixed (see .cpp).
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

class PhobosAlphaMask
{
public:
	// The semantic an .fx must declare to receive this texture:
	//
	//     texture PhobosAlphaMaskTex : PHOBOS_ALPHAMASK;
	//     sampler sPhobosAlphaMask { Texture = PhobosAlphaMaskTex; };
	//
	// Effects that do not declare it are completely unaffected.
	static constexpr const char* Semantic = "PHOBOS_ALPHAMASK";

	static PhobosAlphaMask& Instance();

	// Called once from PhobosReShade::Initialize, and again on every
	// resolution change. Safe to call with a null device to tear down.
	bool OnInit(IDirect3DDevice9* pDevice, reshade::api::device* pReShadeDevice, int width, int height);
	void OnReset();

	// Called from PhobosReShade's round-trip, AFTER the tactical draw pass has
	// ended and BEFORE the effects pass runs. Uploads the completed buffer and
	// (re)binds it to the runtime.
	void UploadAndBind(reshade::runtime* pRuntime);

	// --- Draw-pass interface, called from the ASMJIT hooks -----------------

	// TacticalClass draw pass entry (0x6D8F0F).
	void BeginPass();

	// TacticalClass draw pass exit (0x6D97BF).
	void EndPass();

	// Blit one SHP frame into the active mask buffer. Returns false when the
	// call was rejected (no active pass, clipped away, bad shape).
	bool Blit(SHPCaches* pShape, int frame, DWORD flags,
		const RectangleStruct& bounds, int x, int y);

	// True only between BeginPass and EndPass. The hooks test this so the
	// per-anim FXLightEnable lookup is skipped entirely outside a pass.
	bool IsPassActive() const { return _pass_active; }

private:
	PhobosAlphaMask() = default;

	static bool BlitRaw(uint8_t* pDest, int destStride, const uint8_t* pFrame,
		int srcStride, int srcSkipCols, int srcSkipRows, int width, int rows);

	static bool BlitRLE(uint8_t* pDest, int destStride, const uint8_t* pRow,
		int srcSkipCols, int srcSkipRows, int width, int rows,
		const uint8_t* pRowEnd);

	// GPU side. No staging surface: DYNAMIC textures are lockable directly.
	com_ptr<IDirect3DTexture9> _texture;
	reshade::api::resource_view _srv {};
	reshade::api::device* _reshade_device { nullptr };
	IDirect3DDevice9* _device { nullptr };

	// CPU side. _draw is what Blit writes into; _upload is what UploadAndBind
	// reads. Swapped under the lock in EndPass, so the two never alias and the
	// mutex is never held across a LockRect.
	std::vector<uint8_t> _draw;
	std::vector<uint8_t> _upload;

	// Non-null only inside a pass. Doubles as the "are we in a tactical draw
	// pass" guard, same role as the original's dword_10298580.
	uint8_t* _active { nullptr };

	std::mutex _mutex;

	int _width { 0 };
	int _height { 0 };

	// DIFF/BUGFIX: the original gated EndPass's unlock on re-reading the same
	// global BeginPass tested. If that global changed mid-pass you either
	// deadlocked the next pass or unlocked a mutex this thread did not own.
	// Latching the state at BeginPass makes the pairing unconditional.
	bool _pass_active { false };

	bool _upload_pending { false };
};