#include "PhobosFXDebug.h"

#include <Utilities/Debug.h>

#include <Windows.h>

#include <algorithm>
#include <iterator>

namespace
{
	// Grey/colour gain. The interesting values sit far below 65535 - the
	// AlphaBuffer neutral is 127, so an ungained view is visually black.
	constexpr int DisplayGain = 96;

	// Values FORCE ALL cycles through. Chosen to straddle the 127 neutral so
	// both darkening and brightening are reachable, and deliberately excluding
	// 127 itself - Queue() rejects a record equal to the clear value.
	//
	//   0      full dark
	//   64     below neutral
	//   300    modestly above
	//   1200   clearly above
	//   20000  saturated, visible even with gain at 1
	constexpr uint16_t ForcedValues[] = { 0, 64, 300, 1200, 20000 };
	constexpr int ForcedValueCount = static_cast<int>(std::size(ForcedValues));

	const char* ModeName(PhobosFXDebug::Mode m)
	{
		switch (m)
		{
		case PhobosFXDebug::Mode::Raw:       return "Raw";
		case PhobosFXDebug::Mode::Deviation: return "Deviation";
		case PhobosFXDebug::Mode::Coverage:  return "Coverage";
		case PhobosFXDebug::Mode::Records:   return "Records";
		default:                             return "Off";
		}
	}

	// VERIFY: assumes a 16-bit RGB565 surface, which is YR's normal mode. If
	// the mod runs a 32-bit surface this writes garbage - check
	// DSurface::Composite->GetBytesPerPixel() before trusting the output.
	constexpr uint16_t Pack565(int r, int g, int b)
	{
		const int rr = std::clamp(r, 0, 255) >> 3;
		const int gg = std::clamp(g, 0, 255) >> 2;
		const int bb = std::clamp(b, 0, 255) >> 3;

		return static_cast<uint16_t>((rr << 11) | (gg << 5) | bb);
	}

	// Rising-edge detection so a held key cycles once, not once per frame.
	bool KeyPressed(int vk, bool& wasDown)
	{
		const bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
		const bool fired = down && !wasDown;

		wasDown = down;
		return fired;
	}
}

uint16_t PhobosFXDebug::ForcedParam() const
{
	return ForcedValues[_forced_index % ForcedValueCount];
}

PhobosFXDebug& PhobosFXDebug::Instance()
{
	static PhobosFXDebug instance;
	return instance;
}

void PhobosFXDebug::PollInput()
{
	if (KeyPressed(VK_F9, _mode_key_down))
	{
		_mode = static_cast<Mode>(
			(static_cast<int>(_mode) + 1) % static_cast<int>(Mode::Count));

		Debug::Log("[PhobosFXDebug] mode = %s\n", ModeName(_mode));
	}

	if (KeyPressed(VK_F8, _force_key_down))
	{
		_force_all = !_force_all;

		Debug::Log("[PhobosFXDebug] force all = %s (value %u)\n",
			_force_all ? "ON" : "off", ForcedParam());
	}

	if (KeyPressed(VK_F7, _value_key_down))
	{
		_forced_index = (_forced_index + 1) % ForcedValueCount;
		Debug::Log("[PhobosFXDebug] forced value = %u\n", ForcedParam());
	}

	if (KeyPressed(VK_F10, _channel_key_down))
	{
		_channel = static_cast<FXSemantic>(
			(static_cast<size_t>(_channel) + 1) % static_cast<size_t>(FXSemantic::Count));

		Debug::Log("[PhobosFXDebug] channel = %s\n",
			PhobosFXBufferManager::Instance().Get(_channel).Semantic());
	}
}

void PhobosFXDebug::Render()
{
	PollInput();

	// Note PollInput runs before this return: FORCE ALL has to be toggleable
	// while the visual overlay is off, since it also affects what the ReShade
	// effect path sees.
	if (_mode == Mode::Off)
		return;

	PhobosFXBufferManager& manager = PhobosFXBufferManager::Instance();

	if (!manager.IsEnabled())
	{
		// Worth saying out loud - a disabled manager looks identical to an
		// empty buffer, and that ambiguity costs an evening.
		if (--_log_cooldown <= 0)
		{
			_log_cooldown = 60;
			Debug::Log("[PhobosFXDebug] manager disabled - buffers never allocated\n");
		}

		return;
	}

	if (_mode == Mode::Records)
	{
		LogRecords();
		return;
	}

	DrawChannel(manager.Get(_channel));
}

