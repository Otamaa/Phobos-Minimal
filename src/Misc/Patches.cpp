#include "Patches.h"
#include <Utilities/Macro.h>


#pragma once

#include <AlphaLightingRemapClass.h>
#include <Drawing.h>

// YR always uses WORD565
/* template<typename T = WORD>
template<size_t Translucency>
class RLEBlitTransLucentAlphaZRead
{
public:
	static_assert(Translucency <= 32u, "Translucency must range in [0, 32]!");

	virtual ~RLEBlitTransLucentAlphaZRead() = default;

	virtual void Blit(WORD* pDst, byte* pSrc, int nLength, int nLineStart, int zBase, WORD* zBuf, WORD* aBuf, DWORD dwAlphaLevel, DWORD dwUnknown, char* zAdjustBuf)
	{
		// fast mapping dwAlphaLevel from [0, 2000] to [0, 254]
		int nAlphaLookup = MaxImpl(254, (261 * MinImpl(dwAlphaLevel, 0)) >> 11);
		const auto& AlphaData = AlphaRemap->Table[nAlphaLookup];

		// RLE Preprocess
		if (nLinestart > 0)
		{
			int off = -nLineStart;
			do
			{
				if (*pSrc++)
					++off;
				else
					off += *pSrc++;
			}
			while (off < 0);

			pDst += off;
			nLength -= off;
			zBuf += off;
			aBuf += off;
			if (zBuf >= ZBuffer::Instance->BufferTail)
				zBuf -= ZBuffer::Instance->BufferSize / sizeof(WORD);
			if (aBuf >= ABuffer::Instance->BufferTail)
				aBuf -= ABuffer::Instance->BufferSize / sizeof(WORD);
		}

		for (int len = nLength; nLength > 0; len = nLength)
		{
			if (auto srcv = *pSrc++)
			{
				// Not RLE Compressed
				auto zVal = zBase - *zAdjustBuf++;
				if (zVal < *zBuf++)
				{
					// extend to 32-bit as the formula required
					DWORD fg = PaletteDatas[AlphaData[*aBuf] | srcv];
					DWORD bg = *pDst;

					// Converts  0000000000000000rrrrrggggggbbbbb
					//     into  00000gggggg00000rrrrr000000bbbbb
					// with mask 00000111111000001111100000011111
					// This is useful because it makes space for a parallel fixed-point multiply
					bg = (bg | (bg << 16)) & 0b00000111111000001111100000011111;
					fg = (fg | (fg << 16)) & 0b00000111111000001111100000011111;

					// This implements the linear interpolation formula: result = bg * (1.0 - alpha) + fg * alpha
					// This can be factorized into: result = bg + (fg - bg) * alpha
					// alpha is in Q1.5 format, so 0.0 is represented by 0, and 1.0 is represented by 32
					DWORD result = ((((fg - bg) * Translucency) >> 5) + bg) & 0b00000111111000001111100000011111;
					*pDst = static_cast<WORD>((result >> 16) | result);
				}
				pDst++;
				nLength--;
				aBuf++;
			}
			else
			{
				// RLE Compressed
				auto off = *pSrc++;
				nLength -= off;
				pDst += off;
				zBuf += off;
				aBuf += off;
				zAdjustBuf += off;
			}

			if (zBuf >= ZBuffer::Instance->BufferTail)
				zBuf -= ZBuffer::Instance->BufferSize / sizeof(WORD);
			if (aBuf >= ABuffer::Instance->BufferTail)
				aBuf -= ABuffer::Instance->BufferSize / sizeof(WORD);
		}
	}

	virtual void BlitTint(WORD* pDst, byte* pSrc, int nLength, int nLineStart, int zBase, WORD* zBuf, WORD* aBuf, DWORD dwAlphaLevel, DWORD dwUnknown, char* zAdjustBuf, DWORD dwTint)
	{
		Blit(pDst, pSrc, nLength, nLineStart, zBase, zBuf, aBuf, dwAlphaLevel, dwUnknown, zAdjustBuf);
	}

	WORD* PaletteDatas; // ConvertClass->FullColorData
	AlphaLightingRemapClass* AlphaRemap; // ConvertClass->ShadeCount AlphaLightingRemap
	WORD MaxColorMask; // Just WW's Bitmask, we don't need it

	static_assert(sizeof(RLEBlitTransLucentAlphaZRead) == 0x10);
};
*/
// Author: Apollo

