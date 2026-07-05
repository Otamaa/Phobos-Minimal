#pragma once

#include "ASMMacros.h"
#include <Base/Always.h>

struct Base64
{
	static constexpr char _pad = '=';
	static constexpr char encoder[65] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	// ─── Decoder table ────────────────────────────────────────────────────────────
	// .rdata:0x7E3914  256 entries, one per byte value 0x00–0xFF
	//  -2 (0xFE) = invalid / whitespace — skip
	//  -1 (0xFF) = pad '='              — flush and stop
	//   0–63     = valid Base64 value
	static constexpr signed char decoder[256] = {
		// 0x00–0x2A (43 bytes): invalid
		-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  // 0x00
		-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  // 0x10
		-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,                       // 0x20–0x2A
		// 0x2B '+'  = 62
		62,
		// 0x2C–0x2E: invalid
		-2, -2, -2,
		// 0x2F '/'  = 63
		63,
		// 0x30–0x39 '0'–'9' = 52–61
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
		// 0x3A–0x3C: invalid
		-2, -2, -2,
		// 0x3D '='  = pad (-1)
		-1,
		// 0x3E–0x40: invalid
		-2, -2, -2,
		// 0x41–0x5A 'A'–'Z' = 0–25
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
		13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
		// 0x5B–0x60: invalid
		-2, -2, -2, -2, -2, -2,
		// 0x61–0x7A 'a'–'z' = 26–51
		26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
		39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
		// 0x7B–0xFF: all invalid (133 entries)
		-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  // 0x7B
		-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  // 0x8B
		-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  // 0x9B
		-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  // 0xAB
		-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  // 0xBB
		-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  // 0xCB
		-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  // 0xDB
		-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  // 0xEB
		-2, -2, -2, -2, -2                                                  // 0xFB–0xFF
	};

	//		JMP_STD(0x42FD30);
	static int __fastcall Encode(const void* source, int slen, void* dest, int dlen) {
	
		if (!source || !slen || !dest || !dlen)
			return 0;

		auto const* src = static_cast<unsigned char const*>(source);
		auto* dst = static_cast<unsigned char*>(dest);
		int         written = 0;

		while (slen > 0 && dlen >= 4) {
			// Read up to 3 bytes; padCount counts missing bytes (0, 1, or 2)
			unsigned int packet = 0;
			int          padCount = 0;

			packet = static_cast<unsigned int>(*src++) << 16;
			--slen;

			if (slen > 0) {
				packet |= static_cast<unsigned int>(*src++) << 8;
				--slen;

				if (slen > 0) {
					packet |= static_cast<unsigned int>(*src++);
					--slen;
				} else {
					padCount = 1;   // two bytes read → one '=' pad
				}
			} else {
				padCount = 2;       // one byte read → two '=' pads
			}

			// Emit 4 Base64 characters
			dst[0] = encoder[(packet >> 18) & 0x3F];
			dst[1] = encoder[(packet >> 12) & 0x3F];
			dst[2] = (padCount >= 2) ? _pad : encoder[(packet >> 6) & 0x3F];
			dst[3] = (padCount >= 1) ? _pad : encoder[(packet >> 0) & 0x3F];

			dst += 4;
			dlen -= 4;
			written += 4;
		}

		// Null-terminate if output space remains (assembly: mov byte ptr [ecx], 0)
		if (dlen > 0)
			*dst = '\0';

		return written;
	}

	//		JMP_STD(0x42FE50);
	static int __fastcall Decode(const void* source, int slen, void* dest, int dlen)
	{ 
		if (!source || !slen || !dest || !dlen)
			return 0;

		auto const* src = static_cast<unsigned char const*>(source);
		auto* dst = static_cast<unsigned char*>(dest);
		int         written = 0;

		while (slen > 0 && dlen > 0) {
			unsigned int packet = 0;
			int          groupIdx = 0;   // edi: counts valid chars placed into packet (0–4)
			bool         flushNow = false;

			// Inner loop: accumulate 4 valid Base64 chars, skipping invalid bytes
			while (groupIdx < 4) {
				if (slen <= 0) {
					flushNow = true;
					break;
				}

				unsigned char const raw = *src++;
				--slen;
				signed char const   entry = decoder[static_cast<unsigned char>(raw)];

				if (entry == static_cast<signed char>(-2))
					continue;   // 0xFE: invalid / whitespace — skip (loc_42FF0B inc edi skipped)

				if (entry == static_cast<signed char>(-1)) {
					// 0xFF: pad '=' — store packet, zero slen, flush (loc_42FF61)
					flushNow = true;
					slen = 0;
					break;
				}

				// Pack 6 bits into correct position (switch at loc_42FED6–42FF00)
				static constexpr int          shifts[4] = { 18, 12, 6, 0 };
				static constexpr unsigned int masks[4] = {
					0xFF03FFFFu, 0xFFFC0FFFu, 0xFFFFF03Fu, 0xFFFFFFC0u
				};
				packet = (packet & masks[groupIdx]) | ((static_cast<unsigned int>(entry) & 0x3F) << shifts[groupIdx]);
				++groupIdx;
			}

			// Flush decoded bytes (loc_42FF14–42FF4D)
			// Assembly always writes byte2 first, then byte1 if groupIdx>2, byte0 if groupIdx>3
			if (dlen > 0) {
				*dst++ = static_cast<unsigned char>((packet >> 16) & 0xFF);
				++written;
				--dlen;
			}

			if (dlen > 0 && groupIdx > 2) {
				*dst++ = static_cast<unsigned char>((packet >> 8) & 0xFF);
				++written;
				--dlen;
			}

			if (dlen > 0 && groupIdx > 3) {
				*dst++ = static_cast<unsigned char>(packet & 0xFF);
				++written;
				--dlen;
			}

			if (flushNow || slen <= 0)
				break;
		}

		return written;
	}
};