void PhobosFXDebug::LogRecords()
{
	if (--_log_cooldown > 0)
		return;

	_log_cooldown = 60;

	PhobosFXBufferManager& manager = PhobosFXBufferManager::Instance();

	for (size_t i = 0; i < static_cast<size_t>(FXSemantic::Count); ++i)
	{
		const PhobosFXBuffer& buffer = manager.Get(static_cast<FXSemantic>(i));
		const std::vector<uint16_t>& pixels = buffer.Published();

		// Count how many pixels actually differ from the clear value. A record
		// count above zero with a touched count of zero means records are being
		// queued but the blitter is rejecting or misplacing every one of them -
		// a completely different bug from "nothing is queueing".
		size_t touched = 0;
		uint16_t lo = 0xFFFF;
		uint16_t hi = 0;

		for (const uint16_t v : pixels)
		{
			if (v != buffer.ClearValue())
				++touched;

			lo = std::min(lo, v);
			hi = std::max(hi, v);
		}

		Debug::Log("[PhobosFXDebug] %-18s %4dx%-4d records=%-5u touched=%-8u min=%-5u max=%u\n",
			buffer.Semantic(),
			buffer.Width(), buffer.Height(),
			static_cast<unsigned>(buffer.LastRecordCount()),
			static_cast<unsigned>(touched),
			pixels.empty() ? 0u : lo,
			hi);
	}
}

void PhobosFXDebug::DrawChannel(const PhobosFXBuffer& buffer)
{
	const std::vector<uint16_t>& pixels = buffer.Published();

	if (pixels.empty() || DSurface::Composite() == nullptr)
		return;

	const int bw = buffer.Width();
	const int bh = buffer.Height();
	const uint16_t clear = buffer.ClearValue();

	auto* const pSurface = DSurface::Composite();

	auto* pBits = static_cast<uint8_t*>(pSurface->Lock(0, 0));

	if (pBits == nullptr)
		return;

	const int pitch = pSurface->Get_Pitch();
	const int surfaceWidth = pSurface->Get_Width();
	const int surfaceHeight = pSurface->Get_Height();

	// Tactical-space buffers are smaller than the surface, so they are drawn at
	// the origin rather than stretched. A screen-space buffer lines up 1:1.
	const int drawWidth = std::min(bw, surfaceWidth);
	const int drawHeight = std::min(bh, surfaceHeight);

	for (int y = 0; y < drawHeight; ++y)
	{
		auto* const pRow = reinterpret_cast<uint16_t*>(pBits + static_cast<size_t>(pitch) * y);
		const uint16_t* const pSrc = pixels.data() + static_cast<size_t>(bw) * y;

		for (int x = 0; x < drawWidth; ++x)
		{
			const int v = pSrc[x];
			const int deviation = v - static_cast<int>(clear);

			switch (_mode)
			{
			case Mode::Raw:
			{
				const int grey = std::clamp(v * DisplayGain / 256, 0, 255);
				pRow[x] = Pack565(grey, grey, grey);
				break;
			}

			case Mode::Deviation:
			{
				const int mag = std::clamp(std::abs(deviation) * DisplayGain / 32, 0, 255);

				if (deviation == 0)
					pRow[x] = 0;
				else if (deviation > 0)
					pRow[x] = Pack565(0, mag, 0);
				else
					pRow[x] = Pack565(mag, 0, 0);

				break;
			}

			case Mode::Coverage:
			{
				// Leave the game image alone where nothing was written, so the
				// overlay can be checked against what is actually on screen.
				if (deviation != 0)
					pRow[x] = Pack565(0, 255, 255);

				break;
			}

			default:
				break;
			}
		}
	}

	pSurface->Unlock();
}
