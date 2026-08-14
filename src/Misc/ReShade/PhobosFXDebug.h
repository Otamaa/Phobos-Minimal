#pragma once

// ===========================================================================
// PhobosFXDebug - visualise FX buffers without involving ReShade
//
// Writes the buffer contents straight into the game's own surface, so it works
// whether or not effects load, whether or not the ReShade overlay draws, and
// whether or not the texture semantics bind. If the game renders at all, this
// renders.
//
// That isolation is the point. A blank ReShade effect could mean the buffer is
// empty, the upload failed, the semantic did not bind, or the shader did not
// compile. This tells you which.
//
// ---------------------------------------------------------------------------
// CONTROLS
// ---------------------------------------------------------------------------
//
//   F8    toggle FORCE ALL   ignore every extension gate, emit for all art
//   F7    cycle forced value  0 / 64 / 300 / 1200 / 20000
//   F9    cycle mode          Off -> Raw -> Deviation -> Coverage -> Records
//   F10   cycle channel       LightMask -> Light -> TopMask -> ZBuffer -> ...
//
// FORCE ALL exists so the pipeline can be seen working before any INI or
// extension data is authored. With it on, every anim and bullet that reaches a
// producer hook contributes the forced value, and no AnimTypeExtData member is
// read at all - see PHOBOS_FX_USE_EXT below.
//
// Keys are polled, not hooked, so they work regardless of the message pump and
// cannot interfere with the game's own input handling.
//
// ---------------------------------------------------------------------------
// READING IT
// ---------------------------------------------------------------------------
//
//   Raw        grey = value/65535, gained. Untouched pixels read the clear
//              value, so LightMask idles at a dim uniform grey (127/65535),
//              NOT black. A pure black screen means the buffer is zeroed,
//              which is itself the answer to the clear-value question.
//
//   Deviation  green above the clear value, red below, black exactly at it.
//              On an empty map with no light-emitting art this should be
//              ENTIRELY BLACK. Any colour is a record landing somewhere.
//
//   Coverage   cyan wherever a pixel differs from the clear value, drawn over
//              the live game image. Use it for REGISTRATION - fire a weapon
//              and check the cyan lands on the explosion, not offset from it.
//
//   Records    no pixels drawn; logs replay counts once per second. Use when
//              the visual modes show nothing and you need to know whether
//              producers are queueing at all.
// ===========================================================================

#include "PhobosFXBuffer.h"

class PhobosFXDebug
{
public:
	enum class Mode
	{
		Off = 0,
		Raw,
		Deviation,
		Coverage,
		Records,

		Count
	};

	static PhobosFXDebug& Instance();

	// True when every extension gate should be bypassed. Producers check this
	// FIRST and skip their ext lookup entirely when set.
	bool ForceAll() const { return _force_all; }

	// The value forced records carry. Guaranteed never to equal a channel's
	// clear value, which Queue() would reject as a no-op.
	uint16_t ForcedParam() const;

	// Call once per frame, after PhobosFXBufferManager::EndFrame() - it reads
	// the published buffer, which only exists after the swap.
	void Render();

	Mode CurrentMode() const { return _mode; }

private:
	PhobosFXDebug() = default;

	void PollInput();
	void DrawChannel(const PhobosFXBuffer& buffer);
	void LogRecords();

	Mode       _mode { Mode::Off };
	FXSemantic _channel { FXSemantic::LightMask };

	bool _force_all { false };
	int  _forced_index { 2 };

	bool _mode_key_down { false };
	bool _channel_key_down { false };
	bool _force_key_down { false };
	bool _value_key_down { false };

	int _log_cooldown { 0 };
};