void BlittersFix::Apply() //C3 Z-aware SHP translucency fixes
{
	// 25% translucency blitter
	Patch::Apply_RAW(0x4989EE, {
		0x66, 0xBE, 0xDE, 0xF7,					// mov    si, 0xF7DE
		0x66, 0x8B, 0x0A,						// mov    cx, WORD PTR [edx]
		0x31, 0xC0,								// xor    eax, eax
		0x66, 0x8B, 0x44, 0x4D, 0x00,			// mov    ax, WORD PTR [ebp+ecx*2+0x0]
		0x8B, 0x4C, 0x24, 0x18,					// mov    ecx, DWORD PTR [esp+0x18]
		0x81, 0xE1, 0xFF, 0x00, 0x00, 0x00,		// and    ecx, 0xDD
		0x8B, 0x6C, 0x24, 0x2C,					// mov    ebp, DWORD PTR [esp+0x2C]
		0x09, 0xC8,								// or     eax, ecx
		0x8B, 0x4C, 0x24, 0x10,					// mov    ecx, DWORD PTR [esp+0x10]
		0x8B, 0x49, 0x04,						// mov    ecx, DWORD PTR [ecx+0x4]
		0x8B, 0x04, 0x41,						// mov    eax, DWORD PTR [ecx+eax*2]
		0x89, 0xC1,								// mov    ecx, eax
		0x33, 0x0F,								// xor    ecx, DWORD PTR [edi]
		0x21, 0xF1,								// and    ecx, esi
		0xD1, 0xE9,								// shr    ecx, 1
		0x50,									// push   eax
		0x0B, 0x07,								// or     eax, DWORD PTR [edi]
		0x29, 0xC8,								// sub    eax, ecx
		0x59,									// pop    ecx
		0x31, 0xC1,								// xor    ecx, eax
		0x09, 0xC8,								// or     eax, ecx
		0x21, 0xF1,								// and    ecx, esi
		0xD1, 0xE9,								// shr    ecx, 1
		0x29, 0xC8,								// sub    eax, ecx
		0x66, 0x89, 0x07,						// mov    WORD PTR [edi], ax
		0x47,									// inc    edi
		0x47									// inc    edi
		}
	);

	// 50% translucency blitter pt. 1
	Patch::Apply_RAW(0x4987F7,{
		0x66, 0xBA, 0xDE, 0xF7	// mov	dx, 0xF7DE
	 });

	// 50% translucency blitter pt. 2
	Patch::Apply_RAW(0x498817,{
		0x8B, 0x04, 0x41,			// mov    eax, DWORD PTR [ecx+eax*2]
		0x89, 0xC1,					// mov    ecx, eax
		0x33, 0x0B,					// xor    ecx, DWORD PTR [ebx]
		0x21, 0xD1,					// and    ecx, edx
		0xD1, 0xE9,					// shr    ecx, 1
		0x0B, 0x03,					// or     eax, DWORD PTR [ebx]
		0x29, 0xC8,					// sub    eax, ecx
		0x90, 0x90, 0x90, 0x90		// nop nop nop nop
	});

	// 75% translucency blitter
	Patch::Apply_RAW(0x4985FE,{
		0x66, 0xBE, 0xDE, 0xF7,					// mov    si, 0xF7DE
		0x66, 0x8B, 0x0A,						// mov    cx, WORD PTR [edx]
		0x31, 0xC0,								// xor    eax, eax
		0x66, 0x8B, 0x44, 0x4D, 0x00,			// mov    ax, WORD PTR [ebp+ecx*2+0x0]
		0x8B, 0x4C, 0x24, 0x18,					// mov    ecx, DWORD PTR [esp+0x18]
		0x81, 0xE1, 0xFF, 0x00, 0x00, 0x00,		// and    ecx, 0xFF
		0x8B, 0x6C, 0x24, 0x2C,					// mov    ebp, DWORD PTR [esp+0x2C]
		0x09, 0xC8,								// or     eax, ecx
		0x8B, 0x4C, 0x24, 0x10,					// mov    ecx, DWORD PTR [esp+0x10]
		0x8B, 0x49, 0x04,						// mov    ecx, DWORD PTR [ecx+0x4]
		0x8B, 0x04, 0x41,						// mov    eax, DWORD PTR [ecx+eax*2]
		0x89, 0xC1,								// mov    ecx, eax
		0x33, 0x0F,								// xor    ecx, DWORD PTR [edi]
		0x21, 0xF1,								// and    ecx, esi
		0xD1, 0xE9,								// shr    ecx, 1
		0x0B, 0x07,								// or     eax, DWORD PTR [edi]
		0x29, 0xC8,								// sub    eax, ecx
		0x89, 0xC1,								// mov    ecx, eax
		0x33, 0x07,								// xor    eax, DWORD PTR [edi]
		0x09, 0xC1,								// or     ecx, eax
		0x21, 0xF0,								// and    eax, esi
		0xD1, 0xE8,								// shr    eax, 1
		0x29, 0xC1,								// sub    ecx, eax
		0x8B, 0x44, 0x24, 0x20,					// mov    eax, DWORD PTR [esp+0x20]
		0x66, 0x89, 0x0F,						// mov    WORD PTR [edi], cx
		0x47,									// inc    edi
		0x47									// inc    edi
	});
}

void PoseDirOverride::Apply()
{
	Patch::Apply_RAW(0x41B7BE,{
		0x8B, 0xC0,  // mov    eax , eax
		0x90 // nop
	});
}